//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_main.c
 *	@brief  キーシステムメインソース
 *	@author	Toru=Nagihashi
 *	@data		2011.05.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "gamesystem/game_data.h"
#include "system/bmp_winframe.h"
#include "print/wordset.h"
#include "field/zonedata.h"

//アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "arc/key_system_gra.naix"
#include "msg/msg_key_system.h"
#include "message.naix"

//自分のモジュール
#include "key_system_util.h"
#include "key_system_graphic.h"
#include "key_system_net.h"
#include "key_system_main.h"
#include "key_system_flow.h"

//セーブ
#include "savedata/save_outside.h"
#include "savedata/outside_permanent.h"

//デバッグ
#include "test/nagihashi/debugwin_keysystem.h"

//外部公開
#include "net_app/key_system.h"

FS_EXTERN_OVERLAY( outside_save );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// プロセス
//=====================================
static GFL_PROC_RESULT Key_System_Proc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Key_System_Proc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT Key_System_Proc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
const GFL_PROC_DATA KEY_SYSTEM_ProcData = 
{
  Key_System_Proc_Init,
  Key_System_Proc_Main,
  Key_System_Proc_Exit,
};
//=============================================================================
/**
 *					プロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Key_System_Proc_Init
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(outside_save) );

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_KEY_SYSTEM, 0x40000 );
  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(KEY_SYSTEM_WORK), HEAPID_KEY_SYSTEM );
  GFL_STD_MemClear( p_wk, sizeof(KEY_SYSTEM_WORK) );
  p_wk->p_param = p_param_adrs;

  //もし引数がNULLならばタイトルから呼び出された＝APPモード
  if( p_param_adrs == NULL )
  {
    p_wk->p_param = GFL_HEAP_AllocClearMemory(HEAPID_KEY_SYSTEM,sizeof(KEY_SYSTEM_PARAM) );
    p_wk->p_param->mode = KEY_SYSTEM_MODE_APP;
    p_wk->p_param->p_gamedata    = GAMEDATA_Create( HEAPID_KEY_SYSTEM );
    ZONEDATA_Open( HEAPID_KEY_SYSTEM );
  }

  //管理外セーブシステム
  p_wk->p_outside_sv = OutsideSave_SystemLoad( HEAPID_KEY_SYSTEM );
  {
    OUTSIDE_PERMANENT *p_per = OutsideSave_GetPermanentPtr( p_wk->p_outside_sv );
    p_wk->p_key_sv = OUTSIDE_PERMANENT_GetKeySystem( p_per );
  }

  //グラフィック初期化
  p_wk->p_graphic = KEY_SYSTEM_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_KEY_SYSTEM );

  //ネット初期化
  {
    KEYSYS_NET_INIT_PARAM param;
    param.net_type = KEYSYS_NET_TYPE_IRCWIRELESS;
    param.p_gamedata = p_wk->p_param->p_gamedata;
    p_wk->p_net = KEYSYS_NET_Init( &param, HEAPID_KEY_SYSTEM );
  }

  //共通モジュールの初期化
  p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_KEY_SYSTEM );
  p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_KEY_SYSTEM );
  p_wk->p_msg		  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_key_system_dat, HEAPID_KEY_SYSTEM );
  p_wk->p_word    = WORDSET_Create( HEAPID_KEY_SYSTEM );
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 128, HEAPID_KEY_SYSTEM );

  //BG
  KEYSYS_BG_Load( &p_wk->bg, HEAPID_KEY_SYSTEM );

  //OBJ
  KEYSYS_OBJ_Init( &p_wk->obj, p_wk->p_graphic, HEAPID_KEY_SYSTEM );
  KEYSYS_OBJ_Load( &p_wk->obj, HEAPID_KEY_SYSTEM );

  //選択履歴
  KEYSYS_STACK_Init( &p_wk->select, HEAPID_KEY_SYSTEM );

  //シーケンス
  p_wk->p_seq = KEYSYS_SEQ_Init( 5, p_wk, NULL, HEAPID_KEY_SYSTEM );

  //ディスプレイ作成
  p_wk->p_display = KEYSYS_DISPLAY_Init( p_wk, HEAPID_KEY_SYSTEM );
 
  //上画面、下部テキスト作成
  p_wk->p_text = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 1, 19, 30, 4, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, HEAPID_KEY_SYSTEM );
  KEYSYS_TEXT_SetColor( p_wk->p_text, PRINTSYS_LSB_Make( 1, 2, 0xF ) );
  KEYSYS_TEXT_WriteWindowFrame( p_wk->p_text, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M );

  //上画面タイトル作成
  p_wk->p_title = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 1,0,14, 2, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, HEAPID_KEY_SYSTEM );
  KEYSYS_TEXT_SetColor( p_wk->p_title, PRINTSYS_LSB_Make( 1, 2, 0 ) );

  KEYSYS_KEYFLOW_Init( p_wk, HEAPID_KEY_SYSTEM );

#ifdef PM_DEBUG
  //デバッグウィンドウ
  DEBUGWIN_InitProc( KEYSYS_BG_FRAME_TEXT_M, p_wk->p_font );
  DEBUGWIN_KEY_SYSTEM_Init( p_wk->p_key_sv, HEAPID_KEY_SYSTEM );
#endif //PM_DEBUG

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Key_System_Proc_Exit
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

#ifdef PM_DEBUG
  //デバッグウィンドウ
  DEBUGWIN_KEY_SYSTEM_Exit();
  DEBUGWIN_ExitProc();
#endif //PM_DEBUG

  //テキスト破棄
  if( p_wk->p_title )
  {
    KEYSYS_TEXT_Exit( p_wk->p_title );
    p_wk->p_title = NULL;
  }

  //テキスト破棄
  if( p_wk->p_text )
  {
    KEYSYS_TEXT_Exit( p_wk->p_text );
    p_wk->p_text = NULL;
  }

  //ディスプレイ
  KEYSYS_DISPLAY_Exit( p_wk->p_display );

  //シーケンス破棄
  KEYSYS_SEQ_Exit( p_wk->p_seq );

  //OBJ破棄
  KEYSYS_OBJ_Release( &p_wk->obj );

  //共通モジュール破棄
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  WORDSET_Delete( p_wk->p_word );
	GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

  //通信処理破棄
  KEYSYS_NET_Exit( p_wk->p_net );

  //グラフィック破棄
  KEY_SYSTEM_GRAPHIC_Exit( p_wk->p_graphic );

  //管理外セーブ
  OutsideSave_SystemUnload( p_wk->p_outside_sv );

  //もし引数がNULLならばタイトルから呼び出された＝APPモード
  if( p_param_adrs == NULL )
  {
    ZONEDATA_Close();
    GAMEDATA_Delete( p_wk->p_param->p_gamedata );
    GFL_HEAP_FreeMemory( p_wk->p_param );

    //タイトルに戻る
    OS_ResetSystem(0);
  }

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_KEY_SYSTEM );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(outside_save) );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SYSプロセス初期化
 *
 *  @param  GFL_PROC *p_proc  プロセス
 *  @param  *p_seq            シーケンス
 *  @param  *p_param          親ワーク
 *  @param  *p_work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT Key_System_Proc_Main
  ( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //通信エラー処理
  {
    KEYSYS_NET_ERROR_TYPE err = KEYSYS_NET_HandleError( p_wk->p_net );
    switch( err )
    {
    case KEYSYS_NET_ERROR_TYPE_NONE:        //エラーが起こっていません。
      /*  なにもしない  */
      break;
    case KEYSYS_NET_ERROR_TYPE_RETURN:      //再度通信をすればつながります。
      KEYSYS_NET_ClearError( p_wk->p_net );
      return GFL_PROC_RES_CONTINUE;
    default:
      GF_ASSERT(0);
    case KEYSYS_NET_ERROR_TYPE_DISCONNECT:  //アプリ終了orWi-Fiから切断してください。
      return GFL_PROC_RES_FINISH;
    }
  }

  //シーケンスメイン
  KEYSYS_SEQ_Main( p_wk->p_seq );

  //グラフィック描画
  KEY_SYSTEM_GRAPHIC_2D_Draw( p_wk->p_graphic );

  //画面作成・破棄メイン
  KEYSYS_DISPLAY_Main( p_wk->p_display );

  //BG処理メイン
  KEYSYS_BG_Main( &p_wk->bg );

  //キューメイン
  PRINTSYS_QUE_Main( p_wk->p_que );

  //タイトル描画
  KEYSYS_TEXT_Draw( p_wk->p_title );

  //テキスト描画
  if( p_wk->p_text )
  {
    KEYSYS_TEXT_Draw( p_wk->p_text );
  }

  //テキストセット描画
  if( p_wk->p_textset )
  {
    KEYSYS_TEXTSET_Draw( p_wk->p_textset );
  }

  //メニュー描画
  if( p_wk->p_menu )
  {
    KEYSYS_MENU_Draw( p_wk->p_menu );
  }

  //リスト描画
  if( p_wk->p_list )
  {
    KEYSYS_LIST_Draw( p_wk->p_list );
  }

  //通信処理
  KEYSYS_NET_Main( p_wk->p_net );

  //終了チェック
  if( KEYSYS_SEQ_IsEnd( p_wk->p_seq ) )
  {
    return GFL_PROC_RES_FINISH;
  }
  else
  {
    return GFL_PROC_RES_CONTINUE;
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
void KeySys_Seq_FadeInBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
	enum
	{	
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		(*p_seq)++;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
		  (*p_seq)++;
		}
		break;

	case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

	default:
		GF_ASSERT(0);
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
void KeySys_Seq_FadeOutBlack( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
	enum
	{
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};	

	switch( *p_seq )
	{	
	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    (*p_seq)++;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
      (*p_seq)++;
		}
		break;

	case SEQ_EXIT:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

	default:
		GF_ASSERT(0);
	}
}


