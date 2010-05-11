//============================================================================================
/**
 * @brief	playtime.h
 * @brief	プレイ時間
 * @date	2006.03.09
 * @author	tamada
 */
//============================================================================================

#ifndef	__PLAYTIME_H__
#define	__PLAYTIME_H__

#include "savedata/save_control.h"

//============================================================================================
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	プレイ時間の不完全型定義
 */
//---------------------------------------------------------------------------
typedef struct PLAYTIME PLAYTIME;

///セーブした日時
typedef struct{
	u32	save_year:7;			// セーブ時の日付（年）
	u32	save_month:4;			// セーブ時の日付（月）
	u32	save_day:5;				// セーブ時の日付（日）
	u32	save_hour:5;			// セーブ時の時間（時）
	u32	save_minute:6;		// セーブ時の時間（分）
	u32	dmy:5;						// あまり
}PLAYTIME_SAVE_DATE;

//---------------------------------------------------------------------------
//		プレイ時間の最大値の定義
//---------------------------------------------------------------------------
#define	PTIME_HOUR_MAX		(999)			///<プレイ時間（時）の最大値
#define	PTIME_MINUTE_MAX	(59)			///<プレイ時間（分）の最大値
#define	PTIME_SECOND_MAX	(59)			///<プレイ時間（秒）の最大値

//============================================================================================
//============================================================================================

extern PLAYTIME * SaveData_GetPlayTime(SAVE_CONTROL_WORK * sv);

extern PLAYTIME * PLAYTIME_Allock( HEAPID heapID );
extern void PLAYTIME_Init(PLAYTIME * coin);
extern void PLAYTIME_CountUp(PLAYTIME * ptime, u32 value);
extern void PLAYTIME_SetAllData( PLAYTIME * ptime, const PLAYTIME * data );
extern u16 PLAYTIME_GetHour(const PLAYTIME * ptime);
extern u8 PLAYTIME_GetMinute(const PLAYTIME * ptime);
extern u8 PLAYTIME_GetSecond(const PLAYTIME * ptime);

extern void PLAYTIME_Set(PLAYTIME * ptime, u32 hour, u32 min, u32 sec);

extern void PLAYTIME_SetSaveTime( PLAYTIME * ptime );
extern u32 PLAYTIME_GetSaveYear( const PLAYTIME * ptime );
extern u32 PLAYTIME_GetSaveMonth( const PLAYTIME * ptime );
extern u32 PLAYTIME_GetSaveDay( const PLAYTIME * ptime );
extern u32 PLAYTIME_GetSaveHour( const PLAYTIME * ptime );
extern u32 PLAYTIME_GetSaveMinute( const PLAYTIME * ptime );
extern void PLAYTIME_GetSaveDate( const PLAYTIME * ptime, PLAYTIME_SAVE_DATE *dest );
extern void PLAYTIME_SetSaveDate( PLAYTIME * ptime, const PLAYTIME_SAVE_DATE *src );

#endif	//__PLAYTIME_H__
