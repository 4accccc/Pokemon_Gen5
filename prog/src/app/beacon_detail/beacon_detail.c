//=============================================================================
/**
 *
 *	@file		beacon_detail.c
 *	@brief  すれ違い詳細画面
 *	@author	Miyuki Iwasawa
 *	@data		2010.02.01
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// 簡易会話表示システム
#include "system/pms_draw.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "beacon_detail_gra.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//マップモデルのnsbtx
#include "fieldmap/fldmmdl_mdlres.naix"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//ポケアイコン
#include "pokeicon/pokeicon.h"

//MCSS
#include "system/mcss.h"
#include "system/mcss_tool.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//どうぐアイコン
#include "item/item.h"
#include "item_icon.naix"

//ポケモンBG,OBJ読みこみ
#include "system/poke2dgra.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/beacon_detail.h"


//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
#include "mictest.naix"	// アーカイブ
#include "msg/msg_mictest.h"  // GMM
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "beacon_status.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース

#include "beacon_detail_def.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define BEACON_DETAIL_PMSDRAW       // 簡易会話表示

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
///	BG管理ワーク
//==============================================================
typedef struct 
{
	int dummy;
  ARCHANDLE* handle;
} BEACON_DETAIL_BG_WORK;


#ifdef BEACON_DETAIL_PMSDRAW
#define BEACON_DETAIL_PMSDRAW_NUM (3) ///< 簡易会話の個数
#endif // BEACON_DETAIL_PMSDRAW 

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;
  int                       seq;
	int											  sub_seq;

	BEACON_DETAIL_BG_WORK				wk_bg;

	//描画設定
	BEACON_DETAIL_GRAPHIC_WORK	*graphic;
  ARCHANDLE* handle;

	//タッチバー
	TOUCHBAR_WORK							*touchbar;

	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msg;

#ifdef	BEACON_DETAIL_PRINT_TOOL
	//プリントユーティリティ
	PRINT_UTIL								print_util;
#endif	//BEACON_DETAIL_PRINT_TOOL

#ifdef BEACON_DETAIL_PMSDRAW
  GFL_BMPWIN*               pms_win[ BEACON_DETAIL_PMSDRAW_NUM ];
  PMS_DRAW_WORK*            pms_draw;
#endif //BEACON_DETAIL_PMSDRAW

} BEACON_DETAIL_WORK;


//=============================================================================
/**
 *							データ定義
 */
//=============================================================================


//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT BeaconDetailProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT BeaconDetailProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT BeaconDetailProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );


static int seq_Main( BEACON_DETAIL_WORK* wk );
static int seq_FadeIn( BEACON_DETAIL_WORK* wk );
static int seq_FadeOut( BEACON_DETAIL_WORK* wk );

//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void BeaconDetail_BGResInit( BEACON_DETAIL_WORK* wk, HEAPID heapID );
static void BeaconDetail_BGResRelease( BEACON_DETAIL_WORK* wk );

//-------------------------------------
///	タッチバー
//=====================================
static TOUCHBAR_WORK * BeaconDetail_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID );
static void BeaconDetail_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar );
static void BeaconDetail_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );

