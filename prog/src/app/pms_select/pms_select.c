//=============================================================================
/**
 *
 *	@file		pms_select.c
 *	@brief  登録済み簡易会話選択画面
 *	@author	hosaka genya
 *	@data		2009.10.20
 *
 */
//=============================================================================

//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// 簡易会話表示システム
#include "system/pms_draw.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"
#include "print/wordset.h"

//描画設定
#include "pms_select_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//タッチバー
#include "ui/touchbar.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"
#include "pmsi.naix"
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "message.naix"
#include "msg/msg_pms_input.h"

//外部公開
#include "app/pms_select.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define PMS_SELECT_TOUCHBAR
#define PMS_SELECT_TASKMENU
#define PMS_SELECT_PMSDRAW       // 簡易会話表示

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  PMS_SELECT_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};


//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_BAR_M	= GFL_BG_FRAME1_M,
	BG_FRAME_POKE_M	= GFL_BG_FRAME2_M,
	BG_FRAME_BACK_M	= GFL_BG_FRAME3_M,
	BG_FRAME_BACK_S	= GFL_BG_FRAME2_S,
  BG_FRAME_TEXT_S = GFL_BG_FRAME0_S, 
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
  PLTID_OBJ_PMS_DRAW = 0, // 2-5本使用 // @TODO 未定
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
} PMS_SELECT_BG_WORK;


#ifdef PMS_SELECT_PMSDRAW
#define PMS_SELECT_PMSDRAW_NUM (3) ///< 簡易会話の個数
#endif // PMS_SELECT_PMSDRAW 

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

	PMS_SELECT_BG_WORK				wk_bg;

	//描画設定
	PMS_SELECT_GRAPHIC_WORK	*graphic;

#ifdef PMS_SELECT_TOUCHBAR
	//タッチバー
	TOUCHBAR_WORK							*touchbar;
	//以下カスタムボタン使用する場合のサンプルリソース
	u32												ncg_btn;
	u32												ncl_btn;
	u32												nce_btn;
#endif //PMS_SELECT_TOUCHBAR

	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msg;

#ifdef PMS_SELECT_TASKMENU
	//タスクメニュー
	APP_TASKMENU_RES					*menu_res;
	APP_TASKMENU_WORK					*menu;
#endif //PMS_SELECT_TASKMENU

#ifdef	PMS_SELECT_PRINT_TOOL
	//プリントユーティリティ
	PRINT_UTIL								print_util;
	u32												seq;
#endif	//PMS_SELECT_PRINT_TOOL

#ifdef PMS_SELECT_PMSDRAW
  GFL_BMPWIN*               pms_win[ PMS_SELECT_PMSDRAW_NUM ];
  PMS_DRAW_WORK*            pms_draw;
#endif //PMS_SELECT_PMSDRAW

} PMS_SELECT_MAIN_WORK;


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
static GFL_PROC_RESULT PMSSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PMSSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT PMSSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void PMSSelect_BG_LoadResource( PMS_SELECT_BG_WORK* wk, HEAPID heapID );

