//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_fourkings.h
 *	@brief  四天王部屋　演出
 *	@author	tomoya takahashi
 *	@date		2009.11.21
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "gflib.h"
#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
#define FIELD_EVENT_FOURKINGS_MAX (4)

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


// 歩いてあがる演出イベント
extern GMEVENT* EVENT_FourKings_CircleWalk( 
     GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap, u32 fourkins_no );


//  頂上カメラIDのテーブル
extern u32 EVENT_FourKings_GetCameraID( u32 fourkins_no );

// ゴースト部屋の雷音管理システム発動
extern void FIELDMAPFUNC_FourkingsGhostSparkSound( FIELDMAP_WORK* p_fieldmap );

#ifdef _cplusplus
}	// extern "C"{
#endif



