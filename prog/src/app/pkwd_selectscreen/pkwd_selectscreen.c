//=============================================================================
/**
 *
 *	@file		pkwd_select_screen.c
 *	@brief  ポケウッド上映選択画面
 *	@author	mori
 *	@data		2011.07.19
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
#include "pkwd_selectscreen_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//マップモデルのnsbtx
#include "fieldmap/fldmmdl_mdlres.naix"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

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

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/pkwd_selectscreen.h"


//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
#include "pkwd_announce_gra.naix"	// アーカイブ
#include "msg/msg_mictest.h"  // GMM
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define PKWD_SELECT_SCREEN_TOUCHBAR
#define PKWD_SELECT_SCREEN_TASKMENU
//#define PKWD_SELECT_SCREEN_POKE2D				// ポケモンBG,OBJ読みこみ

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  PKWD_SELECT_SCREEN_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};


//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_BAR_M   = GFL_BG_FRAME1_M,
	BG_FRAME_POKE_M  = GFL_BG_FRAME2_M,
	BG_FRAME_BACK_M  = GFL_BG_FRAME3_M,
	BG_FRAME_BACK_S  = GFL_BG_FRAME2_S,
	BG_FRAME_FRAME_S = GFL_BG_FRAME1_S,
  BG_FRAME_TEXT_S  = GFL_BG_FRAME0_S, 
};
//-------------------------------------
///	パレット
//=====================================
enum
{	
	//メインBG
	PLTID_BG_BACK_M				= 0,
	PLTID_BG_POKE_M				= 1,
	PLTID_BG_TASKMENU_M		= 11,
	PLTID_BG_TOUCHBAR_M		= 13,
	PLTID_BG_INFOWIN_M		= 15,
	//サブBG
	PLTID_BG_BACK_S				=	0,

	//メインOBJ
	PLTID_OBJ_TOUCHBAR_M	= 0, // 3本使用
	PLTID_OBJ_TYPEICON_M	= 3, // 3本使用
  PLTID_OBJ_OAM_MAPMODEL_M = 6, // 1本使用
  PLTID_OBJ_POKEICON_M = 7,     // 3本使用
  PLTID_OBJ_POKEITEM_M = 10,    // 1本使用
  PLTID_OBJ_ITEMICON_M = 11,
  PLTID_OBJ_POKE_M = 12,
  PLTID_OBJ_BALLICON_M = 13, // 1本使用
	PLTID_OBJ_TOWNMAP_M	= 14,		
	//サブOBJ
  PLTID_OBJ_PMS_DRAW = 0, // 5本使用
};

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
} PKWD_SELECT_SCREEN_BG_WORK;



//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

	PKWD_SELECT_SCREEN_BG_WORK				wk_bg;

	//描画設定
	PKWD_SELECT_SCREEN_GRAPHIC_WORK	*graphic;

#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバー
	TOUCHBAR_WORK							*touchbar;
	//以下カスタムボタン使用する場合のサンプルリソース
	u32												ncg_btn;
	u32												ncl_btn;
	u32												nce_btn;
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msg;

#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//タスクメニュー
	APP_TASKMENU_RES					*menu_res;
	APP_TASKMENU_WORK					*menu;
#endif //PKWD_SELECT_SCREEN_TASKMENU




#ifdef PKWD_SELECT_SCREEN_POKE2D
	u32												ncg_poke2d;
	u32												ncl_poke2d;
	u32												nce_poke2d;
	GFL_CLWK									*clwk_poke2d;
#endif //PKWD_SELECT_SCREEN_POKE2D


#ifdef	PKWD_SELECT_SCREEN_PRINT_TOOL
	//プリントユーティリティ
	PRINT_UTIL								print_util;
	u32												seq;
#endif	//PKWD_SELECT_SCREEN_PRINT_TOOL


} PKWD_SELECT_SCREEN_MAIN_WORK;


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
static GFL_PROC_RESULT PkwdSelectScreenProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdSelectScreenProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PkwdSelectScreenProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void PkwdSelectScreen_BG_LoadResource( PKWD_SELECT_SCREEN_BG_WORK* wk, HEAPID heapID );


#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
//-------------------------------------
///	タッチバー
//=====================================
static TOUCHBAR_WORK * PkwdSelectScreen_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID );
static void PkwdSelectScreen_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar );
static void PkwdSelectScreen_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );
//以下カスタムボタン使用サンプル用
static TOUCHBAR_WORK * PkwdSelectScreen_TOUCHBAR_InitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void PkwdSelectScreen_TOUCHBAR_ExitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU
//-------------------------------------
///	リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * PkwdSelectScreen_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID );
static void PkwdSelectScreen_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void PkwdSelectScreen_TASKMENU_Main( APP_TASKMENU_WORK *menu );
#endif //PKWD_SELECT_SCREEN_TASKMENU



#ifdef PKWD_SELECT_SCREEN_POKE2D
//-------------------------------------
///	ポケモンOBJ,BG読みこみ
//=====================================
static void PkwdSelectScreen_POKE2D_LoadResourceBG( PKWD_SELECT_SCREEN_MAIN_WORK *wk, HEAPID heapID );
static void PkwdSelectScreen_POKE2D_UnLoadResourceBG( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_POKE2D_LoadResourceOBJ( PKWD_SELECT_SCREEN_MAIN_WORK *wk, HEAPID heapID );
static void PkwdSelectScreen_POKE2D_UnLoadResourceOBJ( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_POKE2D_CreateCLWK( PKWD_SELECT_SCREEN_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void PkwdSelectScreen_POKE2D_DeleteCLWK( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
#endif //PKWD_SELECT_SCREEN_POKE2D




//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA PkwdSelectScreenProcData = 
{
	PkwdSelectScreenProc_Init,
	PkwdSelectScreenProc_Main,
	PkwdSelectScreenProc_Exit,
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
 *	@param	*pwk					PKWD_SELECT_SCREEN_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdSelectScreenProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PKWD_SELECT_SCREEN_MAIN_WORK *wk;
	PKWD_SELECT_SCREEN_PARAM *param;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//引数取得
	param	= pwk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PKWD_SELSCR, PKWD_SELECT_SCREEN_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(PKWD_SELECT_SCREEN_MAIN_WORK), HEAPID_PKWD_SELSCR );
  GFL_STD_MemClear( wk, sizeof(PKWD_SELECT_SCREEN_MAIN_WORK) );

  // 初期化
  wk->heapID = HEAPID_PKWD_SELSCR;
	
	//描画設定初期化
	wk->graphic	= PKWD_SELECT_SCREEN_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_mictest_dat, wk->heapID );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );

	//BGリソース読み込み
	PkwdSelectScreen_BG_LoadResource( &wk->wk_bg, wk->heapID );


#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= PKWD_SELECT_SCREEN_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= PkwdSelectScreen_TOUCHBAR_InitEx( wk, clunit, wk->heapID );
	}
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_BAR_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
	wk->menu			= PkwdSelectScreen_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );
#endif //PKWD_SELECT_SCREEN_TASKMENU


#ifdef PKWD_SELECT_SCREEN_POKE2D
	//ポケモンOBJ,BG読みこみ
	PkwdSelectScreen_POKE2D_LoadResourceBG( wk, wk->heapID );
	{	
		GFL_CLUNIT *clunit = PKWD_SELECT_SCREEN_GRAPHIC_GetClunit( wk->graphic );
		PkwdSelectScreen_POKE2D_LoadResourceOBJ( wk, wk->heapID );
		PkwdSelectScreen_POKE2D_CreateCLWK( wk, clunit, wk->heapID );
	}
#endif //PKWD_SELECT_SCREEN_POKE2D

	//@todo	フェードシーケンスがないので
	GX_SetMasterBrightness(0);
	GXS_SetMasterBrightness(0);

  GFL_NET_ReloadIconTopOrBottom( FALSE, HEAPID_PKWD_SELSCR);
  
  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					PKWD_SELECT_SCREEN_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdSelectScreenProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	PKWD_SELECT_SCREEN_MAIN_WORK* wk = mywk;


#ifdef PKWD_SELECT_SCREEN_POKE2D
	PkwdSelectScreen_POKE2D_DeleteCLWK( wk );
	PkwdSelectScreen_POKE2D_UnLoadResourceOBJ( wk );
	PkwdSelectScreen_POKE2D_UnLoadResourceBG( wk );
#endif //PKWD_SELECT_SCREEN_POKE2D


#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//TASKMENUシステム＆リソース破棄
	PkwdSelectScreen_TASKMENU_Exit( wk->menu );
	APP_TASKMENU_RES_Delete( wk->menu_res );	
#endif //PKWD_SELECT_SCREEN_TASKMENU


#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバー
	PkwdSelectScreen_TOUCHBAR_ExitEx( wk );
#endif //PKWD_SELECT_SCREEN_TOUCHBAR



	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );

	//描画設定破棄
	PKWD_SELECT_SCREEN_GRAPHIC_Exit( wk->graphic );

	//PROC用メモリ解放
  {
    HEAPID heapID = wk->heapID;
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( heapID );
  }

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
 *	@param	*pwk					PKWD_SELECT_SCREEN_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PkwdSelectScreenProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	PKWD_SELECT_SCREEN_MAIN_WORK* wk = mywk;

  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG )
  {
    return GFL_PROC_RES_FINISH;
  }

#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバーメイン処理
	PkwdSelectScreen_TOUCHBAR_Main( wk->touchbar );
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//タスクメニューメイン処理
	PkwdSelectScreen_TASKMENU_Main( wk->menu );
#endif //PKWD_SELECT_SCREEN_TASKMENU

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );


	//2D描画
	PKWD_SELECT_SCREEN_GRAPHIC_2D_Draw( wk->graphic );


  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *								static関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	PKWD_SELECT_SCREEN_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_BG_LoadResource( PKWD_SELECT_SCREEN_BG_WORK* wk, HEAPID heapID )
{
  //@TODO とりあえずマイクテストのリソース
  //@TODO 複数枚ロードできるようにして切り離す？
  enum
  {
    RES_ARC = ARCID_PKWD_ANNOUNCE,

    RES_PAL_01S_SIZE = 0x20*4,
    RES_PAL_02S_SIZE = 0x20,
    RES_PAL_M_SIZE = 0x20*5,

    // ロゴの周りの枠
    RES_PAL_01S_ID = NARC_pkwd_announce_gra_pkwd_title_select_u_NCLR,
    RES_CHR_01S_ID = NARC_pkwd_announce_gra_pkwd_title_select_u_NCGR,
    RES_SCR_01S_ID = NARC_pkwd_announce_gra_pkwd_title_select_u01_NSCR,

    // ロゴ（デフォルト）
    RES_PAL_02S_ID = NARC_pkwd_announce_gra_pkwd_title01_logo_NCLR,
    RES_CHR_02S_ID = NARC_pkwd_announce_gra_pkwd_title01_logo_NCGR,
    RES_SCR_02S_ID = NARC_pkwd_announce_gra_pkwd_title01_NSCR,

    // 下画面
    RES_PAL_M_ID = NARC_pkwd_announce_gra_pkwd_title_select_d_NCLR,
    RES_CHR_M_ID = NARC_pkwd_announce_gra_pkwd_title_select_d_NCGR,
    RES_SCR_M_ID = NARC_pkwd_announce_gra_pkwd_title_select_d01_NSCR,

  };

	ARCHANDLE	*handle;

	handle	= GFL_ARC_OpenDataHandle( RES_ARC, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_M_ID, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, RES_PAL_M_SIZE, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_01S_ID, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_01S_SIZE, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_02S_ID, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_02S_SIZE, heapID );
	
  //	----- SUB -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_01S_ID, BG_FRAME_FRAME_S, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_01S_ID,	BG_FRAME_FRAME_S, 0, 0, 0, heapID );	
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_02S_ID, BG_FRAME_BACK_S,  0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_02S_ID,	BG_FRAME_BACK_S,  0, 0, 0, heapID );	

	//	----- MAIN -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_M_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_M_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );		

	GFL_ARC_CloseDataHandle( handle );
}






#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
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
static TOUCHBAR_WORK * PkwdSelectScreen_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID )
{	
	//アイコンの設定
	//数分作る
	TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
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
			TOUCHBAR_ICON_CHECK,
			{	TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_Y_CHECK },
		},
	};

	//設定構造体
	//さきほどの窓情報＋リソース情報をいれる
	TOUCHBAR_SETUP	touchbar_setup;
	GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

	touchbar_setup.p_item		= touchbar_icon_tbl;				//上の窓情報
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
static void PkwdSelectScreen_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Exit( touchbar );
}

//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR初期化	+ カスタムアイコン版
 *
 * @param  wk										ワーク
 *	@param	GFL_CLUNIT *clunit	CLUNIT
 *	@param	heapID							ヒープID
 *
 *	@return	TOUCHBAR_WORK
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK * PkwdSelectScreen_TOUCHBAR_InitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
{	

	//リソース読みこみ
	//サンプルとしてタウンマップの拡大縮小アイコンをカスタムボタンに登録する
	{	
		ARCHANDLE *handle;
		//ハンドル
		handle	= GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );
		//リソース読みこみ
		wk->ncg_btn	= GFL_CLGRP_CGR_Register( handle,
					NARC_townmap_gra_tmap_obj_d_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );
		wk->ncl_btn	= GFL_CLGRP_PLTT_RegisterEx( handle,
				NARC_townmap_gra_tmap_bg_d_NCLR, CLSYS_DRAW_MAIN, PLTID_OBJ_TOWNMAP_M*0x20,
				2, 2, heapID );	
		wk->nce_btn	= GFL_CLGRP_CELLANIM_Register( handle,
					NARC_townmap_gra_tmap_obj_d_NCER,
				 	NARC_townmap_gra_tmap_obj_d_NANR, heapID );
	
		GFL_ARC_CloseDataHandle( handle ) ;
	}

	//タッチバーの設定
	{	
	//アイコンの設定
	//数分作る
	TOUCHBAR_SETUP	touchbar_setup;
	TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
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
			TOUCHBAR_ICON_CHECK,
			{	TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_Y_CHECK }
		},
		{	
			TOUCHBAR_ICON_CUTSOM1,	//カスタムボタン１を拡大縮小アイコンに,
			{	TOUCHBAR_ICON_X_04, TOUCHBAR_ICON_Y },
		},
	};

	//以下カスタムボタンならば入れなくてはいけない情報
	touchbar_icon_tbl[3].cg_idx	=  wk->ncg_btn;				//キャラリソース
	touchbar_icon_tbl[3].plt_idx	= wk->ncl_btn;				//パレットリソース
	touchbar_icon_tbl[3].cell_idx	=	wk->nce_btn;				//セルリソース
	touchbar_icon_tbl[3].active_anmseq	=	8;						//アクティブのときのアニメ
	touchbar_icon_tbl[3].noactive_anmseq	=		12;						//ノンアクティブのときのアニメ
	touchbar_icon_tbl[3].push_anmseq	=		10;						//押したときのアニメ（STOPになっていること）
	touchbar_icon_tbl[3].key	=		PAD_BUTTON_START;		//キーで押したときに動作させたいならば、ボタン番号
	touchbar_icon_tbl[3].se	=		0;									//押したときにSEならしたいならば、SEの番号

	//設定構造体
	//さきほどの窓情報＋リソース情報をいれる
	GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

	touchbar_setup.p_item		= touchbar_icon_tbl;				//上の窓情報
	touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
	touchbar_setup.p_unit		= clunit;										//OBJ読み込みのためのCLUNIT
	touchbar_setup.bar_frm	= BG_FRAME_BAR_M;						//BG読み込みのためのBG面
	touchbar_setup.bg_plt		= PLTID_BG_TOUCHBAR_M;			//BGﾊﾟﾚｯﾄ
	touchbar_setup.obj_plt	= PLTID_OBJ_TOUCHBAR_M;			//OBJﾊﾟﾚｯﾄ
	touchbar_setup.mapping	= APP_COMMON_MAPPING_128K;	//マッピングモード

	return TOUCHBAR_Init( &touchbar_setup, heapID );
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR破棄	カスタムボタン版
 *
 *	@param	TOUCHBAR_WORK	*touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_TOUCHBAR_ExitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{	
	//タッチバー破棄
	TOUCHBAR_Exit( wk->touchbar );

	//リソース破棄
	{	
		GFL_CLGRP_PLTT_Release( wk->ncl_btn );
		GFL_CLGRP_CGR_Release( wk->ncg_btn );
		GFL_CLGRP_CELLANIM_Release( wk->nce_btn );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBARメイン処理
 *
 *	@param	TOUCHBAR_WORK	*touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの初期化
 *
 *	@param	menu_res	リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * PkwdSelectScreen_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID )
{	
	int i;
	APP_TASKMENU_INITWORK	init;
	APP_TASKMENU_ITEMWORK	item[3];
	APP_TASKMENU_WORK			*menu;	

	//窓の設定
	for( i = 0; i < NELEMS(item); i++ )
	{	
		item[i].str	= GFL_MSG_CreateString( msg, 0 );	//文字列
		item[i].msgColor	= APP_TASKMENU_ITEM_MSGCOLOR;	//文字色
		item[i].type			= APP_TASKMENU_WIN_TYPE_NORMAL;	//窓の種類
	}

	//初期化
	init.heapId		= heapID;
	init.itemNum	= NELEMS(item);
	init.itemWork = item;
	init.posType	= ATPT_RIGHT_DOWN;
	init.charPosX	= 32;
	init.charPosY = 21;
	init.w				= APP_TASKMENU_PLATE_WIDTH;
	init.h				= APP_TASKMENU_PLATE_HEIGHT;

	menu	= APP_TASKMENU_OpenMenu( &init, menu_res );

	//文字列解放
	for( i = 0; i < NELEMS(item); i++ )
	{	
		GFL_STR_DeleteBuffer( item[i].str );
	}

	return menu;

}
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの破棄
 *
 *	@param	APP_TASKMENU_WORK *menu	ワーク
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_CloseMenu( menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUのメイン処理
 *
 *	@param	APP_TASKMENU_WORK *menu	ワーク
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_UpdateMenu( menu );
}
#endif //PKWD_SELECT_SCREEN_TASKMENU




#ifdef PKWD_SELECT_SCREEN_POKE2D
//=============================================================================
/**
 *	ポケモンOBJ,BG読みこみ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンBG読みこみ
 *
 *	@param	PKWD_SELECT_SCREEN_MAIN_WORK *wk	ワーク
 *	@param	heapID										ヒープID
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_POKE2D_LoadResourceBG( PKWD_SELECT_SCREEN_MAIN_WORK *wk, HEAPID heapID )
{	

	POKEMON_PASO_PARAM	*ppp;
	
	//PPP作成
	ppp	= (POKEMON_PASO_PARAM	*)PP_Create( MONSNO_KAMEKKUSU, 0, 0, heapID );

	//1キャラ目に空白キャラいれて
	GFL_BG_FillCharacter( BG_FRAME_POKE_M, 0, 1,  0 );

	//ポケモンの絵を転送
	//1は↑で1キャラ作成したので。
	POKE2DGRA_BG_TransResourcePPP( ppp, POKEGRA_DIR_FRONT, BG_FRAME_POKE_M,
			1, PLTID_BG_POKE_M, heapID );

	//スクリーンに書き込んで転送
	POKE2DGRA_BG_WriteScreen( BG_FRAME_POKE_M, 1, PLTID_BG_POKE_M, 32/2-12/2+12, 24/2-12/2 );
	GFL_BG_LoadScreenReq( BG_FRAME_POKE_M );

	//PPP破棄
	GFL_HEAP_FreeMemory( ppp );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンBG破棄
 *
 *	@param	PKWD_SELECT_SCREEN_MAIN_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_POKE2D_UnLoadResourceBG( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{		
	GFL_BG_FillCharacterRelease( BG_FRAME_POKE_M, 1, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief	リソース読みこみ
 *
 *	@param	PKWD_SELECT_SCREEN_MAIN_WORK *wk	ワーク
 *	@param	heapID										ヒープID
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_POKE2D_LoadResourceOBJ( PKWD_SELECT_SCREEN_MAIN_WORK *wk, HEAPID heapID )
{	
	POKEMON_PASO_PARAM	*ppp;
	ARCHANDLE						*handle;

	//PPP作成
	ppp	= (POKEMON_PASO_PARAM	*)PP_Create( MONSNO_RIZAADON, 0, 0, heapID );

	//ハンドル
	handle	= POKE2DGRA_OpenHandle( heapID );
	//リソース読みこみ
	wk->ncg_poke2d	= POKE2DGRA_OBJ_CGR_RegisterPPP( handle, ppp, POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, heapID );
	wk->ncl_poke2d	= POKE2DGRA_OBJ_PLTT_RegisterPPP( handle, ppp, POKEGRA_DIR_FRONT ,CLSYS_DRAW_MAIN,  PLTID_OBJ_POKE_M*0x20,  heapID );
	wk->nce_poke2d	= POKE2DGRA_OBJ_CELLANM_RegisterPPP( ppp, POKEGRA_DIR_FRONT, APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, heapID );
	GFL_ARC_CloseDataHandle( handle );

	//PP破棄
	GFL_HEAP_FreeMemory( ppp );
}
//----------------------------------------------------------------------------
/**
 *	@brief	リソース破棄
 *
 *	@param	PKWD_SELECT_SCREEN_MAIN_WORK *wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_POKE2D_UnLoadResourceOBJ( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{	
	//リソース破棄
	GFL_CLGRP_PLTT_Release( wk->ncl_poke2d );
	GFL_CLGRP_CGR_Release( wk->ncg_poke2d );
	GFL_CLGRP_CELLANIM_Release( wk->nce_poke2d );

}
//----------------------------------------------------------------------------
/**
 *	@brief	CLWK作成
 *
 *	@param	PKWD_SELECT_SCREEN_MAIN_WORK *wk	ワーク
 *	@param	*clunit	ユニット
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_POKE2D_CreateCLWK( PKWD_SELECT_SCREEN_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
{	
	GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		cldata.pos_x	= 38;
		cldata.pos_y	= 80;
		wk->clwk_poke2d	= GFL_CLACT_WK_Create( clunit, 
				wk->ncg_poke2d,
				wk->ncl_poke2d,
				wk->nce_poke2d,
				&cldata, 
				CLSYS_DEFREND_MAIN, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	CLWK破棄
 *
 *	@param	PKWD_SELECT_SCREEN_MAIN_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
static void PkwdSelectScreen_POKE2D_DeleteCLWK( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{	
	GFL_CLACT_WK_Remove( wk->clwk_poke2d );
}
#endif //PKWD_SELECT_SCREEN_POKE2D


#ifdef	PKWD_SELECT_SCREEN_PRINT_TOOL
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
static BOOL PrintTool_MainFunc( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
	switch( wk->seq ){
	case 0:
		PrintTool_AddBmpWin( wk );			// BMPWIN作成
		PrintTool_PrintHP( wk );				// ＨＰ表示
		PrintTool_ScreenTrans( wk );		// スクリーン転送
		wk->seq = 1;
		break;

	case 1:
		// プリント終了待ち
		if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
			wk->seq = 2;
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
static void PrintTool_AddBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
	wk->print_util.win = GFL_BMPWIN_Create(
													GFL_BG_FRAME0_M,					// ＢＧフレーム
													1, 1,											// 表示座標
													20, 2,										// 表示サイズ
													15,												// パレット
													GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
}

// BMPWIN削除
static void PrintTool_DelBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
		GFL_BMPWIN_Delete( wk->print_util.win );
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
	GFL_BMPWIN_MakeScreen( wk->print_util.win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util.win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
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
#endif	//PKWD_SELECT_SCREEN_PRINT_TOOL