//=============================================================================
/**
 *      BG処理
 */
//=============================================================================
#define KEYSYS_BG_SCROLL_SPEED     (-FX32_CONST(0.2))
//----------------------------------------------------------------------------
/**
 *	@brief  BG読み込み
 *
 *	@param	KEYSYS_WORK *p_wk  ワーク
 *	@param	heapID  読み込み用ヒープID
 */
//-----------------------------------------------------------------------------
void KEYSYS_BG_Load( KEYSYS_BG_WORK *p_wk, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_BG_WORK));

  //最初にクリアキャラセット
  GFL_BG_SetClearCharacter( KEYSYS_BG_FRAME_TEXT_M, 0x20, KEYSYS_CGR_CLEAR_M*0x20, heapID );

  {
    int i;
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_KEY_SYSTEM, heapID );

    //----背景----
    //PLT
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_key_system_gra_back00_NCLR,
        PALTYPE_MAIN_BG, KEYSYS_PLT_BG_BACK_M*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_key_system_gra_back00_NCLR,
        PALTYPE_SUB_BG, KEYSYS_PLT_BG_BACK_S*0x20, 0x20, heapID );
    //CHR
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_key_system_gra_back00_NCGR, 
        KEYSYS_BG_FRAME_BACK_M, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_key_system_gra_back00_NCGR, 
        KEYSYS_BG_FRAME_BACK_S, 0, 0, FALSE, heapID );
    //SCR
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_key_system_gra_back00_NSCR,
        KEYSYS_BG_FRAME_BACK_M, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_key_system_gra_back00_NSCR,
        KEYSYS_BG_FRAME_BACK_S, 0, 0, FALSE, heapID );

    //----メニュー----
    //PLT
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_key_system_gra_menu_frame_NCLR,
        PALTYPE_MAIN_BG, KEYSYS_PLT_BG_MENU_NORMAL_M*0x20, 0x20*2, heapID );

    //CHR
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_key_system_gra_menu_frame_NCGR, 
        KEYSYS_BG_FRAME_TEXT_M, KEYSYS_CGR_MENU_M, 0, FALSE, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  //フレーム
  BmpWinFrame_GraphicSet( KEYSYS_BG_FRAME_TEXT_M, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M, MENU_TYPE_SYSTEM, HEAPID_KEY_SYSTEM );

  //フォント
  {	
    ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_FONT, heapID );

    //上下画面フォントパレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_MAIN_BG, KEYSYS_PLT_BG_FONT_M*0x20, 0x20, heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_SUB_BG, KEYSYS_PLT_BG_FONT_S*0x20, 0x20, heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  BGメイン処理
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_BG_Main( KEYSYS_BG_WORK *p_wk )
{
  p_wk->scroll_now += KEYSYS_BG_SCROLL_SPEED;
  GFL_BG_SetScrollReq( KEYSYS_BG_FRAME_BACK_M, GFL_BG_SCROLL_Y_SET, p_wk->scroll_now >> FX32_SHIFT );
  GFL_BG_SetScrollReq( KEYSYS_BG_FRAME_BACK_S, GFL_BG_SCROLL_Y_SET, p_wk->scroll_now >> FX32_SHIFT );

}

