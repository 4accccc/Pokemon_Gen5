//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_sunny.h
 *	@brief		天気　晴れ
 *	@author		tomoya takahashi
 *	@date		2009.03.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __WEATHER_SUNNY_H__
#define __WEATHER_SUNNY_H__

#ifdef _cplusplus
extern "C"{
#endif

#include "weather_task.h"

//-----------------------------------------------------------------------------
/**
 *			天気情報
 */
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SUNNY;
FS_EXTERN_OVERLAY(field_weather_sunny);

#ifdef _cplusplus
}	// extern "C"{
#endif

#endif		// __WEATHER_SUNNY_H__


