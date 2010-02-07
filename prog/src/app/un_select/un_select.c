//=============================================================================
/**
 *
 *	@file		un_select.c
 *	@brief  国連 フロア選択
 *	@author	genya hosaka
 *	@data		2010.02.05
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/bmp_winframe.h" // for BmpWinFrame_

// 簡易会話表示システム
#include "system/pms_draw.h"

#include "gamesystem/msgspeed.h"  // for MSGSPEED_GetWait

// 文字列関連
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h"
#include "font/font.naix"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "un_select_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//タッチバー
#include "ui/touchbar.h"

// シーンコントローラー
#include "ui/ui_scene.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/un_select.h"

#include "message.naix"
#include "un_select_gra.naix"	// アーカイブ

// サウンド
#include "sound/pm_sndsys.h"

//@TODO
#include "msg/msg_mictest.h"  // GMM

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define UN_SELECT_TOUCHBAR

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  UN_SELECT_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};


//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_MENU_M	= GFL_BG_FRAME1_M,
	BG_FRAME_TEXT_M	= GFL_BG_FRAME2_M,
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
	PLTID_BG_TEXT_M				= 1,
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
} UN_SELECT_BG_WORK;

typedef struct
{ 
  //[IN]
  HEAPID heapID;
  //[PRIVATE]
  GFL_TCBLSYS   *tcblsys;     ///< タスクシステム
  GFL_MSGDATA   *msghandle;   ///< メッセージハンドル
	GFL_FONT			*font;        ///< フォント
	PRINT_QUE			*print_que;   ///< プリントキュー
  WORDSET       *wordset;     ///< ワードセット

  STRBUF        *strbuf;
  STRBUF        *exp_strbuf;

  // ストリーム再生
  PRINT_STREAM* print_stream;
  GFL_BMPWIN    *win_talk;

} UN_SELECT_MSG_WORK;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

	UN_SELECT_BG_WORK				wk_bg;

	//描画設定
	UN_SELECT_GRAPHIC_WORK	*graphic;

#ifdef UN_SELECT_TOUCHBAR
	//タッチバー
	TOUCHBAR_WORK							*touchbar;
	//以下カスタムボタン使用する場合のサンプルリソース
	u32												ncg_btn;
	u32												ncl_btn;
	u32												nce_btn;
#endif //UN_SELECT_TOUCHBAR
	
  //タスクメニュー
	APP_TASKMENU_RES					*menu_res;
	APP_TASKMENU_WORK					*menu;

  UI_SCENE_CNT_PTR cntScene;

  UN_SELECT_PARAM* pwk;

  UN_SELECT_MSG_WORK* wk_msg;

} UN_SELECT_MAIN_WORK;

//=============================================================================
/**
 *							シーン定義
 */
//=============================================================================

// フロア選択
static BOOL SceneSelectFloor_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneSelectFloor_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneSelectFloor_End( UI_SCENE_CNT_PTR cnt, void* work );

// 確認画面
static BOOL SceneConfirm_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneConfirm_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneConfirm_End( UI_SCENE_CNT_PTR cnt, void* work );

//--------------------------------------------------------------
///	SceneID
//==============================================================
typedef enum
{ 
  UNS_SCENE_ID_SELECT_FLOOR = 0,  ///< フロア選択
  UNS_SCENE_ID_CONFIRM,           ///< 確認画面

  UNS_SCENE_ID_MAX,
} UNS_SCENE_ID;

//--------------------------------------------------------------
///	SceneFunc
//==============================================================
static const UI_SCENE_FUNC_SET c_scene_func_tbl[ UNS_SCENE_ID_MAX ] = 
{
  // UNS_SCENE_ID_SELECT_FLOOR
  {
    SceneSelectFloor_Init,
    NULL,
    SceneSelectFloor_Main,
    NULL,
    SceneSelectFloor_End,
  },
  // UNS_SCENE_ID_CONFIRM
  {
    SceneConfirm_Init,
    NULL,
    SceneConfirm_Main,
    NULL,
    SceneConfirm_End,
  },
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT UNSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT UNSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT UNSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void UNSelect_BG_LoadResource( UN_SELECT_BG_WORK* wk, HEAPID heapID );

#ifdef UN_SELECT_TOUCHBAR
//-------------------------------------
///	タッチバー
//=====================================
static TOUCHBAR_WORK * UNSelect_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID );
static void UNSelect_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar );
static void UNSelect_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );
#endif //UN_SELECT_TOUCHBAR

//-------------------------------------
///	リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * UNSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID );
static void UNSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void UNSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu );

