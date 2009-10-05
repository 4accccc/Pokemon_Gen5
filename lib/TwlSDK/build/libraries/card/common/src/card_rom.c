/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_rom.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-14#$
  $Rev: 10904 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro/card/rom.h>
#include <nitro/card/pullOut.h>
#include <nitro/card/rom.h>


#include "../include/card_common.h"
#include "../include/card_rom.h"


#if defined(SDK_ARM9) && defined(SDK_TWL)
#define CARD_USING_HASHCHECK
#endif // defined(SDK_ARM9) && defined(SDK_TWL)

#if defined(SDK_ARM9) || (defined(SDK_ARM7) && defined(SDK_ARM7_READROM_SUPPORT))
#define CARD_USING_ROMREADER
#endif // defined(SDK_ARM9) || (defined(SDK_ARM7) && defined(SDK_ARM7_READROM_SUPPORT))


/*---------------------------------------------------------------------------*/
/* constants */

#define CARD_COMMAND_PAGE           0x01000000
#define CARD_COMMAND_ID             0x07000000
#define CARD_COMMAND_REFRESH        0x00000000
#define CARD_COMMAND_STAT           CARD_COMMAND_ID
#define CARD_COMMAND_MASK           0x07000000
#define CARD_RESET_HI               0x20000000
#define CARD_COMMAND_OP_G_READID    0xB8
#define CARD_COMMAND_OP_G_READPAGE  0xB7
#define CARD_COMMAND_OP_G_READSTAT  0xD6
#define CARD_COMMAND_OP_G_REFRESH   0xB5
#ifdef SDK_TWL
#define CARD_COMMAND_OP_N_READID    0x90
#define CARD_COMMAND_OP_N_READPAGE  0x00
#define CARD_COMMAND_OP_N_READSTAT  CARD_COMMAND_OP_G_READSTAT
#define CARD_COMMAND_OP_N_REFRESH   CARD_COMMAND_OP_G_REFRESH
#endif

// ROM ID

#define CARD_ROMID_1TROM_MASK       0x80000000UL  // 1T-ROM type
#define CARD_ROMID_TWLROM_MASK      0x40000000UL  // TWL-ROM
#define CARD_ROMID_REFRESH_MASK     0x20000000UL  // リフレッシュサポート

// ROM STATUS

#define CARD_ROMST_RFS_WARN_L1_MASK 0x00000004UL
#define CARD_ROMST_RFS_WARN_L2_MASK 0x00000008UL
#define CARD_ROMST_RFS_READY_MASK   0x00000020UL


/*---------------------------------------------------------------------------*/
/* variables */

// リード時のベースオフセット。(通常は 0)
u32         cardi_rom_base;

// その他の内部情報。
static int                (*CARDiReadRomFunction) (void *userdata, void *buffer, u32 offset, u32 length);

static CARDTransferInfo     CARDiDmaReadInfo[1];
static CARDTransferInfo    *CARDiDmaReadRegisteredInfo;

static u32                  cache_page = 1;
static u8                   CARDi_cache_buf[CARD_ROM_PAGE_SIZE] ATTRIBUTE_ALIGN(32);
static BOOL                 CARDiEnableCacheInvalidationIC = FALSE;
static BOOL                 CARDiEnableCacheInvalidationDC = TRUE;

extern BOOL OSi_IsThreadInitialized;

static u8                   CARDiOwnSignature[CARD_ROM_DOWNLOAD_SIGNATURE_SIZE] ATTRIBUTE_ALIGN(4);


/*---------------------------------------------------------------------------*/
/* functions */

