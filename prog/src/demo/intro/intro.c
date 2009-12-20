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

//ローカルヘッダ
#include "intro_sys.h"

//描画設定
#include "intro_graphic.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "demo/intro.h"

#include "message.naix"

#include "intro_mcss.h" // for INTRO_MCSS_WORK
#include "intro_g3d.h" // for INTRO_G3D_WORK

#include "intro_cmd.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================

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

	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msg;

  INTRO_CMD_WORK*   cmd;
  INTRO_MCSS_WORK*  mcss;
  INTRO_G3D_WORK*   g3d;

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

  // 3D関連初期化
  wk->g3d = INTRO_G3D_Create( wk->graphic, wk->heapID );

  // コマンド初期化
  wk->cmd = Intro_CMD_Init( wk->g3d, wk->mcss, wk->param, wk->heapID );

  // ブライドネス設定 真っ黒
//  SetBrightness( -16, (PLANEMASK_BG0|PLANEMASK_BG2|PLANEMASK_BG3|PLANEMASK_OBJ), MASK_MAIN_DISPLAY );
  // フェードイン リクエスト
//  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );

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
  INTRO_G3D_Exit( wk->g3d );

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

#if 0
  // フェード中は処理しない
  if( GFL_FADE_CheckFade() == TRUE )
  {
    return GFL_PROC_RES_CONTINUE;
  }
  // 特定キーでアプリ終了
  if( CHECK_KEY_TRG( PAD_BUTTON_DEBUG ) )
  {
    return GFL_PROC_RES_FINISH;
  }
#endif

	//コマンド実行
  is_coutinue = Intro_CMD_Main( wk->cmd );
  
  // 終了判定
  if( is_coutinue == FALSE )
  {
    return GFL_PROC_RES_FINISH;
  }

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );

	//2D描画
	INTRO_GRAPHIC_2D_Draw( wk->graphic );

  //3D描画
  INTRO_GRAPHIC_3D_StartDraw( wk->graphic );

  INTRO_MCSS_Main( wk->mcss );
  INTRO_G3D_Main( wk->g3d );

  INTRO_GRAPHIC_3D_EndDraw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================


