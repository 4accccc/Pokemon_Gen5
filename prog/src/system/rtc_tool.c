//============================================================================================
/**
 * @file	rtc_tool.c
 * @brief	RTCの中で、ゲームに依存するもの
 * @author	tamada	GAME FREAK inc.
 * @date	2005.11.18
 */
//============================================================================================

#include <gflib.h>
#include "system/rtc_tool.h"


//============================================================================================
//
//		ツール関数
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief	夜かどうかの判定
 * @retval	TRUE	今は夜
 * @retval	FALSE	今は昼
 */
//--------------------------------------------------------------
BOOL GFL_RTC_IsNightTime(void)
{
	switch (GFL_RTC_GetTimeZone()) {
	case TIMEZONE_MIDNIGHT:
	case TIMEZONE_NIGHT:
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief	時間帯の取得
 * @return	int	時間帯（timezone.hを参照）
 */
//--------------------------------------------------------------
int GFL_RTC_GetTimeZone(void)
{
	RTCTime time;
	GFL_RTC_GetTime(&time);
	return GFL_RTC_ConvertHourToTimeZone(time.hour);
}

//--------------------------------------------------------------
/**
 * @brief	時間から時間帯への変換
 * @param	hour	時間（０－２３）
 * @return	int		時間帯（timezone.hを参照）
 */
//--------------------------------------------------------------
int GFL_RTC_ConvertHourToTimeZone(int hour)
{
	static const u8 timezone[24] = {
		//00:00 - 3:59
		TIMEZONE_MIDNIGHT,TIMEZONE_MIDNIGHT,TIMEZONE_MIDNIGHT,TIMEZONE_MIDNIGHT,
		//04:00 - 9:59
		TIMEZONE_MORNING,TIMEZONE_MORNING,TIMEZONE_MORNING,TIMEZONE_MORNING,
		TIMEZONE_MORNING,TIMEZONE_MORNING,
		//10:00 - 16:59
		TIMEZONE_NOON,TIMEZONE_NOON,TIMEZONE_NOON,TIMEZONE_NOON,TIMEZONE_NOON,
		TIMEZONE_NOON,TIMEZONE_NOON,
		//17:00 - 19:59
		TIMEZONE_EVENING,TIMEZONE_EVENING,TIMEZONE_EVENING,
		//20:00 - 23:59
		TIMEZONE_NIGHT,TIMEZONE_NIGHT,TIMEZONE_NIGHT,TIMEZONE_NIGHT
	};
	GF_ASSERT(0 <= hour && hour < 24);
	return timezone[hour];
}

//--------------------------------------------------------------
/**
 * @brief	経過時間の計算
 * @param	start_sec
 * @param	end_sec
 */
//--------------------------------------------------------------
s64 GFL_RTC_GetPassTime(s64 start_sec, s64 end_sec)
{
	/** RTCが保持する最大の時間を秒数に変換した値 */
	enum{
		RTC_MAX_SECOND = 3155759999,
	};

	RTCDate date = { 99, 12, 31, 0};
	RTCTime time = { 23, 59, 59 };
	s64 MaxSec = RTC_ConvertDateTimeToSecond(&date, &time);
	GF_ASSERT(MaxSec == RTC_MAX_SECOND);

	if (start_sec < end_sec) {
		return end_sec - start_sec;
	}
	return end_sec + (RTC_MAX_SECOND - start_sec);
}

