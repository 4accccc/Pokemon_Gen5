/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     common.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_COMMON_H_
#define NITRO_CARD_COMMON_H_


#include <nitro/card/types.h>

#include <nitro/memorymap.h>
#include <nitro/mi/dma.h>
#include <nitro/os.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* constants */

// カードスレッドのデフォルト優先レベル
#define	CARD_THREAD_PRIORITY_DEFAULT	4


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_Init

  Description:  DARDライブラリを初期化

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_Init(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_IsAvailable

  Description:  CARDライブラリが利用可能か判定

  Arguments:    None.

  Returns:      CARD_Init関数をすでに呼び出されていればTRUE
 *---------------------------------------------------------------------------*/
BOOL    CARD_IsAvailable(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_IsEnabled

  Description:  カードにアクセス可能か判定

  Arguments:    None.

  Returns:      カードにアクセスする権限があればTRUE
 *---------------------------------------------------------------------------*/
BOOL    CARD_IsEnabled(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_CheckEnabled

  Description:  カードにアクセスする権限があるか判定し、なければ強制停止

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_CheckEnabled(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_Enable

  Description:  カードにアクセスする権限を設定

  Arguments:    enable      アクセスする権限があればTRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_Enable(BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetThreadPriority

  Description:  ライブラリ内部の非同期処理用スレッドの優先度を取得

  Arguments:    None.

  Returns:      内部スレッドの優先度
 *---------------------------------------------------------------------------*/
u32     CARD_GetThreadPriority(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_SetThreadPriority

  Description:  ライブラリ内部の非同期処理用スレッドの優先度を設定

  Arguments:    None.

  Returns:      設定前の内部スレッドの優先度
 *---------------------------------------------------------------------------*/
u32     CARD_SetThreadPriority(u32 prior);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetResultCode

  Description:  最後に呼び出したCARD関数の結果を取得

  Arguments:    None.

  Returns:      最後に呼び出したCARD関数の結果
 *---------------------------------------------------------------------------*/
CARDResult CARD_GetResultCode(void);


/*---------------------------------------------------------------------------*
 * for internal use
 *---------------------------------------------------------------------------*/

// CARDライブラリ内部イベント
typedef u32 CARDEvent;
#define CARD_EVENT_PULLEDOUT  0x00000001
#define CARD_EVENT_SLOTRESET  0x00000002

// イベントフックコールバックとフック登録構造体
typedef void (*CARDHookFunction)(void*, CARDEvent, void*);

typedef struct CARDHookContext
{
    struct CARDHookContext *next;
    void                   *userdata;
    CARDHookFunction        callback;
}
CARDHookContext;

/*---------------------------------------------------------------------------*
  Name:         CARDi_RegisterHook

  Description:  CARDライブラリの内部イベントフックを登録

  Arguments:    hook        登録に使用するフック構造体
                callback    イベント発生時に呼び出すコールバック関数
                arg         コールバック引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_RegisterHook(CARDHookContext *hook, CARDHookFunction callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CARDi_UnregisterHook

  Description:  CARDライブラリの内部イベントフックを解除

  Arguments:    hook        登録に使用したフック構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_UnregisterHook(CARDHookContext *hook);

/*---------------------------------------------------------------------------*
  Name:         CARDi_NotifyEvent

  Description:  CARDライブラリの内部イベントを全てのフックに通知

  Arguments:    event       発生したイベント
                arg         イベントごとの引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_NotifyEvent(CARDEvent event, void *arg);


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_COMMON_H_
