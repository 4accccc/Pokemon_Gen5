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
#include "gamesystem/game_data.h"

#include "system/main.h"  // for HEAP_ID

#include "system/wipe.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "gamesystem\msgspeed.h"

#include "habitat_zone_gra.cdat"

#include "../zukan_common.h" // for ZKNCOMM_FADE_WAIT

#include "arc_def.h"        // ARCID_
#include "zukan_gra.naix"
#include "font/font.naix"
#include "townmap_gra.naix"

#include "../../../../../resource/fldmapdata/zonetable/zone_id.h"

#include "system/bmp_winframe.h"      // BMPウィンドウフレーム

//ポケアイコン
#include "pokeicon/pokeicon.h"

#include "app/townmap_util.h" // for TOWNMAP_UTIL_*


FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(townmap);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

// ＢＧパレット番号
#define APP_WINFRAME_USE_PALETE       ( 12 )      // ウィンドウフレーム用パレット
#define	HABITAT_ZONE_MBG_PAL_FONT			( 15 )		// メイン画面フォントパレット
#define	HABITAT_ZONE_SBG_PAL_FONT			( 15 )		// サブ画面フォントパレット
#define	HABITAT_ZONE_TITLE_PAL_FONT		( 4 )		// サブ画面フォントパレット

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
static void GRA_InitBlinkAnm( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_InitFrameScroll( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_InitBmpWin( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_HideSpotInit( HABITAT_ZONE_GRA_WK* p_wk );

// 終了系
static void GRA_ResetWindow( void );
static void GRA_ExitPaletteFade( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_ExitBg( void );
static void GRA_ExitObj( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_ExitBmpWin( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_ExitBlinkAnm( HABITAT_ZONE_GRA_WK *p_wk );

// 処理系
static void GRA_PrintMain( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_BlinkAnmMain( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_GetPokeIconPos( u8 index ,GFL_CLACTPOS *p_pos );
static void GRA_AlpaMain( HABITAT_ZONE_GRA_WK* p_wk );
static GFL_CLWK* GRA_LoadPokeIcon( HABITAT_ZONE_GRA_WK *p_wk ,u32 mons_no ,u32 form_no, u32 sex,u8 index );
static void GRA_ObjExistAlphaReset( HABITAT_ZONE_GRA_WK* p_wk );
static GFL_BMPWIN* GetBmpWin( HABITAT_ZONE_GRA_WK *p_wk ,PRINT_UTIL_WIN win_id);
static void GRA_StreamMain( HABITAT_ZONE_GRA_WK *p_wk );
static void GRA_ReLoadBlinkAnm( HABITAT_ZONE_GRA_WK *p_wk ,HABITAT_GROUND_TYPE type );
static void GRA_TrancePokeIcon( HABITAT_ZONE_GRA_WK *p_wk ,u8 index );

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
void HABITAT_ZONE_GRA_Init( HABITAT_ZONE_GRA_WK *p_wk ,GAMEDATA *p_gdata )
{
  
  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  GFL_OVERLAY_Load( FS_OVERLAY_ID(townmap) );
 
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
 
  // タウンマップデータ
  p_wk->p_townmap_data = TOWNMAP_DATA_Alloc( HEAPID_ZUKAN_HABITAT_ZONE );

  // アーカイブハンドル取得
	p_wk->p_pokeicon_ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_ZUKAN_HABITAT_ZONE );

  // ゲームデータを保存
  p_wk->p_gdata = p_gdata;
  p_wk->is_stop_blink = FALSE;
  
  // 初期化
  GRA_InitVram();           // VRAM
  GRA_InitBg();             // BG
  GRA_InitObj( p_wk );      // OBJ
  GRA_InitBmpWin( p_wk );   // BMPWIN
  GRA_LoadObjGraphic( p_wk );     // OBJ初期化
  GRA_LoadBgGraphic();            // BGリソース読み込み
  GRA_InitPaletteFade( p_wk );    // パレットフェード
  GRA_InitBlinkAnm( p_wk );       // 点滅アニメ
  GRA_InitFrameScroll( p_wk );    // スクロール
  GRA_HideSpotInit( p_wk );       // 隠しマップ
 
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

  // タウンマップデータ
  TOWNMAP_DATA_Free( p_wk->p_townmap_data );

  // ポケモンアイコングラフィックのアーカイブハンドル
  GFL_ARC_CloseDataHandle( p_wk->p_pokeicon_ah );

  // BMPWINの開放
  GRA_ExitBmpWin( p_wk );

  // OBJの終了処理
  GRA_ExitObj( p_wk );
  
  // BGの終了処理
	GRA_ExitBg();

  // 点滅アニメ終了
  GRA_ExitBlinkAnm( p_wk );
  
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
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(townmap) );
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
  s32 index;
	GFL_CLACT_SYS_VBlankFunc();

  // 転送待ちポケモンアイコンを転送
  for( index = 0; index < HABITAT_ZONE_POKE_LIST_MAX; index++ )
  {
    if(p_wk->icon_trance_data[index].is_trance == TRUE )
    {
      GRA_TrancePokeIcon( p_wk ,index);
      p_wk->icon_trance_data[index].is_trance = FALSE;
    }
  }

  // パレットの転送
  if( p_wk->is_trance_pal_col== TRUE)
  {
    // パレット転送
    switch( p_wk->set_trance_traypal_type )
    {
    case HABITAT_GROUND_LAND:
      GFL_ARC_UTIL_TransVramPaletteEx(
          ARCID_ZUKAN_GRA, NARC_zukan_gra_list_list_bgd_NCLR, PALTYPE_MAIN_BG,
          TRAY_PAL_LAND_START*0x20, TRAY_PAL_START*0x20,
          TRAY_PAL_NUM*0x20 ,HEAPID_ZUKAN_HABITAT_ZONE );
      break;
    case HABITAT_GROUND_WATERSIDE:
      GFL_ARC_UTIL_TransVramPaletteEx(
          ARCID_ZUKAN_GRA, NARC_zukan_gra_list_list_bgd_other_NCLR, PALTYPE_MAIN_BG,
          TRAY_PAL_WATERSIDE_START*0x20, TRAY_PAL_START*0x20,
          TRAY_PAL_NUM*0x20 ,HEAPID_ZUKAN_HABITAT_ZONE );
      break;
    case HABITAT_GROUND_FISHING:
      GFL_ARC_UTIL_TransVramPaletteEx(
          ARCID_ZUKAN_GRA, NARC_zukan_gra_list_list_bgd_other_NCLR, PALTYPE_MAIN_BG,
          TRAY_PAL_FISHING_START*0x20, TRAY_PAL_START*0x20,
          TRAY_PAL_NUM*0x20 ,HEAPID_ZUKAN_HABITAT_ZONE );
      break;
    default:
      GF_ASSERT(0);
      break;
    }

    // 点滅アニメ読み込み
    GRA_ReLoadBlinkAnm( p_wk ,p_wk->set_trance_traypal_type );
  
    // フェードパレット再読み込み
    PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_MAIN_BG, 0, FADE_PAL_ALL_SIZE );
    PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_SUB_BG, 0, FADE_PAL_ALL_SIZE );
    PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_MAIN_OBJ, 0, FADE_PAL_ALL_SIZE );
    PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_SUB_OBJ, 0, FADE_PAL_ALL_SIZE );

    // 転送終了
    p_wk->is_trance_pal_col = FALSE;
  }

  PaletteFadeTrans( p_wk->p_pfd );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコン転送終了チェック
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL HABITAT_ZONE_GRA_CheckPokeIconTranceEnd( HABITAT_ZONE_GRA_WK *p_wk )
{
  s32 index;
  // ポケモンアイコンが全て転送済みかチェック
  for( index = 0; index < HABITAT_ZONE_POKE_LIST_MAX; index++ )
  {
    if(p_wk->icon_trance_data[index].is_trance == TRUE )
    {
      return FALSE;
    }
  }
  return TRUE;
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
 
  if( p_wk->is_stop_blink == FALSE )
  {
    // カーソル点滅アニメ
    GRA_BlinkAnmMain( p_wk );
  }

  // アルファアニメーション
  GRA_AlpaMain( p_wk );
  
  // プリントストリームの更新
  if( p_wk->p_print_stream != NULL )
  {
    GRA_StreamMain( p_wk );
  }

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

//----------------------------------------------------------------------------
/**
 *	@brief  ストリーム更新
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_StreamMain( HABITAT_ZONE_GRA_WK *p_wk )
{
  BOOL ret;

  if( p_wk->p_event_win_sub != NULL )
  {
    // タスクの更新
    GFL_TCBL_Main( p_wk->p_tcb_sys );

    // キーカーソルの更新
    APP_KEYCURSOR_Main(p_wk->p_cursor,p_wk->p_print_stream ,p_wk->p_event_win_sub );

    // ストリームの更新
    ret = APP_PRINTSYS_COMMON_PrintStreamFunc( &p_wk->print_sys_wk , p_wk->p_print_stream );

    if( ret == TRUE )
    {
      if( p_wk->is_cursor_draw == TRUE )
      {
        APP_KEYCURSOR_Write( p_wk->p_cursor , GFL_BMPWIN_GetBmp( p_wk->p_event_win_sub ) , 0xF );
        GFL_BMPWIN_TransVramCharacter( p_wk->p_event_win_sub );
      }
    }
  }
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
  u16 col = 0;
  GXS_LoadBGPltt( &col ,0 ,sizeof(col) ); // バックドロップを黒にしておく

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

	{	// イベント用メッセージウィンドウ
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGM_FRAME_EVMSG_WIN, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGM_FRAME_EVMSG_WIN );
		GFL_BG_LoadScreenV_Req( BGM_FRAME_EVMSG_WIN );
    GFL_BG_SetVisible( BGM_FRAME_EVMSG_WIN ,VISIBLE_ON );
    GFL_BG_FillCharacter( BGM_FRAME_EVMSG_WIN, 0, 1, 0 );
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

  // 上画面 --------------------------------------------------------------
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
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, 0x8000,
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
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_23, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGS_FRAME_MAP, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGS_FRAME_MAP );
		GFL_BG_LoadScreenV_Req( BGS_FRAME_MAP );
    GFL_BG_SetVisible( BGS_FRAME_MAP ,VISIBLE_ON );
	}

	{	// ルート面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x1c000, 0x8000,
			GX_BG_EXTPLTT_23, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGS_FRAME_ROOT, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGS_FRAME_ROOT );
		GFL_BG_LoadScreenV_Req( BGS_FRAME_ROOT );
    GFL_BG_SetVisible( BGS_FRAME_ROOT ,VISIBLE_ON );
	}

  // 表示状態変更
	GFL_DISP_GX_SetVisibleControl(
		BGM_FRAME_FONT | BGM_FRAME_EVMSG_WIN | BGM_FRAME_BACK | BGM_FRAME_SCROLL_WIN
      ,VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		BGS_FRAME_FONT | BGS_FRAME_BAR | BGS_FRAME_MAP | BGS_FRAME_ROOT , VISIBLE_ON );
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

  // プリントキューの作成
  p_wk->p_print_que = PRINTSYS_QUE_Create( HEAPID_ZUKAN_HABITAT_ZONE );

  // メニュー用BMPWIN作成
  {
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

    // プリントユーティリティーの準備
    PRINT_UTIL_Setup( &p_wk->print_util[ PRINT_UTIL_INFO_WIN ] ,p_wk->p_font_win  );
  }

  // タイトル用BMPWIN作成
  {
    p_wk->p_font_win_sub = GFL_BMPWIN_Create( BGS_FRAME_FONT,
        0   , 0   ,
        32  , 24  ,
        HABITAT_ZONE_TITLE_PAL_FONT , GFL_BMP_CHRAREA_GET_F );

    // スクリーンの作成
    GFL_BMPWIN_MakeScreen( p_wk->p_font_win_sub );

    // ウィンドウのクリアー
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_font_win_sub), 0 );

    // 転送
    GFL_BMPWIN_TransVramCharacter( p_wk->p_font_win_sub );

    // 転送リクエスト
    GFL_BG_LoadScreenV_Req( BGS_FRAME_FONT );
    
    // プリントユーティリティーの準備
    PRINT_UTIL_Setup( &p_wk->print_util[ PRINT_UTIL_MAP_NAME ] ,
                                                  p_wk->p_font_win_sub  );
  }

  // 達成イベント用BMPWINクリア
  p_wk->p_event_win_sub = NULL;
  
	// フォント
  p_wk->p_font	= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

  // プリントシステムの初期化
  APP_PRINTSYS_COMMON_PrintStreamInit( &p_wk->print_sys_wk , APP_PRINTSYS_COMMON_TYPE_BOTH );
  
  // プリントシステム用タスクの生成
  p_wk->p_tcb_sys = GFL_TCBL_Init( HEAPID_ZUKAN_HABITAT_ZONE,HEAPID_ZUKAN_HABITAT_ZONE , 1 , 0 );
  
  // カーソルの作成
  p_wk->p_cursor = APP_KEYCURSOR_Create( 0xf , TRUE , TRUE , HEAPID_ZUKAN_HABITAT_ZONE );
 
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
        ah, NARC_zukan_gra_habitat_list_habitatbase_bgd_NSCR,
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
    // 拡張パレットに転送(スロット３にMAPパレット)
    GFL_ARCHDL_UTIL_TransVramPalette(
        ah, NARC_townmap_gra_tmap_bg_d_NCLR, PALTYPE_SUB_BG_EX,0x6000, 0,HEAPID_ZUKAN_HABITAT_ZONE );

    // 地図グラフィック
    // キャラクタ
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_townmap_gra_tmap_map_d_NCGR, BGS_FRAME_MAP, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );
    // スクリーン
    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_townmap_gra_tmap_map_d_NSCR,
        BGS_FRAME_MAP, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

    // ルート
    // キャラクタ
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_townmap_gra_tmap_bg_d_NCGR, BGS_FRAME_ROOT, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );
    // スクリーン
    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_townmap_gra_tmap_root_d_NSCR,
        BGS_FRAME_ROOT, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_ZONE );

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
  s32 i;

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
    prm.pltt_line = COMMON_PAL_START;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num  = COMMON_PAL_NUM;

    UI_EASY_CLWK_LoadResource( &p_wk->clres_common_obj, &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_ZONE );
  }

  // アプリ共通素材読読み込み
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = APP_COMMON_GetArcId();
    prm.pltt_id   = APP_COMMON_GetBarIconPltArcIdx();
    prm.ncg_id    = APP_COMMON_GetBarIconCharArcIdx();
    prm.cell_id   = APP_COMMON_GetBarIconCellArcIdx( APP_COMMON_MAPPING_128K ); 
    prm.anm_id    = APP_COMMON_GetBarIconAnimeArcIdx( APP_COMMON_MAPPING_128K );
    prm.pltt_line = APPCOM_PAL_START;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num  = APPCOM_PAL_NUM;

    UI_EASY_CLWK_LoadResource( &p_wk->clres_app_menu_obj, &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_ZONE );
  }

  // 上画面リソース読み込み
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_SUB;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = ARCID_ZUKAN_GRA;
    prm.pltt_id   = NARC_zukan_gra_info_map_obj_NCLR;
    prm.ncg_id    = NARC_zukan_gra_info_map_obj_NCGR;
    prm.cell_id   = NARC_zukan_gra_info_map_obj_NCER; 
    prm.anm_id    = NARC_zukan_gra_info_map_obj_NANR;
    prm.pltt_line = 0;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num  = 1;

    UI_EASY_CLWK_LoadResource( &p_wk->clres_root_obj, &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_ZONE );
  }

  // タウンマップリソース読み込み
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_SUB;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = ARCID_TOWNMAP_GRAPHIC;
    prm.pltt_id   = NARC_townmap_gra_tmap_obj_NCLR;
    prm.ncg_id    = NARC_townmap_gra_tmap_obj_d_NCGR;
    prm.cell_id   = NARC_townmap_gra_tmap_obj_d_NCER; 
    prm.anm_id    = NARC_townmap_gra_tmap_obj_d_NANR;
    prm.pltt_line = 0x1;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num  = 0x05;

    UI_EASY_CLWK_LoadResource( &p_wk->clres_tmap_obj, &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_ZONE );
  }

  // セルアクターワークを作成
  {
    // 季節Lボタン
    p_wk->p_clwk_common_obj[OBJ_WK_SEASON_L] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_L_X, OBJ_POS_L_Y, OBJ_ANM_SEQ_L, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[OBJ_WK_SEASON_L] ,1 );
    GFL_CLACT_WK_SetAutoAnmFlag(p_wk->p_clwk_common_obj[OBJ_WK_SEASON_L],TRUE );

    // 季節Rボタン
    p_wk->p_clwk_common_obj[OBJ_WK_SEASON_R] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_R_X, OBJ_POS_R_Y, OBJ_ANM_SEQ_R, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[OBJ_WK_SEASON_R] ,1 );
    GFL_CLACT_WK_SetAutoAnmFlag(p_wk->p_clwk_common_obj[OBJ_WK_SEASON_R],TRUE );

    // メニューバー
    p_wk->p_clwk_common_obj[OBJ_WK_MENU_BAR] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      MENU_BAR_POS_X, MENU_BAR_POS_Y, OBJ_ANM_SEQ_MENU_BAR, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[OBJ_WK_MENU_BAR] ,1 );
    GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk_common_obj[OBJ_WK_MENU_BAR] ,1 );

    // 大きなスタンプ
    p_wk->p_clwk_common_obj[OBJ_WK_STAMP_BIG] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_STAMP_BIG_X, OBJ_POS_STAMP_BIG_Y,
      OBJ_ANM_SEQ_STAMP_BIG_LOOK, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_CLACT_WK_SetAutoAnmFlag(p_wk->p_clwk_common_obj[OBJ_WK_STAMP_BIG],FALSE );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_common_obj[OBJ_WK_STAMP_BIG],FALSE );
    GFL_CLACT_WK_SetAffineParam( p_wk->p_clwk_common_obj[OBJ_WK_STAMP_BIG], CLSYS_AFFINETYPE_DOUBLE );
    

    // イベントアイコン
    for( i = OBJ_WK_EVENT_ICON_01; i <= OBJ_WK_EVENT_ICON_03 ; i++ )
    {
      u8 pos_x = ((i-OBJ_WK_EVENT_ICON_01)*(OBJ_POS_BUTTON_WATERSIDE_X-OBJ_POS_BUTTON_LAND_X));
      p_wk->p_clwk_common_obj[i] = UI_EASY_CLWK_CreateCLWK( 
          &p_wk->clres_common_obj, p_wk->p_clunit, 
          OBJ_POS_BUTTON_LAND_X + pos_x + EVENT_ICON_POS_OFS_X,
          OBJ_POS_BUTTON_LAND_Y + EVENT_ICON_POS_OFS_Y,
          OBJ_ANM_SEQ_EVENT_ICON, HEAPID_ZUKAN_HABITAT_ZONE );
    }

    // 陸地ボタン
    p_wk->p_clwk_common_obj[OBJ_WK_BUTTON_LAND] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_BUTTON_LAND_X, OBJ_POS_BUTTON_LAND_Y,
      OBJ_ANM_SEQ_BUTTON_LAND, HEAPID_ZUKAN_HABITAT_ZONE );

    // 水辺ボタン
    p_wk->p_clwk_common_obj[OBJ_WK_BUTTON_WATERSIDE] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_BUTTON_WATERSIDE_X, OBJ_POS_BUTTON_WATERSIDE_Y,
      OBJ_ANM_SEQ_BUTTON_WATERSIDE, HEAPID_ZUKAN_HABITAT_ZONE );

    // 釣りボタン
    p_wk->p_clwk_common_obj[OBJ_WK_BUTTON_FISHING] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_BUTTON_FISHING_X, OBJ_POS_BUTTON_FISHING_Y,
      OBJ_ANM_SEQ_BUTTON_FISHING, HEAPID_ZUKAN_HABITAT_ZONE );

    // ポケアイコン用見た、捕獲マーク
    for( i = 0; i < HABITAT_ZONE_POKE_LIST_MAX ; i++ )
    {
      GFL_CLACTPOS pos;
      GRA_GetPokeIconPos( i ,&pos );
      p_wk->p_clwk_common_obj[OBJ_WK_STAMP_SMALL_START+i]=UI_EASY_CLWK_CreateCLWK( 
          &p_wk->clres_common_obj, p_wk->p_clunit, 
          pos.x + SMALL_ICON_OFFSET_X, pos.y + SMALL_ICON_OFFSET_Y,
          OBJ_ANM_SEQ_STAMP_SMALL_GET, HEAPID_ZUKAN_HABITAT_ZONE );
      GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[OBJ_WK_STAMP_SMALL_START+i] ,1 );
    }

    // 説明を見るボタン(フォント面より下においておく)
    p_wk->p_clwk_common_obj[OBJ_WK_BUTTON_EXPLANATION] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      OBJ_POS_BUTTON_EXPLANATION_X, OBJ_POS_BUTTON_EXPLANATION_Y,
      OBJ_ANM_SEQ_BUTTON_EXPLANATION, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_CLACT_WK_SetDrawEnable( 
        p_wk->p_clwk_common_obj[OBJ_WK_BUTTON_EXPLANATION],FALSE);
    GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[OBJ_WK_BUTTON_EXPLANATION] ,1 );
 
    // 主人公アイコン
    p_wk->p_clwk_common_obj[OBJ_WK_MAP_HERO] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_tmap_obj, p_wk->p_clunit, 
      0, 0,
      OBJ_ANM_SEQ_HERO_ICON, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_CLACT_WK_SetDrawEnable( 
        p_wk->p_clwk_common_obj[OBJ_WK_MAP_HERO],FALSE);
    GFL_CLACT_WK_SetAutoAnmFlag(p_wk->p_clwk_common_obj[OBJ_WK_MAP_HERO],TRUE );
 
    // マップ上点滅アイコン(ゾーン情報にあわせて、マップ情報を切り替える)
    p_wk->p_clwk_common_obj[OBJ_WK_MAP_BLINK] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_root_obj, p_wk->p_clunit, 
      0, 0, 0, HEAPID_ZUKAN_HABITAT_ZONE );
    GFL_CLACT_WK_SetObjMode(p_wk->p_clwk_common_obj[OBJ_WK_MAP_BLINK],GX_OAM_MODE_XLU);

    // 共通素材系読み込み ------------------------------------------------
    
    // 上矢印
    p_wk->p_clwk_common_obj[OBJ_WK_UP_ARROW] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      OBJ_POS_BUTTON_UP_ARROW_X, OBJ_POS_BUTTON_UP_ARROW_Y,
      APP_COMMON_BARICON_CURSOR_UP, HEAPID_ZUKAN_HABITAT_ZONE );
    
    // 下矢印
    p_wk->p_clwk_common_obj[OBJ_WK_DOWN_ARROW] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      OBJ_POS_BUTTON_DOWN_ARROW_X, OBJ_POS_BUTTON_DOWN_ARROW_Y,
      APP_COMMON_BARICON_CURSOR_DOWN, HEAPID_ZUKAN_HABITAT_ZONE );
 
    // チェックボックス
    p_wk->p_clwk_common_obj[OBJ_WK_CHECK_BOX] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      OBJ_POS_BUTTON_CHECK_X, OBJ_POS_BUTTON_CHECK_Y,
      APP_COMMON_BARICON_CHECK_OFF, HEAPID_ZUKAN_HABITAT_ZONE );
 
    // キャンセル
    p_wk->p_clwk_common_obj[OBJ_WK_CANCEL] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      OBJ_POS_BUTTON_CANCEL_X, OBJ_POS_BUTTON_CANCEL_Y,
      APP_COMMON_BARICON_EXIT, HEAPID_ZUKAN_HABITAT_ZONE );
 
    // 戻る
    p_wk->p_clwk_common_obj[OBJ_WK_RETURN] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      OBJ_POS_BUTTON_RETURN_X, OBJ_POS_BUTTON_RETURN_Y,
      APP_COMMON_BARICON_RETURN, HEAPID_ZUKAN_HABITAT_ZONE );

  }

  // ポケモンアイコンロード
  for( i = 0; i < HABITAT_ZONE_POKE_LIST_MAX; i++ )
  {
    // リソース読み込み
    GFL_CLACTPOS pos;
    GFL_CLWK *p_clwk = GRA_LoadPokeIcon( p_wk , 0,0,0,i );
    GRA_GetPokeIconPos( i ,&pos );
    GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DEFREND_MAIN );
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
static GFL_CLWK* GRA_LoadPokeIcon( HABITAT_ZONE_GRA_WK *p_wk ,u32 mons_no , u32 form_no, u32 sex,u8 index )
{
  // ポケアイコン本体
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NCLR;
    prm.arc_id    = ARCID_POKEICON;
    prm.pltt_id   = POKEICON_GetPalArcIndex();
    prm.ncg_id    = POKEICON_GetCgxArcIndexByMonsNumber( mons_no, form_no, sex, FALSE );
    ONOUE_Printf("フォルム%d\n",form_no);
    prm.cell_id   = POKEICON_GetCellArcIndex(); 
    prm.anm_id    = POKEICON_GetAnmArcIndex();
    prm.pltt_line = POKE_PAL_START;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num = POKE_PAL_NUM;
    
    UI_EASY_CLWK_LoadResource( &p_wk->clres_poke_icon[index], &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_ZONE );

    // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
    p_wk->p_clwk_poke_icon[index] = UI_EASY_CLWK_CreateCLWK( &p_wk->clres_poke_icon[index], p_wk->p_clunit, 0, 0, 1, HEAPID_ZUKAN_HABITAT_ZONE );

    // 上にアイテムアイコンを描画するので優先度を下げておく
    GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk_poke_icon[index], HABITAT_ZONE_OBJPRI_POKEICON );
    GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_poke_icon[index] ,1 );

    // オートアニメ OFF
    GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk_poke_icon[index], FALSE );

    {
      u8 pal_num = POKEICON_GetPalNum( mons_no, form_no, sex, FALSE );
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
  
  PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_MAIN_BG, 0, FADE_PAL_ALL_SIZE );
  PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_SUB_BG, 0, FADE_PAL_ALL_SIZE );
  PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_MAIN_OBJ, 0, FADE_PAL_ALL_SIZE );
	PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_SUB_OBJ, 0, FADE_PAL_ALL_SIZE );

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
    if( p_wk->p_clwk_poke_icon[i] != NULL )
    {
      UI_EASY_CLWK_UnLoadResource( &p_wk->clres_poke_icon[i] );
    }
  }
  UI_EASY_CLWK_UnLoadResource( &p_wk->clres_common_obj );
  UI_EASY_CLWK_UnLoadResource( &p_wk->clres_tmap_obj );

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
 *	@brief  点滅アニメーションの終了
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_ExitBlinkAnm( HABITAT_ZONE_GRA_WK *p_wk )
{
  // 終了
  BLINKPALANM_Exit( p_wk->p_blinkanm_wk );
  BLINKPALANM_Exit( p_wk->p_blinkanm_wk_land_icon );
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

  // カーソル削除
  APP_KEYCURSOR_Delete( p_wk->p_cursor );

  // タスクの終了
  GFL_TCBL_Exit( p_wk->p_tcb_sys );

  // ビットマップウィンドウの削除
	GFL_BMPWIN_Exit();
  GFL_BMPWIN_Delete( p_wk->p_font_win );
  GFL_BMPWIN_Delete( p_wk->p_font_win_sub );
}

//=============================================================================
//
//  パレットフェード操作
//
//=============================================================================
void HABITAT_ZONE_GRA_CallFade( HABITAT_ZONE_GRA_WK *p_wk, u8 start, u8 end )
{
	PaletteFadeReq( p_wk->p_pfd, PF_BIT_MAIN_BG,  0xbfff, ZKNCOMM_FADE_WAIT,
                                        start, end, 0, GFUser_VIntr_GetTCBSYS() );
	PaletteFadeReq( p_wk->p_pfd, PF_BIT_SUB_BG,   0xffef, 
      ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
	//PaletteFadeReq( p_wk->p_pfd, PF_BIT_MAIN_OBJ, 0xf8bf, 
	PaletteFadeReq( p_wk->p_pfd, PF_BIT_MAIN_OBJ, 0x38b8, 
      ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
	PaletteFadeReq( p_wk->p_pfd, PF_BIT_SUB_OBJ,  0x3fff, 
	ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
}

//----------------------------------------------------------------------------
/**
 *	@brief パレットフェード終了チェック
 *
 *	@param	*p_wk 
 *
 * @retval  "0 = 終了"
 */
//-----------------------------------------------------------------------------
u16 HABITAT_ZONE_GRA_CheckFade( HABITAT_ZONE_GRA_WK *p_wk )
{
	return PaletteFadeCheck( p_wk->p_pfd );
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
  GFL_BG_SetScrollReq( BGS_FRAME_ROOT, GFL_BG_SCROLL_Y_SET, MAP_Y_SCROLL_COUNT );
}

//----------------------------------------------------------------------------
/**
 *	@brief  点滅アニメーション初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_InitBlinkAnm( HABITAT_ZONE_GRA_WK *p_wk )
{
  ARCHANDLE * ah;

  // 図鑑グラフィックアーカイブ
  ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_HABITAT_ZONE );
	
  // 点滅させるパレットの指定
  p_wk->p_blinkanm_wk = BLINKPALANM_Create( BLINK_PALID_POKE_CURSOR_CHANGE*16  ,
        BLINK_PAL_SIZE_POKE_CURSOR , BGM_FRAME_BACK, HEAPID_ZUKAN_HABITAT_ZONE );

  // パレットアニメデータの作成
  BLINKPALANM_SetPalBufferArcHDL( p_wk->p_blinkanm_wk, ah,
                    NARC_zukan_gra_list_list_bgd_NCLR, 
                    BLINK_PALID_POKE_CURSOR_START*16, 
                    BLINK_PALID_POKE_CURSOR_END*16 );
 	
  // 点滅させるパレットの指定
  p_wk->p_blinkanm_wk_land_icon = BLINKPALANM_Create( 
      BLINK_PALID_LAND_ICON_CHANGE*16  ,
      BLINK_PAL_SIZE_LAND_ICON , BLINKPALANM_MODE_MAIN_OBJ, HEAPID_ZUKAN_HABITAT_ZONE );

  // パレットアニメデータの作成
  BLINKPALANM_SetPalBufferArcHDL( p_wk->p_blinkanm_wk_land_icon, ah,
                    NARC_zukan_gra_list_list_objd_NCLR, 
                    BLINK_PALID_LAND_ICON_START*16, 
                    BLINK_PALID_LAND_ICON_END*16 );
       
  // アーカイブクローズ
  GFL_ARC_CloseDataHandle( ah );
	
}


//=============================================================================
//
//  グラフィック操作系
//
//=============================================================================

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
  p_wk->is_print_util_wati = TRUE;

  // プリントキューを更新
  PRINTSYS_QUE_Main( p_wk->p_print_que );

  // プリントユーティリティーを更新
  for( i = 0; i < PRINT_UTIL_MAX ; i++ )
  {
    trans_end = PRINT_UTIL_Trans( &p_wk->print_util[i], p_wk->p_print_que );
    if( trans_end == FALSE )
    {
      p_wk->is_print_util_wati = FALSE;
      break;
    }
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
 *	@param	STRBUF* str
 *	@param	*p_wk 
 *	@param	x
 *	@param	y 
 *	@param	w
 *	@param	h 
 *	@param	font_colro 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_DrawStr( PRINT_UTIL_WIN win_id, const STRBUF* str,
    HABITAT_ZONE_GRA_WK *p_wk ,s16 x, s16 y ,s16 w, s16 h ,PRINTSYS_LSB font_colro )
{
  GFL_BMPWIN *p_win = GetBmpWin( p_wk,win_id );
	
  // 塗りつぶし
  GFL_BMP_Fill( GFL_BMPWIN_GetBmp(p_win) ,x ,y ,w ,h ,0 );

  // 文字描画
  PRINT_UTIL_PrintColor( &p_wk->print_util[win_id],
      p_wk->p_print_que , x , y ,
      str ,p_wk->p_font , font_colro);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ウィンドの指定領域をクリアする
 *
 *	@param	win_id 
 *	@param	*p_wk 
 *	@param	x
 *	@param	y 
 *	@param	w
 *	@param	h 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_ClearWin( PRINT_UTIL_WIN win_id ,HABITAT_ZONE_GRA_WK *p_wk ,s16 x, s16 y ,s16 w, s16 h )
{
  GFL_BMPWIN *p_win = GetBmpWin( p_wk,win_id );
	
  // 塗りつぶし
  GFL_BMP_Fill( GFL_BMPWIN_GetBmp(p_win) ,x ,y ,w ,h ,0 );

  
  // 転送
  GFL_BMPWIN_TransVramCharacter( p_win );
}

//----------------------------------------------------------------------------
/**
 *	@brief  点滅アニメーションメイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_BlinkAnmMain( HABITAT_ZONE_GRA_WK *p_wk )
{
  // カーソル点滅アニメーション
  BLINKPALANM_Main( p_wk->p_blinkanm_wk );
  BLINKPALANM_Main( p_wk->p_blinkanm_wk_land_icon );
}

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
  // クリア
  GFL_BMPWIN *p_win = GetBmpWin( p_wk,win_id );
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

//----------------------------------------------------------------------------
/**
 *	@brief  地形アイコンのパレットを書き換える
 *
 *	@param	*p_wk 
 *	@param	icon_type
 *	@param	status 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetLnadTypeIcon( HABITAT_ZONE_GRA_WK *p_wk ,
                      GRA_LANDICON_TYPE icon_type, GRA_LANDICON_STATUS status )
{
  // セルアクターワーク
  GFL_CLWK *p_clwk;

  // 状態ごとのボタンON/OFF状態
  BOOL is_button[] =
  {
    TRUE,
    TRUE,
    FALSE,
  };
  // 状態ごとのボタンアニメーションシーケンス
  s32 anm_seq[GRA_LANDICON_TYPE_MAX][3] =
  {
    // 陸上
    {
      OBJ_ANM_SEQ_BUTTON_LAND_ANM,
      OBJ_ANM_SEQ_BUTTON_LAND,
      OBJ_ANM_SEQ_BUTTON_LAND,
    },
    // 水辺
    {
      OBJ_ANM_SEQ_BUTTON_WATERSIDE_ANM,
      OBJ_ANM_SEQ_BUTTON_WATERSIDE,
      OBJ_ANM_SEQ_BUTTON_WATERSIDE,
    },
    // 釣り
    {
      OBJ_ANM_SEQ_BUTTON_FISHING_ANM,
      OBJ_ANM_SEQ_BUTTON_FISHING,
      OBJ_ANM_SEQ_BUTTON_FISHING,
    },
  };
  // アイコンタイプごとに書き換え位置を変える
  switch( icon_type )
  {
  case GRA_LANDICON_TYPE_LAND:
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( p_wk ,OBJ_WK_BUTTON_LAND );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,is_button[ status ]);
    GFL_CLACT_WK_SetAnmSeq( p_clwk , anm_seq[GRA_LANDICON_TYPE_LAND][status]);
    GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
    break;

  case GRA_LANDICON_TYPE_WATERSIDE:
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( p_wk ,OBJ_WK_BUTTON_WATERSIDE );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,is_button[ status ]);
    GFL_CLACT_WK_SetAnmSeq( p_clwk , anm_seq[GRA_LANDICON_TYPE_WATERSIDE][status]);
    GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
    break;

  case GRA_LANDICON_TYPE_FISHING:
    p_clwk = HABITAT_ZONE_GRA_GetComClWk( p_wk ,OBJ_WK_BUTTON_FISHING );
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,is_button[ status ]);
    GFL_CLACT_WK_SetAnmSeq( p_clwk , anm_seq[GRA_LANDICON_TYPE_FISHING][status]);
    GFL_CLACT_WK_SetAnmFrame( p_clwk ,0);
    break;

  default:
    GF_ASSERT(0);
    break;
  }
  // スクリーン転送
	GFL_BG_LoadScreenV_Req( BGM_FRAME_BACK );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンをセットする
 *
 *	@param	*p_wk 
 *	@param	monsno 
 *	@param	index 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetPokeIcon( HABITAT_ZONE_GRA_WK *p_wk ,u16 monsno ,u32 form_no, u32 sex,u8 index )
{
  p_wk->icon_trance_data[index].is_trance = TRUE;
  p_wk->icon_trance_data[index].monsno = monsno;
  p_wk->icon_trance_data[index].form_no = form_no;
  p_wk->icon_trance_data[index].sex = sex;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンを転送する
 *
 *	@param	*p_wk 
 *	@param	index 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void GRA_TrancePokeIcon( HABITAT_ZONE_GRA_WK *p_wk ,u8 index )
{
	NNSG2dCharacterData * chr;
	void * buf;

  u32 monsno  = p_wk->icon_trance_data[index].monsno;
  u8  form_no = p_wk->icon_trance_data[index].form_no;
  u8  sex     = p_wk->icon_trance_data[index].sex;

  // キャラクターデータの読み込み
	buf = GFL_ARCHDL_UTIL_LoadOBJCharacter(
					p_wk->p_pokeicon_ah, 
          POKEICON_GetCgxArcIndexByMonsNumber(monsno,form_no,sex,FALSE),
          FALSE, &chr, HEAPID_ZUKAN_HABITAT_ZONE );

  // キャラクタの置換え
  GFL_CLGRP_CGR_ReplaceEx(
      GFL_CLACT_WK_GetCgrIndex(p_wk->p_clwk_poke_icon[index]),
      chr->pRawData, POKEICON_SIZE_CGX, 0, CLSYS_DRAW_MAIN );
  GFL_CLGRP_CGR_Replace( 
      GFL_CLACT_WK_GetCgrIndex(p_wk->p_clwk_poke_icon[index]),chr );

  GFL_CLACT_WK_SetPlttOffs(
      p_wk->p_clwk_poke_icon[index], POKEICON_GetPalNum(monsno,form_no,sex,FALSE),
      CLWK_PLTTOFFS_MODE_PLTT_TOP );

  // 開放処理
  GFL_HEAP_FreeMemory( buf );

  // 表示する
  HABITAT_ZONE_GRA_SetSetDrawEnableIcon( p_wk ,index ,TRUE);
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンのアニメ状態をセットする
 *
 *	@param	*p_wk 
 *	@param	index 
 *	@param	is_anm 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetPokeAnmFlag( HABITAT_ZONE_GRA_WK *p_wk ,u8 index ,
                                                                    BOOL is_anm )
{
  if( p_wk->p_clwk_poke_icon[index] != NULL )
  {
    GFL_CLACT_WK_StartAnm( p_wk->p_clwk_poke_icon[index] );
    GFL_CLACT_WK_SetAutoAnmFlag(p_wk->p_clwk_poke_icon[index],is_anm );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンの表示状態を設定する
 *
 *	@param	*p_wk 
 *	@param	index 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetSetDrawEnableIcon( HABITAT_ZONE_GRA_WK *p_wk ,u8 index,BOOL is_draw )
{
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_poke_icon[index],is_draw );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルの見た目状態を変更する
 *
 *	@param	*p_wk 
 *	@param	index 
 *	@param	status 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetCursorStatus( HABITAT_ZONE_GRA_WK *p_wk ,u8 index ,
                                              GRA_POKEICON_CURSOR_STATUS status )
{
  u8 sx = POKEICON_CURSOR_POS_X + (index%POKEICON_LINE)*POKEICON_CURSOR_POS_W;
  u8 sy = POKEICON_CURSOR_POS_Y + (index/POKEICON_LINE)*POKEICON_CURSOR_POS_H;

  if( status == GRA_POKEICON_CURSOR_ON )
  {
    GFL_BG_ChangeScreenPalette( BGM_FRAME_BACK ,sx, sy,
        POKEICON_CURSOR_POS_W,POKEICON_CURSOR_POS_H,POKEICON_CURSOR_ON_PALID);
  }
  else if( status == GRA_POKEICON_CURSOR_OFF )
  {
    GFL_BG_ChangeScreenPalette( BGM_FRAME_BACK ,sx, sy,
        POKEICON_CURSOR_POS_W,POKEICON_CURSOR_POS_H,POKEICON_CURSOR_OFF_PALID);
  }
  else if( status == GRA_POKEICON_CURSOR_NONE )
  {
    GFL_BG_ChangeScreenPalette( BGM_FRAME_BACK ,sx, sy,
        POKEICON_CURSOR_POS_W,POKEICON_CURSOR_POS_H,POKEICON_CURSOR_NONE_PALID);
  }
  else
  {
    GF_ASSERT(0);
  }
  // スクリーン転送
  GFL_BG_LoadScreenV_Req( BGM_FRAME_BACK );

}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンアイコンの座標を取得する
 *
 *	@param	index 
 *	@param	*p_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_GetPokeIconPos( u8 index ,GFL_CLACTPOS *p_pos )
{
  p_pos->x = POKEICON_START_X + (index%POKEICON_LINE)*POKEICON_MARGIN_W;
  p_pos->y = POKEICON_START_Y + (index/POKEICON_LINE)*POKEICON_MARGIN_Y;
}


//----------------------------------------------------------------------------
/**
 *	@brief  マップ点滅処理
 *
 *	@param	*p_wk 
 *	@param	zone_group 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetMapBlink( HABITAT_ZONE_GRA_WK *p_wk ,u16 zone_group )
{
  GFL_CLACTPOS pos;

  u16 root_zone = TOWNMAP_UTIL_GetRootZoneID(p_wk->p_gdata, zone_group );
  u16 dataIndex = TOWNMAP_DATA_SearchRootZoneID( p_wk->p_townmap_data ,root_zone );

  // アニメーションシーケンスの設定
  u16 anm_seq = TOWNMAP_DATA_GetParam( p_wk->p_townmap_data
                                  ,dataIndex ,TOWNMAP_DATA_PARAM_ZKN_ANM );
  GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk_common_obj[OBJ_WK_MAP_BLINK],  anm_seq );
  
  // 場所を設定する
  GFL_STD_MemClear( &pos, sizeof(GFL_CLACTPOS) );
  pos.x = (s16)TOWNMAP_DATA_GetParam( p_wk->p_townmap_data, 
                                  dataIndex, TOWNMAP_DATA_PARAM_ZKN_POS_X );
  pos.y = (s16)TOWNMAP_DATA_GetParam( p_wk->p_townmap_data, 
                                  dataIndex, TOWNMAP_DATA_PARAM_ZKN_POS_Y );
  pos.y -= MAP_Y_SCROLL_COUNT;
  GFL_CLACT_WK_SetPos( p_wk->p_clwk_common_obj[OBJ_WK_MAP_BLINK]
                                                , &pos, CLSYS_DEFREND_SUB );

  // アルファアニメーションをリセット
  GRA_ObjExistAlphaReset( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アルファアニメーションメイン
 *
 *	@param	p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_AlpaMain( HABITAT_ZONE_GRA_WK* p_wk )
{
  // 更新
  {
    G2S_SetBlendAlpha(
        GX_BLEND_PLANEMASK_NONE,/*GX_BLEND_PLANEMASK_OBJ,*/
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
        p_wk->obj_exist_ev1,
        16 - p_wk->obj_exist_ev1 );
  }

  // アルファカウントを進める
  ZKNCOMM_AlphaUpdate( &p_wk->obj_exist_alpha_anime_count ,&p_wk->obj_exist_ev1 );

}

//----------------------------------------------------------------------------
/**
 *	@brief  アルファアニメーションのリセット
 *
 *	@param	p_wk 
 *
 *	@retval
 *
 */
//-----------------------------------------------------------------------------
static void GRA_ObjExistAlphaReset( HABITAT_ZONE_GRA_WK* p_wk )
{
  // アルファカウントリセット
  ZKNCOMM_AlphaReset( &p_wk->obj_exist_alpha_anime_count ,&p_wk->obj_exist_ev1 );

  // 更新
  {
    G2S_SetBlendAlpha(
        GX_BLEND_PLANEMASK_NONE,/*GX_BLEND_PLANEMASK_OBJ,*/
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
        p_wk->obj_exist_ev1,
        16 - p_wk->obj_exist_ev1 );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  小さいアイコンのステータスを設定する
 *
 *	@param	p_wk 
 *	@param	index 
 *	@param	status
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetSmallIconStatus( HABITAT_ZONE_GRA_WK* p_wk,
                                          u16 index ,GRA_SMALLICON_STATUS status)
{
  s32 i;
  GFL_CLWK* p_clwk = p_wk->p_clwk_common_obj[OBJ_WK_STAMP_SMALL_START+index];
  
  switch( status )
  {
  case GRA_SMALLICON_GET:
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE );
    GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_STAMP_SMALL_GET);
    break;
  case GRA_SMALLICON_LOOK:
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE );
    GFL_CLACT_WK_SetAnmSeq( p_clwk , OBJ_ANM_SEQ_STAMP_SMALL_LOOK);
    break;
  case GRA_SMALLICON_NONE:
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE );
    break;
  default:
    GF_ASSERT(0);
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベント用BMPWINを生成
 *
 *	@param	*p_wk 
 *	@param	pos_y   Y位置指定
 *	@param	size_y  Y高さ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_MakeEventBmpWinWin( HABITAT_ZONE_GRA_WK *p_wk ,u8 pos_y ,u8 size_y )
{
  // 達成イベント用BMPWIN作成
  p_wk->p_event_win_sub = GFL_BMPWIN_Create( BGM_FRAME_EVMSG_WIN,
      2   , pos_y   ,
      32 -4  , size_y ,
      HABITAT_ZONE_MBG_PAL_FONT , GFL_BMP_CHRAREA_GET_B );

  // スクリーンの作成
  GFL_BMPWIN_MakeScreen( p_wk->p_event_win_sub );

  // ウィンドウのクリアー
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_event_win_sub), 0 );

  // 転送
  GFL_BMPWIN_TransVramCharacter( p_wk->p_event_win_sub );

  // 転送リクエスト
  GFL_BG_LoadScreenV_Req( BGM_FRAME_EVMSG_WIN );

  // プリントユーティリティーの準備
  PRINT_UTIL_Setup( &p_wk->print_util[ PRINT_UTIL_EVENT_MSG_WIN ] ,
      p_wk->p_event_win_sub  );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フレーム付きメッセージウィンドウを作成
 *
 *	@param	STRBUF* str
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_MakeFrameWinMsg(const STRBUF* str,HABITAT_ZONE_GRA_WK *p_wk )
{
  GFL_BMPWIN *p_win;

  
  p_win = GetBmpWin( p_wk,PRINT_UTIL_EVENT_MSG_WIN );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_win) ,15 );

  // スクリーンの作成
  GFL_BMPWIN_MakeScreen( p_win );

  // グラフィックのセット
  BmpWinFrame_GraphicSet( BGM_FRAME_EVMSG_WIN,1 , APP_WINFRAME_USE_PALETE , MENU_TYPE_SYSTEM , HEAPID_ZUKAN_HABITAT_ZONE );
  BmpWinFrame_Write( p_win , WINDOW_TRANS_ON_V, 1, APP_WINFRAME_USE_PALETE );
 
  // プリントストリームの作成
  p_wk->p_print_stream = PRINTSYS_PrintStream( p_wk->p_event_win_sub , 0 , 0 , str ,
      p_wk->p_font , MSGSPEED_GetWait() , p_wk->p_tcb_sys , 0 , HEAPID_ZUKAN_HABITAT_ZONE , 15 );

  p_wk->is_cursor_draw = FALSE;

  // 転送
  GFL_BG_LoadScreenV_Req( BGM_FRAME_EVMSG_WIN );
  GFL_BMPWIN_TransVramCharacter( p_win );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ウィンドウの削除
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_DeleteFrameWinMsg( HABITAT_ZONE_GRA_WK *p_wk )
{
  GFL_BMPWIN *p_win = GetBmpWin( p_wk,PRINT_UTIL_EVENT_MSG_WIN );
  PRINTSYS_PrintStreamDelete( p_wk->p_print_stream );
  BmpWinFrame_Clear( p_win,WINDOW_TRANS_ON_V );
  GFL_BMPWIN_ClearScreen( p_win );
  GFL_BG_LoadScreenV_Req( BGM_FRAME_FONT );
  p_wk->p_print_stream = NULL;
  
  // BMPWinの削除
  GFL_BMPWIN_Delete( p_wk->p_event_win_sub );
  p_wk->p_event_win_sub = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  プリントストリームの状態を取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
PRINTSTREAM_STATE HABITAT_ZONE_GRA_CheckPrintStreamStatus( HABITAT_ZONE_GRA_WK *p_wk )
{
  return PRINTSYS_PrintStreamGetState( p_wk->p_print_stream );
}
//----------------------------------------------------------------------------
/**
 *	@brief  カーソルを表示する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_EnableStrCursor( HABITAT_ZONE_GRA_WK *p_wk )
{
  p_wk->is_cursor_draw = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief フレームIDに対応したBMPWINを返す
 *
 *	@param	*p_wk 
 *	@param	win_id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_BMPWIN* GetBmpWin( HABITAT_ZONE_GRA_WK *p_wk ,PRINT_UTIL_WIN win_id)
{
  switch( win_id )
  {
  case PRINT_UTIL_INFO_WIN:
    return p_wk->p_font_win;
  case PRINT_UTIL_MAP_NAME:
    return p_wk->p_font_win_sub;
  case PRINT_UTIL_EVENT_MSG_WIN:
    return p_wk->p_event_win_sub;
  default:
    GF_ASSERT(0);
    break;
  }
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーションシーケンスを設定する
 *
 *	@param	*p_wk 
 *	@param	clwkid 
 *	@param	anm_seq 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetAnmSeq( HABITAT_ZONE_GRA_WK *p_wk ,s32 clwkid ,u32 anm_seq )
{
  GFL_CLWK *p_clwk;
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( p_wk ,clwkid );
  GFL_CLACT_WK_SetAnmSeq( p_clwk, anm_seq );
  GFL_CLACT_WK_SetAnmFrame( p_clwk ,0 );
  GFL_CLACT_WK_SetAutoAnmFlag( p_clwk , TRUE );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  主人公アイコンを表示
 *
 *	@param	*p_wk 
 *	@param	zone_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetHeroIcon( HABITAT_ZONE_GRA_WK *p_wk )
{
  GFL_CLACTPOS pos;
  GFL_CLWK    *p_clwk = p_wk->p_clwk_common_obj[OBJ_WK_MAP_HERO];
  const PLAYER_WORK *p_player_wk = GAMEDATA_GetMyPlayerWork( p_wk->p_gdata );
  ZONEID zone_id = PLAYERWORK_getZoneID( p_player_wk );

  u16 root_zone = TOWNMAP_UTIL_GetRootZoneID(p_wk->p_gdata, zone_id );
  u16 player_pos_data_idx = TOWNMAP_DATA_SearchRootZoneID( p_wk->p_townmap_data ,root_zone );
  if( player_pos_data_idx != TOWNMAP_DATA_ERROR )
  {
    pos.x = (s16)TOWNMAP_DATA_GetParam( p_wk->p_townmap_data, player_pos_data_idx, TOWNMAP_DATA_PARAM_POS_X );
    pos.y = (s16)TOWNMAP_DATA_GetParam( p_wk->p_townmap_data ,player_pos_data_idx, TOWNMAP_DATA_PARAM_POS_Y );
    pos.y -= MAP_Y_SCROLL_COUNT;
    GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DEFREND_SUB );
    GFL_CLACT_WK_SetDrawEnable( p_clwk,TRUE);
  }
  else
  {
    GFL_CLACT_WK_SetDrawEnable( p_clwk,FALSE);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定フレームのスクロールオブジェクトを操作する
 *
 *	@param	*p_wk 
 *	@param	ypos
 *	@param	is_main  
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_AddScrollFrame( HABITAT_ZONE_GRA_WK *p_wk ,s32 yval,BOOL is_main  )
{
  s32 i;
  GFL_CLACTPOS	pos;

  if( is_main )
  {
    // OBJスクロール
    for( i = M_SCROLL_OBJ_START ; i<=M_SCROLL_OBJ_END; i++ )
    {
      GFL_CLWK *p_clwk = p_wk->p_clwk_common_obj[i];
      GFL_CLACT_WK_GetPos( p_clwk , &pos, CLSYS_DRAW_MAIN );
      pos.y += yval;
      GFL_CLACT_WK_SetPos( p_clwk , &pos, CLSYS_DRAW_MAIN );
    }
  }
  else
  {
    // BGスクロール
	  GFL_BG_SetScrollReq( BGS_FRAME_FONT, GFL_BG_SCROLL_Y_DEC, yval );
	  GFL_BG_SetScrollReq( BGS_FRAME_BAR, GFL_BG_SCROLL_Y_DEC, yval );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief 地形に対応したパレットを転送する
 *
 *	@param	*p_wk 
 *	@param	type   
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_TransTrayColor( HABITAT_ZONE_GRA_WK *p_wk ,HABITAT_GROUND_TYPE type )
{
  p_wk->is_trance_pal_col = TRUE;
  p_wk->set_trance_traypal_type = type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレット点滅アニメ作成
 *
 *	@param	*p_wk 
 *	@param	type   
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_ReLoadBlinkAnm( HABITAT_ZONE_GRA_WK *p_wk ,HABITAT_GROUND_TYPE type )
{
  // 終了
  BLINKPALANM_Exit( p_wk->p_blinkanm_wk );
 	
  // 点滅させるパレットの指定
  p_wk->p_blinkanm_wk = BLINKPALANM_Create( BLINK_PALID_POKE_CURSOR_CHANGE*16  ,
        BLINK_PAL_SIZE_POKE_CURSOR , BGM_FRAME_BACK, HEAPID_ZUKAN_HABITAT_ZONE );

  // パレットアニメデータの作成
  switch( type )
  {
  case HABITAT_GROUND_LAND:
    BLINKPALANM_SetPalBufferArc( p_wk->p_blinkanm_wk, ARCID_ZUKAN_GRA,
        NARC_zukan_gra_list_list_bgd_NCLR, 
        (TRAY_PAL_LAND_START)*16, 
        (TRAY_PAL_LAND_START+1)*16 );

    break;
  case HABITAT_GROUND_WATERSIDE:
    BLINKPALANM_SetPalBufferArc( p_wk->p_blinkanm_wk, ARCID_ZUKAN_GRA,
        NARC_zukan_gra_list_list_bgd_other_NCLR, 
        (TRAY_PAL_WATERSIDE_START)*16, 
        (TRAY_PAL_WATERSIDE_START+1)*16 );
    break;
  case HABITAT_GROUND_FISHING:
    BLINKPALANM_SetPalBufferArc( p_wk->p_blinkanm_wk, ARCID_ZUKAN_GRA,
        NARC_zukan_gra_list_list_bgd_other_NCLR, 
        (TRAY_PAL_FISHING_START)*16, 
        (TRAY_PAL_FISHING_START+1)*16 );
    break;
  default:
    GF_ASSERT(0);
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  トレイのパレット転送中か確認
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL HABITAT_ZONE_GRA_CheckTrayPalTrance( HABITAT_ZONE_GRA_WK *p_wk )
{
  return p_wk->is_trance_pal_col;
}


//-------------------------------------
/// 隠しスポット
//-------------------------------------
static void GRA_HideSpotInit( HABITAT_ZONE_GRA_WK* p_wk )
{
  u8 i;
  
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    u16 hide_flag	= TOWNMAP_DATA_GetParam( p_wk->p_townmap_data, i, TOWNMAP_DATA_PARAM_HIDE_FLAG );
    if( hide_flag != TOWNMAP_DATA_ERROR )
    {
      BOOL is_hide = !TOWNMAP_UTIL_CheckFlag( p_wk->p_gdata, hide_flag );
      if( is_hide )
      {
        p_wk->hide_spot[i].state    = HIDE_STATE_HIDE_TRUE;
        p_wk->hide_spot[i].obj_clwk = NULL;
      }
      else
      {
        u16 placetype = TOWNMAP_DATA_GetParam( p_wk->p_townmap_data, i, TOWNMAP_DATA_PARAM_PLACE_TYPE );
        s16	pos_x;				// ｘ座標
        s16 pos_y;				// ｙ座標
	      u16 anmseq;				// アニメーションシーケンス

        pos_x    = (s16)TOWNMAP_DATA_GetParam( p_wk->p_townmap_data, i, TOWNMAP_DATA_PARAM_POS_X );
        pos_y    = (s16)TOWNMAP_DATA_GetParam( p_wk->p_townmap_data, i, TOWNMAP_DATA_PARAM_POS_Y );
        pos_y -= MAP_Y_SCROLL_COUNT;
        anmseq   = (placetype==TOWNMAP_PLACETYPE_HIDE)?(14):(16);  // 偽の場合はTOWNMAP_PLACETYPE_HIDE_TOWN_Sとなっているはず

        p_wk->hide_spot[i].state    = HIDE_STATE_HIDE_FALSE;

        // 隠しMAPアイコン
        p_wk->hide_spot[i].obj_clwk = UI_EASY_CLWK_CreateCLWK( 
            &p_wk->clres_tmap_obj, p_wk->p_clunit, 
            pos_x, pos_y,
            anmseq, HEAPID_ZUKAN_HABITAT_ZONE );
        GFL_CLACT_WK_SetDrawEnable( p_wk->hide_spot[i].obj_clwk, TRUE);
        GFL_CLACT_WK_SetAutoAnmFlag( p_wk->hide_spot[i].obj_clwk, TRUE);
        GFL_CLACT_WK_SetObjMode( p_wk->hide_spot[i].obj_clwk, GX_OAM_MODE_NORMAL );
      }
    }
    else
    {
      p_wk->hide_spot[i].state    = HIDE_STATE_NO;
      p_wk->hide_spot[i].obj_clwk = NULL;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  点滅の停止フラグを設定
 *
 *	@param	*p_wk
 *	@param	is_stop 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetBlinkStop( HABITAT_ZONE_GRA_WK *p_wk,BOOL is_stop )
{
  p_wk->is_stop_blink = is_stop;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベントアイコンの表示状態を設定
 *
 *	@param	*p_wk
 *	@param	is_enable 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_ZONE_GRA_SetEventIconEnable( HABITAT_ZONE_GRA_WK *p_wk,BOOL is_enable )
{
  GFL_CLWK *p_clwk;

  // イベントボタン
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( p_wk ,OBJ_WK_EVENT_ICON_01 );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,is_enable);
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( p_wk ,OBJ_WK_EVENT_ICON_02 );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,is_enable);
  p_clwk = HABITAT_ZONE_GRA_GetComClWk( p_wk ,OBJ_WK_EVENT_ICON_03 );
  GFL_CLACT_WK_SetDrawEnable( p_clwk ,is_enable);
}
