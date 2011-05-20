//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gym_ground_ent.h
 *	@brief  地面ジムエントランス
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.05.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern void GYM_GROUND_ENT_Setup(FIELDMAP_WORK *p_fieldmap);
extern void GYM_GROUND_ENT_End(FIELDMAP_WORK *p_fieldmap);
extern void GYM_GROUND_ENT_Move(FIELDMAP_WORK *p_fieldmap);

// スクリプトコマンド用
extern GMEVENT *GYM_GROUND_ENT_CreateExitLiftMoveEvt( GAMESYS_WORK *p_gamesys, const BOOL inExit );
extern void GYM_GROUND_ENT_SetEnterPos( GAMESYS_WORK *p_gamesys );

