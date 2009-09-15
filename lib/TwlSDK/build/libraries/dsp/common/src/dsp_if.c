/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_if.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/dsp.h>

#include <dsp_coff.h>
#include "dsp_process.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define reg_CFG_DSP_RST             *(vu8*)REG_RST_ADDR

#define REG_DSP_PCFG_RRIE_MASK      (REG_DSP_PCFG_PRIE0_MASK | REG_DSP_PCFG_PRIE1_MASK | REG_DSP_PCFG_PRIE2_MASK)
#define REG_DSP_PCFG_RRIE_SHIFT     REG_DSP_PCFG_PRIE0_SHIFT
#define REG_DSP_PSTS_RCOMIM_SHIFT   REG_DSP_PSTS_RCOMIM0_SHIFT
#define REG_DSP_PSTS_RRI_SHIFT      REG_DSP_PSTS_RRI0_SHIFT


#define DSP_DPRINTF(...)    ((void) 0)
//#define DSP_DPRINTF OS_TPrintf

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct DSPData
{
    u16 send;
    u16 reserve1;
    u16 recv;
    u16 reserve2;
}
DSPData;


/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static volatile DSPData *const dspData = (DSPData*)REG_COM0_ADDR;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOn

  Description:  DSPの電源をオンにして、リセット状態にします。
                DSPを起動させるには DSP_ResetOff() を呼び出す必要があります。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_PowerOnCore(void)  // DSP_Init
{
    SCFG_ResetDSP();                                // DSPブロックのリセット確認
    SCFG_SupplyClockToDSP(TRUE);                    // DSPブロックの電源On
    OS_SpinWaitSysCycles(2);                        // wait 8 cycle @ 134MHz
    SCFG_ReleaseResetDSP();                         // DSPブロックのリセット解除
    DSP_ResetOnCore();                                  // DSPコアのリセット設定
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOff

  Description:  DSPの電源をオフにします。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_PowerOffCore(void) // DSP_End
{
    SCFG_ResetDSP();                                // DSPブロックのリセット設定
    SCFG_SupplyClockToDSP(FALSE);                   // DSPブロックの電源Off
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOn

  Description:  DSPをリセットします。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ResetOnCore(void)
{
    if ((reg_DSP_PCFG & REG_DSP_PCFG_DSPR_MASK) == 0)
    {
        reg_DSP_PCFG |= REG_DSP_PCFG_DSPR_MASK;
        while ( reg_DSP_PSTS & REG_DSP_PSTS_PRST_MASK )
        {
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOff

  Description:  リセット状態からDSPを起動します。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ResetOffCore(void)
{
    while ( reg_DSP_PSTS & REG_DSP_PSTS_PRST_MASK )
    {
    }
    DSP_ResetInterfaceCore();   // DSP-A9IFの初期化
    reg_DSP_PCFG &= ~REG_DSP_PCFG_DSPR_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOffEx

  Description:  リセット状態からDSPを起動します。

  Arguments:    bitmap : DSPからARM9への割り込みを許可するセマフォ、リプライレジスタのビットマップ。

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ResetOffExCore(u16 bitmap)
{
    SDK_ASSERT(bitmap >= 0 && bitmap <= 7);
    
    while ( reg_DSP_PSTS & REG_DSP_PSTS_PRST_MASK )
    {
    }
    DSP_ResetInterfaceCore();   // DSP-A9IFの初期化
    reg_DSP_PCFG |= (bitmap) << REG_DSP_PCFG_RRIE_SHIFT;
    reg_DSP_PCFG &= ~REG_DSP_PCFG_DSPR_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetInterface

  Description:  レジスタの初期化を行います。
                DSPをリセットした場合に呼び出す必要があります。

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ResetInterfaceCore(void)
{
    if (reg_DSP_PCFG & REG_DSP_PCFG_DSPR_MASK)
    {
        u16 dummy;
        reg_DSP_PCFG &= ~REG_DSP_PCFG_RRIE_MASK;
        reg_DSP_PSEM = 0;
        reg_DSP_PCLEAR = 0xFFFF;
        dummy = dspData[0].recv;
        dummy = dspData[1].recv;
        dummy = dspData[2].recv;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableRecvDataInterrupt

  Description:  指定したリプライレジスタの割り込みを有効にします。

  Arguments:    dataNo : リプライレジスタ番号 (0-2)

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_EnableRecvDataInterruptCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    reg_DSP_PCFG |= (1 << dataNo) << REG_DSP_PCFG_RRIE_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableRecvDataInterrupt

  Description:  指定したリプライレジスタの割り込みを無効にします。

  Arguments:    dataNo : リプライレジスタの番号 (0-2)

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_DisableRecvDataInterruptCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    reg_DSP_PCFG &= ~((1 << dataNo) << REG_DSP_PCFG_RRIE_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendDataIsEmpty

  Description:  指定したコマンドレジスタのデータをDSPが受信済みかチェックします。

  Arguments:    dataNo : コマンドレジスタの番号 (0-2)

  Returns:      DSPが既にデータを受信していれば TRUE
 *---------------------------------------------------------------------------*/
BOOL DSP_SendDataIsEmptyCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    return (reg_DSP_PSTS & ((1 << dataNo) << REG_DSP_PSTS_RCOMIM_SHIFT)) ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvDataIsReady

  Description:  指定したリプライレジスタにDSPからデータが送信されたかチェックします。

  Arguments:    dataNo : リプライレジスタの番号 (0-2)

  Returns:      DSPがデータを送信していれば TRUE
 *---------------------------------------------------------------------------*/
BOOL DSP_RecvDataIsReadyCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    return (reg_DSP_PSTS & ((1 << dataNo) << REG_DSP_PSTS_RRI_SHIFT)) ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendData

  Description:  DSPにデータを送信します。

  Arguments:    dataNo : コマンドレジスタの番号 (0-2)
                data   : 送信するデータ

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_SendDataCore(u32 dataNo, u16 data)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    while (DSP_SendDataIsEmptyCore(dataNo) == FALSE)
    {
    }
    dspData[dataNo].send = data;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvData

  Description:  DSPからのデータを受信します。

  Arguments:    dataNo : リプライレジスタの番号 (0-2)

  Returns:      受信したデータ
 *---------------------------------------------------------------------------*/
u16 DSP_RecvDataCore(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    while (DSP_RecvDataIsReadyCore(dataNo) == FALSE)
    {
    }
    return dspData[dataNo].recv;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableFifoInterrupt

  Description:  FIFOの割り込みを有効にします。

  Arguments:    type : FIFOの割り込み要因

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_EnableFifoInterruptCore(DSPFifoIntr type)
{
    reg_DSP_PCFG |= type;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableFifoInterrupt

  Description:  FIFOの割り込みを無効にします。

  Arguments:    type : FIFOの割り込み要因

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_DisableFifoInterruptCore(DSPFifoIntr type)
{
    reg_DSP_PCFG &= ~type;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendFifoEx

  Description:  DSPのメモリ空間内にデータを書き込みます。

  Arguments:    memsel : データを書き込むメモリ空間
                dest   : 書き込み先アドレス (ハーフワード).
                         上位 16 ビットを設定する場合は、事前に DMA レジスタに
                         値を設定する必要があります。
                src    : 書き込むデータ
                size   : 書き込むデータの長さ (ハーフワード)
                flags  : 読み出しモードを DSPFifoFlag の
                         DSP_FIFO_FLAG_RECV_UNIT_* 以外から選択します。

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_SendFifoExCore(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size, u16 flags)
{
    OSIntrMode  bak = OS_DisableInterrupts();
    u16 incmode = (u16)((flags & DSP_FIFO_FLAG_DEST_FIX) ? 0 : REG_DSP_PCFG_AIM_MASK);

    reg_DSP_PADR = dest;
    reg_DSP_PCFG = (u16)((reg_DSP_PCFG & ~(REG_DSP_PCFG_MEMSEL_MASK|REG_DSP_PCFG_AIM_MASK))
                        | memsel | incmode);

    if (flags & DSP_FIFO_FLAG_SRC_FIX)
    {
        while (size-- > 0)
        {
            while (reg_DSP_PSTS & REG_DSP_PSTS_WFFI_MASK)
            {
            }
            reg_DSP_PDATA = *src;
        }
    }
    else
    {
        while (size-- > 0)
        {
            while (reg_DSP_PSTS & REG_DSP_PSTS_WFFI_MASK)
            {
            }
            reg_DSP_PDATA = *src++;
        }
    }
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvFifoEx

  Description:  DSPのメモリ空間内からデータを読み込みます。

  Arguments:    memsel : データを読み込むメモリ空間 (プログラムメモリ以外)
                dest   : 受信先アドレス
                src    : 受信元アドレス (ハーフワード)
                         上位 16 ビットを設定する場合は、事前に DMA レジスタに
                         値を設定する必要があります。
                size   : 読み込むデータのサイズ (ハーフワード)
                flags  : 書き込みモードを DSPFifoFlag から選択します。

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_RecvFifoExCore(DSPFifoMemSel memsel, u16* dest, u16 src, int size, u16 flags)
{
    OSIntrMode  bak = OS_DisableInterrupts();
    DSPFifoRecvLength len;
    u16 incmode = (u16)((flags & DSP_FIFO_FLAG_SRC_FIX) ? 0 : REG_DSP_PCFG_AIM_MASK);

    SDK_ASSERT(memsel != DSP_FIFO_MEMSEL_PROGRAM);

    switch (flags & DSP_FIFO_FLAG_RECV_MASK)
    {
    case DSP_FIFO_FLAG_RECV_UNIT_2B:
        len = DSP_FIFO_RECV_2B;
        size = 1;
        break;
    case DSP_FIFO_FLAG_RECV_UNIT_16B:
        len = DSP_FIFO_RECV_16B;
        size = 8;
        break;
    case DSP_FIFO_FLAG_RECV_UNIT_32B:
        len = DSP_FIFO_RECV_32B;
        size = 16;
        break;
    default:
        len = DSP_FIFO_RECV_CONTINUOUS;
        break;
    }

    reg_DSP_PADR = src;
    reg_DSP_PCFG = (u16)((reg_DSP_PCFG & ~(REG_DSP_PCFG_MEMSEL_MASK|REG_DSP_PCFG_DRS_MASK|REG_DSP_PCFG_AIM_MASK))
                        | memsel | len | incmode | REG_DSP_PCFG_RS_MASK);

    if (flags & DSP_FIFO_FLAG_DEST_FIX)
    {
        while (size-- > 0)
        {
            while ((reg_DSP_PSTS & REG_DSP_PSTS_RFNEI_MASK) == 0)
            {
            }
            *dest = reg_DSP_PDATA;
        }
    }
    else
    {
        while (size-- > 0)
        {
            while ((reg_DSP_PSTS & REG_DSP_PSTS_RFNEI_MASK) == 0)
            {
            }
            *dest++ = reg_DSP_PDATA;
        }
    }
    reg_DSP_PCFG &= ~REG_DSP_PCFG_RS_MASK;
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetCommandReg

  Description:  コマンドレジスタに値を書き込みます。

  Arguments:    regNo : コマンドレジスタ番号 (0-2)
                data  : データ

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_SetCommandRegCore(u32 regNo, u16 data)
{
    SDK_ASSERT(regNo >= 0 && regNo <= 2);
    dspData[regNo].send = data;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetReplyReg

  Description:  リプライレジスタから値を読み込みます。

  Arguments:    regNo : リプライレジスタ番号 (0-2)

  Returns:      読み込んだデータ
 *---------------------------------------------------------------------------*/
u16 DSP_GetReplyRegCore(u32 regNo)
{
    SDK_ASSERT(regNo >= 0 && regNo <= 2);
    return dspData[regNo].recv;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetSemaphore

  Description:  ARMからDSPへの通知用セマフォレジスタに値を書き込みます。

  Arguments:    mask : セットする値

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_SetSemaphoreCore(u16 mask)
{
    reg_DSP_PSEM = mask;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetSemaphore

  Description:  DSPからARMへの通知用セマフォレジスタの値を読み込みます。

  Arguments:    なし

  Returns:      DSPがセマフォに書き込んだ値
 *---------------------------------------------------------------------------*/
u16 DSP_GetSemaphoreCore(void)
{
    return reg_DSP_SEM;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ClearSemaphore

  Description:  DSPからARMへの通知用セマフォレジスタの値をクリアします。

  Arguments:    mask : クリアするビット

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_ClearSemaphoreCore(u16 mask)
{
    reg_DSP_PCLEAR = mask;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_MaskSemaphore

  Description:  割り込みを無効にするDSPからARMへの通知用セマフォレジスタのビットを設定します。

  Arguments:    mask : 割り込みを無効にするビット

  Returns:      なし
 *---------------------------------------------------------------------------*/
void DSP_MaskSemaphoreCore(u16 mask)
{
    reg_DSP_PMASK = mask;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_CheckSemaphoreRequest

  Description:  セマフォレジスタによる割り込み要求があるかチェックします。

  Arguments:    なし

  Returns:      要求がある場合は TRUE
 *---------------------------------------------------------------------------*/
BOOL DSP_CheckSemaphoreRequestCore(void)
{
    return (reg_DSP_PSTS & REG_DSP_PSTS_PSEMI_MASK) >> REG_DSP_PSTS_PSEMI_SHIFT;
}


#if defined(DSP_SUPPORT_OBSOLETE_LOADER)
/*---------------------------------------------------------------------------*
 * 以下は現在使用されていないと思われる廃止候補インタフェース。
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * 以下はストレートマッピング方式による古いインタフェース。
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSPi_MapProcessSlotAsStraight

  Description:  セグメント番号とWRAMスロット番号が一致するようスロットマップを初期化。
                (この方式はExが付かない初期のロード関数でのみ使用される)

  Arguments:    context : DSPProcessContext構造体。
                slotB   : コードメモリのために使用を許可されたWRAM-B。
                slotC   : データメモリのために使用を許可されたWRAM-C。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL DSPi_MapProcessSlotAsStraight(DSPProcessContext *context, int slotB, int slotC)
{
    int     segment;
    for (segment = 0; segment < MI_WRAM_B_MAX_NUM; ++segment)
    {
        if (context->segmentCode & (1 << segment) != 0)
        {
            int     slot = segment;
            if ((slotB & (1 << slot)) == 0)
            {
                return FALSE;
            }
            context->slotOfSegmentCode[segment] = slot;
        }
    }
    for (segment = 0; segment < MI_WRAM_C_MAX_NUM; ++segment)
    {
        if (context->segmentData & (1 << segment) != 0)
        {
            int     slot = segment;
            if ((slotC & (1 << slot)) == 0)
            {
                return FALSE;
            }
            context->slotOfSegmentData[segment] = slot;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadFileAuto

  Description:  COFF形式のDSPプログラムをロードして必要なWRAMをDSPに割り当てる

  Arguments:    image : COFFファイル

  Returns:      成功ならば TRUE
 *---------------------------------------------------------------------------*/
BOOL DSP_LoadFileAutoCore(const void *image)
{
    // 一時的にメモリファイルへ変換。
    FSFile  memfile[1];
    if(DSPi_CreateMemoryFile(memfile, image))
    {
        DSPProcessContext   context[1];
        DSP_InitProcessContext(context, NULL);
        return DSP_StartupProcess(context, memfile, 0xFF, 0xFF, DSPi_MapProcessSlotAsStraight);
    }
    return FALSE;
}


#endif

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
