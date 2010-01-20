//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_core.c
 *	@brief	WIFIのバトルマッチコア画面
 *	@author	Toru=Nagihashi
 *	@data		2009.11.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	ライブラリ
#include <gflib.h>

//	システム
#include "system/main.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "system/gf_date.h"
#include "pokeicon/pokeicon.h"
#include "gamesystem/game_data.h"
#include "system/net_err.h"

//	アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_wifi_match.h"

//	文字表示
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"

//  セーブデータ
#include "savedata/dreamworld_data.h"

//WIFIバトルマッチのモジュール
#include "wifibattlematch_graphic.h"
#include "wifibattlematch_view.h"
#include "wifibattlematch_net.h"
#include "wifibattlematch_util.h"

//外部公開
#include "wifibattlematch_core.h"

//デバッグ
#include "debug/debugwin_sys.h"

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(dpw_common);

//-------------------------------------
///	デバッグマクロ
//=====================================
#ifdef PM_DEBUG
#define DEBUGWIN_USE
#endif //PM_DEBUG


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	シンク
//=====================================
#define ENEMYDATA_WAIT_SYNC (180)

//=============================================================================
/**
 *        定義
*/
//=============================================================================
//-------------------------------------
///	メインワーク前方宣言
//=====================================
typedef struct
{
	//グラフィック設定
	WIFIBATTLEMATCH_GRAPHIC_WORK	*p_graphic;

  //リソース
  WIFIBATTLEMATCH_VIEW_RESOURCE *p_res;

	//共通で使うフォント
	GFL_FONT			            *p_font;

	//共通で使うキュー
	PRINT_QUE				          *p_que;

	//共通で使うメッセージ
	GFL_MSGDATA	          		*p_msg;

	//共通で使う単語
	WORDSET				          	*p_word;

	//上画面情報
	PLAYERINFO_WORK         	*p_playerinfo;

	//対戦者情報
	MATCHINFO_WORK          	*p_matchinfo;

  //待機アイコン
  WBM_WAITICON_WORK         *p_wait;

  //テキスト面
  WBM_TEXT_WORK             *p_text;

  //リスト
  WBM_LIST_WORK             *p_list;

  //ネット
  WIFIBATTLEMATCH_NET_WORK  *p_net;

  //メインシーケンス
  WBM_SEQ_WORK              *p_seq;

  //引数
  WIFIBATTLEMATCH_CORE_PARAM  *p_param;

  //サーバーからの受信バッファ
  WIFIBATTLEMATCH_GDB_RND_SCORE_DATA rnd_score;

  //ウエイト
  u32 cnt;

#ifdef DEBUGWIN_USE
  u32 playerinfo_mode;
  u32 matchinfo_mode;
  u32 playerinfo_rank;
  u32 matchinfo_rank;
  u32 waiticon_draw;
#endif //DEBUGWIN_USE

} WIFIBATTLEMATCH_RND_WORK;

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_RND_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_RND_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_RND_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	ランダムマッチシーケンス関数
//=====================================
static void WbmRndSeq_Init( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//レーティング処理
static void WbmRndSeq_Rate_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Rate_Matching( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Rate_EndBattle( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//フリー処理
static void WbmRndSeq_Free_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Free_Matching( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Free_EndBattle( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//エラー処理
static void WbmRndSeq_Err_ReturnLogin( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	便利
//=====================================
static void Util_PlayerInfo_Create( WIFIBATTLEMATCH_RND_WORK *p_wk, WIFIBATTLEMATCH_CORE_RETMODE mode );
static void Util_PlayerInfo_Delete( WIFIBATTLEMATCH_RND_WORK *p_wk );
static BOOL Util_PlayerInfo_Move( WIFIBATTLEMATCH_RND_WORK *p_wk );
static void Util_MatchInfo_Create( WIFIBATTLEMATCH_RND_WORK *p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_enemy_data );
static void Util_MatchInfo_Delete( WIFIBATTLEMATCH_RND_WORK *p_wk );
static BOOL Util_MatchInfo_Main( WIFIBATTLEMATCH_RND_WORK *p_wk );
static void Util_SaveScore( RNDMATCH_DATA *p_save, WIFIBATTLEMATCH_BTLRULE btl_rule, const WIFIBATTLEMATCH_RND_WORK *cp_wk );

//-------------------------------------
///	便利
//=====================================
#ifdef DEBUGWIN_USE
static void DEBUGWIN_VisiblePlayerInfo( void* userWork , DEBUGWIN_ITEM* item );
static void DEBUGWIN_VisibleMatchInfo( void* userWork , DEBUGWIN_ITEM* item );
static void DEBUGWIN_ChangePlayerInfo( void* userWork , DEBUGWIN_ITEM* item );
static void DEBUGWIN_ChangeMatchInfo( void* userWork , DEBUGWIN_ITEM* item );
static void DEBUGWIN_ChangeWaitIcon( void* userWork , DEBUGWIN_ITEM* item );
#endif //DEBUGWIN_USE
//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	WifiBattleMaptchRnd_ProcData =
{	
	WIFIBATTLEMATCH_RND_PROC_Init,
	WIFIBATTLEMATCH_RND_PROC_Main,
	WIFIBATTLEMATCH_RND_PROC_Exit,
};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチランダム対戦	コアプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_RND_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	WIFIBATTLEMATCH_RND_WORK	*p_wk;
  WIFIBATTLEMATCH_CORE_PARAM *p_param  = p_param_adrs;


	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_CORE, 0x30000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_RND_WORK), HEAPID_WIFIBATTLEMATCH_CORE );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_RND_WORK) );	
  p_wk->p_param = p_param;

	//グラフィック設定
	p_wk->p_graphic	= WIFIBATTLEMATCH_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_WIFIBATTLEMATCH_CORE );

  //リソース読み込み
  p_wk->p_res     = WIFIBATTLEMATCH_VIEW_LoadResource( WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic ), WIFIBATTLEMATCH_MODE_RANDOM, HEAPID_WIFIBATTLEMATCH_CORE );

	//共通モジュールの作成
	p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
												NARC_message_wifi_match_dat, HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_word	= WORDSET_CreateEx( WORDSET_DEFAULT_SETNUM, WORDSET_COUNTRY_BUFLEN, HEAPID_WIFIBATTLEMATCH_CORE );

	//モジュールの作成
	{	
    DWCUserData *p_user_data;
    WIFI_LIST   *p_wifilist;
    p_wifilist  = GAMEDATA_GetWiFiList( p_param->p_param->p_game_data );
    p_user_data = WifiList_GetMyUserInfo( p_wifilist );

    p_wk->p_net   = WIFIBATTLEMATCH_NET_Init( p_user_data, p_wifilist, HEAPID_WIFIBATTLEMATCH_CORE );
  }
  p_wk->p_text  = WBM_TEXT_Init( BG_FRAME_M_TEXT, PLT_FONT_M, PLT_TEXT_M, CGR_OFS_M_TEXT, p_wk->p_que, p_wk->p_font, HEAPID_WIFIBATTLEMATCH_CORE );
  p_wk->p_seq   = WBM_SEQ_Init( p_wk, WbmRndSeq_Init, HEAPID_WIFIBATTLEMATCH_CORE );

  { 
    GFL_CLUNIT  *p_unit;
    p_unit  = WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
    p_wk->p_wait  = WBM_WAITICON_Init( p_unit, p_wk->p_res, HEAPID_WIFIBATTLEMATCH_CORE );
    WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
	}

