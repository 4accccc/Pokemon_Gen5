/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - libraries
  File:     swclock.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date: 2008-01-11#$
  $Rev: 9449 $
  $Author: tokunaga_eiji $
 *---------------------------------------------------------------------------*/

#include <nitro/os.h>
#include <nitro/rtc.h>
#include <nitro/spi/ARM9/pm.h>

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static u16       rtcSWClockInitialized;       // チック初期化確認フラグ
static OSTick    rtcSWClockBootTicks;         // ブート時 RTC のチック換算値
static RTCResult rtcLastResultOfSyncSWClock;  // 最後の同期時の RTCResult
static PMSleepCallbackInfo rtcSWClockSleepCbInfo; // スリープ復帰時コールバック情報

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

static void RtcGetDateTimeExFromSWClock(RTCDate * date, RTCTimeEx *time);
static void RtcSleepCallbackForSyncSWClock(void* args);

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
RTCResult RTC_InitSWClock(void)
{
    SDK_ASSERT(OS_IsTickAvailable());
    
    // 既に初期化されている場合は何もせずに RTC_RESULT_SUCCESS を返す
    if(rtcSWClockInitialized)
    {
        return RTC_RESULT_SUCCESS;
    }
    
    // ソフトウェアクロック値を現在の RTC 値と同期する。
    (void) RTC_SyncSWClock();

    // スリープ復帰時コールバックを登録する。
    PM_SetSleepCallbackInfo(&rtcSWClockSleepCbInfo, RtcSleepCallbackForSyncSWClock, NULL);
    PM_AppendPostSleepCallback(&rtcSWClockSleepCbInfo);
    
    rtcSWClockInitialized = TRUE;

    return rtcLastResultOfSyncSWClock;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetSWClockTicks

  Description:  現在のチック値とブート時 RTC のチック換算値を合計して 2000年1月
                1日午前0時0分0秒から現在時刻までの時間をチック換算値で返す。

  Arguments:    None

  Returns:      OSTick - 2000年1月1日午前0時0分0秒から現在時刻までチック換算時間
                         最後の同期に失敗している場合は 0
 *---------------------------------------------------------------------------*/
OSTick RTC_GetSWClockTick(void)
{
    if(rtcLastResultOfSyncSWClock == RTC_RESULT_SUCCESS)
    {
        return OS_GetTick() + rtcSWClockBootTicks;
    }
    else
    {
        return 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetLastSyncSWClockResult

  Description:  最後のソフトウェアクロック同期時に呼び出された RTC_GetDateTime
                の結果を返す。

  Arguments:    None

  Returns:      RTCResult - 同期時に呼び出された RTC_GetDateTime の結果
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetLastSyncSWClockResult(void)
{
    return rtcLastResultOfSyncSWClock;
}


/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateTimeExFromSWClock

  Description:  CPU チックを利用するソフトウェアクロックからミリ秒単位までの
                日付・時刻データを読み出す。

  Arguments:    date      - 日付データを格納するバッファを指定。
                time      - 時刻データを格納するバッファを指定。

  Returns:      RTCResult - 最後の同期時に保存された RTCResult を返す
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateTimeExFromSWClock(RTCDate *date, RTCTimeEx *time)
{
    SDK_NULL_ASSERT(date);
    SDK_NULL_ASSERT(time);

    RtcGetDateTimeExFromSWClock(date, time);
    
    return rtcLastResultOfSyncSWClock;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SyncSWClock

  Description:  CPU チックを利用するソフトウェアクロックを RTC に同期する。
                同期時に呼び出された RTC_GetDateTime の結果が保存され、
                RTC_GetDateTimeExFromSWClock の戻り値として返される。

  Arguments:    None

  Returns:      RTCResult - デバイス操作の処理結果を返す。
 *---------------------------------------------------------------------------*/
RTCResult RTC_SyncSWClock(void)
{   
    RTCDate currentDate;
    RTCTime currentTime;
    
    // RTC_GetDateTime の結果を保存する。
    rtcLastResultOfSyncSWClock = RTC_GetDateTime(&currentDate, &currentTime);
    // 現在の RTC をチック換算し、現在 CPU チックとの差分を取って、ブート時 RTC のチック換算値を保存する。    
    rtcSWClockBootTicks = OS_SecondsToTicks(RTC_ConvertDateTimeToSecond(&currentDate, &currentTime)) - OS_GetTick();

    return rtcLastResultOfSyncSWClock;
}

/*---------------------------------------------------------------------------*
  Name:         RtcGetDateTimeExFromSWClock

  Description:  現在のチックとソフトウェアクロックが RTC に同期した時に保存さ
                れたチックの差分から、日付と時刻データを計算する。

  Arguments:    date      - buffer for storing RTCDate
                time      - buffer for storing RTCTimeEx

  Returns:      None
 *---------------------------------------------------------------------------*/
static void RtcGetDateTimeExFromSWClock(RTCDate * date, RTCTimeEx *time)
{
    OSTick currentTicks;
    s64 currentSWClockSeconds;

    currentTicks = RTC_GetSWClockTick();
    currentSWClockSeconds = (s64) OS_TicksToSeconds(currentTicks);
        
    // ミリ秒をのぞいた現在の秒数を RTCDate と RTCTime に変換する。
    RTC_ConvertSecondToDateTime(date, (RTCTime*)time, currentSWClockSeconds);

    // ミリ秒を別個に加える
    time->millisecond = (u32) (OS_TicksToMilliSeconds(currentTicks) % 1000);  
}

/*---------------------------------------------------------------------------*
  Name:         RtcSleepCallbackForSyncSWClock

  Description:  RTC_InitSWClock 呼び出し時に登録されるスリープコールバック。
                スリープ復帰時にソフトウェアクロックを RTC に同期させる。

  Arguments:    args     - 利用されない引数

  Returns:      None
 *---------------------------------------------------------------------------*/
#define RTC_SWCLOCK_SYNC_RETRY_INTERVAL   1   // 単位 milliseconds
static void RtcSleepCallbackForSyncSWClock(void* args)
{
#pragma unused(args)
    for (;;)
    {
        (void) RTC_SyncSWClock();

        // RTC が他のスレッドもしくは ARM7 で処理中か、PXI FIFO が FULL の時は
        // RTC_SWCLOCK_SYNC_RETRY_INTERVAL ミリ秒待ってリトライ
        if(rtcLastResultOfSyncSWClock != RTC_RESULT_BUSY &&
           rtcLastResultOfSyncSWClock != RTC_RESULT_SEND_ERROR)
        {
            break;
        }

        OS_TWarning("RTC_SyncSWClock() failed at sleep callback. Retry... \n");
        OS_Sleep(RTC_SWCLOCK_SYNC_RETRY_INTERVAL);
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
