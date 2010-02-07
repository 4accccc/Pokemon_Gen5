//=============================================================================
/**
 *
 *	@file		ui_template.c
 *	@brief  新規アプリ用テンプレート
 *	@author	Toru=Nagihashi / genya hosaka
 *	@data		2009.09.29
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
#include "ui_template_graphic.h"

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

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "ui_template.h"


//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
#include "mictest.naix"	// アーカイブ
#include "msg/msg_mictest.h"  // GMM
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define UI_TEMPLATE_INFOWIN
#define UI_TEMPLATE_TOUCHBAR
#define UI_TEMPLATE_MCSS
#define UI_TEMPLATE_TASKMENU
#define UI_TEMPLATE_TYPEICON      // 分類、技アイコン
#define UI_TEMPLATE_ITEM_ICON     // どうぐアイコン
#define UI_TEMPLATE_POKE_ICON     // ポケアイコン(ポケアイコン用アイテムアイコン込み)
#define UI_TEMPLATE_OAM_MAPMODEL  // マップモデルをOAMで表示
#define UI_TEMPLATE_POKE2D				// ポケモンBG,OBJ読みこみ
#define UI_TEMPLATE_BALL					// ボールアイコン読みこみ
#define UI_TEMPLATE_PMSDRAW       // 簡易会話表示

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  UI_TEMPLATE_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
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
} UI_TEMPLATE_BG_WORK;


#ifdef UI_TEMPLATE_PMSDRAW
#define UI_TEMPLATE_PMSDRAW_NUM (3) ///< 簡易会話の個数
#endif // UI_TEMPLATE_PMSDRAW 

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

	UI_TEMPLATE_BG_WORK				wk_bg;

	//描画設定
	UI_TEMPLATE_GRAPHIC_WORK	*graphic;

#ifdef UI_TEMPLATE_TOUCHBAR
	//タッチバー
	TOUCHBAR_WORK							*touchbar;
	//以下カスタムボタン使用する場合のサンプルリソース
	u32												ncg_btn;
	u32												ncl_btn;
	u32												nce_btn;
#endif //UI_TEMPLATE_TOUCHBAR

#ifdef UI_TEMPLATE_MCSS
	//MCSS
	MCSS_SYS_WORK							*mcss_sys;
	MCSS_WORK									*mcss_wk;
#endif //UI_TEMPLATE_MCSS

	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msg;

#ifdef UI_TEMPLATE_TASKMENU
	//タスクメニュー
	APP_TASKMENU_RES					*menu_res;
	APP_TASKMENU_WORK					*menu;
#endif //UI_TEMPLATE_TASKMENU

#ifdef UI_TEMPLATE_TYPEICON
	//分類、技アイコン
  UI_EASY_CLWK_RES      clres_type_icon;
  GFL_CLWK                  *clwk_type_icon;
#endif //UI_TEMPLATE_TYPEICON

#ifdef UI_TEMPLATE_ITEM_ICON
  // どうぐアイコン
  UI_EASY_CLWK_RES      clres_item_icon;
  GFL_CLWK                  *clwk_item_icon;
#endif //UI_TEMPLATE_ITEM_ICON

#ifdef UI_TEMPLATE_POKE_ICON
  // ポケアイコン
  UI_EASY_CLWK_RES      clres_poke_icon;
  GFL_CLWK                  *clwk_poke_icon;
  // ポケアイコン用アイテムアイコン
  UI_EASY_CLWK_RES      clres_poke_item;
  GFL_CLWK                  *clwk_poke_item;
#endif //UI_TEMPLATE_POKE_ICON

#ifdef UI_TEMPLATE_OAM_MAPMODEL
  // OAMマップモデル
  UI_EASY_CLWK_RES      clres_oam_mmdl;
  GFL_CLWK                  *clwk_oam_mmdl;
#endif //UI_TEMPLATE_OAM_MAPMODEL

#ifdef UI_TEMPLATE_POKE2D
	u32												ncg_poke2d;
	u32												ncl_poke2d;
	u32												nce_poke2d;
	GFL_CLWK									*clwk_poke2d;
#endif //UI_TEMPLATE_POKE2D

#ifdef UI_TEMPLATE_BALL
  // ボールアイコン
  UI_EASY_CLWK_RES      clres_ball;
  GFL_CLWK                  *clwk_ball;
#endif //UI_TEMPLATE_BALL

#ifdef	UI_TEMPLATE_PRINT_TOOL
	//プリントユーティリティ
	PRINT_UTIL								print_util;
	u32												seq;
#endif	//UI_TEMPLATE_PRINT_TOOL

#ifdef UI_TEMPLATE_PMSDRAW
  GFL_BMPWIN*               pms_win[ UI_TEMPLATE_PMSDRAW_NUM ];
  PMS_DRAW_WORK*            pms_draw;
#endif //UI_TEMPLATE_PMSDRAW

} UI_TEMPLATE_MAIN_WORK;


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
static GFL_PROC_RESULT UITemplateProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT UITemplateProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT UITemplateProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void UITemplate_BG_LoadResource( UI_TEMPLATE_BG_WORK* wk, HEAPID heapID );

#ifdef UI_TEMPLATE_INFOWIN
//-------------------------------------
///	INFOWIN
//=====================================
static void UITemplate_INFOWIN_Init( GAMESYS_WORK *gamesys, HEAPID heapID );
static void UITemplate_INFOWIN_Exit( void );
static void UITemplate_INFOWIN_Main( void );
#endif //UI_TEMPLATE_INFOWIN

#ifdef UI_TEMPLATE_TOUCHBAR
//-------------------------------------
///	タッチバー
//=====================================
static TOUCHBAR_WORK * UITemplate_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID );
static void UITemplate_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar );
static void UITemplate_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );
//以下カスタムボタン使用サンプル用
static TOUCHBAR_WORK * UITemplate_TOUCHBAR_InitEx( UI_TEMPLATE_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void UITemplate_TOUCHBAR_ExitEx( UI_TEMPLATE_MAIN_WORK *wk );
#endif //UI_TEMPLATE_TOUCHBAR

#ifdef UI_TEMPLATE_MCSS
//-------------------------------------
///	MCSS
//=====================================
static MCSS_SYS_WORK * UITemplate_MCSS_Init( u16 wk_max, HEAPID heapID );
static void UITemplate_MCSS_Exit( MCSS_SYS_WORK * mcss );
static void UITemplate_MCSS_Draw( MCSS_SYS_WORK * mcss );
static MCSS_WORK * UITemplate_MCSS_CreateWkPP( MCSS_SYS_WORK * mcss, POKEMON_PARAM *pp, const VecFx32 *pos );
static void UITemplate_MCSS_DeleteWk( MCSS_SYS_WORK * mcss, MCSS_WORK *wk );
#endif //UI_TEMPLATE_MCSS

#ifdef UI_TEMPLATE_TASKMENU
//-------------------------------------
///	リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * UITemplate_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID );
static void UITemplate_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void UITemplate_TASKMENU_Main( APP_TASKMENU_WORK *menu );
#endif //UI_TEMPLATE_TASKMENU

#ifdef UI_TEMPLATE_TYPEICON
//-------------------------------------
///	分類、技アイコン
//=====================================
static void UITemplate_TYPEICON_CreateCLWK( UI_TEMPLATE_MAIN_WORK *wk, PokeType type, GFL_CLUNIT *unit, HEAPID heapID );
static void UITemplate_TYPEICON_DeleteCLWK( UI_TEMPLATE_MAIN_WORK *wk );
#endif //UI_TEMPLATE_TYPEICON

#ifdef UI_TEMPLATE_ITEM_ICON
//-------------------------------------
///	どうぐアイコン
//=====================================
static void UITemplate_ITEM_ICON_CreateCLWK( UI_TEMPLATE_MAIN_WORK* wk, u16 item_id, GFL_CLUNIT* unit, HEAPID heapID );
static void UITemplate_ITEM_ICON_DeleteCLWK( UI_TEMPLATE_MAIN_WORK* wk );
#endif //UI_TEMPLATE_ITEM_ICON

#ifdef UI_TEMPLATE_POKE_ICON
//-------------------------------------
///	ポケアイコン
//=====================================
static void UITemplate_POKE_ICON_CreateCLWK( UI_TEMPLATE_MAIN_WORK* wk, u32 mons, u32 form_no, BOOL egg, GFL_CLUNIT* unit, HEAPID heapID );
static void UITemplate_POKE_ICON_DeleteCLWK( UI_TEMPLATE_MAIN_WORK* wk );
#endif //UI_TEMPLATE_POKE_ICON

#ifdef UI_TEMPLATE_OAM_MAPMODEL
//-------------------------------------
///	OAMでマップモデル表示
//=====================================
static void UITemplate_OAM_MAPMODEL_CreateCLWK( UI_TEMPLATE_MAIN_WORK *wk, u16 tex_idx, u8 ptn_ofs, GFL_CLUNIT *unit, HEAPID heapID );
static void UITemplate_OAM_MAPMODEL_DeleteCLWK( UI_TEMPLATE_MAIN_WORK* wk );
#endif //UI_TEMPLATE_OAM_MAPMODEL

#ifdef UI_TEMPLATE_POKE2D
//-------------------------------------
///	ポケモンOBJ,BG読みこみ
//=====================================
static void UITemplate_POKE2D_LoadResourceBG( UI_TEMPLATE_MAIN_WORK *wk, HEAPID heapID );
static void UITemplate_POKE2D_UnLoadResourceBG( UI_TEMPLATE_MAIN_WORK *wk );
static void UITemplate_POKE2D_LoadResourceOBJ( UI_TEMPLATE_MAIN_WORK *wk, HEAPID heapID );
static void UITemplate_POKE2D_UnLoadResourceOBJ( UI_TEMPLATE_MAIN_WORK *wk );
static void UITemplate_POKE2D_CreateCLWK( UI_TEMPLATE_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void UITemplate_POKE2D_DeleteCLWK( UI_TEMPLATE_MAIN_WORK *wk );
#endif //UI_TEMPLATE_POKE2D

#ifdef UI_TEMPLATE_BALL
//-------------------------------------
///	ボールアイコン
//=====================================
static void UITemplate_BALLICON_CreateCLWK( UI_TEMPLATE_MAIN_WORK *wk, BALL_ID ballID, GFL_CLUNIT *unit, HEAPID heapID );
static void UITemplate_BALLICON_DeleteCLWK( UI_TEMPLATE_MAIN_WORK* wk );
#endif //UI_TEMPLATE_BALL


#ifdef UI_TEMPLATE_PMSDRAW
//-------------------------------------
///	簡易会話表示
//=====================================
static void UITemplate_PMSDRAW_Init( UI_TEMPLATE_MAIN_WORK* wk );
static void UITemplate_PMSDRAW_Exit( UI_TEMPLATE_MAIN_WORK* wk );
static void UITemplate_PMSDRAW_Proc( UI_TEMPLATE_MAIN_WORK* wk );
#endif // UI_TEMPLATE_PMSDRAW


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA UITemplateProcData = 
{
	UITemplateProc_Init,
	UITemplateProc_Main,
	UITemplateProc_Exit,
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
 *	@param	*pwk					UI_TEMPLATE_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UITemplateProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	UI_TEMPLATE_MAIN_WORK *wk;
	UI_TEMPLATE_PARAM *param;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//引数取得
	param	= pwk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UI_DEBUG, UI_TEMPLATE_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(UI_TEMPLATE_MAIN_WORK), HEAPID_UI_DEBUG );
  GFL_STD_MemClear( wk, sizeof(UI_TEMPLATE_MAIN_WORK) );

  // 初期化
  wk->heapID = HEAPID_UI_DEBUG;
	
	//描画設定初期化
	wk->graphic	= UI_TEMPLATE_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_mictest_dat, wk->heapID );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );

	//BGリソース読み込み
	UITemplate_BG_LoadResource( &wk->wk_bg, wk->heapID );

#ifdef UI_TEMPLATE_INFOWIN
	//INFOWINの初期化
	UITemplate_INFOWIN_Init( param->gamesys, wk->heapID );
#endif //UI_TEMPLATE_INFOWIN

#ifdef UI_TEMPLATE_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= UI_TEMPLATE_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= UITemplate_TOUCHBAR_InitEx( wk, clunit, wk->heapID );
	}
#endif //UI_TEMPLATE_TOUCHBAR

#ifdef UI_TEMPLATE_MCSS
	//MCSS初期化&ワーク作成
	wk->mcss_sys	= UITemplate_MCSS_Init( 1, wk->heapID );
	{
		POKEMON_PARAM	*pp;  
		VecFx32 pos	= { 0, -FX32_ONE*10, 0	};

		pp	= PP_Create( MONSNO_HUSIGIBANA, 0, 0, wk->heapID );
		wk->mcss_wk		= UITemplate_MCSS_CreateWkPP( wk->mcss_sys, pp, &pos );
		GFL_HEAP_FreeMemory( pp );
	}
#endif //UI_TEMPLATE_MCSS

#ifdef UI_TEMPLATE_TASKMENU
	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_BAR_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
	wk->menu			= UITemplate_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );
#endif //UI_TEMPLATE_TASKMENU

#ifdef UI_TEMPLATE_TYPEICON
	//属性アイコンの読み込み
	{	
		GFL_CLUNIT	*clunit	= UI_TEMPLATE_GRAPHIC_GetClunit( wk->graphic );
		UITemplate_TYPEICON_CreateCLWK( wk, POKETYPE_KUSA, clunit, wk->heapID );
	}
#endif //UI_TEMPLATE_TYPEICON

#ifdef UI_TEMPLATE_OAM_MAPMODEL
  //OAMマップモデルの読み込み
  {
    // 主人公は現状 0=後ろ, 1=後ろアニメ1, 2=後ろアニメ2, 3=正面....
    // トレーナーなどは違う可能性があるので注意。
    u16 ptn_ofs = 3;
		GFL_CLUNIT	*clunit	= UI_TEMPLATE_GRAPHIC_GetClunit( wk->graphic );

		UITemplate_OAM_MAPMODEL_CreateCLWK( wk, NARC_fldmmdl_mdlres_hero_nsbtx, ptn_ofs, clunit, wk->heapID );
  }
#endif //UI_TEMPLATE_OAM_MAPMODEL

#ifdef UI_TEMPLATE_POKE_ICON
  // ポケアイコンの読み込み
  {
		GFL_CLUNIT	*clunit	= UI_TEMPLATE_GRAPHIC_GetClunit( wk->graphic );
    UITemplate_POKE_ICON_CreateCLWK( wk, MONSNO_HUSIGIDANE, 0, FALSE, clunit, wk->heapID );
  }
#endif //UI_TEMPLATE_POKE_ICON

#ifdef UI_TEMPLATE_ITEM_ICON
  // どうぐアイコンの読み込み
  { 
		GFL_CLUNIT	*clunit	= UI_TEMPLATE_GRAPHIC_GetClunit( wk->graphic );
    UITemplate_ITEM_ICON_CreateCLWK( wk, ITEM_KIZUGUSURI, clunit, wk->heapID );
  }
#endif //UI_TEMPLATE_ITEM_ICON

#ifdef UI_TEMPLATE_POKE2D
	//ポケモンOBJ,BG読みこみ
	UITemplate_POKE2D_LoadResourceBG( wk, wk->heapID );
	{	
		GFL_CLUNIT *clunit = UI_TEMPLATE_GRAPHIC_GetClunit( wk->graphic );
		UITemplate_POKE2D_LoadResourceOBJ( wk, wk->heapID );
		UITemplate_POKE2D_CreateCLWK( wk, clunit, wk->heapID );
	}
#endif //UI_TEMPLATE_POKE2D

#ifdef UI_TEMPLATE_BALL
  // ボールアイコンの読み込み
  { 
		GFL_CLUNIT	*clunit	= UI_TEMPLATE_GRAPHIC_GetClunit( wk->graphic );
    UITemplate_BALLICON_CreateCLWK( wk, BALLID_MONSUTAABOORU, clunit, wk->heapID );
  }
#endif //UI_TEMPLATE_BALL

#ifdef UI_TEMPLATE_PMSDRAW  
  UITemplate_PMSDRAW_Init( wk );
#endif //UI_TEMPLATE_PMSDRAW

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
 *	@param	*pwk					UI_TEMPLATE_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UITemplateProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	UI_TEMPLATE_MAIN_WORK* wk = mywk;

#ifdef UI_TEMPLATE_BALL
  // どうぐアイコンの破棄
   UITemplate_BALLICON_DeleteCLWK( wk );
#endif //UI_TEMPLATE_BALL

#ifdef UI_TEMPLATE_POKE2D
	UITemplate_POKE2D_DeleteCLWK( wk );
	UITemplate_POKE2D_UnLoadResourceOBJ( wk );
	UITemplate_POKE2D_UnLoadResourceBG( wk );
#endif //UI_TEMPLATE_POKE2D

#ifdef UI_TEMPLATE_TYPEICON
	//属性アイコンの破棄
	UITemplate_TYPEICON_DeleteCLWK( wk );
#endif //UI_TEMPLATE_TYPEICON

#ifdef UI_TEMPLATE_ITEM_ICON
#endif //UI_TEMPLATE_ITEM_ICON

#ifdef UI_TEMPLATE_ITEM_ICON
  //どうぐアイコンの破棄
  UITemplate_ITEM_ICON_DeleteCLWK( wk );
#endif //UI_TEMPLATE_ITEM_ICON

#ifdef UI_TEMPLATE_OAM_MAPMODEL
  //OAMマップモデルの破棄
  UITemplate_OAM_MAPMODEL_DeleteCLWK( wk );
#endif //UI_TEMPLATE_OAM_MAPMODEL

#ifdef UI_TEMPLATE_POKE_ICON
   UITemplate_POKE_ICON_DeleteCLWK( wk );
#endif //UI_TEMPLATE_POKE_ICON

#ifdef UI_TEMPLATE_TASKMENU
	//TASKMENUシステム＆リソース破棄
	UITemplate_TASKMENU_Exit( wk->menu );
	APP_TASKMENU_RES_Delete( wk->menu_res );	
#endif //UI_TEMPLATE_TASKMENU

#ifdef UI_TEMPLATE_MCSS
	//MCSS破棄
	UITemplate_MCSS_DeleteWk( wk->mcss_sys, wk->mcss_wk );
	UITemplate_MCSS_Exit( wk->mcss_sys );
#endif //UI_TEMPLATE_MCSS

#ifdef UI_TEMPLATE_TOUCHBAR
	//タッチバー
	UITemplate_TOUCHBAR_ExitEx( wk );
#endif //UI_TEMPLATE_TOUCHBAR

#ifdef UI_TEMPLATE_INFOWIN
	//INFWIN
	UITemplate_INFOWIN_Exit();
#endif //UI_TEMPLATE_INFOWIN

#ifdef UI_TEMPLATE_PMSDRAW
  UITemplate_PMSDRAW_Exit( wk );
#endif //UI_TEMPLATE_PMSDRAW

	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );

	//描画設定破棄
	UI_TEMPLATE_GRAPHIC_Exit( wk->graphic );

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
 *	@param	*pwk					UI_TEMPLATE_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UITemplateProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	UI_TEMPLATE_MAIN_WORK* wk = mywk;

  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG )
  {
    return GFL_PROC_RES_FINISH;
  }

#ifdef UI_TEMPLATE_TOUCHBAR
	//タッチバーメイン処理
	UITemplate_TOUCHBAR_Main( wk->touchbar );
#endif //UI_TEMPLATE_TOUCHBAR

#ifdef UI_TEMPLATE_INFOWIN
	//INFWINメイン処理
	UITemplate_INFOWIN_Main();
#endif //UI_TEMPLATE_INFOWIN

#ifdef UI_TEMPLATE_TASKMENU
	//タスクメニューメイン処理
	UITemplate_TASKMENU_Main( wk->menu );
#endif //UI_TEMPLATE_TASKMENU

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );

#ifdef UI_TEMPLATE_PMSDRAW
  UITemplate_PMSDRAW_Proc( wk );
#endif //UI_TEMPLATE_PMSDRAW

	//2D描画
	UI_TEMPLATE_GRAPHIC_2D_Draw( wk->graphic );

	//3D描画
	UI_TEMPLATE_GRAPHIC_3D_StartDraw( wk->graphic );

#ifdef UI_TEMPLATE_MCSS
	UITemplate_MCSS_Draw( wk->mcss_sys );
#endif //UI_TEMPLATE_MCSS
	UI_TEMPLATE_GRAPHIC_3D_EndDraw( wk->graphic );

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
 *	@param	UI_TEMPLATE_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UITemplate_BG_LoadResource( UI_TEMPLATE_BG_WORK* wk, HEAPID heapID )
{
  //@TODO とりあえずマイクテストのリソース
  enum
  {
    RES_ARC = ARCID_MICTEST_GRA,

    RES_PAL_S_SIZE = 0x20,
    RES_PAL_M_SIZE = 0x20,

    RES_PAL_S_ID = NARC_mictest_back_bg_up_NCLR,
    RES_CHR_S_ID = NARC_mictest_back_bg_up_NCGR,
    RES_SCR_S_ID = NARC_mictest_back_bg_up_NSCR,

    RES_PAL_M_ID = NARC_mictest_back_bg_down_NCLR,
    RES_CHR_M_ID = NARC_mictest_back_bg_down_NCGR,
    RES_SCR_M_ID = NARC_mictest_back_bg_down_NSCR,

  };

	ARCHANDLE	*handle;

	handle	= GFL_ARC_OpenDataHandle( RES_ARC, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_M_ID, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, RES_PAL_M_SIZE, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_S_ID, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_S_SIZE, heapID );
	
  //	----- SUB -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_S_ID, BG_FRAME_BACK_S, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_S_ID,	BG_FRAME_BACK_S, 0, 0, 0, heapID );	

	//	----- MAIN -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_M_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_M_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );		

	GFL_ARC_CloseDataHandle( handle );
}





#ifdef UI_TEMPLATE_INFOWIN
//=============================================================================
/**
 *		INFOWIN
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	INFOWIN初期化
 *
 *	@param	GAMESYS_WORK *gamesys	ゲームシステム
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void UITemplate_INFOWIN_Init( GAMESYS_WORK *gamesys, HEAPID heapID )
{	
	GAME_COMM_SYS_PTR comm;

  GF_ASSERT( gamesys );

	comm	= GAMESYSTEM_GetGameCommSysPtr(gamesys);
	INFOWIN_Init( BG_FRAME_BAR_M, PLTID_BG_INFOWIN_M, comm, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	INFOWIN破棄
 *
 *	@param	void 
 */
