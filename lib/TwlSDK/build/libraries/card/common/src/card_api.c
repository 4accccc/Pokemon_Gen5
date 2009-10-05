/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_api.c

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


#include <nitro.h>

#include "../include/card_common.h"
#include "../include/card_event.h"
#include "../include/card_rom.h"


/*---------------------------------------------------------------------------*/
/* variables */

static BOOL CARDi_EnableFlag = FALSE;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_LockBusCondition

  Description:   イベントリスナー構造体のイベント完了を待機。

  Arguments:    userdata : ロックID

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL CARDi_LockBusCondition(void *userdata)
{
    u16     lockID = *(const u16 *)userdata;
    return (OS_TryLockCard(lockID) == OS_LOCK_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_Init

  Description:  DARDライブラリを初期化

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_Init(void)
{
    CARDiCommon *p = &cardi_common;

    if (!p->flag)
    {
        p->flag = CARD_STAT_INIT;

#if !defined(SDK_SMALL_BUILD) && defined(SDK_ARM9)
        // 非カードブート時に格納されるCARD-ROMヘッダ情報は
        // カードブート時に空データなのでこれを正しく退避。
        if (OS_GetBootType() == OS_BOOTTYPE_ROM)
        {
            MI_CpuCopy8((const void *)HW_ROM_HEADER_BUF, (void *)HW_CARD_ROM_HEADER,
                        HW_CARD_ROM_HEADER_SIZE);
        }
#endif // !defined(SDK_SMALL_BUILD) && defined(SDK_ARM9)

#if defined(SDK_ARM9)
        // ライブラリの内部変数を初期化。
        p->src = 0;
        p->dst = 0;
        p->len = 0;
        p->dma = MI_DMA_NOT_USE;
        p->DmaCall = NULL;
        p->flush_threshold_ic = 0x400;
        p->flush_threshold_dc = 0x2400;
#endif
        cardi_rom_base = 0;
        p->priority = CARD_THREAD_PRIORITY_DEFAULT;

        // リソースロックの初期化。
        CARDi_InitResourceLock();

        // タスクスレッドを起動。
#if defined(SDK_ARM9)
        // ARM9側は互換性のため旧形式の待ち受けスレッドを採用。
        p->callback = NULL;
        p->callback_arg = NULL;
        OS_InitThreadQueue(p->busy_q);
        OS_CreateThread(p->thread.context,
                        CARDi_OldTypeTaskThread, NULL,
                        p->thread.stack + sizeof(p->thread.stack),
                        sizeof(p->thread.stack), p->priority);
        OS_WakeupThreadDirect(p->thread.context);
#else // defined(SDK_ARM9)
        // ARM7側は新形式の安定した待ち受けスレッドを採用。
        CARDi_InitTaskQueue(p->task_q);
        OS_CreateThread(p->thread.context,
                        CARDi_TaskWorkerProcedure, p->task_q,
                        p->thread.stack + sizeof(p->thread.stack),
                        sizeof(p->thread.stack), p->priority);
        OS_WakeupThreadDirect(p->thread.context);
#endif

        // コマンド通信システムを初期化。
        CARDi_InitCommand();

        // ROMドライバを初期化。
        CARDi_InitRom();

        // カードブート時のみ無条件にカードアクセスを許可。
        // それ以外の起動モードでカードアクセスを希望する場合、
        // ガイドラインに記載された所定の条件に従った上で
        // 明示的にCARD_Enable関数を呼び出す必要がある。
        if (OS_GetBootType() == OS_BOOTTYPE_ROM)
        {
            CARD_Enable(TRUE);
        }

#if !defined(SDK_SMALL_BUILD)
        //---- for detect pulled out card
        CARD_InitPulledOutCallback();
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_IsAvailable

  Description:  CARDライブラリが利用可能か判定

  Arguments:    None.

  Returns:      CARD_Init関数をすでに呼び出されていればTRUE
 *---------------------------------------------------------------------------*/
BOOL CARD_IsAvailable(void)
{
    CARDiCommon *const p = &cardi_common;
    return (p->flag != 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_IsEnabled

  Description:  カードにアクセス可能か判定

  Arguments:    None.

  Returns:      カードにアクセスする権限があればTRUE
 *---------------------------------------------------------------------------*/
BOOL CARD_IsEnabled(void)
{
    return CARDi_EnableFlag;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_CheckEnabled

  Description:  カードにアクセスする権限があるか判定し、なければ強制停止

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_CheckEnabled(void)
{
    if (!CARD_IsEnabled())
    {
        OS_TPanic("NITRO-CARD permission denied");
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_Enable

  Description:  カードにアクセスする権限を設定

  Arguments:    enable      アクセスする権限があればTRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_Enable(BOOL enable)
{
    CARDi_EnableFlag = enable;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetThreadPriority

  Description:  ライブラリ内部の非同期処理用スレッドの優先度を取得

  Arguments:    None.

  Returns:      内部スレッドの優先度
 *---------------------------------------------------------------------------*/
u32 CARD_GetThreadPriority(void)
{
    CARDiCommon *const p = &cardi_common;
    SDK_ASSERT(CARD_IsAvailable());

    return p->priority;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_SetThreadPriority

  Description:  ライブラリ内部の非同期処理用スレッドの優先度を設定

  Arguments:    None.

  Returns:      設定前の内部スレッドの優先度
 *---------------------------------------------------------------------------*/
u32 CARD_SetThreadPriority(u32 prior)
{
    CARDiCommon *const p = &cardi_common;
    SDK_ASSERT(CARD_IsAvailable());

    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        u32     ret = p->priority;
        SDK_ASSERT((prior >= OS_THREAD_PRIORITY_MIN) && (prior <= OS_THREAD_PRIORITY_MAX));
        p->priority = prior;
        (void)OS_SetThreadPriority(p->thread.context, p->priority);
        (void)OS_RestoreInterrupts(bak_psr);
        return ret;
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetResultCode

  Description:  最後に呼び出したCARD関数の結果を取得

  Arguments:    None.

  Returns:      最後に呼び出したCARD関数の結果
 *---------------------------------------------------------------------------*/
CARDResult CARD_GetResultCode(void)
{
    CARDiCommon *const p = &cardi_common;
    SDK_ASSERT(CARD_IsAvailable());

    return p->cmd->result;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomHeader

  Description:  現在挿入されているカードのROMヘッダ情報を取得

  Arguments:    None.

  Returns:      CARDRomHeader構造体へのポインタ
 *---------------------------------------------------------------------------*/
const u8 *CARD_GetRomHeader(void)
{
    return (const u8 *)HW_CARD_ROM_HEADER;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetOwnRomHeader

  Description:  現在実行しているプログラムのROMヘッダ情報を取得。

  Arguments:    None.

  Returns:      CARDRomHeader構造体へのポインタ
 *---------------------------------------------------------------------------*/
const CARDRomHeader *CARD_GetOwnRomHeader(void)
{
    return (const CARDRomHeader *)HW_ROM_HEADER_BUF;
}

#if defined(SDK_TWL)

/*---------------------------------------------------------------------------*
  Name:         CARD_GetOwnRomHeaderTWL

  Description:  現在実行しているプログラムのTWL-ROMヘッダ情報を取得。

  Arguments:    None.

  Returns:      CARDRomHeaderTWL構造体へのポインタ
 *---------------------------------------------------------------------------*/
const CARDRomHeaderTWL *CARD_GetOwnRomHeaderTWL(void)
{
    return (const CARDRomHeaderTWL *)HW_TWL_ROM_HEADER_BUF;
}

#endif // SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCacheFlushThreshold

  Description:  キャッシュ無効化を部分的に行うか全体へ行うかの閾値を取得

  Arguments:    icache            命令キャッシュの無効化閾値を格納するポインタ
                                  NULLであれば無視される
                dcache            データキャッシュの無効化閾値を格納するポインタ
                                  NULLであれば無視される

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_GetCacheFlushThreshold(u32 *icache, u32 *dcache)
{
#if defined(SDK_ARM9)
    SDK_ASSERT(CARD_IsAvailable());
    if (icache)
    {
        *icache = cardi_common.flush_threshold_ic;
    }
    if (dcache)
    {
        *dcache = cardi_common.flush_threshold_dc;
    }
#else
    (void)icache;
    (void)dcache;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CARD_SetCacheFlushThreshold

  Description:  キャッシュ無効化を部分的に行うか全体へ行うかの閾値を設定

  Arguments:    icache            命令キャッシュの無効化閾値
                dcache            データキャッシュの無効化閾値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_SetCacheFlushThreshold(u32 icache, u32 dcache)
{
#if defined(SDK_ARM9)
    SDK_ASSERT(CARD_IsAvailable());
    cardi_common.flush_threshold_ic = icache;
    cardi_common.flush_threshold_dc = dcache;
#else
    (void)icache;
    (void)dcache;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CARD_LockRom

  Description:  ROMアクセスのためにカードバスをロックする

  Arguments:    lock id.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_LockRom(u16 lock_id)
{
    SDK_ASSERT(CARD_IsAvailable());

    /* リソースをロック */
    CARDi_LockResource(lock_id, CARD_TARGET_ROM);
    /* カードバスをロック */
    {
        // ARM7がロックしている場合はCPUが長時間ブロッキングしてしまうため
        // もっとも安価なVカウントアラームを使用してスリープする。
#define CARD_USING_SLEEPY_LOCK
#ifdef CARD_USING_SLEEPY_LOCK
        CARDEventListener   el[1];
        CARDi_InitEventListener(el);
        CARDi_SetEventListener(el, CARDi_LockBusCondition, &lock_id);
        CARDi_WaitForEvent(el);
#else
        // 動作検証が済めば不要。
        (void)OS_LockCard(lock_id);
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_UnlockRom

  Description:  ロックしたカードバスを解放する

  Arguments:    lock id which is used by CARD_LockRom().

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_UnlockRom(u16 lock_id)
{
    SDK_ASSERT(CARD_IsAvailable());
    SDK_ASSERT(cardi_common.lock_target == CARD_TARGET_ROM);

    /* カードバスをアンロック */
    {
        (void)OS_UnlockCard(lock_id);
    }
    /* リソースをアンロック */
    CARDi_UnlockResource(lock_id, CARD_TARGET_ROM);

}

/*---------------------------------------------------------------------------*
  Name:         CARD_LockBackup

  Description:  バックアップデバイスアクセスのためにカードバスをロックする

  Arguments:    lock_id     OS_GetLockID関数で確保した排他制御用ID

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_LockBackup(u16 lock_id)
{
    SDK_ASSERT(CARD_IsAvailable());

    /* プロセッサ内のROM/バックアップ排他を取る */
    CARDi_LockResource(lock_id, CARD_TARGET_BACKUP);
    /* 常に ARM7 がアクセス */
#if defined(SDK_ARM7)
    (void)OS_LockCard(lock_id);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CARD_UnlockBackup

  Description:  バックアップデバイスアクセスのためにロックしたカードバスを解放する

  Arguments:    lock_id     CARD_LockBackup関数で使用した排他制御用ID

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_UnlockBackup(u16 lock_id)
{
    SDK_ASSERT(CARD_IsAvailable());
    SDK_ASSERT(cardi_common.lock_target == CARD_TARGET_BACKUP);

#if defined(SDK_ARM9)
    /* 非同期処理の実行中にバスを解放しようとしたらブロッキング */
    if (!CARD_TryWaitBackupAsync())
    {
        OS_TWarning("called CARD_UnlockBackup() during backup asynchronous operation. (force to wait)\n");
        (void)CARD_WaitBackupAsync();
    }
#endif // defined(SDK_ARM9)

    /* 常に ARM7 がアクセス */
#if defined(SDK_ARM7)
    (void)OS_UnlockCard(lock_id);
#endif
    /* リソースをアンロック */
    CARDi_UnlockResource(lock_id, CARD_TARGET_BACKUP);
}