//-------------------------------------
// メッセージ管理モジュール
//=====================================
static UN_SELECT_MSG_WORK* MSG_Create( HEAPID heapID );
static void MSG_Delete( UN_SELECT_MSG_WORK* wk );
static void MSG_Main( UN_SELECT_MSG_WORK* wk );
static void MSG_SetPrint( UN_SELECT_MSG_WORK* wk, int str_id );
static BOOL MSG_PrintProc( UN_SELECT_MSG_WORK* wk );
static GFL_FONT* MSG_GetFont( UN_SELECT_MSG_WORK* wk );
static PRINT_QUE* MSG_GetPrintQue( UN_SELECT_MSG_WORK* wk );

// PROTOTYPE
static UN_SELECT_MAIN_WORK* proc_init( GFL_PROC* proc, UN_SELECT_PARAM* prm );
static void param_out_data( UN_SELECT_MAIN_WORK* wk );

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA UNSelectProcData = 
{
	UNSelectProc_Init,
	UNSelectProc_Main,
	UNSelectProc_Exit,
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
 *	@param	*pwk					UN_SELECT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UNSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	UN_SELECT_MAIN_WORK *wk;
	
  GF_ASSERT( pwk );

  switch( *seq )
  {
  case 0:
    wk = proc_init( proc, pwk );
  
    // フェードイン リクエスト
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 1 );

    (*seq)++;
    break;

  case 1:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      return GFL_PROC_RES_FINISH;
    }
    break;

  default : GF_ASSERT(0);
  }

  return GFL_PROC_RES_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					UN_SELECT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UNSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	UN_SELECT_MAIN_WORK* wk = mywk;
    
  if( GFL_FADE_CheckFade() == TRUE )
  {
     return GFL_PROC_RES_CONTINUE;
  }
  
  // シーンコントーラ削除
  UI_SCENE_CNT_Delete( wk->cntScene );

  // メッセージ消去
  MSG_Delete( wk->wk_msg );

	//TASKMENUシステム＆リソース破棄
	UNSelect_TASKMENU_Exit( wk->menu );
	APP_TASKMENU_RES_Delete( wk->menu_res );	

#ifdef UN_SELECT_TOUCHBAR
	//タッチバー
	UNSelect_TOUCHBAR_Exit( wk->touchbar );
#endif //UN_SELECT_TOUCHBAR

	//描画設定破棄
	UN_SELECT_GRAPHIC_Exit( wk->graphic );

	//PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heapID );

	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  HOSAKA_Printf(" PROC終了！ \n");

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					UN_SELECT_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UNSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	UN_SELECT_MAIN_WORK* wk = mywk;
  
  // SCENE
  if( UI_SCENE_CNT_Main( wk->cntScene ) )
  {
    return GFL_PROC_RES_FINISH;
  }

  // メッセージ主処理
  MSG_Main( wk->wk_msg );

#ifdef UN_SELECT_TOUCHBAR
	//タッチバーメイン処理
	UNSelect_TOUCHBAR_Main( wk->touchbar );
#endif //UN_SELECT_TOUCHBAR

	//2D描画
	UN_SELECT_GRAPHIC_2D_Draw( wk->graphic );

	//3D描画
	UN_SELECT_GRAPHIC_3D_StartDraw( wk->graphic );

	UN_SELECT_GRAPHIC_3D_EndDraw( wk->graphic );

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
 *	@param	UN_SELECT_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void UNSelect_BG_LoadResource( UN_SELECT_BG_WORK* wk, HEAPID heapID )
{
	ARCHANDLE	*handle;

	handle	= GFL_ARC_OpenDataHandle( ARCID_UN_SELECT_GRA, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_un_select_gra_kokuren_bg_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0x20, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_un_select_gra_kokuren_bg_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0x20, heapID );
	
  //	----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_un_select_gra_kokuren_bg_NCGR,
						BG_FRAME_BACK_S, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_un_select_gra_kokuren_bgu_NSCR,
						BG_FRAME_BACK_S, 0, 0, 0, heapID );	

	//	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_un_select_gra_kokuren_bg_NCGR,
						BG_FRAME_BACK_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_un_select_gra_kokuren_bgd_NSCR,
						BG_FRAME_BACK_M, 0, 0, 0, heapID );		

	GFL_ARC_CloseDataHandle( handle );
}


#ifdef UN_SELECT_TOUCHBAR
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
static TOUCHBAR_WORK * UNSelect_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID )
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
	touchbar_setup.bar_frm	= BG_FRAME_MENU_M;						//BG読み込みのためのBG面
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
static void UNSelect_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar )
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
static void UNSelect_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}
#endif //UN_SELECT_TOUCHBAR

//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの初期化
 *
 *	@param	menu_res	リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * UNSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID )
{	
	int i;
	APP_TASKMENU_INITWORK	init;
	APP_TASKMENU_ITEMWORK	item[2];
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
static void UNSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
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
static void UNSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_UpdateMenu( menu );
}