//-----------------------------------------------------------------------------
static void UITemplate_INFOWIN_Exit( void )
{	
	INFOWIN_Exit();
}
//----------------------------------------------------------------------------
/**
 *	@brief	INFWINメイン処理
 *
 *	@param	void 
 */
//-----------------------------------------------------------------------------
static void UITemplate_INFOWIN_Main( void )
{	
	INFOWIN_Update();
}
#endif //UI_TEMPLATE_INFOWIN

#ifdef UI_TEMPLATE_TOUCHBAR
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
static TOUCHBAR_WORK * UITemplate_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID )
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
static void UITemplate_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar )
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
static TOUCHBAR_WORK * UITemplate_TOUCHBAR_InitEx( UI_TEMPLATE_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
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
static void UITemplate_TOUCHBAR_ExitEx( UI_TEMPLATE_MAIN_WORK *wk )
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
static void UITemplate_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}
#endif //UI_TEMPLATE_TOUCHBAR

#ifdef UI_TEMPLATE_MCSS
//=============================================================================
/**
 *		MCSS
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	MCSS初期化
 *
 *	@param	u16 wk_max		MCSSのワーク作成最大数
 *	@param	heapID				ヒープID
 *
 *	@return	MCSS_SYS
 */
//-----------------------------------------------------------------------------
static MCSS_SYS_WORK * UITemplate_MCSS_Init( u16 wk_max, HEAPID heapID )
{	
	MCSS_SYS_WORK *mcss;
	mcss = MCSS_Init( wk_max , heapID );
	MCSS_SetTextureTransAdrs( mcss, 0 );
	MCSS_SetOrthoMode( mcss );
	return mcss;
}
//----------------------------------------------------------------------------
/**
 *	@brief	MCSS破棄
 *
 *	@param	MCSS_SYS_WORK * mcss	MCSS＿SYS
 */
