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

// 動作
// cp_playerway	主人公が最後に移動した方向を渡してください・
// まだ動いていない場合には、距離０のベクトルを渡してください。
extern void FIELD_PLAYER_NOGRID_Rail_Move( FIELD_PLAYER *fld_player, FLDEFF_CTRL *fectrl, const VecFx32* cp_playerway, int key, const FIELD_CAMERA* cp_camera );

extern void FIELD_PLAYER_NOGRID_Free_Move( FIELD_PLAYER *fld_player, int key, fx32 onedist );

extern void FIELD_PLAYER_C3_Move(
		FIELD_PLAYER *fld_player, int key, u16 angle );
