//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_rain.h
 *	@brief		ìVãCâJ
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
 *		ïÅí ÇÃâJ
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_RAIN;

//-----------------------------------------------------------------------------
/**
 *		óãâJ
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_SPARKRAIN;


//-----------------------------------------------------------------------------
/**
 *		ó[óß
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_EVENING_RAIN;


//-----------------------------------------------------------------------------
/**
 *		à⁄ìÆÉ|ÉPÉÇÉì
*/
//-----------------------------------------------------------------------------
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_RAIKAMI;
extern WEATHER_TASK_DATA c_WEATHER_TASK_DATA_KAZAKAMI;

#ifdef _cplusplus
}	// extern "C"{
#endif

#endif		// __WEATHER_RAIN_H__


