//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_list_gra.c
 *	@brief  生息地リスト：グラフィックモジュール
 *	@author	Masayuki Onoue
 *	@date		2011.08.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "habitat_list_gra.h"
#include "gamesystem/game_data.h"

#include "system/main.h"  // for HEAP_ID

#include "system/wipe.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"

#include "habitat_list_gra.cdat"

#include "../zukan_common.h" // for ZKNCOMM_FADE_WAIT

#include "arc_def.h"        // for ARCID_*
#include "zukan_gra.naix"   // fro NARC_zukan_gra_*
#include "font/font.naix"   // for NARC_font_large_gftr

#include "message.naix"     // for NARC_message_place_name_dat

#include "ui/print_tool.h"  // for PRINTTOOL_PrintColor

#include "field/zonedata.h" // for ZONEDATA_GetPlaceNameID

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

// ＢＧパレット番号
#define	HABITAT_LIST_MBG_PAL_TOUCHBAR	( 14 )		// タッチバー
#define	HABITAT_LIST_MBG_PAL_FONT			( 15 )		// メイン画面フォントパレット
#define	HABITAT_LIST_SBG_PAL_FONT			( 15 )		// サブ画面フォントパレット
#define	HABITAT_TITLE_SBG_PAL_FONT		( 0 )		// サブ画面フォントパレット

// サブオブジェクトパレットフェードリクエストビット
#define PF_SUB_OBJ_REQ_BIT              (0x00FF)  // 0000 0000 1111 1111  // 0<= <8

// タッチバー定義
#define	HABITAT_LIST_TOUCH_BAR_PX		( 0 )
#define	HABITAT_LIST_TOUCH_BAR_PY		( 21 )
#define	HABITAT_LIST_TOUCH_BAR_SX		( 32 )
#define	HABITAT_LIST_TOUCH_BAR_SY		( 3 )

#define	FRAME_SCROLL_X_WAIN	  ( 4 )		// 4フレームに1階動く
#define	FRAME_SCROLL_X_VALUE	( 1 )		// 背景スクロールカウント
#define	Y_SCROLL_COUNT	( -24 )		// マップのY軸ずれ

// リスト表示項目数
#define	ZKNLISTMAIN_LIST_POS_MAX		( 7 )

// リスト項目サイズ
#define	ZKNLISTMAIN_LIST_PX					( 8 ) // キャラ単位
#define	ZKNLISTMAIN_LIST_PY					( 0 )
#define	ZKNLISTMAIN_LIST_SX					( 24 )
#define	ZKNLISTMAIN_LIST_SY					( 3 )

#define FCOL_WP01WN		( PRINTSYS_LSB_Make(14,15,0) )	// フォントカラー：両用０１白抜

#define	SCROLL_BAR_PX	( 252 )
#define	SCROLL_BAR_UY	( 12 )
#define	SCROLL_BAR_DY	( 156 )

#define	BLEND_EV1		( 6 )				// ブレンド係数
#define	BLEND_EV2		( 10 )			// ブレンド係数

// コールバック関数
static void ListCallBack_Draw( void * work, u32 itemNum, PRINT_UTIL * util, s16 py, BOOL disp );
static void ListCallBack_Move( void * work, u32 item_index, BOOL flg );
static void ListCallBack_Scroll( void * work, s8 mv );
static void SetLRButtonAnm( HABITAT_LIST_GRA_WK *p_wk ,BOOL is_force );

// リストコールバック
static const FRAMELIST_CALLBACK	sc_frm_list_callback = {
	ListCallBack_Draw,
	ListCallBack_Move,
	ListCallBack_Scroll,
};