#ifdef DEBUGWIN_USE
  DEBUGWIN_InitProc( GFL_BG_FRAME0_M, p_wk->p_font );
  DEBUGWIN_AddGroupToTop( 0, "ランダムマッチ", HEAPID_WIFIBATTLEMATCH_CORE );
  DEBUGWIN_AddItemToGroup( "じぶんカード", 
                              DEBUGWIN_VisiblePlayerInfo, 
                              p_wk, 
                              0, 
                              HEAPID_WIFIBATTLEMATCH_CORE );

  DEBUGWIN_AddItemToGroup( "あいてカード", 
                              DEBUGWIN_VisibleMatchInfo, 
                              p_wk, 
                              0, 
                              HEAPID_WIFIBATTLEMATCH_CORE );

  DEBUGWIN_AddItemToGroup( "じぶんランク", 
                              DEBUGWIN_ChangePlayerInfo, 
                              p_wk, 
                              0, 
                              HEAPID_WIFIBATTLEMATCH_CORE );

  DEBUGWIN_AddItemToGroup( "あいてランク", 
                              DEBUGWIN_ChangeMatchInfo, 
                              p_wk, 
                              0, 
                              HEAPID_WIFIBATTLEMATCH_CORE );

  DEBUGWIN_AddItemToGroup( "ぐるぐる[ON]", 
                              DEBUGWIN_ChangeWaitIcon, 
                              p_wk, 
                              0, 
                              HEAPID_WIFIBATTLEMATCH_CORE );
#endif
	
	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチランダム対戦	コアプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_RND_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum 
  { 
    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,
  };

	WIFIBATTLEMATCH_RND_WORK	      *p_wk	= p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_param_adrs;

  if( p_param->result ==  WIFIBATTLEMATCH_CORE_RESULT_FINISH )
  { 
    switch( *p_seq )
    { 
    case SEQ_START_DISCONNECT:
      WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
      GFL_NET_Exit( NULL );
      *p_seq  = SEQ_WAIT_DISCONNECT;
      return GFL_PROC_RES_CONTINUE; 

    case SEQ_WAIT_DISCONNECT:
      if( GFL_NET_IsExit() )
      { 
        break;
      }
      return GFL_PROC_RES_CONTINUE;
    }
  }

#ifdef DEBUGWIN_USE
  DEBUGWIN_RemoveGroup( 0 );
  DEBUGWIN_ExitProc();
