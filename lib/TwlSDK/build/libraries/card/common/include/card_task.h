/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_task.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_TASK_H__
#define NITRO_LIBRARIES_CARD_TASK_H__


#include <nitro/os.h>


/*---------------------------------------------------------------------------*
 * 優先度つきタスクのキューを管理するユーティリティ。
 * CARDライブラリ内部でのみ使用しているが単体として切り出し可能。
 *---------------------------------------------------------------------------*/


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* declarations */


// ワーカスレッドにバックグラウンドで処理させるタスク構造体。
struct CARDTask;
typedef void (*CARDTaskFunction)(struct CARDTask *);

typedef struct CARDTask
{
    struct CARDTask    *next;
    u32                 priority;
    void               *userdata;
    CARDTaskFunction    function;
    CARDTaskFunction    callback;
}
CARDTask;

// タスクキュー構造体。
typedef struct CARDTaskQueue
{
    CARDTask * volatile list;
    OSThreadQueue       workers[1];
    u32                 quit:1;
    u32                 dummy:31;
}
CARDTaskQueue;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitTaskQueue

  Description:  タスクキュー構造体を初期化。

  Arguments:    queue  : CARDTaskQueue構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitTaskQueue(CARDTaskQueue *queue);

/*---------------------------------------------------------------------------*
  Name:         CARDi_QuitTaskQueue

  Description:  タスクキューを監視する全てのワーカスレッドを終了しQUIT状態にする。
                タスクキューの状態はそのまま残り、処理中のタスクは完遂される。

  Arguments:    queue : CARDTaskQueue構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_QuitTaskQueue(CARDTaskQueue *queue);

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
                    CARDTaskFunction function, CARDTaskFunction callback);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProcessTask

  Description:  タスクをこの場で実行するかタスクキューへ追加。

  Arguments:    queue          : CARDTaskQueue構造体。
                task           : 実行またはキューへ追加するタスク。
                blocking       : キューへ追加せずこの場で実行するならTRUE。
                                 キューへ追加してワーカスレッドへ任せるならFALSE。
                changePriority : タスクに設定された優先度で実行するならTRUE。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ProcessTask(CARDTaskQueue *queue, CARDTask *task, BOOL blocking, BOOL changePriority);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReceiveTask

  Description:  タスクキューから次のタスクを取得。

  Arguments:    queue    : CARDTaskQueue構造体。
                blocking : キューが空の場合にブロックするならTRUE。

  Returns:      取得できた新規タスク。
                非ブロッキングかつキューが空か、またはキューがQUIT状態ならNULL。
 *---------------------------------------------------------------------------*/
CARDTask* CARDi_ReceiveTask(CARDTaskQueue *queue, BOOL blocking);

/*---------------------------------------------------------------------------*
  Name:         CARDi_TaskWorkerProcedure

  Description:  タスクキューを監視し続けるワーカスレッドのプロシージャ。
                このプロシージャを指定して生成したスレッドは
                CARDi_QuitTask関数を発行するまでタスクを処理し続ける。
                単一のタスクキューに対してワーカスレッドを複数生成してもよい。

  Arguments:    arg : 監視すべきCARDTaskQueue構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_TaskWorkerProcedure(void *arg);


/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_LIBRARIES_CARD_TASK_H__
