//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_fade.h
 *	@brief  バトルレコーダー用フェードシステム
 *	@author	Toru=Nagihashi
 *	@date		2009.12.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	フェードの種類
//=====================================
typedef enum
{
  BR_FADE_TYPE_MASTERBRIGHT_BLACK,  //輝度  黒
  BR_FADE_TYPE_ALPHA_BG012OBJ,      //アルファ　BG0,1,2,OBJ
  BR_FADE_TYPE_PALLETE,             //パレットフェード

} BR_FADE_TYPE;

//-------------------------------------
///	フェードをする箇所
//=====================================
typedef enum
{ 
  BR_FADE_DISPLAY_MAIN  = 1<<0,
  BR_FADE_DISPLAY_SUB   = 1<<1,
  BR_FADE_DISPLAY_BOTH  = BR_FADE_DISPLAY_MAIN | BR_FADE_DISPLAY_SUB,

}BR_FADE_DISPLAY;

//-------------------------------------
///	フェードの方向
//=====================================
typedef enum
{ 
  BR_FADE_DIR_IN,   //16->0
  BR_FADE_DIR_OUT,  //0->16
}BR_FADE_DIR;

//-------------------------------------
///	パレットフェードで使用する色
//=====================================
#define BR_FADE_COLOR_BLUE  (0x7e05)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	フェードシステム
//=====================================
typedef struct _BR_FADE_WORK BR_FADE_WORK;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//-------------------------------------
///	システム系
//=====================================
extern BR_FADE_WORK * BR_FADE_Init( HEAPID heapID );
extern void BR_FADE_Exit( BR_FADE_WORK *p_wk );
extern void BR_FADE_Main( BR_FADE_WORK *p_wk );

//-------------------------------------
///	フェード操作
//=====================================
extern void BR_FADE_StartFade( BR_FADE_WORK *p_wk, BR_FADE_TYPE type, BR_FADE_DISPLAY disp, BR_FADE_DIR  dir );
extern void BR_FADE_StartFadeEx( BR_FADE_WORK *p_wk, BR_FADE_TYPE type, BR_FADE_DISPLAY disp, BR_FADE_DIR dir, u32 sync );
extern BOOL BR_FADE_IsEnd( const BR_FADE_WORK *cp_wk );

//-------------------------------------
///	その他特殊
//=====================================
extern void BR_FADE_PALETTE_Copy( BR_FADE_WORK *p_wk );
extern void BR_FADE_PALETTE_SetColor( BR_FADE_WORK *p_wk, GXRgb rgb );
extern void BR_FADE_PALETTE_TransColor( BR_FADE_WORK *p_wk, BR_FADE_DISPLAY display );

