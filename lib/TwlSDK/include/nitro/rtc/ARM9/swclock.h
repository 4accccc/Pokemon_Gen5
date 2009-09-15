/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - include
  File:     swclock.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date$
  $Rev$
  $Author: tokunaga_eiji $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_RTC_ARM9_SWCLOCK_H_
#define NITRO_RTC_ARM9_SWCLOCK_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include    <nitro/types.h>

/*---------------------------------------------------------------------------*
  Name:         RTC_InitSWClock

  Description:  ソフトウェアクロックを初期化する。ソフトウェアクロック値を現在
                の RTC 値と同期し、スリープ復帰時コールバックに RTC 値とソフト
                ウェアクロックを再同期する関数を追加する。
                RTC_GetDateTimeExFromSWClock 関数および RTC_SyncSWClock 関数を
                呼ぶ前に呼んでおく必要がある。

  Arguments:    None

  Returns:      RTCResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult RTC_InitSWClock(void);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetSWClockTick

  Description:  現在のチック値とブート時 RTC のチック換算値を合計して 2000年1月
                1日0時00分から現在時刻までの時間をチック換算値で返す。

  Arguments:    None

  Returns:      OSTick - 2000年1月1日0時00分から現在時刻までの時間のチック換算値
 *---------------------------------------------------------------------------*/
OSTick RTC_GetSWClockTick(void);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetSyncSWClockResult

  Description:  最後のソフトウェアクロック同期時に呼び出された RTC_GetDateTime
                の結果を返す。

  Arguments:    None

  Returns:      RTCResult - 同期時に呼び出された RTC_GetDateTime の結果
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetLastSyncSWClockResult(void);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateTimeExFromSWClock

  Description:  CPU チックを利用するソフトウェアクロックからミリ秒単位までの
                日付・時刻データを読み出す。

  Arguments:    date      - 日付データを格納するバッファを指定。
                time      - 時刻データを格納するバッファを指定。

  Returns:      RTCResult - 最後の同期時に保存された RTCResult を返す
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateTimeExFromSWClock(RTCDate *date, RTCTimeEx *time);

/*---------------------------------------------------------------------------*
  Name:         RTC_SyncSWClock

  Description:  CPU チックを利用するソフトウェアクロックを RTC に同期する。

  Arguments:    None

  Returns:      RTCResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult RTC_SyncSWClock(void);

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_RTC_ARM9_SWCLOCK_H_ */


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
