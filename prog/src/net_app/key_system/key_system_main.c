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
#include "key_system_snd.h"
#include "data_convert_flow.h"

//セーブ
#include "savedata/save_outside.h"
#include "savedata/outside_permanent.h"

//デバッグ
#include "test/nagihashi/debugwin_keysystem.h"
#include "test/nagihashi/debugwin_data_convert.h"

//外部公開
#include "net_app/key_system.h"

FS_EXTERN_OVERLAY( outside_save );
FS_EXTERN_OVERLAY( multiboot );

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

//-------------------------------------
/// シーケンス
//=====================================
//メインシーケンス
static void KeySys_Seq_Main( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//サブシーケンス
static void KeySys_Seq_MainMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//画面構築シーケンス
static void KeySys_Seq_CreateDisplayMainMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
/// 画面作成・破棄
//=====================================
//メインメニュー
static void KeySys_Display_CreateMainMenu( void *p_wk_adrs, HEAPID heapID );
static BOOL KeySys_Display_IsEndCreateMainMenu( void *p_wk_adrs );
static void KeySys_Display_DeleteMainMenu( void *p_wk_adrs );

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

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_KEY_SYSTEM, 0x40000 );
  //プロセスワーク作成
  p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(KEY_SYSTEM_WORK), HEAPID_KEY_SYSTEM );
  GFL_STD_MemClear( p_wk, sizeof(KEY_SYSTEM_WORK) );
  p_wk->p_param = p_param_adrs;

  //サブプロセス
  p_wk->p_procsys				= GFL_PROC_LOCAL_boot( HEAPID_KEY_SYSTEM );

  //もし引数がNULLならばタイトルから呼び出された＝APPモード
  if( p_param_adrs == NULL )
  {
    p_wk->p_param = GFL_HEAP_AllocClearMemory(HEAPID_KEY_SYSTEM,sizeof(KEY_SYSTEM_PARAM) );
    p_wk->p_param->mode = KEY_SYSTEM_MODE_APP;
    p_wk->p_param->p_gamedata    = GAMEDATA_Create( HEAPID_KEY_SYSTEM );
    ZONEDATA_Open( HEAPID_KEY_SYSTEM );
  }

  //ネット初期化
  {
    KEYSYS_NET_INIT_PARAM param;
    param.net_type = KEYSYS_NET_TYPE_IRCWIRELESS;
    param.p_gamedata = p_wk->p_param->p_gamedata;
    p_wk->p_net = KEYSYS_NET_Init( &param, HEAPID_KEY_SYSTEM );
  }

  //選択履歴
  KEYSYS_MAP_Init( &p_wk->select, HEAPID_KEY_SYSTEM );

  //シーケンス
  p_wk->p_seq = KEYSYS_SEQ_Init( 8, p_wk, NULL, HEAPID_KEY_SYSTEM );


  {
    //初回シーケンス
    static const KEYSYS_SEQ_FUNCTION sc_init_seq[KEY_SYSTEM_MODE_MAX]  =
    {
      KEYSYS_KEYFLOW_SEQ_GetKey,//KEY_SYSTEM_MODE_GET
      KeySys_Seq_Main,//KEY_SYSTEM_MODE_APP
    };
    GF_ASSERT( p_wk->p_param->mode < KEY_SYSTEM_MODE_MAX );
    KEYSYS_SEQ_Next( p_wk->p_seq, sc_init_seq[p_wk->p_param->mode] );
  }

  //一時モジュールの作成
  KEYSYS_InitTemporaryModules( p_wk, HEAPID_KEY_SYSTEM );


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

  //一時モジュールの破棄
  KEYSYS_ExitTemporaryModules( p_wk );

  //シーケンス破棄
  KEYSYS_SEQ_Exit( p_wk->p_seq );

  //通信処理破棄
  KEYSYS_NET_Exit( p_wk->p_net );

  //もし引数がNULLならばタイトルから呼び出された＝APPモード
  if( p_param_adrs == NULL )
  {
    ZONEDATA_Close();
    GAMEDATA_Delete( p_wk->p_param->p_gamedata );
    GFL_HEAP_FreeMemory( p_wk->p_param );

    //タイトルに戻る
    OS_ResetSystem(0);
  }

  //サブプロセス
	GFL_PROC_LOCAL_Exit( p_wk->p_procsys );

  //プロセスワーク破棄
  GFL_PROC_FreeWork( p_proc );
  //ヒープ破棄
  GFL_HEAP_DeleteHeap( HEAPID_KEY_SYSTEM );
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
  if( p_wk->p_net )
  {
    KEYSYS_NET_ERROR_TYPE err = KEYSYS_NET_HandleError( p_wk->p_net );
    switch( err )
    {
    case KEYSYS_NET_ERROR_TYPE_NONE:        //エラーが起こっていません。
      /*  なにもしない  */
      break;
    default:
      GF_ASSERT(0);
    case KEYSYS_NET_ERROR_TYPE_RETURN:      //再度通信をすればつながります。
    case KEYSYS_NET_ERROR_TYPE_DISCONNECT:  //切断したらメインメニューに戻る
      KEYSYS_NET_ClearError( p_wk->p_net );
      PMSND_StopSE(); //通信のループ音を消す
      if( p_wk->p_mb )
      {
        MB_PARENT_DATACONV_Exit( p_wk->p_mb );
        p_wk->p_mb = NULL;
      }
      //通信エラーが起こったら強制的にメインメニューへ
      KEYSYS_SEQ_Clear( p_wk->p_seq );  //積んだシーケンスを全部クリア
      KEYSYS_DISPLAY_DeleteForce( p_wk->p_display );  //どんな画面も強制クリア
      KEYSYS_MAP_Clear( &p_wk->select );  //今まで選んだメニュー位置クリア
      KEYSYS_SEQ_Next( p_wk->p_seq, KeySys_Seq_MainMenu );  //新たにメインメニューへ
      return GFL_PROC_RES_CONTINUE;
    }
  }

  //シーケンスメイン
  KEYSYS_SEQ_Main( p_wk->p_seq );

  //グラフィック描画
  if( p_wk->p_graphic )
  {
    KEY_SYSTEM_GRAPHIC_2D_Draw( p_wk->p_graphic );
  }

  //画面作成・破棄メイン
  if( p_wk->p_display )
  {
    KEYSYS_DISPLAY_Main( p_wk->p_display );
  }

  //BG処理メイン
  if( p_wk->p_bg )
  {
    KEYSYS_BG_Main( p_wk->p_bg );
  }

  //キューメイン
  if( p_wk->p_que )
  {
    PRINTSYS_QUE_Main( p_wk->p_que );
  }

  //タイトル描画
  if( p_wk->p_title )
  {
    KEYSYS_TEXT_Draw( p_wk->p_title );
  }

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
  if( p_wk->p_net )
  {
    KEYSYS_NET_Main( p_wk->p_net );
  }

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
//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウトホワイト
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
void KeySys_Seq_FadeOutWhite( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
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
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, 0 );
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

