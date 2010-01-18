//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		shinka_demo_graphic.h
 *	@brief	グラフィック設定
 *	@author	Koji Kawada
 *	@date		2010.01.13
 *	@note   ui_template_graphic.hをコピペして名前を変更しただけです。
 *
 *	モジュール名：SHINKADEMO_GRAPHIC
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
typedef struct _SHINKADEMO_GRAPHIC_WORK  SHINKADEMO_GRAPHIC_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///初期化
//	ディスプレイセレクトにはGX_DISP_SELECT_MAIN_SUB or GX_DISP_SELECT_SUB_MAINを
//	渡してください。途中で書き換えたい場合、GFL_DISP_SetDispSelectを使用して下さい。
//  サブBGの初期化は行わない。
//=====================================
extern SHINKADEMO_GRAPHIC_WORK * SHINKADEMO_GRAPHIC_Init( int display_select, HEAPID heapID );
//-------------------------------------
///破棄
//  サブBGの破棄は行わない。
//=====================================
extern void SHINKADEMO_GRAPHIC_Exit( SHINKADEMO_GRAPHIC_WORK *p_wk );
//-------------------------------------
///２D描画
//=====================================
extern void SHINKADEMO_GRAPHIC_2D_Draw( SHINKADEMO_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画	３Dを使わない場合、読んでも何もしません
//=====================================
extern void SHINKADEMO_GRAPHIC_3D_StartDraw( SHINKADEMO_GRAPHIC_WORK *p_wk );
extern void SHINKADEMO_GRAPHIC_3D_EndDraw( SHINKADEMO_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得	内部で設定したCLUNITを返します	OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * SHINKADEMO_GRAPHIC_GetClunit( const SHINKADEMO_GRAPHIC_WORK *cp_wk );

//-------------------------------------
///情報取得	内部で設定したカメラを返します	3D使わない場合NULL
//=====================================
extern GFL_G3D_CAMERA * SHINKADEMO_GRAPHIC_GetCamera( const SHINKADEMO_GRAPHIC_WORK *cp_wk );


//-------------------------------------
/// サブBGの初期化
//=====================================
extern void SHINKADEMO_GRAPHIC_InitBGSub( SHINKADEMO_GRAPHIC_WORK* p_wk );
//-------------------------------------
/// サブBGの破棄
//=====================================
extern void SHINKADEMO_GRAPHIC_ExitBGSub( SHINKADEMO_GRAPHIC_WORK* p_wk );
