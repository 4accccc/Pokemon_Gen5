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

#include "savedata/save_control.h"

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

#include "ui/ui_scene.h"

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

//SE
#include "sound/pm_sndsys.h"

#include "app/pms_input.h"

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

//--------------------------------------------------------------
///	SE定義
//==============================================================
enum
{ 
  SE_DECIDE = SEQ_SE_DECIDE1, ///< 決定
};

enum
{ 
  PMS_SELECT_HEAP_SIZE = 0x20000,  ///< ヒープサイズ
};

//--------------------------------------------------------------
///	シーケンス
//==============================================================

//--------------------------------------------------------------
///	SceneFunc
//==============================================================
// プレート選択
static BOOL ScenePlateSelect_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL ScenePlateSelect_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL ScenePlateSelect_End( UI_SCENE_CNT_PTR cnt, void* work );
// コマンド選択
static BOOL SceneCmdSelect_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneCmdSelect_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneCmdSelect_End( UI_SCENE_CNT_PTR cnt, void* work );
// コマンド > 決定
static BOOL SceneDecide_Main( UI_SCENE_CNT_PTR cnt, void* work );
// コマンド > 編集
static BOOL SceneEdit_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneEdit_End( UI_SCENE_CNT_PTR cnt, void* work );
// コマンド > キャンセル
static BOOL SceneCancel_Main( UI_SCENE_CNT_PTR cnt, void* work );

//--------------------------------------------------------------
///	SceneID
//==============================================================
typedef enum
{ 
  PMSS_SCENE_ID_PLATE_SELECT = 0, ///< プレート選択
  PMSS_SCENE_ID_CMD_SELECT,       ///< メニュー
  PMSS_SCENE_ID_DECIDE,           ///< けってい
  PMSS_SCENE_ID_EDIT,             ///< へんしゅう
  PMSS_SCENE_ID_CANCEL,           ///< やめる

  PMSS_SCENE_ID_MAX,
} PMSS_SCENE;

//--------------------------------------------------------------
///	SceneTable
//==============================================================
static const UI_SCENE_FUNC_SET c_scene_func_tbl[ PMSS_SCENE_ID_MAX ] = 
{ 
  // PMSS_SCENE_ID_PLATE_SELECT 
  {
    ScenePlateSelect_Init, 
    NULL,
    ScenePlateSelect_Main, 
    NULL, 
    ScenePlateSelect_End,
  },
  // PMSS_SCENE_ID_CMD_SELECT
  {
    SceneCmdSelect_Init,
    NULL,
    SceneCmdSelect_Main, 
    NULL,
    SceneCmdSelect_End,
  },
  // PMSS_SCENE_ID_DECIDE
  {
    NULL, NULL,
    SceneDecide_Main,
    NULL, NULL,
  },
  // PMSS_SCENE_ID_EDIT
  {
    NULL, NULL,
    SceneEdit_Main,
    NULL, 
    SceneEdit_End,
  },
  // PMSS_SCENE_ID_CANCEL
  {
    NULL, NULL,
    SceneCancel_Main,
    NULL, NULL,
  },
};

//--------------------------------------------------------------
///	タスクメニューID
//==============================================================
enum
{ 
  TASKMENU_ID_DECIDE,
  TASKMENU_ID_EDIT,
  TASKMENU_ID_CANCEL,
};

//-------------------------------------
///	フレーム
//=====================================
enum
{	
  // MAIN
	BG_FRAME_MENU_M	= GFL_BG_FRAME0_M,
	BG_FRAME_BAR_M	= GFL_BG_FRAME1_M,
	BG_FRAME_TEXT_M	= GFL_BG_FRAME2_M,
	BG_FRAME_BACK_M	= GFL_BG_FRAME3_M,
  // SUB
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
  PLTID_OBJ_PMS_DRAW    = 3, // 5本使用 
  PLTID_OBJ_BALLICON_M  = 13, // 1本使用
	PLTID_OBJ_TOWNMAP_M	  = 14,		
	//サブOBJ
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
#define PMS_SELECT_PMSDRAW_NUM (4) ///< 簡易会話の個数
#endif // PMS_SELECT_PMSDRAW 

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  //[IN]
  PMS_SELECT_PARAM* out_param;

  //[PRIVATE]
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

#ifdef PMS_SELECT_PMSDRAW
  GFL_BMPWIN*               pms_win[ PMS_SELECT_PMSDRAW_NUM ];
  PMS_DRAW_WORK*            pms_draw;
#endif //PMS_SELECT_PMSDRAW

