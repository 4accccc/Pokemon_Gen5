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
#include "field/fieldmap_func.h"

FS_EXTERN_OVERLAY(field_event_fourkings);

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

// 四天王部屋管理タスク発動
extern const FLDMAPFUNC_DATA c_FLDMAPFUNC_GHOST_SPARK_SOUND;
extern const FLDMAPFUNC_DATA c_FLDMAPFUNC_ESPERT_SOUND;
extern const FLDMAPFUNC_DATA c_FLDMAPFUNC_BAD_SOUND;
extern const FLDMAPFUNC_DATA c_FLDMAPFUNC_FIGHT_SOUND;

#ifdef _cplusplus
}	// extern "C"{
#endif