//-----------------------------------------------------------------------------
static void UITemplate_MCSS_Exit( MCSS_SYS_WORK * mcss )
{	
	MCSS_Exit(mcss);
}
//----------------------------------------------------------------------------
/**
 *	@brief	MCSS描画
 *
 *	@param	MCSS_SYS_WORK * mcss MCSS＿SYS
 */
//-----------------------------------------------------------------------------
static void UITemplate_MCSS_Draw( MCSS_SYS_WORK * mcss )
{	
	MCSS_Main( mcss );
	MCSS_Draw( mcss );
}
//----------------------------------------------------------------------------
/**
 *	@brief	MCSSワーク作成
 *
 *	@param	MCSS_SYS_WORK * mcss	MCSS_SYS
 *	@param	*ppp					ポケモンパーソナルパラメータ
 *	@param	VecFx32 *pos	位置
 *
 *	@return	MCSS_WK
 */
//-----------------------------------------------------------------------------
static MCSS_WORK * UITemplate_MCSS_CreateWkPP( MCSS_SYS_WORK * mcss, POKEMON_PARAM *pp, const VecFx32 *pos )
{	
	MCSS_WORK *poke;
	MCSS_ADD_WORK addWork;

	VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};

	MCSS_TOOL_MakeMAWPP( pp , &addWork , MCSS_DIR_FRONT );
	poke = MCSS_Add( mcss, pos->x, pos->y , pos->z, &addWork );
	MCSS_SetScale( poke , &scale );
	return poke;
}
//----------------------------------------------------------------------------
/**
 *	@brief	MCSSワーク破棄
 *
 *	@param	MCSS_SYS_WORK * mcss	MCSS_SYS
 *	@param	*wk										ワーク
 *
 */
