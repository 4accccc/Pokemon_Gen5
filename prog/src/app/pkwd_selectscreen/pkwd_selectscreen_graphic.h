//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   pkwd_select_screen_graphic.h
 *  @brief  ポケウッド上映選択画面
 *  @author mori
 *  @date   2011.07.19
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
typedef struct _PKWD_SELECT_SCREEN_GRAPHIC_WORK  PKWD_SELECT_SCREEN_GRAPHIC_WORK;

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
extern PKWD_SELECT_SCREEN_GRAPHIC_WORK * PKWD_SELECT_SCREEN_GRAPHIC_Init( int display_select, HEAPID heapID );
//-------------------------------------
///破棄
//=====================================
extern void PKWD_SELECT_SCREEN_GRAPHIC_Exit( PKWD_SELECT_SCREEN_GRAPHIC_WORK *p_wk );
//-------------------------------------
///２D描画
//=====================================
extern void PKWD_SELECT_SCREEN_GRAPHIC_2D_Draw( PKWD_SELECT_SCREEN_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画  ３Dを使わない場合、読んでも何もしません
//=====================================
extern void PKWD_SELECT_SCREEN_GRAPHIC_3D_StartDraw( PKWD_SELECT_SCREEN_GRAPHIC_WORK *p_wk );
extern void PKWD_SELECT_SCREEN_GRAPHIC_3D_EndDraw( PKWD_SELECT_SCREEN_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得 内部で設定したCLUNITを返します  OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * PKWD_SELECT_SCREEN_GRAPHIC_GetClunit( const PKWD_SELECT_SCREEN_GRAPHIC_WORK *cp_wk );

//-------------------------------------
///情報取得 内部で設定したカメラを返します  3D使わない場合NULL
//=====================================
extern GFL_G3D_CAMERA * PKWD_SELECT_SCREEN_GRAPHIC_GetCamera( const PKWD_SELECT_SCREEN_GRAPHIC_WORK *cp_wk );
