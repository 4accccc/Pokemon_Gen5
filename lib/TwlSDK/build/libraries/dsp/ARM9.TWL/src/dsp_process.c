/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_process.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/


#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/pipe.h>

#include "dsp_coff.h"


/*---------------------------------------------------------------------------*/
/* constants */

#define DSP_DPRINTF(...)    ((void) 0)
//#define DSP_DPRINTF OS_TPrintf

// プロセスイメージ内のセクション列挙コールバック。
typedef BOOL (*DSPSectionEnumCallback)(DSPProcessContext *, const COFFFileHeader *, const COFFSectionTable *);


/*---------------------------------------------------------------------------*/
/* variables */

// 現在実行中のプロセスコンテキスト。
static DSPProcessContext   *DSPiCurrentComponent = NULL;
static PMExitCallbackInfo   DSPiShutdownCallbackInfo[1];
static BOOL                 DSPiShutdownCallbackIsRegistered = FALSE;
static PMSleepCallbackInfo  DSPiPreSleepCallbackInfo[1];
static BOOL                 DSPiPreSleepCallbackIsRegistered = FALSE;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         DSP_EnumSections

  Description:  プロセスイメージ内のセクションを列挙して所定の処理を実行。

  Arguments:    context  : コールバック引数に渡すDSPProcessContext構造体。
                callback : 各セクションに対して呼び出されるコールバック。

  Returns:      全てのセクションが列挙されればTRUE、途中で終了したらFALSE。
 *---------------------------------------------------------------------------*/
BOOL DSP_EnumSections(DSPProcessContext *context, DSPSectionEnumCallback callback);

/*---------------------------------------------------------------------------*
  Name:         DSP_StopDSPComponent

  Description:  DSPの終了準備を行う。
                現在はDSPのDMAを停止させているだけ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_StopDSPComponent(void)
{
    DSPProcessContext  *context = DSPiCurrentComponent;
    context->hookFactors = 0;
    DSP_SendData(DSP_PIPE_COMMAND_REGISTER, DSP_PIPE_FLAG_EXIT_OS);
    (void)DSP_RecvData(DSP_PIPE_COMMAND_REGISTER);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_ShutdownCallback

  Description:  システムシャットダウンの強制終了コールバック。

  Arguments:    name : プロセス名。
                       NULLを指定したら最後に登録したプロセスを検索。

  Returns:      指定した名前に合致するDSPProcessContext構造体。
 *---------------------------------------------------------------------------*/