//----------------------------------------------------------------------------
/**
 *	@brief OBJ初期化
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 *	@param	*p_graphic  グラフィック
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void KEYSYS_OBJ_Init( KEYSYS_OBJ_WORK *p_wk, const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_OBJ_WORK));
  p_wk->p_clunit = KEY_SYSTEM_GRAPHIC_GetClunit( cp_graphic );
}
//----------------------------------------------------------------------------
/**
 *	@brief  OBJに読み込み処理
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
void KEYSYS_OBJ_Load( KEYSYS_OBJ_WORK *p_wk, HEAPID heapID )
{
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_KEY_SYSTEM, heapID );

    p_wk->res[KEYSYS_RES_OBJ_NCG] =
      GFL_CLGRP_CGR_Register( p_handle, NARC_key_system_gra_obj_NCGR,
          FALSE, CLSYS_DRAW_MAIN, heapID );

    p_wk->res[KEYSYS_RES_OBJ_NCL] =
      GFL_CLGRP_PLTT_RegisterEx(
          p_handle ,NARC_key_system_gra_obj_NCLR, CLSYS_DRAW_MAIN, 0, KEYSYS_PLT_OBJ_COMMON_M, 1, heapID );

    p_wk->res[KEYSYS_RES_OBJ_NCE] =
      GFL_CLGRP_CELLANIM_Register(
      p_handle, NARC_key_system_gra_obj_NCER, NARC_key_system_gra_obj_NANR, heapID  );

    GFL_ARC_CloseDataHandle(p_handle);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  OBJ読み込み破棄
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_OBJ_Release( KEYSYS_OBJ_WORK *p_wk )
{
  GFL_CLGRP_PLTT_Release( p_wk->res[KEYSYS_RES_OBJ_NCL] );
  GFL_CLGRP_CGR_Release( p_wk->res[KEYSYS_RES_OBJ_NCG] );
  GFL_CLGRP_CELLANIM_Release( p_wk->res[KEYSYS_RES_OBJ_NCE] );
}
//----------------------------------------------------------------------------
/**
 *	@brief  CLWKを追加
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 *	@param	id      作成するもの
 *	@param	heapID  ヒープID
 *
 *	@return GFL_CLWK
 */
