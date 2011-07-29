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

//パレット点滅処理
#include "system/blink_palanm.h"

//メッセージデータ
#include "msg/msg_pw_movietitle.h"

//wipe
#include "system/wipe.h"

#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"

//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
#include "pkwd_announce_gra.naix"	// アーカイブ
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース

#include "savedata/battle_rec.h"
#include "savedata/pokewood_rec.h"


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
  PKWD_SELECT_SCREEN_HEAP_SIZE = 0x50000,  ///< ヒープサイズ
};


//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_TALK_M  = GFL_BG_FRAME0_M,
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

  PKWD_TITLE_STR_MAX = 26*2*2,  // タイトル文字列の文字数ＭＡＸ

  PKWD_SELECT_SCREEN_BMPWIN_MAX = 7,  // BMPWINの数

  PKWD_TITLE_LIST_NUM = 4,  // 映画タイトルを一度に表示できる最大数

  // BMPWIN定義
  PKWD_BMP_TITLE01=0,
  PKWD_BMP_TITLE02,
  PKWD_BMP_TITLE03,
  PKWD_BMP_TITLE04,
  PKWD_BMP_SUBTITLE_U,
  PKWD_BMP_SUBTITLE_D,
  PKWD_BMP_MSGWIN,

  SE_PKWDSELECT_DECIDE = SEQ_SE_DECIDE1,
  SE_PKWDSELECT_MOVE   = SEQ_SE_SELECT1,

};



