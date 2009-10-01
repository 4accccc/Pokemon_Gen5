//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		graphic.h
 *	@brief	グラフィック設定
 *	@author	Toru=Nagihashi
 *	@date		2009.09.30
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include <gflib.h>
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
typedef struct _UI_TEMPLATE_GRAPHIC_WORK  UI_TEMPLATE_GRAPHIC_WORK;

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
extern UI_TEMPLATE_GRAPHIC_WORK * UI_TEMPLATE_GRAPHIC_Init( int display_select, HEAPID heapID );
//-------------------------------------
///破棄
//=====================================
extern void UI_TEMPLATE_GRAPHIC_Exit( UI_TEMPLATE_GRAPHIC_WORK *p_wk );
//-------------------------------------
///２D描画
//=====================================
extern void UI_TEMPLATE_GRAPHIC_2D_Draw( UI_TEMPLATE_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画	３Dを使わない場合、読んでも何もしません
//=====================================
extern void UI_TEMPLATE_GRAPHIC_3D_StartDraw( UI_TEMPLATE_GRAPHIC_WORK *p_wk );
extern void UI_TEMPLATE_GRAPHIC_3D_EndDraw( UI_TEMPLATE_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得	内部で設定したCLUNITを返します	OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * UI_TEMPLATE_GRAPHIC_GetClunit( const UI_TEMPLATE_GRAPHIC_WORK *cp_wk );

//-------------------------------------
///情報取得	内部で設定したカメラを返します	3D使わない場合NULL
//=====================================
extern GFL_G3D_CAMERA * UI_TEMPLATE_GRAPHIC_GetCamera( const UI_TEMPLATE_GRAPHIC_WORK *cp_wk );