#ifdef PMS_SELECT_TOUCHBAR
//-------------------------------------
///	タッチバー
//=====================================
static TOUCHBAR_WORK * PMSSelect_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID );
static void PMSSelect_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar );
static void PMSSelect_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );
//以下カスタムボタン使用サンプル用
static TOUCHBAR_WORK * PMSSelect_TOUCHBAR_InitEx( PMS_SELECT_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void PMSSelect_TOUCHBAR_ExitEx( PMS_SELECT_MAIN_WORK *wk );
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_TASKMENU
//-------------------------------------
///	リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * PMSSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID );
static void PMSSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void PMSSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu );
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_PMSDRAW
//-------------------------------------
///	簡易会話表示
//=====================================
static void PMSSelect_PMSDRAW_Init( PMS_SELECT_MAIN_WORK* wk );
static void PMSSelect_PMSDRAW_Exit( PMS_SELECT_MAIN_WORK* wk );
static void PMSSelect_PMSDRAW_Proc( PMS_SELECT_MAIN_WORK* wk );
#endif // PMS_SELECT_PMSDRAW


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA ProcData_PMSSelect = 
{
	PMSSelectProc_Init,
	PMSSelectProc_Main,
	PMSSelectProc_Exit,
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
 *	@param	*pwk					PMS_SELECT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PMSSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	PMS_SELECT_MAIN_WORK *wk;
	PMS_SELECT_PARAM *param;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//引数取得
	param	= pwk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UI_DEBUG, PMS_SELECT_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(PMS_SELECT_MAIN_WORK), HEAPID_UI_DEBUG );
  GFL_STD_MemClear( wk, sizeof(PMS_SELECT_MAIN_WORK) );

  // 初期化
  wk->heapID = HEAPID_UI_DEBUG;
	
	//描画設定初期化
	wk->graphic	= PMS_SELECT_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_pms_input_dat, wk->heapID );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );

	//BGリソース読み込み
	PMSSelect_BG_LoadResource( &wk->wk_bg, wk->heapID );

#ifdef PMS_SELECT_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= PMS_SELECT_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= PMSSelect_TOUCHBAR_InitEx( wk, clunit, wk->heapID );

    TOUCHBAR_SetActive( wk->touchbar, TOUCHBAR_ICON_CLOSE, FALSE );
	}
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_TASKMENU
	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_BAR_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
	wk->menu			= PMSSelect_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_PMSDRAW  
  PMSSelect_PMSDRAW_Init( wk );
#endif //PMS_SELECT_PMSDRAW

	//@todo	フェードシーケンスがないので
	GX_SetMasterBrightness(0);
	GXS_SetMasterBrightness(0);

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					PMS_SELECT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PMSSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	PMS_SELECT_MAIN_WORK* wk = mywk;

#ifdef PMS_SELECT_TASKMENU
	//TASKMENUシステム＆リソース破棄
	PMSSelect_TASKMENU_Exit( wk->menu );
	APP_TASKMENU_RES_Delete( wk->menu_res );	
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_TOUCHBAR
	//タッチバー
	PMSSelect_TOUCHBAR_ExitEx( wk );
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_PMSDRAW
  PMSSelect_PMSDRAW_Exit( wk );
#endif //PMS_SELECT_PMSDRAW

	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );

	//描画設定破棄
	PMS_SELECT_GRAPHIC_Exit( wk->graphic );

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
 *	@param	*pwk					PMS_SELECT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT PMSSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	PMS_SELECT_MAIN_WORK* wk = mywk;

  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG )
  {
    return GFL_PROC_RES_FINISH;
  }

#ifdef PMS_SELECT_TOUCHBAR
	//タッチバーメイン処理
	PMSSelect_TOUCHBAR_Main( wk->touchbar );
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_TASKMENU
	//タスクメニューメイン処理
	PMSSelect_TASKMENU_Main( wk->menu );
#endif //PMS_SELECT_TASKMENU

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );

#ifdef PMS_SELECT_PMSDRAW
  PMSSelect_PMSDRAW_Proc( wk );
#endif //PMS_SELECT_PMSDRAW

	//2D描画
	PMS_SELECT_GRAPHIC_2D_Draw( wk->graphic );

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
 *	@param	PMS_SELECT_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void PMSSelect_BG_LoadResource( PMS_SELECT_BG_WORK* wk, HEAPID heapID )
{
	ARCHANDLE	*handle;
	
  handle	= GFL_ARC_OpenDataHandle( ARCID_PMSI_GRAPHIC, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_pmsi_pms2_bg_main_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_pmsi_pms2_bg_main_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0, heapID );
	
  //	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_pmsi_pms2_bg_back_NCGR,
						BG_FRAME_BACK_S, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_pmsi_pms2_bg_back_NSCR,
						BG_FRAME_BACK_S, 0, 0, 0, heapID );

	//	----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_pmsi_pms2_bg_main_NCGR,
						BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_pmsi_pms2_bg_main1_NSCR,
						BG_FRAME_BACK_M, 0, 0, 0, heapID );

	GFL_ARC_CloseDataHandle( handle );
}

