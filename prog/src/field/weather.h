//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather.h
 *	@brief		天気システム
 *	@author		tomoya takahashi
 *	@date		2009.03.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef _cplusplus
extern "C"{
#endif

#ifndef __WEATHER_H__
#define __WEATHER_H__

#include "field/weather_no.h"

#include "field_light.h"
#include "field_fog.h"
#include "field_zonefog.h"
#include "field_sound.h"
#include "field_season_time.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	天気情報なし
//=====================================
#define FIELD_WEATHER_NO_NONE	( 0xffff )

//-------------------------------------
///	天気セルユニット優先順位
//=====================================
#define FIELD_WEATHER_CLUNIT_PRI	( 8 )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	天気システムワーク
//=====================================
typedef struct _FIELD_WEATHER FIELD_WEATHER;



//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	天気システム	初期化破棄
//=====================================
extern FIELD_WEATHER* FIELD_WEATHER_Init( const FIELD_CAMERA* cp_camera, FIELD_LIGHT* p_light, FIELD_FOG_WORK* p_fog, const FIELD_ZONEFOGLIGHT* cp_zonefog, const FIELD_SOUND* cp_sound, const FLD_SEASON_TIME* cp_season_time, HEAPID heapID );
extern void FIELD_WEATHER_Exit( FIELD_WEATHER* p_sys );
extern void FIELD_WEATHER_Main( FIELD_WEATHER* p_sys, HEAPID heapID );
extern void FIELD_WEATHER_3DWrite( FIELD_WEATHER* p_sys );

//-------------------------------------
///	天気システム	管理
//=====================================
extern void FIELD_WEATHER_Set( FIELD_WEATHER* p_sys, WEATHER_NO weather_no, HEAPID heapID );
extern void FIELD_WEATHER_Change( FIELD_WEATHER* p_sys, WEATHER_NO weather_no );
extern WEATHER_NO FIELD_WEATHER_GetWeatherNo( const FIELD_WEATHER* cp_sys );
extern WEATHER_NO FIELD_WEATHER_GetNextWeatherNo( const FIELD_WEATHER* cp_sys );


#endif		// __WEATHER_H__

#ifdef _cplusplus
}
#endif