//=============================================================================
//
//
// メッセージ管理クラス
//
//
//=============================================================================
enum
{ 
  STRBUF_SIZE = 1600,
};

#define MSG_SKIP_BTN (PAD_BUTTON_A|PAD_BUTTON_B)

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ管理クラス 生成
 *
 *	@param	HEAPID heapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static UN_SELECT_MSG_WORK* MSG_Create( HEAPID heapID )
{
  UN_SELECT_MSG_WORK* wk;

  // メモリ アロック
  wk = GFL_HEAP_AllocClearMemory( heapID, sizeof( UN_SELECT_MSG_WORK ) );

  // メンバ初期化
  wk->heapID = heapID;
  
  // ワードセット生成
  wk->wordset = WORDSET_Create( heapID );

	//メッセージ
	wk->msghandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_mictest_dat, heapID );
  //@TODO マイクテストのリソース

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( heapID );

	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

  // STRBU 生成
  wk->strbuf      = GFL_STR_CreateBuffer( STRBUF_SIZE, heapID );
  wk->exp_strbuf  = GFL_STR_CreateBuffer( STRBUF_SIZE, heapID );

  wk->tcblsys = GFL_TCBL_Init( heapID, heapID, 1, 0 );
  
  // ウィンドウ生成
  wk->win_talk = GFL_BMPWIN_Create( BG_FRAME_TEXT_M, 1, 19, 30, 4, PLTID_BG_TEXT_M, GFL_BMP_CHRAREA_GET_B );
  
  // フレーム生成
  BmpWinFrame_Write( wk->win_talk, WINDOW_TRANS_OFF, 1, GFL_BMPWIN_GetPalette(wk->win_talk) );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win_talk), 0x0 );

  GFL_BMPWIN_MakeTransWindow( wk->win_talk );

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ管理クラス 破棄
 *
 *	@param	UN_SELECT_MSG_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MSG_Delete( UN_SELECT_MSG_WORK* wk )
{
  // BMPWIN 破棄
  GFL_BMPWIN_Delete( wk->win_talk );
  // STRBUF 破棄
  GFL_STR_DeleteBuffer( wk->strbuf );
  GFL_STR_DeleteBuffer( wk->exp_strbuf );
	//メッセージ破棄
	GFL_MSG_Delete( wk->msghandle );
	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	//FONT
	GFL_FONT_Delete( wk->font );
  // TCBL
  GFL_TCBL_Exit( wk->tcblsys );
  // ワードセット 破棄
  WORDSET_Delete( wk->wordset );

  // メモリ破棄
  GFL_HEAP_FreeMemory( wk );
}


