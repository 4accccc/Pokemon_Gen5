/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_task.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-21#$
  $Rev: 9385 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_task.h"


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitTaskQueue

  Description:  タスクキュー構造体を初期化。

  Arguments:    queue  : CARDTaskQueue構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitTaskQueue(CARDTaskQueue *queue)
{
    queue->list = NULL;
    queue->quit = FALSE;
    OS_InitThreadQueue(queue->workers);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_QuitTaskQueue

  Description:  タスクキューを監視する全てのスレッドを終了。

  Arguments:    queue : CARDTaskQueue構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_QuitTaskQueue(CARDTaskQueue *queue)
{
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    queue->quit = TRUE;
    OS_WakeupThread(queue->workers);
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitTask

  Description:  タスク構造体を初期化。

  Arguments:    task     : CARDTask構造体。
                priority : スレッド優先度。
                userdata : タスクに関連付ける任意のユーザ定義データ。
                function : 実行させるタスクの関数ポインタ。
                callback : タスク完了後に呼び出すコールバック。 (不要ならNULL)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitTask(CARDTask *task, u32 priority, void *userdata,
                    CARDTaskFunction function, CARDTaskFunction callback)
{
    task->next = NULL;
    task->priority = priority;
    task->userdata = userdata;
    task->function = function;
    task->callback = callback;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProcessTask

  Description:  タスクキューの終端にタスク構造体を追加するかこの場で実行。

  Arguments:    queue          : CARDTaskQueue構造体。
                task           : 追加または実行するタスク。
                blocking       : ブロッキング操作ならTRUE。
                                 この場合、キューへは追加せずこの場で実行する。
                changePriority : タスクに設定された優先度に従うならTRUE。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ProcessTask(CARDTaskQueue *queue, CARDTask *task, BOOL blocking, BOOL changePriority)
{
    // 非ブロッキング操作ならキュー終端へ追加。
    if (!blocking)
    {
        OSIntrMode  bak_cpsr = OS_DisableInterrupts();
        CARDTask  **pp = (CARDTask **)&queue->list;
        for(; *pp; pp = &(*pp)->next)
        {
        }
        *pp = task;
        // 空のキューに対する最初のタスクだったらワーカスレッドへ通知。
        if (pp == &queue->list)
        {
            OS_WakeupThread(queue->workers);
        }
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    // ブロッキング操作ならこの場でタスクとコールバックを実行。
    else
    {
        // 必要ならタスクに設定された優先度のもとで動作する。
        OSThread   *curth = OS_GetCurrentThread();
        u32         prio = 0;
        if (changePriority)
        {
            prio = OS_GetThreadPriority(curth);
            (void)OS_SetThreadPriority(curth, task->priority);
        }
        if (task->function)
        {
            (*task->function)(task);
        }
        // コールバック内で次のタスクを設定されても良いようにする。
        if (task->callback)
        {
            (*task->callback)(task);
        }
        // 一時的に変更した優先度を復帰する。
        if (changePriority)
        {
            (void)OS_SetThreadPriority(curth, prio);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReceiveTask

  Description:  タスクリストから次のタスクを取得。

  Arguments:    queue    : CARDTaskQueue構造体。
                blocking : 存在しない場合にブロッキングするならTRUE。

  Returns:      取得した新規タスクまたはNULL
 *---------------------------------------------------------------------------*/
CARDTask* CARDi_ReceiveTask(CARDTaskQueue *queue, BOOL blocking)
{
    CARDTask   *retval = NULL;
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    while (!queue->quit)
    {
        retval = queue->list;
        if ((retval != NULL) || !blocking)
        {
            break;
        }
        OS_SleepThread(queue->workers);
    }
    if (retval)
    {
        queue->list = retval->next;
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_TaskWorkerProcedure

  Description:  タスクリストを処理し続けるワーカスレッドのプロシージャ。

  Arguments:    arg : CARDTaskQueue構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_TaskWorkerProcedure(void *arg)
{
    CARDTaskQueue  *queue = (CARDTaskQueue*)arg;
    // どのような優先度のタスクも受け付けるため、待機中は最高レベル。
    (void)OS_SetThreadPriority(OS_GetCurrentThread(), 0);
    for (;;)
    {
        // キューを監視して次のタスク到着まで待機。
        CARDTask   *task = CARDi_ReceiveTask(queue, TRUE);
        // QUIT状態ならブロッキングモードでもNULLが返される。
        if (!task)
        {
            break;
        }
        // 指定された優先度のもとにタスクとコールバックを実行。
        CARDi_ProcessTask(queue, task, TRUE, TRUE);
    }
}





#if defined(SDK_ARM9)
/*---------------------------------------------------------------------------*
 * 古いタスクスレッド処理
 *---------------------------------------------------------------------------*/

#include "../include/card_common.h"

/*---------------------------------------------------------------------------*
  Name:         CARDi_ExecuteOldTypeTask

  Description:  非同期ならワーカスレッドへ投げて同期ならこの場で実行。
                (すでにタスクスレッドがCARDi_WaitForTask()によって排他制御
                 されていることはこの関数の呼び出し元で保証する)

  Arguments:    task       設定するタスク関数
                async      非同期処理を希望するならTRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL CARDi_ExecuteOldTypeTask(void (*task) (CARDiCommon *), BOOL async)
{
    CARDiCommon *p = &cardi_common;
    if (async)
    {
        // まずスリープ中のタスクスレッド優先度を変更。
        (void)OS_SetThreadPriority(p->thread.context, p->priority);
        // 処理させるタスクを設定し, スレッドを起こす。
        p->task_func = task;
        p->flag |= CARD_STAT_TASK;
        OS_WakeupThreadDirect(p->thread.context);
    }
    else
    {
        (*task)(p);
        CARDi_EndTask(p);
    }
    return async ? TRUE : (p->cmd->result == CARD_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_OldTypeTaskThread

  Description:  タスクスレッドのメイン関数

  Arguments:    arg          不使用

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_OldTypeTaskThread(void *arg)
{
    CARDiCommon *p = &cardi_common;
    (void)arg;
    for (;;)
    {
        // 次の処理を待つ。
        OSIntrMode bak_psr = OS_DisableInterrupts();
        for (;;)
        {
            if ((p->flag & CARD_STAT_TASK) != 0)
            {
                break;
            }
            OS_SleepThread(NULL);
        }
        (void)OS_RestoreInterrupts(bak_psr);
        // リクエスト処理。
        (void)CARDi_ExecuteOldTypeTask(p->task_func, FALSE);
    }
}

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
BOOL CARDi_WaitForTask(CARDiCommon *p, BOOL restart, MIDmaCallback callback, void *callback_arg)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    // 現在の処理が完了するまで待機。
    while ((p->flag & CARD_STAT_BUSY) != 0)
    {
        OS_SleepThread(p->busy_q);
    }
    // 直ちに次のコマンドを実行するならこの場で再び処理中へ。
    if (restart)
    {
        p->flag |= CARD_STAT_BUSY;
        p->callback = callback;
        p->callback_arg = callback_arg;
    }
    (void)OS_RestoreInterrupts(bak_psr);
    return (p->cmd->result == CARD_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EndTask

  Description:  タスク終了を通知してタスクスレッドの使用権利を解放

  Arguments:    p            ライブラリのワークバッファ (効率のために引数渡し)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EndTask(CARDiCommon *p)
{
    MIDmaCallback   callback = p->callback;
    void           *userdata = p->callback_arg;

    OSIntrMode bak_psr = OS_DisableInterrupts();
    // 完了状態にして待機中のスレッドへ通知。
    p->flag &= ~(CARD_STAT_BUSY | CARD_STAT_TASK | CARD_STAT_CANCEL);
    OS_WakeupThread(p->busy_q);
    (void)OS_RestoreInterrupts(bak_psr);
    // 必要ならコールバックを呼び出す。
    if (callback)
    {
        (*callback) (userdata);
    }
}

#endif // defined(SDK_ARM9)
