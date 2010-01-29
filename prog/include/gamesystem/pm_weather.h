//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		pm_weather.h
 *	@brief  天気管理　移動ポケモン　ゾーン　カレンダー
 *	@author	tomoya takahashi
 *	@date		2010.01.29
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "game_data.h"


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// WeatherNoを取得
extern u32 GAMEDATA_GetWeatherNo( const GAMEDATA* cp_data );


// 現在の指定ZONEの天気を取得する
extern u8 PM_WEATHER_GetZoneChangeWeatherNo( GAMEDATA* p_data, int zone_id );

// ZoneChangeでの天気を求める
extern void PM_WEATHER_UpdateZoneChangeWeatherNo( GAMEDATA* p_data, int zone_id );

// セーブ復帰での天気を求める
extern void PM_WEATHER_UpdateSaveLoadWeatherNo( GAMEDATA* p_data, int zone_id );


#ifdef _cplusplus
}	// extern "C"{
#endif