// リストタッチデータ
static const FRAMELIST_TOUCH_DATA sc_touch_tbl[] =
{
	{ {   0,  23, 128, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },		// 00: ポケモンアイコン
	{ {  24,  47, 128, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },		// 01: ポケモンアイコン
	{ {  48,  71, 128, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },		// 02: ポケモンアイコン
	{ {  72,  95, 128, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },		// 03: ポケモンアイコン
	{ {  96, 119, 128, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },		// 04: ポケモンアイコン
	{ { 120, 143, 128, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },		// 05: ポケモンアイコン
	{ { 144, 167, 128, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },		// 06: ポケモンアイコン

	{ {   8, 160, 232, 255 }, FRAMELIST_TOUCH_PARAM_RAIL },					// 07: レール

	{ { 168, 191, 136, 159 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },			// 08: 左
	{ { 168, 191, 160, 183 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },		// 09: 右

	{ { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};

// リストヘッダーデータ
static const FRAMELIST_HEADER	sc_list_header = {
	BGM_FRAME_LIST,							// 下画面ＢＧ
	BGS_FRAME_LIST,							// 上画面ＢＧ

	ZKNLISTMAIN_LIST_PX,					// 項目フレーム表示開始Ｘ座標
	ZKNLISTMAIN_LIST_PY,					// 項目フレーム表示開始Ｙ座標
	ZKNLISTMAIN_LIST_SX,					// 項目フレーム表示Ｘサイズ
	ZKNLISTMAIN_LIST_SY,					// 項目フレーム表示Ｙサイズ

	7,							// フレーム内に表示するBMPWINの表示Ｘ座標
	0,							// フレーム内に表示するBMPWINの表示Ｙ座標
	15,							// フレーム内に表示するBMPWINの表示Ｘサイズ
	3,							// フレーム内に表示するBMPWINの表示Ｙサイズ
	5,							// フレーム内に表示するBMPWINのパレット

	{ 24, 12, 8, 6, 4, 3 },		// スクロール速度 [0] = 最速

	7,							// 選択項目のパレット

	8,							// スクロールバーのＹサイズ

	0,							      // 項目登録数
	FLIST_BACK_TYPE_MAX,  // 背景登録数

	0,												// 初期位置
	ZKNLISTMAIN_LIST_POS_MAX,	// カーソル移動範囲
	0,												// 初期スクロール値

	sc_touch_tbl,			// タッチデータ

	&sc_frm_list_callback,	// コールバック関数
	NULL,
};

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
static void GRA_InitObj( HABITAT_LIST_GRA_WK *p_wk );
static void GRA_InitClact( void );
static void GRA_LoadBgGraphic(void);
static void GRA_LoadObjGraphic( HABITAT_LIST_GRA_WK *p_wk );
static void GRA_InitPaletteFade( HABITAT_LIST_GRA_WK *p_wk );
static void GRA_InitBlinkAnm( HABITAT_LIST_GRA_WK *p_wk );
static void GRA_InitBmpWin( HABITAT_LIST_GRA_WK *p_wk );
static void GRA_SetBlendAlpha(void);
static void GRA_SetWindow(void);

// 終了系
static void GRA_ResetWindow( void );
static void GRA_ExitPaletteFade( HABITAT_LIST_GRA_WK *p_wk );
static void GRA_ExitBg( void );
static void GRA_ExitObj( HABITAT_LIST_GRA_WK *p_wk );
static void GRA_ExitBmpWin( HABITAT_LIST_GRA_WK *p_wk );
static void GRA_ExitBlinkAnm( HABITAT_LIST_GRA_WK *p_wk );

// 処理系
static void GRA_PrintMain( HABITAT_LIST_GRA_WK *p_wk );
static void GRA_BlinkAnmMain( HABITAT_LIST_GRA_WK *p_wk );
static s32 GetBackGroundScreenID( ZUKANHABITAT_LIST_LISTDATA *p_data );
static void SetFrameMenuIcon( HABITAT_LIST_GRA_WK *p_wk ,u32 item_index ,s16 py ,BOOL disp );
static void SetCursorEnable( HABITAT_LIST_GRA_WK *p_wk , BOOL flg );
static void RefreshCursorPos( HABITAT_LIST_GRA_WK *p_wk );
static void GetExistLandType( ZUKANHABITAT_LIST_LISTDATA *p_data ,u8 *is_l ,u8 *is_w ,u8 *is_f );

//===============================================================================
// 初期化・終了関数呼び出し
//===============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval FALSEの時、初期化終了
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_Init( HABITAT_LIST_GRA_WK *p_wk ,GAMEDATA *p_gdata )
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
  GRA_InitBmpWin( p_wk );   // BMPWIN
  GRA_LoadObjGraphic( p_wk );     // OBJ初期化
  GRA_LoadBgGraphic();            // BGリソース読み込み
  GRA_InitPaletteFade( p_wk );    // パレットフェード
  GRA_InitBlinkAnm( p_wk );       // 点滅アニメ
  GRA_SetBlendAlpha();            // 半透明処理
  GRA_SetWindow();                // ウィンドウ設定
  
  // リストの更新フラグを立てた状態ではじめ、
  // ミニマップアイコンの初期化時のアニメーションを固定
  p_wk->list_update = TRUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief  フレームリストの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval TRUEの時初期化終了
 */
//-----------------------------------------------------------------------------
BOOL HABITAT_LIST_GRA_FrameListInit( HABITAT_LIST_GRA_WK *p_wk )
{
  // リストの初期化待ち
  if( FRAMELIST_Init( p_wk->p_lwk ) == TRUE )
  {
    // カーソル位置をLRボタンの表示状態を設定
    SetCursorEnable( p_wk, TRUE );
    SetLRButtonAnm( p_wk , FALSE);
    return FALSE;
  }
  return TRUE;
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
void HABITAT_LIST_GRA_Exit( HABITAT_LIST_GRA_WK *p_wk )
{
  // ウィンドウのリセット
  GRA_ResetWindow();

  // BMPWINの開放
  GRA_ExitBmpWin( p_wk );

  // 点滅アニメの開放
  GRA_ExitBlinkAnm( p_wk );

  // OBJの終了処理
  GRA_ExitObj( p_wk );
  
  // BGの終了処理
	GRA_ExitBg();

  // パレットフェードの終了
  GRA_ExitPaletteFade( p_wk );

  // ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();

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
void HABITAT_LIST_GRA_VblankMain( HABITAT_LIST_GRA_WK *p_wk )
{
	GFL_CLACT_SYS_VBlankFunc();
  PaletteFadeTrans( p_wk->p_pfd );
}

//----------------------------------------------------------------------------
/**
 *	@brief  HBlankメイン
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_HblankMain( HABITAT_LIST_GRA_WK *p_wk )
{
  s32	vcount = GX_GetVCount();

  if( vcount >= 168 ){
    G2S_ChangeBlendAlpha( 11, 5 );
  }else if( vcount >= 144 ){
    G2S_ChangeBlendAlpha( 10, 6 );
  }else if( vcount >= 120 ){
    G2S_ChangeBlendAlpha( 9, 7 );
  }else if( vcount >= 96 ){
    G2S_ChangeBlendAlpha( 8, 8 );
  }else if( vcount >= 72 ){
    G2S_ChangeBlendAlpha( 6, 10 );
  }else{
    G2S_ChangeBlendAlpha( 4, 12 );
  }
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
void HABITAT_LIST_GRA_Main( HABITAT_LIST_GRA_WK *p_wk )
{
  // 文字描画更新
  GRA_PrintMain( p_wk );

  // スクロールバーの位置を補正
  HABITAT_LIST_GRA_GetScrollBarYPos( p_wk );

  // カーソル点滅アニメ
  if( p_wk->is_stop_blink == FALSE )
  {
    GRA_BlinkAnmMain( p_wk );
  }

  // 背景スクロール
  p_wk->back_bg_scroll_wait_count++;
  if( p_wk->back_bg_scroll_wait_count == FRAME_SCROLL_X_WAIN )
  {
    GFL_BG_SetScrollReq( BGM_FRAME_SCROLL_WIN,
                              GFL_BG_SCROLL_X_INC, FRAME_SCROLL_X_VALUE );
    GFL_BG_SetScrollReq( BGS_FRAME_SCROLL_WIN,
                              GFL_BG_SCROLL_X_INC, FRAME_SCROLL_X_VALUE );
    p_wk->back_bg_scroll_wait_count = 0;
  }

  // セルアクター更新
  GFL_CLACT_SYS_Main();
}

//----------------------------------------------------------------------------
/**
 *	@brief  リスト更新
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 HABITAT_LIST_GRA_ListMain( HABITAT_LIST_GRA_WK *p_wk )
{
  u32 ret = FRAMELIST_Main( p_wk->p_lwk );
  if( ret == FRAMELIST_RET_NONE )
  {
    SetCursorEnable( p_wk, TRUE );
    SetLRButtonAnm( p_wk , FALSE);
    p_wk->list_update = FALSE;
  }
  else
  {
    p_wk->list_update = TRUE;
  }
  return ret;
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
	GFL_BG_Init( HEAPID_ZUKAN_HABITAT_LIST );

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
		GFL_BG_SetClearCharacter( BGM_FRAME_FONT, 0x20, 0, HEAPID_ZUKAN_HABITAT_LIST );
		GFL_BG_LoadScreenV_Req( BGM_FRAME_FONT );
    GFL_BG_SetVisible( BGM_FRAME_FONT ,VISIBLE_ON );
	}

	{	// メニューバー
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x4000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGM_FRAME_MENUBAR, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( BGM_FRAME_MENUBAR ,VISIBLE_ON );
	}
	{	// リスト面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGM_FRAME_LIST, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGM_FRAME_LIST );
		GFL_BG_LoadScreenV_Req( BGM_FRAME_LIST );
    GFL_BG_SetVisible( BGM_FRAME_LIST ,VISIBLE_ON );
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
		GFL_BG_SetClearCharacter( BGS_FRAME_FONT, 0x20, 0, HEAPID_ZUKAN_HABITAT_LIST );
		GFL_BG_LoadScreenV_Req( BGS_FRAME_FONT );
    GFL_BG_SetVisible( BGS_FRAME_FONT ,VISIBLE_ON );
	}

	{	// サブ画面：タイトルバー
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGS_FRAME_BAR, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGS_FRAME_BAR );
		GFL_BG_LoadScreenV_Req( BGS_FRAME_BAR );
    GFL_BG_SetVisible( BGS_FRAME_BAR ,VISIBLE_ON );
	}
	{	// リスト面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGS_FRAME_LIST, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGS_FRAME_LIST );
		GFL_BG_LoadScreenV_Req( BGS_FRAME_LIST );
    GFL_BG_SetVisible( BGS_FRAME_LIST ,VISIBLE_ON );
	}

	{	// 背景面
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x1c000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( BGS_FRAME_SCROLL_WIN, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( BGS_FRAME_SCROLL_WIN );
		GFL_BG_LoadScreenV_Req( BGS_FRAME_SCROLL_WIN );
    GFL_BG_SetVisible( BGS_FRAME_SCROLL_WIN ,VISIBLE_ON );
	}

  // 表示状態変更
	GFL_DISP_GX_SetVisibleControl(
		BGM_FRAME_FONT | BGM_FRAME_MENUBAR | BGM_FRAME_LIST | BGM_FRAME_SCROLL_WIN
      ,VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		BGS_FRAME_FONT | BGS_FRAME_BAR | BGS_FRAME_LIST | BGS_FRAME_SCROLL_WIN , VISIBLE_ON );

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
static void GRA_InitObj( HABITAT_LIST_GRA_WK *p_wk )
{
  // セルアクターの初期化
  GRA_InitClact();

  // ユニットの作成
	p_wk->p_clunit	= GFL_CLACT_UNIT_Create( HABITAT_LIST_UNIT_CLWK_MAX, 0, HEAPID_ZUKAN_HABITAT_LIST );

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
static void GRA_InitBmpWin( HABITAT_LIST_GRA_WK *p_wk )
{
  // BMPWINの初期化
  GFL_BMPWIN_Init( HEAPID_ZUKAN_HABITAT_LIST );

  // プリントキューの作成
  p_wk->p_print_que = PRINTSYS_QUE_Create( HEAPID_ZUKAN_HABITAT_LIST );

  // メニュー用BMPWIN作成
  {
    p_wk->p_font_win = GFL_BMPWIN_Create( BGM_FRAME_FONT,
        0   , 0   ,
        32  , 24  ,
        HABITAT_LIST_MBG_PAL_FONT , GFL_BMP_CHRAREA_GET_F );

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

  // メニュー用BMPWIN作成
  {
    p_wk->p_font_win_sub = GFL_BMPWIN_Create( BGS_FRAME_FONT,
        0   , 0   ,
        32  , 24  ,
        HABITAT_TITLE_SBG_PAL_FONT , GFL_BMP_CHRAREA_GET_F );

    // スクリーンの作成
    GFL_BMPWIN_MakeScreen( p_wk->p_font_win_sub );

    // ウィンドウのクリアー
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_font_win_sub), 0 );

    // 転送
    GFL_BMPWIN_TransVramCharacter( p_wk->p_font_win_sub );

    // 転送リクエスト
    GFL_BG_LoadScreenV_Req( BGS_FRAME_FONT );
    
    // プリントユーティリティーの準備
    PRINT_UTIL_Setup( &p_wk->print_util[ PRINT_UTIL_TITLE_WIN ] ,
                                                  p_wk->p_font_win_sub  );
  }
 
//	// フォント
  p_wk->p_font	= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_ZUKAN_HABITAT_LIST );

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

	  HABITAT_LIST_UNIT_CLWK_MAX,	// 登録できるキャラデータ数
		HABITAT_LIST_UNIT_CLWK_MAX,	// 登録できるパレットデータ数
		HABITAT_LIST_UNIT_CLWK_MAX,	// 登録できるセルアニメパターン数
		0,											// 登録できるマルチセルアニメパターン数（※現状未対応）

	  16,										// メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	  16										// サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
	};
	GFL_CLACT_SYS_Create( &init, &VramTbl, HEAPID_ZUKAN_HABITAT_LIST );
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
    ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_HABITAT_LIST );

    // 下画面パレット転送
    GFL_ARCHDL_UTIL_TransVramPalette(
        ah, NARC_zukan_gra_list_list_bgd_NCLR, 
        PALTYPE_MAIN_BG, 0, 0, HEAPID_ZUKAN_HABITAT_LIST );

    // 上画面パレット転送(通常パレット)
    GFL_ARCHDL_UTIL_TransVramPalette(
        ah, NARC_zukan_gra_list_list_bgu_NCLR,
        PALTYPE_SUB_BG, 0, 0, HEAPID_ZUKAN_HABITAT_LIST );

    // スクロールウィンドウ（下画面）
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_zukan_gra_list_listbase_bgd_NCGR,
        BGM_FRAME_SCROLL_WIN, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );

    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_zukan_gra_habitat_list_habitatbase_bgd_NSCR,
        BGM_FRAME_SCROLL_WIN, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );

    // キャラクタ転送（サブタイトルバー）
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_zukan_gra_list_listbase_bgu_NCGR,
        BGS_FRAME_BAR, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );

    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_zukan_gra_habitat_list_listtitle_seisoku_NSCR,
        BGS_FRAME_BAR, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );

    // リストフレーム用キャラクタ（メイン）
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_zukan_gra_list_listframe_bgd_NCGR,
        BGM_FRAME_LIST, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );

    // リストフレーム用キャラクタ（サブ）
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_zukan_gra_list_listframe_bgu_NCGR,
        BGS_FRAME_LIST, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );

    // スクロールウィンドウ（上画面）
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
        ah, NARC_zukan_gra_list_listbase_bgu_NCGR,
        BGS_FRAME_SCROLL_WIN, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );

    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, NARC_zukan_gra_habitat_list_habitatbase_bgu_NSCR,
        BGS_FRAME_SCROLL_WIN, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );


    GFL_ARC_CloseDataHandle( ah );
  }

  // 共通素材アーカイブ
  {
    // タッチバー
    ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_ZUKAN_HABITAT_LIST );

    GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
        ah, APP_COMMON_GetBarCharArcIdx(),
        BGM_FRAME_MENUBAR, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );
    GFL_ARCHDL_UTIL_TransVramPalette(
        ah, APP_COMMON_GetBarPltArcIdx(),
        PALTYPE_MAIN_BG,
        HABITAT_LIST_MBG_PAL_TOUCHBAR*0x20, 0x20, HEAPID_ZUKAN_HABITAT_LIST );
    GFL_ARCHDL_UTIL_TransVramScreen(
        ah, APP_COMMON_GetBarScrnArcIdx(),
        BGM_FRAME_MENUBAR, 0, 0, FALSE, HEAPID_ZUKAN_HABITAT_LIST );
    GFL_BG_ChangeScreenPalette(
        BGM_FRAME_MENUBAR, HABITAT_LIST_TOUCH_BAR_PX, HABITAT_LIST_TOUCH_BAR_PY,
        HABITAT_LIST_TOUCH_BAR_SX, HABITAT_LIST_TOUCH_BAR_SY,
        HABITAT_LIST_MBG_PAL_TOUCHBAR );
    GFL_BG_LoadScreenV_Req( BGM_FRAME_MENUBAR );

    GFL_ARC_CloseDataHandle( ah );
  }

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		HABITAT_LIST_MBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_HABITAT_LIST );
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		HABITAT_LIST_SBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_HABITAT_LIST );
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
static void GRA_LoadObjGraphic( HABITAT_LIST_GRA_WK *p_wk )
{
  s32 i;
  // アプリ常駐OBJリソース読み込み(下画面)
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

    UI_EASY_CLWK_LoadResource( &p_wk->clres_common_obj, &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_LIST );
  }
  // アプリ常駐OBJリソース読み込み(上画面)
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_SUB;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = ARCID_ZUKAN_GRA;
    prm.pltt_id   = NARC_zukan_gra_list_list_objd_NCLR;
    prm.ncg_id    = NARC_zukan_gra_list_list_objd_NCGR;
    prm.cell_id   = NARC_zukan_gra_list_list_objd_NCER; 
    prm.anm_id    = NARC_zukan_gra_list_list_objd_NANR;
    prm.pltt_line = 0;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num  = 7;

    UI_EASY_CLWK_LoadResource( &p_wk->clres_common_obj_s, &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_LIST );
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
    prm.pltt_line = 10;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num  = APP_COMMON_BARICON_PLT_NUM;

    UI_EASY_CLWK_LoadResource( &p_wk->clres_app_menu_obj, &prm, p_wk->p_clunit, HEAPID_ZUKAN_HABITAT_LIST );
  }

  // リスト用アイコン --------------------------------------------------
  for( i = LIST_ICON_CLWKID_START; i < LIST_ICON_CLWKID_END; i++ )
  {
    p_wk->p_clwk_common_obj[i] = UI_EASY_CLWK_CreateCLWK( 
        &p_wk->clres_common_obj, p_wk->p_clunit, 
        0, 0, OBJ_ANM_SEQ_NEW_ICON, HEAPID_ZUKAN_HABITAT_LIST );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_common_obj[i], FALSE );
    // 説明ボタンより下
    GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[i] ,2 );
    GFL_CLACT_WK_SetObjMode(p_wk->p_clwk_common_obj[i],GX_OAM_MODE_XLU);
  }

  for( i = LIST_ICON_CLWKID_START_S; i < LIST_ICON_CLWKID_END_S; i++ )
  {
    p_wk->p_clwk_common_obj[i] = UI_EASY_CLWK_CreateCLWK( 
        &p_wk->clres_common_obj_s, p_wk->p_clunit, 
        0, 0, OBJ_ANM_SEQ_NEW_ICON, HEAPID_ZUKAN_HABITAT_LIST );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_common_obj[i], FALSE );
    // 説明ボタンより下
    GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[i] ,2 );
    GFL_CLACT_WK_SetObjMode(p_wk->p_clwk_common_obj[i],GX_OAM_MODE_XLU);
  }

  // 半透明ウィンドウ
  for( i = LIST_ICON_CLWKID_START_SW; i < LIST_ICON_CLWKID_END_SW; i++ )
  {
    p_wk->p_clwk_common_obj[i] = UI_EASY_CLWK_CreateCLWK( 
        &p_wk->clres_common_obj_s, p_wk->p_clunit, 
        0, 0, OBJ_ANM_SEQ_NEW_ICON, HEAPID_ZUKAN_HABITAT_LIST );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_common_obj[i], TRUE );
    GFL_CLACT_WK_SetObjMode(p_wk->p_clwk_common_obj[i],GX_OAM_MODE_OBJWND);
  }

  // ミニマップ --------------------------------------------------
  p_wk->p_clwk_common_obj[MINIMAP_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      MINI_MAPX_POS_X,MINI_MAPX_POS_Y,
      OBJ_ANM_SEQ_MINIMAP, HEAPID_ZUKAN_HABITAT_LIST );
  GFL_CLACT_WK_SetDrawEnable( 
              p_wk->p_clwk_common_obj[MINIMAP_CLWKID], TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk_common_obj[MINIMAP_CLWKID] , TRUE );

  // モード切替ボタン1 --------------------------------------------------
  p_wk->p_clwk_common_obj[MODE_BUTTON_01_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      MODE_BUTTON_01_POS_X,MODE_BUTTON_01_POS_Y,
      OBJ_ANM_SEQ_MODE_BUTTON, HEAPID_ZUKAN_HABITAT_LIST );
  GFL_CLACT_WK_SetDrawEnable( 
              p_wk->p_clwk_common_obj[MODE_BUTTON_01_CLWKID], TRUE );
  GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[MODE_BUTTON_01_CLWKID] ,2 ); // ボタンより下
  // モード切替ボタン2 --------------------------------------------------
  p_wk->p_clwk_common_obj[MODE_BUTTON_02_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      MODE_BUTTON_02_POS_X,MODE_BUTTON_02_POS_Y,
      OBJ_ANM_SEQ_MODE_BUTTON, HEAPID_ZUKAN_HABITAT_LIST );
  GFL_CLACT_WK_SetDrawEnable( 
              p_wk->p_clwk_common_obj[MODE_BUTTON_02_CLWKID], TRUE );
  GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[MODE_BUTTON_02_CLWKID] ,2 ); // ボタンより下
  // モード切替ボタン3 --------------------------------------------------
  p_wk->p_clwk_common_obj[MODE_BUTTON_03_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      MODE_BUTTON_03_POS_X,MODE_BUTTON_03_POS_Y,
      OBJ_ANM_SEQ_MODE_BUTTON_ANM, HEAPID_ZUKAN_HABITAT_LIST );
  GFL_CLACT_WK_SetDrawEnable( 
              p_wk->p_clwk_common_obj[MODE_BUTTON_03_CLWKID], TRUE );
  GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[MODE_BUTTON_03_CLWKID] ,2 ); // ボタンより下
  // セレクトボタン --------------------------------------------------
  p_wk->p_clwk_common_obj[SELECT_BUTTON_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      SELECT_BUTTON_POS_X,SELECT_BUTTON_POS_Y,
      OBJ_ANM_SEQ_SELECT_BUTTON, HEAPID_ZUKAN_HABITAT_LIST );
  GFL_CLACT_WK_SetDrawEnable( 
              p_wk->p_clwk_common_obj[SELECT_BUTTON_CLWKID], TRUE );
  // スクロールボタン --------------------------------------------------
  p_wk->p_clwk_common_obj[SCROLL_BUTTON_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit,0,0,
      OBJ_ANM_SEQ_SCROLL_BUTTON, HEAPID_ZUKAN_HABITAT_LIST );
  // スクロールバー --------------------------------------------------
  p_wk->p_clwk_common_obj[SCROLL_BAR_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      SCROLL_BAR_POS_X,SCROLL_BAR_POS_Y,
      OBJ_ANM_SEQ_SCROLL_BAR, HEAPID_ZUKAN_HABITAT_LIST );
  GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[SCROLL_BAR_CLWKID] ,1 ); // ボタンより下

  // リストカーソル --------------------------------------------------
  p_wk->p_clwk_common_obj[LIST_CURSOR_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_common_obj, p_wk->p_clunit, 
      0,0,
      OBJ_ANM_SEQ_LIST_CURSOR01, HEAPID_ZUKAN_HABITAT_LIST );
  GFL_CLACT_WK_SetBgPri( p_wk->p_clwk_common_obj[LIST_CURSOR_CLWKID] ,1 ); // ボタンより下


  // 共通素材系読み込み ------------------------------------------------

  // 左矢印
  p_wk->p_clwk_common_obj[L_BUTTON_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      BUTTON_LEFT_ARROW_X, BUTTON_LEFT_ARROW_Y,
      APP_COMMON_BARICON_CURSOR_LEFT, HEAPID_ZUKAN_HABITAT_LIST );

  // 右矢印
  p_wk->p_clwk_common_obj[R_BUTTON_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      BUTTON_RIGHT_ARROW_X, BUTTON_RIGHT_ARROW_Y,
      APP_COMMON_BARICON_CURSOR_RIGHT, HEAPID_ZUKAN_HABITAT_LIST );

  // チェックボックス
  p_wk->p_clwk_common_obj[CHECKBOX_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      BUTTON_CHECK_X, BUTTON_CHECK_Y,
      APP_COMMON_BARICON_CHECK_OFF, HEAPID_ZUKAN_HABITAT_LIST );

  // キャンセル
  p_wk->p_clwk_common_obj[EXIT_BUTTON_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      BUTTON_CANCEL_X, BUTTON_CANCEL_Y,
      APP_COMMON_BARICON_EXIT, HEAPID_ZUKAN_HABITAT_LIST );

  // 戻る
  p_wk->p_clwk_common_obj[RETURN_BUTTON_CLWKID] = UI_EASY_CLWK_CreateCLWK( 
      &p_wk->clres_app_menu_obj, p_wk->p_clunit, 
      BUTTON_RETURN_X, BUTTON_RETURN_Y,
      APP_COMMON_BARICON_RETURN, HEAPID_ZUKAN_HABITAT_LIST );

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
static void GRA_InitPaletteFade( HABITAT_LIST_GRA_WK *p_wk )
{
	p_wk->p_pfd = PaletteFadeInit( HEAPID_ZUKAN_HABITAT_LIST );

	PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_MAIN_BG, FADE_PAL_ALL_SIZE, HEAPID_ZUKAN_HABITAT_LIST );
	PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_SUB_BG, FADE_PAL_ALL_SIZE, HEAPID_ZUKAN_HABITAT_LIST );
	PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_MAIN_OBJ, FADE_PAL_ALL_SIZE, HEAPID_ZUKAN_HABITAT_LIST );
	PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_SUB_OBJ, FADE_PAL_ALL_SIZE, HEAPID_ZUKAN_HABITAT_LIST );

  PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_MAIN_BG, 0, FADE_PAL_ALL_SIZE );
  PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_SUB_BG, 0, FADE_PAL_ALL_SIZE );
  PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_MAIN_OBJ, 0, FADE_PAL_ALL_SIZE );
	PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_SUB_OBJ, 0, FADE_PAL_ALL_SIZE );

}

//----------------------------------------------------------------------------------
/**
 * @brief		ウィンドウ設定
 *
 * @param		none
 *
 * @return	none
 */
//----------------------------------------------------------------------------------
static void GRA_SetWindow(void)
{
	G2S_SetWndOBJInsidePlane(
		GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 |
		GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ, TRUE );
	G2S_SetWndOutsidePlane(
		GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 |
		GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ, TRUE );
	GXS_SetVisibleWnd( GX_WNDMASK_OW );
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
static void GRA_ExitObj( HABITAT_LIST_GRA_WK *p_wk )
{
  s32 i;
  UI_EASY_CLWK_UnLoadResource( &p_wk->clres_common_obj );
  UI_EASY_CLWK_UnLoadResource( &p_wk->clres_common_obj_s );
  UI_EASY_CLWK_UnLoadResource( &p_wk->clres_app_menu_obj );
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
static void GRA_ExitPaletteFade( HABITAT_LIST_GRA_WK *p_wk )
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
static void GRA_ExitBmpWin( HABITAT_LIST_GRA_WK *p_wk )
{
  // プリントキューの開放
  PRINTSYS_QUE_Clear( p_wk->p_print_que );
  PRINTSYS_QUE_Delete( p_wk->p_print_que );

  // フォント
	GFL_FONT_Delete( p_wk->p_font );

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
void HABITAT_LIST_GRA_CallFade( HABITAT_LIST_GRA_WK *p_wk, u8 start, u8 end )
{
  PaletteFadeReq( p_wk->p_pfd, PF_BIT_MAIN_BG,  0xbfff, ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
  PaletteFadeReq( p_wk->p_pfd, PF_BIT_SUB_BG,   0x3ffe, ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
  PaletteFadeReq( p_wk->p_pfd, PF_BIT_MAIN_OBJ, 0x3f, ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
	PaletteFadeReq( p_wk->p_pfd, PF_BIT_SUB_OBJ,  0x3fff, ZKNCOMM_FADE_WAIT, start, end, 0, GFUser_VIntr_GetTCBSYS() );
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
static void GRA_InitBlinkAnm( HABITAT_LIST_GRA_WK *p_wk )
{
  ARCHANDLE * ah;

  // 図鑑グラフィックアーカイブ
  ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_HABITAT_LIST );
	
  // 点滅させるパレットの指定
  p_wk->p_blinkanm_wk = BLINKPALANM_Create( BLINK_PALID_POKE_CURSOR_CHANGE*16  ,
        BLINK_PAL_SIZE_POKE_CURSOR , BLINKPALANM_MODE_MAIN_OBJ,
        HEAPID_ZUKAN_HABITAT_LIST );

  // パレットアニメデータの作成
  BLINKPALANM_SetPalBufferArcHDL( p_wk->p_blinkanm_wk, ah,
                    NARC_zukan_gra_list_list_objd_NCLR, 
                    BLINK_PALID_POKE_CURSOR_START*16, 
                    BLINK_PALID_POKE_CURSOR_END*16 );
   
  // 点滅は停止状態で開始
  p_wk->is_stop_blink = TRUE;

  // アーカイブクローズ
  GFL_ARC_CloseDataHandle( ah );
}

//----------------------------------------------------------------------------
/**
 *	@brief  点滅アニメ終了処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GRA_ExitBlinkAnm( HABITAT_LIST_GRA_WK *p_wk )
{
  // 終了
  BLINKPALANM_Exit( p_wk->p_blinkanm_wk );
}

//----------------------------------------------------------------------------------
/**
 * @brief		アルファブレンド設定
 *
 * @param		none
 *
 * @return	none
 */
//----------------------------------------------------------------------------------
static void GRA_SetBlendAlpha(void)
{
	G2S_SetBlendAlpha(
		GX_BLEND_PLANEMASK_BG2,
		GX_BLEND_PLANEMASK_BG3,
		BLEND_EV1, BLEND_EV2 );
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
u16 HABITAT_LIST_GRA_CheckFade( HABITAT_LIST_GRA_WK *p_wk )
{
	return PaletteFadeCheck( p_wk->p_pfd );
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
static void GRA_PrintMain( HABITAT_LIST_GRA_WK *p_wk )
{
  // ユーティリティーの更新
  int i;
  BOOL trans_end;

  // 描画待ちフラグを初期化
  p_wk->is_print_util_wati = FALSE;

  // プリントキューを更新
  PRINTSYS_QUE_Main( p_wk->p_print_que );

  // アイコンをリフレッシュする
  {
    s32 i ;
    BOOL cursor_pos_refresh = FALSE;
    for( i = LIST_ICON_CLWKID_START; i < LIST_ICON_CLWKID_END; i ++ )
    {
      GFL_CLWK *p_clwk = p_wk->p_clwk_common_obj[i];
      GRA_ICON_DATA *p_icon = &p_wk->icon_data_main[i - LIST_ICON_CLWKID_START];
      if( p_icon->is_refresh == TRUE )
      {
        GFL_CLACT_WK_SetDrawEnable( p_clwk , p_icon->draw_flag );
        GFL_CLACT_WK_SetAnmSeq( p_clwk, p_icon->anm_seq );
        GFL_CLACT_WK_SetPos( p_clwk , &p_icon->pos, CLSYS_DRAW_MAIN );
        p_icon->is_refresh = FALSE;
        cursor_pos_refresh = TRUE;
      }
    }
    for( i = LIST_ICON_CLWKID_START_S; i < LIST_ICON_CLWKID_END_S; i ++ )
    {
      GFL_CLWK *p_clwk = p_wk->p_clwk_common_obj[i];
      GFL_CLWK *p_clwk_win;
      GRA_ICON_DATA *p_icon = &p_wk->icon_data_sub[i - LIST_ICON_CLWKID_START_S];
      s32 win_id = (i-LIST_ICON_CLWKID_START_S)+LIST_ICON_CLWKID_START_SW;
      p_clwk_win = p_wk->p_clwk_common_obj[win_id];
      if( p_icon->is_refresh == TRUE )
      {
        GFL_CLACT_WK_SetDrawEnable( p_clwk , p_icon->draw_flag );
        GFL_CLACT_WK_SetAnmSeq( p_clwk, p_icon->anm_seq );
        GFL_CLACT_WK_SetPos( p_clwk , &p_icon->pos, CLSYS_DRAW_SUB );
        p_icon->is_refresh = FALSE;
        cursor_pos_refresh = TRUE;

        // 同時にウィンドウの形、場所も移動させる
        GFL_CLACT_WK_SetDrawEnable( p_clwk_win , p_icon->draw_flag );
        GFL_CLACT_WK_SetPos( p_clwk_win , &p_icon->pos, CLSYS_DRAW_SUB );
        GFL_CLACT_WK_SetAnmSeq( p_clwk_win, p_icon->anm_seq );
      }
    }
  }

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
BOOL HABITAT_LIST_GRA_CheckPrintUtilStatus( HABITAT_LIST_GRA_WK *p_wk )
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
void HABITAT_LIST_GRA_DrawStr( PRINT_UTIL_WIN win_id, const STRBUF* str,
    HABITAT_LIST_GRA_WK *p_wk ,s16 x, s16 y ,s16 w, s16 h ,PRINTSYS_LSB font_colro )
{
  GFL_BMPWIN *p_win;

  switch( win_id )
  {
  case PRINT_UTIL_INFO_WIN:
      p_win = p_wk->p_font_win;
    break;
  case PRINT_UTIL_TITLE_WIN:
      p_win = p_wk->p_font_win_sub;
    break;
  default:
    GF_ASSERT(0);
  }
	
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
void HABITAT_LIST_GRA_ClearWin( PRINT_UTIL_WIN win_id ,HABITAT_LIST_GRA_WK *p_wk ,s16 x, s16 y ,s16 w, s16 h )
{
  GFL_BMPWIN *p_win;

  switch( win_id )
  {
  case PRINT_UTIL_INFO_WIN:
      p_win = p_wk->p_font_win;
    break;
  case PRINT_UTIL_TITLE_WIN:
      p_win = p_wk->p_font_win_sub;
    break;
  default:
    GF_ASSERT(0);
  }
	
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
static void GRA_BlinkAnmMain( HABITAT_LIST_GRA_WK *p_wk )
{
  // カーソル点滅アニメーション
  BLINKPALANM_Main( p_wk->p_blinkanm_wk );
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
void HABITAT_LIST_GRA_BmpFil( PRINT_UTIL_WIN win_id ,u8 col_code ,HABITAT_LIST_GRA_WK *p_wk )
{
  GFL_BMPWIN *p_win;
  u8 frame;
  switch( win_id )
  {
  case PRINT_UTIL_INFO_WIN:
      p_win = p_wk->p_font_win;
    break;
  case PRINT_UTIL_TITLE_WIN:
      p_win = p_wk->p_font_win_sub;
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
GFL_CLWK* HABITAT_LIST_GRA_GetComClWk( HABITAT_LIST_GRA_WK *p_wk ,s32 obj_id )
{
  return p_wk->p_clwk_common_obj[ obj_id ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  リストデータを生成する
 *
 *	@param	p_wk
 *	@param	list_size
 *	@param	*p_list_data 
 *	@param	default_zone_group
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_MakeList( HABITAT_LIST_GRA_WK* p_wk, u8 list_size,
    ZUKANHABITAT_LIST_LISTDATA *p_list_data ,u16 default_zone_group,u16 initPos)
{
	ARCHANDLE * ah;
	u32	i;

	ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_HABITAT_LIST );

	// リストワーク作成
	{
		FRAMELIST_HEADER	hed;

		hed = sc_list_header;
		hed.itemMax = PLACE_DATA_MAX;
		hed.cbWork  = p_wk;
    hed.initPos = initPos;
		for( i = 0; i < list_size ; i++ )
		{
      const ZUKANHABITAT_LIST_LISTDATA *p_ld = &p_list_data[i];
			if( p_ld->zone_group == default_zone_group ){
				if( i > (list_size-ZKNLISTMAIN_LIST_POS_MAX) ){
					hed.initPos    = i - ( list_size - ZKNLISTMAIN_LIST_POS_MAX );
					hed.initScroll = i - hed.initPos;
				}else if( i < ZKNLISTMAIN_LIST_POS_MAX ){
					hed.initPos    = i;
				}else{
					hed.initScroll = i;
				}
				break;
			}
		}
		p_wk->p_lwk = FRAMELIST_Create( &hed, HEAPID_ZUKAN_HABITAT_LIST );
	}

	// 項目背景設定
  // 未コンプリート
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_seisoku1_NSCR, FALSE, FLIST_BACK_L );
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_seisoku2_NSCR, FALSE, FLIST_BACK_W );
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_seisoku3_NSCR, FALSE, FLIST_BACK_LW );
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_seisoku4_NSCR, FALSE, FLIST_BACK_WF );
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_seisoku5_NSCR, FALSE, FLIST_BACK_LWF );
  // コンプリート
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_comp1_NSCR, FALSE, FLIST_BACK_L_CMP );
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_comp2_NSCR, FALSE, FLIST_BACK_W_CMP );
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_comp3_NSCR, FALSE, FLIST_BACK_LW_CMP );
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_comp4_NSCR, FALSE, FLIST_BACK_WF_CMP );
	FRAMELIST_LoadFrameGraphicAH( p_wk->p_lwk, ah,
      NARC_zukan_gra_habitat_list_list_comp5_NSCR, FALSE, FLIST_BACK_LWF_CMP );

	GFL_ARC_CloseDataHandle( ah );

  // リストパラメータ設定・項目文字列作成
  {
    GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE ,
        NARC_message_place_name_dat ,HEAPID_ZUKAN_HABITAT_LIST);

    for( i=0; i < list_size; i++ )
    {
      s32 screen_id;
      u16 place_id = ZONEDATA_GetPlaceNameID( p_list_data[i].zone_group );
      p_wk->p_place_name[i] = GFL_MSG_CreateString( p_msg , place_id );
      screen_id = GetBackGroundScreenID( &p_list_data[i] );
      FRAMELIST_AddItem( p_wk->p_lwk, screen_id, TRUE );
    }
    GFL_MSG_Delete( p_msg );
  }

	// 点滅アニメパレット設定(リストカーソル)
  {
    // 選択時の色は明滅しない
    ARCHANDLE * ah;
    ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_HABITAT_LIST );
    FRAMELIST_LoadBlinkPalette( p_wk->p_lwk, ah, NARC_zukan_gra_list_list_bgd_NCLR, BLINK_PAL_NO_SELECT, BLINK_PAL_NO_SELECT );
    GFL_ARC_CloseDataHandle( ah );
  }

  // スクロール出来ないときはバーを非表示にする
	if( list_size <= ZKNLISTMAIN_LIST_POS_MAX )
  {
    GFL_CLWK *p_clwk;
    p_clwk= HABITAT_LIST_GRA_GetComClWk( p_wk ,SCROLL_BAR_CLWKID);
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
    p_clwk= HABITAT_LIST_GRA_GetComClWk( p_wk ,SCROLL_BUTTON_CLWKID);
    GFL_CLACT_WK_SetDrawEnable( p_clwk ,FALSE);
  }
  else
  {
    // スクロールバーの位置を補正
    HABITAT_LIST_GRA_GetScrollBarYPos( p_wk );
  }
	// 表示項目数
	if( list_size < ZKNLISTMAIN_LIST_POS_MAX ){
		p_wk->list_pos_max = list_size;
	}else{
		p_wk->list_pos_max = ZKNLISTMAIN_LIST_POS_MAX;
	}

  // リストサイズを格納
  p_wk->list_size = list_size;

  // リストデータを格納
  p_wk->p_list_data = p_list_data;

}

//----------------------------------------------------------------------------
/**
 *	@brief  状態に合わせたスクリーンIDを返す
 *
 *	@param	*p_list_data 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static s32 GetBackGroundScreenID( ZUKANHABITAT_LIST_LISTDATA *p_data )
{
  s32 index = -1;
  s32 i;
  BOOL is_notice = FALSE;
  BOOL is_cmp;
  u8 is_l,is_w,is_f;
 
  const BOOL land_type[][HABITAT_GROUND_MAX]=
  {
    {TRUE,FALSE,FALSE}, // 草地だけ
    {FALSE,TRUE,FALSE}, // 水辺だけ
    {TRUE,TRUE,FALSE},  // 草地、水辺
    {FALSE,TRUE,TRUE},  // 水辺、釣り
    {TRUE,TRUE,TRUE},   // 草地、水辺、釣り
  };
 
  // 指定地形が存在しているか取得する
  GetExistLandType( p_data, &is_l ,&is_w ,&is_f );

  for( i = 0; i < FLIST_BACK_NORMAL_MAX ; i++ )
  {
    if( (land_type[i][ HABITAT_GROUND_LAND ] == is_l ) &&
        (land_type[i][ HABITAT_GROUND_WATERSIDE ] == is_w ) &&
        (land_type[i][ HABITAT_GROUND_FISHING ] == is_f ) )
    {
      index = i;
      break;
    }
  }

  ONOUE_Printf("dt:%d %d %d\n",p_data->is_exist_landtype[ HABITAT_GROUND_LAND ],p_data->is_exist_landtype[ HABITAT_GROUND_WATERSIDE ],p_data->is_exist_landtype[ HABITAT_GROUND_FISHING ]);
  ONOUE_Printf("fg:%d %d %d\n",is_l,is_w,is_f);
  if( index < 0 )
  {
    GF_ASSERT_MSG(0,"未知の背景タイプが指定されています\n");
    index = 0;
  }

  // コンプリートチェック(非表示でもチェック)
  is_cmp = TRUE; 
  for( i = 0; i < HABITAT_GROUND_MAX ; i++ )
  {
    if( (p_data->is_exist_landtype[i] == TRUE ) &&
        (p_data->is_complete[i] != HABITAT_ZONE_STATUS_GET ) )
    {
      is_cmp = FALSE; 
      break;
    }
  }

  for( i = 0; i < HABITAT_GROUND_MAX ; i++ )
  {
    // お知らせマークチェック
    if( p_data->list_status[i] == HABITAT_LIST_STATUS_NOTICE )
    {
      is_notice = TRUE;
      break;
    }
  }

  // コンプリートしていた場合はずらす
  if( (is_cmp == TRUE) && ( is_notice == FALSE ) )
  {
    index += FLIST_BACK_COMP_OFFSET;
  }

  return index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  リストを開放
 *
 *	@param	p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_DeleteList( HABITAT_LIST_GRA_WK* p_wk )
{
  s32 i;
  for( i=0; i < p_wk->list_size; i++ )
  {
    // バッファーの消去 
    GFL_STR_DeleteBuffer( p_wk->p_place_name[i] );
  }
  FRAMELIST_Exit( p_wk->p_lwk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  リストコールバック：項目表示
 *
 * @param		work			図鑑リストワーク
 * @param		itemNum		項目番号
 * @param		util			項目で使用しているPRINT_UTIL
 * @param		py				表示Ｙ座標
 * @param		disp			描画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ListCallBack_Draw( void *work, u32 item_index, PRINT_UTIL *p_util, s16 py, BOOL disp )
{
  HABITAT_LIST_GRA_WK *p_wk = work;
  PRINT_QUE           *p_que;
  u32 prm;

  p_que = FRAMELIST_GetPrintQue( p_wk->p_lwk );
  PRINTTOOL_PrintColor( p_util, p_que, 0, 4, p_wk->p_place_name[item_index],
      p_wk->p_font, FCOL_WP01WN, PRINTTOOL_MODE_LEFT );

  // パラメータ取得
	prm = FRAMELIST_GetItemParam( p_wk->p_lwk, item_index );

  // アイコン描画
  if( prm == TRUE )
  {
    SetFrameMenuIcon( p_wk ,item_index ,py ,disp );
  }
  else
  {
    ONOUE_Printf("非表示-不正データ\n");
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リスト画面描画コールバック
 *
 *	@param	* work    リスト画面ワーク
 *	@param	item_index  リスト位置
 *	@param	flg       FALSE = 初期化時
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ListCallBack_Move( void * work, u32 item_index, BOOL flg )
{
  HABITAT_LIST_GRA_WK *p_wk = work;
  RefreshCursorPos( p_wk );

  if( GFL_UI_TP_GetTrg() )
  {
    HABITAT_LIST_GRA_StartMiniMapIconAnm( p_wk );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  リストコールバック：リストスクロール
 *
 * @param		work			図鑑リストワーク
 * @param		mv				スクロール値
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ListCallBack_Scroll( void * work, s8 mv )
{
  s32 i ;
  HABITAT_LIST_GRA_WK *p_wk = work;
  for( i = 0; i < LIST_ICON_MAX; i ++ )
  {
    GRA_ICON_DATA *p_icon_m = &p_wk->icon_data_main[i];
    GRA_ICON_DATA *p_icon_s = &p_wk->icon_data_sub[i];
    p_icon_m->pos.y -= mv;
    p_icon_s->pos.y -= mv;
    p_icon_m->is_refresh = TRUE;
    p_icon_s->is_refresh = TRUE;
  }
  
  SetCursorEnable( p_wk, FALSE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  リストアイコンを描画する
 *
 *	@param	*p_wk 
 *	@param	item_index  
 *	@param	py          リストの移動量
 *	@param	disp        TRUEの時メイン画面
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetFrameMenuIcon( HABITAT_LIST_GRA_WK *p_wk ,u32 item_index ,s16 py ,BOOL disp )
{
  s32 clwk_ofs = 0;
  GRA_ICON_DATA *p_icondata;
  GFL_CLACTPOS pos;
  ZUKANHABITAT_LIST_LISTDATA *p_data = &p_wk->p_list_data[item_index];
  u8 is_l,is_w,is_f;
 
  // HABITAT_LIST_STATUSと同じ順番で、アニメーションシーケンスを定義
  u16 anm_seq;
  u16 sc_anm_seq_tbl[] =
  {
    OBJ_ANM_SEQ_NEW_ICON,   // 新着アイコン
    OBJ_ANM_SEQ_EX_ICON,    // びっくりマーク
    OBJ_ANM_SEQ_GET_ICON,   // 捕獲遺恨
    OBJ_ANM_SEQ_LOOK_ICON,  // 見たアイコン
    0,                      // ダミー（なし）
  };
 
  // 指定地形が存在しているか取得する
  GetExistLandType( p_data, &is_l ,&is_w ,&is_f );

  if( disp == TRUE )
  {
    p_icondata = &p_wk->icon_data_main[(item_index%LIST_BAR_NUM)*LIST_ICON_NUM];
  }
  else
  {
    p_icondata = &p_wk->icon_data_sub[(item_index%LIST_BAR_NUM)*LIST_ICON_NUM];
  }

  // アイコンの位置を設定する
  pos.x = (ZKNLISTMAIN_LIST_PX*8)+LIST_ICON_X_OFFSET;
  pos.y = (ZKNLISTMAIN_LIST_PY*8)+LIST_ICON_Y_OFFSET + py;
  p_icondata[2].pos = pos;
  // 左
  pos.x += LIST_ICON_SET_W;
  p_icondata[1].pos = pos;
  // 中央
  pos.x += LIST_ICON_SET_W;
  p_icondata[0].pos = pos;

  // アイコンを全て非表示に
  p_icondata[0].draw_flag = FALSE;
  p_icondata[1].draw_flag = FALSE;
  p_icondata[2].draw_flag = FALSE;

  // 釣り、水辺、草地の順でつめる
  if( is_f == TRUE )
  {
    HABITAT_LIST_STATUS st = p_data->list_status[ HABITAT_GROUND_FISHING ];
    if( st != HABITAT_LIST_STATUS_NONE )
    {
      p_icondata[clwk_ofs].anm_seq = sc_anm_seq_tbl[ st ];
      p_icondata[clwk_ofs].draw_flag = TRUE;
    }
    clwk_ofs++;
  }
  if( is_w == TRUE )
  {
    HABITAT_LIST_STATUS st = p_data->list_status[ HABITAT_GROUND_WATERSIDE ];
    if( st != HABITAT_LIST_STATUS_NONE )
    {
      p_icondata[clwk_ofs].anm_seq = sc_anm_seq_tbl[ st ];
      p_icondata[clwk_ofs].draw_flag = TRUE;
    }
    clwk_ofs++;
  }
  if( is_l == TRUE )
  {
    HABITAT_LIST_STATUS st = p_data->list_status[ HABITAT_GROUND_LAND ];
    if( st != HABITAT_LIST_STATUS_NONE )
    {
      p_icondata[clwk_ofs].anm_seq = sc_anm_seq_tbl[ st ];
      p_icondata[clwk_ofs].draw_flag = TRUE;
    }
    clwk_ofs++;
  }

  // アイコンを更新状態にする
  p_icondata[0].is_refresh = TRUE;
  p_icondata[1].is_refresh = TRUE;
  p_icondata[2].is_refresh = TRUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーの位置を補正する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_GetScrollBarYPos( HABITAT_LIST_GRA_WK *p_wk )
{
	u32	py;
  GFL_CLACTPOS	pos;
  GFL_CLWK *p_clwk;

  // 取得
  p_clwk= HABITAT_LIST_GRA_GetComClWk( p_wk ,SCROLL_BUTTON_CLWKID);
  GFL_CLACT_WK_GetPos( p_clwk , &pos, CLSYS_DRAW_MAIN );
  py = FRAMELIST_GetScrollBarPY2( p_wk->p_lwk, pos.y );
  
  // 補正
  if( py < SCROLL_BAR_UY ){
    py = SCROLL_BAR_UY;
  }else if( py > SCROLL_BAR_DY ){
    py = SCROLL_BAR_DY;
  }

  // 座標をセット
  pos.x = SCROLL_BAR_PX;
  pos.y = py;
  GFL_CLACT_WK_SetPos( p_clwk , &pos, CLSYS_DRAW_MAIN );
}

//----------------------------------------------------------------------------
/**
 *	@brief  リストがジャンプした時に呼び出す
 *	新しく描画されたもののみ描画するようにする
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_RecoverDrawFlag( HABITAT_LIST_GRA_WK *p_wk )
{
  s32 i;
  // 古いアイコン表示情報を保存
  for( i = 0; i < LIST_ICON_MAX; i++ )
  {
    GRA_ICON_DATA *p_ic_m;
    GRA_ICON_DATA *p_ic_s;
    p_ic_m = &p_wk->icon_data_main[i];
    p_ic_s = &p_wk->icon_data_sub[i];
    // 更新が無かったものは非表示にする
    if( p_ic_m->is_refresh == FALSE )
    {
      p_ic_m->is_refresh = TRUE;
      p_ic_m->draw_flag = FALSE;
    }
    if( p_ic_s->is_refresh == FALSE )
    {
      p_ic_s->is_refresh = TRUE;
      p_ic_s->draw_flag = FALSE;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リスト位置を取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 HABITAT_LIST_GRA_GetListPos( HABITAT_LIST_GRA_WK *p_wk )
{
  return FRAMELIST_GetListPos( p_wk->p_lwk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソル位置を取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 HABITAT_LIST_GRA_GetCursorPos( HABITAT_LIST_GRA_WK *p_wk )
{
  return FRAMELIST_GetCursorPos( p_wk->p_lwk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  リストを下に下げれるかを取得
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL HABITAT_LIST_GRA_GetCanListDown( HABITAT_LIST_GRA_WK *p_wk )
{
  if( (FRAMELIST_CheckScrollMax( p_wk->p_lwk ) == FALSE ) &&
      ( (p_wk->list_pos_max -1)== HABITAT_LIST_GRA_GetCursorPos(p_wk)))
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールカウント取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u32 HABITAT_LIST_GRA_GetScrollCount( HABITAT_LIST_GRA_WK *p_wk )
{
  return FRAMELIST_GetScrollCount( p_wk->p_lwk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソル位置を設定する
 *
 *	@param	*p_wk 
 *	@param	cursor_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_SetCursorPos( HABITAT_LIST_GRA_WK *p_wk ,u32 cursor_pos )
{
  FRAMELIST_SetCursorPos( p_wk->p_lwk, cursor_pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  選択項目のパレットを変える
 *
 *	@param	*p_wk 
 *	@param	pal 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_ChageSelectItemPalette( HABITAT_LIST_GRA_WK *p_wk ,u8 pal )
{
  // パレットを変える
  u32 cursor_pos = FRAMELIST_GetCursorPos( p_wk->p_lwk );
  FRAMELIST_ChangePosPalette( p_wk->p_lwk,cursor_pos, pal );
  GFL_BG_LoadScreenV_Req( BGM_FRAME_LIST );
}

//----------------------------------------------------------------------------
/**
 *	@brief リストデータに関連付けされているデータを取得する
 *
 *	@param	*p_wk 
 *	@param	list_index 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
ZUKANHABITAT_LIST_LISTDATA* HABITAT_LIST_GRA_GetListData(
                                  HABITAT_LIST_GRA_WK *p_wk ,u32 list_index )
{
  return &p_wk->p_list_data[list_index];
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
void HABITAT_LIST_GRA_SetAnmSeq( HABITAT_LIST_GRA_WK *p_wk ,s32 clwkid ,u32 anm_seq )
{
  GFL_CLWK *p_clwk;
  p_clwk= HABITAT_LIST_GRA_GetComClWk( p_wk ,clwkid);
  GFL_CLACT_WK_SetAnmSeq( p_clwk, anm_seq );
  GFL_CLACT_WK_SetAnmFrame( p_clwk ,0 );
  GFL_CLACT_WK_SetAutoAnmFlag( p_clwk , TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーション起動チェック
 *
 *	@param	*p_wk 
 *	@param	clwkid 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL HABITAT_LIST_GRA_CheckAnmActive( HABITAT_LIST_GRA_WK *p_wk ,s32 clwkid )
{
  return GFL_CLACT_WK_CheckAnmActive( p_wk->p_clwk_common_obj[clwkid]);
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーションシーケンスを取得する
 *
 *	@param	*p_wk 
 *	@param	clwkid 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 HABITAT_LIST_GRA_GetAnmSeq( HABITAT_LIST_GRA_WK *p_wk ,s32 clwkid )
{
  return GFL_CLACT_WK_GetAnmSeq( p_wk->p_clwk_common_obj[clwkid]);
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルの表示状態を設定する
 *
 *	@param	*p_wk 
 *	@param	flg 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetCursorEnable( HABITAT_LIST_GRA_WK *p_wk , BOOL flg )
{
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_common_obj[LIST_CURSOR_CLWKID], flg );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソル位置をリフレッシュする
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void RefreshCursorPos( HABITAT_LIST_GRA_WK *p_wk )
{
  s32 i;
  s32 exist_cnt = -1;
  GFL_CLACTPOS	pos;
  GFL_CLWK *p_clwk = p_wk->p_clwk_common_obj[LIST_CURSOR_CLWKID];
  u32 cursor_pos = FRAMELIST_GetCursorPos( p_wk->p_lwk );
  u32 sc_cnt = HABITAT_LIST_GRA_GetScrollCount( p_wk );

  u32 anm_seq_table[] =
  {
    OBJ_ANM_SEQ_LIST_CURSOR01,
    OBJ_ANM_SEQ_LIST_CURSOR02,
    OBJ_ANM_SEQ_LIST_CURSOR03,
  };

  pos.y = (ZKNLISTMAIN_LIST_PY*8)+LIST_ICON_Y_OFFSET + (LIST_BAR_H*cursor_pos);
  pos.x = LIST_BAR_POS_X;

  // リストアイコンの位置決定
  for( i = 0; i < HABITAT_GROUND_MAX; i++ )
  {
    if( (p_wk->p_list_data[cursor_pos+sc_cnt].is_exist_landtype[i] == TRUE) &&
        (p_wk->p_list_data[cursor_pos+sc_cnt].is_vanish[i] == TRUE))
    {
      pos.x -= 16;
      exist_cnt ++;
    }
  }
  GFL_CLACT_WK_SetPos( p_clwk , &pos, CLSYS_DRAW_MAIN );
  
  GF_ASSERT( exist_cnt >= 0 );
  HABITAT_LIST_GRA_SetAnmSeq( p_wk,
      LIST_CURSOR_CLWKID,anm_seq_table[exist_cnt]);

}


//----------------------------------------------------------------------------
/**
 *	@brief  左右矢印ボタンのアニメーションシーケンスを設定する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetLRButtonAnm( HABITAT_LIST_GRA_WK *p_wk ,BOOL is_force )
{
  u32 sc_cnt = HABITAT_LIST_GRA_GetScrollCount( p_wk );
  u32 cursor_pos  = HABITAT_LIST_GRA_GetCursorPos( p_wk );

  if(((HABITAT_LIST_GRA_CheckAnmActive(p_wk,L_BUTTON_CLWKID ) == FALSE)||
      (HABITAT_LIST_GRA_GetAnmSeq(p_wk,L_BUTTON_CLWKID) != APP_COMMON_BARICON_CURSOR_LEFT_ON)) ||
      (is_force == TRUE ))
  {
    if( sc_cnt == 0 && cursor_pos == 0)
    {
      HABITAT_LIST_GRA_SetAnmSeq( p_wk,
          L_BUTTON_CLWKID,APP_COMMON_BARICON_CURSOR_LEFT_OFF);
    }
    else
    {
      HABITAT_LIST_GRA_SetAnmSeq( p_wk,
          L_BUTTON_CLWKID,APP_COMMON_BARICON_CURSOR_LEFT);
    }
  }

  if( ((HABITAT_LIST_GRA_CheckAnmActive( p_wk ,R_BUTTON_CLWKID ) == FALSE ) ||
      (HABITAT_LIST_GRA_GetAnmSeq(p_wk,R_BUTTON_CLWKID) != APP_COMMON_BARICON_CURSOR_RIGHT_ON)) ||
      (is_force == TRUE ))
  {
    if( HABITAT_LIST_GRA_GetCanListDown( p_wk ) )
    {
      HABITAT_LIST_GRA_SetAnmSeq( p_wk,
          R_BUTTON_CLWKID,APP_COMMON_BARICON_CURSOR_RIGHT_OFF);
    }
    else
    {
      HABITAT_LIST_GRA_SetAnmSeq( p_wk,
          R_BUTTON_CLWKID,APP_COMMON_BARICON_CURSOR_RIGHT);
    }
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
void HABITAT_LIST_GRA_AddScrollFrame( HABITAT_LIST_GRA_WK *p_wk ,s32 yval,BOOL is_main  )
{
  s32 i;
  GFL_CLACTPOS	pos;

  if( is_main )
  {
    // BGスクロール
	  GFL_BG_SetScrollReq( BGM_FRAME_MENUBAR, GFL_BG_SCROLL_Y_DEC, yval );
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
 *	@brief  ミニマップアイコンアニメーション
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_StartMiniMapIconAnm( HABITAT_LIST_GRA_WK *p_wk )
{
  GFL_CLWK *p_clwk = p_wk->p_clwk_common_obj[MINIMAP_CLWKID];
  if( p_wk->list_update == FALSE )
  {
    GFL_CLACT_WK_SetAnmFrame( p_clwk ,0 );
    GFL_CLACT_WK_SetAnmSeq( p_clwk, OBJ_ANM_SEQ_MINIMAP_ANM );
  }
  else
  {
    GFL_CLACT_WK_SetAnmSeq( p_clwk, OBJ_ANM_SEQ_MINIMAP );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  点滅アニメの停止状況を設定
 *
 *	@param	*p_wk 
 *	@param	is_stop 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITAT_LIST_GRA_SetBlinkStop( HABITAT_LIST_GRA_WK *p_wk ,BOOL is_stop )
{
  p_wk->is_stop_blink = is_stop;
}

//----------------------------------------------------------------------------
/**
 *	@brief  地形タイプが存在するかチェック
 *
 *	@param	*p_data 
 *	@param	is_l 
 *	@param	is_w 
 *	@param	is_f 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetExistLandType( ZUKANHABITAT_LIST_LISTDATA *p_data ,u8 *is_l ,u8 *is_w ,u8 *is_f )
{
  (*is_l) = p_data->is_exist_landtype[ HABITAT_GROUND_LAND ] & 
    p_data->is_vanish[ HABITAT_GROUND_LAND ];
  (*is_w) = p_data->is_exist_landtype[ HABITAT_GROUND_WATERSIDE ] &
    p_data->is_vanish[ HABITAT_GROUND_WATERSIDE ];
  (*is_f) = p_data->is_exist_landtype[ HABITAT_GROUND_FISHING ] &
    p_data->is_vanish[ HABITAT_GROUND_FISHING ];
}