//-----------------------------------------------------------------------------
GFL_CLWK * KEYSYS_OBJ_AddClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id, HEAPID heapID )
{
  GFL_CLWK_DATA clwk_data;
  CLSYS_DRAW_TYPE draw_type;

  GF_ASSERT( p_wk->p_clwk[ id ] == NULL );

  switch( id )
  {
  case KEYSYS_CLWK_WAVE_D:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 128;
    clwk_data.pos_y = 113;
    clwk_data.anmseq  = 0;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 1;
    break;

  case KEYSYS_CLWK_WAVE_U:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 128;
    clwk_data.pos_y = 77;
    clwk_data.anmseq  = 1;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 1;
    break;

  case KEYSYS_CLWK_DS_D:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 57;
    clwk_data.pos_y = 154;
    clwk_data.anmseq  = 2;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 1;
    break;

  case KEYSYS_CLWK_DS_U:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 200;
    clwk_data.pos_y = 38;
    clwk_data.anmseq  = 3;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 1;
    break;

  case KEYSYS_CLWK_LOCK_00:
  case KEYSYS_CLWK_LOCK_01:
  case KEYSYS_CLWK_LOCK_02:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 0;
    clwk_data.pos_y = 0;
    clwk_data.anmseq  = 7;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 0;
    break;

  case KEYSYS_CLWK_MARK:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x = 200;
    clwk_data.pos_y = 38;
    clwk_data.anmseq  = 6;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 0;
    break;
    
  case KEYSYS_CLWK_KEY:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x   = 128;
    clwk_data.pos_y   = 98;
    clwk_data.anmseq  = 8;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 0;
    break;

  default:
    GF_ASSERT(0);
    return NULL;
  }

  p_wk->p_clwk[ id ] = GFL_CLACT_WK_Create( p_wk->p_clunit,
                                              p_wk->res[KEYSYS_RES_OBJ_NCG],
                                              p_wk->res[KEYSYS_RES_OBJ_NCL],
                                              p_wk->res[KEYSYS_RES_OBJ_NCE],
                                              &clwk_data, draw_type, heapID );

  GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[ id ], TRUE );
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[ id ], TRUE );
  GFL_CLACT_WK_SetAffineParam( p_wk->p_clwk[ id ], CLSYS_AFFINETYPE_DOUBLE );

  return p_wk->p_clwk[ id ];
}
//----------------------------------------------------------------------------
/**
 *	@brief CLWKを破棄
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk ワーク
 *	@param	id 破棄するもの
 */
