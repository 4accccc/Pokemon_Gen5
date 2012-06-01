//=============================================================================
/**
 *
 *	@file		fes_rom.c
 *	@brief  フェスミッション配信アプリケーション 
 *	@author	Miyuki Iwasawa
 *	@data		2012.03.29
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

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "fes_rom_gra.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//マップモデルのnsbtx
#include "fieldmap/fldmmdl_mdlres.naix"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//ポケモンBG,OBJ読みこみ
#include "system/poke2dgra.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "fes_rom.h"

#include "message.naix"
#include "msg/msg_beacon_detail.h"  // GMM
#include "config_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "wifi_unionobj.naix"
#include "wmi.naix"

#include "fes_rom_local.h"
#include "fes_rom_sub.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
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
static GFL_PROC_RESULT FesRomProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT FesRomProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT FesRomProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );


static int seq_BaseMain( FES_ROM_WORK* wk );
static int seq_MEditMain( FES_ROM_WORK* wk );
static int seq_SendMain( FES_ROM_WORK* wk );
static int seq_TaskWait( FES_ROM_WORK* wk );
static int seq_InitialDraw( FES_ROM_WORK* wk );
static int seq_FadeIn( FES_ROM_WORK* wk );
static int seq_FadeOut( FES_ROM_WORK* wk );

//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void _sub_DataSetup(FES_ROM_WORK* wk);
static void _sub_DataExit(FES_ROM_WORK* wk);
static void _sub_SystemSetup( FES_ROM_WORK* wk);
static void _sub_SystemExit( FES_ROM_WORK* wk);
static void _sub_BGResInit( FES_ROM_WORK* wk, HEAPID heapID );
static void _sub_BGResRelease( FES_ROM_WORK* wk );
static void _sub_ActorCreate(FES_ROM_WORK* wk);
static void _sub_ActorDelete(FES_ROM_WORK* wk);
static void _sub_BmpWinCreate(FES_ROM_WORK* wk);
static void _sub_BmpWinDelete(FES_ROM_WORK* wk);

//-------------------------------------
/// アクター関連	
//=====================================
static void obj_ObjResInit( FES_ROM_WORK* wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* src, ARCHANDLE* p_handle );
static void obj_ObjResRelease( FES_ROM_WORK* wk, OBJ_RES_TBL* res );
static void _sub_ActorResourceLoad( FES_ROM_WORK* wk, ARCHANDLE *handle);
static void _sub_ActorResourceUnload( FES_ROM_WORK* wk );


//-------------------------------------
///	簡易会話表示
//=====================================
static void _sub_BeaconWinInit( FES_ROM_WORK* wk );
static void _sub_BeaconWinProc( FES_ROM_WORK* wk );
static void _sub_BeaconWinExit( FES_ROM_WORK* wk );


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA FesRomProcData = 
{
	FesRomProc_Init,
	FesRomProc_Main,
	FesRomProc_Exit,
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
 *	@param	*pwk					FES_ROM_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FesRomProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	FES_ROM_WORK *wk;
	
  //オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_FES_ROM, FES_ROM_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(FES_ROM_WORK), HEAPID_FES_ROM );
  GFL_STD_MemClear( wk, sizeof(FES_ROM_WORK) );
	
  //引数取得
	wk->sys_param	= pwk;
	wk->sys_wk = wk->sys_param->sys;
  wk->save_wk = &wk->sys_wk->save_wk;
  wk->sys_save = &wk->sys_wk->save_wk.save;

  wk->app_mode = wk->sys_param->mode;

  //呼び出しモード補正
  if( wk->app_mode == FESROM_APP_MODE_BASE ){
    if( GFL_UI_KEY_GetCont() != ( PAD_BUTTON_R|PAD_BUTTON_L|PAD_BUTTON_X) &&
        wk->sys_wk->save_enable_f &&
        wk->sys_wk->ret_mode == FESROM_APP_RET_NONE){
      //送信モードから始める
      wk->app_mode = FESROM_APP_MODE_SEND;
    }
  }
  if( wk->sys_wk->init_pos >= 0 ){
    wk->pos = wk->sys_wk->init_pos;
  } 
  wk->initial_f = TRUE; //初回呼び出しフラグ
  OS_Printf("AppMode=%d\n",wk->app_mode);

  // 初期化
  wk->heapID = HEAPID_FES_ROM;
  wk->tmpHeapID = GFL_HEAP_LOWID( HEAPID_FES_ROM );
	
	//描画設定初期化
	wk->graphic	= FES_ROM_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, wk->heapID );

  _sub_DataSetup( wk );
  _sub_SystemSetup( wk );
	_sub_BGResInit( wk, wk->heapID );
  _sub_BmpWinCreate( wk );
  _sub_ActorResourceLoad( wk, wk->handle);
  _sub_ActorCreate( wk );
  //ウィンドウ初期化
  _sub_BeaconWinInit( wk );
  
  OS_Printf("AppMode=%d\n",wk->app_mode);
  wk->sys_wk->init_pos = -1;
  wk->sys_wk->ret_mode = FESROM_APP_RET_NONE;

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					FES_ROM_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FesRomProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	FES_ROM_WORK* wk = mywk;

  _sub_BeaconWinExit( wk );
  _sub_ActorDelete( wk );
  _sub_ActorResourceUnload( wk );
  _sub_BmpWinDelete( wk );
	_sub_BGResRelease( wk );
  _sub_SystemExit( wk );
  _sub_DataExit( wk );

	//描画設定破棄
	FES_ROM_GRAPHIC_Exit( wk->graphic );

	//PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_FES_ROM );

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
 *	@param	*pwk					FES_ROM_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FesRomProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	FES_ROM_WORK* wk = mywk;

  switch(*seq){
  case SEQ_INI_DRAW:
    *seq = seq_InitialDraw( wk );
    break;
  case SEQ_FADEIN:
    *seq = seq_FadeIn( wk );
    break;
  case SEQ_BASE_MAIN:
    *seq = seq_BaseMain( wk );
    break;
  case SEQ_MEDIT_MAIN:
    *seq = seq_MEditMain( wk );
    break;
  case SEQ_SEND_MAIN:
    *seq = seq_SendMain( wk );
    break;
  case SEQ_TASK_WAIT:
    *seq = seq_TaskWait( wk );
    break;
  case SEQ_FADEOUT:
    *seq = seq_FadeOut( wk );
    break;
  case SEQ_EXIT:
    return GFL_PROC_RES_FINISH;
  }
  GFL_TCBL_Main( wk->pTcbSys );

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );
  _sub_BeaconWinProc( wk );

	//2D描画
	FES_ROM_GRAPHIC_2D_Draw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *								global関数
 */
