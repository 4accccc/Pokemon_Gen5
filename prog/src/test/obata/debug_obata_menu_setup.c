#include <gflib.h>
#include "debug_obata_menu_setup.h"


//============================================================================================
/**
 * @brief BGデータ
 */
//============================================================================================
// 優先順位
enum
{
	PRIORITY_MAIN_BG1,
	PRIORITY_MAIN_BG0,
};

// VRAMバンクの割り当て
static const GFL_DISP_VRAM sc_VRAM_param = 
{ 
	GX_VRAM_BG_128_B,				      // メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,		  	// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,		  	  // サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,  	// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_80_EF,				    // メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		  // メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,			    // サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_0_A,				      // テクスチャイメージスロット
	GX_VRAM_TEXPLTT_0_G,			    // テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_32K,
	GX_OBJVRAMMODE_CHAR_1D_32K,
};


// BGモード設定
static const GFL_BG_SYS_HEADER sc_BG_mode = 
{
	GX_DISPMODE_GRAPHICS,	// 表示モード
	GX_BGMODE_0,		   	  // BGモード(メインスクリーン)
	GX_BGMODE_0,		      // BGモード(サブスクリーン) 
	GX_BG0_AS_3D,		      // BG0の2D or 3Dモード選択
};

// BGコントロール設定
static const GFL_BG_BGCNT_HEADER sc_BGCNT1_M = 
{
	0, 0,				              // 初期表示位置
	0x800,					          // スクリーンバッファサイズ
	0,						            // スクリーンバッファオフセット
	GFL_BG_SCRSIZ_256x256,	  // スクリーンサイズ
	GX_BG_COLORMODE_16,		    // カラーモード
	GX_BG_SCRBASE_0x0000,	    // スクリーンベースブロック
	GX_BG_CHARBASE_0x04000,	  // キャラクタベースブロック
	GFL_BG_CHRSIZ_256x256,	  // キャラクタエリアサイズ
	GX_BG_EXTPLTT_01,		      // BG拡張パレットスロット選択
	PRIORITY_MAIN_BG1,		    // 表示プライオリティー
	0,						            // エリアオーバーフラグ
	0,						            // DUMMY
	FALSE,					          // モザイク設定
};

// パレットデータ
static const u16 palette[] = 
{
  0x0000,
  0xffff,
  0x7c00,
  0x001f,
};

//-------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 */
//-------------------------------------------------------------------------------------------
void DEBUG_OBATA_MENU_SETUP_Init( HEAPID heap_id )
{
	//VRAM設定
	GFL_DISP_SetBank( &sc_VRAM_param );

  // BGL初期化
	GFL_BG_Init( heap_id );	// システム初期化
	GFL_BG_SetBGMode( &sc_BG_mode );	  // BGモード設定
	GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &sc_BGCNT1_M, GFL_BG_MODE_TEXT );	// MAIN_BG1の設定 
	GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );						// MAIN_BG1を表示ON

  // パレット転送
  GFL_BG_LoadPalette( GFL_BG_FRAME1_M, (void*)palette, NELEMS(palette)*sizeof(u16), 0 );

  // ビットマップ管理システムの初期化
  GFL_BMPWIN_Init( heap_id );
}

//-------------------------------------------------------------------------------------------
/**
 * @brief 終了処理
 */
//-------------------------------------------------------------------------------------------
void DEBUG_OBATA_MENU_SETUP_Exit()
{ 
  // BGL破棄
  GFL_BG_Exit();

  // ビットマップ管理システムの終了
  GFL_BMPWIN_Exit();
}
