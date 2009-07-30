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

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================

//======================================================================
//	extern
//======================================================================

// 初期化
extern void FIELD_PLAYER_NOGRID_Rail_SetUp( FIELD_PLAYER *fld_player, RAIL_KEY way, VecFx32* p_playerway );

// 動作
// p_playerway	の値は、関数内部で、キーの方向に書き換わります。
extern void FIELD_PLAYER_NOGRID_Rail_Move( FIELD_PLAYER *fld_player, FLDEFF_CTRL *fectrl, VecFx32* p_playerway, int key, const FIELD_CAMERA* cp_camera );

extern void FIELD_PLAYER_NOGRID_Free_Move( FIELD_PLAYER *fld_player, int key, fx32 onedist );

extern void FIELD_PLAYER_C3_Move(
		FIELD_PLAYER *fld_player, int key, u16 angle );