static void DSPi_ShutdownCallback(void *arg)
{
    (void)arg;
    for (;;)
    {
        DSPProcessContext  *context = DSP_FindProcess(NULL);
        if (!context)
        {
            break;
        }
//        OS_TWarning("force-exit %s component.\n", context->name);
        DSP_QuitProcess(context);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_PreSleepCallback

  Description:  スリープ警告コールバック。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_PreSleepCallback(void *arg)
{
#pragma unused( arg )
    OS_TPanic("Could not sleep while DSP is processing.\n");
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_MasterInterrupts

  Description:  DSP割り込み処理

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_MasterInterrupts(void)
{
    DSPProcessContext  *context = DSPiCurrentComponent;
    if (context)
    {
        // DSP割り込み要因のどれかが初めて発生した瞬間しか
        // OSへの割り込み要因フラグ(IF)は発生しないため、
        // 複数のDSP割り込み要因を逐次処理している間に
        // 新たなDSP割り込み要因が発生した可能性があるなら
        // 割り込みハンドラに頼らずこの場で繰り返す必要がある。
        for (;;)
        {
            // DSP割り込み要因になりうるメッセージを一括で判定。
            int     ready = (reg_DSP_SEM | (((reg_DSP_PSTS >> REG_DSP_PSTS_RRI0_SHIFT) & 7) << 16));
            int     factors = (ready & context->hookFactors);
            if (factors == 0)
            {
                break;
            }
            else
            {
                // 該当するフック処理を順に処理。
                // グループ登録されているならまとめて1回だけ通知される。
                while (factors != 0)
                {
                    int     index = (int)MATH_CTZ((u32)factors);
                    factors &= ~context->hookGroup[index];
                    (*context->hookFunction[index])(context->hookUserdata[index]);
                }
            }
        }
    }
    OS_SetIrqCheckFlag(OS_IE_DSP);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_ReadProcessImage

  Description:  プロセスイメージからデータを読み出す。

  Arguments:    context : DSPProcessContext構造体。
                offset  : イメージ内のオフセット。
                buffer  : 転送先バッファ。
                length  : 転送サイズ。

  Returns:      指定のサイズを正しく読み取れたらTRUE。
 *---------------------------------------------------------------------------*/
static BOOL DSPi_ReadProcessImage(DSPProcessContext *context, int offset, void *buffer, int length)
{
    return FS_SeekFile(context->image, offset, FS_SEEK_SET) &&
           (FS_ReadFile(context->image, buffer, length) == length);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_CommitWram

  Description:  指定したセグメントを特定のプロセッサへ割り当て。

  Arguments:    context : DSPProcessContext構造体。
                wram    : Code/Data。
                segment : 切り替えるセグメント番号。
                to      : 切り替えるプロセッサ。

  Returns:      全てのスロットが正しく切り替えられればTRUE。
 *---------------------------------------------------------------------------*/
static BOOL DSPi_CommitWram(DSPProcessContext *context, MIWramPos wram, int segment, MIWramProc to)
{
    BOOL    retval = FALSE;
    int     slot = DSP_GetProcessSlotFromSegment(context, wram, segment);
    // 現在割り当てられているプロセッサから切断。
    if (!MI_IsWramSlotUsed(wram, slot) ||
        MI_FreeWramSlot(wram, slot, MI_WRAM_SIZE_32KB, MI_GetWramBankMaster(wram, slot)) > 0)
    {
        // 指定のプロセッサへ割り当て。
        void   *physicalAddr = (void *)MI_AllocWramSlot(wram, slot, MI_WRAM_SIZE_32KB, to);
        if (physicalAddr != 0)
        {
            // プロセッサごとに所定のオフセットへ移動。
            vu8    *bank = &((vu8*)((wram == MI_WRAM_B) ? REG_MBK_B0_ADDR  : REG_MBK_C0_ADDR))[slot];
            if (to == MI_WRAM_ARM9)
            {
                *bank = (u8)((*bank & ~MI_WRAM_OFFSET_MASK_B) | (slot << MI_WRAM_OFFSET_SHIFT_B));
            }
            else if (to == MI_WRAM_DSP)
            {
                *bank = (u8)((*bank & ~MI_WRAM_OFFSET_MASK_B) | (segment << MI_WRAM_OFFSET_SHIFT_B));
            }
            retval = TRUE;
        }
    }
    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         DSPi_MapAndLoadProcessImageCallback

  Description:  プロセスイメージを1パスでマップ＆ロードするコールバック。

  Arguments:    context : DSPProcessContext構造体。
                header  : COFFファイルヘッダ情報。
                section : 列挙されたセクション。

  Returns:      列挙を継続するならTRUE、終了してよいならFALSE。
 *---------------------------------------------------------------------------*/
static BOOL DSPi_MapAndLoadProcessImageCallback(DSPProcessContext *context,
                                                const COFFFileHeader *header,
                                                const COFFSectionTable *section)
{
    BOOL        retval = TRUE;

    // 単一セクションがCODE/DATAあわせて最大4箇所に配置される可能性があるため、
    // 必要な配置先情報をリストアップしてからまとめて配置する。
    enum
    {
        placement_kind_max = 2, // { CODE, DATA }
        placement_code_page_max = 2,
        placement_data_page_max = 2,
        placement_max = placement_code_page_max + placement_data_page_max
    };
    MIWramPos   wrams[placement_max];
    int         addrs[placement_max];
    BOOL        nolds[placement_max];
    int         placement = 0;

    // 配置されるべきページ番号はセクション名の接尾子から判定することができる。
    // Nameフィールドが文字列を直接あらわす場合は一箇所にのみ配置されるようだが
    // 文字列テーブルから参照すべき長い名前なら複数箇所へ配置される可能性がある。
    const char *name = (const char *)section->Name;
    char        longname[128];
    if (*(u32*)name == 0)
    {
        u32     stringtable = header->PointerToSymbolTable + 0x12 * header->NumberOfSymbols;
        if(!DSPi_ReadProcessImage(context,
                              (int)(stringtable + *(u32*)&section->Name[4]),
                              longname, sizeof(longname)))
        {
            retval = FALSE;
            return retval;
        }
        name = longname;
    }

    // 特殊な目印となるセクションの存在もここで判定する。
    if (STD_CompareString(name, "SDK_USING_OS@d0") == 0)
    {
        context->flags |= DSP_PROCESS_FLAG_USING_OS;
    }

    // CODEセクションはWRAM-B。
    if ((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_CODE_MEMORY) != 0)
    {
        int         baseaddr = (int)(section->s_paddr * 2);
        BOOL        noload = ((section->s_flags & COFF_SECTION_ATTR_NOLOAD_FOR_CODE_MEMORY) != 0);
        if (STD_StrStr(name, "@c0") != NULL)
        {
            wrams[placement] = MI_WRAM_B;
            addrs[placement] = baseaddr + DSP_WRAM_SLOT_SIZE * 4 * 0;
            nolds[placement] = noload;
            ++placement;
        }
        if (STD_StrStr(name, "@c1") != NULL)
        {
            wrams[placement] = MI_WRAM_B;
            addrs[placement] = baseaddr + DSP_WRAM_SLOT_SIZE * 4 * 1;
            nolds[placement] = noload;
            ++placement;
        }
    }

    // DATAセクションはWRAM-C。
    if ((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_DATA_MEMORY) != 0)
    {
        int         baseaddr = (int)(section->s_vaddr * 2);
        BOOL        noload = ((section->s_flags & COFF_SECTION_ATTR_NOLOAD_FOR_DATA_MEMORY) != 0);
        if (STD_StrStr(name, "@d0") != NULL)
        {
            wrams[placement] = MI_WRAM_C;
            addrs[placement] = baseaddr + DSP_WRAM_SLOT_SIZE * 4 * 0;
            nolds[placement] = noload;
            ++placement;
        }
        if (STD_StrStr(name, "@d1") != NULL)
        {
            wrams[placement] = MI_WRAM_C;
            addrs[placement] = baseaddr + DSP_WRAM_SLOT_SIZE * 4 * 1;
            nolds[placement] = noload;
            ++placement;
        }
    }

    // 配置先情報がリストアップできたので各アドレスへ配置。
    {
        int     i;
        for (i = 0; i < placement; ++i)
        {
            MIWramPos   wram = wrams[i];
            int     dstofs = addrs[i];
            int     length = (int)section->s_size;
            int     srcofs = (int)section->s_scnptr;
            // スロット境界をまたがないよう分割ロード。
            while (length > 0)
            {
                // スロット境界でサイズをクリップする。
                int     ceil = MATH_ROUNDUP(dstofs + 1, DSP_WRAM_SLOT_SIZE);
                int     curlen = MATH_MIN(length, ceil - dstofs);
                BOOL    newmapped = FALSE;
                // 該当するセグメントがまだマップされていなければここでマップして初期化。
                if (DSP_GetProcessSlotFromSegment(context, wram, dstofs / DSP_WRAM_SLOT_SIZE) == -1)
                {
                    int     segment = (dstofs / DSP_WRAM_SLOT_SIZE);
                    u16    *slots = (wram == MI_WRAM_B) ? &context->slotB : &context->slotC;
                    int    *segbits = (wram == MI_WRAM_B) ? &context->segmentCode : &context->segmentData;
                    int    *map = (wram == MI_WRAM_B) ? context->slotOfSegmentCode : context->slotOfSegmentData;
                    int     slot = (int)MATH_CountTrailingZeros((u32)*slots);
                    if (slot >= MI_WRAM_B_MAX_NUM)
                    {
                        retval = FALSE;
                        break;
                    }
                    else
                    {
                        newmapped = TRUE;
                        map[segment] = slot;
                        *slots &= ~(1 << slot);
                        *segbits |= (1 << segment);
                        if (!DSPi_CommitWram(context, wram, segment, MI_WRAM_ARM9))
                        {
                            retval = FALSE;
                            break;
                        }
                    }
                    MI_CpuFillFast(DSP_ConvertProcessAddressFromDSP(context, wram,
                                                                    segment * (DSP_WRAM_SLOT_SIZE / 2)),
                                   0, DSP_WRAM_SLOT_SIZE);
                }
                // noload指定なら無視。
                if (nolds[i])
                {
                    DSP_DPRINTF("$%04X (noload)\n", dstofs);
                }
                else
                {
                    // 今回の転送範囲に該当するオフセットを計算。
                    u8     *dstbuf = (u8*)DSP_ConvertProcessAddressFromDSP(context, wram, dstofs / 2);
                    if (!DSPi_ReadProcessImage(context, srcofs, dstbuf, curlen))
                    {
                        retval = FALSE;
                        break;
                    }
                    DSP_DPRINTF("$%04X -> mem:%08X\n", dstofs, dstbuf);
                }
                srcofs += curlen;
                dstofs += curlen;
                length -= curlen;
            }
        }
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_InitProcessContext

  Description:  プロセス情報構造体を初期化。

  Arguments:    context : DSPProcessContext構造体。
                name    : プロセス名またはNULL。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_InitProcessContext(DSPProcessContext *context, const char *name)
{
    int     i;
    int     segment;
    MI_CpuFill8(context, 0, sizeof(*context));
    context->next = NULL;
    context->flags = 0;
    (void)STD_CopyString(context->name, name ? name : "");
    context->image = NULL;
    // セグメントマップを空に。
    context->segmentCode = 0;
    context->segmentData = 0;
    // スロットマップを空に。
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        context->slotOfSegmentCode[segment] = -1;
    }
    for (segment = 0; segment < MI_WRAM_C_MAX_NUM; ++segment)
    {
        context->slotOfSegmentData[segment] = -1;
    }
    // フック登録状況を空に。
    context->hookFactors = 0;
    for (i = 0; i < DSP_HOOK_MAX; ++i)
    {
        context->hookFunction[i] = NULL;
        context->hookUserdata[i] = NULL;
        context->hookGroup[i] = 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnumSections

  Description:  プロセスイメージ内のセクションを列挙して所定の処理を実行。

  Arguments:    context  : コールバック引数に渡すDSPProcessContext構造体。
                callback : 各セクションに対して呼び出されるコールバック。

  Returns:      全てのセクションが列挙されればTRUE、途中で終了したらFALSE。
 *---------------------------------------------------------------------------*/
BOOL DSP_EnumSections(DSPProcessContext *context, DSPSectionEnumCallback callback)
{
    BOOL            retval = FALSE;
    // イメージバッファのメモリ境界を考慮して一度テンポラリへコピー。
    COFFFileHeader  header[1];
    if (DSPi_ReadProcessImage(context, 0, header, sizeof(header)))
    {
        int     base = (int)(sizeof(header) + header->SizeOfOptionalHeader);
        int     index;
        for (index = 0; index < header->NumberOfSections; ++index)
        {
            COFFSectionTable    section[1];
            if (!DSPi_ReadProcessImage(context, (int)(base + index * sizeof(section)), section, (int)sizeof(section)))
            {
                break;
            }
            // BLOCK-HEADER 属性が無くサイズ1以上のセクションだけを選別する。
            if (((section->s_flags & COFF_SECTION_ATTR_BLOCK_HEADER) == 0) && (section->s_size != 0))
            {
                if (callback && !(*callback)(context, header, section))
                {
                    break;
                }
            }
        }
        retval = (index >= header->NumberOfSections);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_AttachProcessMemory

  Description:  プロセスの未使用領域に連続したメモリ空間を割り当てる。

  Arguments:    context : DSPProcessContext構造体。
                wram    : 割り当てるメモリ空間。 (MI_WRAM_B/MI_WRAM_C)
                slots   : 割り当てるWRAMスロット。

  Returns:      割り当てられたDSPメモリ空間の先頭アドレスまたは0。
 *---------------------------------------------------------------------------*/
u32 DSP_AttachProcessMemory(DSPProcessContext *context, MIWramPos wram, int slots)
{
    u32     retval = 0;
    int    *segbits = (wram == MI_WRAM_B) ? &context->segmentCode : &context->segmentData;
    int    *map = (wram == MI_WRAM_B) ? context->slotOfSegmentCode : context->slotOfSegmentData;
    // 必要な分だけの連続した未使用領域を検索。
    int     need = (int)MATH_CountPopulation((u32)slots);
    u32     region = (u32)((1 << need) - 1);
    u32     space = (u32)(~*segbits & 0xFF);
    int     segment = 0;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        // DSPのページ境界(4セグメント)には様々な制限があるので
        // これをまたがないように確保するよう注意。
        if ((((segment ^ (segment + need - 1)) & 4) == 0) &&
            (((space >> segment) & region) == region))
        {
            // 充分な未使用領域が存在すれば下位順にスロットを割り当てる。
            retval = (u32)(DSP_ADDR_TO_DSP(DSP_WRAM_SLOT_SIZE) * segment);
            while (slots)
            {
                int     slot = (int)MATH_CountTrailingZeros((u32)slots);
                map[segment] = slot;
                slots &= ~(1 << slot);
                *segbits |= (1 << segment);
                segment += 1;
            }
            break;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DetachProcessMemory

  Description:  プロセスの使用領域に割り当てられたWRAMスロットを解放する。

  Arguments:    context : DSPProcessContext構造体。
                slots   : 割り当て済みのWRAMスロット。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DetachProcessMemory(DSPProcessContext *context, MIWramPos wram, int slots)
{
    int    *segbits = (wram == MI_WRAM_B) ? &context->segmentCode : &context->segmentData;
    int    *map = (wram == MI_WRAM_B) ? context->slotOfSegmentCode : context->slotOfSegmentData;
    int     segment;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        if ((((1 << segment) & *segbits) != 0) && (((1 << map[segment]) & slots) != 0)) 
        {
            *segbits &= ~(1 << segment);
            map[segment] = -1;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SwitchProcessMemory

  Description:  指定のプロセスが使用中のDSPアドレス管理を切り替える。

  Arguments:    context : DSPProcessContext構造体。
                wram    : 切り替えるメモリ空間。 (MI_WRAM_B/MI_WRAM_C)
                address : 切り替える先頭アドレス。(DSPメモリ空間のワード単位)
                length  : 切り替えるメモリサイズ。(DSPメモリ空間のワード単位)
                to      : 新しいマスタープロセッサ。

  Returns:      全てのスロットが正しく切り替えられればTRUE。
 *---------------------------------------------------------------------------*/
BOOL DSP_SwitchProcessMemory(DSPProcessContext *context, MIWramPos wram, u32 address, u32 length, MIWramProc to)
{
    address = DSP_ADDR_TO_ARM(address);
    length = DSP_ADDR_TO_ARM(MATH_MAX(length, 1));
    {
        int    *segbits = (wram == MI_WRAM_B) ? &context->segmentCode : &context->segmentData;
        int     lower = (int)(address / DSP_WRAM_SLOT_SIZE);
        int     upper = (int)((address + length - 1) / DSP_WRAM_SLOT_SIZE);
        int     segment;
        for (segment = lower; segment <= upper; ++segment)
        {
            if ((*segbits & (1 << segment)) != 0)
            {
                if (!DSPi_CommitWram(context, wram, segment, to))
                {
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_MapAndLoadProcessImage

  Description:  指定されたプロセスイメージを1パスでマップ＆ロード。
                セグメントマップは算出されていなくてもよい。

  Arguments:    context : DSPProcessContext構造体。
                image   : プロセスイメージを指すファイルハンドル。
                          この関数内でのみ参照される。
                slotB   : コードメモリのために使用を許可されたWRAM-B。
                slotC   : データメモリのために使用を許可されたWRAM-C。

  Returns:      全てのイメージが正しくロードされればTRUE。
 *---------------------------------------------------------------------------*/
static BOOL DSP_MapAndLoadProcessImage(DSPProcessContext *context, FSFile *image, int slotB, int slotC)
{
    BOOL    retval = FALSE;
    const u32   dspMemSize = DSP_ADDR_TO_DSP(DSP_WRAM_SLOT_SIZE) * MI_WRAM_B_MAX_NUM;
    // プロセスイメージを実際にロードしながらWRAMをマッピング。
    context->image = image;
    context->slotB = (u16)slotB;
    context->slotC = (u16)slotC;

    if (DSP_EnumSections(context, DSPi_MapAndLoadProcessImageCallback))
    {
        DC_FlushRange((const void*)MI_GetWramMapStart_B(), MI_WRAM_B_SIZE);
        DC_FlushRange((const void*)MI_GetWramMapStart_C(), MI_WRAM_C_SIZE);
        // 割り当てられたWRAMスロットを全てDSPに切り替え。
        if (DSP_SwitchProcessMemory(context, MI_WRAM_B, 0, dspMemSize, MI_WRAM_DSP) &&
            DSP_SwitchProcessMemory(context, MI_WRAM_C, 0, dspMemSize, MI_WRAM_DSP))
        {
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetProcessHook

  Description:  プロセスへのDSP割り込み要因に対してフックを設定。

  Arguments:    context  : DSPProcessContext構造体。
                factors  : 指定する割り込み要因のビット集合。
                           ビット0-15がセマフォ、ビット16-18がリプライ。
                function : 呼び出すべきフック関数。
                userdata : フック関数に与える任意のユーザ定義引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetProcessHook(DSPProcessContext *context, int factors, DSPHookFunction function, void *userdata)
{
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    int     i;
    for (i = 0; i < DSP_HOOK_MAX; ++i)
    {
        int     bit = (1 << i);
        if ((bit & factors) != 0)
        {
            context->hookFunction[i] = function;
            context->hookUserdata[i] = userdata;
            context->hookGroup[i] = factors;
        }
    }
    {
        // 割り込み要因を変更。
        int         modrep = (((factors >> 16) & 0x7) << REG_DSP_PCFG_PRIE0_SHIFT);
        int         modsem = ((factors >> 0) & 0xFFFF);
        int         currep = reg_DSP_PCFG;
        int         cursem = reg_DSP_PMASK;
        if (function != NULL)
        {
            reg_DSP_PCFG = (u16)(currep | modrep);
            reg_DSP_PMASK = (u16)(cursem & ~modsem);
            context->hookFactors |= factors;
        }
        else
        {
            reg_DSP_PCFG = (u16)(currep & ~modrep);
            reg_DSP_PMASK = (u16)(cursem | modsem);
            context->hookFactors &= ~factors;
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_HookPostStartProcess

  Description:  DSPプロセスイメージをロードした直後のフック。
                DSPコンポーネント開発者がデバッガを起動するために必要。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void DSP_HookPostStartProcess(void)
{
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ExecuteProcess

  Description:  DSPプロセスイメージをロードして起動。

  Arguments:    context : 状態管理に使用するDSPProcessContext構造体。
                          プロセスを破棄するまでシステムによって参照される。
                image   : プロセスイメージを指すファイルハンドル。
                          この関数内でのみ参照される。
                slotB   : コードメモリのために使用を許可されたWRAM-B。
                slotC   : データメモリのために使用を許可されたWRAM-C。

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL DSP_ExecuteProcess(DSPProcessContext *context, FSFile *image, int slotB, int slotC)
{
    BOOL    retval = FALSE;
    if (!FS_IsAvailable())
    {
        OS_TWarning("FS is not initialized yet.\n");
        FS_Init(FS_DMA_NOT_USE);
    }
    // すべてのコンポーネントに共通のDSPシステム初期化処理。
    DSP_InitPipe();
    OS_SetIrqFunction(OS_IE_DSP, DSPi_MasterInterrupts);
    DSP_SetProcessHook(context,
                       DSP_HOOK_SEMAPHORE_(15) | DSP_HOOK_REPLY_(2),
                       (DSPHookFunction)DSP_HookPipeNotification, NULL);
    (void)OS_EnableIrqMask(OS_IE_DSP);
    // プロセスイメージをメモリへマップしてロード。
    if (!DSP_MapAndLoadProcessImage(context, image, slotB, slotC))
    {
        OS_TWarning("you should check wram\n");
    }
    else
    {
        OSIntrMode  bak_cpsr = OS_DisableInterrupts();
        // プロセスリストにコンテキストを登録。
        DSPProcessContext  **pp = &DSPiCurrentComponent;
        for (pp = &DSPiCurrentComponent; *pp && (*pp != context); pp = &(*pp)->next)
        {
        }
        *pp = context;
        context->image = NULL;
        // シャットダウン時の強制終了コールバックを設定。
        if (!DSPiShutdownCallbackIsRegistered)
        {
            PM_SetExitCallbackInfo(DSPiShutdownCallbackInfo, DSPi_ShutdownCallback, NULL);
            PMi_InsertPostExitCallbackEx(DSPiShutdownCallbackInfo, PM_CALLBACK_PRIORITY_DSP);
            DSPiShutdownCallbackIsRegistered = TRUE;
        }
        // スリープ時の警告コールバックを設定。
        if (!DSPiPreSleepCallbackIsRegistered)
        {
            PM_SetSleepCallbackInfo(DSPiPreSleepCallbackInfo, DSPi_PreSleepCallback, NULL);
            PMi_InsertPreSleepCallbackEx(DSPiPreSleepCallbackInfo, PM_CALLBACK_PRIORITY_DSP);
            DSPiPreSleepCallbackIsRegistered = TRUE;
        }
        // 起動直後にできるだけ早く、必要な割り込み要因だけを設定。
        DSP_PowerOn();
        DSP_ResetOffEx((u16)(context->hookFactors >> 16));
        DSP_MaskSemaphore((u16)~(context->hookFactors >> 0));
        // 起動直後のこのタイミングでARM側を一時停止しておけば
        // DSPデバッガで同じコンポーネントを再ロードしてトレース可能。
        DSP_HookPostStartProcess();
        // ARM9からのコマンドを待たずDSP側から自発的に
        // コマンドを送信するようなコンポーネントの場合、
        // DSPデバッガによって再ロードしてしまうと
        // コマンド値が0に初期化されつつARM9側に未読ビットが残ってしまう。
        // この0をライブラリ内部で読み捨てる。
        if ((context->flags & DSP_PROCESS_FLAG_USING_OS) != 0)
        {
            u16     id;
            for (id = 0; id < 3; ++id)
            {
                vu16    dummy;
                while (dummy = DSP_RecvDataCore(id), dummy != 1)
                {
                }
            }
        }
        // 起動から設定までの間に取りこぼした割り込みがないか念のため確認。
        DSPi_MasterInterrupts();
        retval = TRUE;
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_QuitProcess

  Description:  DSPプロセスを終了してメモリを解放。

  Arguments:    context : 状態管理に使用するDSPProcessContext構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_QuitProcess(DSPProcessContext *context)
{
    OSIntrMode  bak_cpsr;
    
    // まずは TEAK の DMA を停止させる
    DSP_StopDSPComponent();
    
    bak_cpsr = OS_DisableInterrupts();
    // DSPを停止。
    DSP_ResetOn();
    DSP_PowerOff();
    // 割り込みを無効化。
    (void)OS_DisableIrqMask(OS_IE_DSP);
    OS_SetIrqFunction(OS_IE_DSP, NULL);
    // 使用していた全てのWRAMを返却。
    (void)MI_FreeWram(MI_WRAM_B, MI_WRAM_DSP);
    (void)MI_FreeWram(MI_WRAM_C, MI_WRAM_DSP);
    {
        // プロセスリストからコンテキストを解放。
        DSPProcessContext  **pp = &DSPiCurrentComponent;
        for (pp = &DSPiCurrentComponent; *pp; pp = &(*pp)->next)
        {
            if (*pp == context)
            {
                *pp = (*pp)->next;
                break;
            }
        }
        context->next = NULL;
    }
    (void)context;
    (void)OS_RestoreInterrupts(bak_cpsr);

    // スリープ時の警告コールバックを削除。
    PM_DeletePreSleepCallback(DSPiPreSleepCallbackInfo);
    DSPiPreSleepCallbackIsRegistered = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_FindProcess

  Description:  実行中のプロセスを検索。

  Arguments:    name : プロセス名。
                       NULLを指定したら最後に登録したプロセスを検索。

  Returns:      指定した名前に合致するDSPProcessContext構造体。
 *---------------------------------------------------------------------------*/
DSPProcessContext* DSP_FindProcess(const char *name)
{
    DSPProcessContext  *ptr = NULL;
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    ptr = DSPiCurrentComponent;
    if (name)
    {
        for (; ptr && (STD_CompareString(ptr->name, name) != 0); ptr = ptr->next)
        {
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
    return ptr;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ReadProcessPipe

  Description:  プロセスの所定ポートに関連付けられたパイプから受信。

  Arguments:    context : DSPProcessContext構造体。
                port    : 受信元のポート。
                buffer  : 受信データ。
                length  : 受信サイズ。(バイト単位)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ReadProcessPipe(DSPProcessContext *context, int port, void *buffer, u32 length)
{
    DSPPipe input[1];
    (void)DSP_LoadPipe(input, port, DSP_PIPE_INPUT);
    DSP_ReadPipe(input, buffer, (DSPByte)length);
    (void)context;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_WriteProcessPipe

  Description:  プロセスの所定ポートに関連付けられたパイプへ送信。

  Arguments:    context : DSPProcessContext構造体。
                port    : 送信先のポート。
                buffer  : 送信データ。
                length  : 送信サイズ。(バイト単位)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_WriteProcessPipe(DSPProcessContext *context, int port, const void *buffer, u32 length)
{
    DSPPipe output[1];
    (void)DSP_LoadPipe(output, port, DSP_PIPE_OUTPUT);
    DSP_WritePipe(output, buffer, (DSPByte)length);
    (void)context;
}


#if defined(DSP_SUPPORT_OBSOLETE_LOADER)
/*---------------------------------------------------------------------------*
 * 以下は現在使用されていないと思われる廃止候補インタフェース。
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSPi_MapProcessSegmentCallback

  Description:  プロセスが占有するセグメントマップを算出するコールバック。

  Arguments:    context : DSPProcessContext構造体。
                header  : COFFファイルヘッダ情報。
                section : 列挙されたセクション。

  Returns:      列挙を継続するならTRUE、終了してよいならFALSE。
 *---------------------------------------------------------------------------*/
static BOOL DSPi_MapProcessSegmentCallback(DSPProcessContext *context,
                                           const COFFFileHeader *header,
                                           const COFFSectionTable *section)
{
    (void)header;
    // TODO: 除算を避けるとより高速になるかもしれない。
    if((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_CODE_MEMORY) != 0)
    {
        u32     addr = DSP_ADDR_TO_ARM(section->s_paddr);
        int     lower = (int)(addr / DSP_WRAM_SLOT_SIZE);
        int     upper = (int)((addr + section->s_size - 1) / DSP_WRAM_SLOT_SIZE);
        int     segment;
        for(segment = lower; segment <= upper; ++segment)
        {
            context->segmentCode |= (1 << segment);
        }
    }
    else if((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_DATA_MEMORY) != 0)
    {
        u32     addr = DSP_ADDR_TO_ARM(section->s_vaddr);
        int     lower = (int)(addr / DSP_WRAM_SLOT_SIZE);
        int     upper = (int)((addr + section->s_size - 1) / DSP_WRAM_SLOT_SIZE);
        int     segment;
        for(segment = lower; segment <= upper; ++segment)
        {
            context->segmentData |= (1 << segment);
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_MapProcessSlotDefault

  Description:  空き番号を下位から順に使用してスロットマップを初期化。

  Arguments:    context : DSPProcessContext構造体。
                slotB   : コードメモリのために使用を許可されたWRAM-B。
                slotC   : データメモリのために使用を許可されたWRAM-C。

  Returns:      条件を満たすようにスロットマップを確保できればTRUE。
 *---------------------------------------------------------------------------*/
static BOOL DSPi_MapProcessSlotDefault(DSPProcessContext *context, int slotB, int slotC)
{
    BOOL    retval = TRUE;
    int     segment;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        if ((context->segmentCode & (1 << segment)) != 0)
        {
            int     slot = (int)MATH_CountTrailingZeros((u32)slotB);
            if (slot >= MI_WRAM_B_MAX_NUM)
            {
                retval = FALSE;
                break;
            }
            context->slotOfSegmentCode[segment] = slot;
            slotB &= ~(1 << slot);
        }
    }
    for (segment = 0; segment < MI_WRAM_C_MAX_NUM; ++segment)
    {
        if ((context->segmentData & (1 << segment)) != 0)
        {
            int     slot = (int)MATH_CountTrailingZeros((u32)slotC);
            if (slot >= MI_WRAM_C_MAX_NUM)
            {
                retval = FALSE;
                break;
            }
            context->slotOfSegmentData[segment] = slot;
            slotC &= ~(1 << slot);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsProcessMemoryReady

  Description:  プロセスに割り当てられたはずのWRAMスロットが使用中でないか確認。

  Arguments:    context : DSPProcessContext構造体。

  Returns:      全てのWRAMスロットが使用されずに準備状態であればTRUE。
 *---------------------------------------------------------------------------*/
static BOOL DSP_IsProcessMemoryReady(DSPProcessContext *context)
{
    BOOL    retval = TRUE;
    int     segment;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        if ((context->segmentCode & (1 << segment)) != 0)
        {
            int     slot = context->slotOfSegmentCode[segment];
            if (MI_IsWramSlotUsed(MI_WRAM_B, slot))
            {
                OS_TWarning("slot:%d for DSP:%05X is now used by someone.\n", slot, segment * DSP_WRAM_SLOT_SIZE);
                retval = FALSE;
                break;
            }
        }
    }
    for (segment = 0; segment < MI_WRAM_C_MAX_NUM; ++segment)
    {
        if ((context->segmentData & (1 << segment)) != 0)
        {
            int     slot = context->slotOfSegmentData[segment];
            if (MI_IsWramSlotUsed(MI_WRAM_C, slot))
            {
                OS_TWarning("slot:%d for DSP:%05X is now used by someone.\n", slot, segment * DSP_WRAM_SLOT_SIZE);
                retval = FALSE;
                break;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_LoadProcessImageCallback

  Description:  プロセスイメージをグループ方式でロード。

  Arguments:    context : DSPProcessContext構造体。
                header  : COFFファイルヘッダ情報。
                section : 列挙されたセクション。

  Returns:      列挙を継続するならTRUE、終了してよいならFALSE。
 *---------------------------------------------------------------------------*/
static BOOL DSPi_LoadProcessImageCallback(DSPProcessContext *context,
                                          const COFFFileHeader *header,
                                          const COFFSectionTable *section)
{
    BOOL        retval = TRUE;
    MIWramPos   wram = MI_WRAM_A;
    int         dstofs = 0;
    BOOL        noload = FALSE;
    (void)header;
    // CODEセクションはWRAM-B。
    if ((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_CODE_MEMORY) != 0)
    {
        wram = MI_WRAM_B;
        dstofs = (int)(section->s_paddr * 2);
        if ((section->s_flags & COFF_SECTION_ATTR_NOLOAD_FOR_CODE_MEMORY) != 0)
        {
            noload = TRUE;
        }
    }
    // DATAセクションはWRAM-C。
    else if ((section->s_flags & COFF_SECTION_ATTR_MAPPED_IN_DATA_MEMORY) != 0)
    {
        wram = MI_WRAM_C;
        dstofs = (int)(section->s_vaddr * 2);
        if ((section->s_flags & COFF_SECTION_ATTR_NOLOAD_FOR_DATA_MEMORY) != 0)
        {
            noload = TRUE;
        }
    }
    // セグメントごとに適切なWRAMスロットを選択。
    // (「どのWRAMでもない」という定数が存在しないのでWRAM-Aを代用)
    if (wram != MI_WRAM_A)
    {
        // noload指定なら無視。
        if (noload)
        {
            DSP_DPRINTF("$%04X (noload)\n", dstofs);
        }
        else
        {
            // スロット境界をまたがないよう分割ロード。
            int     length = (int)section->s_size;
            int     srcofs = (int)section->s_scnptr;
            while (length > 0)
            {
                // スロット境界でサイズをクリップする。
                int     ceil = MATH_ROUNDUP(dstofs + 1, DSP_WRAM_SLOT_SIZE);
                int     curlen = MATH_MIN(length, ceil - dstofs);
                // 今回の転送範囲に該当するオフセットを計算。
                u8     *dstbuf = (u8*)DSP_ConvertProcessAddressFromDSP(context, wram, dstofs/2);
                if (!DSPi_ReadProcessImage(context, srcofs, dstbuf, length))
                {
                    retval = FALSE;
                    break;
                }
                DSP_DPRINTF("$%04X -> mem:%08X\n", dstofs, dstbuf);
                srcofs += curlen;
                dstofs += curlen;
                length -= curlen;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_MapProcessSegment

  Description:  プロセスが占有するセグメントマップを算出。

  Arguments:    context : DSPProcessContext構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_MapProcessSegment(DSPProcessContext *context)
{
    (void)DSP_EnumSections(context, DSPi_MapProcessSegmentCallback);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadProcessImage

  Description:  指定されたプロセスイメージをロードする。
                セグメントマップはすでに算出済みでなければならない。

  Arguments:    context  : DSPProcessContext構造体。

  Returns:      全てのイメージが正しくロードされればTRUE。
 *---------------------------------------------------------------------------*/
BOOL DSP_LoadProcessImage(DSPProcessContext *context)
{
    BOOL    retval = FALSE;
    // DSPに割り当てられたはずのWRAMスロットが本当に利用可能か確認。
    if (DSP_IsProcessMemoryReady(context))
    {
        const u32   dspMemSize = DSP_ADDR_TO_DSP(DSP_WRAM_SLOT_SIZE) * MI_WRAM_B_MAX_NUM;
        // 割り当てられたWRAMスロットを全てARM9に切り替え。
        if (DSP_SwitchProcessMemory(context, MI_WRAM_B, 0, dspMemSize, MI_WRAM_ARM9) &&
            DSP_SwitchProcessMemory(context, MI_WRAM_C, 0, dspMemSize, MI_WRAM_ARM9))
        {
            // プロセスイメージを実際にロードしてWRAMへFlush。
            if (DSP_EnumSections(context, DSPi_LoadProcessImageCallback))
            {
                DC_FlushRange((const void*)MI_GetWramMapStart_B(), MI_WRAM_B_SIZE);
                DC_FlushRange((const void*)MI_GetWramMapStart_C(), MI_WRAM_C_SIZE);
                // 割り当てられたWRAMスロットを全てDSPに切り替え。
                if (DSP_SwitchProcessMemory(context, MI_WRAM_B, 0, dspMemSize, MI_WRAM_DSP) &&
                    DSP_SwitchProcessMemory(context, MI_WRAM_C, 0, dspMemSize, MI_WRAM_DSP))
                {
                    retval = TRUE;
                }
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StartupProcess

  Description:  プロセスイメージのセグメントマップを算出してWRAMへロードする。
                DSP_MapProcessSegment関数とDSP_LoadProcessImage関数の複合版。

  Arguments:    context    : DSPProcessContext構造体。
                image      : プロセスイメージを指すファイルハンドル。
                             この関数内でのみ参照される。
                slotB      : コードメモリのために使用を許可されたWRAM-B。
                slotC      : データメモリのために使用を許可されたWRAM-C。
                slotMapper : セグメントにWRAMスロットを割り当てるアルゴリズム。
                             NULLを指定すればデフォルトの最適な処理が選択される。

  Returns:      全てのイメージが正しくロードされればTRUE。
 *---------------------------------------------------------------------------*/
BOOL DSP_StartupProcess(DSPProcessContext *context, FSFile *image,
                        int slotB, int slotC, BOOL (*slotMapper)(DSPProcessContext *, int, int))
{
    BOOL    retval = FALSE;
    if (!slotMapper)
    {
        slotMapper = DSPi_MapProcessSlotDefault;
    }
    if (!FS_IsAvailable())
    {
        OS_TWarning("FS is not initialized yet.\n");
        FS_Init(FS_DMA_NOT_USE);
    }
    context->image = image;
    DSP_MapProcessSegment(context);
    if (!(*slotMapper)(context, slotB, slotC) ||
        !DSP_LoadProcessImage(context))
    {
        OS_TWarning("you should check wram\n");
    }
    else
    {
        retval = TRUE;
    }
    context->image = NULL;
    return retval;
}


#endif

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
