//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_zone_gra.c
 *	@brief  生息地リスト：グラフィックモジュール
 *	@author	Masayuki Onoue
 *	@date		2011.08.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "habitat_zone_gra.h"

#include "system/main.h"  // for HEAP_ID

#include "system/wipe.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "app/app_menu_common.h"

#include "habitat_zone_gra.cdat"

#include "../zukan_common.h" // for ZKNCOMM_FADE_WAIT

#include "arc_def.h"        // ARCID_
#include "zukan_gra.naix"
#include "font/font.naix"
#include "townmap_gra.naix"

//ポケアイコン
#include "pokeicon/pokeicon.h"

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

// ＢＧパレット番号
#define	HABITAT_ZONE_MBG_PAL_TOUCHBAR	( 14 )		// タッチバー
#define	HABITAT_ZONE_MBG_PAL_FONT			( 15 )		// メイン画面フォントパレット
#define	HABITAT_ZONE_SBG_PAL_FONT			( 15 )		// サブ画面フォントパレット

// タッチバー定義
#define	HABITAT_ZONE_TOUCH_BAR_PX		( 0 )
#define	HABITAT_ZONE_TOUCH_BAR_PY		( 21 )
#define	HABITAT_ZONE_TOUCH_BAR_SX		( 32 )
#define	HABITAT_ZONE_TOUCH_BAR_SY		( 3 )

#define	FRAME_SCROLL_X_WAIN	  ( 4 )		// 4フレームに1階動く
#define	FRAME_SCROLL_X_VALUE	( 1 )		// 背景スクロールカウント
#define	MAP_Y_SCROLL_COUNT	( -24 )		// マップのY軸ずれ

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