//=============================================================================
/**
 *							データ定義
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
} PKWD_SELECT_SCREEN_BG_WORK;


#define PKWD_SELECT_SCREEN_PRINT_TOOL
//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

	PKWD_SELECT_SCREEN_BG_WORK	wk_bg;
	PKWD_SELECT_SCREEN_PARAM    *param;

  STRBUF *strbuf;

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
  WORDSET                   *wordset[PKWD_TITLE_LIST_NUM];

	//プリントキュー
	PRINT_QUE									*print_que;
  PRINT_STREAM              *print_stream;
	GFL_MSGDATA								*msg;
  GFL_TCBLSYS               *pMsgTcblSys;

  // サブシーケンス管理
  int seq;
  int subseq;

  // VBLANK 処理
  GFL_TCB    *vblank_task;
  int        bgLogoRequest; // VBlank内BG転送リクエストフラグ

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
	PRINT_UTIL								print_util[PKWD_SELECT_SCREEN_BMPWIN_MAX];
#endif	//PKWD_SELECT_SCREEN_PRINT_TOOL

  // カーソル処理
  s16 nowpos, oldpos;
  int isTouch;
  BLINKPALANM_WORK *blinkWork[4];
  

  // セーブデータ確認
  LOAD_RESULT load_result[POKEWOOD_SAVEDATA_MAX];
  int         scenario_no[POKEWOOD_SAVEDATA_MAX];

} PKWD_SELECT_SCREEN_MAIN_WORK;

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}PKWD_COUNT_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}PKWD_COUNT_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				poff;			// パレットのオフセット
}PKWD_COUNT_BGSCN_DATA;

#define PLTID_MAIN_BG_TITLE ( 0 ) // 下のテーブル読み込みのための定義

// BG定義とサブタイトル定義を取り込み
#include "../pkwd_announce/pkwd_movie_title.cdat"

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
static TOUCHBAR_WORK * PkwdSelectScreen_TOUCHBAR_InitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void PkwdSelectScreen_TOUCHBAR_ExitEx( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PkwdSelectScreen_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );
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
static void PkwdSelectScreen_LOGO_u_LoadResource( PKWD_SELECT_SCREEN_BG_WORK *wk, int scenario_no, int index );


static BOOL PrintTool_MainFunc( PKWD_SELECT_SCREEN_MAIN_WORK * wk );
static void  _load_pokewood_rec_data( PKWD_SELECT_SCREEN_MAIN_WORK *wk, PKWD_SELECT_SCREEN_PARAM *param );
static  int _touch_input( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static  int _key_input( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void _draw_titlelist( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void _draw_movie_subtitle( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int index );
static void SubTitle_Func( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PrintInit( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void PrintExit( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void _message_print( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int msg_id );
static void _blink_palanime_init( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static void VBlankFunc( GFL_TCB *tcb, void * work );

static BOOL SubSeqFunc( PKWD_SELECT_SCREEN_MAIN_WORK *wk );

static BOOL SubSeq_FadeinWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Start( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Main( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_DecideWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_MsgWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_WarningMsg( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Yesno_Normal( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Yesno_Delete1( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_Yesno_Delete2( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_FadeOut( PKWD_SELECT_SCREEN_MAIN_WORK *wk );
static BOOL SubSeq_FadeOutWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk );

static void PrintTool_AddBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk );
static void PrintTool_DelBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk );
static void PrintTool_ScreenTrans( PKWD_SELECT_SCREEN_MAIN_WORK * wk, int index );
static void PrintTool_PrintHP( PKWD_SELECT_SCREEN_MAIN_WORK * wk );


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
	param	    = pwk;
//  *param->result = POKEWOOD_SAVEDATA_MAX;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PKWD_SELSCR, PKWD_SELECT_SCREEN_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(PKWD_SELECT_SCREEN_MAIN_WORK), HEAPID_PKWD_SELSCR );
  GFL_STD_MemClear( wk, sizeof(PKWD_SELECT_SCREEN_MAIN_WORK) );

  // 初期化
  wk->heapID = HEAPID_PKWD_SELSCR;
  wk->param  = param;
	
	//描画設定初期化
	wk->graphic	= PKWD_SELECT_SCREEN_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

  // 文字列描画システム初期化
  PrintInit(wk);

	//BGリソース読み込み
	PkwdSelectScreen_BG_LoadResource( &wk->wk_bg, wk->heapID );


#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= PKWD_SELECT_SCREEN_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= PkwdSelectScreen_TOUCHBAR_InitEx( wk, clunit, wk->heapID );
    TOUCHBAR_SetBGPriorityAll( wk->touchbar, 1 );
	}
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_TALK_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
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

  // 外部セーブデータ確認・読み込み
  _load_pokewood_rec_data( wk, param );

  PrintTool_AddBmpWin( wk );			// BMPWIN作成

  // 文字列描画
  _draw_titlelist( wk );
  
  // 点滅処理初期化
  _blink_palanime_init(wk);
  
  wk->nowpos = 0;

  // タッチorキー検出
  if(GFL_UI_CheckTouchOrKey()==GFL_APP_END_TOUCH){
    wk->nowpos  = -1;
    wk->isTouch = TRUE;
  }else{
    _draw_movie_subtitle( wk, wk->nowpos );
    wk->isTouch = FALSE;
  }
  wk->bgLogoRequest=TRUE;

  // ワーク初期化
  wk->seq = 0;
  
  // VBlankFunc登録
  wk->vblank_task = GFUser_VIntr_CreateTCB( VBlankFunc, wk, 1 );


	//@todo	フェードシーケンスがないので
//	GX_SetMasterBrightness(0);
//	GXS_SetMasterBrightness(0);
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 0, 8, 1, HEAPID_PKWD_SELSCR);

  GFL_NET_ReloadIconTopOrBottom( FALSE, HEAPID_PKWD_SELSCR);
  
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------------
/**
 * @brief	点滅処理初期化
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void _blink_palanime_init( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{

  int i;
  for(i=0;i<4;i++){
    // セーブされているシナリオNOからカテゴリーを取得
    int category;
    if(wk->load_result[i]){
      category = pkwd_movie_type[wk->scenario_no[i]];
    }else{
      category = 4;
    }
    wk->blinkWork[i] = BLINKPALANM_Create( i*16, 16, BG_FRAME_BACK_M, HEAPID_PKWD_SELSCR );
    BLINKPALANM_SetPalBufferArc( wk->blinkWork[i], ARCID_PKWD_ANNOUNCE, 
                                NARC_pkwd_announce_gra_pkwd_title_select_d_NCLR, 
                                16*category, (5+category)*16 );
    BLINKPALANM_Main( wk->blinkWork[i] );

  }


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
  int i;

  // vblankFunc解放
	GFL_TCB_DeleteTask( wk->vblank_task );

  // BMP解放
  PrintTool_DelBmpWin( wk );

  for(i=0;i<4;i++){
    BLINKPALANM_Exit( wk->blinkWork[i] );
  }
  
#ifdef PKWD_SELECT_SCREEN_POKE2D
	PkwdSelectScreen_POKE2D_DeleteCLWK( wk );
	PkwdSelectScreen_POKE2D_UnLoadResourceOBJ( wk );
	PkwdSelectScreen_POKE2D_UnLoadResourceBG( wk );
#endif //PKWD_SELECT_SCREEN_POKE2D


#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//TASKMENUシステム＆リソース破棄
	APP_TASKMENU_RES_Delete( wk->menu_res );	
#endif //PKWD_SELECT_SCREEN_TASKMENU


#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバー
	PkwdSelectScreen_TOUCHBAR_ExitEx( wk );
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


  GFL_STR_DeleteBuffer( wk->strbuf );

  // 文字列描画システム解放
  PrintExit( wk );

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


enum{
  MENU_FADEIN_WAIT=0,
  MENU_START,
  MENU_MAIN,
  MENU_DECIDE_WAIT,
  MENU_MSG_WAIT,
  MENU_WARNING_MSG,
  MENU_YESNO_NORMAL,
  MENU_YESNO_DELETE1,
  MENU_YESNO_DELETE2,
  MENU_FADEOUT,
  MENU_FADEOUT_WAIT,
};


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

  // フロー制御
  if(SubSeqFunc(wk)){
    return GFL_PROC_RES_FINISH;
  }

  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
  {
//    return GFL_PROC_RES_FINISH;
    wk->param->mode ^= 1;
  }

//  PrintTool_MainFunc( wk );
#ifdef PKWD_SELECT_SCREEN_TOUCHBAR
	//タッチバーメイン処理
	PkwdSelectScreen_TOUCHBAR_Main( wk->touchbar );
#endif //PKWD_SELECT_SCREEN_TOUCHBAR


#ifdef PKWD_SELECT_SCREEN_TASKMENU
	//タスクメニューメイン処理
  if(wk->menu!=NULL){
    PkwdSelectScreen_TASKMENU_Main( wk->menu );
  }
#endif //PKWD_SELECT_SCREEN_TASKMENU

  // メッセージウインドウ用タスク
  if(wk->pMsgTcblSys){
    GFL_TCBL_Main( wk->pMsgTcblSys );
  }

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );

  PrintTool_MainFunc( wk );

  // パレット点滅メイン
  if(wk->nowpos>=0){
    BLINKPALANM_Main( wk->blinkWork[wk->nowpos] );
  }
  
	//2D描画
	PKWD_SELECT_SCREEN_GRAPHIC_2D_Draw( wk->graphic );

  

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief VBlank割り込み関数
 *
 * @param   tcb   
 * @param   work    
 */