//-----------------------------------------------------------------------------
static void UITemplate_MCSS_DeleteWk( MCSS_SYS_WORK * mcss, MCSS_WORK *wk )
{	
	MCSS_SetVanishFlag( wk );
	MCSS_Del(mcss,wk);
}
#endif //UI_TEMPLATE_MCSS

#ifdef UI_TEMPLATE_TASKMENU
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの初期化
 *
 *	@param	menu_res	リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * UITemplate_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID )
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
static void UITemplate_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
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
static void UITemplate_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_UpdateMenu( menu );
}
#endif //UI_TEMPLATE_TASKMENU

#ifdef UI_TEMPLATE_TYPEICON
//=============================================================================
/**
 *	TYPEICON
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	属性、タイプアイコン作成
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *	@param	PokeType					タイプ
 *	@param	GFL_CLUNIT *unit	CLUNIT  ユニット
 *	@param	heapID						ヒープID
 *
 *	@return	void
 */
//-----------------------------------------------------------------------------
static void UITemplate_TYPEICON_CreateCLWK( UI_TEMPLATE_MAIN_WORK *wk, PokeType type, GFL_CLUNIT *unit, HEAPID heapID )
{	
  UI_EASY_CLWK_RES_PARAM prm;

  prm.draw_type = CLSYS_DRAW_MAIN;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
  prm.arc_id    = APP_COMMON_GetArcId();
  prm.pltt_id   = APP_COMMON_GetPokeTypePltArcIdx();
  prm.ncg_id    = APP_COMMON_GetPokeTypeCharArcIdx(type);
  prm.cell_id   = APP_COMMON_GetPokeTypeCellArcIdx( APP_COMMON_MAPPING_128K );
  prm.anm_id    = APP_COMMON_GetPokeTypeAnimeArcIdx( APP_COMMON_MAPPING_128K );
  prm.pltt_line = PLTID_OBJ_TYPEICON_M;
  prm.pltt_src_ofs  = 0;
  prm.pltt_src_num = 3;
  
  UI_EASY_CLWK_LoadResource( &wk->clres_type_icon, &prm, unit, heapID );

  wk->clwk_type_icon = UI_EASY_CLWK_CreateCLWK( &wk->clres_type_icon, unit, 128, 50, 0, heapID );
		
  GFL_CLACT_WK_SetPlttOffs( wk->clwk_type_icon, APP_COMMON_GetPokeTypePltOffset(type),
				CLWK_PLTTOFFS_MODE_PLTT_TOP );
}
//----------------------------------------------------------------------------
/**
 *	@brief	属性、タイプアイコン破棄
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UITemplate_TYPEICON_DeleteCLWK( UI_TEMPLATE_MAIN_WORK *wk )
{	
	//CLWK破棄
	GFL_CLACT_WK_Remove( wk->clwk_type_icon );
	//リソース破棄
  UI_EASY_CLWK_UnLoadResource( &wk->clres_type_icon );
}

#endif //UI_TEMPLATE_TYPEICON

#ifdef UI_TEMPLATE_ITEM_ICON
//=============================================================================
/**
 *	ITEM_ICON
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  どうぐアイコン生成
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk	ワーク
 *	@param	item_id アイテムID
 *	@param	unit    ユニット
 *	@param	heapID  ヒープID
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UITemplate_ITEM_ICON_CreateCLWK( UI_TEMPLATE_MAIN_WORK* wk, u16 item_id, GFL_CLUNIT* unit, HEAPID heapID )
{	
  UI_EASY_CLWK_RES_PARAM prm;

  prm.draw_type = CLSYS_DRAW_MAIN;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
  prm.arc_id    = ARCID_ITEMICON;
  prm.pltt_id   = ITEM_GetIndex( item_id, ITEM_GET_ICON_PAL );
  prm.ncg_id    = ITEM_GetIndex( item_id, ITEM_GET_ICON_CGX );
  prm.cell_id   = NARC_item_icon_itemicon_NCER;
  prm.anm_id    = NARC_item_icon_itemicon_NANR;
  prm.pltt_line = PLTID_OBJ_ITEMICON_M;
  prm.pltt_src_ofs  = 0;
  prm.pltt_src_num = 1;
  
  UI_EASY_CLWK_LoadResource( &wk->clres_item_icon, &prm, unit, heapID );

  wk->clwk_item_icon = UI_EASY_CLWK_CreateCLWK( &wk->clres_item_icon, unit, 16, 32, 0, heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief	どうぐアイコン破棄
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UITemplate_ITEM_ICON_DeleteCLWK( UI_TEMPLATE_MAIN_WORK* wk )
{
	//CLWK破棄
	GFL_CLACT_WK_Remove( wk->clwk_item_icon );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_item_icon );
}

#endif //UI_TEMPLATE_ITEM_ICON


#ifdef UI_TEMPLATE_POKE_ICON
//=============================================================================
/**
 *	POKE_ICON
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  ポケアイコン 生成
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *	@param	unit  ユニット
 *	@param	heapID  ヒープID  
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE_ICON_CreateCLWK( UI_TEMPLATE_MAIN_WORK* wk, u32 mons, u32 form_no, BOOL egg, GFL_CLUNIT* unit, HEAPID heapID )
{
  // ポケアイコン本体
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NCLR;
    prm.arc_id    = ARCID_POKEICON;
    prm.pltt_id   = POKEICON_GetPalArcIndex();
  //  prm.ncg_id    = POKEICON_GetCgxArcIndex(ppp);
    prm.ncg_id    = POKEICON_GetCgxArcIndexByMonsNumber( mons, form_no, egg );
    prm.cell_id   = POKEICON_GetCellArcIndex(); 
    prm.anm_id    = POKEICON_GetAnmArcIndex();
    prm.pltt_line = PLTID_OBJ_POKEICON_M;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num = 3;
    
    UI_EASY_CLWK_LoadResource( &wk->clres_poke_icon, &prm, unit, heapID );

    // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
    wk->clwk_poke_icon = UI_EASY_CLWK_CreateCLWK( &wk->clres_poke_icon, unit, 48, 32, 1, heapID );

    // 上にアイテムアイコンを描画するので優先度を下げておく
    GFL_CLACT_WK_SetSoftPri( wk->clwk_poke_icon, 1 );

    // オートアニメ ON
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk_poke_icon, TRUE );

    {
      u8 pal_num = POKEICON_GetPalNum( mons, form_no, egg );
      GFL_CLACT_WK_SetPlttOffs( wk->clwk_poke_icon, pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
    }
  }

  // ポケアイコン用アイテムアイコン
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = APP_COMMON_GetArcId();
    prm.pltt_id   = APP_COMMON_GetPokeItemIconPltArcIdx();
    prm.ncg_id    = APP_COMMON_GetPokeItemIconCharArcIdx();
    prm.cell_id   = APP_COMMON_GetPokeItemIconCellArcIdx( APP_COMMON_MAPPING_128K );
    prm.anm_id    = APP_COMMON_GetPokeItemIconAnimeArcIdx( APP_COMMON_MAPPING_128K );
    prm.pltt_line = PLTID_OBJ_POKEITEM_M;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num = 1;
    
    UI_EASY_CLWK_LoadResource( &wk->clres_poke_item, &prm, unit, heapID );

    // アニメシーケンスで指定( 0=どうぐ, 1=メール, 2=ボール )
    // ※位置調整はとりあえずの値です。
    wk->clwk_poke_item = UI_EASY_CLWK_CreateCLWK( &wk->clres_poke_item, unit, 48+1*8+4, 32+4, 2, heapID );
  }

}

//-----------------------------------------------------------------------------
/**
 *	@brief  ポケアイコン 破棄
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE_ICON_DeleteCLWK( UI_TEMPLATE_MAIN_WORK* wk )
{ 
	//CLWK破棄
	GFL_CLACT_WK_Remove( wk->clwk_poke_icon );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_poke_icon );
	
  //CLWK破棄
	GFL_CLACT_WK_Remove( wk->clwk_poke_item );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_poke_item );
}

#endif //UI_TEMPLATE_POKE_ICON

#ifdef UI_TEMPLATE_OAM_MAPMODEL
//=============================================================================
/**
 *	OAM_MAPMODEL
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  OAMマップモデル 作成 (人物OBJ特化仕様)
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *	@param	tex_idx   テクスチャのARC内インデックス
 *	@param  ptn_ofs   テクスチャオフセット（アニメパターンのオフセット）
 *	@param	GFL_CLUNIT *unit	CLUNIT  ユニット
 *	@param	heapID						ヒープID
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UITemplate_OAM_MAPMODEL_CreateCLWK( UI_TEMPLATE_MAIN_WORK *wk, u16 tex_idx, u8 ptn_ofs, GFL_CLUNIT *unit, HEAPID heapID )
{	
  UI_EASY_CLWK_RES_PARAM prm;

  prm.draw_type = CLSYS_DRAW_MAIN;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
  prm.arc_id    = APP_COMMON_GetArcId();
  prm.pltt_id   = APP_COMMON_GetNull4x4PltArcIdx();
  prm.ncg_id    = APP_COMMON_GetNull4x4CharArcIdx();
  prm.cell_id   = APP_COMMON_GetNull4x4CellArcIdx( APP_COMMON_MAPPING_128K );
  prm.anm_id    = APP_COMMON_GetNull4x4AnimeArcIdx( APP_COMMON_MAPPING_128K );
  prm.pltt_line = PLTID_OBJ_OAM_MAPMODEL_M;
  prm.pltt_src_ofs  = 0;
  prm.pltt_src_num = 1;

  // リソース読み込み
  UI_EASY_CLWK_LoadResource( &wk->clres_oam_mmdl, &prm, unit, heapID );
  // CLWK生成
  wk->clwk_oam_mmdl = UI_EASY_CLWK_CreateCLWK( &wk->clres_oam_mmdl, unit, 80, 32, 0, heapID );

  // テクスチャを転送
  {
    // 人物は基本 4 x 4
    int sx = NSBTX_DEF_SX;
    int sy = NSBTX_DEF_SY;

    CLWK_TransNSBTX( wk->clwk_oam_mmdl, ARCID_MMDL_RES, tex_idx, ptn_ofs, sx, sy, 0, prm.draw_type, heapID );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OAMマップモデル 破棄
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UITemplate_OAM_MAPMODEL_DeleteCLWK( UI_TEMPLATE_MAIN_WORK* wk )
{
	//CLWK破棄
	GFL_CLACT_WK_Remove( wk->clwk_oam_mmdl );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_oam_mmdl );
}

#endif //UI_TEMPLATE_OAM_MAPMODEL


#ifdef UI_TEMPLATE_POKE2D
//=============================================================================
/**
 *	ポケモンOBJ,BG読みこみ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンBG読みこみ
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk	ワーク
 *	@param	heapID										ヒープID
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_LoadResourceBG( UI_TEMPLATE_MAIN_WORK *wk, HEAPID heapID )
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
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_UnLoadResourceBG( UI_TEMPLATE_MAIN_WORK *wk )
{		
	GFL_BG_FillCharacterRelease( BG_FRAME_POKE_M, 1, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief	リソース読みこみ
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk	ワーク
 *	@param	heapID										ヒープID
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_LoadResourceOBJ( UI_TEMPLATE_MAIN_WORK *wk, HEAPID heapID )
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
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_UnLoadResourceOBJ( UI_TEMPLATE_MAIN_WORK *wk )
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
 *	@param	UI_TEMPLATE_MAIN_WORK *wk	ワーク
 *	@param	*clunit	ユニット
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_CreateCLWK( UI_TEMPLATE_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
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
 *	@param	UI_TEMPLATE_MAIN_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
static void UITemplate_POKE2D_DeleteCLWK( UI_TEMPLATE_MAIN_WORK *wk )
{	
	GFL_CLACT_WK_Remove( wk->clwk_poke2d );
}
#endif //UI_TEMPLATE_POKE2D

#ifdef UI_TEMPLATE_BALL
//=============================================================================
/**
 *	ボールアイコン
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ボールアイコン作成
 *
 *	@param	UI_TEMPLATE_MAIN_WORK *wk	ワーク
 *	@param	ballID	ボールID 
 *	@param	*unit		CLUNIT
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void UITemplate_BALLICON_CreateCLWK( UI_TEMPLATE_MAIN_WORK *wk, BALL_ID ballID, GFL_CLUNIT *unit, HEAPID heapID )
{	
  UI_EASY_CLWK_RES_PARAM prm;

  prm.draw_type = CLSYS_DRAW_MAIN;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
  prm.arc_id    = APP_COMMON_GetArcId();
  prm.pltt_id   = APP_COMMON_GetBallPltArcIdx( ballID );
  prm.ncg_id    = APP_COMMON_GetBallCharArcIdx( ballID );
  prm.cell_id   = APP_COMMON_GetBallCellArcIdx( ballID, APP_COMMON_MAPPING_128K );
  prm.anm_id    = APP_COMMON_GetBallAnimeArcIdx( ballID, APP_COMMON_MAPPING_128K );
  prm.pltt_line = PLTID_OBJ_BALLICON_M;
  prm.pltt_src_ofs  = 0;
  prm.pltt_src_num = 1;

  // リソース読み込み
  UI_EASY_CLWK_LoadResource( &wk->clres_ball, &prm, unit, heapID );
  // CLWK生成
  wk->clwk_ball = UI_EASY_CLWK_CreateCLWK( &wk->clres_ball, unit, 96, 32, 0, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボールアイコン破棄
 *
 *	@param	UI_TEMPLATE_MAIN_WORK* wk ワーク
 */
