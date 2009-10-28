//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		townmap_grh.h
 *	@brief	タウンマップ用基本グラフィック
 *	@author	Toru=Nagihashi
 *	@data		2009.07.17
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
///	パレット
//=====================================
enum
{	
	// メイン画面BG(下画面)
	TOWNMAP_BG_PAL_M_00 = 0,//	
	TOWNMAP_BG_PAL_M_01,		//  
	TOWNMAP_BG_PAL_M_02,		//  
	TOWNMAP_BG_PAL_M_03,		//  
	TOWNMAP_BG_PAL_M_04,		//  
	TOWNMAP_BG_PAL_M_05,		//  
	TOWNMAP_BG_PAL_M_06,		//  
	TOWNMAP_BG_PAL_M_07,		//  
	TOWNMAP_BG_PAL_M_08,		//  
	TOWNMAP_BG_PAL_M_09,		//  
	TOWNMAP_BG_PAL_M_10,		//  
	TOWNMAP_BG_PAL_M_11,		//  
	TOWNMAP_BG_PAL_M_12,		//  
	TOWNMAP_BG_PAL_M_13,		//  
	TOWNMAP_BG_PAL_M_14,		// フォント
	TOWNMAP_BG_PAL_M_15,		// アプリケーションバー


	// サブ画面BG（上画面）
	TOWNMAP_BG_PAL_S_00 = 0,	//  使用してない
	TOWNMAP_BG_PAL_S_01,		//   使用してない
	TOWNMAP_BG_PAL_S_02,		//   使用してない
	TOWNMAP_BG_PAL_S_03,		//   使用してない
	TOWNMAP_BG_PAL_S_04,		//   使用してない
	TOWNMAP_BG_PAL_S_05,		//   使用してない
	TOWNMAP_BG_PAL_S_06,		//   使用してない
	TOWNMAP_BG_PAL_S_07,		// 使用してない
	TOWNMAP_BG_PAL_S_08,		// 使用してない
	TOWNMAP_BG_PAL_S_09,		// 使用してない
	TOWNMAP_BG_PAL_S_10,		// 使用してない
	TOWNMAP_BG_PAL_S_11,		// 使用してない
	TOWNMAP_BG_PAL_S_12,		// 使用してない
	TOWNMAP_BG_PAL_S_13,		// 使用してない
	TOWNMAP_BG_PAL_S_14,		// フォント
	TOWNMAP_BG_PAL_S_15,		//	使用してない

	// メイン画面OBJ(下画面)
	TOWNMAP_OBJ_PAL_M_00 = 0,//	OBJ
	TOWNMAP_OBJ_PAL_M_01,		//  OBJ
	TOWNMAP_OBJ_PAL_M_02,		//	OBJ
	TOWNMAP_OBJ_PAL_M_03,		//  
	TOWNMAP_OBJ_PAL_M_04,		//  
	TOWNMAP_OBJ_PAL_M_05,		//  
	TOWNMAP_OBJ_PAL_M_06,		//  
	TOWNMAP_OBJ_PAL_M_07,		//  
	TOWNMAP_OBJ_PAL_M_08,		//  
	TOWNMAP_OBJ_PAL_M_09,		//  
	TOWNMAP_OBJ_PAL_M_10,		// たっちー
	TOWNMAP_OBJ_PAL_M_11,		// タッチバー
	TOWNMAP_OBJ_PAL_M_12,		// タッチバー
	TOWNMAP_OBJ_PAL_M_13,		// タッチバー
	TOWNMAP_OBJ_PAL_M_14,		// アプリケーションバー
	TOWNMAP_OBJ_PAL_M_15,		// アプリケーションバー


	// サブ画面OBJ（上画面）
	TOWNMAP_OBJ_PAL_S_00 = 0,	//  使用してない
	TOWNMAP_OBJ_PAL_S_01,		//   使用してない
	TOWNMAP_OBJ_PAL_S_02,		//   使用してない
	TOWNMAP_OBJ_PAL_S_03,		//   使用してない
	TOWNMAP_OBJ_PAL_S_04,		//   使用してない
	TOWNMAP_OBJ_PAL_S_05,		//   使用してない
	TOWNMAP_OBJ_PAL_S_06,		//   使用してない
	TOWNMAP_OBJ_PAL_S_07,		// 使用してない
	TOWNMAP_OBJ_PAL_S_08,		// 使用してない
	TOWNMAP_OBJ_PAL_S_09,		// 使用してない
	TOWNMAP_OBJ_PAL_S_10,		// 使用してない
	TOWNMAP_OBJ_PAL_S_11,		// 使用してない
	TOWNMAP_OBJ_PAL_S_12,		// 使用してない
	TOWNMAP_OBJ_PAL_S_13,		// 使用してない
	TOWNMAP_OBJ_PAL_S_14,		// 使用してない
	TOWNMAP_OBJ_PAL_S_15,		//	使用してない

};

