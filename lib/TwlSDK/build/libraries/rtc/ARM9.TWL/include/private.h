/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - rtc
  File:     private.h

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
#ifndef TWL_LIBRARIES_RTC_ARM9_PRIVATE_H_
#define TWL_LIBRARIES_RTC_ARM9_PRIVATE_H_
#ifdef  __cplusplus
extern  "C" {
#endif

#include <nitro/rtc/ARM9/api.h>

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
/* 非同期関数排他処理用ロック定義 */
typedef enum RTCLock
{
    RTC_LOCK_OFF = 0,               /* ロック開錠状態 */
    RTC_LOCK_ON,                    /* ロック施錠状態 */
    RTC_LOCK_MAX
}
RTCLock;

/* 連続コマンド送信を含む処理用のシーケンス定義 */
typedef enum RTCSequence
{
    RTC_SEQ_GET_DATE = 0,           /* 日付取得シーケンス */
    RTC_SEQ_GET_TIME,               /* 時刻取得シーケンス */
    RTC_SEQ_GET_DATETIME,           /* 日付・時刻取得シーケンス */
    RTC_SEQ_SET_DATE,               /* 日付設定シーケンス */
    RTC_SEQ_SET_TIME,               /* 時刻設定シーケンス */
    RTC_SEQ_SET_DATETIME,           /* 日付・時刻設定シーケンス */
    RTC_SEQ_GET_ALARM1_STATUS,      /* アラーム１状態取得シーケンス */
    RTC_SEQ_GET_ALARM2_STATUS,      /* アラーム２状態取得シーケンス */
    RTC_SEQ_GET_ALARM_PARAM,        /* アラーム設定値取得シーケンス */
    RTC_SEQ_SET_ALARM1_STATUS,      /* アラーム１状態変更シーケンス */
    RTC_SEQ_SET_ALARM2_STATUS,      /* アラーム２状態変更シーケンス */
    RTC_SEQ_SET_ALARM1_PARAM,       /* アラーム１設定値変更シーケンス */
    RTC_SEQ_SET_ALARM2_PARAM,       /* アラーム２設定値変更シーケンス */
    RTC_SEQ_SET_HOUR_FORMAT,        /* 時間表記フォーマット変更シーケンス */
    RTC_SEQ_SET_REG_STATUS2,        /* ステータス２レジスタ書き込みシーケンス */
    RTC_SEQ_SET_REG_ADJUST,         /* アジャストレジスタ書き込みシーケンス */
    RTC_SEQ_GET_COUNTER,            /* アップカウンタの値取得シーケンス */
    RTC_SEQ_GET_FOUT,               /* FOUT値取得シーケンス */
    RTC_SEQ_SET_FOUT,               /* FOUT値変更シーケンス */
    RTC_SEQ_MAX
}
RTCSequence;

/* ワーク用構造体 */
typedef struct RTCWork
{
    u32     lock;                   /* 排他ロック */
    RTCCallback callback;           /* 非同期関数コールバック退避用 */
    void   *buffer[2];              /* 非同期関数パラメータ退避用 */
    void   *callbackArg;            /* コールバック関数の引数保存用 */
    u32     sequence;               /* 連続処理モード管理用 */
    u32     index;                  /* 連続処理の状況管理用 */
    RTCInterrupt interrupt;         /* アラーム通知時の呼び出し関数退避用 */
    RTCResult commonResult;         /* 非同期関数の処理結果退避用 */

}
RTCWork;


/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
RTCWork*    RTCi_GetSysWork(void);
RTCResult   RTCEXi_CommonCallback(void);

RTCResult   RTCEXi_GetCounterAsync(u32* count, RTCCallback callback, void* arg);
RTCResult   RTCEXi_GetCounter(u32* count);
RTCResult   RTCEXi_GetFoutAsync(u16* fout, RTCCallback callback, void* arg);
RTCResult   RTCEXi_GetFout(u16* fout);
RTCResult   RTCEXi_SetFoutAsync(const u16* fout, RTCCallback callback, void* arg);
RTCResult   RTCEXi_SetFout(const u16* fout);

BOOL    RTCEXi_ReadRawCounterAsync(void);
BOOL    RTCEXi_ReadRawFoutAsync(void);
BOOL    RTCEXi_WriteRawFoutAsync(void);
BOOL    RTCEXi_ReadRawAlarmEx1Async(void);
BOOL    RTCEXi_WriteRawAlarmEx1Async(void);
BOOL    RTCEXi_ReadRawAlarmEx2Async(void);
BOOL    RTCEXi_WriteRawAlarmEx2Async(void);


/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_LIBRARIES_RTC_ARM9_PRIVATE_H_ */