#ifdef BEACON_DETAIL_PMSDRAW
//-------------------------------------
///	簡易会話表示
//=====================================
static void BeaconDetail_PMSDRAW_Init( BEACON_DETAIL_WORK* wk );
static void BeaconDetail_PMSDRAW_Exit( BEACON_DETAIL_WORK* wk );
static void BeaconDetail_PMSDRAW_Proc( BEACON_DETAIL_WORK* wk );
#endif // BEACON_DETAIL_PMSDRAW


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA BeaconDetailProcData = 
{
	BeaconDetailProc_Init,
	BeaconDetailProc_Main,
	BeaconDetailProc_Exit,
};
//=============================================================================
/**
 *								PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					BEACON_DETAIL_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BeaconDetailProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	BEACON_DETAIL_WORK *wk;
	BEACON_DETAIL_PARAM *param;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//引数取得
	param	= pwk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BEACON_DETAIL, BEACON_DETAIL_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(BEACON_DETAIL_WORK), HEAPID_BEACON_DETAIL );
  GFL_STD_MemClear( wk, sizeof(BEACON_DETAIL_WORK) );

  // 初期化
  wk->heapID = HEAPID_BEACON_DETAIL;
	
	//描画設定初期化
	wk->graphic	= BEACON_DETAIL_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_mictest_dat, wk->heapID );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );

	//BGリソース読み込み
	BeaconDetail_BGResInit( wk, wk->heapID );

	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= BEACON_DETAIL_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= BeaconDetail_TOUCHBAR_Init( clunit, wk->heapID );
	}

#ifdef BEACON_DETAIL_PMSDRAW  
  BeaconDetail_PMSDRAW_Init( wk );
#endif //BEACON_DETAIL_PMSDRAW

#if 0
	//@todo	フェードシーケンスがないので
	GX_SetMasterBrightness(0);
	GXS_SetMasterBrightness(0);
#endif

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					BEACON_DETAIL_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BeaconDetailProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	BEACON_DETAIL_WORK* wk = mywk;

	//タッチバー
	BeaconDetail_TOUCHBAR_Exit( wk->touchbar );

#ifdef BEACON_DETAIL_PMSDRAW
  BeaconDetail_PMSDRAW_Exit( wk );
#endif //BEACON_DETAIL_PMSDRAW
	
  //BGリソース破棄
	BeaconDetail_BGResRelease( wk );

	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );

	//描画設定破棄
	BEACON_DETAIL_GRAPHIC_Exit( wk->graphic );

	//PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heapID );

	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					BEACON_DETAIL_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BeaconDetailProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	BEACON_DETAIL_WORK* wk = mywk;

  switch(*seq){
  case SEQ_FADEIN:
    *seq = seq_FadeIn( wk );
    break;
  case SEQ_MAIN:
    *seq = seq_Main( wk );
    break;
  case SEQ_FADEOUT:
    *seq = seq_FadeOut( wk );
    break;
  case SEQ_EXIT:
    return GFL_PROC_RES_FINISH;
  }

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );

#ifdef BEACON_DETAIL_PMSDRAW
  BeaconDetail_PMSDRAW_Proc( wk );
#endif //BEACON_DETAIL_PMSDRAW

	//2D描画
	BEACON_DETAIL_GRAPHIC_2D_Draw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *								static関数
 */
//=============================================================================

/*
 *  @brief  メイン
 */
static int seq_Main( BEACON_DETAIL_WORK* wk )
{
  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
  {
    return SEQ_FADEOUT;
  }
	
  //タッチバーメイン処理
	BeaconDetail_TOUCHBAR_Main( wk->touchbar );

  return SEQ_MAIN;
}

/*
 *  @brief  フェードイン
 */
static int seq_FadeIn( BEACON_DETAIL_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    wk->seq++;
    break;
  case 1:
    if( !GFL_FADE_CheckFade() ){	
      wk->seq = 0;
			return SEQ_MAIN;
		}
    break;
  }
  return SEQ_FADEIN; 
}

/*
 *  @brief  フェードアウト
 */
static int seq_FadeOut( BEACON_DETAIL_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    wk->seq++;
    break;
  case 1:
    if( !GFL_FADE_CheckFade() ){	
      wk->seq = 0;
      return SEQ_EXIT; 
		}
    break;
  }
  return SEQ_FADEOUT; 
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	BEACON_DETAIL_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void BeaconDetail_BGResInit( BEACON_DETAIL_WORK* wk, HEAPID heapID )
{
  ARCHANDLE* tmap_h;

	wk->handle	= GFL_ARC_OpenDataHandle( ARCID_BEACON_STATUS, heapID );
	tmap_h	= GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( tmap_h, NARC_townmap_gra_tmap_bg_d_NCLR, PALTYPE_MAIN_BG, 0, 0x200, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( wk->handle, NARC_beacon_status_bdetail_bgu_nclr, PALTYPE_SUB_BG, 0, 0x200, heapID );

	//	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	wk->handle, NARC_beacon_status_bdetail_bgu_lz_ncgr,
						BG_FRAME_WIN01_S, 0, 0, TRUE, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	wk->handle, NARC_beacon_status_bdetail_bgu01_lz_nscr,
						BG_FRAME_WIN01_S, 0, 0, TRUE, heapID );		
	GFL_ARCHDL_UTIL_TransVramScreen(	wk->handle, NARC_beacon_status_bdetail_bgu01_lz_nscr,
						BG_FRAME_WIN02_S, 0, 0, TRUE, heapID );		
	
  //	----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	tmap_h, NARC_townmap_gra_tmap_bg_d_NCGR,
						BG_FRAME_MAP01_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	tmap_h, NARC_townmap_gra_tmap_root_d_NSCR,
						BG_FRAME_MAP01_M, 0, 0x800, 0, heapID );	
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	tmap_h, NARC_townmap_gra_tmap_map_d_NCGR,
						BG_FRAME_MAP02_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	tmap_h, NARC_townmap_gra_tmap_map_d_NSCR,
						BG_FRAME_MAP02_M, 0, 0x800, 0, heapID );	

  GFL_ARC_CloseDataHandle( tmap_h );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース解放
 *
 *	@param	BEACON_DETAIL_WORK* wk 管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void BeaconDetail_BGResRelease( BEACON_DETAIL_WORK* wk )
{
	GFL_ARC_CloseDataHandle( wk->handle );
}






