//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_season_time.h
 *	@brief  季節による　時間帯変化
 *	@author	tomoya takahashi
 *	@date		2010.01.28
 *
 *	モジュール名：FLD_SEASON_TIME
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>

#include "system/timezone.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	季節による時間帯変化管理ワーク
//=====================================
typedef struct _FLD_SEASON_TIME FLD_SEASON_TIME;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 生成・破棄
extern FLD_SEASON_TIME* FLD_SEASON_TIME_Create( u8 season_id, HEAPID heapID );
extern void FLD_SEASON_TIME_Delete( FLD_SEASON_TIME* p_sys );


// タイムゾーン取得
extern TIMEZONE FLD_SEASON_TIME_GetTimeZone( const FLD_SEASON_TIME* cp_sys );

// 今の時間からタイムゾーン切り替え時間までの差(秒数)
extern int FLD_SEASON_TIME_GetTimeZoneChangeTime( const FLD_SEASON_TIME* cp_sys, TIMEZONE timezone );



#ifdef _cplusplus
}	// extern "C"{
#endif