//----------------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB *tcb, void * work )
{
  PKWD_SELECT_SCREEN_MAIN_WORK* wk = (PKWD_SELECT_SCREEN_MAIN_WORK*)work;

  //パレット転送
  if( wk->bgLogoRequest){
    PkwdSelectScreen_LOGO_u_LoadResource( &wk->wk_bg, wk->scenario_no[wk->nowpos], wk->nowpos );
    wk->bgLogoRequest = 0;
  }
}


//=============================================================================
/**
 *								static関数
 */
//=============================================================================

  enum
  {
    RES_ARC = ARCID_PKWD_ANNOUNCE,

    RES_PAL_01S_SIZE = 0x20*15,
    RES_PAL_02S_SIZE = 0x20,
    RES_PAL_M_SIZE = 0x20*11,

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

	ARCHANDLE	*handle;

	handle	= GFL_ARC_OpenDataHandle( RES_ARC, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_M_ID, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, RES_PAL_M_SIZE, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_01S_ID, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_01S_SIZE, heapID );
	
  //	----- SUB -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_01S_ID, BG_FRAME_FRAME_S, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_01S_ID,	BG_FRAME_FRAME_S, 0, 0, 0, heapID );	

	//	----- MAIN -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_M_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_M_ID, BG_FRAME_BACK_M, 0, 0, 0, heapID );		



	GFL_ARC_CloseDataHandle( handle );
}

// シナリオデータからタイトルロゴデータを引っ張るためのテーブル
static const u8 scenario_logo_tbl[]={
  0,  1,  2,  0,  //  1 -  4本目  
  1,  1,  1,  1,  //  5 -  8本目
  2,  2,  2,  2,  //  9 - 12本目  
  3,  3,  3,  3,  // 13 - 16本目
  4,  4,  4,  4,  // 17 - 20本目  
  5,  5,  5,  5,  // 21 - 24本目
  6,  6,  6,  6,  // 25 - 28本目  
  7,  7,  7,  7,  // 29 - 32本目
  8,  8,  8,  8,  // 33 - 36本目  
  9,  9,  9,  9,  // 37 - 40本目
};

//----------------------------------------------------------------------------------
/**
 * @brief	ムービーロゴのみ転送
 *
 * @param   wk		      PKWD_SELECT_SCREEN_MAIN_WORK
 * @param   scenario_no	シナリオNO（-1のときはシナリオ指定なし）
 * @param   index       カーソル位置
 */