//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ管理クラス 主処理
 *
 *	@param	UN_SELECT_MSG_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MSG_Main( UN_SELECT_MSG_WORK* wk )
{
	PRINTSYS_QUE_Main( wk->print_que );
  
  GFL_TCBL_Main( wk->tcblsys );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ管理クラス メッセージプリント リクエスト
 *
 *	@param	UN_SELECT_MSG_WORK* wk
 *	@param	str_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MSG_SetPrint( UN_SELECT_MSG_WORK* wk, int str_id )
{
  const u8 clear_color = 15;
  GFL_BMPWIN* win;
  int msgspeed;
  
  msgspeed  = MSGSPEED_GetWait();
  win       = wk->win_talk;
  
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(win), clear_color);
  GFL_FONTSYS_SetColor(1, 2, clear_color);

  GFL_MSG_GetString( wk->msghandle, str_id, wk->strbuf );

#ifdef PM_DEBUG
  GFL_STR_CheckBufferValid( wk->strbuf ); ///< 破損チェック
#endif
  
  WORDSET_ExpandStr( wk->wordset, wk->exp_strbuf, wk->strbuf );

#ifdef PM_DEBUG
  GFL_STR_CheckBufferValid( wk->strbuf ); ///< 破損チェック
#endif

  wk->print_stream = PRINTSYS_PrintStream( win, 4, 0, wk->exp_strbuf, wk->font, msgspeed,
                                           wk->tcblsys, 0xffff, wk->heapID, clear_color );

  // @TODO 必要ない？
  { 
    GFL_BMPWIN_TransVramCharacter( win );
    GFL_BMPWIN_MakeScreen( win );
    GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(win) );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ管理クラス メッセージ処理
 *
 *	@param	UN_SELECT_MSG_WORK* wk 
 *
 *	@retval TRUE:プリント終了
 */
//-----------------------------------------------------------------------------
static BOOL MSG_PrintProc( UN_SELECT_MSG_WORK* wk )
{
  PRINTSTREAM_STATE state;

//  HOSAKA_Printf("print state= %d \n", state );

  if( wk->print_stream )
  {
    state = PRINTSYS_PrintStreamGetState( wk->print_stream );

    switch( state )
    {
    case PRINTSTREAM_STATE_DONE : // 終了
      PRINTSYS_PrintStreamDelete( wk->print_stream );
      wk->print_stream = NULL;
      return TRUE;

    case PRINTSTREAM_STATE_PAUSE : // 一時停止中
      // キー入力待ち
      if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE || (  GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL ) || GFL_UI_TP_GetTrg() )
      {
        PRINTSYS_PrintStreamReleasePause( wk->print_stream );
        GFL_SOUND_PlaySE( SEQ_SE_DECIDE1 );
      }
      break;

    case PRINTSTREAM_STATE_RUNNING :  // 実行中
      // メッセージスキップ
      if( (GFL_UI_KEY_GetCont() & MSG_SKIP_BTN) || GFL_UI_TP_GetCont() )
      {
        PRINTSYS_PrintStreamShortWait( wk->print_stream, 0 );
      }
      break;

    default : GF_ASSERT(0);
    }

    return FALSE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	UN_SELECT_MSG_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_FONT* MSG_GetFont( UN_SELECT_MSG_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->font;
}

//-----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	UN_SELECT_MSG_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static PRINT_QUE* MSG_GetPrintQue( UN_SELECT_MSG_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->print_que;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化
 *
 *	@param	GFL_PROC* proc
 *	@param	prm 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static UN_SELECT_MAIN_WORK* proc_init( GFL_PROC* proc, UN_SELECT_PARAM* prm )
{
  UN_SELECT_MAIN_WORK* wk;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UN_SELECT, UN_SELECT_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(UN_SELECT_MAIN_WORK), HEAPID_UN_SELECT );
  GFL_STD_MemClear( wk, sizeof(UN_SELECT_MAIN_WORK) );

  // 初期化
  wk->heapID = HEAPID_UN_SELECT;
  wk->pwk = prm;
	
	//描画設定初期化
	wk->graphic	= UN_SELECT_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

  // メッセージ生成
  wk->wk_msg = MSG_Create( wk->heapID );
  
  // シーンコントローラ作成
  wk->cntScene = UI_SCENE_CNT_Create( 
      wk->heapID, c_scene_func_tbl, UNS_SCENE_ID_MAX, 
      UNS_SCENE_ID_SELECT_FLOOR, wk );

	//BGリソース読み込み
	UNSelect_BG_LoadResource( &wk->wk_bg, wk->heapID );

#ifdef UN_SELECT_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= UN_SELECT_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= UNSelect_TOUCHBAR_Init( clunit, wk->heapID );
	}
#endif //UN_SELECT_TOUCHBAR

	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_MENU_M, PLTID_BG_TASKMENU_M,
      MSG_GetFont( wk->wk_msg ),
      MSG_GetPrintQue( wk->wk_msg ),
      wk->heapID );

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  選択結果吐き出し
 *
 *	@param	UN_SELECT_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void param_out_data( UN_SELECT_MAIN_WORK* wk )
{
  GF_ASSERT(wk);
  GF_ASSERT(wk->pwk);
  
  //@TODO 
}

//-----------------------------------------------------------------------------
/**
 *	@brief  SCENE フロア選択 初期化処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneSelectFloor_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  SCENE フロア選択 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneSelectFloor_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  // フロア選択処理
  // @TODO
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    UI_SCENE_CNT_SetNextScene( cnt, UNS_SCENE_ID_CONFIRM );
    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  SCENE フロア選択 後処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneSelectFloor_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  SCENE 確認画面 初期化処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneConfirm_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

	wk->menu = UNSelect_TASKMENU_Init( wk->menu_res, wk->wk_msg->msghandle, wk->heapID );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  SCENE 確認画面 主処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneConfirm_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

	//タスクメニューメイン処理
	UNSelect_TASKMENU_Main( wk->menu );

  if( APP_TASKMENU_IsFinish( wk->menu ) )
  {
    u8 pos = APP_TASKMENU_GetCursorPos( wk->menu );

    switch( pos )
    {
    case 0 :
      // データ吐き出し
      param_out_data( wk );
      // フェードアウト リクエスト
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 1 );
      // 終了
      UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
      break;
    case 1 :
      // 選択画面に戻る
      UNSelect_TASKMENU_Exit( wk->menu );
      UI_SCENE_CNT_SetNextScene( cnt, UNS_SCENE_ID_SELECT_FLOOR );
      break;
    default : GF_ASSERT(0);
    }

    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  SCENE 確認画面 後処理
 *
 *	@param	UI_SCENE_CNT_PTR cnt
 *	@param	work 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneConfirm_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  return TRUE;
}