#endif

	//モジュールの破棄
	{	
    WBM_WAITICON_Exit( p_wk->p_wait );

    WBM_SEQ_Exit( p_wk->p_seq );

    WBM_TEXT_Exit( p_wk->p_text );

    if( p_param->result !=  WIFIBATTLEMATCH_CORE_RESULT_FINISH )
    { 
      WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
    }

    Util_MatchInfo_Delete( p_wk );

    Util_PlayerInfo_Delete( p_wk );
	}

	//共通モジュールの破棄
	WORDSET_Delete( p_wk->p_word );
	GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

  //リソース破棄
  WIFIBATTLEMATCH_VIEW_UnLoadResource( p_wk->p_res );

	//グラフィック破棄
	WIFIBATTLEMATCH_GRAPHIC_Exit( p_wk->p_graphic );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH_CORE );
	
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_RND_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_MAIN,
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
  };


	WIFIBATTLEMATCH_RND_WORK	*p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM *p_param  = p_param_adrs;

  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
      *p_seq  = SEQ_FADEIN_WAIT;
    break;
  case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{
      *p_seq  = SEQ_MAIN;
    }
    break;
  case SEQ_MAIN:
    //メインシーケンス
    WBM_SEQ_Main( p_wk->p_seq );

    if( WBM_SEQ_IsEnd( p_wk->p_seq ) )
    { 
      *p_seq  = SEQ_FADEOUT_START;
    }

    //共通のエラー処理（SC,GDB以外）
    if( WIFIBATTLEMATCH_NET_CheckError( p_wk->p_net ) )
    { 
      p_param->result = WIFIBATTLEMATCH_CORE_RESULT_ERR_NEXT_LOGIN;
      *p_seq  = SEQ_FADEOUT_START;
    }
    break;

  case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{
      return GFL_PROC_RES_FINISH;
    }
  }


	//描画
	WIFIBATTLEMATCH_GRAPHIC_2D_Draw( p_wk->p_graphic );

	//プリント
	PRINTSYS_QUE_Main( p_wk->p_que );

  //テキスト
  WBM_TEXT_Main( p_wk->p_text );

  //待機アイコン
  WBM_WAITICON_Main( p_wk->p_wait );

  //BG
  WIFIBATTLEMATCH_VIEW_Main( p_wk->p_res );

  //文字表示
  if( p_wk->p_playerinfo )
  { 
    PLAYERINFO_PrintMain( p_wk->p_playerinfo, p_wk->p_que );
  }
  if( p_wk->p_matchinfo )
  { 
    MATCHINFO_PrintMain( p_wk->p_matchinfo, p_wk->p_que );
  }

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *  シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ初期化
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Init( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  NAGI_Printf( " mode%d ret%d\n", p_param->mode, p_param->retmode );
  switch( p_param->mode )
  { 
  case WIFIBATTLEMATCH_CORE_MODE_START:
    WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Start );
    break;

  case WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE:
    if( p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_RATE )
    { 
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_EndBattle );
      break;
    }
    else if( p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_FREE )
    { 
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_EndBattle );
      break;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ開始
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk  ワーク
 *	@param	WBM_SEQ_WORK *p_wk          シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_INIT,
    SEQ_WAIT_INIT,
    SEQ_START_RECVDATA_SAKE,
    SEQ_WAIT_RECVDATA_SAKE,
    SEQ_CHECK_GPF,
    SEQ_SELECT_MSG,
    SEQ_START_SELECT_MODE,
    SEQ_WAIT_SELECT_MODE,
    SEQ_START_CANCEL_MSG,
    SEQ_START_CANCEL_SELECT,
    SEQ_WAIT_CANCEL_SELECT,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    ///	初期化
    //=====================================
  case SEQ_START_INIT:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_000, WBM_TEXT_TYPE_STREAM );
    WIFIBATTLEMATCH_NET_StartInitialize( p_wk->p_net );
    *p_seq = SEQ_WAIT_INIT;
    break;
    
  case SEQ_WAIT_INIT:
    { 
      DWCGdbError error;
      SAVE_CONTROL_WORK *p_save = GAMEDATA_GetSaveControlWork(p_param->p_param->p_game_data);
      if( WIFIBATTLEMATCH_NET_WaitInitialize( p_wk->p_net, p_save, &error ) )
      { 
        *p_seq = SEQ_START_RECVDATA_SAKE;
      }
      else
      { 
        WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE type;
        type = WIFIBATTLEMATCH_GDB_GetErrorRepairType( error );
        switch( type )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY:       //もう一度
          *p_seq = SEQ_START_INIT;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
          NetErr_DispCallFatal();
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	初期データ受信
    //=====================================
  case SEQ_START_RECVDATA_SAKE:
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_WAIT_RECVDATA_SAKE );
    break;

  case SEQ_WAIT_RECVDATA_SAKE:
    *p_seq       = SEQ_CHECK_GPF;
    break;

  case SEQ_CHECK_GPF:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      DREAMWORLD_SAVEDATA *p_dream = DREAMWORLD_SV_GetDreamWorldSaveData( p_sv );
      if( DREAMWORLD_SV_GetSignin( p_dream ) )
      { 
        *p_seq = SEQ_SELECT_MSG;
      }
      else
      { 

        *p_seq = 0;
        p_param->retmode = WIFIBATTLEMATCH_CORE_RETMODE_FREE;
        WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_Start );
        break;
      }
    }
    break;

    //-------------------------------------
    ///	モード選択
    //=====================================
  case SEQ_SELECT_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_001, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_MODE );
    break;

  case SEQ_START_SELECT_MODE:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_003;
      setup.strID[1]= WIFIMATCH_TEXT_004;
      setup.strID[2]= WIFIMATCH_TEXT_005;
      setup.list_max= 3;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_SELECT_MODE;
    }
    break;

  case SEQ_WAIT_SELECT_MODE:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          *p_seq = 0;
          p_param->retmode = WIFIBATTLEMATCH_CORE_RETMODE_FREE;
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_Start );
          break;
        case 1:
          *p_seq = 0;
          p_param->retmode = WIFIBATTLEMATCH_CORE_RETMODE_RATE;
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_Start );
          break;
        case 2:
          *p_seq = SEQ_START_CANCEL_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	やめる
    //=====================================
  case SEQ_START_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_006, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_CANCEL_SELECT );
    break;

  case SEQ_START_CANCEL_SELECT:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_CANCEL_SELECT;
    }
    break;

  case SEQ_WAIT_CANCEL_SELECT:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          WBM_SEQ_End( p_seqwk );
          break;

        case 1:
          *p_seq = SEQ_SELECT_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
    break;
  };
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  レーティング開始
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Rate_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_RECVRATE_SAKE,
    SEQ_WAIT_RECVRATE_SAKE,
    SEQ_WAIT_CARDIN,
    SEQ_START_POKECHECK_SERVER,
    SEQ_WAIT_POKECHECK_SERVER,
    SEQ_CHECK_POKE,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    ///	レーティングデータ受信
    //=====================================
  case SEQ_START_RECVRATE_SAKE:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_000, WBM_TEXT_TYPE_STREAM );
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->rnd_score );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_WAIT_RECVRATE_SAKE );
    break;

  case SEQ_WAIT_RECVRATE_SAKE:
    { 
      DWCGdbError result  = DWC_GDB_ERROR_NONE;
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &result ) )
      { 
        //自分の情報を表示
        Util_PlayerInfo_Create( p_wk, WIFIBATTLEMATCH_CORE_RETMODE_RATE );

        //自分のレートを保存
        { 
          WIFIBATTLEMATCH_ENEMYDATA *p_player;
          p_player  = p_param->p_player_data;
          p_player->rate  = p_wk->rnd_score.single_rate;
        }

        *p_seq       = SEQ_WAIT_CARDIN;
      }
      else
      { 
        WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE type;
        type = WIFIBATTLEMATCH_GDB_GetErrorRepairType( result );
        switch( type )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY:       //もう一度
          *p_seq = SEQ_START_RECVRATE_SAKE;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
          NetErr_DispCallFatal();
          break;
        }
      }
    }
    break;


  case SEQ_WAIT_CARDIN:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq       = SEQ_START_POKECHECK_SERVER;
    }
    break;
    //-------------------------------------
    /// ポケモン不正チェック
    //=====================================
  case SEQ_START_POKECHECK_SERVER:
    *p_seq       = SEQ_WAIT_POKECHECK_SERVER;
    break;

  case SEQ_WAIT_POKECHECK_SERVER:
    *p_seq       = SEQ_CHECK_POKE;
    break;

  case SEQ_CHECK_POKE:
    WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_Matching );
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
    break;

  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  レーティングのマッチング
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Rate_Matching( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_MATCHING,
    SEQ_START_MATCHING_MSG,
    SEQ_WAIT_MATCHING,
    SEQ_START_CHECK_CHEAT,
    SEQ_WAIT_CHECK_CHEAT,
    SEQ_START_SENDDATA,
    SEQ_WAIT_SENDDATA,

    SEQ_WAIT_MATCH_CARDIN,
    SEQ_OK_MATCHING_MSG,
    SEQ_OK_MATCHING_WAIT,
    SEQ_END_MATCHING_MSG,
    SEQ_END_MATCHING,

    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    ///	マッチング開始
    //=====================================
  case SEQ_START_MATCHING:
    WBM_WAITICON_SetDrawEnable( p_wk->p_wait, TRUE );
    WIFIBATTLEMATCH_NET_StartMatchMake( p_wk->p_net, p_param->p_param->btl_rule ); 
    *p_seq = SEQ_START_MATCHING_MSG;
    break;

  case SEQ_START_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_009, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MATCHING;
    break;

  case SEQ_WAIT_MATCHING:
    if( WIFIBATTLEMATCH_NET_GetSeqMatchMake( p_wk->p_net ) < WIFIBATTLEMATCH_NET_SEQ_CONNECT_START )
    { 
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      { 
        *p_seq = SEQ_START_SELECT_CANCEL_MSG;
      }
    }

    if( WIFIBATTLEMATCH_NET_WaitMatchMake( p_wk->p_net ) )
    { 
      *p_seq = SEQ_START_CHECK_CHEAT;
    }
    break;

    //-------------------------------------
    ///	見つかった後の相手の不正チェック
    //=====================================
  case SEQ_START_CHECK_CHEAT:


    *p_seq = SEQ_WAIT_CHECK_CHEAT;
    break;

  case SEQ_WAIT_CHECK_CHEAT:
    *p_seq = SEQ_START_SENDDATA;
    break;

  case SEQ_START_SENDDATA:
    { 
      WIFIBATTLEMATCH_ENEMYDATA *p_my_data = p_param->p_player_data;
      if( WIFIBATTLEMATCH_NET_StartEnemyData( p_wk->p_net, p_my_data ) )
      { 
        *p_seq       = SEQ_WAIT_SENDDATA;
      }
    }
    break;

  case SEQ_WAIT_SENDDATA:
    {
      WIFIBATTLEMATCH_ENEMYDATA *p_recv_data;
      if( WIFIBATTLEMATCH_NET_WaitEnemyData( p_wk->p_net, &p_recv_data ) )
      { 
        //受信したデータをバッファにコピー
        WIFIBATTLEMATCH_ENEMYDATA *p_enemy_data = p_param->p_enemy_data;
        GFL_STD_MemCopy( p_recv_data, p_enemy_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
        WIFIBATTLEMATCH_DATA_ModifiEnemyData( p_enemy_data, HEAPID_WIFIBATTLEMATCH_CORE );

        //対戦者情報表示
        Util_MatchInfo_Create( p_wk, p_param->p_enemy_data );

        WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
        *p_seq  = SEQ_WAIT_MATCH_CARDIN;
      }
    }
    break;

  case SEQ_WAIT_MATCH_CARDIN:
    if( Util_MatchInfo_Main( p_wk ) )
    { 
      *p_seq  = SEQ_OK_MATCHING_MSG;
    }
    break;

  case SEQ_OK_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_010, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_OK_MATCHING_WAIT );
    break;

  case SEQ_OK_MATCHING_WAIT:
    if( p_wk->cnt++ > ENEMYDATA_WAIT_SYNC )
    { 
      p_wk->cnt      = 0;
      *p_seq      = SEQ_END_MATCHING_MSG;
    }
    break;

  case SEQ_END_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_012, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_END_MATCHING );
    break;

  case SEQ_END_MATCHING:
    p_param->result = WIFIBATTLEMATCH_CORE_RESULT_NEXT_BATTLE;
    WBM_SEQ_End( p_seqwk );
    break;

    //-------------------------------------
    ///	キャンセル処理
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_011, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CANCEL );
    break;

  case SEQ_START_SELECT_CANCEL:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_SELECT_CANCEL;
    }
    break;

  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          WBM_SEQ_End( p_seqwk );
          break;
        case 1:
          *p_seq = SEQ_START_MATCHING_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  レーティングの戦闘終了後
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Rate_EndBattle( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
 
  enum
  { 
    SEQ_START_MSG,
    SEQ_START_REPORT_ATLAS,
    SEQ_WAIT_REPORT_ATLAS,

    SEQ_START_RECVDATA_SAKE,
    SEQ_WAIT_RECVDATA_SAKE,

    SEQ_START_SAVE_MSG,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,

    SEQ_START_SELECT_BTLREC_MSG,
    SEQ_START_SELECTBTLREC,
    SEQ_WAIT_SELECTBTLREC,

    SEQ_WAIT_CARDIN,
    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,

    SEQ_START_SELECT_CONTINUE_MSG,
    SEQ_START_SELECT_CONTINUE,
    SEQ_WAIT_SELECT_CONTINUE,

    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    ///	Atlasへの書き込み
    //=====================================
  case SEQ_START_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_013, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_REPORT_ATLAS );
    break;
  case SEQ_START_REPORT_ATLAS:
    WIFIBATTLEMATCH_SC_Start( p_wk->p_net, p_param->p_param->btl_rule, p_param->btl_result );
    *p_seq = SEQ_WAIT_REPORT_ATLAS;
    break;
  case SEQ_WAIT_REPORT_ATLAS:
    { 
      DWCScResult result;
      if( WIFIBATTLEMATCH_SC_Process( p_wk->p_net, &result ) )
      { 
        NAGI_Printf( "送ったよ！%d\n", result );
        *p_seq = SEQ_START_RECVDATA_SAKE;
      }
      else
      { 
        WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE type;
        type = WIFIBATTLEMATCH_SC_GetErrorRepairType( result );
        switch( type )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY:       //もう一度
          *p_seq = SEQ_START_REPORT_ATLAS;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
          NetErr_DispCallFatal();
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	Sakeから取得
    //=====================================
  case SEQ_START_RECVDATA_SAKE:
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->rnd_score );
    *p_seq = SEQ_WAIT_RECVDATA_SAKE;
    break;
  case SEQ_WAIT_RECVDATA_SAKE:
    { 
      DWCGdbError result  = DWC_GDB_ERROR_NONE;
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &result ) )
      { 
        //セーブデータ
        Util_SaveScore( p_param->p_rndmatch, p_param->p_param->btl_rule, p_wk );

        //対戦者情報表示
        Util_PlayerInfo_Create( p_wk, WIFIBATTLEMATCH_CORE_RETMODE_RATE );

        *p_seq       = SEQ_WAIT_CARDIN;
      }
      else
      { 
        WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE type;
        type = WIFIBATTLEMATCH_GDB_GetErrorRepairType( result );
        switch( type )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY:       //もう一度
          *p_seq = SEQ_START_RECVDATA_SAKE;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
          NetErr_DispCallFatal();
          break;
        }
      }
    }
    break;

  case SEQ_WAIT_CARDIN:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq       = SEQ_START_DISCONNECT;
    }
    break;

    //-------------------------------------
    /// 切断処理
    //=====================================
  case SEQ_START_DISCONNECT:
    *p_seq = SEQ_WAIT_DISCONNECT;
    break;

  case SEQ_WAIT_DISCONNECT:
    if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE ) )
    { 
      *p_seq = SEQ_START_SELECT_BTLREC_MSG;
    }
    break;

    //-------------------------------------
    /// レーティングセーブ処理
    //=====================================
  case SEQ_START_SAVE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_017, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_START_SAVE;
    break;
  case SEQ_START_SAVE:
    GAMEDATA_SaveAsyncStart(p_param->p_param->p_game_data);
    *p_seq       = SEQ_WAIT_SAVE;
    break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_param->p_param->p_game_data);
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        *p_seq       = SEQ_START_SELECT_BTLREC_MSG;
        break;
      }
    }
    break;

    //-------------------------------------
    /// 録画確認
    //=====================================
  case SEQ_START_SELECT_BTLREC_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_014, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECTBTLREC );
    break;

  case SEQ_START_SELECTBTLREC:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_SELECTBTLREC;
    }
    break;

  case SEQ_WAIT_SELECTBTLREC:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          *p_seq = SEQ_START_SELECT_CONTINUE_MSG;
          break;
        case 1:
          *p_seq = SEQ_START_SELECT_CONTINUE_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// 対戦続行確認
    //=====================================
  case SEQ_START_SELECT_CONTINUE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_015, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CONTINUE );
    break;
  case SEQ_START_SELECT_CONTINUE:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_SELECT_CONTINUE;
    }
    break;
  case SEQ_WAIT_SELECT_CONTINUE:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_Start );
          break;
        case 1:
          *p_seq = SEQ_START_SELECT_CANCEL_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// ランダムマッチ終了確認
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_016, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CANCEL );
    break;

  case SEQ_START_SELECT_CANCEL:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_SELECT_CANCEL;
    }
    break;
  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          WBM_SEQ_End( p_seqwk );
          break;
        case 1:
          *p_seq = SEQ_START_SELECT_CONTINUE_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk ); 
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  フリーモード開始
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Free_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_FREE_MSG,
    SEQ_WAIT_CARDIN,
    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    ///	フリーモード開始メッセージ
    //=====================================
  case SEQ_START_FREE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_002, WBM_TEXT_TYPE_STREAM );
    { 
      //自分のデータを表示
      Util_PlayerInfo_Create( p_wk, WIFIBATTLEMATCH_CORE_RETMODE_FREE );
    }
    *p_seq       = SEQ_WAIT_CARDIN;
    break;

  case SEQ_WAIT_CARDIN:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq       = SEQ_WAIT_MSG;
    }
    break;

  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    {
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_Matching );
    }
    break;

  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  フリーモードのマッチング処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Free_Matching( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_MATCHING,
    SEQ_START_MATCHING_MSG,
    SEQ_WAIT_MATCHING,
    SEQ_START_CHECK_CHEAT,
    SEQ_WAIT_CHECK_CHEAT,
    SEQ_START_SENDDATA,
    SEQ_WAIT_SENDDATA,
    SEQ_WAIT_MATCH_CARDIN,
    SEQ_OK_MATCHING_MSG,
    SEQ_OK_MATCHING_WAIT,
    SEQ_END_MATCHING_MSG,
    SEQ_END_MATCHING,

    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;
  switch( *p_seq )
  { 
    //-------------------------------------
    ///	マッチング開始
    //=====================================
  case SEQ_START_MATCHING:
    WBM_WAITICON_SetDrawEnable( p_wk->p_wait, TRUE );
    WIFIBATTLEMATCH_NET_StartMatchMake( p_wk->p_net, p_param->p_param->btl_rule ); 
    *p_seq = SEQ_START_MATCHING_MSG;
    break;

  case SEQ_START_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_009, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MATCHING;
    break;

  case SEQ_WAIT_MATCHING:
    if( WIFIBATTLEMATCH_NET_GetSeqMatchMake( p_wk->p_net ) < WIFIBATTLEMATCH_NET_SEQ_CONNECT_START )
    { 
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      { 
        *p_seq = SEQ_START_SELECT_CANCEL_MSG;
      }
    }

    if( WIFIBATTLEMATCH_NET_WaitMatchMake( p_wk->p_net ) )
    { 
      *p_seq = SEQ_START_SENDDATA;
    }
    break;

    //-------------------------------------
    ///	データ送受信
    //=====================================
  case SEQ_START_SENDDATA:
    { 
      WIFIBATTLEMATCH_ENEMYDATA *p_my_data = p_param->p_player_data;
      if( WIFIBATTLEMATCH_NET_StartEnemyData( p_wk->p_net, p_my_data ) )
      { 
        *p_seq       = SEQ_WAIT_SENDDATA;
      }
    }
    break;

  case SEQ_WAIT_SENDDATA:
    {
      WIFIBATTLEMATCH_ENEMYDATA *p_recv_data;
      if( WIFIBATTLEMATCH_NET_WaitEnemyData( p_wk->p_net, &p_recv_data ) )
      { 
        WIFIBATTLEMATCH_ENEMYDATA *p_enemy_data = p_param->p_enemy_data;
        GFL_STD_MemCopy( p_recv_data, p_enemy_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );

        Util_MatchInfo_Create( p_wk, p_param->p_enemy_data );

        WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
        *p_seq       = SEQ_WAIT_MATCH_CARDIN;
      }
    }
    break;

  case SEQ_WAIT_MATCH_CARDIN:
    if( Util_MatchInfo_Main( p_wk ) )
    { 
      *p_seq  = SEQ_OK_MATCHING_MSG;
    }
    break;

  case SEQ_OK_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_010, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_OK_MATCHING_WAIT );
    break;

  case SEQ_OK_MATCHING_WAIT:
    if( p_wk->cnt++ > ENEMYDATA_WAIT_SYNC )
    { 
      p_wk->cnt      = 0;
      *p_seq      = SEQ_END_MATCHING_MSG;
    }
    break;

  case SEQ_END_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_012, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_END_MATCHING );
    break;

  case SEQ_END_MATCHING:
    p_param->result = WIFIBATTLEMATCH_CORE_RESULT_NEXT_BATTLE;
    WBM_SEQ_End( p_seqwk );
    break;

    //-------------------------------------
    ///	キャンセル処理
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_011, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CANCEL );
    break;

  case SEQ_START_SELECT_CANCEL:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_SELECT_CANCEL;
    }
    break;

  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          WBM_SEQ_End( p_seqwk );
          break;
        case 1:
          *p_seq = SEQ_START_MATCHING_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  ランダムマッチ  フリーモードの戦闘終了後
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Free_EndBattle( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_SELECT_BTLREC_MSG,
    SEQ_START_SELECTBTLREC,
    SEQ_WAIT_SELECTBTLREC,

    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,

    SEQ_START_SAVE_MSG,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,

    SEQ_START_SELECT_CONTINUE_MSG,
    SEQ_START_SELECT_CONTINUE,
    SEQ_WAIT_SELECT_CONTINUE,

    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    /// 録画確認
    //=====================================
  case SEQ_START_SELECT_BTLREC_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_014, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECTBTLREC );
    break;
  case SEQ_START_SELECTBTLREC:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_SELECTBTLREC;
    }
    break;
  case SEQ_WAIT_SELECTBTLREC:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          *p_seq = SEQ_START_DISCONNECT;  //@todo
          break;
        case 1:
          *p_seq = SEQ_START_DISCONNECT;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// 切断処理
    //=====================================
  case SEQ_START_DISCONNECT:
    *p_seq = SEQ_WAIT_DISCONNECT;
    break;

  case SEQ_WAIT_DISCONNECT:
    if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE ) )
    { 

      switch( p_param->btl_result )
      {
      case BTL_RESULT_WIN:
      case BTL_RESULT_RUN_ENEMY:
        RNDMATCH_AddParam( p_param->p_rndmatch, p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_WIN );
        break;
      case BTL_RESULT_LOSE:
      case BTL_RESULT_RUN:
        RNDMATCH_AddParam( p_param->p_rndmatch, p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_LOSE );
        break;
      }


      *p_seq = SEQ_START_SELECT_CONTINUE_MSG;
    }
    break;

    //-------------------------------------
    /// セーブ処理
    //=====================================
  case SEQ_START_SAVE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_017, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_START_SAVE;
    break;

  case SEQ_START_SAVE:
    GAMEDATA_SaveAsyncStart(p_param->p_param->p_game_data);
    *p_seq       = SEQ_WAIT_SAVE;
    break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_param->p_param->p_game_data);
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        *p_seq       = SEQ_START_SELECT_CONTINUE_MSG;
        break;
      }
    }
    break;

    //-------------------------------------
    /// 対戦続行確認
    //=====================================
  case SEQ_START_SELECT_CONTINUE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_015, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CONTINUE );
    break;
  case SEQ_START_SELECT_CONTINUE:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_SELECT_CONTINUE;
    }
    break;
  case SEQ_WAIT_SELECT_CONTINUE:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_Start );
          break;
        case 1:
          *p_seq = SEQ_START_SELECT_CANCEL_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    /// ランダムマッチ終了確認
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_016, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CANCEL );
    break;
  case SEQ_START_SELECT_CANCEL:
    { 
      WBM_LIST_SETUP  setup;
      GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      setup.frm     = BG_FRAME_M_TEXT;
      setup.font_plt= PLT_FONT_M;
      setup.frm_plt = PLT_LIST_M;
      setup.frm_chr = CGR_OFS_M_LIST;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq     = SEQ_WAIT_SELECT_CANCEL;
    }
    break;
  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = WBM_LIST_Main( p_wk->p_list );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        WBM_LIST_Exit( p_wk->p_list );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          WBM_SEQ_End( p_seqwk );
          break;
        case 1:
          *p_seq = SEQ_START_SELECT_CONTINUE_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	共通
    //=====================================
  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
    break;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  エラーのためログインに戻る
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Err_ReturnLogin( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;
  
  p_param->result = WIFIBATTLEMATCH_CORE_RESULT_ERR_NEXT_LOGIN;
  WBM_SEQ_End( p_seqwk );
  
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を表示
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_PlayerInfo_Create( WIFIBATTLEMATCH_RND_WORK *p_wk, WIFIBATTLEMATCH_CORE_RETMODE mode )
{ 
  if( p_wk->p_playerinfo == NULL )
  {
    MYSTATUS    *p_my;
    GFL_CLUNIT	*p_unit;
    BOOL is_rate;

    //自分のデータを表示
    PLAYERINFO_RANDOMMATCH_DATA info_setup;

    p_my  = GAMEDATA_GetMyStatus( p_wk->p_param->p_param->p_game_data); 
    p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );

    GFL_STD_MemClear( &info_setup, sizeof(PLAYERINFO_RANDOMMATCH_DATA) );


    if( mode == WIFIBATTLEMATCH_CORE_RETMODE_RATE )
    { 
      info_setup.btl_rule = p_wk->p_param->p_param->btl_rule;
      is_rate = TRUE;

      switch( p_wk->p_param->p_param->btl_rule )
      { 
      case WIFIBATTLEMATCH_BTLRULE_SINGLE:  
        info_setup.rate     = p_wk->rnd_score.single_rate;
        info_setup.win_cnt  = p_wk->rnd_score.single_win;
        info_setup.lose_cnt = p_wk->rnd_score.single_lose;
        info_setup.btl_cnt  = p_wk->rnd_score.single_win + p_wk->rnd_score.single_lose;
        break;

      case WIFIBATTLEMATCH_BTLRULE_DOUBLE:
        info_setup.rate     = p_wk->rnd_score.double_rate;
        info_setup.win_cnt  = p_wk->rnd_score.double_win;
        info_setup.lose_cnt = p_wk->rnd_score.double_lose;
        info_setup.btl_cnt  = p_wk->rnd_score.double_win + p_wk->rnd_score.double_lose;
        break;

      case WIFIBATTLEMATCH_BTLRULE_TRIPLE:
        info_setup.rate     = p_wk->rnd_score.triple_rate;
        info_setup.win_cnt  = p_wk->rnd_score.triple_win;
        info_setup.lose_cnt = p_wk->rnd_score.triple_lose;
        info_setup.btl_cnt  = p_wk->rnd_score.triple_win + p_wk->rnd_score.triple_lose;
        break;

      case WIFIBATTLEMATCH_BTLRULE_ROTATE:
        info_setup.rate     = p_wk->rnd_score.rot_rate;
        info_setup.win_cnt  = p_wk->rnd_score.rot_win;
        info_setup.lose_cnt = p_wk->rnd_score.rot_lose;
        info_setup.btl_cnt  = p_wk->rnd_score.rot_win + p_wk->rnd_score.rot_lose;
        break;

      case WIFIBATTLEMATCH_BTLRULE_SHOOTER:
        info_setup.rate     = p_wk->rnd_score.shooter_rate;
        info_setup.win_cnt  = p_wk->rnd_score.shooter_win;
        info_setup.lose_cnt = p_wk->rnd_score.shooter_lose;
        info_setup.btl_cnt  = p_wk->rnd_score.shooter_win + p_wk->rnd_score.shooter_lose;
        break;
      }
    }
    else if( mode == WIFIBATTLEMATCH_CORE_RETMODE_FREE )
    { 
      is_rate = FALSE;
      info_setup.btl_rule = p_wk->p_param->p_param->btl_rule;
      info_setup.rate     = 0;
      info_setup.win_cnt  = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_WIN );
      info_setup.lose_cnt = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_LOSE );
      info_setup.btl_cnt  = info_setup.win_cnt + info_setup.lose_cnt;
    }
    
    p_wk->p_playerinfo	= PLAYERINFO_RND_Init( &info_setup, is_rate, p_my, p_unit, p_wk->p_res, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, HEAPID_WIFIBATTLEMATCH_CORE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を破棄
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_PlayerInfo_Delete( WIFIBATTLEMATCH_RND_WORK *p_wk )
{ 
  if( p_wk->p_playerinfo )
  { 
    PLAYERINFO_RND_Exit( p_wk->p_playerinfo );
    p_wk->p_playerinfo  = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のカードをスライドイン
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk  ワーク
 *
 *	@return TRUEで完了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL Util_PlayerInfo_Move( WIFIBATTLEMATCH_RND_WORK *p_wk )
{ 
  return PLAYERINFO_MoveMain( p_wk->p_playerinfo );
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者の情報を表示
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_MatchInfo_Create( WIFIBATTLEMATCH_RND_WORK *p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_enemy_data )
{ 
  if( p_wk->p_matchinfo == NULL )
  { 
    GFL_CLUNIT  *p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
    p_wk->p_matchinfo		= MATCHINFO_Init( cp_enemy_data, p_unit, p_wk->p_res, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, HEAPID_WIFIBATTLEMATCH_CORE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者の情報を破棄
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_MatchInfo_Delete( WIFIBATTLEMATCH_RND_WORK *p_wk )
{ 
  if( p_wk->p_matchinfo )
  { 
    MATCHINFO_Exit( p_wk->p_matchinfo );
    p_wk->p_matchinfo = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者のカードをスライドイン
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk  ワーク
 *
 *	@return TRUEで完了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL Util_MatchInfo_Main( WIFIBATTLEMATCH_RND_WORK *p_wk )
{
  return MATCHINFO_MoveMain( p_wk->p_matchinfo );
}
//----------------------------------------------------------------------------
/**
 *	@brief  セーブ  レートモード用
 *
 *	@param	RNDMATCH_DATA *p_save セーブ
 *	@param	mode                  ルール
 *	@param	WIFIBATTLEMATCH_RND_WORK *cp_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_SaveScore( RNDMATCH_DATA *p_save, WIFIBATTLEMATCH_BTLRULE btl_rule, const WIFIBATTLEMATCH_RND_WORK *cp_wk )
{ 
  u32 rate, win_cnt, lose_cnt, btl_cnt;

  switch( btl_rule )
  { 
  case WIFIBATTLEMATCH_BTLRULE_SINGLE:  
    rate     = cp_wk->rnd_score.single_rate;
    win_cnt  = cp_wk->rnd_score.single_win;
    lose_cnt = cp_wk->rnd_score.single_lose;
    btl_cnt  = cp_wk->rnd_score.single_win + cp_wk->rnd_score.single_lose;
    break;

  case WIFIBATTLEMATCH_BTLRULE_DOUBLE:
    rate     = cp_wk->rnd_score.double_rate;
    win_cnt  = cp_wk->rnd_score.double_win;
    lose_cnt = cp_wk->rnd_score.double_lose;
    btl_cnt  = cp_wk->rnd_score.double_win + cp_wk->rnd_score.double_lose;
    break;

  case WIFIBATTLEMATCH_BTLRULE_TRIPLE:
    rate     = cp_wk->rnd_score.triple_rate;
    win_cnt  = cp_wk->rnd_score.triple_win;
    lose_cnt = cp_wk->rnd_score.triple_lose;
    btl_cnt  = cp_wk->rnd_score.triple_win + cp_wk->rnd_score.triple_lose;
    break;

  case WIFIBATTLEMATCH_BTLRULE_ROTATE:
    rate     = cp_wk->rnd_score.rot_rate;
    win_cnt  = cp_wk->rnd_score.rot_win;
    lose_cnt = cp_wk->rnd_score.rot_lose;
    btl_cnt  = cp_wk->rnd_score.rot_win + cp_wk->rnd_score.rot_lose;
    break;

  case WIFIBATTLEMATCH_BTLRULE_SHOOTER:
    rate     = cp_wk->rnd_score.shooter_rate;
    win_cnt  = cp_wk->rnd_score.shooter_win;
    lose_cnt = cp_wk->rnd_score.shooter_lose;
    btl_cnt  = cp_wk->rnd_score.shooter_win + cp_wk->rnd_score.shooter_lose;
    break;
  } 

  //セーブデータ
  RNDMATCH_SetParam( p_save, RNDMATCH_TYPE_RATE_SINGLE + btl_rule, RNDMATCH_PARAM_IDX_RATE, rate );
  RNDMATCH_SetParam( p_save, RNDMATCH_TYPE_RATE_SINGLE + btl_rule, RNDMATCH_PARAM_IDX_WIN, win_cnt );
  RNDMATCH_SetParam( p_save, RNDMATCH_TYPE_RATE_SINGLE + btl_rule, RNDMATCH_PARAM_IDX_LOSE, btl_cnt );
}

//=============================================================================
/**
 *    DEBUG
 */
//=============================================================================
#ifdef DEBUGWIN_USE
//----------------------------------------------------------------------------
/**
 *	@brief  自分のカード表示
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGWIN_VisiblePlayerInfo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *scp_name_tbl[] =
  { 
    "レート",
    "フリー",
  };

  WIFIBATTLEMATCH_RND_WORK  *p_wk = userWork;

  if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
  { 

    p_wk->playerinfo_mode++;
    p_wk->playerinfo_mode %= NELEMS( scp_name_tbl );

    Util_PlayerInfo_Delete( p_wk );
    Util_PlayerInfo_Create( p_wk, p_wk->playerinfo_mode );

    while( !Util_PlayerInfo_Move( p_wk ) )
    { 
      GFL_BG_VBlankFunc();
      GFL_CLACT_SYS_VBlankFunc();
      PLAYERINFO_PrintMain( p_wk->p_playerinfo, p_wk->p_que );
    }

    DEBUGWIN_ITEM_SetNameV( item , "じぶんカード[%s]", scp_name_tbl[ p_wk->playerinfo_mode ] );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者のカード表示
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGWIN_VisibleMatchInfo( void* userWork , DEBUGWIN_ITEM* item )
{ 

  WIFIBATTLEMATCH_RND_WORK  *p_wk = userWork;

  if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
  { 
    WIFIBATTLEMATCH_DATA_ModifiEnemyData( p_wk->p_param->p_enemy_data, HEAPID_WIFIBATTLEMATCH_CORE );

    Util_MatchInfo_Delete( p_wk );
    Util_MatchInfo_Create( p_wk, p_wk->p_param->p_enemy_data );

    while( !Util_MatchInfo_Main( p_wk ) )
    { 
      GFL_BG_VBlankFunc();
      GFL_CLACT_SYS_VBlankFunc();
      MATCHINFO_PrintMain( p_wk->p_matchinfo, p_wk->p_que );
    }

  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のカード切り替え
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGWIN_ChangePlayerInfo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *scp_name_tbl[] =
  { 
    "ブラック",
    "ゴールド",
    "シルバー",
    "ブロンズ",
    "カッパー",
    "ノーマル",
  };

  WIFIBATTLEMATCH_RND_WORK  *p_wk = userWork;
  u32 win;

  if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
  { 

    p_wk->playerinfo_rank++;
    p_wk->playerinfo_rank %= NELEMS( scp_name_tbl );

    switch( p_wk->playerinfo_rank )
    { 
    case 0: //ブラック
      win = WBM_CARD_WIN_BLACK;
      break;
    case 1:  //ゴールド
      win = WBM_CARD_WIN_GOLD;
      break;
    case 2: //シルバー
      win = WBM_CARD_WIN_SILVER;
      break;
    case 3: //ブロンズ
      win = WBM_CARD_WIN_BRONZE;
      break;
    case 4: //カッパー
      win = WBM_CARD_WIN_COPPER;
      break;
    case 5: //ノーマル
      win = WBM_CARD_WIN_NORMAL;
      break;
    }
    p_wk->rnd_score.single_win  = win;
    p_wk->rnd_score.double_win  = win;
    p_wk->rnd_score.triple_win  = win;
    p_wk->rnd_score.rot_win  = win;
    p_wk->rnd_score.shooter_win  = win;
    RNDMATCH_SetParam( p_wk->p_param->p_rndmatch, p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_WIN, win );

    Util_PlayerInfo_Delete( p_wk );
    Util_PlayerInfo_Create( p_wk, p_wk->playerinfo_mode );

    while( !Util_PlayerInfo_Move( p_wk ) )
    { 
      GFL_BG_VBlankFunc();
      GFL_CLACT_SYS_VBlankFunc();
      PLAYERINFO_PrintMain( p_wk->p_playerinfo, p_wk->p_que );
    }

    DEBUGWIN_ITEM_SetNameV( item , "じぶんランク[%s]", scp_name_tbl[ p_wk->playerinfo_rank ] );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者のカード切り替え
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGWIN_ChangeMatchInfo( void* userWork , DEBUGWIN_ITEM* item )
{ 
  static const char *scp_name_tbl[] =
  { 
    "ブラック",
    "ゴールド",
    "シルバー",
    "ブロンズ",
    "カッパー",
    "ノーマル",
  };

  WIFIBATTLEMATCH_RND_WORK  *p_wk = userWork;
  u32 win;

  if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
  { 
    p_wk->matchinfo_rank++;
    p_wk->matchinfo_rank %= NELEMS( scp_name_tbl );

    switch( p_wk->matchinfo_rank )
    { 
    case 0: //ブラック
      win = WBM_CARD_WIN_BLACK;
      break;
    case 1:  //ゴールド
      win = WBM_CARD_WIN_GOLD;
      break;
    case 2: //シルバー
      win = WBM_CARD_WIN_SILVER;
      break;
    case 3: //ブロンズ
      win = WBM_CARD_WIN_BRONZE;
      break;
    case 4: //カッパー
      win = WBM_CARD_WIN_COPPER;
      break;
    case 5: //ノーマル
      win = WBM_CARD_WIN_NORMAL;
      break;
    }
    p_wk->p_param->p_enemy_data->win_cnt = win;

    WIFIBATTLEMATCH_DATA_ModifiEnemyData( p_wk->p_param->p_enemy_data, HEAPID_WIFIBATTLEMATCH_CORE );

    Util_MatchInfo_Delete( p_wk );
    Util_MatchInfo_Create( p_wk, p_wk->p_param->p_enemy_data );

    while( !Util_MatchInfo_Main( p_wk ) )
    { 
      GFL_BG_VBlankFunc();
      GFL_CLACT_SYS_VBlankFunc();
      MATCHINFO_PrintMain( p_wk->p_matchinfo, p_wk->p_que );
    }

    DEBUGWIN_ITEM_SetNameV( item , "あいてランク[%s]", scp_name_tbl[ p_wk->matchinfo_rank ] );
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アイコン表示
 *
 *	@param	void* userWork  ワーク
 *	@param	item            アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGWIN_ChangeWaitIcon( void* userWork , DEBUGWIN_ITEM* item )
{
  static const char *scp_name_tbl[] =
  { 
    "OFF",
    "ON",
  };

  if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
  { 
    WIFIBATTLEMATCH_RND_WORK  *p_wk = userWork;

    p_wk->waiticon_draw++;
    p_wk->waiticon_draw %= NELEMS( scp_name_tbl );

    switch( p_wk->waiticon_draw )
    { 
    case 0:
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
      break;
    case 1:
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, TRUE );
      break;
    }

    DEBUGWIN_ITEM_SetNameV( item , "ぐるぐる[%s]", scp_name_tbl[ p_wk->waiticon_draw ] );
    DEBUGWIN_RefreshScreen();
  }
}

#endif //DEBUGWIN_USE