//----------------------------------------------------------------------------------
static void PkwdSelectScreen_LOGO_u_LoadResource( PKWD_SELECT_SCREEN_BG_WORK *wk, int scenario_no, int index )
{
  int pal, cgx, scr;
  ARCHANDLE	*handle;

  // タッチ待ちでカーソルがどこにも無いときは空白ロゴを転送
  if(index==-1){
    pal = NARC_pkwd_announce_gra_pkwd_title99_logo_NCLR;
    cgx = NARC_pkwd_announce_gra_pkwd_title99_logo_NCGR;
    scr = NARC_pkwd_announce_gra_pkwd_title99_NSCR;
  }else{
    pal = pwc_bgpal_01_tbl[scenario_logo_tbl[scenario_no]].arcid;
    cgx = pwc_bgchr_01_tbl[scenario_logo_tbl[scenario_no]].arcid;
    scr = pwc_bgscn_01_tbl[scenario_logo_tbl[scenario_no]].arcid;
  }
  OS_Printf("scenario_no=%d\n", scenario_no);

	handle	= GFL_ARC_OpenDataHandle( ARCID_PKWD_ANNOUNCE, HEAPID_PKWD_SELSCR );

	GFL_ARCHDL_UTIL_TransVramPalette( handle, pal, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_02S_SIZE, HEAPID_PKWD_SELSCR );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, cgx, BG_FRAME_BACK_S,  0, 0, 0, HEAPID_PKWD_SELSCR );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, scr,	BG_FRAME_BACK_S,  0, 0, 0, HEAPID_PKWD_SELSCR );	

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
	};

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
	APP_TASKMENU_ITEMWORK	item[2];
	APP_TASKMENU_WORK			*menu;	

	//窓の設定
	for( i = 0; i < NELEMS(item); i++ )
	{	
		item[i].str	      = GFL_MSG_CreateString( msg, pw_select_yes+i );	//文字列
		item[i].msgColor	= APP_TASKMENU_ITEM_MSGCOLOR;	//文字色
		item[i].type			= APP_TASKMENU_WIN_TYPE_NORMAL;	//窓の種類
	}

	//初期化
	init.heapId		= heapID;
	init.itemNum	= NELEMS(item);
	init.itemWork = item;
	init.posType	= ATPT_RIGHT_DOWN;
	init.charPosX	= 32;
	init.charPosY = 23;
	init.w				= APP_TASKMENU_PLATE_WIDTH_YN_WIN;
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
  {
    int i;
    for(i=0;i<PKWD_SELECT_SCREEN_BMPWIN_MAX;i++){
      PRINT_UTIL_Trans( &wk->print_util[i], wk->print_que );
    }
  }
	return TRUE;
}


#define TITLE_WIN_W ( 26 )
#define TITLE_WIN_H (  5 )
#define SUBTITLE_WIN1_W  ( 26 )
#define SUBTITLE_WIN1_H  (  3 )
#define SUBTITLE_WIN2_W  ( 26 )
#define SUBTITLE_WIN2_H  (  4 )
#define MSGWIN_W      ( 28 )
#define MSGWIN_H      (  4 )
#define TITLE_PAL     ( 10 )
#define SUBTITLE_PAL  ( 14 )
#define MSGWINFRAME_PAL   ( 14 )
#define MSGWIN_PAL        ( 15 )
#define MSGWINFRAME_CGX   ( 32 )

typedef struct {
  u16 frame;
  u16 pal;
  u8  x, y;
  u8  w, h;
}BMPWIN_DAT;


static const BMPWIN_DAT bmpdat[]={
  
  { // 下画面タイトルリスト１
    GFL_BG_FRAME2_M, TITLE_PAL, 5, 0, TITLE_WIN_W, TITLE_WIN_H,
  },
  { // 下タイトルリスト２
    GFL_BG_FRAME2_M, TITLE_PAL, 5, 5, TITLE_WIN_W, TITLE_WIN_H,
  },
  { // 下タイトルリスト３
    GFL_BG_FRAME2_M, TITLE_PAL, 5, 10, TITLE_WIN_W, TITLE_WIN_H,
  },
  { // 下タイトルリスト４
    GFL_BG_FRAME2_M, TITLE_PAL, 5, 15, TITLE_WIN_W, TITLE_WIN_H,
  },
  { // 上画面サブタイトル１
    GFL_BG_FRAME0_S, SUBTITLE_PAL, 3,  0, SUBTITLE_WIN1_W, SUBTITLE_WIN1_H,
  },
  { // 上画面サブタイトル２
    GFL_BG_FRAME0_S, SUBTITLE_PAL, 3, 16, SUBTITLE_WIN2_W, SUBTITLE_WIN2_H,
  },
  { // メッセージウインドウ
    GFL_BG_FRAME0_M, MSGWIN_PAL, 2, 1, MSGWIN_W, MSGWIN_H,
  },
};