#ifdef PMS_SELECT_TOUCHBAR
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
static TOUCHBAR_WORK * PMSSelect_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID )
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
static void PMSSelect_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar )
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
static TOUCHBAR_WORK * PMSSelect_TOUCHBAR_InitEx( PMS_SELECT_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
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
static void PMSSelect_TOUCHBAR_ExitEx( PMS_SELECT_MAIN_WORK *wk )
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
static void PMSSelect_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_TASKMENU
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの初期化
 *
 *	@param	menu_res	リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * PMSSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID )
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
static void PMSSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
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
static void PMSSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_UpdateMenu( menu );
}
#endif //PMS_SELECT_TASKMENU

#ifdef	PMS_SELECT_PRINT_TOOL
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
static BOOL PrintTool_MainFunc( PMS_SELECT_MAIN_WORK * wk )
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
static void PrintTool_AddBmpWin( PMS_SELECT_MAIN_WORK * wk )
{
	wk->print_util.win = GFL_BMPWIN_Create(
													GFL_BG_FRAME0_M,					// ＢＧフレーム
													1, 1,											// 表示座標
													20, 2,										// 表示サイズ
													15,												// パレット
													GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
}

// BMPWIN削除
static void PrintTool_DelBmpWin( PMS_SELECT_MAIN_WORK * wk )
{
		GFL_BMPWIN_Delete( wk->print_util.win );
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( PMS_SELECT_MAIN_WORK * wk )
{
	GFL_BMPWIN_MakeScreen( wk->print_util.win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util.win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( PMS_SELECT_MAIN_WORK * wk )
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
#endif	//PMS_SELECT_PRINT_TOOL

#ifdef PMS_SELECT_PMSDRAW

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 初期化処理
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_PMSDRAW_Init( PMS_SELECT_MAIN_WORK* wk )
{
	GFL_CLUNIT	*clunit;
  
  clunit = PMS_SELECT_GRAPHIC_GetClunit( wk->graphic );

  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_SUB, wk->print_que, wk->font, 
      PLTID_OBJ_PMS_DRAW, PMS_SELECT_PMSDRAW_NUM ,wk->heapID );
  
  {
    int i;
    PMS_DATA pms;

    // PMS表示用BMPWIN生成
    for( i=0; i<PMS_SELECT_PMSDRAW_NUM; i++ )
    {
      wk->pms_win[i] = GFL_BMPWIN_Create(
          BG_FRAME_TEXT_S,					// ＢＧフレーム
          2+4*i, 0 + 6 * i,					  	// 表示座標(キャラ単位)
          28, 4,    							  // 表示サイズ
          15,												// パレット
          GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
    }
    
    // 1個目
    PMSDAT_SetDebug( &pms );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[0], &pms ,0 );

    // 2個目 デコメ表示
    PMSDAT_SetDeco( &pms, 0, PMS_DECOID_HERO );
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
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_PMSDRAW_Exit( PMS_SELECT_MAIN_WORK* wk )
{
  PMS_DRAW_Exit( wk->pms_draw );
  {
    int i;
    for( i=0; i<PMS_SELECT_PMSDRAW_NUM; i++ )
    {
      GFL_BMPWIN_Delete( wk->pms_win[i] );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 主処理
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_PMSDRAW_Proc( PMS_SELECT_MAIN_WORK* wk )
{
#if 1
  // SELECTでクリア
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    int i;
    for( i=0; i<PMS_SELECT_PMSDRAW_NUM; i++ )
    {
      PMS_DRAW_Clear( wk->pms_draw, i );
    }
  }
  // STARTでランダム挿入（二重登録するとアサート）
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    int i;
    for( i=0; i<PMS_SELECT_PMSDRAW_NUM; i++ )
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

#endif // PMS_SELECT_PMSDRAW

