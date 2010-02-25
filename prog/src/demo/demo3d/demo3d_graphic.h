//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		demo3d_graphic.h
 *	@brief	グラフィック設定
 *	@author	genya_hosaka
 *	@date		2009.11.27
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>

#include "demo/demo3d.h" // for DEMO3D_ID

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
//-------------------------------------
///	グラフィックワーク
//=====================================
typedef struct _DEMO3D_GRAPHIC_WORK  DEMO3D_GRAPHIC_WORK;

/*
 *  @brief  3Dシーンパラメータ指定構造体
 */
typedef struct _DEMO3D_3DSCENE_PARAM{
  u8  edge_marking_f:1;
  u8  fog_f:1;
  u8  dmy:6;

  GXRgb  edgeColorTbl[8]; 
}DEMO3D_3DSCENE_PARAM;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///初期化
//	ディスプレイセレクトにはGX_DISP_SELECT_MAIN_SUB or GX_DISP_SELECT_SUB_MAINを
//	渡してください。途中で書き換えたい場合、GFL_DISP_SetDispSelectを使用して下さい。
//=====================================
extern DEMO3D_GRAPHIC_WORK * DEMO3D_GRAPHIC_Init( int display_select, DEMO3D_ID demo_id, HEAPID heapID );
//-------------------------------------
///破棄
//=====================================
extern void DEMO3D_GRAPHIC_Exit( DEMO3D_GRAPHIC_WORK *p_wk );
//-------------------------------------
///２D描画
//=====================================
extern void DEMO3D_GRAPHIC_2D_Draw( DEMO3D_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画	３Dを使わない場合、読んでも何もしません
//=====================================
extern void DEMO3D_GRAPHIC_3D_StartDraw( DEMO3D_GRAPHIC_WORK *p_wk );
extern void DEMO3D_GRAPHIC_3D_EndDraw( DEMO3D_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得	内部で設定したCLUNITを返します	OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * DEMO3D_GRAPHIC_GetClunit( const DEMO3D_GRAPHIC_WORK *cp_wk );

//----------------------------------------------------------------------------
/**
 *	@brief	デモシーンの3Dパラメータ設定
 *
 *	@param	const GRAPHIC_WORK *cp_wk		ワーク
 */
//-----------------------------------------------------------------------------
extern void DEMO3D_GRAPHIC_Scene3DParamSet( DEMO3D_GRAPHIC_WORK *p_wk, const FIELD_LIGHT_STATUS* f_light, DEMO3D_3DSCENE_PARAM* prm );