// BMPWIN作成
static void PrintTool_AddBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
  int i;
  
  for(i=0;i<PKWD_SELECT_SCREEN_BMPWIN_MAX;i++){
    const BMPWIN_DAT *bmp = &bmpdat[i];
    wk->print_util[i].win = GFL_BMPWIN_Create(
                              bmp->frame,     // ＢＧフレーム
                              bmp->x, bmp->y, // 表示座標
                              bmp->w, bmp->h, // 表示サイズ
                              bmp->pal,       // パレット
                              GFL_BMP_CHRAREA_GET_B );// キャラ取得方向
    OS_Printf("add bmpwin %d\n", i);
  }

  // メッセージウインドウ用のフレームリソース転送
  BmpWinFrame_PalSet( GFL_BG_FRAME0_M, MSGWINFRAME_PAL, 0, HEAPID_PKWD_SELSCR );
  BmpWinFrame_GraphicSet( GFL_BG_FRAME0_M, MSGWINFRAME_CGX, MSGWINFRAME_PAL, 0, HEAPID_PKWD_SELSCR );

}

// BMPWIN削除
static void PrintTool_DelBmpWin( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
  int i;
  
  for(i=0;i<PKWD_SELECT_SCREEN_BMPWIN_MAX;i++){
    GFL_BMPWIN_Delete( wk->print_util[i].win );
  }
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( PKWD_SELECT_SCREEN_MAIN_WORK * wk, int index )
{
  GFL_BMPWIN_MakeScreen( wk->print_util[index].win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util[index].win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( PKWD_SELECT_SCREEN_MAIN_WORK * wk )
{
//  GFL_MSG_GetString(wk->msg, mes_mictest_title, wk->strbuf );

	// BMPWIN内の座標(32,0)を基準に右詰めで「ＨＰ」と表示
  PRINT_UTIL_Print( &wk->print_util[0], wk->print_que, 32,0, wk->strbuf, wk->font );

}
#endif	//PKWD_SELECT_SCREEN_PRINT_TOOL


#define LOAD_SAVEDATA_MAX ( 4 )
//----------------------------------------------------------------------------------
/**
 * @brief	外部セーブデータ確認・読み込み
 *
 * @param   wk    
 * @param   param 
 */
//----------------------------------------------------------------------------------
static void  _load_pokewood_rec_data( PKWD_SELECT_SCREEN_MAIN_WORK *wk, PKWD_SELECT_SCREEN_PARAM *param )
{
  int i;

  BattleRec_LoadToolModule(); //gimmickのオーバーレイと干渉するので注意
  
  // ポケウッド録画データアクセス初期化
  POKEWOOD_REC_Init( HEAPID_PKWD_SELSCR );

  for(i=0;i<LOAD_SAVEDATA_MAX;i++){
    wk->load_result[i] = POKEWOOD_REC_Load( param->sv, HEAPID_PKWD_SELSCR, i );
    OS_Printf("%d:load result=%d\n", i , wk->load_result[i]);

    // 読み込んで各種情報を取得したい場合はココで確保
    if(wk->load_result[i]==RECLOAD_RESULT_OK){
      wk->scenario_no[i] = POKEWOOD_REC_GetData( POKEWOOD_REC_PARAM_SENARIO, NULL);
      OS_Printf("%d: senario_no=%d\n", i , wk->scenario_no[i]);
    }
  }

  // ポケウッド録画データアクセス処理終了
  POKEWOOD_REC_Exit();

  BattleRec_UnloadToolModule();
}

// タッチ入力判定用定義
static const GFL_UI_TP_HITTBL tp_data[] ={
	{ 0, 40, 0, 255 },
	{ 0, 80, 0, 255 },
	{ 0,120, 0, 255 },
	{ 0,160, 0, 255 },
  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};
//----------------------------------------------------------------------------------
/**
 * @brief	タッチパネル判定
 *
 * @param   wk		PKWD_SELECT_SCREEN_MAIN_WORK
 *
 * @retval  int		選択できる上映番号
 */
//----------------------------------------------------------------------------------
static int _touch_input( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result = GFL_UI_TP_HitTrg( tp_data );
  if( result!=GFL_UI_TP_HIT_NONE ){
    if(wk->load_result[result]){
      wk->isTouch = TRUE;
      wk->nowpos = result;
      wk->bgLogoRequest=TRUE;
      _draw_movie_subtitle( wk, wk->nowpos );
      return result;
    }
  }

  return GFL_UI_TP_HIT_NONE;
}


static int _key_input( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(wk->isTouch==TRUE && GFL_UI_KEY_GetTrg()){
    wk->isTouch = FALSE;
    wk->nowpos = 0;
  }else{
    
    if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
      if(wk->nowpos!=0){
        if(wk->load_result[wk->nowpos-1]){
          PMSND_PlaySE( SE_PKWDSELECT_MOVE );
          wk->nowpos--;
        }
      }
    }else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
      if(wk->nowpos<3){
        if(wk->load_result[wk->nowpos+1]){
          PMSND_PlaySE( SE_PKWDSELECT_MOVE );
          wk->nowpos++;
        }
      }
    }else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
      return wk->nowpos;
    }

  }
  return GFL_UI_TP_HIT_NONE;
}


