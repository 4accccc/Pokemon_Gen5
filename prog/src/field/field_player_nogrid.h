//======================================================================
/**
 * @file	field_player_nogrid.h
 * @brief	ノングリッド移動　フィールドプレイヤー制御
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "field_player.h"
#include "field_camera.h"
#include "fldmmdl.h"
#include "field_effect.h"


#include "field_rail.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
//-------------------------------------
///	レール動作用補助ワーク
//=====================================
typedef struct 
{
  FIELD_RAIL_WORK* railwork;
  VecFx32          way;
} FIELD_PLAYER_NOGRID_WORK;

//======================================================================
//	extern
//======================================================================

// 初期化
extern void FIELD_PLAYER_NOGRID_Rail_SetUp( FIELD_PLAYER *fld_player, FIELD_PLAYER_NOGRID_WORK* p_wk );

// 動作
// p_playerway	の値は、関数内部で、キーの方向に書き換わります。
extern void FIELD_PLAYER_NOGRID_Rail_Move( FIELD_PLAYER *fld_player, FLDEFF_CTRL *fectrl, const FIELD_CAMERA* cp_camera, int key, FIELD_PLAYER_NOGRID_WORK* p_wk, const FLDNOGRID_MAPPER* cp_nogridMapper );

extern void FIELD_PLAYER_NOGRID_Free_Move( FIELD_PLAYER *fld_player, int key, fx32 onedist );

extern void FIELD_PLAYER_C3_Move(
		FIELD_PLAYER *fld_player, int key, u16 angle );


