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
	TOWNMAP_BG_PAL_M_00 = 0,//	256pltここから
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
	TOWNMAP_BG_PAL_M_13,		//  256pltここまで
	TOWNMAP_BG_PAL_M_14,		// フォント
	TOWNMAP_BG_PAL_M_15,		//


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

	TOWNMAP_BG_FRAME_FONT_S,	//文字
	TOWNMAP_BG_FRAME_WND_S,		//文字のウィンドウ

	TOWNMAP_BG_FRAME_MAX,			//ソース内で使用
}TOWNMAP_GRAPHIC_BG_FRAME;


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