//----------------------------------------------------------------------------------
/**
 * @brief	文字列描画システム系初期化
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void PrintInit( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;
	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  // メッセージフォント用パレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT,NARC_font_default_nclr, PALTYPE_MAIN_BG, 0x20*15, 0x20, HEAPID_PKWD_SELSCR );
  

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_pw_movietitle_dat, wk->heapID );
  wk->strbuf = GFL_STR_CreateBuffer( PKWD_TITLE_STR_MAX, HEAPID_PKWD_SELSCR );

  // WORDSET(リストの数だけ作成）
  for(i=0;i<PKWD_TITLE_LIST_NUM;i++){
    wk->wordset[i] = WORDSET_Create( HEAPID_PKWD_SELSCR );
  }

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );
  
  // メッセージ表示用タスクシステム初期化
  wk->pMsgTcblSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 32 , 32 );


}

//----------------------------------------------------------------------------------
/**
 * @brief	文字列描画システム系解放
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void PrintExit( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;

  // メッセージ表示用タスクシステム終了
  GFL_TCBL_Exit( wk->pMsgTcblSys );

	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );

  // WORDSET解放
  for(i=0;i<PKWD_TITLE_LIST_NUM;i++){
    WORDSET_Delete( wk->wordset[i] );
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief 下画面リスト描画
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static void _draw_titlelist( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int i;
  STRBUF *strbuf = GFL_STR_CreateBuffer( PKWD_TITLE_STR_MAX, GetHeapLowID( HEAPID_PKWD_SELSCR ));

  for(i=0;i<PKWD_TITLE_LIST_NUM;i++){
    if(wk->load_result[i]){
      GFL_MSG_GetString( wk->msg, pw_screen_01+i, strbuf );
      PRINT_UTIL_PrintColor( &wk->print_util[i], wk->print_que, 0, 4, strbuf, 
                             wk->font, PRINTSYS_LSB_Make( 15, 14, 0 ) );
      GFL_MSG_GetString( wk->msg, pw_title_00+wk->scenario_no[i], strbuf );
      PRINT_UTIL_PrintColor( &wk->print_util[i], wk->print_que, 0, 4+16, strbuf, 
                             wk->font, PRINTSYS_LSB_Make( 15, 14, 0 ) );
      PrintTool_ScreenTrans( wk, i );		// スクリーン転送

    }
  }

  GFL_STR_DeleteBuffer(strbuf);

  OS_Printf(" remain memory = %08x\n", GFL_HEAP_GetHeapFreeSize(HEAPID_PKWD_SELSCR));
}


//----------------------------------------------------------------------------------
/**
 * @brief	センタリングして文字列描画
 */
//----------------------------------------------------------------------------------
static void _centering_print( PRINT_UTIL *util, PRINT_QUE *que, STRBUF *strbuf, int y, GFL_FONT *font, PRINTSYS_LSB col)
{
  int x,str_w,bmp_w;
  str_w = PRINTSYS_GetStrWidth( strbuf, font, 0 );
  bmp_w = GFL_BMPWIN_GetSizeX( util->win )*8;
  x = bmp_w/2 - str_w/2;

  PRINT_UTIL_PrintColor( util, que, x, y, strbuf, font, col );
  
}

//----------------------------------------------------------------------------------
/**
 * @brief	上画面のサブタイトルを描画する
 *
 * @param   wk    PKWD_SELECT_SCREEN_MAIN_WORK
 * @param   index 録画データ番号( 0-4 )
 */
