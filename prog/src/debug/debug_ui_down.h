//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_ui_down.h
 *	@brief
 *	@author	
 *	@date		2010.04.29
 *
 *	ÉÇÉWÉÖÅ[ÉãñºÅF
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


#include <gflib.h>

FS_EXTERN_OVERLAY(debug_ui_down);

#define  DEBUG_UI_DOWN_DATA_MAX ( 4 )
extern const GFL_UI_DEBUG_OVERWRITE c_DEBUG_UI_DOWN[DEBUG_UI_DOWN_DATA_MAX];

#ifdef _cplusplus
}	// extern "C"{
#endif



