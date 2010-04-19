//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_camera_debug.h
 *	@brief  フィールドカメラ　デバック機能
 *	@author	tomoya takahashi
 *	@date		2010.04.19
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include "field_camera.h"
#include "test/camera_adjust_view.h"

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

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

#ifdef  PM_DEBUG
//------------------------------------------------------------------
//  デバッグ用：下画面操作とのバインド
//------------------------------------------------------------------
extern void FIELD_CAMERA_DEBUG_BindSubScreen(FIELD_CAMERA * camera, void * param, FIELD_CAMERA_DEBUG_BIND_TYPE type, HEAPID heapID );
extern void FIELD_CAMERA_DEBUG_ReleaseSubScreen(FIELD_CAMERA * camera);
extern void FIELD_CAMERA_DEBUG_GetBindSubScreenTarget( const FIELD_CAMERA * camera, VecFx32* p_target );



//------------------------------------------------------------------
//  カメラ　デバック表示関数
//------------------------------------------------------------------
extern void FIELD_CAMERA_DEBUG_Draw( const FIELD_CAMERA* camera);




//------------------------------------------------------------------
//  デバッグカメラ詳細操作
/*
 *  NONE：ターゲット、カメラ平行移動
 *	B：カメラ公転（ターゲット座標が変わる）
 *	Y：カメラ自転（カメラ座標が変わる）
 *	A：パース操作
 *	X：ターゲットカメラバインドのON・OFF
 *	
 *	START：バッファリングモード変更
 *
 *	戻り値　TRUE  変更あり    FALSE　変更なし
 */
//------------------------------------------------------------------
extern void FIELD_CAMERA_DEBUG_InitControl( FIELD_CAMERA* camera, HEAPID heapID );
extern void FIELD_CAMERA_DEBUG_ExitControl( FIELD_CAMERA* camera );
extern BOOL FIELD_CAMERA_DEBUG_Control( FIELD_CAMERA* camera, int trg, int cont, int repeat );
extern void FIELD_CAMERA_DEBUG_DrawInfo( FIELD_CAMERA* camera, GFL_BMPWIN* p_win, fx32 map_size_x, fx32 map_size_z );
extern void FIELD_CAMERA_DEBUG_DrawControlHelp( FIELD_CAMERA* camera, GFL_BMPWIN* p_win );

#endif

#ifdef _cplusplus
}	// extern "C"{
#endif