//----------------------------------------------------------------------------------
static void _draw_movie_subtitle( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int index )
{
  STRBUF *strbuf    = GFL_STR_CreateBuffer( PKWD_TITLE_STR_MAX, GetHeapLowID( HEAPID_PKWD_SELSCR ));
  STRBUF *expandbuf = GFL_STR_CreateBuffer( PKWD_TITLE_STR_MAX, GetHeapLowID( HEAPID_PKWD_SELSCR ));
  int sex = MyStatus_GetMySex(wk->param->mystatus);
  
  // サブタイトル上部を描画
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->print_util[PKWD_BMP_SUBTITLE_U].win), 0 );
  GFL_MSG_GetString( wk->msg, pw_movietitle_01+wk->scenario_no[index], strbuf );
  _centering_print( &wk->print_util[PKWD_BMP_SUBTITLE_U], wk->print_que, 
                    strbuf, 6, wk->font, PRINTSYS_LSB_Make( 15, 14, 0 ) );
  
  // 下部を描画
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->print_util[PKWD_BMP_SUBTITLE_D].win), 0 );
  GFL_MSG_GetString( wk->msg, pkwd_subtitle_tbl[wk->scenario_no[index]][sex], strbuf );
  WORDSET_ExpandStr( wk->wordset[index], expandbuf, strbuf );
  _centering_print( &wk->print_util[PKWD_BMP_SUBTITLE_D], wk->print_que,
                    strbuf, 0, wk->font, PRINTSYS_LSB_Make( 13, 12, 0 ) );

  GFL_STR_DeleteBuffer(expandbuf);
  GFL_STR_DeleteBuffer(strbuf);

  PrintTool_ScreenTrans( wk, PKWD_BMP_SUBTITLE_U );		// スクリーン転送
  PrintTool_ScreenTrans( wk, PKWD_BMP_SUBTITLE_D );		// スクリーン転送
  
}


//----------------------------------------------------------------------------------
/**
 * @brief	選択箇所がが更新されている場合は上画面に反映させる
 *
 * @param   wk		PKWD_SELECT_SCREEN_MAIN_WORK
 */
//----------------------------------------------------------------------------------
static void SubTitle_Func( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(wk->oldpos!=wk->nowpos && wk->nowpos>=0){
    wk->bgLogoRequest=TRUE;
    _draw_movie_subtitle( wk, wk->nowpos );
    BLINKPALANM_InitAnimeCount(wk->blinkWork[wk->nowpos] );
  }
  wk->oldpos = wk->nowpos;
}


#define WINCLR_COL(col) (((col)<<4)|(col))

//----------------------------------------------------------------------------------
/**
 * @brief	メッセージウインドウ描画開始
 *
 * @param   wk      PKWD_SELECT_SCREEN_MAIN_WORK *
 * @param   msg_id	メッセージＩＤ
 */
//----------------------------------------------------------------------------------
static void _message_print( PKWD_SELECT_SCREEN_MAIN_WORK *wk, int msg_id )
{
  GFL_BMPWIN *bmpwin = wk->print_util[PKWD_BMP_MSGWIN].win;
  
  BmpWinFrame_Write( bmpwin, WINDOW_TRANS_OFF, MSGWINFRAME_CGX, MSGWINFRAME_PAL );
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( bmpwin ), WINCLR_COL(15));

  GFL_MSG_GetString( wk->msg, msg_id, wk->strbuf );
  wk->print_stream = PRINTSYS_PrintStream(
    bmpwin, 0, 0, wk->strbuf, wk->font,
    MSGSPEED_GetWait(), wk->pMsgTcblSys, 4, HEAPID_PKWD_SELSCR, 15 );
  
  GFL_BMPWIN_MakeTransWindow( bmpwin );

}  

// サブシーケンス関数テーブル
static BOOL (*SubSeqFuncTbl[])( PKWD_SELECT_SCREEN_MAIN_WORK *wk )={
  SubSeq_FadeinWait,  // MENU_FADEIN_WAIT=0
  SubSeq_Start,       // MENU_START,
  SubSeq_Main,        // MENU_MAIN,
  SubSeq_DecideWait,  // MENU_DECIDE_WAIT,
  SubSeq_MsgWait,     // MENU_MSG_WAIT,
  SubSeq_WarningMsg,  // MENU_WARNING_MSG,
  SubSeq_Yesno_Normal,       // MENU_YESNO_NORMAL,
  SubSeq_Yesno_Delete1,
  SubSeq_Yesno_Delete2,
  SubSeq_FadeOut,     // MENU_FADEOUT,
  SubSeq_FadeOutWait, // MENU_FADEOUT_WAIT,
};

//----------------------------------------------------------------------------------
/**
 * @brief	サブシーケンス呼び出し関数
 *
 * @param   wk		
 *
 * @retval  BOOL		
 */
//----------------------------------------------------------------------------------
static BOOL SubSeqFunc( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  return SubSeqFuncTbl[wk->seq]( wk );
}


