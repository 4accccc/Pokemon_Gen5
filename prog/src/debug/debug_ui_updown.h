//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_ui_updown.h
 *	@brief  上下移動　テーブル
 *	@author	tomoya takahashi
 *	@date		2010.04.29
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>

FS_EXTERN_OVERLAY(debug_ui_updown);

#define  DEBUG_UI_UPDOWN_DATA_MAX ( 64 )
extern const GFL_UI_DEBUG_OVERWRITE c_DEBUG_UI_UPDOWN[DEBUG_UI_UPDOWN_DATA_MAX];

#ifdef _cplusplus
}	// extern "C"{
#endif



