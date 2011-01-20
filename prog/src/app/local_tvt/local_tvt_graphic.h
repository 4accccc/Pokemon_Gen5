//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		graphic.h
 *	@brief	グラフィック基本設定
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
//BG面
enum
{
  LTVT_FRAME_MESSAGE    = (GFL_BG_FRAME0_M),
  LTVT_FRAME_NAME       = (GFL_BG_FRAME0_M),  //背景確保のため統合
  LTVT_FRAME_NOIZE_BG   = (GFL_BG_FRAME1_M),
  LTVT_FRAME_CHARA      = (GFL_BG_FRAME2_M),
  LTVT_FRAME_BG         = (GFL_BG_FRAME3_M),
  LTVT_FRAME_SUB_MSG    = (GFL_BG_FRAME0_S),
  LTVT_FRAME_SUB_BAR    = (GFL_BG_FRAME1_S),
  LTVT_FRAME_SUB_MISC   = (GFL_BG_FRAME2_S),
  LTVT_FRAME_SUB_BG     = (GFL_BG_FRAME3_S),
};

//メインBGのパレット
#define LTVT_PLT_MAIN_NOIZE    (11)
#define LTVT_PLT_MAIN_FONT     (12)
#define LTVT_PLT_MAIN_WINFRAME (13)

//メインOBJのパレット
#define LTVT_PLT_OBJ_MAIN     (0)
#define LTVT_PLT_OBJ_CHARA     (3)  //2*4本
#define LTVT_PLT_OBJ_WEATEHER  (12)  //1*4本

//サブOBJのパレット
#define LTVT_PLT_S_OBJ        (0)
#define LTVT_PLT_OBJ_SUB_POKEMON    (8)  //3本
#define LTVT_PLT_OBJ_SUB_BARICON   (11) //3本

//サブBGのパレット
#define LTVT_PLT_BG_SUB_BAR   (6) //1本
#define LTVT_PLT_BG_SUB_TASKMENU   (7) //2本
#define LTVT_PLT_BG_SUB_WINFRAME  (9) //1本
#define LTVT_PLT_BG_SUB_BACK     (12)
#define LTVT_PLT_BG_SUB_LIST     (13)
#define LTVT_PLT_BG_SUB_LIST_ON  (14)
#define LTVT_PLT_BG_SUB_FONT     (15)

//拡張BGは面ごとに持つので両方0からでOK
#define LTVT_PLT_CHARA (0)
#define LTVT_PLT_BG (0)

#define LTVT_VRAM_ADR_CHARA (0x10000)
#define LTVT_VRAM_ADR_BG    (0x28000)
#define LTVT_VRAM_PAGE_SIZE (0x0C000)

#define LTVT_TRANS_COUNT_MAX (20)
#define LTVT_TRANS_SIZE (0x3000)    //1フレームに送る転送量

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	グラフィックワーク
//=====================================
typedef struct _LOCAL_TVT_GRAPHIC_WORK  LOCAL_TVT_GRAPHIC_WORK;

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
extern LOCAL_TVT_GRAPHIC_WORK * LOCAL_TVT_GRAPHIC_Init( int display_select, HEAPID heapID );
//-------------------------------------
///破棄
//=====================================
extern void LOCAL_TVT_GRAPHIC_Exit( LOCAL_TVT_GRAPHIC_WORK *p_wk );
//-------------------------------------
///２D描画
//=====================================
extern void LOCAL_TVT_GRAPHIC_2D_Draw( LOCAL_TVT_GRAPHIC_WORK *p_wk );

//-------------------------------------
///３D描画	３Dを使わない場合、読んでも何もしません
//=====================================
extern void LOCAL_TVT_GRAPHIC_3D_StartDraw( LOCAL_TVT_GRAPHIC_WORK *p_wk );
extern void LOCAL_TVT_GRAPHIC_3D_EndDraw( LOCAL_TVT_GRAPHIC_WORK *p_wk );

//-------------------------------------
///情報取得	内部で設定したCLUNITを返します	OBJ使わない場合NULL
//=====================================
extern GFL_CLUNIT * LOCAL_TVT_GRAPHIC_GetClunit( const LOCAL_TVT_GRAPHIC_WORK *cp_wk );

//-------------------------------------
///情報取得	内部で設定したカメラを返します	3D使わない場合NULL
//=====================================
extern GFL_G3D_CAMERA * LOCAL_TVT_GRAPHIC_GetCamera( const LOCAL_TVT_GRAPHIC_WORK *cp_wk );
