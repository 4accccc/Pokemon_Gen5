//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_util.h
 *	@brief  ローカルライブキャスター汎用モジュール
 *	@author	Toru=Nagihashi
 *	@date		2011.01.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "gamesystem/game_data.h"
//=============================================================================
/**
 *					タイムゾーン
*/
//=============================================================================
typedef enum
{
  LTTZ_NOON,
  LTTZ_EVENING,
  LTTZ_NIGHT,
}LOCAL_TVT_TIME_ZONE;

extern LOCAL_TVT_TIME_ZONE LOCAL_TVT_UTIL_GetTimeZone( const GAMEDATA *cp_gamedata );

//=============================================================================
/**
 *					背景素材
*/
//=============================================================================
extern void LOCAL_TVT_UTIL_LoadBg( GAMEDATA *p_gamedata, HEAPID heapID );
