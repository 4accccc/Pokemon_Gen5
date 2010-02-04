//=============================================================================
/**
 *
 *	@file		beacon_detail.c
 *	@brief  すれ違い詳細画面
 *	@author	Miyuki Iwasawa
 *	@data		2010.02.01
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
#include "beacon_detail_gra.h"

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
#include "app/beacon_detail.h"

#include "message.naix"
#include "msg/msg_beacon_detail.h"  // GMM
#include "beacon_status.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "townmap_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース
#include "wifi_unionobj.naix"
#include "wmi.naix"

#include "beacon_detail_local.h"
#include "beacon_detail_sub.h"

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
static GFL_PROC_RESULT BeaconDetailProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT BeaconDetailProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT BeaconDetailProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );


static int seq_Main( BEACON_DETAIL_WORK* wk );
static int seq_FadeIn( BEACON_DETAIL_WORK* wk );
static int seq_FadeOut( BEACON_DETAIL_WORK* wk );

//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void _sub_DataSetup(BEACON_DETAIL_WORK* wk);
static void _sub_DataExit(BEACON_DETAIL_WORK* wk);
static void _sub_SystemSetup( BEACON_DETAIL_WORK* wk);
static void _sub_SystemExit( BEACON_DETAIL_WORK* wk);
static void _sub_BGResInit( BEACON_DETAIL_WORK* wk, HEAPID heapID );
static void _sub_BGResRelease( BEACON_DETAIL_WORK* wk );
static void _sub_BmpWinCreate(BEACON_DETAIL_WORK* wk);
static void _sub_BmpWinDelete(BEACON_DETAIL_WORK* wk);

//-------------------------------------
///	タッチバー
//=====================================
static TOUCHBAR_WORK * _sub_TouchBarInit( GFL_CLUNIT *clunit, HEAPID heapID );
static void _sub_TouchBarExit( TOUCHBAR_WORK	*touchbar );
static void _sub_TouchBarMain( TOUCHBAR_WORK	*touchbar );

//-------------------------------------
/// アクター関連	
//=====================================
static void obj_ObjResInit( BEACON_DETAIL_WORK* wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* src, ARCHANDLE* p_handle );
static void obj_ObjResRelease( BEACON_DETAIL_WORK* wk, OBJ_RES_TBL* res );
static void _sub_ActorResourceLoad( BEACON_DETAIL_WORK* wk, ARCHANDLE *handle);
static void _sub_ActorResourceUnload( BEACON_DETAIL_WORK* wk );


//-------------------------------------
///	簡易会話表示
//=====================================
static void _sub_BeaconWinInit( BEACON_DETAIL_WORK* wk );
static void _sub_BeaconWinExit( BEACON_DETAIL_WORK* wk );
static void _sub_BeaconWinProc( BEACON_DETAIL_WORK* wk );


//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA BeaconDetailProcData = 
{
	BeaconDetailProc_Init,
	BeaconDetailProc_Main,
	BeaconDetailProc_Exit,
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
 *	@param	*pwk					BEACON_DETAIL_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BeaconDetailProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	BEACON_DETAIL_WORK *wk;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BEACON_DETAIL, BEACON_DETAIL_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(BEACON_DETAIL_WORK), HEAPID_BEACON_DETAIL );
  GFL_STD_MemClear( wk, sizeof(BEACON_DETAIL_WORK) );
	
  //引数取得
	wk->param	= pwk;

  // 初期化
  wk->heapID = HEAPID_BEACON_DETAIL;
  wk->tmpHeapID = GFL_HEAP_LOWID( HEAPID_BEACON_DETAIL );
	
	//描画設定初期化
	wk->graphic	= BEACON_DETAIL_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

  _sub_DataSetup( wk );
  _sub_SystemSetup( wk );
	_sub_BGResInit( wk, wk->heapID );
  _sub_BmpWinCreate( wk );
  _sub_ActorResourceLoad( wk, wk->handle);

	//タッチバーの初期化
	wk->touchbar = _sub_TouchBarInit( BEACON_DETAIL_GRAPHIC_GetClunit( wk->graphic ), wk->heapID );

  //ウィンドウ初期化
  _sub_BeaconWinInit( wk );

  //初期描画
  BeaconDetail_InitialDraw( wk );

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					BEACON_DETAIL_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BeaconDetailProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	BEACON_DETAIL_WORK* wk = mywk;

  _sub_BeaconWinExit( wk );
	_sub_TouchBarExit( wk->touchbar );
  _sub_ActorResourceUnload( wk );
  _sub_BmpWinDelete( wk );
	_sub_BGResRelease( wk );
  _sub_SystemExit( wk );
  _sub_DataExit( wk );

	//描画設定破棄
	BEACON_DETAIL_GRAPHIC_Exit( wk->graphic );

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
 *	@param	*pwk					BEACON_DETAIL_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BeaconDetailProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	BEACON_DETAIL_WORK* wk = mywk;

  switch(*seq){
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

	//PRINT_QUE
	PRINTSYS_QUE_Main( wk->print_que );
  _sub_BeaconWinProc( wk );

	//2D描画
	BEACON_DETAIL_GRAPHIC_2D_Draw( wk->graphic );

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
static int seq_Main( BEACON_DETAIL_WORK* wk )
{
  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
  {
    return SEQ_FADEOUT;
  }
	
  //タッチバーメイン処理
	_sub_TouchBarMain( wk->touchbar );

  return SEQ_MAIN;
}

/*
 *  @brief  フェードイン
 */
