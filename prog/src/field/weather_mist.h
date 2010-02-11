//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_mist.h
 *	@brief  天気：霧
 *	@author	tomoya takahashi
 *	@date		2010.02.11
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "weather_task.h"

FS_EXTERN_OVERLAY(field_weather_mist);

//-----------------------------------------------------------------------------
/**
 *		霧
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_MIST;


#ifdef _cplusplus
}	// extern "C"{
#endif



