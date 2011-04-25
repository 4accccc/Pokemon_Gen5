//=============================================================================
/**
 *
 *	@file		fmission_app.c
 *	@brief  フェスミッションアプリ メイン 
 *	@author Miyuki Iwasawa	
 *	@data	  2011.04.22	
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "fmission_app_local.h"
#include "fmission_app_sub.h"

//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
#include "msg/msg_mictest.h"  // GMM
#include "fmission_app.naix"		//
#include "townmap_gra.naix"		//

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
static GFL_PROC_RESULT FMissionAppProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT FMissionAppProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT FMissionAppProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

static int seq_Main( FMISSION_APP_WORK* wk );
static int seq_InitialDraw( FMISSION_APP_WORK* wk );
static int seq_FadeIn( FMISSION_APP_WORK* wk );
static int seq_FadeOut( FMISSION_APP_WORK* wk );

//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void FMissionApp_BG_LoadResource( FMISSION_APP_BGWK* wk, HEAPID heapID );

#ifdef FMISSION_INFOWIN
//-------------------------------------
///	INFOWIN
//=====================================
static void FMissionApp_INFOWIN_Init( GAMESYS_WORK *gsys, HEAPID heapID );
static void FMissionApp_INFOWIN_Exit( void );
static void FMissionApp_INFOWIN_Main( void );
#endif //FMISSION_INFOWIN

#ifdef FMISSION_TOUCHBAR
//-------------------------------------
///	タッチバー
//=====================================
static TOUCHBAR_WORK * FMissionApp_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID );
static void FMissionApp_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar );
static void FMissionApp_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar );
#endif //FMISSION_TOUCHBAR

#ifdef FMISSION_MCSS
//-------------------------------------
///	MCSS
//=====================================
static MCSS_SYS_WORK * FMissionApp_MCSS_Init( u16 wk_max, HEAPID heapID );
static void FMissionApp_MCSS_Exit( MCSS_SYS_WORK * mcss );
static void FMissionApp_MCSS_Draw( MCSS_SYS_WORK * mcss );
static MCSS_WORK * FMissionApp_MCSS_CreateWkPP( MCSS_SYS_WORK * mcss, POKEMON_PARAM *pp, const VecFx32 *pos );
static void FMissionApp_MCSS_DeleteWk( MCSS_SYS_WORK * mcss, MCSS_WORK *wk );
#endif //FMISSION_MCSS

#ifdef FMISSION_TASKMENU
//-------------------------------------
///	リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * FMissionApp_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID );
static void FMissionApp_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void FMissionApp_TASKMENU_Main( APP_TASKMENU_WORK *menu );
#endif //FMISSION_TASKMENU

#ifdef FMISSION_POKE_ICON
//-------------------------------------
///	ポケアイコン
//=====================================
static void FMissionApp_POKE_ICON_CreateCLWK( FMISSION_APP_WORK* wk, u32 mons, u32 form_no, u32 sex, BOOL egg, GFL_CLUNIT* unit, HEAPID heapID );
static void FMissionApp_POKE_ICON_DeleteCLWK( FMISSION_APP_WORK* wk );
#endif //FMISSION_POKE_ICON

#ifdef FMISSION_POKE2D
//-------------------------------------
///	ポケモンOBJ,BG読みこみ
//=====================================
static void FMissionApp_POKE2D_LoadResourceBG( FMISSION_APP_WORK *wk, HEAPID heapID );
static void FMissionApp_POKE2D_UnLoadResourceBG( FMISSION_APP_WORK *wk );
static void FMissionApp_POKE2D_LoadResourceOBJ( FMISSION_APP_WORK *wk, HEAPID heapID );
static void FMissionApp_POKE2D_UnLoadResourceOBJ( FMISSION_APP_WORK *wk );
static void FMissionApp_POKE2D_CreateCLWK( FMISSION_APP_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void FMissionApp_POKE2D_DeleteCLWK( FMISSION_APP_WORK *wk );
#endif //FMISSION_POKE2D

#ifdef FMISSION_PMSDRAW
//-------------------------------------
///	簡易会話表示
//=====================================
static void FMissionApp_PMSDRAW_Init( FMISSION_APP_WORK* wk );
static void FMissionApp_PMSDRAW_Exit( FMISSION_APP_WORK* wk );
static void FMissionApp_PMSDRAW_Proc( FMISSION_APP_WORK* wk );
#endif // FMISSION_PMSDRAW


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA FMissionAppProcData = 
{
	FMissionAppProc_Init,
	FMissionAppProc_Main,
	FMissionAppProc_Exit,
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
 *	@param	*pwk					FMISSION_APP_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FMissionAppProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	FMISSION_APP_WORK *wk;
	FMISSION_APP_PARAM *param;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//引数取得
	param	= pwk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UI_DEBUG, FMISSION_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(FMISSION_APP_WORK), HEAPID_UI_DEBUG );
  GFL_STD_MemClear( wk, sizeof(FMISSION_APP_WORK) );

  // 初期化
  wk->heapID = HEAPID_UI_DEBUG;
	
	//描画設定初期化
	wk->graphic	= FMISSION_APP_GRA_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

	//フォント作成
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	//メッセージ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			NARC_message_mictest_dat, wk->heapID );

	//PRINT_QUE作成
	wk->print_que		= PRINTSYS_QUE_Create( wk->heapID );

	//BGリソース読み込み
	FMissionApp_BG_LoadResource( &wk->wk_bg, wk->heapID );

#ifdef FMISSION_INFOWIN
	//INFOWINの初期化
	FMissionApp_INFOWIN_Init( param->gsys, wk->heapID );
#endif //FMISSION_INFOWIN

#ifdef FMISSION_TOUCHBAR
	//タッチバーの初期化
	{	
		GFL_CLUNIT	*clunit	= FMISSION_APP_GRA_GetClunit( wk->graphic );
		wk->touchbar	= FMissionApp_TOUCHBAR_Init( clunit, wk->heapID );
	}
#endif //FMISSION_TOUCHBAR

#ifdef FMISSION_MCSS
	//MCSS初期化&ワーク作成
	wk->mcss_sys	= FMissionApp_MCSS_Init( 1, wk->heapID );
	{
		POKEMON_PARAM	*pp;  
		VecFx32 pos	= { 0, -FX32_ONE*10, 0	};

		pp	= PP_Create( MONSNO_HUSIGIBANA, 0, 0, wk->heapID );
		wk->mcss_wk		= FMissionApp_MCSS_CreateWkPP( wk->mcss_sys, pp, &pos );
		GFL_HEAP_FreeMemory( pp );
	}
#endif //FMISSION_MCSS

#ifdef FMISSION_TASKMENU
	//TASKMENUリソース読み込み＆初期化
	wk->menu_res	= APP_TASKMENU_RES_Create( BG_FRAME_BAR_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
	wk->menu			= FMissionApp_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );
#endif //FMISSION_TASKMENU

#ifdef FMISSION_POKE_ICON
  // ポケアイコンの読み込み
  {
		GFL_CLUNIT	*clunit	= FMISSION_APP_GRA_GetClunit( wk->graphic );
    FMissionApp_POKE_ICON_CreateCLWK( wk, MONSNO_HUSIGIDANE, 0, 0, FALSE, clunit, wk->heapID );
  }
#endif //FMISSION_POKE_ICON

#ifdef FMISSION_POKE2D
	//ポケモンOBJ,BG読みこみ
	FMissionApp_POKE2D_LoadResourceBG( wk, wk->heapID );
	{	
		GFL_CLUNIT *clunit = FMISSION_APP_GRA_GetClunit( wk->graphic );
		FMissionApp_POKE2D_LoadResourceOBJ( wk, wk->heapID );
		FMissionApp_POKE2D_CreateCLWK( wk, clunit, wk->heapID );
	}
#endif //FMISSION_POKE2D

#ifdef FMISSION_PMSDRAW  
  FMissionApp_PMSDRAW_Init( wk );
#endif //FMISSION_PMSDRAW

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
 *	@param	*pwk					FMISSION_APP_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FMissionAppProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	FMISSION_APP_WORK* wk = mywk;

#ifdef FMISSION_POKE2D
	FMissionApp_POKE2D_DeleteCLWK( wk );
	FMissionApp_POKE2D_UnLoadResourceOBJ( wk );
	FMissionApp_POKE2D_UnLoadResourceBG( wk );
#endif //FMISSION_POKE2D

#ifdef FMISSION_POKE_ICON
   FMissionApp_POKE_ICON_DeleteCLWK( wk );
#endif //FMISSION_POKE_ICON

#ifdef FMISSION_TASKMENU
	//TASKMENUシステム＆リソース破棄
	FMissionApp_TASKMENU_Exit( wk->menu );
	APP_TASKMENU_RES_Delete( wk->menu_res );	
#endif //FMISSION_TASKMENU

#ifdef FMISSION_MCSS
	//MCSS破棄
	FMissionApp_MCSS_DeleteWk( wk->mcss_sys, wk->mcss_wk );
	FMissionApp_MCSS_Exit( wk->mcss_sys );
#endif //FMISSION_MCSS

#ifdef FMISSION_TOUCHBAR
	//タッチバー
	FMissionApp_TOUCHBAR_Exit( wk->touchbar );
#endif //FMISSION_TOUCHBAR

#ifdef FMISSION_INFOWIN
	//INFWIN
	FMissionApp_INFOWIN_Exit();
#endif //FMISSION_INFOWIN

#ifdef FMISSION_PMSDRAW
  FMissionApp_PMSDRAW_Exit( wk );
#endif //FMISSION_PMSDRAW

	//メッセージ破棄
	GFL_MSG_Delete( wk->msg );

	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );

	//FONT
	GFL_FONT_Delete( wk->font );

	//描画設定破棄
	FMISSION_APP_GRA_Exit( wk->graphic );

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
 *	@param	*pwk					FMISSION_APP_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FMissionAppProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	FMISSION_APP_WORK* wk = mywk;

  switch(*seq){
  case SEQ_INI_DRAW:
    *seq = seq_InitialDraw( wk );
    break;
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

#ifdef FMISSION_TOUCHBAR
	//タッチバーメイン処理
	FMissionApp_TOUCHBAR_Main( wk->touchbar );
#endif //FMISSION_TOUCHBAR

#ifdef FMISSION_INFOWIN
	//INFWINメイン処理
	FMissionApp_INFOWIN_Main();
#endif //FMISSION_INFOWIN

#ifdef FMISSION_TASKMENU
	//タスクメニューメイン処理
	FMissionApp_TASKMENU_Main( wk->menu );
#endif //FMISSION_TASKMENU

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );

#ifdef FMISSION_PMSDRAW
  FMissionApp_PMSDRAW_Proc( wk );
#endif //FMISSION_PMSDRAW

	//2D描画
	FMISSION_APP_GRA_2D_Draw( wk->graphic );

	//3D描画
	FMISSION_APP_GRA_3D_StartDraw( wk->graphic );

#ifdef FMISSION_MCSS
	FMissionApp_MCSS_Draw( wk->mcss_sys );
#endif //FMISSION_MCSS
	FMISSION_APP_GRA_3D_EndDraw( wk->graphic );

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
static int seq_Main( FMISSION_APP_WORK* wk )
{
  return FMissionApp_InputCheck( wk );
}

/*
 *  @brief  初期描画
 */