  UI_SCENE_CNT_PTR          cntScene;
  void*                     subproc_work;   ///< サブPROC用ワーク保存領域

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

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_GRAPHIC_Load( PMS_SELECT_MAIN_WORK* wk )
{ 
	//描画設定初期化
	wk->graphic	= PMS_SELECT_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

	//BGリソース読み込み
	PMSSelect_BG_LoadResource( &wk->wk_bg, wk->heapID );

#ifdef PMS_SELECT_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= PMS_SELECT_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= PMSSelect_TOUCHBAR_InitEx( wk, clunit, wk->heapID );
	}
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_TASKMENU
	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_MENU_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_PMSDRAW  
  PMSSelect_PMSDRAW_Init( wk );
#endif //PMS_SELECT_PMSDRAW
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	PMS_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PMSSelect_GRAPHIC_UnLoad( PMS_SELECT_MAIN_WORK* wk )
{
#ifdef PMS_SELECT_TASKMENU
	//TASKMENUシステム＆リソース破棄
	APP_TASKMENU_RES_Delete( wk->menu_res );	
#endif //PMS_SELECT_TASKMENU

#ifdef PMS_SELECT_TOUCHBAR
	//タッチバー
	PMSSelect_TOUCHBAR_ExitEx( wk );
#endif //PMS_SELECT_TOUCHBAR

#ifdef PMS_SELECT_PMSDRAW
  PMSSelect_PMSDRAW_Exit( wk );
#endif //PMS_SELECT_PMSDRAW

	//描画設定破棄
	PMS_SELECT_GRAPHIC_Exit( wk->graphic );
}

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
	
	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pms_input_dat, wk->heapID );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );

  PMSSelect_GRAPHIC_Load( wk );

  // SCENE
	wk->cntScene = UI_SCENE_CNT_Create( wk->heapID, c_scene_func_tbl, PMSS_SCENE_ID_MAX, wk );

	//@TODO	フェードシーケンスがないので
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

  PMSSelect_GRAPHIC_UnLoad( wk );

  // SCENE
  UI_SCENE_CNT_Delete( wk->cntScene );

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
	
  // SCENE
  if( UI_SCENE_CNT_Main( wk->cntScene ) )
  {
    return GFL_PROC_RES_FINISH;
  }

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
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_pmsi_pms2_bg_back_NCGR,
						BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_pmsi_pms2_bg_back01_NSCR,
						BG_FRAME_BACK_M, 0, 0, 0, heapID );
	
  GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_pmsi_pms2_bg_back_NCGR,
						BG_FRAME_TEXT_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_pmsi_pms2_bg_back01_NSCR,
						BG_FRAME_TEXT_M, 0, 0, 0, heapID );

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
	TOUCHBAR_SETUP	touchbar_setup = {0};

	TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
	{	
		{	
			TOUCHBAR_ICON_RETURN,
			{	TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
		},
		{	
			TOUCHBAR_ICON_CUTSOM1,	//カスタムボタン１を拡大縮小アイコンに,
			{	TOUCHBAR_ICON_X_04, TOUCHBAR_ICON_Y },
		},
	};

	//以下カスタムボタンならば入れなくてはいけない情報
	touchbar_icon_tbl[1].cg_idx	  = wk->ncg_btn;		//キャラリソース
	touchbar_icon_tbl[1].plt_idx	= wk->ncl_btn;		//パレットリソース
	touchbar_icon_tbl[1].cell_idx	=	wk->nce_btn;		//セルリソース
	touchbar_icon_tbl[1].active_anmseq	  =	8;			//アクティブのときのアニメ
	touchbar_icon_tbl[1].noactive_anmseq	=	12;			//ノンアクティブのときのアニメ
	touchbar_icon_tbl[1].push_anmseq	    =	10;			//押したときのアニメ（STOPになっていること）
	touchbar_icon_tbl[1].key	=	PAD_BUTTON_START;		//キーで押したときに動作させたいならば、ボタン番号
	touchbar_icon_tbl[1].se	  = 0;									//押したときにSEならしたいならば、SEの番号

	//設定構造体
	//さきほどの窓情報＋リソース情報をいれる
	touchbar_setup.p_item		= touchbar_icon_tbl;				//上の窓情報
	touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
	touchbar_setup.p_unit		= clunit;										//OBJ読み込みのためのCLUNIT
	touchbar_setup.bar_frm	= BG_FRAME_BAR_M;						//BG読み込みのためのBG面
	touchbar_setup.bg_plt		= PLTID_BG_TOUCHBAR_M;			//BGﾊﾟﾚｯﾄ
	touchbar_setup.obj_plt	= PLTID_OBJ_TOUCHBAR_M;			//OBJﾊﾟﾚｯﾄ
	touchbar_setup.mapping	= APP_COMMON_MAPPING_64K;	//マッピングモード

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

  static u8 strtbl[] = {
    str_decide,
    str_edit,
    str_cancel,
  };

	//窓の設定
	for( i = 0; i < NELEMS(item); i++ )
	{	
		item[i].str	      = GFL_MSG_CreateString( msg, strtbl[i] );	//文字列
		item[i].msgColor	= APP_TASKMENU_ITEM_MSGCOLOR;	//文字色
		item[i].type			= APP_TASKMENU_WIN_TYPE_NORMAL;	//窓の種類
	}
	
  // 「もどる」記号
  item[2].type  = APP_TASKMENU_WIN_TYPE_RETURN,	//もどる記号が入った窓（←┘）←こんなの

	//初期化
	init.heapId		= heapID;
	init.itemNum	= NELEMS(item);
	init.itemWork = item;
	init.posType	= ATPT_RIGHT_DOWN;
	init.charPosX	= 32;
	init.charPosY = 24;
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

  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_MAIN, wk->print_que, wk->font, 
      PLTID_OBJ_PMS_DRAW, PMS_SELECT_PMSDRAW_NUM ,wk->heapID );
  
  {
    int i;
    PMS_DATA pms;

    // PMS表示用BMPWIN生成
    for( i=0; i<PMS_SELECT_PMSDRAW_NUM; i++ )
    {
      wk->pms_win[i] = GFL_BMPWIN_Create(
          BG_FRAME_TEXT_M,					// ＢＧフレーム
          2, 1 + 6 * i,			      	// 表示座標(キャラ単位)
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
    
    // 4個目 デコメ二個表示
    PMSDAT_SetDebugRandomDeco( &pms, wk->heapID );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[3], &pms ,3 );
    
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

//=============================================================================
// SCENE
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート選択 初期化
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL ScenePlateSelect_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMS_SELECT_MAIN_WORK* wk = work;

  // 全て表示
  TOUCHBAR_SetVisibleAll( wk->touchbar, TRUE );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート選択主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL ScenePlateSelect_Main( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  PMS_SELECT_MAIN_WORK* wk = work;

  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG )
  {
    UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
  }

	//タッチバーメイン処理
	PMSSelect_TOUCHBAR_Main( wk->touchbar );

  // タッチバー入力
  switch( TOUCHBAR_GetTrg( wk->touchbar ) )
  {
  case TOUCHBAR_ICON_RETURN :
    UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
    return TRUE;

  // @TODO 左右と項目アイコン

  case TOUCHBAR_ICON_CUTSOM1 :
  case TOUCHBAR_SELECT_NONE :
    break;

  default : GF_ASSERT(0);
  }
    
  // @TODO とりあえずXでメニューを開く
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
  {
    UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_CMD_SELECT );
    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレート選択 後処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL ScenePlateSelect_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMS_SELECT_MAIN_WORK* wk = work;
  
  // 全て消す
  TOUCHBAR_SetVisibleAll( wk->touchbar, FALSE );

  return TRUE;
}
  
//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneCmdSelect_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMS_SELECT_MAIN_WORK* wk = work;

  wk->menu = PMSSelect_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  プレートに対するコマンド選択
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneCmdSelect_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMS_SELECT_MAIN_WORK* wk = work;
	
  //タスクメニューメイン処理
	PMSSelect_TASKMENU_Main( wk->menu );

  if( APP_TASKMENU_IsFinish( wk->menu ) )
  {
    u8 pos = APP_TASKMENU_GetCursorPos( wk->menu );

    switch( pos )
    {
    case TASKMENU_ID_DECIDE :
      UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_DECIDE );
      break;
    case TASKMENU_ID_EDIT :
      UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_EDIT );
      break;
    case TASKMENU_ID_CANCEL :
      UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_CANCEL );
      break;
    default : GF_ASSERT(0);
    }

    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneCmdSelect_End( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  PMS_SELECT_MAIN_WORK* wk = work;

	PMSSelect_TASKMENU_Exit( wk->menu );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  けってい→終了
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneDecide_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  へんしゅう→簡易会話入力
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneEdit_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  PMSI_PARAM* pmsi;
  PMS_SELECT_MAIN_WORK* wk = work;

  PMSSelect_GRAPHIC_UnLoad( wk );

  //@TODO セーブポインタグローバル参照
  pmsi = PMSI_PARAM_Create(PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT, SaveControl_GetPointer(), wk->heapID );

  // PROC切替 入力画面呼び出し
  GFL_PROC_SysCallProc( NO_OVERLAY_ID, &ProcData_PMSInput, pmsi );
  wk->subproc_work = pmsi;
 
  HOSAKA_Printf("call! \n");

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneEdit_End( UI_SCENE_CNT_PTR cnt, void* work )
{ 
  PMS_SELECT_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );

  switch( seq )
  {
  case 0:
    HOSAKA_Printf("return! \n");
    PMSI_PARAM_Delete( wk->subproc_work );
    PMSSelect_GRAPHIC_Load( wk );

    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0);
    UI_SCENE_CNT_IncSubSeq( cnt );
    break;

  case 1:
    // フェード待ち
    if( GFL_FADE_CheckFade() == FALSE )
    {
      // 次のシーンをセット
      UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_PLATE_SELECT );
      return TRUE;
    }
    break;

  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キャンセル→選択画面に戻る
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneCancel_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  UI_SCENE_CNT_SetNextScene( cnt, PMSS_SCENE_ID_PLATE_SELECT );

  return TRUE;
}