// 初期化系 ---------------------------------------
static void GRA_InitVram( void );
static void GRA_InitBg( void );
static void GRA_InitObj( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_InitClact( void );
static void GRA_LoadBgGraphic(void);
static void GRA_LoadObjGraphic( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_InitPaletteFade( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_InitFrameScroll( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_InitBmpWin( HABITAT_ZONE_GRA_WK *p_wk );

// 終了系
static void GRA_ResetWindow( void );
static void GRA_ExitPaletteFade( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_ExitBg( void );
static void GRA_ExitObj( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_ExitBmpWin( HABITAT_ZONE_GRA_WK *p_wk );

// 処理系
static void GRA_SetPalFadeSeq( HABITAT_ZONE_GRA_WK *p_wk, u8 start, u8 end );
static void GRA_PrintMain( HABITAT_ZONE_GRA_WK *p_wk );

//===============================================================================
// 初期化・終了関数呼び出し
//===============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_Init( HABITAT_ZONE_GRA_WK *p_wk )
{
  
  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  
  // 表示初期化
  GFL_DISP_GX_SetVisibleControlDirect( 0 );
  GFL_DISP_GXS_SetVisibleControlDirect( 0 );

  // ブレンド初期化
  G2_BlendNone();
  G2S_BlendNone();

  // 輝度を最低にしておく(フェード前に解除する)
  GX_SetMasterBrightness( -16 );
  GXS_SetMasterBrightness( -16 );

  // サブ画面をメインに
  GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

  // 初期化
  GRA_InitVram();           // VRAM
  GRA_InitBg();             // BG
  GRA_InitObj( p_wk );      // OBJ
  GRA_InitBmpWin( p_wk );
  GRA_LoadObjGraphic( p_wk ); // OBJ初期化
  GRA_LoadBgGraphic();      // BGリソース読み込み
  GRA_InitPaletteFade( p_wk );    // パレットフェード
  GRA_InitFrameScroll( p_wk );          // スクロール
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックの終了処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_Exit( HABITAT_ZONE_GRA_WK *p_wk )
{
  // ウィンドウのリセット
  GRA_ResetWindow();

  // BMPWINの開放
  GRA_ExitBmpWin( p_wk );

  // OBJの終了処理
  GRA_ExitObj( p_wk );
  
  // BGの終了処理
	GRA_ExitBg();

  // パレットフェードの終了
  GRA_ExitPaletteFade( p_wk );

	// 輝度を最低にしておく
	GX_SetMasterBrightness( -16 );
	GXS_SetMasterBrightness( -16 );

	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();

	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
  
  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
}

//----------------------------------------------------------------------------
/**
 *	@brief  VBlankメイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_VblankMain( HABITAT_ZONE_GRA_WK *p_wk )
{
	GFL_CLACT_SYS_VBlankFunc();
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックメイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_Main( HABITAT_ZONE_GRA_WK *p_wk )
{
  // 文字描画更新
  GRA_PrintMain( p_wk );

  // 背景スクロール
  p_wk->back_bg_scroll_wait_count++;
  if( p_wk->back_bg_scroll_wait_count == FRAME_SCROLL_X_WAIN )
  {
    GFL_BG_SetScrollReq( BGM_FRAME_SCROLL_WIN,
                              GFL_BG_SCROLL_X_INC, FRAME_SCROLL_X_VALUE );
    p_wk->back_bg_scroll_wait_count = 0;
  }

  // セルアクター更新
  GFL_CLACT_SYS_Main();
}

//===============================================================================
// 初期化・終了関数郡
//===============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  VRAM初期化
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_InitVram( void )
{
	GFL_DISP_SetBank( &VramTbl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG初期化
 *
 *	@param	none
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_InitBg(void)
{
	GFL_BG_Init( HEAPID_ZUKAN_HABITAT_ZONE );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

  { // フォント面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGM_FRAME_FONT, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGM_FRAME_FONT );
		GFL_BG_SetClearCharacter( BGM_FRAME_FONT, 0x20, 0, HEAPID_ZUKAN_HABITAT_ZONE );
		GFL_BG_LoadScreenV_Req( BGM_FRAME_FONT );
    GFL_BG_SetVisible( BGM_FRAME_FONT ,VISIBLE_ON );
	}

	{	// メニューバー
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x4000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGM_FRAME_MENUBAR, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( BGM_FRAME_MENUBAR ,VISIBLE_ON );
	}
	{	// 背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGM_FRAME_BACK, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGM_FRAME_BACK );
		GFL_BG_LoadScreenV_Req( BGM_FRAME_BACK );
    GFL_BG_SetVisible( BGM_FRAME_BACK ,VISIBLE_ON );
	}
	{	// 最背面（スクロール）
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGM_FRAME_SCROLL_WIN, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( BGM_FRAME_SCROLL_WIN ,VISIBLE_ON );
	}

  // 上画面 ------------------------------------------------------------------------------
	{	// フォント面（上）
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGS_FRAME_FONT, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGS_FRAME_FONT );
		GFL_BG_SetClearCharacter( BGS_FRAME_FONT, 0x20, 0, HEAPID_ZUKAN_HABITAT_ZONE );
		GFL_BG_LoadScreenV_Req( BGS_FRAME_FONT );
    GFL_BG_SetVisible( BGS_FRAME_FONT ,VISIBLE_ON );
	}

	{	// サブ画面：タイトルバー
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGS_FRAME_BAR, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGS_FRAME_BAR );
		GFL_BG_LoadScreenV_Req( BGS_FRAME_BAR );
    GFL_BG_SetVisible( BGS_FRAME_BAR ,VISIBLE_ON );
	}
	{	// サブ画面：マップ
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, 0x8000,
			GX_BG_EXTPLTT_23, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGS_FRAME_MAP, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGS_FRAME_MAP );
		GFL_BG_LoadScreenV_Req( BGS_FRAME_MAP );
    GFL_BG_SetVisible( BGS_FRAME_MAP ,VISIBLE_ON );
	}

	{	// 未使用面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_23, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGS_FRAME_NO_USE, &cnth, GFL_BG_MODE_256X16 );
    GFL_BG_SetVisible( BGS_FRAME_NO_USE ,VISIBLE_OFF );
	}

  // 表示状態変更
	GFL_DISP_GX_SetVisibleControl(
		BGM_FRAME_FONT | BGM_FRAME_MENUBAR | BGM_FRAME_BACK | BGM_FRAME_SCROLL_WIN
      ,VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		BGS_FRAME_FONT | BGS_FRAME_BAR | BGS_FRAME_MAP , VISIBLE_ON );

}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJの初期化
 *
 *	@param	*p_wk  
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_InitObj( HABITAT_ZONE_GRA_WK *p_wk )
{
  // セルアクターの初期化
  GRA_InitClact();

  // ユニットの作成
	p_wk->p_clunit	= GFL_CLACT_UNIT_Create( HABITAT_ZONE_UNIT_CLWK_MAX, 0, HEAPID_ZUKAN_HABITAT_ZONE );

  // デフォルトレンダラーに設定
  GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );

	//表示
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

}

//----------------------------------------------------------------------------
/**
 *	@brief  ビットマップウィンドウの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_InitBmpWin( HABITAT_ZONE_GRA_WK *p_wk )
{
  // BMPWINの初期化
  GFL_BMPWIN_Init( HEAPID_ZUKAN_HABITAT_ZONE );

  // メニュー用BMPWIN作成
  p_wk->p_font_win = GFL_BMPWIN_Create( BGM_FRAME_FONT,
      0   , 0   ,
      32  , 24  ,
      HABITAT_ZONE_MBG_PAL_FONT , GFL_BMP_CHRAREA_GET_F );

  // スクリーンの作成
  GFL_BMPWIN_MakeScreen( p_wk->p_font_win );
 
  // ウィンドウのクリアー
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_font_win), 0 );

  // 転送
  GFL_BMPWIN_TransVramCharacter( p_wk->p_font_win );

  // 転送リクエスト
  GFL_BG_LoadScreenV_Req( BGM_FRAME_FONT );

  // プリントキューの作成
  p_wk->p_print_que = PRINTSYS_QUE_Create( HEAPID_ZUKAN_HABITAT_ZONE );
 
  // プリントユーティリティーの準備
  PRINT_UTIL_Setup( &p_wk->print_util[ PRINT_UTIL_INFO_WIN ] , p_wk->p_font_win  );
 
	// フォント
  p_wk->p_font	= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

}

//----------------------------------------------------------------------------
/**
 *	@brief  セルアクターの初期化
 *
 *	@param	none
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_InitClact( void )
{
	const GFL_CLSYS_INIT init = {
		0, 0,									// メイン　サーフェースの左上座標
		0, 512,								// サブ　サーフェースの左上座標
		4,										// メイン画面OAM管理開始位置	4の倍数
		124,									// メイン画面OAM管理数				4の倍数
		4,										// サブ画面OAM管理開始位置		4の倍数
		124,									// サブ画面OAM管理数					4の倍数
		0,										// セルVram転送管理数

	  HABITAT_ZONE_UNIT_CLWK_MAX,	// 登録できるキャラデータ数
		HABITAT_ZONE_UNIT_CLWK_MAX,	// 登録できるパレットデータ数
		HABITAT_ZONE_UNIT_CLWK_MAX,	// 登録できるセルアニメパターン数
		0,											// 登録できるマルチセルアニメパターン数（※現状未対応）

	  16,										// メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	  16										// サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	};
	GFL_CLACT_SYS_Create( &init, &VramTbl, HEAPID_ZUKAN_HABITAT_ZONE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGの読み込み
 *
 *	@param	none
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_LoadBgGraphic(void)
{
	ARCHANDLE * ah;

  // 図鑑アーカイブ
  {
    ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_HABITAT_ZONE );

    // パレット転送
    GFL_ARCHDL_UTIL_TransVramPalette(
        ah, NARC_zukan_gra_list_list_bgd_NCLR, PALTYPE_MAIN_BG, 0, 0, HEAPID_ZUKAN_HABITAT_ZONE );

    // 下画面パレット転送(通常パレット)
    GFL_ARCHDL_UTIL_TransVramPalette(
        ah, NARC_zukan_gra_list_list_bgu_NCLR, PALTYPE_SUB_BG, 0, 0, HEAPID_ZUKAN_HABITAT_ZONE );

    // 背景（した画面）
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_zukan_gra_list_listbase_bgd_NCGR, BGM_FRAME_BACK, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_zukan_gra_habitat_zone_seisoku_bg_NSCR,
        BGM_FRAME_BACK, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

    // スクロールウィンドウ（下画面）
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_zukan_gra_list_listbase_bgd_NCGR, BGM_FRAME_SCROLL_WIN, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_zukan_gra_list_listbase_bgd_NSCR,
        BGM_FRAME_SCROLL_WIN, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

    // キャラクタ転送（サブタイトルバー）
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_zukan_gra_list_listbase_bgu_NCGR, BGS_FRAME_BAR, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_zukan_gra_habitat_zone_seisoku_title_NSCR,
        BGS_FRAME_BAR, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );
    
    GFL_ARC_CloseDataHandle( ah );
  }

  // タウンマップアーカイブ
  {
    
    // タウンマップスクリーン（パレット0を参照している）
    ah	= GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, HEAPID_ZUKAN_HABITAT_ZONE );

    // 拡張パレットに転送(スロット２にMAPパレット)
    GFL_ARCHDL_UTIL_TransVramPalette(
        ah, NARC_townmap_gra_tmap_bg_d_NCLR, PALTYPE_SUB_BG_EX,0x4000, 0,HEAPID_ZUKAN_HABITAT_ZONE );
    // キャラクタ
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_townmap_gra_tmap_map_d_NCGR, BGS_FRAME_MAP, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );
    // スクリーン
    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_townmap_gra_tmap_map_d_NSCR,
        BGS_FRAME_MAP, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

    GFL_ARC_CloseDataHandle( ah );
  }

  // 共通素材アーカイブ
  {
    // タッチバー
    ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_ZUKAN_HABITAT_ZONE );

    GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
        ah, APP_COMMON_GetBarCharArcIdx(), BGM_FRAME_MENUBAR, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_ARCHDL_UTIL_TransVramPalette(
        ah, APP_COMMON_GetBarPltArcIdx(), PALTYPE_MAIN_BG,
        HABITAT_ZONE_MBG_PAL_TOUCHBAR*0x20, 0x20, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, APP_COMMON_GetBarScrnArcIdx(),
        BGM_FRAME_MENUBAR, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_BG_ChangeScreenPalette(
        BGM_FRAME_MENUBAR, HABITAT_ZONE_TOUCH_BAR_PX, HABITAT_ZONE_TOUCH_BAR_PY,
        HABITAT_ZONE_TOUCH_BAR_SX, HABITAT_ZONE_TOUCH_BAR_SY, HABITAT_ZONE_MBG_PAL_TOUCHBAR );
    GFL_BG_LoadScreenV_Req( BGM_FRAME_MENUBAR );

    GFL_ARC_CloseDataHandle( ah );
  }

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		HABITAT_ZONE_MBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_HABITAT_ZONE );
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		HABITAT_ZONE_SBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_HABITAT_ZONE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJリソースの読み込み
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_LoadObjGraphic( HABITAT_ZONE_GRA_WK *p_wk )
{
  // アプリ常駐OBJリソース読み込み
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = ARCID_ZUKAN_GRA;
    prm.pltt_id   = NARC_zukan_gra_list_list_objd_NCLR;
    prm.ncg_id    = NARC_zukan_gra_list_list_objd_NCGR;
    prm.cell_id   = NARC_zukan_gra_list_list_objd_NCER; 
    prm.anm_id    = NARC_zukan_gra_list_list_objd_NANR;
    prm.pltt_line = 0;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num  = 7;

    UI_EASY_CLWK_LoadResource( &p_wk->clres_common_obj, &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_ZONE );
  }

  // セルアクターワークを作成
  {
    // 季節Lボタン
    p_wk->p_clwk_common_obj[OBJ_WK_SEASON_L] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_L_X, OBJ_POS_L_Y, OBJ_ANM_SEQ_L, HEAPID_ZUKAN_HABITAT_ZONE );

    // 季節Rボタン
    p_wk->p_clwk_common_obj[OBJ_WK_SEASON_R] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_R_X, OBJ_POS_R_Y, OBJ_ANM_SEQ_R, HEAPID_ZUKAN_HABITAT_ZONE );

    // 大きなスタンプ
    p_wk->p_clwk_common_obj[OBJ_WK_STAMP_BIG] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_STAMP_BIG_X, OBJ_POS_STAMP_BIG_Y,
      OBJ_ANM_SEQ_STAMP_BIG_LOOK, HEAPID_ZUKAN_HABITAT_ZONE );
    // @todo スタンプ状態でアニメーションフレームを設定できるようにする
    GFL_CLACT_WK_SetAutoAnmFlag(p_wk->p_clwk_common_obj[OBJ_WK_STAMP_BIG],TRUE );

    // 陸地スタンプ
    p_wk->p_clwk_common_obj[OBJ_WK_STAMP_LAND] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_STAMP_LAND_X, OBJ_POS_STAMP_LAND_Y,
      OBJ_ANM_SEQ_STAMP_MIDDLE_LOOK, HEAPID_ZUKAN_HABITAT_ZONE );

    // 水辺スタンプ
    p_wk->p_clwk_common_obj[OBJ_WK_STAMP_WATERSIDE] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_STAMP_WATERSIDE_X, OBJ_POS_STAMP_WATERSIDE_Y,
      OBJ_ANM_SEQ_STAMP_MIDDLE_LOOK, HEAPID_ZUKAN_HABITAT_ZONE );

    // 釣りスタンプ
    p_wk->p_clwk_common_obj[OBJ_WK_STAMP_FISHING] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_STAMP_FISHING_X, OBJ_POS_STAMP_FISHING_Y,
      OBJ_ANM_SEQ_STAMP_MIDDLE_LOOK, HEAPID_ZUKAN_HABITAT_ZONE );

  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンOBJリソースを読み込み
 *
 *	@param	*p_wk 
 *
 *	@retval 生成したセルアクターワーク
 */
//-----------------------------------------------------------------------------
GFL_CLWK* HABITAT_ZONE_GRA_LoadPokeIcon( HABITAT_ZONE_GRA_WK *p_wk ,u32 mons_no ,u8 index )
{
  // ポケアイコン本体
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NCLR;
    prm.arc_id    = ARCID_POKEICON;
    prm.pltt_id   = POKEICON_GetPalArcIndex();
    prm.ncg_id    = POKEICON_GetCgxArcIndexByMonsNumber( mons_no, 0, 0, 0 );
    prm.cell_id   = POKEICON_GetCellArcIndex(); 
    prm.anm_id    = POKEICON_GetAnmArcIndex();
    prm.pltt_line = 7;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num = 3;
    
    UI_EASY_CLWK_LoadResource( &p_wk->clres_poke_icon[index], &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_ZONE );

    // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
    p_wk->p_clwk_poke_icon[index] = UI_EASY_CLWK_CreateCLWK( &p_wk->clres_poke_icon[index], p_wk->p_clunit, 0, 0, 1, HEAPID_ZUKAN_HABITAT_ZONE );

    // 上にアイテムアイコンを描画するので優先度を下げておく
    GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk_poke_icon[index], HABITAT_ZONE_OBJPRI_POKEICON );

    // オートアニメ OFF
    GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk_poke_icon[index], FALSE );

    {
      u8 pal_num = POKEICON_GetPalNum( mons_no, 0, 0, 0 );
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk_poke_icon[index], pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
    }
  }

  return p_wk->p_clwk_poke_icon[index];
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレットフェードの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_InitPaletteFade( HABITAT_ZONE_GRA_WK *p_wk )
{
	p_wk->p_pfd = PaletteFadeInit( HEAPID_ZUKAN_HABITAT_ZONE );

	PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_MAIN_BG, FADE_PAL_ALL_SIZE, HEAPID_ZUKAN_HABITAT_ZONE );
	PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_SUB_BG, FADE_PAL_ALL_SIZE, HEAPID_ZUKAN_HABITAT_ZONE );
	PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_MAIN_OBJ, FADE_PAL_ALL_SIZE, HEAPID_ZUKAN_HABITAT_ZONE );
	PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_SUB_OBJ, FADE_PAL_ALL_SIZE, HEAPID_ZUKAN_HABITAT_ZONE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ウィンドウのリセット
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_ResetWindow( void )
{
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGの終了処理
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_ExitBg( void )
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

	GFL_BG_Exit();
}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJ終了処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_ExitObj( HABITAT_ZONE_GRA_WK *p_wk )
{
  s32 i;
  // リソースの開放
  for( i = 0; i < HABITAT_ZONE_POKE_LIST_MAX; i++ )
  {
    UI_EASY_CLWK_UnLoadResource( &p_wk->clres_poke_icon[i] );
  }
  UI_EASY_CLWK_UnLoadResource( &p_wk->clres_common_obj );
  // ユニットの開放
  GFL_CLACT_UNIT_Delete( p_wk->p_clunit );
  // セルアクターシステムの終了
	GFL_CLACT_SYS_Delete();
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレットフェードの終了処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_ExitPaletteFade( HABITAT_ZONE_GRA_WK *p_wk )
{
	PaletteFadeWorkAllocFree( p_wk->p_pfd, FADE_MAIN_BG );
	PaletteFadeWorkAllocFree( p_wk->p_pfd, FADE_SUB_BG );
	PaletteFadeWorkAllocFree( p_wk->p_pfd, FADE_MAIN_OBJ );
	PaletteFadeWorkAllocFree( p_wk->p_pfd, FADE_SUB_OBJ );

	PaletteFadeFree( p_wk->p_pfd );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINの削除
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_ExitBmpWin( HABITAT_ZONE_GRA_WK *p_wk )
{
  // プリントキューの開放
  PRINTSYS_QUE_Clear( p_wk->p_print_que );
  PRINTSYS_QUE_Delete( p_wk->p_print_que );

  // フォント
	GFL_FONT_Delete( p_wk->p_font );

  // ビットマップウィンドウの削除
	GFL_BMPWIN_Exit();
  GFL_BMPWIN_Delete( p_wk->p_font_win );
}

//=============================================================================
//
//  パレットフェード操作
//
//=============================================================================
static void GRA_SetPalFadeSeq( HABITAT_ZONE_GRA_WK *p_wk, u8 start, u8 end )
{
	PaletteFadeReq( p_wk->p_pfd, PF_BIT_MAIN_BG,  0xbfff, ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
	PaletteFadeReq( p_wk->p_pfd, PF_BIT_SUB_BG,   0xffef, ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
	PaletteFadeReq( p_wk->p_pfd, PF_BIT_MAIN_OBJ, 0xf878, ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
	PaletteFadeReq( p_wk->p_pfd, PF_BIT_SUB_OBJ,  0xbfff, ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGフレームのスクロールシーケンス
 *
 *	@param	* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_InitFrameScroll( HABITAT_ZONE_GRA_WK *p_wk )
{
  p_wk->back_bg_scroll_wait_count = 0;
  GFL_BG_SetScrollReq( BGS_FRAME_MAP, GFL_BG_SCROLL_Y_SET, MAP_Y_SCROLL_COUNT );
}


//=============================================================================
//
//  グラフィック操作系
//
//=============================================================================
void HABITAT_ZONE_GRA_CallOpeningFade( HABITAT_ZONE_GRA_WK *p_wk )
{
	GRA_SetPalFadeSeq( p_wk, 16, 0 );

  ONOUE_Printf("colmode = %x\n",GFL_BG_GetScreenColorMode(BGS_FRAME_FONT));
  ONOUE_Printf("colmode = %x\n",GFL_BG_GetScreenColorMode(BGS_FRAME_BAR));
  ONOUE_Printf("colmode = %x\n",GFL_BG_GetScreenColorMode(BGS_FRAME_MAP));
  ONOUE_Printf("colmode = %x\n",GFL_BG_GetScreenColorMode(BGS_FRAME_NO_USE));
}

//----------------------------------------------------------------------------
/**
 *	@brief  文字描画メイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_PrintMain( HABITAT_ZONE_GRA_WK *p_wk )
{
  // ユーティリティーの更新
  int i;
  BOOL trans_end;

  // 描画待ちフラグを初期化
  p_wk->is_print_util_wati = FALSE;

  // プリントキューを更新
  PRINTSYS_QUE_Main( p_wk->p_print_que );

  // プリントユーティリティーを更新
  for( i = 0; i < PRINT_UTIL_MAX ; i++ )
  {
    trans_end = PRINT_UTIL_Trans( &p_wk->print_util[i], p_wk->p_print_que );
    p_wk->is_print_util_wati = trans_end;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  プリントユーティリティーの状況をチェック
 *
 *	@param	*p_wk 
 *
 *	@retval TRUEの時は転送が終わっている
 */
//-----------------------------------------------------------------------------
BOOL HABITAT_ZONE_GRA_CheckPrintUtilStatus( HABITAT_ZONE_GRA_WK *p_wk )
{
  return p_wk->is_print_util_wati;
}

//----------------------------------------------------------------------------
/**
 *	@brief  文字列描画
 *
 *	@param	win_id
 *	@param	str
 *	@param	*p_wk 
 *	@param	x
 *	@param	y 
 *	@param	font_colro 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_WIN win_id, const STRBUF* str,
    HABITAT_ZONE_GRA_WK *p_wk ,s16 x, s16 y ,PRINTSYS_LSB font_colro )
{
  PRINT_UTIL_PrintColor( &p_wk->print_util[win_id],
      p_wk->p_print_que , x , y ,
      str ,p_wk->p_font , font_colro);
}

////----------------------------------------------------------------------------
///**
// *	@brief  指定ウィンドウの塗りつぶし
// *
// *	@param	win_id 
// *	@param	x 
// *	@param	y
// *	@param	w 
// *	@param	h 
// *	@param	col_code 
// *	@param	*p_wk 
// *
// *	@retval
// */
////-----------------------------------------------------------------------------
//void HABITAT_ZONE_GRA_FillBmpWin( PRINT_UTIL_WIN win_id ,u8 x ,u8 y, u8 w ,u8 h ,u8 col_code ,HABITAT_ZONE_GRA_WK *p_wk )
//{
//  
//  GFL_BMPWIN *p_win;
//  u8 frame;
//  switch( win_id )
//  {
//  case PRINT_UTIL_INFO_WIN:
//      p_win = p_wk->p_font_win;
//    break;
//
//  default:
//    GF_ASSERT(0);
//  }
//
//  // 塗りつぶして転送
//  frame = GFL_BMPWIN_GetFrame( p_win );
//  GFL_BMP_Fill( GFL_BMPWIN_GetBmp(p_win) ,x ,y ,w ,h ,col_code );
//  GFL_BMPWIN_TransVramCharacter( p_win );
//  GFL_BG_LoadScreenV_Req(frame);
//}

//----------------------------------------------------------------------------
/**
 *	@brief  指定ウィンドウをクリアする
 *
 *	@param	win_id 
 *	@param	col_code 
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_BmpFil( PRINT_UTIL_WIN win_id ,u8 col_code ,HABITAT_ZONE_GRA_WK *p_wk )
{
  
  GFL_BMPWIN *p_win;
  u8 frame;
  switch( win_id )
  {
  case PRINT_UTIL_INFO_WIN:
      p_win = p_wk->p_font_win;
    break;

  default:
    GF_ASSERT(0);
  }

  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_win) ,col_code );
}

//----------------------------------------------------------------------------
/**
 *	@brief  セルアクターワークを取得する（コモンリソース用）
 *
 *	@param	*p_wk 
 *	@param	obj_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_CLWK* HABITAT_ZONE_GRA_GetComClWk( HABITAT_ZONE_GRA_WK *p_wk ,s32 obj_id )
{
  return p_wk->p_clwk_common_obj[ obj_id ];
}

