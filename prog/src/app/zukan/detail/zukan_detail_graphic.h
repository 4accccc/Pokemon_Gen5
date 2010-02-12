//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		zukan_detail_graphic.h
 *	@brief	グラフィック設定
 *	@author	Koji Kawada
 *	@date		2010.02.02
 *	@note   ui_template_graphic.hをコピペして名前を変更しただけです。
 *
 *	モジュール名：ZUKAN_DETAIL_GRAPHIC
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
typedef struct _ZUKAN_DETAIL_GRAPHIC_WORK  ZUKAN_DETAIL_GRAPHIC_WORK;

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
extern ZUKAN_DETAIL_GRAPHIC_WORK * ZUKAN_DETAIL_GRAPHIC_Init( int display_select, HEAPID heapID, BOOL use_3d );
//-------------------------------------
///破棄
//=====================================
extern void ZUKAN_DETAIL_GRAPHIC_Exit( ZUKAN_DETAIL_GRAPHIC_WORK *p_wk );
//-------------------------------------
///２D描画
//=====================================
extern void ZUKAN_DETAIL_GRAPHIC_2D_Draw( ZUKAN_DETAIL_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画	３Dを使わない場合、読んでも何もしません
//=====================================
extern void ZUKAN_DETAIL_GRAPHIC_3D_StartDraw( ZUKAN_DETAIL_GRAPHIC_WORK *p_wk );
extern void ZUKAN_DETAIL_GRAPHIC_3D_EndDraw( ZUKAN_DETAIL_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得	内部で設定したCLUNITを返します	OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * ZUKAN_DETAIL_GRAPHIC_GetClunit( const ZUKAN_DETAIL_GRAPHIC_WORK *cp_wk );

//-------------------------------------
///情報取得	内部で設定したカメラを返します	3D使わない場合NULL
//=====================================
extern GFL_G3D_CAMERA * ZUKAN_DETAIL_GRAPHIC_GetCamera( const ZUKAN_DETAIL_GRAPHIC_WORK *cp_wk );

//-------------------------------------
/// 3Dを有効/無効にする
//=====================================
extern void ZUKAN_DETAIL_GRAPHIC_Init3D( ZUKAN_DETAIL_GRAPHIC_WORK* p_wk, HEAPID heapID );
extern void ZUKAN_DETAIL_GRAPHIC_Exit3D( ZUKAN_DETAIL_GRAPHIC_WORK* p_wk );

