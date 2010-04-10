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

#include "timezone.cdat"


//============================================================================================
//
//		ツール関数
//
//============================================================================================

//--------------------------------------------------------------
/**
 * @brief	時間帯の取得
 * @return	int	時間帯（timezone.hを参照）
 */
//--------------------------------------------------------------
int PM_RTC_GetTimeZone(int season)
{
	RTCTime time;
	GFL_RTC_GetTime(&time);
	return PM_RTC_ConvertHourToTimeZone(season, time.hour);
}

//--------------------------------------------------------------
/**
 * @brief	時間から時間帯への変換
 * @param	season 季節 gamesystem/pm_season.h参照
 * @param	hour	時間（０－２３）
 * @return	int		時間帯（timezone.hを参照）
 */
//--------------------------------------------------------------
int PM_RTC_ConvertHourToTimeZone(int season, int hour)
{
  GF_ASSERT( season < PMSEASON_TOTAL );
	GF_ASSERT(0 <= hour && hour < 24);
	return sc_SEASON_TIMEZONE[season][hour];
}

//----------------------------------------------------------------------------
/**
 *	@brief  季節の時間帯変更　時間を取得
 *
 *	@param	season      季節
 *	@param	timezone    タイムゾーン
 *
 *	@return 変わる時間
 */
//-----------------------------------------------------------------------------
int PM_RTC_GetTimeZoneChangeHour( int season, TIMEZONE timezone )
{
  GF_ASSERT( season < PMSEASON_TOTAL );
  GF_ASSERT( timezone < TIMEZONE_MAX );

  return sc_SEASON_TIMEZONE_CHANGE[ season ][ timezone ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  季節の時間帯が変わるまでの時間を取得
 *
 *	@param	season    季節
 *	@param	timezone  タイムゾーン
 *
 *	@return 変わるまでの時間（-12時間～11時間）
 */
//-----------------------------------------------------------------------------
int PM_RTC_GetTimeZoneChangeMargin( int season, TIMEZONE timezone )
{
	RTCTime time;
  int change_time;
  int time_second;
  int sub_second;
  
  GF_ASSERT( season < PMSEASON_TOTAL );
  GF_ASSERT( timezone < TIMEZONE_MAX );
  
  GFL_RTC_GetTime( &time );

  change_time = sc_SEASON_TIMEZONE_CHANGE[season][timezone];
  time_second = (time.hour * 60 * 60) + (time.minute * 60) + time.second;
  sub_second = (change_time*60*60) - time_second;
  if( sub_second > (12*60*60) )
  {
    sub_second = (24*60*60) - sub_second;
  }
  return sub_second;
}

//--------------------------------------------------------------
/**
 * @brief	夜かどうかの判定
 * @retval	TRUE	今は夜
 * @retval	FALSE	今は昼
 */
//--------------------------------------------------------------
BOOL PM_RTC_IsNightTime( int season )
{
	switch ( PM_RTC_GetTimeZone(season) ) {
	case TIMEZONE_MIDNIGHT:
	case TIMEZONE_NIGHT:
		return TRUE;
	}
	return FALSE;
}


#if 0
//--------------------------------------------------------------
/**
 * @brief	時間から時間帯への変換
 * @param	hour	時間（０－２３）
 * @return	int		時間帯（timezone.hを参照）
 */
//--------------------------------------------------------------
int GFL_RTC_ConvertHourToTimeZone(int hour)
{
  return PM_RTC_ConvertHourToTimeZone( PM_RTC_TIMEZONE_DEFAULT_SEASON, hour );
}
#endif


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