//-----------------------------------------------------------------------------
static void UITemplate_BALLICON_DeleteCLWK( UI_TEMPLATE_MAIN_WORK* wk )
{	
	//CLWK破棄
	GFL_CLACT_WK_Remove( wk->clwk_ball );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_ball );
}
#endif //UI_TEMPLATE_BALL

#ifdef	UI_TEMPLATE_PRINT_TOOL
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
static BOOL PrintTool_MainFunc( UI_TEMPLATE_MAIN_WORK * wk )
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
static void PrintTool_AddBmpWin( UI_TEMPLATE_MAIN_WORK * wk )
{
	wk->print_util.win = GFL_BMPWIN_Create(
													GFL_BG_FRAME0_M,					// ＢＧフレーム
													1, 1,											// 表示座標
													20, 2,										// 表示サイズ
													15,												// パレット
													GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
}

// BMPWIN削除
static void PrintTool_DelBmpWin( UI_TEMPLATE_MAIN_WORK * wk )
{
		GFL_BMPWIN_Delete( wk->print_util.win );
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( UI_TEMPLATE_MAIN_WORK * wk )
{
	GFL_BMPWIN_MakeScreen( wk->print_util.win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util.win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( UI_TEMPLATE_MAIN_WORK * wk )
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
#endif	//UI_TEMPLATE_PRINT_TOOL

#ifdef UI_TEMPLATE_PMSDRAW

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 初期化処理
 *
 *	@param	UI_TEMPLATE_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void UITemplate_PMSDRAW_Init( UI_TEMPLATE_MAIN_WORK* wk )
{
	GFL_CLUNIT	*clunit;
  
  clunit = UI_TEMPLATE_GRAPHIC_GetClunit( wk->graphic );

  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_SUB, wk->print_que, wk->font, 
      PLTID_OBJ_PMS_DRAW, UI_TEMPLATE_PMSDRAW_NUM ,wk->heapID );
  
  {
    int i;
    PMS_DATA pms;

    // PMS表示用BMPWIN生成
    for( i=0; i<UI_TEMPLATE_PMSDRAW_NUM; i++ )
    {
      wk->pms_win[i] = GFL_BMPWIN_Create(
          BG_FRAME_TEXT_S,					// ＢＧフレーム
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
 *	@param	UI_TEMPLATE_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void UITemplate_PMSDRAW_Exit( UI_TEMPLATE_MAIN_WORK* wk )
{
  PMS_DRAW_Exit( wk->pms_draw );
  {
    int i;
    for( i=0; i<UI_TEMPLATE_PMSDRAW_NUM; i++ )
    {
      GFL_BMPWIN_Delete( wk->pms_win[i] );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 主処理
 *
 *	@param	UI_TEMPLATE_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void UITemplate_PMSDRAW_Proc( UI_TEMPLATE_MAIN_WORK* wk )
{
#if 1
  // SELECTでクリア
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    int i;
    for( i=0; i<UI_TEMPLATE_PMSDRAW_NUM; i++ )
    {
      PMS_DRAW_Clear( wk->pms_draw, i, TRUE );
    }
  }
  // STARTでランダム挿入（二重登録するとアサート）
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    int i;
    for( i=0; i<UI_TEMPLATE_PMSDRAW_NUM; i++ )
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

#endif // UI_TEMPLATE_PMSDRAW

