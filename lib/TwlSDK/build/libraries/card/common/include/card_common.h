/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_common.h

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
#ifndef NITRO_LIBRARIES_CARD_COMMON_H__
#define NITRO_LIBRARIES_CARD_COMMON_H__


#include <nitro/card/common.h>
#include <nitro/card/backup.h>


#include "../include/card_utility.h"
#include "../include/card_task.h"
#include "../include/card_command.h"



#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* constants */

// CARD 内部状態
enum
{
    // 初期化済み。
    CARD_STAT_INIT = (1 << 0),

    // コマンドPXI初期化済み。 (ARM9)
    CARD_STAT_INIT_CMD = (1 << 1),

    // コマンドを発行して完了していない状態。(ARM9)
    // WaitForTask()の処理前もこの状態に含まれる。
    CARD_STAT_BUSY = (1 << 2),

    // 移管されたタスクがある状態。(ARM9/ARM7)
    CARD_STAT_TASK = (1 << 3),

    // ARM7からのリクエスト完了通知待ち。 (ARM9)
    CARD_STAT_WAITFOR7ACK = (1 << 5),

    // キャンセル要求中。
    CARD_STAT_CANCEL = (1 << 6)
};

#define CARD_UNSYNCHRONIZED_BUFFER  (void*)0x80000000

typedef enum
{
    CARD_TARGET_NONE,
    CARD_TARGET_ROM,
    CARD_TARGET_BACKUP,
    CARD_TARGET_RW
}
CARDTargetMode;

typedef u32 CARDAccessLevel;
#define CARD_ACCESS_LEVEL_NONE      0
#define CARD_ACCESS_LEVEL_BACKUP    1
#define CARD_ACCESS_LEVEL_ROM       2
#define CARD_ACCESS_LEVEL_FULL      3


/*---------------------------------------------------------------------------*/
/* declarations */

typedef s32 CARDiOwner;

// CARD 共通パラメータ
typedef struct CARDiCommon
{
    // リクエスト通信用共有メモリ。
    CARDiCommandArg *cmd;

    // 状態フラグ。
    volatile u32    flag;

    // タスクのデフォルト優先度。
    u32             priority;

#if	defined(SDK_ARM9)
    // ROMリード時のキャッシュ全体無効化閾値。
    // 一定サイズを上回る場合はFlushRangeよりFlushAllの方が高効率。
    u32     flush_threshold_ic;
    u32     flush_threshold_dc;
#endif

    // カードアクセス権管理。
    // プロセッサ内のカード/バックアップ排他を取る。
    // これはCARDアクセスを使用する複数の非同期関数(Rom&Backup)が
    // 同一スレッドから呼び出される場合があるために必要。
    //
    // OSMutex がスレッドに付いて回るのに対してこれは lock-ID に付いて回る。
    volatile CARDiOwner lock_owner;    // ==s32 with Error status
    volatile int        lock_ref;
    OSThreadQueue       lock_queue[1];
    CARDTargetMode      lock_target;

    // タスクスレッド情報。
    struct
    {
        OSThread    context[1];
        u8          stack[0x400];
    }
    thread;

#if defined(SDK_ARM7)
    // 新形式のタスクプロシージャ。
    CARDTask        task[1];
    CARDTaskQueue   task_q[1];
    // ARM9から命令されている最新のコマンド。
    int             command;
    u8              padding[20];
#else
    // 旧形式のタスクプロシージャ。
    void          (*task_func) (struct CARDiCommon *);
    // ユーザコールバックと引数。
    MIDmaCallback   callback;
    void           *callback_arg;
    // タスク完了待ちスレッド。
    OSThreadQueue   busy_q[1];
    // APIタスクパラメータ。
    u32                     src;
    u32                     dst;
    u32                     len;
    u32                     dma;
    const CARDDmaInterface *DmaCall;
    // バックアップコマンド管理情報。
    CARDRequest             req_type;
    int                     req_retry;
    CARDRequestMode         req_mode;
    OSThread               *current_thread_9;
//    u8                      padding[32];
#endif

}
CARDiCommon;


SDK_COMPILER_ASSERT(sizeof(CARDiCommon) % 32 == 0);


/*---------------------------------------------------------------------------*/
/* variables */

extern CARDiCommon cardi_common;
extern u32  cardi_rom_base;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_ExecuteOldTypeTask

  Description:  非同期ならワーカスレッドへ投げて同期ならこの場で実行。
                (すでにタスクスレッドがCARDi_WaitForTask()によって排他制御
                 されていることはこの関数の呼び出し元で保証する)

  Arguments:    task       設定するタスク関数
                async      非同期処理を希望するならTRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL CARDi_ExecuteOldTypeTask(void (*task) (CARDiCommon *), BOOL async);

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitForTask

  Description:  タスクスレッドの使用権利を取得できるまで待つ
                (所定のバスのロックはこの関数の呼び出し元で保証する)

  Arguments:    p            ライブラリのワークバッファ
                restart      さらに次のタスクを開始させるならTRUE
                callback     アクセス終了後のコールバック関数
                callback_arg コールバック関数の引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL CARDi_WaitForTask(CARDiCommon *p, BOOL restart, MIDmaCallback callback, void *callback_arg);

/*---------------------------------------------------------------------------*
  Name:         CARDi_EndTask

  Description:  タスク終了を通知してタスクスレッドの使用権利を解放

  Arguments:    p            ライブラリのワークバッファ (効率のために引数渡し)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EndTask(CARDiCommon *p);

void CARDi_OldTypeTaskThread(void *arg);


/*---------------------------------------------------------------------------*
  Name:         CARDi_GetTargetMode

  Description:  CARD バスの現在のロックターゲットを取得

  Arguments:    None.

  Returns:      CARDTargetMode で示される3つの状態のいずれか
 *---------------------------------------------------------------------------*/
SDK_INLINE CARDTargetMode CARDi_GetTargetMode(void)
{
    return cardi_common.lock_target;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_LockResource

  Description:  リソース排他のロック

  Arguments:    owner      ロックの所有者を示す lock-ID
                target     ロックすべきカードバス上のリソースターゲット

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_LockResource(CARDiOwner owner, CARDTargetMode target);

/*---------------------------------------------------------------------------*
  Name:         CARDi_UnlockResource

  Description:  リソース排他のアンロック

  Arguments:    owner      ロックの所有者を示す lock-ID
                target     アンロックすべきカードバス上のリソースターゲット

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_UnlockResource(CARDiOwner owner, CARDTargetMode target);

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetAccessLevel

  Description:  許可されているROMアクセスレベルを取得

  Arguments:    None.

  Returns:      許可されているROMアクセスレベル
 *---------------------------------------------------------------------------*/
CARDAccessLevel CARDi_GetAccessLevel(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitAsync

  Description:  非同期完了を待つ

  Arguments:    None.

  Returns:      最新の処理結果がCARD_RESULT_SUCCESSであればTRUE
 *---------------------------------------------------------------------------*/
BOOL    CARDi_WaitAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_TryWaitAsync

  Description:  非同期完了待機を試行して成否に関わらずただちに制御を返す

  Arguments:    None.

  Returns:      最新の非同期処理が完了していればTRUE
 *---------------------------------------------------------------------------*/
BOOL    CARDi_TryWaitAsync(void);

void CARDi_InitResourceLock(void);



void CARDi_InitCommand(void);


#ifdef __cplusplus
} // extern "C"
#endif



#endif // NITRO_LIBRARIES_CARD_COMMON_H__