static BOOL SubSeq_FadeinWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(WIPE_SYS_EndCheck()){
    wk->seq = MENU_START;
  }
  return FALSE;
}


static BOOL SubSeq_Start( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  wk->seq = MENU_MAIN;
  return FALSE;
}
static BOOL SubSeq_Main( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result;
  if((result=_touch_input(wk))==GFL_UI_TP_HIT_NONE){
    result = _key_input(wk);
  }
  SubTitle_Func(wk);
  if(result!=GFL_UI_TP_HIT_NONE){
    PMSND_PlaySE( SE_PKWDSELECT_DECIDE );
    BLINKPALANM_SetDecideAnime( wk->blinkWork[wk->nowpos] );
    wk->seq = MENU_DECIDE_WAIT;
  }else{
    if(TOUCHBAR_GetTrg(wk->touchbar)==TOUCHBAR_ICON_RETURN ||
       TOUCHBAR_GetTouch(wk->touchbar)==TOUCHBAR_ICON_RETURN ){
      wk->seq = MENU_FADEOUT;
    }
  }
  return FALSE;

}
static BOOL SubSeq_DecideWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(BLINKPALANM_CheckDecideAnime( wk->blinkWork[wk->nowpos] )){
    if(wk->param->mode==PKWD_SELECT_MODE_NORMAL){
      _message_print( wk, pw_select_01 ); //「このさくひんを　かんしょうしますか？」
      wk->seq    = MENU_MSG_WAIT;
      wk->subseq = MENU_YESNO_NORMAL;
    }else{
      _message_print( wk, pw_select_02 ); //「このさくひんを　さくじょしますか？」
      wk->seq    = MENU_MSG_WAIT;
      wk->subseq = MENU_YESNO_DELETE1;
    }
  }
  return FALSE;

}
static BOOL SubSeq_MsgWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  //メッセージ描画終了待ち
  if(PRINTSYS_PrintStreamGetState( wk->print_stream )==PRINTSTREAM_STATE_DONE)
  {
    PRINTSYS_PrintStreamDelete( wk->print_stream );
    wk->menu= PkwdSelectScreen_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );
    wk->seq = wk->subseq;
  }
  return FALSE;

}
static BOOL SubSeq_WarningMsg( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
   _message_print( wk, pw_select_03 ); //「このさくひんを　さくじょしますか？」
   wk->seq    = MENU_MSG_WAIT;
   wk->subseq = MENU_YESNO_DELETE2;
  return FALSE;

}

static int _yesno_wait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result = -1;
  if(APP_TASKMENU_IsFinish(wk->menu)){
    if(APP_TASKMENU_GetCursorPos( wk->menu )==0){
      result = 0;
    }else{
      result = 1;
    }
    PkwdSelectScreen_TASKMENU_Exit( wk->menu );
    wk->menu = NULL;
    BmpWinFrame_Clear( wk->print_util[PKWD_BMP_MSGWIN].win, WINDOW_TRANS_OFF );
    GFL_BMPWIN_ClearTransWindow( wk->print_util[PKWD_BMP_MSGWIN].win );

  }
  return result;

}

static BOOL SubSeq_Yesno_Normal( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result;
  if((result=_yesno_wait(wk))>=0){
    if(result==0){
        wk->seq = MENU_FADEOUT;
    }else{
      wk->seq   = MENU_START;
      BLINKPALANM_InitAnimeCount(wk->blinkWork[wk->nowpos] );
    }
  }
  return FALSE;
  
}
static BOOL SubSeq_Yesno_Delete1( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result;
  if((result=_yesno_wait(wk))>=0){
    if(result==0){
        wk->seq = MENU_WARNING_MSG;
    }else{
      wk->seq   = MENU_START;
      BLINKPALANM_InitAnimeCount(wk->blinkWork[wk->nowpos] );
    }
  }
  return FALSE;
}
static BOOL SubSeq_Yesno_Delete2( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  int result;
  if((result=_yesno_wait(wk))>=0){
    if(result==0){
        wk->seq = MENU_FADEOUT;
    }else{
      wk->seq   = MENU_START;
      BLINKPALANM_InitAnimeCount(wk->blinkWork[wk->nowpos] );
    }
  }
  return FALSE;
}


static BOOL SubSeq_FadeOut( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 0, 8, 1, HEAPID_PKWD_SELSCR);
  wk->seq = MENU_FADEOUT_WAIT;
  return FALSE;

}
static BOOL SubSeq_FadeOutWait( PKWD_SELECT_SCREEN_MAIN_WORK *wk )
{
  if(WIPE_SYS_EndCheck()){
    return TRUE;
  }
  return FALSE;
}