void CARD_RefreshRom(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetRomOp

  Description:  カードコマンド設定

  Arguments:    command    コマンド
                offset     転送ページ数

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_SetRomOp(u32 command, u32 offset)
{
    u32     cmd1 = (u32)((offset >> 8) | (command << 24));
    u32     cmd2 = (u32)((offset << 24));
    // 念のため前回のROMコマンドの完了待ち。
    while ((reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK) != 0)
    {
    }
    // マスターイネーブル。
    reg_MI_MCCNT0 = (u16)(REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK |
                          (reg_MI_MCCNT0 & ~REG_MI_MCCNT0_SEL_MASK));
    // コマンド設定。
    reg_MI_MCCMD0 = MI_HToBE32(cmd1);
    reg_MI_MCCMD1 = MI_HToBE32(cmd2);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetRomFlag

  Description:  カードコマンドコントロールパラメータを取得

  Arguments:    flag       カードデバイスへ発行するコマンドのタイプ
                           (CARD_COMMAND_PAGE / CARD_COMMAND_ID /
                            CARD_COMMAND_STAT / CARD_COMMAND_REFRESH)

  Returns:      カードコマンドコントロールパラメータ
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 CARDi_GetRomFlag(u32 flag)
{
    u32     rom_ctrl = *(vu32 *)(HW_CARD_ROM_HEADER + 0x60);
    return (u32)(flag | REG_MI_MCCNT1_START_MASK | CARD_RESET_HI | (rom_ctrl & ~CARD_COMMAND_MASK));
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_IsTwlRom

  Description:  ゲームカードがTWL対応ROMを搭載しているかどうか判定。
                NitroアプリケーションでもTWL対応ROM搭載時はTRUEを返します。

  Arguments:    None.

  Returns:      TWL対応ROMならTRUE。
 *---------------------------------------------------------------------------*/
BOOL CARDi_IsTwlRom(void)
{
    u32 iplCardID = *(u32 *)(HW_BOOT_CHECK_INFO_BUF);

    // 起動時からカードが無ければ常にFALSE
    if ( ! iplCardID )
    {
        return FALSE;
    }

    return (CARDi_ReadRomID() & CARD_ROMID_TWLROM_MASK) ? TRUE : FALSE;
}

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         CARDi_IsNormalMode

  Description:  ゲームカードがNORMALモードかどうか判定。

  Arguments:    None.

  Returns:      ゲームカードがNORMALモードならTRUE。
 *---------------------------------------------------------------------------*/
static BOOL CARDi_IsNormalMode(void)
{
    const CARDRomHeaderTWL *oh = CARD_GetOwnRomHeaderTWL();

    return OS_IsRunOnTwl() &&
           (OS_GetBootType() != OS_BOOTTYPE_ROM) &&
           oh->access_control.game_card_on &&
           ! oh->access_control.game_card_nitro_mode;
}

#endif

/*---------------------------------------------------------------------------*
  Name:         CARDi_StartRomPageTransfer

  Description:  ROMページ転送を開始。

  Arguments:    offset     転送元のROMオフセット

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_StartRomPageTransfer(u32 offset)
{
    u8 op = CARD_COMMAND_OP_G_READPAGE;
#ifdef SDK_TWL
    if ( CARDi_IsNormalMode() )
    {
        op = CARD_COMMAND_OP_N_READPAGE;
    }
#endif

    CARDi_SetRomOp(op, offset);
    reg_MI_MCCNT1 = CARDi_GetRomFlag(CARD_COMMAND_PAGE);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomIDCore

  Description:  カード ID の読み出し。

  Arguments:    None.

  Returns:      カード ID
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomIDCore(void)
{
    u8 op = CARD_COMMAND_OP_G_READID;
#ifdef SDK_TWL
    if ( CARDi_IsNormalMode() )
    {
        op = CARD_COMMAND_OP_N_READID;
    }
#endif

    CARDi_SetRomOp(op, 0);
    reg_MI_MCCNT1 = (u32)(CARDi_GetRomFlag(CARD_COMMAND_ID) & ~REG_MI_MCCNT1_L1_MASK);
    while ((reg_MI_MCCNT1 & REG_MI_MCCNT1_RDY_MASK) == 0)
    {
    }
    return reg_MI_MCD1;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomStatusCore

  Description:  カードステータスの読み出し。
                リフレッシュ対応ROM検出時のみ発行。
                対応ROMを搭載したNITROアプリケーションでも必要になる。

  Arguments:    None.

  Returns:      カードステータス
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomStatusCore(void)
{
    u32 iplCardID = *(u32 *)(HW_BOOT_CHECK_INFO_BUF);

    if ( ! (iplCardID & CARD_ROMID_REFRESH_MASK) )
    {
        return CARD_ROMST_RFS_READY_MASK;
    }

    CARDi_SetRomOp(CARD_COMMAND_OP_G_READSTAT, 0);
    reg_MI_MCCNT1 = (u32)(CARDi_GetRomFlag(CARD_COMMAND_STAT) & ~REG_MI_MCCNT1_L1_MASK);
    while ((reg_MI_MCCNT1 & REG_MI_MCCNT1_RDY_MASK) == 0)
    {
    }
    return reg_MI_MCD1;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_RefreshRom

  Description:  カードROMバッドブロックのリフレッシュ。
                リフレッシュ対応ROM検出時のみ発行。
                該当ROMを搭載したNITROアプリケーションでも必要になる。
                CARD_ReadRom内でも極めて稀にECC補正不能間近の状態になると、
                強制的にリフレッシュされますが、リフレッシュ処理は50msec程度
                かかる可能性がありますので、アプリケーションにとって
                遅延が入っても安全なタイミングで定期的に呼び出して下さい。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_RefreshRom(void)
{
    SDK_ASSERT(CARD_IsAvailable());
    SDK_TASSERTMSG(CARDi_GetTargetMode() == CARD_TARGET_ROM, "must be locked by CARD_LockRom()");

#if defined(SDK_ARM9)
    (void)CARDi_WaitForTask(&cardi_common, TRUE, NULL, NULL);
    // ここでカード抜け検出を行う
    CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
#endif // defined(SDK_ARM9)

    CARDi_RefreshRom(CARD_ROMST_RFS_WARN_L1_MASK | CARD_ROMST_RFS_WARN_L2_MASK);

#if defined(SDK_ARM9)
    cardi_common.cmd->result = CARD_RESULT_SUCCESS;
    CARDi_EndTask(&cardi_common);
#endif // defined(SDK_ARM9)
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RefreshRom

  Description:  カードROMバッドブロックのリフレッシュ。
                リフレッシュ対応ROM検出時のみ発行。
                該当ROMを搭載したNITROアプリケーションでも必要になる。

  Arguments:    warn_mask         ワーニングレベル指定

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_RefreshRom(u32 warn_mask)
{
    if (CARDi_ReadRomStatusCore() & warn_mask)
    {
        CARDi_RefreshRomCore();
        // リフレッシュ完了まで100msec以上かかる可能性がある
        while ( !(CARDi_ReadRomStatusCore() & CARD_ROMST_RFS_READY_MASK) )
        {
            // 可能であればスリープ
            if ( OSi_IsThreadInitialized && 
                 OS_IsAlarmAvailable() )
            {
                OS_Sleep(1);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_RefreshRomCore

  Description:  カードROMバッドブロックのリフレッシュ。
                該当ROMを搭載したNITROアプリケーションでも必要になる。
                カードへコマンドを一方的に発行するだけなのでレイテンシ設定は無効。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_RefreshRomCore(void)
{
    CARDi_SetRomOp(CARD_COMMAND_OP_G_REFRESH, 0);
    reg_MI_MCCNT1 = (u32)(CARDi_GetRomFlag(CARD_COMMAND_REFRESH) & ~REG_MI_MCCNT1_L1_MASK);
    while (reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK)
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomWithCPU

  Description:  CPUを使用してROM転送。
                キャッシュやページ単位の制限を考慮する必要は無いが
                転送完了まで関数がブロッキングする点に注意。

  Arguments:    userdata          (他のコールバックとして使用するためのダミー)
                buffer            転送先バッファ
                offset            転送元ROMオフセット
                length            転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
int CARDi_ReadRomWithCPU(void *userdata, void *buffer, u32 offset, u32 length)
{
    int     retval = (int)length;
    // 頻繁に使用するグローバル変数をローカル変数へキャッシュ。
    u32         cachedPage = cache_page;
    u8  * const cacheBuffer = CARDi_cache_buf;
    while (length > 0)
    {
        // ROM転送は常にページ単位。
        u8     *ptr = (u8 *)buffer;
        u32     n = CARD_ROM_PAGE_SIZE;
        u32     pos = MATH_ROUNDDOWN(offset, CARD_ROM_PAGE_SIZE);
        // 以前のページと同じならばキャッシュを使用。
        if (pos == cachedPage)
        {
            ptr = cacheBuffer;
        }
        else
        {
            // バッファへ直接転送できないならキャッシュへ転送。
            if(((pos != offset) || (((u32)buffer & 3) != 0) || (length < n)))
            {
                cachedPage = pos;
                ptr = cacheBuffer;
            }
            // 4バイト整合の保証されたバッファへCPUで直接リード。
            CARDi_StartRomPageTransfer(pos);
            {
                u32     word = 0;
                for (;;)
                {
                    // 1ワード転送完了を待つ。
                    u32     ctrl = reg_MI_MCCNT1;
                    if ((ctrl & REG_MI_MCCNT1_RDY_MASK) != 0)
                    {
                        // データを読み出し、必要ならバッファへ格納。
                        u32     data = reg_MI_MCD1;
                        if (word < (CARD_ROM_PAGE_SIZE / sizeof(u32)))
                        {
                            ((u32 *)ptr)[word++] = data;
                        }
                    }
                    // 1ページ転送完了なら終了。
                    if ((ctrl & REG_MI_MCCNT1_START_MASK) == 0)
                    {
                        break;
                    }
                }
            }
        }
        // キャッシュ経由ならキャッシュから転送。
        if (ptr == cacheBuffer)
        {
            u32     mod = offset - pos;
            n = MATH_MIN(length, CARD_ROM_PAGE_SIZE - mod);
            MI_CpuCopy8(cacheBuffer + mod, buffer, n);
        }
        buffer = (u8 *)buffer + n;
        offset += n;
        length -= n;
    }
    // アクセス中に発生したカード抜けを判定。
    CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
    // ローカル変数からグローバル変数へ反映。
    cache_page = cachedPage;
    (void)userdata;
    return retval;
}

#if defined(CARD_USING_ROMREADER)
/*---------------------------------------------------------------------------*
  Name:         CARDi_DmaReadPageCallback

  Description:  ROMページDMA転送完了コールバック。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_DmaReadPageCallback(void)
{
    CARDTransferInfo   *info = CARDiDmaReadRegisteredInfo;
    if (info)
    {
        info->src += CARD_ROM_PAGE_SIZE;
        info->dst += CARD_ROM_PAGE_SIZE;
        info->len -= CARD_ROM_PAGE_SIZE;
        // 必要なら次のページを転送。
        if (info->len > 0)
        {
            CARDi_StartRomPageTransfer(info->src);
        }
        // 転送完了。
        else
        {
            cardi_common.DmaCall->Stop(cardi_common.dma);
            (void)OS_DisableIrqMask(OS_IE_CARD_DATA);
            (void)OS_ResetRequestIrqMask(OS_IE_CARD_DATA);
            CARDiDmaReadRegisteredInfo = NULL;
            // アクセス中に発生したカード抜けを判定。
            CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
            if (info->callback)
            {
                (*info->callback)(info->userdata);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomWithDMA

  Description:  DMAを使用してROM転送。
                最初のページの転送を開始したら関数はただちに制御を返す。

  Arguments:    info              転送情報

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ReadRomWithDMA(CARDTransferInfo *info)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    CARDiDmaReadRegisteredInfo = info;
    // カード転送完了割り込みを設定。
    (void)OS_SetIrqFunction(OS_IE_CARD_DATA, CARDi_DmaReadPageCallback);
    (void)OS_ResetRequestIrqMask(OS_IE_CARD_DATA);
    (void)OS_EnableIrqMask(OS_IE_CARD_DATA);
    (void)OS_RestoreInterrupts(bak_psr);
    // CARD-DMAの設定。 (転送待機)
    cardi_common.DmaCall->Recv(cardi_common.dma, (void *)&reg_MI_MCD1, (void *)info->dst, CARD_ROM_PAGE_SIZE);
    // 最初のROM転送開始。
    CARDi_StartRomPageTransfer(info->src);
}

static void CARDi_DmaReadDone(void *userdata)
{
    (void)userdata;
#ifdef SDK_ARM9
    // アクセス中に発生したカード抜けを判定。
    CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
#endif
    // ROM-ECC補正の最終警告段階でリフレッシュ
    CARDi_RefreshRom(CARD_ROMST_RFS_WARN_L2_MASK);

    cardi_common.cmd->result = CARD_RESULT_SUCCESS;
    CARDi_EndTask(&cardi_common);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_IsRomDmaAvailable

  Description:  ROM転送に非同期DMAが使用可能か判定。

  Arguments:    dma        DMAチャンネル
                dst        転送先バッファ
                src        転送元ROMオフセット
                len        転送サイズ

  Returns:      ROM転送にDMAが使用可能ならTRUE。
 *---------------------------------------------------------------------------*/
static BOOL CARDi_IsRomDmaAvailable(u32 dma, void *dst, u32 src, u32 len)
{
    return (dma <= MI_DMA_MAX_NUM) && (len > 0) && (((u32)dst & 31) == 0) &&
#ifdef SDK_ARM9
        (((u32)dst + len <= OS_GetITCMAddress()) || ((u32)dst >= OS_GetITCMAddress() + HW_ITCM_SIZE)) &&
        (((u32)dst + len <= OS_GetDTCMAddress()) || ((u32)dst >= OS_GetDTCMAddress() + HW_DTCM_SIZE)) &&
#endif
        (((src | len) & (CARD_ROM_PAGE_SIZE - 1)) == 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomSyncCore

  Description:  同期カード読み込みの共通処理

  Arguments:    c          CARDiCommon 構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_ReadRomSyncCore(CARDiCommon *c)
{
    // 実行環境に応じたROMアクセスルーチンを実行。
    (void)(*CARDiReadRomFunction)(NULL, (void*)c->dst, c->src, c->len);
#ifdef SDK_ARM9
    // アクセス中に発生したカード抜けを判定。
    CARDi_CheckPulledOutCore(CARDi_ReadRomIDCore());
#endif
    // ROM-ECC補正の最終警告段階でリフレッシュ
    CARDi_RefreshRom(CARD_ROMST_RFS_WARN_L2_MASK);

    cardi_common.cmd->result = CARD_RESULT_SUCCESS;
}
#endif // defined(CARD_USING_ROMREADER)

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRom

  Description:  ROM リードの基本関数

  Arguments:    dma        使用する DMA チャンネル
                src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期モードを指定するなら TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ReadRom(u32 dma,
                   const void *src, void *dst, u32 len,
                   MIDmaCallback callback, void *arg, BOOL is_async)
{
#if defined(CARD_USING_ROMREADER)
    CARDiCommon *const c = &cardi_common;

    SDK_ASSERT(CARD_IsAvailable());
    SDK_ASSERT(CARDi_GetTargetMode() == CARD_TARGET_ROM);
    SDK_TASSERTMSG((dma != 0), "cannot specify DMA channel 0");

    // CARDアクセスに対する正当性の判定。
    CARD_CheckEnabled();
    if ((CARDi_GetAccessLevel() & CARD_ACCESS_LEVEL_ROM) == 0)
    {
        OS_TPanic("this program cannot access CARD-ROM!");
    }

    // ARM9 側の排他待ち。
    (void)CARDi_WaitForTask(c, TRUE, callback, arg);

    // DMAインタフェースの取得。
    c->DmaCall = CARDi_GetDmaInterface(dma);
    c->dma = (u32)((c->DmaCall != NULL) ? (dma & MI_DMA_CHANNEL_MASK) : MI_DMA_NOT_USE);
    if (c->dma <= MI_DMA_MAX_NUM)
    {
        c->DmaCall->Stop(c->dma);
    }

    // 今回の転送パラメータを設定。
    c->src = (u32)((u32)src + cardi_rom_base);
    c->dst = (u32)dst;
    c->len = (u32)len;

    // 転送パラメータを設定。
    {
        CARDTransferInfo   *info = CARDiDmaReadInfo;
        info->callback = CARDi_DmaReadDone;
        info->userdata = NULL;
        info->src = c->src;
        info->dst = c->dst;
        info->len = c->len;
        info->work = 0;
    }

    // ハッシュチェックが無効な環境なら場合によってDMA転送が利用可能。
    if ((CARDiReadRomFunction == CARDi_ReadRomWithCPU) &&
        CARDi_IsRomDmaAvailable(c->dma, (void *)c->dst, c->src, c->len))
    {
#if defined(SDK_ARM9)
        // 転送範囲のサイズに応じてキャッシュ無効化の方法を切り替え。
        OSIntrMode bak_psr = OS_DisableInterrupts();
        if (CARDiEnableCacheInvalidationIC)
        {
            CARDi_ICInvalidateSmart((void *)c->dst, c->len, c->flush_threshold_ic);
        }
        if (CARDiEnableCacheInvalidationDC)
        {
            CARDi_DCInvalidateSmart((void *)c->dst, c->len, c->flush_threshold_dc);
        }
        (void)OS_RestoreInterrupts(bak_psr);
#endif
        // DMAによる非同期転送開始。
        CARDi_ReadRomWithDMA(CARDiDmaReadInfo);
        // 同期転送を要求されているならここで完了待ち。
        if (!is_async)
        {
            CARD_WaitRomAsync();
        }
    }
    else
    {
        // CPU転送の場合でも命令キャッシュは不整合になるので無効化が必要。
        if (CARDiEnableCacheInvalidationIC)
        {
            CARDi_ICInvalidateSmart((void *)c->dst, c->len, c->flush_threshold_ic);
        }
        // CPUによる何らかの直接処理が必要ならスレッドへタスク登録。
        (void)CARDi_ExecuteOldTypeTask(CARDi_ReadRomSyncCore, is_async);
    }
#else
    (void)dma;
    (void)is_async;
    (void)CARDi_ReadRomWithCPU(NULL, dst, (u32)src, len);
    if (callback)
    {
        (*callback)(arg);
    }
#endif // defined(CARD_USING_ROMREADER)
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomID

  Description:  カード ID の読み出し

  Arguments:    None.

  Returns:      カード ID.
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomID(void)
{
    u32     ret = 0;

    SDK_ASSERT(CARD_IsAvailable());
    SDK_TASSERTMSG(CARDi_GetTargetMode() == CARD_TARGET_ROM, "must be locked by CARD_LockRom()");

#if defined(SDK_ARM9)
    (void)CARDi_WaitForTask(&cardi_common, TRUE, NULL, NULL);
#endif // defined(SDK_ARM9)

    /* 直接アクセス可能なのでここで実行 */
    ret = CARDi_ReadRomIDCore();
#ifdef SDK_ARM9
    // ここでカード抜け検出を行う
    CARDi_CheckPulledOutCore(ret);
#endif

#if defined(SDK_ARM9)
    cardi_common.cmd->result = CARD_RESULT_SUCCESS;
    CARDi_EndTask(&cardi_common);
#endif // defined(SDK_ARM9)

    return ret;
}

#if defined(CARD_USING_HASHCHECK)
#include <twl/ltdmain_begin.h>

// ROMハッシュ計算用バッファ。
u8     *CARDiHashBufferAddress = NULL;
u32     CARDiHashBufferLength = 0;
static CARDRomHashContext   context[1];

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadCardWithHash

  Description:  ハッシュを確認しつつROM転送。

  Arguments:    buffer            転送先バッファ
                offset            転送元ROMオフセット
                length            転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static int CARDi_ReadCardWithHash(void *userdata, void *buffer, u32 offset, u32 length)
{
    (void)userdata;
    CARD_ReadRomHashImage(context, buffer, offset, length);
    return (int)length;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadCardWithHashInternalAsync

  Description:  ハッシュ検証版ROM転送の内部で部分的に使用する非同期DMA転送。

  Arguments:    userdata          ユーザ定義の引数
                buffer            転送先バッファ
                offset            転送元ROMオフセット
                length            転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static int CARDi_ReadCardWithHashInternalAsync(void *userdata, void *buffer, u32 offset, u32 length)
{
    if (cardi_common.dma == MI_DMA_NOT_USE)
    {
        length = 0;
    }
    else
    {
        CARDRomHashContext *context = (CARDRomHashContext *)userdata;
        static CARDTransferInfo card_hash_info[1];
        DC_FlushRange(buffer, length);
        card_hash_info->callback = (void(*)(void*))CARD_NotifyRomHashReadAsync;
        card_hash_info->userdata = context;
        card_hash_info->src = offset;
        card_hash_info->dst = (u32)buffer;
        card_hash_info->len = length;
        card_hash_info->command = 0;
        card_hash_info->work = 0;
        CARDi_ReadRomWithDMA(card_hash_info);
    }
    return (int)length;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitRomHash

  Description:  可能ならハッシュ検証版ROM転送に切り替える。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_InitRomHash(void)
{
    // FS_Init関数などの呼び出しで必要とされるより前に
    // アプリケーションがアリーナを全て消費してしまうことが多いため、
    // ハッシュコンテキスト用バッファは常にここで確保しておく。
    u8     *lo = (u8 *)MATH_ROUNDUP((u32)OS_GetMainArenaLo(), 32);
    u8     *hi = (u8 *)MATH_ROUNDDOWN((u32)OS_GetMainArenaHi(), 32);
    u32     len = CARD_CalcRomHashBufferLength(CARD_GetOwnRomHeaderTWL());
    if (&lo[len] > hi)
    {
        OS_TPanic("cannot allocate memory for ROM-hash from ARENA");
    }
    else
    {
        OS_SetMainArenaLo(&lo[len]);
        CARDiHashBufferAddress = lo;
        CARDiHashBufferLength = len;
        // ROMブートかつハッシュテーブルが存在する環境なら実際にロード。
        if ((OS_GetBootType() == OS_BOOTTYPE_ROM) &&
            ((((const u8 *)HW_TWL_ROM_HEADER_BUF)[0x1C] & 0x01) != 0))
        {
            cardi_common.dma = MI_DMA_NOT_USE;
            CARDiReadRomFunction = CARDi_ReadCardWithHash;
            {
                u16     id = (u16)OS_GetLockID();
                CARD_LockRom(id);
                CARD_InitRomHashContext(context,
                                        CARD_GetOwnRomHeaderTWL(),
                                        CARDiHashBufferAddress,
                                        CARDiHashBufferLength,
                                        CARDi_ReadRomWithCPU,
                                        CARDi_ReadCardWithHashInternalAsync,
                                        context);
                // 同じバッファを競合して使用されないようポインタを破棄。
                CARDiHashBufferAddress = NULL;
                CARDiHashBufferLength = 0;
                CARD_UnlockRom(id);
                OS_ReleaseLockID(id);
            }
        }
    }
}
#include <twl/ltdmain_end.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitRom

  Description:  ROMアクセス管理情報を初期化。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitRom(void)
{
#if defined(CARD_USING_ROMREADER)
    CARDiReadRomFunction = CARDi_ReadRomWithCPU;
    // TWL-SDKではROMデータアクセスにハッシュ検証を実施するが
    // DSダウンロードプレイの署名データはハッシュ計算後に埋め込まれるため、
    // 今後はCARD_ReadRom()で不用意に読み出すことが禁じられ
    // 明示的にCARDi_GetOwnSignature()で参照する必要がある。
    if ((OS_GetBootType() == OS_BOOTTYPE_ROM) && CARD_GetOwnRomHeader()->rom_size)
    {
        u16     id = (u16)OS_GetLockID();
        CARD_LockRom(id);
        (void)CARDi_ReadRomWithCPU(NULL, CARDiOwnSignature,
                                   CARD_GetOwnRomHeader()->rom_size,
                                   CARD_ROM_DOWNLOAD_SIGNATURE_SIZE);
        CARD_UnlockRom(id);
        OS_ReleaseLockID(id);
    }
#if defined(CARD_USING_HASHCHECK)
    // 利用可能ならハッシュ検証付きのルーチンを採用。
    if (OS_IsRunOnTwl())
    {
		CARDi_InitRomHash();
    }
#endif
#else
    CARDiReadRomFunction = NULL;
#endif
}


/*---------------------------------------------------------------------------*
  Name:         CARD_WaitRomAsync

  Description:  ROMアクセス関数が完了するまで待機

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_WaitRomAsync(void)
{
    (void)CARDi_WaitAsync();
}

/*---------------------------------------------------------------------------*
  Name:         CARD_TryWaitRomAsync

  Description:  ROMアクセス関数が完了したか判定

  Arguments:    None.

  Returns:      ROMアクセス関数が完了していればTRUE
 *---------------------------------------------------------------------------*/
BOOL CARD_TryWaitRomAsync(void)
{
    return CARDi_TryWaitAsync();
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetOwnSignature

  Description:  自分自身のDSダウンロードプレイ署名データを取得。

  Arguments:    None.

  Returns:      自分自身のDSダウンロードプレイ署名データ。
 *---------------------------------------------------------------------------*/
const u8* CARDi_GetOwnSignature(void)
{
    return CARDiOwnSignature;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetOwnSignature

  Description:  自分自身のDSダウンロードプレイ署名データを設定。
                非カードブート時に上位ライブラリから呼び出す。

  Arguments:    DSダウンロードプレイ署名データ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_SetOwnSignature(const void *signature)
{
    MI_CpuCopy8(signature, CARDiOwnSignature, CARD_ROM_DOWNLOAD_SIGNATURE_SIZE);
}

#if defined(SDK_ARM9)
/*---------------------------------------------------------------------------*
  Name:         CARD_GetCacheFlushFlag

  Description:  キャッシュ無効化を自動的に行うかどうかの設定フラグを取得。

  Arguments:    icache            命令キャッシュの自動無効化フラグを格納するポインタ
                                  NULLであれば無視される
                dcache            データキャッシュの自動無効化フラグを格納するポインタ
                                  NULLであれば無視される

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_GetCacheFlushFlag(BOOL *icache, BOOL *dcache)
{
    SDK_ASSERT(CARD_IsAvailable());
    if (icache)
    {
        *icache = CARDiEnableCacheInvalidationIC;
    }
    if (dcache)
    {
        *dcache = CARDiEnableCacheInvalidationDC;
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_SetCacheFlushFlag

  Description:  キャッシュ無効化を自動的に行うかどうかを設定。
                デフォルトでは命令キャッシュがFALSEでデータキャッシュがTRUE。

  Arguments:    icache            命令キャッシュの自動無効化を有効にするならTRUE
                dcache            データキャッシュの自動無効化が有効にするならTRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_SetCacheFlushFlag(BOOL icache, BOOL dcache)
{
    SDK_ASSERT(CARD_IsAvailable());
    CARDiEnableCacheInvalidationIC = icache;
    CARDiEnableCacheInvalidationDC = dcache;
}
#endif