//----------------------------------------------------------------------------
/**
 *	@brief  ディスプレイ破棄
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
void KeySys_Seq_DeleteDisplay( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_DELETE_START,
    SEQ_DISPLAY_DELETE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_DELETE_START:
    KEYSYS_DISPLAY_Delete( p_wk->p_display );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_DELETE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
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

//=============================================================================
/**
 *      BG処理
 */
//=============================================================================
#define KEYSYS_BG_SCROLL_SPEED     (-FX32_CONST(0.2))
//----------------------------------------------------------------------------
/**
 *	@brief  BG初期化
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
KEYSYS_BG_WORK * KEYSYS_BG_Init( HEAPID heapID )
{
  KEYSYS_BG_WORK *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(KEYSYS_BG_WORK));
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  BG破棄
 *
 *	@param	KEYSYS_BG_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_BG_Exit( KEYSYS_BG_WORK *p_wk )
{
  GFL_HEAP_FreeMemory(p_wk);
}
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
  p_wk->heapID = heapID;

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
        KEYSYS_BG_FRAME_WND_M, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_key_system_gra_back00_NCGR, 
        KEYSYS_BG_FRAME_BACK_S, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_key_system_gra_back00_NCGR, 
        KEYSYS_BG_FRAME_WND_S, 0, 0, FALSE, heapID );
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
 *	@brief  スクリーン転送
 *
 *	@param	KEYSYS_BG_WORK *p_wk  ワーク
 *	@param	frm   転送先フレーム
 *	@param	type  転送するスクリーン
 */
