/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_event.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-21#$
  $Rev: 9385 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_EVENT_H__
#define NITRO_LIBRARIES_CARD_EVENT_H__


#include <nitro.h>


// 完了通知が無いソフトウェア的な状態変化を監視するための構造体と関数群。
// 現時点ではCARD_LockRom関数でしか使用していないがモジュールとして分離しておく。


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* declarations */


// 完了通知が無いソフトウェア的な状態変化を監視するための構造体。
typedef struct CARDEventListener
{
    OSEvent     event[1];
    OSVAlarm    valarm[1];
    BOOL(*Condition)(void*);
    void       *userdata;
    u16         lockID;
    u8          padding[2];
}
CARDEventListener;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_PeekEventListener

  Description:  イベント条件の判定。
                条件が満たされなければVカウントアラームを発行する。

  Arguments:    arg : CARDEventListener構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_PeekEventListener(void *arg)
{
    CARDEventListener  *el = (CARDEventListener *)arg;
    if (el->Condition(el->userdata))
    {
        OS_SignalEvent(el->event, 0x00000001UL);
    }
    else
    {
        OS_SetVAlarm(el->valarm, HW_LCD_HEIGHT, OS_VALARM_DELAY_MAX, CARDi_PeekEventListener, el);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitEventListener

  Description:  イベントリスナー構造体を初期化。

  Arguments:    el : CARDEventListener構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_InitEventListener(CARDEventListener *el)
{
    OS_InitEvent(el->event);
    OS_CreateVAlarm(el->valarm);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetEventListener

  Description:   イベントリスナー構造体にイベントを設定。

  Arguments:    el        : CARDEventListener構造体
                condition : イベント条件
                userdata  : イベント条件に指定可能な任意のユーザ定義引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_SetEventListener(CARDEventListener *el, BOOL(*condition)(void*), void *userdata)
{
    el->Condition = condition;
    el->userdata = userdata;
    CARDi_PeekEventListener(el);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitForEvent

  Description:   イベントリスナー構造体の完了を待機。

  Arguments:    el        : CARDEventListener構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_WaitForEvent(CARDEventListener *el)
{
    (void)OS_WaitEventEx(el->event, 0x00000001UL, OS_EVENT_MODE_AND, 0x00000001UL);
}


/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_LIBRARIES_CARD_EVENT_H__
