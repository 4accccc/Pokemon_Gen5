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
#include "system/timezone.h"

//--------------------------------------------------------------
//	ツール関数
//--------------------------------------------------------------
extern int GF_RTC_ConvertHourToTimeZone(int hour);
extern int GF_RTC_GetTimeZone(void);
extern int GF_RTC_GetDaysOffset(const RTCDate * date);
//夜かどうかチェック
extern BOOL GF_RTC_IsNightTime(void);

static inline BOOL GF_RTC_IsLeapYear(u32 year)
{
	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

#endif	//WB_SYSTEM_RTC_TOOL_H__