//=============================================================================
void FES_ROM_SetFadeInOutSeq( FES_ROM_WORK* wk, int fadein_seq, int fadeout_seq )
{
  if( fadein_seq < SEQ_MAX ){
    wk->fadein_seq = fadein_seq;
  }
  if( fadeout_seq < SEQ_MAX ){
    wk->fadeout_seq = fadeout_seq;
  }
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

/*
 *  @brief  ベースメイン
 */
static int seq_BaseMain( FES_ROM_WORK* wk )
{
  return FesRom_InputCheckBase( wk );
}

/*
 *  @brief  ミッション編集メイン
 */
static int seq_MEditMain( FES_ROM_WORK* wk )
{
  return FesRom_InputCheckMEdit( wk );
}

/*
 *  @brief  ミッション送信メイン
 */
static int seq_SendMain( FES_ROM_WORK* wk )
{
  return FesRom_InputCheckSend( wk );
}

/*
 *  @brief  タスク待ち
 */
static int seq_TaskWait( FES_ROM_WORK* wk )
{
  if( wk->task_ct ){
    return SEQ_TASK_WAIT;
  }
  return wk->ret_seq;
}

/*
 *  @brief  初期描画
 */
static int seq_InitialDraw( FES_ROM_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    FesRom_InitialDraw( wk );
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
static int seq_FadeIn( FES_ROM_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    wk->seq++;
    break;
  case 1:
    if( !GFL_FADE_CheckFade() ){	
      wk->seq = 0;
			return wk->fadein_seq;
		}
    break;
  }
  return SEQ_FADEIN; 
}

/*
 *  @brief  フェードアウト
 */
static int seq_FadeOut( FES_ROM_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    wk->seq++;
    break;
  case 1:
    if( !GFL_FADE_CheckFade() ){	
      wk->seq = 0;
			return wk->fadeout_seq;
		}
    break;
  }
  return SEQ_FADEOUT; 
}

