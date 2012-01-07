//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_ui_up.h
 *	@brief
 *	@author	
 *	@date		2012.01.07
 *
 *	ÉÇÉWÉÖÅ[ÉãñºÅF
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


#include <gflib.h>

FS_EXTERN_OVERLAY(debug_ui_up);

#define  DEBUG_UI_UP_DATA_MAX ( 4 )
extern const GFL_UI_DEBUG_OVERWRITE c_DEBUG_UI_UP[DEBUG_UI_UP_DATA_MAX];

#ifdef _cplusplus
}	// extern "C"{
#endif