//-----------------------------------------------------------------------------
void KEYSYS_OBJ_RemoveClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id )
{
  if( p_wk->p_clwk[id] ) 
  {
    GFL_CLACT_WK_Remove( p_wk->p_clwk[id] );
    p_wk->p_clwk[id] = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  CLWKを取得
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk　ワーク
 *	@param	id  取得するもの
 *
 *	@return GFL_CLWK
 */
//-----------------------------------------------------------------------------
GFL_CLWK * KEYSYS_OBJ_GetClwk( KEYSYS_OBJ_WORK *p_wk, KEYSYS_CLWK id )
{
  return p_wk->p_clwk[ id ];
}

//=============================================================================
/**
 *      STACK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  スタック  初期化
 *
 *	@param	KEYSYS_STACK *p_wk あw－く
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void KEYSYS_STACK_Init( KEYSYS_STACK *p_wk, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_STACK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スタックに値をPush
 *
 *	@param	KEYSYS_STACK *p_wk ワーク
 *	@param	num 選んだもの
 */
//-----------------------------------------------------------------------------
void KEYSYS_STACK_Push( KEYSYS_STACK *p_wk, u32 num )
{
  p_wk->stack[ p_wk->pos++ ] = num;

  GF_ASSERT( p_wk->pos < KEYSYS_STACK_MAX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スタック  値をPop
 *
 *	@param	const KEYSYS_STACK *cp_wk ワーク
 *
 *	@return 値
 */
//-----------------------------------------------------------------------------
u32 KEYSYS_STACK_Pop( KEYSYS_STACK *p_wk )
{
  GF_ASSERT( p_wk->pos != 0 );
  return p_wk->stack[ --p_wk->pos ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  スタック  現在の値を取得
 *
 *	@param	const KEYSYS_STACK *cp_wk ワーク
 *
 *	@return 値
 */
//-----------------------------------------------------------------------------
u32 KEYSYS_STACK_GetNow( const KEYSYS_STACK *cp_wk )
{
  GF_ASSERT( cp_wk->pos != 0 );
  return cp_wk->stack[ cp_wk->pos - 1 ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  スタック数を取得
 *
 *	@param	const KEYSYS_STACK *cp_wk   ワーク
 *
 *	@return スタック数
 */
//-----------------------------------------------------------------------------
u32 KEYSYS_STACK_GetStackNum( const KEYSYS_STACK *cp_wk )
{
  return cp_wk->pos;
}