//=============================================================================
/**
 *	TOUCHBAR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR初期化
 *
 *	@param	GFL_CLUNIT *clunit	CLUNIT
 *	@param	heapID							ヒープID
 *
 *	@return	TOUCHBAR_WORK
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK * BeaconDetail_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID )
{	
	//アイコンの設定
	//数分作る
	static const TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
	{	
		{	
			TOUCHBAR_ICON_RETURN,
			{	TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
		},
		{	
			TOUCHBAR_ICON_CLOSE,
			{	TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_Y },
		},
		{	
	    TOUCHBAR_ICON_CUR_D,	//↓ボタン
			{	TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_Y },
		},
		{	
	    TOUCHBAR_ICON_CUR_U,	//↓ボタン
			{	TOUCHBAR_ICON_X_04, TOUCHBAR_ICON_Y },
		},
	};

	//設定構造体
	//さきほどの窓情報＋リソース情報をいれる
	TOUCHBAR_SETUP	touchbar_setup;
	GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

	touchbar_setup.p_item		= (TOUCHBAR_ITEM_ICON*)touchbar_icon_tbl;				//上の窓情報
	touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
	touchbar_setup.p_unit		= clunit;										//OBJ読み込みのためのCLUNIT
	touchbar_setup.bar_frm	= BG_FRAME_BAR_M;						//BG読み込みのためのBG面
	touchbar_setup.bg_plt		= PLTID_BG_TOUCHBAR_M;			//BGﾊﾟﾚｯﾄ
	touchbar_setup.obj_plt	= PLTID_OBJ_TOUCHBAR_M;			//OBJﾊﾟﾚｯﾄ
	touchbar_setup.mapping	= APP_COMMON_MAPPING_128K;	//マッピングモード

	return TOUCHBAR_Init( &touchbar_setup, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR破棄
 *
 *	@param	TOUCHBAR_WORK	*touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void BeaconDetail_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Exit( touchbar );
}

//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBARメイン処理
 *
 *	@param	TOUCHBAR_WORK	*touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void BeaconDetail_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}

#ifdef	BEACON_DETAIL_PRINT_TOOL
//=============================================================================
/**
 *	PRINT_TOOL
 */
//=============================================================================
/*
		・「HP ??/??」を表示するサンプルです
		・BMPWINのサイズは 20 x 2 です
		・現在のHP = 618, 最大HP = 999 とします
		・サンプルメインが FALSE を返すと終了です
*/

// サンプルメイン
static BOOL PrintTool_MainFunc( BEACON_DETAIL_WORK * wk )
{
	switch( wk->sub_seq ){
	case 0:
		PrintTool_AddBmpWin( wk );			// BMPWIN作成
		PrintTool_PrintHP( wk );				// ＨＰ表示
		PrintTool_ScreenTrans( wk );		// スクリーン転送
		wk->sub_seq = 1;
		break;

	case 1:
		// プリント終了待ち
		if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
			wk->sub_seq = 2;
		}
		break;

	case 2:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
			PrintTool_DelBmpWin( wk );		// BMPWIN削除
			return FALSE;
		}
	}

	PRINT_UTIL_Trans( &wk->print_util, wk->que );
	return TRUE;
}

