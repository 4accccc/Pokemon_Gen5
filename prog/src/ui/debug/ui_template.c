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
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "font/font.naix"

#include "arc_def.h"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "hoge_graphic.h"

//タッチバー
#include "ui/touchbar.h"

//外部公開
#include "ui_template.h"

#include "message.naix" // GMM

//@TODO とりあえずマイクテストのリソース
#include "mictest.naix"	// アーカイブ
#include "msg/msg_mictest.h"  // GMM

// 下記defineをコメントアウトすると、機能を取り除ける
#define UI_POKE_ICON

// プロセス定義データ
const GFL_PROC_DATA UITemplateProcData = {
	UITemplateProc_Init,
	UITemplateProc_Main,
	UITemplateProc_Exit,
};

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  UI_TEMPLATE_HEAP_SIZE = 0x20000,  ///< ヒープサイズ
};

//-------------------------------------
///	フレーム
//=====================================
enum
{	
	BG_FRAME_BAR_M	= GFL_BG_FRAME0_M,
	BG_FRAME_BACK_M	= GFL_BG_FRAME1_M,
	BG_FRAME_BACK_S	= GFL_BG_FRAME1_S,
};
//-------------------------------------
///	パレット
//=====================================
enum
{	
	//メインBG
	PLTID_BG_BACK_M			= 0,
	PLTID_BG_TOUCHBAR_M	= 13,
	PLTID_BG_INFOWIN_M	= 15,
	//サブBG
	PLTID_BG_BACK_S	=		 0,

	//メインOBJ
	PLTID_OBJ_TOUCHBAR_M	= 0,
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//@TODO プリントQUE対応
//--------------------------------------------------------------
///	BG管理ワーク
//==============================================================
typedef struct {
  GFL_FONT* fontHandle;
} UI_TEMPLATE_BG_WORK;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct {
  HEAPID heap_id;

	UI_TEMPLATE_BG_WORK   wk_bg;

	//描画設定
	HOGE_GRAPHIC_WORK			*p_graphic;

	//タッチバー
	TOUCHBAR_WORK					*p_touchbar;
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
static void UITemplate_BG_LoadResource( UI_TEMPLATE_BG_WORK* wk, HEAPID heap_id );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	*pwk
 *	@param	*mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT UITemplateProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
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
  wk->heap_id = HEAPID_UI_DEBUG;
	
	//描画設定初期化
	wk->p_graphic	= HOGE_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heap_id );

	//BGリソース読み込み
	UITemplate_BG_LoadResource( &wk->wk_bg, wk->heap_id );

	//INFOWINの初期化
	{
		GAME_COMM_SYS_PTR comm;
		comm	= GAMESYSTEM_GetGameCommSysPtr(param->p_gamesys);
		INFOWIN_Init( BG_FRAME_BAR_M, 
				PLTID_BG_INFOWIN_M, comm, wk->heap_id );
	}

	//タッチバーの初期化
	{	
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
		TOUCHBAR_SETUP	touchbar_setup;
		touchbar_setup.p_item		= touchbar_icon_tbl;
		touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);
		touchbar_setup.p_unit		= HOGE_GRAPHIC_GetClunit( wk->p_graphic );
		touchbar_setup.bar_frm	= BG_FRAME_BAR_M;
		touchbar_setup.bg_plt		= PLTID_BG_TOUCHBAR_M;
		touchbar_setup.obj_plt	= PLTID_OBJ_TOUCHBAR_M;
		touchbar_setup.mapping	= APP_COMMON_MAPPING_128K;

		wk->p_touchbar	= TOUCHBAR_Init( &touchbar_setup, wk->heap_id );
	}


	//@todo	フェードシーケンスがないので
	GX_SetMasterBrightness(0);
	GXS_SetMasterBrightness(0);

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	*pwk
 *	@param	*mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT UITemplateProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	UI_TEMPLATE_MAIN_WORK* wk = mywk;

	//タッチバー破棄
	TOUCHBAR_Exit( wk->p_touchbar );

	//INFOWIN破棄
	INFOWIN_Exit();

	//描画設定破棄
	HOGE_GRAPHIC_Exit( wk->p_graphic );


  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heap_id );

	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	*pwk
 *	@param	*mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT UITemplateProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	UI_TEMPLATE_MAIN_WORK* wk = mywk;

  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG )
  {
    return GFL_PROC_RES_FINISH;
  }

	//INFOWINメイン
	INFOWIN_Update();

	//描画設定メイン
	HOGE_GRAPHIC_2D_Draw( wk->p_graphic );

	//TOUCHBAR
	TOUCHBAR_Main( wk->p_touchbar );

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
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void UITemplate_BG_LoadResource( UI_TEMPLATE_BG_WORK* wk, HEAPID heap_id )
{
  //@TODO とりあえずマイクテストのリソース
	ARCHANDLE	*handle;

	handle	= GFL_ARC_OpenDataHandle( ARCID_MICTEST_GRA, heap_id );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_mictest_back_bg_down_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0x20, heap_id );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_mictest_back_bg_up_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0x20, heap_id );
	
  //	----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_mictest_back_bg_down_NCGR,
						BG_FRAME_BACK_S, 0, 0, 0, heap_id );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_mictest_back_bg_down_NSCR,
						BG_FRAME_BACK_S, 0, 0, 0, heap_id );	

	//	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, NARC_mictest_back_bg_up_NCGR,
						BG_FRAME_BACK_M, 0, 0, 0, heap_id );
	GFL_ARCHDL_UTIL_TransVramScreen(	handle, NARC_mictest_back_bg_up_NSCR,
						BG_FRAME_BACK_M, 0, 0, 0, heap_id );		

	GFL_ARC_CloseDataHandle( handle );
}

