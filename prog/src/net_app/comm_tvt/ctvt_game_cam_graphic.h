//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		graphic.h
 *	@brief	グラフィック設定
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.01.18
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
//-------------------------------------
///	BGフレーム
//=====================================
enum
{
  // メイン
  BG_FRAME_BACK_M   = GFL_BG_FRAME0_M,    // 背景
  BG_FRAME_TEXT_M   = GFL_BG_FRAME1_M,    // テキスト欄
  BG_FRAME_CAMERA_M = GFL_BG_FRAME3_M,    // 撮影したキャプチャ表示[TWL]

  // サブ
  BG_FRAME_BACK_S   = GFL_BG_FRAME0_S,    // 背景[NITRO]
  BG_FRAME_CAMERA_S = GFL_BG_FRAME3_S,    // カメラキャプチャ出力[TWL]
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	グラフィックワーク
//=====================================
typedef struct _CTVT_GAME_CAM_GRAPHIC_WORK  CTVT_GAME_CAM_GRAPHIC_WORK;

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
extern CTVT_GAME_CAM_GRAPHIC_WORK * CTVT_GAME_CAM_GRAPHIC_Init( int display_select, HEAPID heapID );
//-------------------------------------
///破棄
//=====================================
extern void CTVT_GAME_CAM_GRAPHIC_Exit( CTVT_GAME_CAM_GRAPHIC_WORK *p_wk );
//-------------------------------------
///２D描画
//=====================================
extern void CTVT_GAME_CAM_GRAPHIC_2D_Draw( CTVT_GAME_CAM_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画	３Dを使わない場合、読んでも何もしません
//=====================================
extern void CTVT_GAME_CAM_GRAPHIC_3D_StartDraw( CTVT_GAME_CAM_GRAPHIC_WORK *p_wk );
extern void CTVT_GAME_CAM_GRAPHIC_3D_EndDraw( CTVT_GAME_CAM_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得	内部で設定したCLUNITを返します	OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * CTVT_GAME_CAM_GRAPHIC_GetClunit( const CTVT_GAME_CAM_GRAPHIC_WORK *cp_wk );

//-------------------------------------
///情報取得	内部で設定したカメラを返します	3D使わない場合NULL
//=====================================
extern GFL_G3D_CAMERA * CTVT_GAME_CAM_GRAPHIC_GetCamera( const CTVT_GAME_CAM_GRAPHIC_WORK *cp_wk );
