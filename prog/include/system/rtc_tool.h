//============================================================================================
/**
 * @file	rtc_tool.h
 * @brief	RTCの中で、ゲームに依存するもの
 * @author	tamada	GAME FREAK inc.
 * @date	2005.11.21
 *
 * 直接のRTCライブラリ使用は禁止する。
 * この関数経由でのアクセスを行う。
 */
//============================================================================================
#ifndef	WB_SYSTEM_RTC_TOOL_H__  // インクルードガード
#define	WB_SYSTEM_RTC_TOOL_H__

#include <gflib.h>
#include "gamesystem/pm_season.h"
#include "system/timezone.h"

//--------------------------------------------------------------
//	時間帯関数
//--------------------------------------------------------------
// 季節　時間帯
// 季節ごとに変わる時間帯です。
// 反映箇所は
//  フィールドライト
//  配置モデル時間帯アニメ
extern int PM_RTC_GetTimeZone(int season);
extern int PM_RTC_ConvertHourToTimeZone(int season, int hour);
extern int PM_RTC_GetTimeZoneChangeHour( int season, TIMEZONE timezone );
extern int PM_RTC_GetTimeZoneChangeMargin( int season, TIMEZONE timezone );

// Default 時間帯  
#define PM_RTC_TIMEZONE_DEFAULT_SEASON  ( PMSEASON_SUMMER )
//夜かどうかチェック
extern BOOL PM_RTC_IsNightTime( int season );

/// @todo 上記に置き換わったら廃止する
static inline BOOL GFL_RTC_IsNightTime( void )
{
  return PM_RTC_IsNightTime( PM_RTC_TIMEZONE_DEFAULT_SEASON );
}

//extern int GFL_RTC_ConvertHourToTimeZone(int hour);

//--------------------------------------------------------------
//	ツール関数
//--------------------------------------------------------------
extern int GFL_RTC_GetDaysOffset(const RTCDate * date);


static inline BOOL GFL_RTC_IsLeapYear(u32 year)
{
	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

#endif	//WB_SYSTEM_RTC_TOOL_H__
