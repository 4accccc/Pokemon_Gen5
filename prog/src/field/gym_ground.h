//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gym_ground.h
 *	@brief  地面ジム
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.05.11
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
extern void GYM_GROUND_Setup(FIELDMAP_WORK *p_fieldmap);
extern void GYM_GROUND_End(FIELDMAP_WORK *p_fieldmap);
extern void GYM_GROUND_Move(FIELDMAP_WORK *p_fieldmap);

// ベルトコンベア イベント
extern GMEVENT * GYM_GROUND_CheckBeltEvent( GAMESYS_WORK *p_gamesys );

// スクリプトコマンド用
extern GMEVENT *GYM_GROUND_CreateLiftMoveEvent( GAMESYS_WORK *p_gamesys, const int lift_id, const int way );
extern GMEVENT *GYM_GROUND_CreateExitLiftMoveEvent( GAMESYS_WORK *p_gamesys, const BOOL exit );
extern void GYM_GROUND_SetEnterPos( GAMESYS_WORK *p_gamesys );


