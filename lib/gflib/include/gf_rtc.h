//============================================================================================
/**
 * @file	gf_rtc.h
 * @brief	RTCアクセスラッパー用ヘッダ
 * @author	tamada	GAME FREAK inc.
 * @date	2005.11.21
 *
 * 直接のRTCライブラリ使用は禁止する。
 * この関数経由でのアクセスを行う。
 */
//============================================================================================
#ifndef	GFL_GF_STANDARD_GF_RTC_H__  // インクルードガード
#define	GFL_GF_STANDARD_GF_RTC_H__

#include <nitro.h>
#include <nnsys.h>

//--------------------------------------------------------------
//  定数
//--------------------------------------------------------------
#define GFL_RTC_TIME_SECOND_MAX   (86399)   //GFL_RTC_GetTimeBySecond関数で得られる値の最大値

//--------------------------------------------------------------
//	初期化＆メイン
//--------------------------------------------------------------
extern void GFL_RTC_Init(void);
extern void GFL_RTC_Main(void);

//--------------------------------------------------------------
//	取得用関数
//--------------------------------------------------------------
extern void GFL_RTC_GetDateTime(RTCDate * date, RTCTime * time);
extern void GFL_RTC_GetTime(RTCTime * time);
extern void GFL_RTC_GetDate(RTCDate * date);
extern int GFL_RTC_GetTimeBySecond(void);
extern s64 GFL_RTC_GetDateTimeBySecond(void);

//経過時間の取得
extern s64 GFL_RTC_GetPassTime(s64 start_sec, s64 end_sec);
extern int GFL_RTC_GetDaysOffset(const RTCDate * date);

#ifdef	PM_DEBUG
//--------------------------------------------------------------
//	デバッグ用関数
//--------------------------------------------------------------
extern void GFL_RTC_DEBUG_StartFakeTime(int rate);
extern void GFL_RTC_DEBUG_StopFakeTime(void);
extern void GFL_RTC_DEBUG_StartFakeFixTime(int hour, int minute);

#endif

#endif	//GFL_GF_STANDARD_GF_RTC_H__
