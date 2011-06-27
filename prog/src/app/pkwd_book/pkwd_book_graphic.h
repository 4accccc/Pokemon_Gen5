//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   pkwd_book_graphic.h
 *  @brief  ポケウッド台本(グラフィック設定)
 *  @author 
 *  @date   2009.09.30
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include <gflib.h>

#include "system/palanm.h"

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
typedef struct _PKWD_BOOK_GRAPHIC_WORK  PKWD_BOOK_GRAPHIC_WORK;

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
extern PKWD_BOOK_GRAPHIC_WORK * PKWD_BOOK_GRAPHIC_Init( int display_select, HEAPID heapID );

//-------------------------------------
///破棄
//=====================================
extern void PKWD_BOOK_GRAPHIC_Exit( PKWD_BOOK_GRAPHIC_WORK *p_wk );

//-------------------------------------
///２D描画
//=====================================
extern void PKWD_BOOK_GRAPHIC_2D_Draw( PKWD_BOOK_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画  ３Dを使わない場合、読んでも何もしません
//=====================================
extern void PKWD_BOOK_GRAPHIC_3D_StartDraw( PKWD_BOOK_GRAPHIC_WORK *p_wk );
extern void PKWD_BOOK_GRAPHIC_3D_EndDraw(   PKWD_BOOK_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得 内部で設定したCLUNITを返します  OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * PKWD_BOOK_GRAPHIC_GetClunit( const PKWD_BOOK_GRAPHIC_WORK *cp_wk );

//-------------------------------------
///情報取得 内部で設定したカメラを返します  3D使わない場合NULL
//=====================================
extern GFL_G3D_CAMERA * PKWD_BOOK_GRAPHIC_GetCamera( const PKWD_BOOK_GRAPHIC_WORK *cp_wk );

// パレットアニメ用
extern PALETTE_FADE_PTR PKWD_BOOK_GRAPHIC_GetPalFadePtr( const PKWD_BOOK_GRAPHIC_WORK *cp_wk );