////////////////////////////////////////////////////////////////////////////////////
/***********************************************************************************
 *
 ***********************************************************************************/
////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------
/**
 * データ関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_DataSetup(FES_ROM_WORK* wk)
{
}

//--------------------------------------------------------------
/**
 * データ関連の破棄＆終了処理
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_DataExit(FES_ROM_WORK* wk)
{
}

//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_SystemSetup( FES_ROM_WORK* wk)
{
  wk->pTcbSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 64, 128 );
  
  //フェスミッションデータマネージャ
  wk->fm_data_man = FMISSION_DATA_Init( wk->heapID );
  //フェスミッションアーカイブ取得
  wk->fm_arc_tbl = FMISSION_DATA_AllocArcAll( wk->fm_data_man, wk->heapID );

  //グラフィックハンドル取得
  wk->handle	= GFL_ARC_OpenDataHandle( ARCID_CONFIG_GRA, wk->heapID );

  //メッセージ関連
//  GFL_FONTSYS_SetColor( FCOL_POPUP_MAIN, FCOL_POPUP_SDW, FCOL_POPUP_BASE );
  GFL_FONTSYS_SetDefaultColor();
	
  wk->print_que = PRINTSYS_QUE_CreateEx( 2048, wk->heapID );
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  wk->wset = WORDSET_CreateEx( WORDSET_FES_ROM_SETNUM, WORDSET_FES_ROM_BUFLEN, wk->heapID );
	
  //メッセージデータ
	wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			        NARC_message_fes_rom_dat, wk->heapID );

  wk->str_tmp = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  wk->str_expand = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
  wk->str_popup = GFL_STR_CreateBuffer( BUFLEN_TMP_MSG, wk->heapID );
}

//--------------------------------------------------------------
/**
 * システム関連の破棄
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_SystemExit( FES_ROM_WORK* wk)
{
  GFL_STR_DeleteBuffer(wk->str_popup);
  GFL_STR_DeleteBuffer(wk->str_expand);
  GFL_STR_DeleteBuffer(wk->str_tmp);

  GFL_MSG_Delete(wk->msgman);
	WORDSET_Delete(wk->wset);
  
  GFL_FONT_Delete(wk->font);
  PRINTSYS_QUE_Clear(wk->print_que);
  PRINTSYS_QUE_Delete(wk->print_que);
  GFL_FONTSYS_SetDefaultColor();
 
  GFL_TCBL_Exit( wk->pTcbSys );
	
  GFL_ARC_CloseDataHandle( wk->handle );
  
  GFL_HEAP_FreeMemory( wk->fm_arc_tbl );
  FMISSION_DATA_Release( wk->fm_data_man );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	FES_ROM_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void _sub_BGResInit( FES_ROM_WORK* wk, HEAPID heapID )
{
	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( wk->handle, NARC_config_gra_est_uegamen_01_NCLR, PALTYPE_MAIN_BG, 0, 0x200, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( wk->handle, NARC_config_gra_est_uegamen_01_NCLR, PALTYPE_SUB_BG, 0, 0x200, heapID );
  
  //	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	wk->handle, NARC_config_gra_est_uegamen_01_NCGR,
						BG_FRAME_BACK_M, 0, 0, FALSE, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	wk->handle, NARC_config_gra_est_uegamen_01_NSCR,
						BG_FRAME_BACK_M, 0, 0, FALSE, heapID );		

	//	----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	wk->handle, NARC_config_gra_est_uegamen_01_NCGR,
						BG_FRAME_BACK_S, 0, 0, FALSE, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	wk->handle, NARC_config_gra_est_uegamen_01_NSCR,
						BG_FRAME_BACK_S, 0, 0, FALSE, heapID );		

	GFL_BG_LoadScreenReq( BG_FRAME_BACK_M );
	GFL_BG_LoadScreenReq( BG_FRAME_BACK_S );

  //ウィンドウカラー再転送用パレット
  wk->resPlttPanel.buf = GFL_ARC_LoadDataAllocByHandle( wk->handle, 
                              NARC_config_gra_est_bar_01_NCLR, wk->heapID );

  NNS_G2dGetUnpackedPaletteData( wk->resPlttPanel.buf, &wk->resPlttPanel.p_pltt );
  wk->resPlttPanel.dat = (u16*)wk->resPlttPanel.p_pltt->pRawData;

//  G2_SetBlendAlpha( ALPHA_1ST_M, ALPHA_2ND_M, ALPHA_EV1_M, ALPHA_EV2_M);
//  G2S_SetBlendAlpha( ALPHA_1ST_S, ALPHA_2ND_S, ALPHA_EV1_S, ALPHA_EV2_S);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース解放
 *
 *	@param	FES_ROM_WORK* wk 管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void _sub_BGResRelease( FES_ROM_WORK* wk )
{
  GFL_HEAP_FreeMemory( wk->resPlttPanel.buf );
  G2S_BlendNone();
  G2_BlendNone();
}

//--------------------------------------------------------------
/**
 * アクターリソース取得
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void obj_ObjResInit( FES_ROM_WORK* wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* src, ARCHANDLE* p_handle )
{
  MI_CpuClear8( res, sizeof(OBJ_RES_TBL));

  res->pltt = GFL_CLGRP_PLTT_RegisterEx(  p_handle, src->pltt_id, src->type, src->pltt_ofs*0x20, 0, src->pltt_siz, wk->heapID );
  res->cgr = GFL_CLGRP_CGR_Register( p_handle, src->cgr_id, FALSE, src->type, wk->heapID );
  res->cell = GFL_CLGRP_CELLANIM_Register( p_handle, src->cell_id, src->cell_anm_id, wk->heapID );
}

//--------------------------------------------------------------
/**
 * アクターリソース解放
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void obj_ObjResRelease( FES_ROM_WORK* wk, OBJ_RES_TBL* res )
{
  GFL_CLGRP_CELLANIM_Release( res->cell );
  GFL_CLGRP_CGR_Release( res->cgr );
  GFL_CLGRP_PLTT_Release( res->pltt );
}

//--------------------------------------------------------------
/**
 * アクターで使用するリソースのロード
 *
 * @param   view		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _sub_ActorResourceLoad( FES_ROM_WORK* wk, ARCHANDLE *handle )
{
  int i;

  { //ノーマル
    const OBJ_RES_SRC srcNormal = {
      CLSYS_DRAW_MAX, PLTID_OBJ_NORMAL_M, 3,
      NARC_config_gra_est_obj_01_NCLR,
      NARC_config_gra_est_obj_01_NCGR,
      NARC_config_gra_est_obj_01_NCER,
      NARC_config_gra_est_obj_01_NANR,
    };
    obj_ObjResInit( wk, &wk->objResNormal, &srcNormal, handle );
  }
  { //ユニオン
    const OBJ_RES_SRC srcUnion = {
      CLSYS_DRAW_MAIN, PLTID_OBJ_UNION_M, 1,
      NARC_wifi_unionobj_wifi_union_obj_NCLR,
      NARC_wifi_unionobj_front00_NCGR,
      NARC_wifi_unionobj_front00_NCER,
      NARC_wifi_unionobj_front00_NANR,
    };
    ARCHANDLE* h_union = GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, wk->tmpHeapID );

    obj_ObjResInit( wk, &wk->objResUnion, &srcUnion, h_union );
   
    //再転送用のリソースを確保
    wk->resPlttUnion.buf = GFL_ARC_LoadDataAllocByHandle( h_union,
                              NARC_wifi_unionobj_wifi_union_obj_NCLR, wk->heapID );

    NNS_G2dGetUnpackedPaletteData( wk->resPlttUnion.buf, &wk->resPlttUnion.p_pltt );
    wk->resPlttUnion.dat = (u16*)wk->resPlttUnion.p_pltt->pRawData;
  
    for(i = 0;i < UNION_CHAR_MAX;i++){
      wk->resCharUnion[i].buf = GFL_ARC_LoadDataAllocByHandle( h_union,
                              NARC_wifi_unionobj_front00_NCGR+i,
                              wk->heapID );
      NNS_G2dGetUnpackedCharacterData( wk->resCharUnion[i].buf, &wk->resCharUnion[i].p_char );
    }
    GFL_ARC_CloseDataHandle( h_union );
  }
}

//--------------------------------------------------------------
/**
 * アクターで使用するアクターのアンロード
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _sub_ActorResourceUnload( FES_ROM_WORK* wk )
{
  int i;

  for( i = 0;i < UNION_CHAR_MAX;i++){
    GFL_HEAP_FreeMemory( wk->resCharUnion[i].buf );
  }
  GFL_HEAP_FreeMemory( wk->resPlttUnion.buf );

  obj_ObjResRelease( wk, &wk->objResUnion );
  obj_ObjResRelease( wk, &wk->objResNormal );
}

//-----------------------------------------------------------------------------
/*
 *  @brief  BmpWin個別追加
 */
