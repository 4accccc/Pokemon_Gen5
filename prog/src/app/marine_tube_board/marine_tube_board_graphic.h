//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   marine_tube_board_graphic.h
 *  @brief  グラフィック設定
 *	@author	ichiraku_katsuhiko
 *	@data		2011.09.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include <gflib.h>

//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *          構造体宣言
*/
//=============================================================================
//-------------------------------------
/// グラフィックワーク
//=====================================
typedef struct _MARINE_TUBE_BOARD_GRAPHIC_WORK  MARINE_TUBE_BOARD_GRAPHIC_WORK;

//=============================================================================
/**
 *          プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///初期化
//  ディスプレイセレクトにはGX_DISP_SELECT_MAIN_SUB or GX_DISP_SELECT_SUB_MAINを
//  渡してください。途中で書き換えたい場合、GFL_DISP_SetDispSelectを使用して下さい。
//=====================================
extern MARINE_TUBE_BOARD_GRAPHIC_WORK * MARINE_TUBE_BOARD_GRAPHIC_Init( int display_select, HEAPID heapID );
//-------------------------------------
///破棄
//=====================================
extern void MARINE_TUBE_BOARD_GRAPHIC_Exit( MARINE_TUBE_BOARD_GRAPHIC_WORK *p_wk );
//-------------------------------------
///２D描画
//=====================================
extern void MARINE_TUBE_BOARD_GRAPHIC_2D_Draw( MARINE_TUBE_BOARD_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画  ３Dを使わない場合、読んでも何もしません
//=====================================
extern void MARINE_TUBE_BOARD_GRAPHIC_3D_StartDraw( MARINE_TUBE_BOARD_GRAPHIC_WORK *p_wk );
extern void MARINE_TUBE_BOARD_GRAPHIC_3D_EndDraw( MARINE_TUBE_BOARD_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得 内部で設定したCLUNITを返します  OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * MARINE_TUBE_BOARD_GRAPHIC_GetClunit( const MARINE_TUBE_BOARD_GRAPHIC_WORK *cp_wk );

//-------------------------------------
///情報取得 内部で設定したカメラを返します  3D使わない場合NULL
//=====================================
extern GFL_G3D_CAMERA * MARINE_TUBE_BOARD_GRAPHIC_GetCamera( const MARINE_TUBE_BOARD_GRAPHIC_WORK *cp_wk );