// BMPWIN作成
static void PrintTool_AddBmpWin( BEACON_DETAIL_WORK * wk )
{
	wk->print_util.win = GFL_BMPWIN_Create(
													GFL_BG_FRAME0_M,					// ＢＧフレーム
													1, 1,											// 表示座標
													20, 2,										// 表示サイズ
													15,												// パレット
													GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
}

// BMPWIN削除
static void PrintTool_DelBmpWin( BEACON_DETAIL_WORK * wk )
{
		GFL_BMPWIN_Delete( wk->print_util.win );
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( BEACON_DETAIL_WORK * wk )
{
	GFL_BMPWIN_MakeScreen( wk->print_util.win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util.win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( BEACON_DETAIL_WORK * wk )
{
	STRBUF * strbuf;

	// BMPWIN内の座標(32,0)を基準に右詰めで「ＨＰ」と表示
	PRINTTOOL_Print(
				&wk->print_util,				// プリントユーティル ( BMPWIN )
				wk->que,								// プリントキュー
				32, 0,									// 表示基準座標
				strbuf,									// 文字列（すでに「ＨＰ」が展開されているものとする）
				wk->font,								// フォント
				PRINTTOOL_MODE_RIGHT );	// 右詰

	// BMPWIN内の座標(100,0)を「／」の中心にしてＨＰを表示 ( hp / mhp )
	PRINTTOOL_PrintFraction(
				&wk->print_util,				// プリントユーティル ( BMPWIN )
				wk->que,								// プリントキュー
				wk->font,								// フォント
				100, 0,									// 表示基準座標
				618,										// hp
				999,										// mhp
				wk->heapID );						// ヒープＩＤ
}
#endif	//BEACON_DETAIL_PRINT_TOOL

#ifdef BEACON_DETAIL_PMSDRAW

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 初期化処理
 *
 *	@param	BEACON_DETAIL_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void BeaconDetail_PMSDRAW_Init( BEACON_DETAIL_WORK* wk )
{
	GFL_CLUNIT	*clunit;
  
  clunit = BEACON_DETAIL_GRAPHIC_GetClunit( wk->graphic );

  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_SUB, wk->print_que, wk->font, 
      PLTID_OBJ_PMS_DRAW, BEACON_DETAIL_PMSDRAW_NUM ,wk->heapID );
  
  {
    int i;
    PMS_DATA pms;

    // PMS表示用BMPWIN生成
    for( i=0; i<BEACON_DETAIL_PMSDRAW_NUM; i++ )
    {
      wk->pms_win[i] = GFL_BMPWIN_Create(
          BG_FRAME_DAT01_S,					// ＢＧフレーム
          2+4*i, 0 + 6 * i,					  	// 表示座標(キャラ単位)
          28, 4,    							  // 表示サイズ
          15,												// パレット
          GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
    }
    
    // 1個目 通常+空欄表示
    PMSDAT_SetDebug( &pms );
    PMSDAT_SetWord( &pms, 1, PMS_WORD_NULL );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[0], &pms ,0 );

    // 2個目 デコメ
    PMSDAT_SetDeco( &pms, 1, PMS_DECOID_HERO );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[1], &pms ,1 );
    
    // 3個目 デコメ二個表示
    PMSDAT_SetDeco( &pms, 0, PMS_DECOID_TANKS );
    PMSDAT_SetDeco( &pms, 1, PMS_DECOID_LOVE );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[2], &pms ,2 );
    
    // 1の要素を2にコピー(移動表現などにご使用ください。)
    PMS_DRAW_Copy( wk->pms_draw, 1, 2 );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 後処理
 *
 *	@param	BEACON_DETAIL_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void BeaconDetail_PMSDRAW_Exit( BEACON_DETAIL_WORK* wk )
{
  PMS_DRAW_Exit( wk->pms_draw );
  {
    int i;
    for( i=0; i<BEACON_DETAIL_PMSDRAW_NUM; i++ )
    {
      GFL_BMPWIN_Delete( wk->pms_win[i] );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 主処理
 *
 *	@param	BEACON_DETAIL_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void BeaconDetail_PMSDRAW_Proc( BEACON_DETAIL_WORK* wk )
{
#if 1
  // SELECTでクリア
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    int i;
    for( i=0; i<BEACON_DETAIL_PMSDRAW_NUM; i++ )
    {
      PMS_DRAW_Clear( wk->pms_draw, i, TRUE );
    }
  }
  // STARTでランダム挿入（二重登録するとアサート）
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    int i;
    for( i=0; i<BEACON_DETAIL_PMSDRAW_NUM; i++ )
    {
      PMS_DATA pms;
      PMSDAT_SetDebugRandom( &pms );
      PMSDAT_SetDeco( &pms, 0, GFUser_GetPublicRand(10)+1 );
      PMS_DRAW_Print( wk->pms_draw, wk->pms_win[i], &pms ,i );
    }
  }
#endif

  PMS_DRAW_Main( wk->pms_draw );
}

#endif // BEACON_DETAIL_PMSDRAW