//-----------------------------------------------------------------------------
void KEYSYS_BG_TransScreen( KEYSYS_BG_WORK *p_wk, int frm, KEYSYS_BG_SCR scr )
{
  ARCDATID datID;

  switch( scr )
  {
  case KEYSYS_BG_SCR_TITLE_M:
    datID = NARC_key_system_gra_back01_NSCR;
    break;
  case KEYSYS_BG_SCR_HAVEKEY_M:
    datID = NARC_key_system_gra_back03_NSCR;
    break;
  case KEYSYS_BG_SCR_HAVEKEY_S:
    datID = NARC_key_system_gra_back02_NSCR;
    break;
  default:
    GF_ASSERT(0);
    return;
  }

  GFL_ARC_UTIL_TransVramScreen( ARCID_KEY_SYSTEM, datID,
      frm, 0, 0, FALSE, p_wk->heapID );
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
          p_handle ,NARC_key_system_gra_obj_NCLR, CLSYS_DRAW_MAIN, 0, KEYSYS_PLT_OBJ_COMMON_M, 4, heapID );

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
  BOOL is_auto_anm = TRUE;
  BOOL is_draw_enable = TRUE;

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
    clwk_data.bgpri   = 1;
    is_auto_anm       = FALSE;
    is_draw_enable    = FALSE;
    break;
  case KEYSYS_CLWK_WINDOW:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x   = 128;
    clwk_data.pos_y   = 98;
    clwk_data.anmseq  = 10;
    clwk_data.softpri = 2;
    clwk_data.bgpri   = 1;
    is_auto_anm       = FALSE;
    is_draw_enable    = FALSE;
    break;
  case KEYSYS_CLWK_KIRAKIRA:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x   = 128;
    clwk_data.pos_y   = 98;
    clwk_data.anmseq  = 11;
    clwk_data.softpri = 0;
    clwk_data.bgpri   = 0;
    is_draw_enable    = FALSE;
    is_auto_anm       = FALSE;
    break;
  case KEYSYS_CLWK_LOCK_OPEN:
    draw_type = CLSYS_DRAW_MAIN;
    clwk_data.pos_x   = 128;
    clwk_data.pos_y   = 98;
    clwk_data.anmseq  = 12;
    clwk_data.softpri = 1;
    clwk_data.bgpri   = 1;
    is_auto_anm       = FALSE;
    is_draw_enable    = FALSE;
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

  GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[ id ], is_auto_anm );
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[ id ], is_draw_enable );
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
//----------------------------------------------------------------------------
/**
 *	@brief  リソースを取得
 *
 *	@param	const KEYSYS_OBJ_WORK *cp_wk  ワーク
 *	@param	res   リソース
 *
 *	@return リソース番号
 */
