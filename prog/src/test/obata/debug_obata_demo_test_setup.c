#include <gflib.h>
#include "debug_obata_demo_test_setup.h"


#define cameraPerspway  ( 0x0b60 )
#define cameraAspect    ( FX32_ONE * 4 / 3 )
#define cameraNear	    ( 1 << FX32_SHIFT )
#define cameraFar       ( 1024 << FX32_SHIFT )


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

static const GFL_BG_BGCNT_HEADER sc_BGCNT1_S = 
{
	0, 0,				            // 初期表示位置
	0x800,					        // スクリーンバッファサイズ
	0,						          // スクリーンバッファオフセット
	GFL_BG_SCRSIZ_256x256,	// スクリーンサイズ
	GX_BG_COLORMODE_16,		  // カラーモード
	GX_BG_SCRBASE_0x0000,	  // スクリーンベースブロック
	GX_BG_CHARBASE_0x04000,	// キャラクタベースブロック
	GFL_BG_CHRSIZ_256x256,	// キャラクタエリアサイズ
	GX_BG_EXTPLTT_01,		    // BG拡張パレットスロット選択
	0,						          // 表示プライオリティー
	0,						          // エリアオーバーフラグ
	0,						          // DUMMY
	FALSE,					        // モザイク設定
};


//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light_data[] = {
	{ 0, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 1, {{  (FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
	{ 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
};
static const GFL_G3D_LIGHTSET_SETUP light_setup = { light_data, NELEMS(light_data) };
GFL_G3D_LIGHTSET* g_lightSet;


//-------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 */
//-------------------------------------------------------------------------------------------
void DEBUG_OBATA_DEMO_TEST_Init( HEAPID heap_id )
{
	//VRAM設定
	GFL_DISP_SetBank(&sc_VRAM_param );

  // BGL初期化
	GFL_BG_Init( heap_id );	// システム初期化
	GFL_BG_SetBGMode( &sc_BG_mode );	  // BGモード設定
	GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &sc_BGCNT1_M, GFL_BG_MODE_TEXT );	// MAIN_BG1の設定 
	//GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &sc_BGCNT1_S, GFL_BG_MODE_TEXT );	// SUB_BG1の設定 
	GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );						// MAIN_BG0を表示ON
	//GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );						// MAIN_BG1を表示ON
	GFL_BG_SetBGControl3D( PRIORITY_MAIN_BG0 );		              // 3D面の表示優先順位を設定

  // 3Dシステムを初期化
  GFL_G3D_Init( 
      GFL_G3D_VMANLNK, GFL_G3D_TEX128K, 
      GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0x1000, heap_id, NULL );

  // ライト作成
  g_lightSet = GFL_G3D_LIGHT_Create( &light_setup, heap_id );
  GFL_G3D_LIGHT_Switching( g_lightSet );

  // カメラ初期設定
  {
    GFL_G3D_PROJECTION proj = { GFL_G3D_PRJPERS, 0, 0, cameraAspect, 0, cameraNear, cameraFar, 0 }; 
    proj.param1 = FX_SinIdx( cameraPerspway ); 
    proj.param2 = FX_CosIdx( cameraPerspway ); 
    GFL_G3D_SetSystemProjection( &proj );	
  }
}

//-------------------------------------------------------------------------------------------
/**
 * @brief 終了処理
 */
//-------------------------------------------------------------------------------------------
void DEBUG_OBATA_DEMO_TEST_Exit()
{
  // ライトセット破棄
  GFL_G3D_LIGHT_Delete( g_lightSet );

  // 3Dシステム終了
  GFL_G3D_Exit();

  // BGL破棄
  GFL_BG_Exit();
}
