//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_snow.h
 *	@brief		ìVãC	ê·
 *	@author		tomoya takahashi
 *	@date		2009.03.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __WEATHER_SNOW_H__
#define __WEATHER_SNOW_H__

#ifdef _cplusplus
extern "C"{
#endif

#include "weather_task.h"


FS_EXTERN_OVERLAY(field_weather_snow);

//-----------------------------------------------------------------------------
/**
 *		í èÌÇÃê·
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SNOW;


//-----------------------------------------------------------------------------
/**
 *		êÅê·
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SNOWSTORM;

//-----------------------------------------------------------------------------
/**
 *		Ç†ÇÁÇÍ
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_ARARE;


#ifdef _cplusplus
}	// extern "C"{
#endif

#endif		// __WEATHER_SNOW_H__


