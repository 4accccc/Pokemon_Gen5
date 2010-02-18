//============================================================================================
/**
 * @brief	playtime.c
 * @brief	プレイ時間操作
 * @date	2006.03.09
 * @author	tamada
 */
//============================================================================================
#include <gflib.h>
#include "savedata/playtime.h"
#include "playtime_local.h"

enum {
	PLAYTIME_HOUR_MAX = 999,
	PLAYTIME_MINUTE_MAX = 59,
	PLAYTIME_SECOND_MAX = 59,
};
//============================================================================================
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	プレイ時間の初期化処理
 * @param	ptime	プレイ時間へのポインタ
 */
//---------------------------------------------------------------------------
void PLAYTIME_Init(PLAYTIME * ptime)
{
	ptime->hour = 0;
	ptime->minute = 0;
	ptime->second = 0;

	ptime->save_year = 0;
	ptime->save_month = 0;
	ptime->save_day = 0;
	ptime->save_hour = 0;
	ptime->save_minute = 0;
}

//---------------------------------------------------------------------------
/**
 * @brief	プレイ時間のセット
 * @param	ptime	プレイ時間へのポインタ
 * @param	hour	時間
 * @param	min		分
 * @param	sec		秒
 *
 * 指定したプレイ時間にセットする処理。
 * デバッグでしか使用しない？
 */
//---------------------------------------------------------------------------
void PLAYTIME_Set(PLAYTIME * ptime, u32 hour, u32 min, u32 sec)
{
	GF_ASSERT(hour <= PLAYTIME_HOUR_MAX); 
	GF_ASSERT(min <= PLAYTIME_MINUTE_MAX); 
	GF_ASSERT(sec <= PLAYTIME_SECOND_MAX); 
	ptime->hour = hour;
	ptime->minute = min;
	ptime->second = sec;
}

//---------------------------------------------------------------------------
/**
 * @brief
 * @param	ptime	プレイ時間へのポインタ
 * @return
 */
//---------------------------------------------------------------------------
void PLAYTIME_CountUp(PLAYTIME * ptime, u32 passed_sec)
{
	u32 hour, min, sec;

	if( (ptime->hour == PLAYTIME_HOUR_MAX)
	&&	(ptime->minute == PLAYTIME_MINUTE_MAX)
	&&	(ptime->second == PLAYTIME_SECOND_MAX)
	){
		return;
	}

	hour = ptime->hour;
	min = ptime->minute;
	sec = ptime->second + passed_sec;
	if( sec > PLAYTIME_SECOND_MAX )
	{
		min = ptime->minute + (sec / (PLAYTIME_SECOND_MAX+1));
		sec %= (PLAYTIME_SECOND_MAX+1);
		if( min > PLAYTIME_MINUTE_MAX )
		{
			hour = ptime->hour + (min / (PLAYTIME_MINUTE_MAX+1));
			min %= (PLAYTIME_MINUTE_MAX+1);
			if( hour >= PLAYTIME_HOUR_MAX )
			{
				hour = PLAYTIME_HOUR_MAX;
				min = PLAYTIME_MINUTE_MAX;
				sec = PLAYTIME_SECOND_MAX;
			}
		}
	}

	ptime->hour = hour;
	ptime->minute = min;
	ptime->second = sec;
}

//---------------------------------------------------------------------------
/**
 * @brief
 * @param	ptime	プレイ時間へのポインタ
 * @return
 */
//---------------------------------------------------------------------------
u16 PLAYTIME_GetHour(const PLAYTIME * ptime)
{
	return ptime->hour;
}

//---------------------------------------------------------------------------
/**
 * @brief
 * @param	ptime	プレイ時間へのポインタ
 * @return
 */
//---------------------------------------------------------------------------
u8 PLAYTIME_GetMinute(const PLAYTIME * ptime)
{
	return ptime->minute;
}

//---------------------------------------------------------------------------
/**
 * @brief
 * @param	ptime	プレイ時間へのポインタ
 * @return
 */
//---------------------------------------------------------------------------
u8 PLAYTIME_GetSecond(const PLAYTIME * ptime)
{
	return ptime->second;
}


//---------------------------------------------------------------------------
/**
 * @brief		セーブした日付と時間を設定
 * @param		ptime		プレイ時間へのポインタ
 * @return	none
 */
//---------------------------------------------------------------------------
void PLAYTIME_SetSaveTime( PLAYTIME * ptime )
{
	RTCDate	date;
	RTCTime	time;

	RTC_GetDate( &date );
	RTC_GetTime( &time );

	ptime->save_year = date.year;
	ptime->save_month = date.month;
	ptime->save_day = date.day;
	ptime->save_hour = time.hour;
	ptime->save_minute = time.minute;
}

//---------------------------------------------------------------------------
/**
 * @brief		セーブした年を取得
 * @param		ptime		プレイ時間へのポインタ
 * @return	年
 */
//---------------------------------------------------------------------------
u32 PLAYTIME_GetSaveYear( const PLAYTIME * ptime )
{
	return ptime->save_year;
}

//---------------------------------------------------------------------------
/**
 * @brief		セーブした月を取得
 * @param		ptime		プレイ時間へのポインタ
 * @return	月
 */
//---------------------------------------------------------------------------
u32 PLAYTIME_GetSaveMonth( const PLAYTIME * ptime )
{
	return ptime->save_month;
}

//---------------------------------------------------------------------------
/**
 * @brief		セーブした日を取得
 * @param		ptime		プレイ時間へのポインタ
 * @return	日
 */
//---------------------------------------------------------------------------
u32 PLAYTIME_GetSaveDay( const PLAYTIME * ptime )
{
	return ptime->save_day;
}

//---------------------------------------------------------------------------
/**
 * @brief		セーブした時を取得
 * @param		ptime		プレイ時間へのポインタ
 * @return	時
 */
//---------------------------------------------------------------------------
u32 PLAYTIME_GetSaveHour( const PLAYTIME * ptime )
{
	return ptime->save_hour;
}

//---------------------------------------------------------------------------
/**
 * @brief		セーブした分を取得
 * @param		ptime		プレイ時間へのポインタ
 * @return	分
 */
//---------------------------------------------------------------------------
u32 PLAYTIME_GetSaveMinute( const PLAYTIME * ptime )
{
	return ptime->save_minute;
}
