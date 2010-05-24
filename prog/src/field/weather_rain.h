//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_rain.h
 *	@brief		天気雨
 *	@author		tomoya takahashi
 *	@date		2009.03.27
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __WEATHER_RAIN_H__
#define __WEATHER_RAIN_H__

#ifdef _cplusplus
extern "C"{
#endif

#include "weather_task.h"

FS_EXTERN_OVERLAY(field_weather_rain);

//-----------------------------------------------------------------------------
/**
 *		普通の雨
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_RAIN;


//-----------------------------------------------------------------------------
/**
 *		移動ポケモン
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_RAIKAMI;
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_KAZAKAMI;

#ifdef _cplusplus
}	// extern "C"{
#endif

#endif		// __WEATHER_RAIN_H__