//-------------------------------------
///	BGフレーム
//	ソースのsc_bgsetupと対応
//=====================================
typedef enum
{
	TOWNMAP_BG_FRAME_BAR_M,		//INFOWINや下のバー
	TOWNMAP_BG_FRAME_FONT_M,	//文字面
	TOWNMAP_BG_FRAME_ROAD_M,	//道
	TOWNMAP_BG_FRAME_MAP_M,		//地図

	TOWNMAP_BG_FRAME_DEBUG_S,	//デバッグ
	TOWNMAP_BG_FRAME_FONT_S,	//文字
	TOWNMAP_BG_FRAME_WND_S,		//文字のウィンドウ

	TOWNMAP_BG_FRAME_MAX,			//ソース内で使用
}TOWNMAP_GRAPHIC_BG_FRAME;

//-------------------------------------
///	BG優先度
//=====================================
enum 
{
	TOWNMAP_BG_PRIORITY_TOP_M	= 0,
	TOWNMAP_BG_PRIORITY_FONT_M	= TOWNMAP_BG_PRIORITY_TOP_M,
	TOWNMAP_BG_PRIORITY_BAR_M,
	TOWNMAP_BG_PRIORITY_ROAD_M,
	TOWNMAP_BG_PRIORITY_MAP_M,

	TOWNMAP_BG_PRIORITY_TOP_S	= 0,
	TOWNMAP_BG_PRIORITY_FONT_S	= TOWNMAP_BG_PRIORITY_TOP_S,
	TOWNMAP_BG_PRIORITY_WND_S,
} ;


//-------------------------------------
///	CLUNIT
//=====================================
typedef enum
{
	TOWNMAP_OBJ_CLUNIT_DEFAULT,	//汎用

	TOWNMAP_OBJ_CLUNIT_MAX
}TOWNMAP_GRAPHIC_OBJ_CLUNIT ;
//-------------------------------------
///	CKWK
//=====================================
typedef enum
{
	TOWNMAP_OBJ_CLWK_WINDOW,
	TOWNMAP_OBJ_CLWK_CURSOR,
	TOWNMAP_OBJ_CLWK_RING_CUR,
	TOWNMAP_OBJ_CLWK_HERO,

	TOWNMAP_OBJ_CLWK_MAX
}TOWNMAP_GRAPHIC_OBJ_CLWK ;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	タウンマップ用基本グラフィックワーク
//=====================================
typedef struct _TOWNMAP_GRAPHIC_SYS TOWNMAP_GRAPHIC_SYS;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern TOWNMAP_GRAPHIC_SYS * TOWNMAP_GRAPHIC_Init( HEAPID heapID );
extern void TOWNMAP_GRAPHIC_Exit( TOWNMAP_GRAPHIC_SYS *p_wk );
extern void TOWNMAP_GRAPHIC_Draw( TOWNMAP_GRAPHIC_SYS *p_wk );

extern u8 TOWNMAP_GRAPHIC_GetFrame( const TOWNMAP_GRAPHIC_SYS *cp_wk, TOWNMAP_GRAPHIC_BG_FRAME frame );
extern GFL_CLUNIT *TOWNMAP_GRAPHIC_GetUnit( const TOWNMAP_GRAPHIC_SYS *cp_wk, TOWNMAP_GRAPHIC_OBJ_CLUNIT id );
extern GFL_CLWK	*TOWNMAP_GRAPHIC_GetClwk( const TOWNMAP_GRAPHIC_SYS *cp_wk, TOWNMAP_GRAPHIC_OBJ_CLWK id );
//extern GFL_CLSYS_REND *TOWNMAP_GRAPHIC_GetRend( const TOWNMAP_GRAPHIC_SYS *cp_wk );
extern void TOWNMAP_GRAPHIC_GetObjResource( const TOWNMAP_GRAPHIC_SYS *cp_wk, u32 *p_chr, u32 *p_cel, u32 *p_plt );
