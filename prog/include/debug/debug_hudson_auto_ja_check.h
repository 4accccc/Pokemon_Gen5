//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_hudson_auto_ja_check.h
 *	@brief  JA店舗生成オートデバッグ
 *	@author	Toru=Nagihashi
 *	@date		2011.12.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "debug/debug_hudson.h"

#ifdef DEBUG_ONLY_FOR_hudson
FS_EXTERN_OVERLAY( debug_resort_check );

//=============================================================================
/**
 *					定数
*/
//=============================================================================
enum
{
  DEBUG_RESORT_SHOP_CHECK_UI_START,
  DEBUG_RESORT_SHOP_CHECK_UI_END,
};

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern const HUDSON_AUTOTEST_WORK HUDSON_AUTOTEST_ResortShopCheck;

#endif // DEBUG_ONLY_FOR_hudson