//-----------------------------------------------------------------------------
u32 KEYSYS_OBJ_GetResource( const KEYSYS_OBJ_WORK *cp_wk, KEYSYS_RES res )
{
  GF_ASSERT( res < KEYSYS_RES_MAX );
  return cp_wk->res[ res ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  CLUNITを取得
 *
 *	@param	KEYSYS_OBJ_WORK *p_wk   ワーク
 *
 *	@return CLUNIT
 */
//-----------------------------------------------------------------------------
GFL_CLUNIT * KEYSYS_OBJ_GetClunit( KEYSYS_OBJ_WORK *p_wk )
{
  return p_wk->p_clunit;
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
 *	@param	KEYSYS_MAP *p_wk あw－く
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void KEYSYS_MAP_Init( KEYSYS_MAP *p_wk, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_MAP) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  要素を追加する
 *
 *	@param	KEYSYS_MAP *p_wk  ワーク
 *	@param	char *cp_key  キー（KEYSYS_MAP_KEY_MAX-1文字まで）
 *	@param	value         値
 */
//-----------------------------------------------------------------------------
void KEYSYS_MAP_Insert( KEYSYS_MAP *p_wk, const char *cp_key, u32 value )
{
  int i;

  GF_ASSERT( GFL_STD_StrLen(cp_key) < KEYSYS_MAP_KEY_MAX );

  GF_ASSERT( KEYSYS_MAP_IsExist( p_wk, cp_key ) == FALSE );

  for( i = 0; i < KEYSYS_MAP_MAX; i++ )
  {
    if( p_wk->data[i].key[0] == 0 )
    {
      GFL_STD_MemCopy( cp_key, p_wk->data[i].key, KEYSYS_MAP_KEY_MAX );
      p_wk->data[i].value = value;
      break;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  要素を削除する
 *
 *	@param	KEYSYS_MAP *p_wk  ワーク
 *	@param	char *cp_key  キー（KEYSYS_MAP_KEY_MAX-1文字まで）
 */
//-----------------------------------------------------------------------------
void KEYSYS_MAP_Erase( KEYSYS_MAP *p_wk, const char *cp_key )
{
  int i;

  GF_ASSERT(GFL_STD_StrLen(cp_key) < KEYSYS_MAP_KEY_MAX);

  for( i = 0; i < KEYSYS_MAP_MAX; i++ )
  {
    if( GFL_STD_StrCmp(p_wk->data[i].key,cp_key) == 0 )
    {
      GFL_STD_MemClear( p_wk->data[i].key, KEYSYS_MAP_KEY_MAX );
      p_wk->data[i].value = 0;
      break;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  要素があるかチェックする
 *
 *	@param	KEYSYS_MAP *p_wk  ワーク
 *	@param	char *cp_key  キー（KEYSYS_MAP_KEY_MAX-1文字まで）
 *
 *	@retval TRUE存在FALSEなし
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_MAP_IsExist( const KEYSYS_MAP *cp_wk, const char *cp_key )
{
  int i;

  GF_ASSERT_MSG(GFL_STD_StrLen(cp_key) < KEYSYS_MAP_KEY_MAX,"len%d\n",GFL_STD_StrLen(cp_key));

  for( i = 0; i < KEYSYS_MAP_MAX; i++ )
  {
    if( GFL_STD_StrCmp(cp_wk->data[i].key,cp_key) == 0 )
    {
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  要素の値を取得する
 *
 *	@param	KEYSYS_MAP *p_wk  ワーク
 *	@param	char *cp_key  キー（KEYSYS_MAP_KEY_MAX-1文字まで）
 *
 *	@return 要素の値
 */
//-----------------------------------------------------------------------------
u32 KEYSYS_MAP_GetValue( const KEYSYS_MAP *cp_wk, const char *cp_key )
{
  int i;

  GF_ASSERT(GFL_STD_StrLen(cp_key) < KEYSYS_MAP_KEY_MAX);

  for( i = 0; i < KEYSYS_MAP_MAX; i++ )
  {
    if( GFL_STD_StrCmp(cp_wk->data[i].key,cp_key) == 0 )
    {
      return cp_wk->data[i].value;
    }
  }

  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  すべての要素をクリア
 *
 *	@param	KEYSYS_MAP *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_MAP_Clear( KEYSYS_MAP *p_wk )
{
  int i;

  for( i = 0; i < KEYSYS_MAP_MAX; i++ )
  {
    GFL_STD_MemClear( p_wk->data[i].key, KEYSYS_MAP_KEY_MAX );
    p_wk->data[i].value = 0;
  }
}
//=============================================================================
/**
 *  その他
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  一時モジュールの作成（別プロセスから帰ってきた際に作成すること）
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
void KEYSYS_InitTemporaryModules( KEY_SYSTEM_WORK *p_wk, HEAPID heapID )
{
  GFL_OVERLAY_Load( FS_OVERLAY_ID(outside_save) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(multiboot) );

  //管理外セーブシステム
  p_wk->p_outside_sv = OutsideSave_SystemLoad( heapID );
  {
    OUTSIDE_PERMANENT *p_per = OutsideSave_GetPermanentPtr( p_wk->p_outside_sv );
    p_wk->p_key_sv = OUTSIDE_PERMANENT_GetKeySystem( p_per );
    p_wk->p_data_sv = OUTSIDE_PERMANENT_GetDataConvert( p_per );
  }
  
  //それぞれの初期化
  KEYSYS_KEYFLOW_Init( p_wk, HEAPID_KEY_SYSTEM );
  KEYSYS_DATACONVFLOW_Init( p_wk, HEAPID_KEY_SYSTEM );

  //グラフィック初期化
  p_wk->p_graphic = KEY_SYSTEM_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, heapID );

  //共通モジュールの初期化
  p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
  p_wk->p_que			= PRINTSYS_QUE_Create( heapID );
  p_wk->p_msg		  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_key_system_dat, heapID );
  p_wk->p_word    = WORDSET_Create( heapID );
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 128, heapID );

  //BG
  p_wk->p_bg = KEYSYS_BG_Init( heapID );
  KEYSYS_BG_Load( p_wk->p_bg, heapID );

  //OBJ
  KEYSYS_OBJ_Init( &p_wk->obj, p_wk->p_graphic, heapID );
  KEYSYS_OBJ_Load( &p_wk->obj, heapID );

  //ディスプレイ作成
  p_wk->p_display = KEYSYS_DISPLAY_Init( p_wk, heapID );

 
  //上画面、下部テキスト作成
  p_wk->p_text = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 1, 19, 30, 4, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, heapID );
  KEYSYS_TEXT_SetColor( p_wk->p_text, PRINTSYS_LSB_Make( 1, 2, 0xF ) );
  KEYSYS_TEXT_WriteWindowFrame( p_wk->p_text, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M );

  //上画面タイトル作成
  p_wk->p_title = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 1, 1, 14, 2, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, heapID );
  KEYSYS_TEXT_SetColor( p_wk->p_title, PRINTSYS_LSB_Make( 1, 2, 0 ) );

#ifdef PM_DEBUG
  //デバッグウィンドウ
  DEBUGWIN_InitProc( KEYSYS_BG_FRAME_TEXT_M, p_wk->p_font );
  DEBUGWIN_KEY_SYSTEM_Init( p_wk->p_key_sv, heapID );
  DEBUGWIN_DATA_CONVERT_Init( p_wk->p_data_sv, heapID );
#endif //PM_DEBUG

}
//----------------------------------------------------------------------------
/**
 *	@brief  一時モジュールの破棄（別プロセスの際に破棄すること）
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_ExitTemporaryModules( KEY_SYSTEM_WORK *p_wk )
{
#ifdef PM_DEBUG
  //デバッグウィンドウ
  DEBUGWIN_DATA_CONVERT_Exit();
  DEBUGWIN_KEY_SYSTEM_Exit();
  DEBUGWIN_ExitProc();
#endif //PM_DEBUG

  //念のためディスプレイ消去
  KEYSYS_DISPLAY_DeleteForce( p_wk->p_display );

  //タイトル破棄
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
  p_wk->p_display = NULL;

  KEYSYS_BG_Exit( p_wk->p_bg );
  p_wk->p_bg = NULL;

  //OBJ破棄
  KEYSYS_OBJ_Release( &p_wk->obj );

  //グラフィック破棄
  KEY_SYSTEM_GRAPHIC_Exit( p_wk->p_graphic );
  p_wk->p_graphic = NULL;

  //管理外セーブ
  OutsideSave_SystemUnload( p_wk->p_outside_sv );
  p_wk->p_outside_sv = NULL;

  //共通モジュール破棄
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  p_wk->p_strbuf = NULL;
  WORDSET_Delete( p_wk->p_word );
  p_wk->p_word = NULL;
	GFL_MSG_Delete( p_wk->p_msg );
  p_wk->p_msg = NULL;
	PRINTSYS_QUE_Delete( p_wk->p_que );
  p_wk->p_que = NULL;
	GFL_FONT_Delete( p_wk->p_font );
  p_wk->p_font = NULL;

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(multiboot) );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(outside_save) );
}

//=============================================================================
/**
 *  フロー
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  メインメニュー
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_Main( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_FADEIN,
    SEQ_MAIN,
    SEQ_FADEOUT,
    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    //最初はメインメニュー作成
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplayMainMenu );
    (*p_seq)++;
    break;
  case SEQ_FADEIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeInBlack );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_MainMenu );
    (*p_seq)++;
    break;
  case SEQ_FADEOUT:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeOutWhite );
    (*p_seq)++;
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_End( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  メインメニュー作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_MainMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_MAIN,
    SEQ_DISPLAY_DELETE,
    SEQ_SELECT_CALL,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    //最初はメインメニュー作成
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_CreateDisplayMainMenu );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    KEYSYS_MENU_Main( p_wk->p_menu );

    if( KEYSYS_MENU_IsChangeSelect( p_wk->p_menu ) )
    {
      if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
      {
        KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_65+KEYSYS_MENU_GetSelect( p_wk->p_menu ), KEYSYS_TEXT_TYPE_QUE ); 
      }
    }

    if( KEYSYS_MENU_IsDecide( p_wk->p_menu ) )
    {
      p_wk->ret = KEYSYS_MENU_GetSelect( p_wk->p_menu );

      if( p_wk->ret == 2 )//もどるのときはFADE後まで画面を残すので消さない
      {
        *p_seq = SEQ_END;
      }
      else
      {
        KEYSYS_MAP_Insert( &p_wk->select, "MAN", KEYSYS_MENU_GetSelect( p_wk->p_menu ) );
        (*p_seq)++;
      }
    }
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;

  case SEQ_SELECT_CALL:
    switch( p_wk->ret )
    {
    case 0://キーシステム
      //カーソル位置を登録
      KEYSYS_SEQ_Call( p_seqwk, KEYSYS_KEYFLOW_SEQ_Main );
      break;
    case 1://データコンバート
      //カーソル位置を登録
      KEYSYS_SEQ_Call( p_seqwk, KEYSYS_DATACONVFLOW_SEQ_Main );
      break;
    default:
      GF_ASSERT(0);
    }
    //もう一度メインメニュー
    (*p_seq ) = 0;
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
 *	@brief  メインメニュー作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Seq_CreateDisplayMainMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl =
  {
    KeySys_Display_CreateMainMenu,
    KeySys_Display_IsEndCreateMainMenu,
    KeySys_Display_DeleteMainMenu,
    NULL,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    //最初はメインメニュー作成
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl, HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
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

//=============================================================================
/**
 * 画面作成・破棄
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  メインメニュー作成
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_CreateMainMenu( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //タイトル
  //KEYSYS_TEXT_Print( p_wk->p_title, p_wk->p_msg, KEYSYS_STR_00, KEYSYS_TEXT_TYPE_QUE );

  //メニュー追加
  {
    int i;
    KEYSYS_MENU_PARAM menu_param;
    GFL_STD_MemClear( &menu_param, sizeof(KEYSYS_MENU_PARAM) );
    menu_param.frm = KEYSYS_BG_FRAME_TEXT_M;
    menu_param.font_plt = KEYSYS_PLT_BG_FONT_M;
    menu_param.frm_chr  = KEYSYS_CGR_MENU_M;
    menu_param.frm_plt  = KEYSYS_PLT_BG_MENU_NORMAL_M;
    menu_param.p_msg    = p_wk->p_msg;
    menu_param.p_font   = p_wk->p_font;
    menu_param.max      = 3;
    if( KEYSYS_MAP_IsExist( &p_wk->select, "MAN" ) )
    {
      menu_param.cursor   = KEYSYS_MAP_GetValue( &p_wk->select, "MAN" );
      KEYSYS_MAP_Erase( &p_wk->select, "MAN" );
    }
    else
    {
      menu_param.cursor   = 0;
    }
    for( i = 0; i < menu_param.max; i++ )
    {
      menu_param.wnd_data[i].w  = 26;
      menu_param.wnd_data[i].h  = 2;
      menu_param.wnd_data[i].x  = 3;
      menu_param.wnd_data[i].y  = 4 + (i*(menu_param.wnd_data[i].h+2+1)); //+2はフレーム分+1は間隔分
    }
    menu_param.wnd_data[0].strID = KEYSYS_STR_62;
    menu_param.wnd_data[1].strID = KEYSYS_STR_63;
    menu_param.wnd_data[2].strID = KEYSYS_STR_64;

    p_wk->p_menu = KEYSYS_MENU_Init( &menu_param, heapID );

    //メッセージ
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_65+menu_param.cursor, KEYSYS_TEXT_TYPE_QUE );
  }

  //タイトル
  KEYSYS_TEXT_Print( p_wk->p_title, p_wk->p_msg, KEYSYS_STR_76, KEYSYS_TEXT_TYPE_QUE );

  KEYSYS_BG_TransScreen( p_wk->p_bg, KEYSYS_BG_FRAME_WND_M, KEYSYS_BG_SCR_TITLE_M );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メインメニュー作成完了待ち
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Display_IsEndCreateMainMenu( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //BOOL is_end_title  = KEYSYS_TEXT_IsMsgEnd( p_wk->p_title );
  BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );
  BOOL is_end_menu = KEYSYS_MENU_IsDrawEnd( p_wk->p_menu );

  if( is_end_text && is_end_menu )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_WND_M );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  メインメニュー破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Display_DeleteMainMenu( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //メニュー破棄
  KEYSYS_MENU_Exit( p_wk->p_menu );
  p_wk->p_menu = NULL;    
  
  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );

}