//-----------------------------------------------------------------------------
static void bmpwin_Add( BMP_WIN* win, u8 frm, u8 pal, u8 px, u8 py, u8 sx, u8 sy )
{
  win->win = GFL_BMPWIN_Create( frm, px, py, sx, sy, pal, GFL_BMP_CHRAREA_GET_B );
  win->bmp = GFL_BMPWIN_GetBmp( win->win );
  PRINT_UTIL_Setup( &win->putil, win->win );
  GFL_BMP_Clear( win->bmp, 0 );
  GFL_BMPWIN_MakeTransWindow( win->win );
}

//-----------------------------------------------------------------------------
/*
 *  @brief  アクター個別追加
 */
static GFL_CLWK* act_Add(
   FES_ROM_WORK* wk, GFL_CLUNIT* unit, OBJ_RES_TBL* res,
  s16 x, s16 y, u8 anm, u8 spri, u8 bg_pri, u16 surface )
{
  GFL_CLWK_DATA ini;
  GFL_CLWK* obj;

  //セルの生成
  ini.pos_x = x;
  ini.pos_y = y;
  ini.anmseq = anm;
  ini.bgpri = bg_pri;
  ini.softpri = spri;
  
  obj = GFL_CLACT_WK_Create( unit,
          res->cgr,res->pltt,res->cell, &ini, surface, wk->heapID );

  return obj;
}

