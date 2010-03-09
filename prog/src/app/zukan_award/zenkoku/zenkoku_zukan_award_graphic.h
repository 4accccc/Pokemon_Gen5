//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		zenkoku_zukan_award_graphic.h
 *	@brief	グラフィック設定
 *	@author	Koji Kawada
 *	@date		2010.03.04
 *	@note   ui_template_graphic.hをコピペして名前を変更しただけです。
 *
 *	モジュール名：ZENKOKU_ZUKAN_AWARD_GRAPHIC
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
typedef struct _ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK  ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK;

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
extern ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK * ZENKOKU_ZUKAN_AWARD_GRAPHIC_Init( int display_select, HEAPID heapID );
//-------------------------------------
///破棄
//=====================================
extern void ZENKOKU_ZUKAN_AWARD_GRAPHIC_Exit( ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK *p_wk );
//-------------------------------------
///２D描画
//=====================================
extern void ZENKOKU_ZUKAN_AWARD_GRAPHIC_2D_Draw( ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画	３Dを使わない場合、読んでも何もしません
//=====================================
extern void ZENKOKU_ZUKAN_AWARD_GRAPHIC_3D_StartDraw( ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK *p_wk );
extern void ZENKOKU_ZUKAN_AWARD_GRAPHIC_3D_EndDraw( ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得	内部で設定したCLUNITを返します	OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * ZENKOKU_ZUKAN_AWARD_GRAPHIC_GetClunit( const ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK *cp_wk );

//-------------------------------------
///情報取得	内部で設定したカメラを返します	3D使わない場合NULL
//=====================================
extern GFL_G3D_CAMERA * ZENKOKU_ZUKAN_AWARD_GRAPHIC_GetCamera( const ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK *cp_wk );
