//=============================================================================
/**
 * @file	location.h	
 * @brief	フィールドマップシステム
 * @author	tamada GAME FREAK inc.
 * @date	2005.12.16
 *
 * 2008.11.21	DPプロジェクトから移植
 */
//=============================================================================

#pragma once

#include "field/rail_location.h"

//=============================================================================
//=============================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  LOCATION_DEFAULT_EXIT_OFS = 0,
};
typedef u16 LOC_EXIT_OFS;
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef enum {
	LOCATION_POS_TYPE_3D = 0, // 3D
	LOCATION_POS_TYPE_RAIL,   // RAIL

	LOCATION_POS_TYPE_MAX,
}LOCATION_POS_TYPE;

//--------------------------------------------------------------
// ロケーションポジション
// ３D座標とレールロケーションの情報を扱います。
//--------------------------------------------------------------
typedef struct 
{
  LOCATION_POS_TYPE type;
  union{
    //  3Dデータ
    VecFx32 pos;
    // レールデータ
    RAIL_LOCATION railpos;
  };
} LOCATION_POS;



//--------------------------------------------------------------
//--------------------------------------------------------------
typedef enum {
	LOCATION_TYPE_INIT = 0,
	LOCATION_TYPE_DIRECT,
	LOCATION_TYPE_EXITID,
	LOCATION_TYPE_SPID,
}LOCATION_TYPE;
//--------------------------------------------------------------
///	LOCATION構造体
//--------------------------------------------------------------
typedef struct {
	LOCATION_TYPE type;
	s16 zone_id;
	s16 exit_id;
	s16 dir_id;
  LOC_EXIT_OFS exit_ofs;
	LOCATION_POS location_pos;
}LOCATION;

//=============================================================================
//=============================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
#ifdef PM_DEBUG
extern void LOCATION_DEBUG_SetDefaultPos(LOCATION * loc, u16 zone_id);
#endif

//--------------------------------------------------------------
/**
 * @brief	ゲーム開始時の位置取得
 */
//--------------------------------------------------------------
extern void LOCATION_SetGameStart(LOCATION * loc);

//--------------------------------------------------------------
/**
 * @brief	LOCATIONの初期化
 */
//--------------------------------------------------------------
extern void LOCATION_Init(LOCATION * loc);

//--------------------------------------------------------------
/**
 * @brief	LOCATIONのセット
 */
//--------------------------------------------------------------
extern void LOCATION_Set(LOCATION * loc, int zone, s16 door, s16 dir, LOC_EXIT_OFS ofs, fx32 x, fx32 y, fx32 z);
extern void LOCATION_SetRail(LOCATION * loc, int zone, s16 door, s16 dir, LOC_EXIT_OFS ofs, u16 rail_index, u16 line_grid, s16 width_grid);

//--------------------------------------------------------------
/**
 * @brief	LOCATIONのセット（zone_id,exit_idのみ)
 */
//--------------------------------------------------------------
extern void LOCATION_SetID(LOCATION * loc, u16 zone_id, u16 exit_id, u16 ofs);
//--------------------------------------------------------------
/**
 * @brief	LOCATIONのセット（直接位置指定）
 */
//--------------------------------------------------------------
extern void LOCATION_SetDirect(LOCATION * loc, int zone, s16 dir, fx32 x, fx32 y, fx32 z);
extern void LOCATION_SetDirectRail(LOCATION * loc, int zone, s16 dir, u16 rail_index, u16 line_grid, s16 width_grid);

//--------------------------------------------------------------
/**
 * @brief	LOCATION内の座標タイプを取得
 */
//--------------------------------------------------------------
extern LOCATION_POS_TYPE LOCATION_GetPosType( const LOCATION * loc );

//--------------------------------------------------------------
/**
 * @brief	LOCATIONから、３D座標の設定・取得
 */
//--------------------------------------------------------------
extern void LOCATION_Set3DPos( LOCATION * loc, const VecFx32 * pos );
extern void LOCATION_Get3DPos( const LOCATION * loc, VecFx32 * pos );

//--------------------------------------------------------------
/**
 * @brief	LOCATIONから、レールロケーションの設定・取得
 */
//--------------------------------------------------------------
extern void LOCATION_SetRailLocation( LOCATION * loc, const RAIL_LOCATION * location );
extern void LOCATION_GetRailLocation( const LOCATION * loc, RAIL_LOCATION * location );