//--------------------------------------------------------------
/**
 * システムアクター作成
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _sub_ActorCreate(FES_ROM_WORK* wk)
{
	GFL_CLUNIT	*clunit;
  
  clunit = FES_ROM_GRAPHIC_GetClunit( wk->graphic );

  //初期座標は適当
  wk->pAct[ACT_ICON_TR] = act_Add( wk, clunit, &wk->objResUnion,
                    128, 120, 0, 1, ACT_ICON_BGPRI, ACT_SF_MAIN );
  GFL_CLACT_WK_SetPlttOffs( wk->pAct[ACT_ICON_TR], 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );
  GFL_CLACT_WK_SetDrawEnable( wk->pAct[ACT_ICON_TR], FALSE );
}

//--------------------------------------------------------------
/**
 * システムアクター削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_ActorDelete(FES_ROM_WORK* wk)
{
  int i;

  for(i = 0;i < ACT_MAX;i++){
    if( wk->pAct[i] == NULL ) continue;
    GFL_CLACT_WK_SetDrawEnable( wk->pAct[i], FALSE );
    GFL_CLACT_WK_Remove( wk->pAct[i] );
  }
}

//--------------------------------------------------------------
/**
 * TARGET_WIN作成
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _sub_TargetWinCreate(FES_ROM_WORK* wk)
{
  int i;

  for( i = 0;i < TARGET_VIEW_MAX; i++ ){
    bmpwin_Add( &wk->target[i].base,
      BG_FRAME_BASE_M+(i*4), BMP_BASE_PAL, BMP_BASE_PX, BMP_BASE_PY,BMP_BASE_SX, BMP_BASE_SY );
    bmpwin_Add( &wk->target[i].input,
      BG_FRAME_WIN_M+(i*4), BMP_INPUT_PAL, BMP_INPUT_PX, BMP_INPUT_PY,BMP_INPUT_SX, BMP_INPUT_SY );
    bmpwin_Add( &wk->target[i].msg,
      BG_FRAME_MSG_M+(i*4), BMP_MSG_PAL, BMP_MSG_PX, BMP_MSG_PY,BMP_MSG_SX, BMP_MSG_SY );
  }
}

//--------------------------------------------------------------
/**
 * TARGET_WIN削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_TargetWinDelete(FES_ROM_WORK* wk)
{
  int i;
  
  for( i = 0;i < TARGET_VIEW_MAX; i++ ){
    GFL_BMPWIN_Delete( wk->target[i].base.win );
    GFL_BMPWIN_Delete( wk->target[i].input.win );
    GFL_BMPWIN_Delete( wk->target[i].msg.win );
  }
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _sub_BmpWinCreate(FES_ROM_WORK* wk)
{
  _sub_TargetWinCreate( wk );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_BmpWinDelete(FES_ROM_WORK* wk)
{
  _sub_TargetWinDelete(wk);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 初期化処理
 *
 *	@param	FES_ROM_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _sub_BeaconWinInit( FES_ROM_WORK* wk )
{
	GFL_CLUNIT	*clunit;
  
  clunit = FES_ROM_GRAPHIC_GetClunit( wk->graphic );

#if 0
  //簡易会話描画初期化
  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_SUB, wk->print_que, wk->font, 
      PLTID_BG_PMS_S, BEACON_WIN_MAX ,wk->heapID );
  PMS_DRAW_SetNullColorPallet( wk->pms_draw, FCOL_WIN_BASE2);
  PMS_DRAW_SetPrintColor( wk->pms_draw, FCOL_WIN02 );
  PMS_DRAW_SetCLWKAutoScrollFlag( wk->pms_draw, TRUE ); 

  {
    int i,j,frm,pal;
    PMS_DATA pms;
    BEACON_WIN* bp;
    
    // PMS表示用BMPWIN生成
    for( i=0; i < BEACON_WIN_MAX; i++ )
    {
      frm = BMP_BEACON_FRM+i;
      
      bp = &wk->beacon_win[i];
      bp->frame = BG_FRAME_WIN01_S+i;

      bp->pms = GFL_BMPWIN_Create(
                  frm, BMP_PMS_PX, BMP_PMS_PY, BMP_PMS_SX, BMP_PMS_SY,
                  BMP_PMS_PAL+i, GFL_BMP_CHRAREA_GET_B );

      for(j = 0;j < BEACON_PROF_MAX;j++){
        bmpwin_Add( &bp->prof[j], frm,
            BMP_PROF_PAL+i, BMP_PROF_PX, BMP_PROF_PY+BMP_PROF_SY*j, BMP_PROF_SX, BMP_PROF_SY );
      }
      bmpwin_Add( &bp->home[0], frm,
        BMP_HOME01_PAL+i, BMP_HOME01_PX, BMP_HOME01_PY, BMP_HOME01_SX, BMP_HOME01_SY );
      bmpwin_Add( &bp->home[1], frm,
        BMP_HOME02_PAL+i, BMP_HOME02_PX, BMP_HOME02_PY, BMP_HOME02_SX, BMP_HOME02_SY );

      bmpwin_Add( &bp->record, frm,
          BMP_RECORD_PAL+i, BMP_RECORD_PX, BMP_RECORD_PY, BMP_RECORD_SX, BMP_RECORD_SY );
    
      bp->cTrainer = act_Add( wk, clunit, &wk->objResTrainer[i],
                        ACT_TRAINER_PX, ACT_TRAINER_PY, 0, 1, ACT_TRAINER_BGPRI+i, ACT_SF_SUB );
      GFL_CLACT_WK_SetObjMode( bp->cTrainer, GX_OAM_MODE_NORMAL );
      GFL_CLACT_WK_SetDrawEnable( bp->cTrainer, FALSE );
      bp->cRank = act_Add( wk, clunit, &wk->objResNormal,
                        ACT_RANK_PX, ACT_RANK_PY, ACTANM_RANK05, 0, ACT_TRAINER_BGPRI+i, ACT_SF_SUB );
      GFL_CLACT_WK_SetObjMode( bp->cRank, GX_OAM_MODE_XLU );
      GFL_CLACT_WK_SetDrawEnable( bp->cRank, FALSE );
    }
  }
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 後処理
 *
 *	@param	FES_ROM_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _sub_BeaconWinExit( FES_ROM_WORK* wk )
{
  int i,j;

#if 0
  for( i=0; i < BEACON_WIN_MAX; i++ )
  {
    BEACON_WIN* bp;
    bp = &wk->beacon_win[i];

    GFL_CLACT_WK_SetDrawEnable( bp->cTrainer, FALSE );
    GFL_CLACT_WK_Remove( bp->cTrainer);
    GFL_CLACT_WK_SetDrawEnable( bp->cRank, FALSE );
    GFL_CLACT_WK_Remove( bp->cRank);

    GFL_BMPWIN_Delete( bp->record.win );
    for(j = 0;j < BEACON_HOME_MAX;j++){
      GFL_BMPWIN_Delete( bp->home[j].win );
    }
    for(j = 0;j < BEACON_PROF_MAX;j++){
      GFL_BMPWIN_Delete( bp->prof[j].win );
    }
    GFL_BMPWIN_Delete( bp->pms );
  }
  PMS_DRAW_Exit( wk->pms_draw );
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 主処理
 *
 *	@param	FES_ROM_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _sub_BeaconWinProc( FES_ROM_WORK* wk )
{
#if 0
  // SELECTでクリア
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    int i;
    for( i=0; i<BEACON_DETAIL_PMSDRAW_NUM; i++ )
    {
      PMS_DRAW_Clear( wk->pms_draw, i, TRUE );
    }
  }
  // STARTでランダム挿入（二重登録するとアサート）
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    int i;
    for( i=0; i<BEACON_DETAIL_PMSDRAW_NUM; i++ )
    {
      PMS_DATA pms;
      PMSDAT_SetDebugRandom( &pms );
      PMSDAT_SetDeco( &pms, 0, GFUser_GetPublicRand(10)+1 );
      PMS_DRAW_Print( wk->pms_draw, wp->pms, &pms ,i );
    }
  }
#endif
//  PMS_DRAW_Main( wk->pms_draw );
}


