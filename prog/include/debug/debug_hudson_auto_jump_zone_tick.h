//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_hudson_auto_jump_zone_tick.h
 *	@brief  マップジャンプ処理負荷チェック
 *	@author	Toru=Nagihashi
 *	@date		2011.01.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "debug/debug_hudson.h"

#ifdef DEBUG_ONLY_FOR_hudson
FS_EXTERN_OVERLAY( debug_jump_tick_check );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern const HUDSON_AUTOTEST_WORK HUDSON_AUTOTEST_JumpTickCheck;
#endif // DEBUG_ONLY_FOR_hudson