static int seq_FadeIn( BEACON_DETAIL_WORK* wk )
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
static int seq_FadeOut( BEACON_DETAIL_WORK* wk )
{
  switch( wk->seq ){
  case 0:
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
static void _sub_DataSetup(BEACON_DETAIL_WORK* wk)
{
  int i,max,target;

  //テンポラリを作成
  wk->tmpInfo = GAMEBEACON_Alloc( wk->heapID );

  wk->b_status = GAMEDATA_GetBeaconStatus( wk->param->gdata );
  wk->infoLog = BEACON_STATUS_GetInfoTbl( wk->b_status );

  target = BEACON_STATUS_GetViewTopOffset( wk->b_status );
  max = GAMEBEACON_InfoTblRing_GetEntryNum( wk->infoLog );

  //詳細が有効なデータindexを取得
  for(i = 0;i < max;i++){
    if( GAMEBEACON_Check_NPC( wk->tmpInfo )){
      continue;
    }
    if( target == i ){
      wk->list_top = wk->list_max;
    }
    wk->list[wk->list_max++] = i;
  }

  //メッセージスピード取得
  wk->msg_spd  = MSGSPEED_GetWait();
}

//--------------------------------------------------------------
/**
 * データ関連の破棄＆終了処理
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_DataExit(BEACON_DETAIL_WORK* wk)
{
  GFL_HEAP_FreeMemory( wk->tmpInfo );
}

//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_SystemSetup( BEACON_DETAIL_WORK* wk)
{
  wk->pTcbSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 16, 128 );

  //グラフィックハンドル取得
  wk->handle	= GFL_ARC_OpenDataHandle( ARCID_BEACON_STATUS, wk->heapID );

  //メッセージ関連
  GFL_FONTSYS_SetColor( FCOL_POPUP_MAIN, FCOL_POPUP_SDW, FCOL_POPUP_BASE );
	
  wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	wk->font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
												GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  wk->wset = WORDSET_Create( wk->heapID);
	
  //メッセージデータ
	wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
			        NARC_message_beacon_detail_dat, wk->heapID );

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
static void _sub_SystemExit( BEACON_DETAIL_WORK* wk)
{
  GFL_STR_DeleteBuffer(wk->str_popup);
  GFL_STR_DeleteBuffer(wk->str_expand);
  GFL_STR_DeleteBuffer(wk->str_tmp);

  GFL_MSG_Delete(wk->msg);
	WORDSET_Delete(wk->wset);
  
  GFL_FONT_Delete(wk->font);
  PRINTSYS_QUE_Delete(wk->print_que);
  GFL_FONTSYS_SetDefaultColor();
 
  GFL_TCBL_Exit( wk->pTcbSys );
	
  GFL_ARC_CloseDataHandle( wk->handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	BEACON_DETAIL_BG_WORK* wk BG管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void _sub_BGResInit( BEACON_DETAIL_WORK* wk, HEAPID heapID )
{
  ARCHANDLE* tmap_h;

	tmap_h	= GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( tmap_h, NARC_townmap_gra_tmap_bg_d_NCLR, PALTYPE_MAIN_BG, 0, 0x200, heapID );
	GFL_ARCHDL_UTIL_TransVramPalette( wk->handle, NARC_beacon_status_bdetail_bgu_nclr, PALTYPE_SUB_BG, 0, 0x200, heapID );

	//	----- 上画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	wk->handle, NARC_beacon_status_bdetail_bgu_lz_ncgr,
						BG_FRAME_WIN01_S, 0, 0, TRUE, heapID );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	wk->handle, NARC_beacon_status_bdetail_bgu_lz_ncgr,
						BG_FRAME_WIN02_S, 0, 0, TRUE, heapID );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	wk->handle, NARC_beacon_status_bdetail_bgu_lz_ncgr,
						BG_FRAME_BASE_S, 0, 0, TRUE, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	wk->handle, NARC_beacon_status_bdetail_bgu01_lz_nscr,
						BG_FRAME_WIN01_S, 0, 0, TRUE, heapID );		
	GFL_ARCHDL_UTIL_TransVramScreen(	wk->handle, NARC_beacon_status_bdetail_bgu01_lz_nscr,
						BG_FRAME_WIN02_S, 0, 0, TRUE, heapID );		
	GFL_ARCHDL_UTIL_TransVramScreen(	wk->handle, NARC_beacon_status_bdetail_bgu02_lz_nscr,
						BG_FRAME_BASE_S, 0, 0, TRUE, heapID );		
	
  //	----- 下画面 -----
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	wk->handle, NARC_beacon_status_bdetail_bgd_lz_ncgr,
						BG_FRAME_POPUP_M, 0, 0, TRUE, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	wk->handle, NARC_beacon_status_bdetail_bgd01_lz_nscr,
						BG_FRAME_POPUP_M, 0, 0x800, TRUE, heapID );	
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	tmap_h, NARC_townmap_gra_tmap_bg_d_NCGR,
						BG_FRAME_MAP01_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	tmap_h, NARC_townmap_gra_tmap_root_d_NSCR,
						BG_FRAME_MAP01_M, 0, 0x800, 0, heapID );	
	GFL_ARCHDL_UTIL_TransVramBgCharacter(	tmap_h, NARC_townmap_gra_tmap_map_d_NCGR,
						BG_FRAME_MAP02_M, 0, 0, 0, heapID );
	GFL_ARCHDL_UTIL_TransVramScreen(	tmap_h, NARC_townmap_gra_tmap_map_d_NSCR,
						BG_FRAME_MAP02_M, 0, 0x800, 0, heapID );	

  GFL_ARC_CloseDataHandle( tmap_h );

  G2_SetBlendAlpha( ALPHA_1ST_M, ALPHA_2ND_M, ALPHA_EV1, ALPHA_EV2);
  G2S_SetBlendAlpha( ALPHA_1ST_S, ALPHA_2ND_S, ALPHA_EV1, ALPHA_EV2);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース解放
 *
 *	@param	BEACON_DETAIL_WORK* wk 管理ワーク
 *	@param	heapID  ヒープID 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void _sub_BGResRelease( BEACON_DETAIL_WORK* wk )
{
  G2S_BlendNone();
  G2_BlendNone();
}






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
static TOUCHBAR_WORK * _sub_TouchBarInit( GFL_CLUNIT *clunit, HEAPID heapID )
{	
	//アイコンの設定
	//数分作る
	static const TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
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
	    TOUCHBAR_ICON_CUR_D,	//↓ボタン
			{	TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_Y },
		},
		{	
	    TOUCHBAR_ICON_CUR_U,	//↓ボタン
			{	TOUCHBAR_ICON_X_04, TOUCHBAR_ICON_Y },
		},
	};

	//設定構造体
	//さきほどの窓情報＋リソース情報をいれる
	TOUCHBAR_SETUP	touchbar_setup;
	GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

	touchbar_setup.p_item		= (TOUCHBAR_ITEM_ICON*)touchbar_icon_tbl;				//上の窓情報
	touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
	touchbar_setup.p_unit		= clunit;										//OBJ読み込みのためのCLUNIT
	touchbar_setup.bar_frm	= BG_FRAME_BAR_M;						//BG読み込みのためのBG面
	touchbar_setup.bg_plt		= PLTID_BG_TOUCHBAR_M;			//BGﾊﾟﾚｯﾄ
	touchbar_setup.obj_plt	= PLTID_OBJ_TOUCHBAR_M;			//OBJﾊﾟﾚｯﾄ
	touchbar_setup.mapping	= APP_COMMON_MAPPING_32K;	//マッピングモード

	return TOUCHBAR_Init( &touchbar_setup, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR破棄
 *
 *	@param	TOUCHBAR_WORK	*touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void _sub_TouchBarExit( TOUCHBAR_WORK	*touchbar )
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
static void _sub_TouchBarMain( TOUCHBAR_WORK	*touchbar )
{	
	TOUCHBAR_Main( touchbar );
}

//--------------------------------------------------------------
/**
 * アクターリソース取得
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void obj_ObjResInit( BEACON_DETAIL_WORK* wk, OBJ_RES_TBL* res, const OBJ_RES_SRC* src, ARCHANDLE* p_handle )
{
  MI_CpuClear8( res, sizeof(OBJ_RES_TBL));

  res->pltt = GFL_CLGRP_PLTT_RegisterEx(  p_handle, src->pltt_id, src->type, src->pltt_ofs*0x20, 0, src->pltt_siz, wk->heapID );
  res->cgr = GFL_CLGRP_CGR_Register( p_handle, src->cgr_id, FALSE, src->type, wk->heapID );
  res->cell = GFL_CLGRP_CELLANIM_Register( p_handle, src->cell_id, src->cell_id+1, wk->heapID );
}

//--------------------------------------------------------------
/**
 * アクターリソース解放
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void obj_ObjResRelease( BEACON_DETAIL_WORK* wk, OBJ_RES_TBL* res )
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
static void _sub_ActorResourceLoad( BEACON_DETAIL_WORK* wk, ARCHANDLE *handle )
{
  int i;

  wk->h_trgra = TR2DGRA_OpenHandle( wk->heapID );

  { //ノーマル
    const OBJ_RES_SRC srcNormal = {
      CLSYS_DRAW_MAX, PLTID_OBJ_NORMAL_M, 3,
      NARC_beacon_status_bdetail_obj_nclr,
      NARC_beacon_status_bdetail_obj_ncgr,
      NARC_beacon_status_bdetail_obj_ncer,
    };
    obj_ObjResInit( wk, &wk->objResNormal, &srcNormal, handle );
  }
  { //ユニオン
    const OBJ_RES_SRC srcUnion = {
      CLSYS_DRAW_MAIN, PLTID_OBJ_UNION_M, 1,
      NARC_wifi_unionobj_wifi_union_obj_NCLR,
      NARC_wifi_unionobj_front00_NCGR,
      NARC_wifi_unionobj_front00_NCER,
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
  { //トレーナー
    int i;

    for(i = 0;i < BEACON_WIN_MAX;i++){
      wk->objResTrainer[i].cgr = 
        TR2DGRA_OBJ_CGR_Register( wk->h_trgra, TRTYPE_TANPAN+i, CLSYS_DRAW_SUB, wk->heapID );
      wk->objResTrainer[i].pltt =
        TR2DGRA_OBJ_PLTT_Register( wk->h_trgra, TRTYPE_TANPAN+i, CLSYS_DRAW_SUB,
            (PLTID_OBJ_TRAINER_S+i)*0x20, wk->heapID );
      wk->objResTrainer[i].cell =
        TR2DGRA_OBJ_CELLANM_Register( TRTYPE_TANPAN+i, APP_COMMON_MAPPING_32K, CLSYS_DRAW_SUB, wk->heapID );
    }
  }
}

//--------------------------------------------------------------
/**
 * アクターで使用するアクターのアンロード
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _sub_ActorResourceUnload( BEACON_DETAIL_WORK* wk )
{
  int i;

  for(i = 0;i < BEACON_WIN_MAX;i++){
    obj_ObjResRelease( wk, &wk->objResTrainer[i] );
  }
  
  for( i = 0;i < UNION_CHAR_MAX;i++){
    GFL_HEAP_FreeMemory( wk->resCharUnion[i].buf );
  }
  GFL_HEAP_FreeMemory( wk->resPlttUnion.buf );

  obj_ObjResRelease( wk, &wk->objResUnion );
  obj_ObjResRelease( wk, &wk->objResNormal );
  
  GFL_ARC_CloseDataHandle( wk->h_trgra );
}

#ifdef	BEACON_DETAIL_PRINT_TOOL
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
static BOOL PrintTool_MainFunc( BEACON_DETAIL_WORK * wk )
{
	switch( wk->sub_seq ){
	case 0:
		PrintTool_AddBmpWin( wk );			// BMPWIN作成
		PrintTool_PrintHP( wk );				// ＨＰ表示
		PrintTool_ScreenTrans( wk );		// スクリーン転送
		wk->sub_seq = 1;
		break;

	case 1:
		// プリント終了待ち
		if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
			wk->sub_seq = 2;
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
static void PrintTool_AddBmpWin( BEACON_DETAIL_WORK * wk )
{
	wk->print_util.win = GFL_BMPWIN_Create(
													GFL_BG_FRAME0_M,					// ＢＧフレーム
													1, 1,											// 表示座標
													20, 2,										// 表示サイズ
													15,												// パレット
													GFL_BMP_CHRAREA_GET_B );	// キャラ取得方向
}

// BMPWIN削除
static void PrintTool_DelBmpWin( BEACON_DETAIL_WORK * wk )
{
		GFL_BMPWIN_Delete( wk->print_util.win );
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( BEACON_DETAIL_WORK * wk )
{
	GFL_BMPWIN_MakeScreen( wk->print_util.win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util.win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( BEACON_DETAIL_WORK * wk )
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
#endif	//BEACON_DETAIL_PRINT_TOOL

//-----------------------------------------------------------------------------
/*
 *  @brief  BmpWin個別追加
 */
static void bmpwin_Add( BMP_WIN* win, u8 frm, u8 pal, u8 px, u8 py, u8 sx, u8 sy )
{
  win->win = GFL_BMPWIN_Create( frm, px, py, sx, sy, pal, GFL_BMP_CHRAREA_GET_B );
  win->bmp = GFL_BMPWIN_GetBmp( win->win );
  PRINT_UTIL_Setup( &win->putil, win->win );
  GFL_BMP_Clear( win->bmp, FCOL_WIN_BASE1 );
  GFL_BMPWIN_MakeTransWindow( win->win );
}

//-----------------------------------------------------------------------------
/*
 *  @brief  アクター個別追加
 */
static GFL_CLWK* act_Add(
   BEACON_DETAIL_WORK* wk, GFL_CLUNIT* unit, OBJ_RES_TBL* res,
  s16 x, s16 y, u8 anm, u8 spri, u8 bg_pri)
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
          res->cgr,res->pltt,res->cell, &ini, CLSYS_DEFREND_SUB, wk->heapID );

  return obj;
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   view		
 */
//--------------------------------------------------------------
static void _sub_BmpWinCreate(BEACON_DETAIL_WORK* wk)
{
  bmpwin_Add( &wk->win_popup,
      BMP_POPUP_FRM, BMP_POPUP_PAL, BMP_POPUP_PX, BMP_POPUP_PY,BMP_POPUP_SX, BMP_POPUP_SY );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_BmpWinDelete(BEACON_DETAIL_WORK* wk)
{
  GFL_BMPWIN_Delete( wk->win_popup.win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 初期化処理
 *
 *	@param	BEACON_DETAIL_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _sub_BeaconWinInit( BEACON_DETAIL_WORK* wk )
{
	GFL_CLUNIT	*clunit;
  
  clunit = BEACON_DETAIL_GRAPHIC_GetClunit( wk->graphic );

  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_SUB, wk->print_que, wk->font, 
      PLTID_BG_PMS_S, BEACON_WIN_MAX ,wk->heapID );
  PMS_DRAW_SetNullColorPallet( wk->pms_draw, FCOL_WIN_BASE2);
  PMS_DRAW_SetPrintColor( wk->pms_draw, FCOL_WIN02 );
  
  {
    int i,j;
    PMS_DATA pms;
    BEACON_WIN* bp;
    
    // PMS表示用BMPWIN生成
    for( i=0; i < BEACON_WIN_MAX; i++ )
    {
      bp = &wk->beacon_win[i];
      bp->pms = GFL_BMPWIN_Create(
                  BMP_BEACON_FRM+i, BMP_PMS_PX, BMP_PMS_PY, BMP_PMS_SX, BMP_PMS_SY,
                  BMP_PMS_PAL+i, GFL_BMP_CHRAREA_GET_B );

      PMSDAT_SetDebugRandom( &pms );
      PMSDAT_SetDeco( &pms, 0, PMS_DECOID_HERO+i );
      PMS_DRAW_Print( wk->pms_draw, bp->pms, &pms , i );

      for(j = 0;j < BEACON_PROF_MAX;j++){
        bmpwin_Add( &bp->prof[j], BMP_BEACON_FRM+i,
            BMP_PROF_PAL+i, BMP_PROF_PX, BMP_PROF_PY+BMP_PROF_OY*j, BMP_PROF_SX, BMP_PROF_SY );
      }
      bmpwin_Add( &bp->record, BMP_BEACON_FRM+i,
          BMP_RECORD_PAL+i, BMP_RECORD_PX, BMP_RECORD_PY, BMP_RECORD_SX, BMP_RECORD_SY );
    
      bp->cTrainer = act_Add( wk, clunit, &wk->objResTrainer[i],
                        ACT_TRAINER_PX, ACT_TRAINER_PY, 0, 0, ACT_TRAINER_BGPRI+i );
      bp->cRank = act_Add( wk, clunit, &wk->objResNormal,
                        ACT_RANK_PX, ACT_RANK_PY, ACTANM_RANK05, 0, ACT_TRAINER_BGPRI+i );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 後処理
 *
 *	@param	BEACON_DETAIL_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _sub_BeaconWinExit( BEACON_DETAIL_WORK* wk )
{
  int i,j;
  BEACON_WIN* bp;
    
  for( i=0; i < BEACON_WIN_MAX; i++ )
  {
    bp = &wk->beacon_win[i];

    GFL_CLACT_WK_SetDrawEnable( bp->cTrainer, FALSE );
    GFL_CLACT_WK_Remove( bp->cTrainer);
    GFL_CLACT_WK_SetDrawEnable( bp->cRank, FALSE );
    GFL_CLACT_WK_Remove( bp->cRank);

    GFL_BMPWIN_Delete( bp->record.win );
    for(j = 0;j < BEACON_PROF_MAX;j++){
      GFL_BMPWIN_Delete( bp->prof[j].win );
    }
    GFL_BMPWIN_Delete( bp->pms );
  }
  PMS_DRAW_Exit( wk->pms_draw );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示 主処理
 *
 *	@param	BEACON_DETAIL_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _sub_BeaconWinProc( BEACON_DETAIL_WORK* wk )
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
  PMS_DRAW_Main( wk->pms_draw );
}


