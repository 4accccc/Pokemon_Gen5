//=============================================================================
/**
 *
 *	@file		intro.c
 *	@brief  3Dデモ再生アプリ
 *	@author	genya hosaka
 *	@data		2009.12.10
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/brightness.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//アプリ共通素材
#include "app/app_menu_common.h"

// 文字列関連
#include "font/font.naix"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h" //PRINT_QUE

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//タッチバー
#include "ui/touchbar.h"

//INFOWIN
#include "infowin/infowin.h"

//ローカルヘッダ
#include "intro_sys.h"

//描画設定
#include "intro_graphic.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "demo/intro.h"

#include "message.naix"

#include "intro_mcss.h"
#include "intro_cmd.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
//#define INTRO_INFOWIN
//#define INTRO_TOUCHBAR
//#define INTRO_TASKMENU

FS_EXTERN_OVERLAY(ui_common);

#define CHECK_KEY_TRG( key ) ( ( GFL_UI_KEY_GetTrg() & (key) ) == (key) )

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
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

  INTRO_PARAM*        param;

	//描画設定
	INTRO_GRAPHIC_WORK	*graphic;

#ifdef INTRO_TOUCHBAR
	//タッチバー
	TOUCHBAR_WORK							*touchbar;
	//以下カスタムボタン使用する場合のサンプルリソース
	u32												ncg_btn;
	u32												ncl_btn;
	u32												nce_btn;
#endif //INTRO_TOUCHBAR

	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msg;

#ifdef INTRO_TASKMENU
	//タスクメニュー
	APP_TASKMENU_RES					*menu_res;
	APP_TASKMENU_WORK					*menu;
#endif //INTRO_TASKMENU

#ifdef	INTRO_PRINT_TOOL
	//プリントユーティリティ
	PRINT_UTIL								print_util;
	u32												seq;
#endif	//INTRO_PRINT_TOOL

  INTRO_CMD_WORK*   cmd;
  INTRO_MCSS_WORK*  mcss;

} INTRO_MAIN_WORK;


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
static GFL_PROC_RESULT IntroProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT IntroProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT IntroProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	汎用処理ユーティリティ
//=====================================

#ifdef INTRO_INFOWIN
//-------------------------------------
///	INFOWIN
//=====================================
static void Intro_INFOWIN_Init( GAMESYS_WORK *gamesys, HEAPID heapID );
static void Intro_INFOWIN_Exit( void );
static void Intro_INFOWIN_Main( void );
#endif //INTRO_INFOWIN

#ifdef INTRO_TOUCHBAR
//-------------------------------------
///	タッチバー
//=====================================
static TOUCHBAR_WORK * Intro_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID );
static void Intro_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar );
static void Intro_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );
//以下カスタムボタン使用サンプル用
static TOUCHBAR_WORK * Intro_TOUCHBAR_InitEx( INTRO_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void Intro_TOUCHBAR_ExitEx( INTRO_MAIN_WORK *wk );
#endif //INTRO_TOUCHBAR

#ifdef INTRO_TASKMENU
//-------------------------------------
///	リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * Intro_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID );
static void Intro_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void Intro_TASKMENU_Main( APP_TASKMENU_WORK *menu );
#endif //INTRO_TASKMENU

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA ProcData_Intro = 
{
	IntroProc_Init,
	IntroProc_Main,
	IntroProc_Exit,
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
 *	@param	*pwk					INTRO_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IntroProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	INTRO_MAIN_WORK *wk;
	INTRO_PARAM *param;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );
	
	//引数取得
	param	= pwk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_INTRO, INTRO_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(INTRO_MAIN_WORK), HEAPID_INTRO );
  GFL_STD_MemClear( wk, sizeof(INTRO_MAIN_WORK) );

  // 初期化
  wk->heapID      = HEAPID_INTRO;
  wk->param       = param;
	
	//描画設定初期化
	wk->graphic	= INTRO_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, wk->heapID );

	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_mictest_dat, wk->heapID );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );

  // MCSS初期化
  wk->mcss = INTRO_MCSS_Create( wk->heapID );

  // コマンド初期化
  wk->cmd = Intro_CMD_Init( wk->mcss, wk->param, wk->heapID );

#ifdef INTRO_INFOWIN
	//INFOWINの初期化
	Intro_INFOWIN_Init( param->gamesys, wk->heapID );
#endif //INTRO_INFOWIN

#ifdef INTRO_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= INTRO_GRAPHIC_GetClunit( wk->graphic );
		wk->touchbar	= Intro_TOUCHBAR_InitEx( wk, clunit, wk->heapID );
	}
#endif //INTRO_TOUCHBAR

#ifdef INTRO_TASKMENU
	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_BAR_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
	wk->menu			= Intro_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );
#endif //INTRO_TASKMENU

  // @TODO 汎用性のために、初期化コマンドを用意するべき
  // ブライドネス設定 真っ黒
  SetBrightness( -16, (PLANEMASK_BG0|PLANEMASK_BG2|PLANEMASK_BG3|PLANEMASK_OBJ), MASK_MAIN_DISPLAY );

  // フェードイン リクエスト
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					INTRO_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IntroProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	INTRO_MAIN_WORK* wk = mywk;
  
  if( *seq == 0 )
  {
    // フェードアウト リクエスト
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 2 );
    (*seq)++;
    return GFL_PROC_RES_CONTINUE;
  }
  else if( GFL_FADE_CheckFade() == TRUE )
  {
    // フェード中は処理に入らない
    return GFL_PROC_RES_CONTINUE;
  }
  
  INTRO_MCSS_Exit( wk->mcss );

#ifdef INTRO_TASKMENU
	//TASKMENUシステム＆リソース破棄
	Intro_TASKMENU_Exit( wk->menu );
	APP_TASKMENU_RES_Delete( wk->menu_res );	
#endif //INTRO_TASKMENU

#ifdef INTRO_TOUCHBAR
	//タッチバー
	Intro_TOUCHBAR_ExitEx( wk );
#endif //INTRO_TOUCHBAR

#ifdef INTRO_INFOWIN
	//INFWIN
	Intro_INFOWIN_Exit();
#endif //INTRO_INFOWIN

	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );
  
  //コマンド 破棄
  Intro_CMD_Exit( wk->cmd );

	//描画設定破棄
	INTRO_GRAPHIC_Exit( wk->graphic );

	//PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heapID );

	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}


static void debug_camera_test( GFL_G3D_CAMERA* camera )
{ 
  VecFx32 pos;

  static BOOL mode = 0;

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    mode = (mode+1)%3;

    if( mode == 0 )
    {
      OS_Printf("mode=%d Pos\n",mode);
    }
    else if( mode == 1 )
    {
      OS_Printf("mode=%d CamUp\n",mode);
    }
    else
    {
      OS_Printf("mode=%d Target\n",mode);
    }
  }
  
  if( mode == 0 )
  {
    GFL_G3D_CAMERA_GetPos( camera, &pos );
  }
  else if( mode == 1 )
  {
    GFL_G3D_CAMERA_GetCamUp( camera, &pos );
  }
  else
  {
    GFL_G3D_CAMERA_GetTarget( camera, &pos );
  }

  if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
  {
    pos.y += 1;
    OS_Printf("mode=%d pos{ 0x%x, 0x%x, 0x%x } \n", mode, pos.x, pos.y, pos.z );
  }
  else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
  {
    pos.y -= 1;
    OS_Printf("mode=%d pos{ 0x%x, 0x%x, 0x%x } \n", mode, pos.x, pos.y, pos.z );
  }    
  else if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
  {
    pos.x += 1;
    OS_Printf("mode=%d pos{ 0x%x, 0x%x, 0x%x } \n", mode, pos.x, pos.y, pos.z );
  }
  else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
  {
    pos.x -= 1;
    OS_Printf("mode=%d pos{ 0x%x, 0x%x, 0x%x } \n", mode, pos.x, pos.y, pos.z );
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    pos.z += 1;
    OS_Printf("mode=%d pos{ 0x%x, 0x%x, 0x%x } \n", mode, pos.x, pos.y, pos.z );
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    pos.z -= 1;
    OS_Printf("mode=%d pos{ 0x%x, 0x%x, 0x%x } \n", mode, pos.x, pos.y, pos.z );
  }
  
  // データセット
  if( mode == 0 )
  {
    GFL_G3D_CAMERA_SetPos( camera, &pos );
  }
  else if( mode == 1 )
  {
    GFL_G3D_CAMERA_SetCamUp( camera, &pos );
  }
  else
  {
    GFL_G3D_CAMERA_SetTarget( camera, &pos );
  }

  // データ吐き出し
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    GFL_G3D_CAMERA_GetPos( camera, &pos );
    OS_Printf("pos { 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
    GFL_G3D_CAMERA_GetCamUp( camera, &pos );
    OS_Printf("CamUp { 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
    GFL_G3D_CAMERA_GetTarget( camera, &pos );
    OS_Printf("Taraget { 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
}



//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					INTRO_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IntroProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	INTRO_MAIN_WORK* wk = mywk;
  BOOL is_coutinue;

  // フェード中は処理しない
  if( GFL_FADE_CheckFade() == TRUE )
  {
    return GFL_PROC_RES_CONTINUE;
  }

  // 特定キーでアプリ終了
  if( 
      CHECK_KEY_TRG( PAD_BUTTON_DEBUG ) 
    )
  {
    return GFL_PROC_RES_FINISH;
  }

#ifdef INTRO_TOUCHBAR
	//タッチバーメイン処理
	Intro_TOUCHBAR_Main( wk->touchbar );
#endif //INTRO_TOUCHBAR

#ifdef INTRO_INFOWIN
	//INFWINメイン処理
	Intro_INFOWIN_Main();
#endif //INTRO_INFOWIN

#ifdef INTRO_TASKMENU
	//タスクメニューメイン処理
	Intro_TASKMENU_Main( wk->menu );
#endif //INTRO_TASKMENU

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );

	//2D描画
	INTRO_GRAPHIC_2D_Draw( wk->graphic );

  // カメラテスト
#ifdef PM_DEBUG
  { 
    static BOOL is_on = TRUE;
 
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
    {
      is_on ^= 1;
    }

    if( is_on )
    {
      GFL_G3D_CAMERA* camera = INTRO_GRAPHIC_GetCamera( wk->graphic );
      debug_camera_test( camera );
    }
  }
#endif

  //3D描画
  INTRO_GRAPHIC_3D_StartDraw( wk->graphic );

  INTRO_MCSS_Main( wk->mcss );

  INTRO_GRAPHIC_3D_EndDraw( wk->graphic );

	//コマンド実行
  is_coutinue = Intro_CMD_Main( wk->cmd );

  // 終了判定
  if( is_coutinue == FALSE )
  {
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================


#ifdef INTRO_INFOWIN
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
static void Intro_INFOWIN_Init( GAMESYS_WORK *gamesys, HEAPID heapID )
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
static void Intro_INFOWIN_Exit( void )
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
static void Intro_INFOWIN_Main( void )
{	
	INFOWIN_Update();
}
#endif //INTRO_INFOWIN

#ifdef INTRO_TOUCHBAR
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
static TOUCHBAR_WORK * Intro_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID )
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
static void Intro_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar )
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
static TOUCHBAR_WORK * Intro_TOUCHBAR_InitEx( INTRO_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
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
static void Intro_TOUCHBAR_ExitEx( INTRO_MAIN_WORK *wk )
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
static void Intro_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}
#endif //INTRO_TOUCHBAR

#ifdef INTRO_TASKMENU
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの初期化
 *
 *	@param	menu_res	リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * Intro_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID )
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
static void Intro_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
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
static void Intro_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_UpdateMenu( menu );
}
#endif //INTRO_TASKMENU

#ifdef	INTRO_PRINT_TOOL
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
static BOOL PrintTool_MainFunc( INTRO_MAIN_WORK * wk )
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
static void PrintTool_AddBmpWin( INTRO_MAIN_WORK * wk )
{
	wk->print_util.win = GFL_BMPWIN_Create(
													GFL_BG_FRAME0_M,					// ＢＧフレーム
													1, 1,											// 表示座標
													20, 2,										// 表示サイズ
													15,												// パレット
													GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
}

// BMPWIN削除
static void PrintTool_DelBmpWin( INTRO_MAIN_WORK * wk )
{
		GFL_BMPWIN_Delete( wk->print_util.win );
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( INTRO_MAIN_WORK * wk )
{
	GFL_BMPWIN_MakeScreen( wk->print_util.win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util.win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( INTRO_MAIN_WORK * wk )
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
#endif	//INTRO_PRINT_TOOL

    