static int seq_InitialDraw( FMISSION_APP_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    //初期描画
    //FMissionApp_InitialDraw( wk );
    wk->seq++;
    break;
  case 1:
    if( PRINTSYS_QUE_IsFinished( wk->print_que ) ){	
      wk->seq = 0;
			return SEQ_FADEIN;
		}
    break;
  }
  return SEQ_INI_DRAW; 
}

/*
 *  @brief  フェードイン
 */
static int seq_FadeIn( FMISSION_APP_WORK* wk )
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
static int seq_FadeOut( FMISSION_APP_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    if( TOUCHBAR_GetTrg( wk->touchbar ) != TOUCHBAR_ICON_RETURN ){  //戻るボタンのアニメを待つ
      break;
    }

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
 *	@param	FMISSION_APP_BGWK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void FMissionApp_BG_LoadResource( FMISSION_APP_BGWK* wk, HEAPID heapID )
{
  enum
  {
    RES_ARC = ARCID_FMISSION_APP,

    RES_PAL_S_SIZE = 0x200,
    RES_PAL_M_SIZE = 0x200,

    RES_PAL_S_ID = NARC_fmission_app_fmapp_bgu_nclr,
    RES_CHR_S_ID = NARC_fmission_app_fmapp_bgu_lz_ncgr,
    RES_SCR_S_ID = NARC_fmission_app_fmapp_bgu03_lz_nscr,

    RES_PAL_M_ID = NARC_fmission_app_fmapp_bgd_nclr,
    RES_CHR_M_ID = NARC_fmission_app_fmapp_bgd_lz_ncgr,
    RES_SCR_M_ID = NARC_fmission_app_fmapp_bgd03_lz_nscr,

  };

	ARCHANDLE	*handle;

	handle	= GFL_ARC_OpenDataHandle( RES_ARC, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_M_ID, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, RES_PAL_M_SIZE, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_S_ID, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_S_SIZE, heapID );
	
  //	----- SUB -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_S_ID, BG_FRAME_BACK_S, 0, 0, TRUE, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_S_ID,	BG_FRAME_BACK_S, 0, 0, TRUE, heapID );	

	//	----- MAIN -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	handle, RES_CHR_M_ID, BG_FRAME_BACK_M, 0, 0, TRUE, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(      handle, RES_SCR_M_ID, BG_FRAME_BACK_M, 0, 0, TRUE, heapID );		

	GFL_ARC_CloseDataHandle( handle );
}





#ifdef FMISSION_INFOWIN
//=============================================================================
/**
 *		INFOWIN
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	INFOWIN初期化
 *
 *	@param	GAMESYS_WORK *gsys	ゲームシステム
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void FMissionApp_INFOWIN_Init( GAMESYS_WORK *gsys, HEAPID heapID )
{	
  GAMEDATA *gamedata;
  
  GF_ASSERT( gsys );

  gamedata = GAMESYSTEM_GetGameData(gsys);
	INFOWIN_Init( BG_FRAME_BAR_M, PLTID_BG_INFOWIN_M, gamedata, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	INFOWIN破棄
 *
 *	@param	void 
 */
//-----------------------------------------------------------------------------
static void FMissionApp_INFOWIN_Exit( void )
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
static void FMissionApp_INFOWIN_Main( void )
{	
	INFOWIN_Update();
}
#endif //FMISSION_INFOWIN

#ifdef FMISSION_TOUCHBAR
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
static TOUCHBAR_WORK * FMissionApp_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heapID )
{	
	//アイコンの設定
	//数分作る
	TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
	{	
		{	
			TOUCHBAR_ICON_RETURN,
			{	TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
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
static void FMissionApp_TOUCHBAR_Exit( TOUCHBAR_WORK	*touchbar )
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
static void FMissionApp_TOUCHBAR_Main( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}
#endif //FMISSION_TOUCHBAR

#ifdef FMISSION_MCSS
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
static MCSS_SYS_WORK * FMissionApp_MCSS_Init( u16 wk_max, HEAPID heapID )
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
static void FMissionApp_MCSS_Exit( MCSS_SYS_WORK * mcss )
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
static void FMissionApp_MCSS_Draw( MCSS_SYS_WORK * mcss )
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
static MCSS_WORK * FMissionApp_MCSS_CreateWkPP( MCSS_SYS_WORK * mcss, POKEMON_PARAM *pp, const VecFx32 *pos )
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
static void FMissionApp_MCSS_DeleteWk( MCSS_SYS_WORK * mcss, MCSS_WORK *wk )
{	
	MCSS_SetVanishFlag( wk );
	MCSS_Del(mcss,wk);
}
#endif //FMISSION_MCSS

#ifdef FMISSION_TASKMENU
//----------------------------------------------------------------------------
/**
 *	@brief	TASKMENUの初期化
 *
 *	@param	menu_res	リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * FMissionApp_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID )
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
static void FMissionApp_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
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
static void FMissionApp_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{	
	APP_TASKMENU_UpdateMenu( menu );
}
#endif //FMISSION_TASKMENU

#ifdef FMISSION_POKE_ICON
//=============================================================================
/**
 *	POKE_ICON
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  ポケアイコン 生成
 *
 *	@param	FMISSION_APP_WORK *wk ワーク
 *	@param	unit  ユニット
 *	@param	heapID  ヒープID  
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void FMissionApp_POKE_ICON_CreateCLWK( FMISSION_APP_WORK* wk, u32 mons, u32 form_no, u32 sex, BOOL egg, GFL_CLUNIT* unit, HEAPID heapID )
{
  // ポケアイコン本体
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NCLR;
    prm.arc_id    = ARCID_POKEICON;
    prm.pltt_id   = POKEICON_GetPalArcIndex();
  //  prm.ncg_id    = POKEICON_GetCgxArcIndex(ppp);
    prm.ncg_id    = POKEICON_GetCgxArcIndexByMonsNumber( mons, form_no, sex, egg );
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
      u8 pal_num = POKEICON_GetPalNum( mons, form_no, sex, egg );
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

    // アニメシーケンスで指定( 0=どうぐ, 1=メール )
    // ※位置調整はとりあえずの値です。
    wk->clwk_poke_item = UI_EASY_CLWK_CreateCLWK( &wk->clres_poke_item, unit, 48+1*8+4, 32+4, 0, heapID );
  }

}

//-----------------------------------------------------------------------------
/**
 *	@brief  ポケアイコン 破棄
 *
 *	@param	FMISSION_APP_WORK *wk ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void FMissionApp_POKE_ICON_DeleteCLWK( FMISSION_APP_WORK* wk )
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

#endif //FMISSION_POKE_ICON

#ifdef FMISSION_POKE2D
//=============================================================================
/**
 *	ポケモンOBJ,BG読みこみ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンBG読みこみ
 *
 *	@param	FMISSION_APP_WORK *wk	ワーク
 *	@param	heapID										ヒープID
 */
//-----------------------------------------------------------------------------
static void FMissionApp_POKE2D_LoadResourceBG( FMISSION_APP_WORK *wk, HEAPID heapID )
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
 *	@param	FMISSION_APP_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
static void FMissionApp_POKE2D_UnLoadResourceBG( FMISSION_APP_WORK *wk )
{		
	GFL_BG_FillCharacterRelease( BG_FRAME_POKE_M, 1, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief	リソース読みこみ
 *
 *	@param	FMISSION_APP_WORK *wk	ワーク
 *	@param	heapID										ヒープID
 */
//-----------------------------------------------------------------------------
static void FMissionApp_POKE2D_LoadResourceOBJ( FMISSION_APP_WORK *wk, HEAPID heapID )
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
 *	@param	FMISSION_APP_WORK *wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void FMissionApp_POKE2D_UnLoadResourceOBJ( FMISSION_APP_WORK *wk )
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
 *	@param	FMISSION_APP_WORK *wk	ワーク
 *	@param	*clunit	ユニット
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void FMissionApp_POKE2D_CreateCLWK( FMISSION_APP_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
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
 *	@param	FMISSION_APP_WORK *wk ワーク
 */
//-----------------------------------------------------------------------------
static void FMissionApp_POKE2D_DeleteCLWK( FMISSION_APP_WORK *wk )
{	
	GFL_CLACT_WK_Remove( wk->clwk_poke2d );
}
#endif //FMISSION_POKE2D

#ifdef	FMISSION_PRINT_TOOL
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
static BOOL PrintTool_MainFunc( FMISSION_APP_WORK * wk )
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
static void PrintTool_AddBmpWin( FMISSION_APP_WORK * wk )
{
	wk->print_util.win = GFL_BMPWIN_Create(
													GFL_BG_FRAME0_M,					// ＢＧフレーム
													1, 1,											// 表示座標
													20, 2,										// 表示サイズ
													15,												// パレット
													GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
}

// BMPWIN削除
static void PrintTool_DelBmpWin( FMISSION_APP_WORK * wk )
{
		GFL_BMPWIN_Delete( wk->print_util.win );
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( FMISSION_APP_WORK * wk )
{
	GFL_BMPWIN_MakeScreen( wk->print_util.win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util.win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( FMISSION_APP_WORK * wk )
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
#endif	//FMISSION_PRINT_TOOL

#ifdef FMISSION_PMSDRAW

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 初期化処理
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void FMissionApp_PMSDRAW_Init( FMISSION_APP_WORK* wk )
{
	GFL_CLUNIT	*clunit;
  
  clunit = FMISSION_APP_GRA_GetClunit( wk->graphic );

  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_SUB, wk->print_que, wk->font, 
      PLTID_OBJ_PMS_DRAW, FMISSION_PMSDRAW_NUM ,wk->heapID );
  
  {
    int i;
    PMS_DATA pms;

    // PMS表示用BMPWIN生成
    for( i=0; i<FMISSION_PMSDRAW_NUM; i++ )
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
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void FMissionApp_PMSDRAW_Exit( FMISSION_APP_WORK* wk )
{
  PMS_DRAW_Exit( wk->pms_draw );
  {
    int i;
    for( i=0; i<FMISSION_PMSDRAW_NUM; i++ )
    {
      GFL_BMPWIN_Delete( wk->pms_win[i] );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 主処理
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void FMissionApp_PMSDRAW_Proc( FMISSION_APP_WORK* wk )
{
#if 1
  // SELECTでクリア
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    int i;
    for( i=0; i<FMISSION_PMSDRAW_NUM; i++ )
    {
      PMS_DRAW_Clear( wk->pms_draw, i, TRUE );
    }
  }
  // STARTでランダム挿入（二重登録するとアサート）
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    int i;
    for( i=0; i<FMISSION_PMSDRAW_NUM; i++ )
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

#endif // FMISSION_PMSDRAW

