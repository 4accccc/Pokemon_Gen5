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

//---------------------------------------------------------------------------
//		プレイ時間の最大値の定義
//---------------------------------------------------------------------------
#define	PTIME_HOUR_MAX		(999)			///<プレイ時間（時）の最大値
#define	PTIME_MINUTE_MAX	(59)			///<プレイ時間（分）の最大値
#define	PTIME_SECOND_MAX	(59)			///<プレイ時間（秒）の最大値

//============================================================================================
//============================================================================================

extern PLAYTIME * SaveData_GetPlayTime(SAVE_CONTROL_WORK * sv);
extern void PLAYTIME_Init(PLAYTIME * coin);
extern void PLAYTIME_CountUp(PLAYTIME * ptime, u32 value);
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

#endif	//__PLAYTIME_H__
