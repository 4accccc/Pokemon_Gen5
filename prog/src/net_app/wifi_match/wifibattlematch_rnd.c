//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_core.c
 *	@brief	WIFIのバトルマッチコア画面  ランダムマッチシーケンス
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
#include "battle/btl_net.h" //BTL_NET_SERVER_VERSION
#include "net/dwc_rapcommon.h"

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
#include "savedata/my_pms_data.h"

//WIFIバトルマッチのモジュール
#include "wifibattlematch_graphic.h"
#include "wifibattlematch_view.h"
#include "wifibattlematch_net.h"
#include "wifibattlematch_util.h"
#include "wifibattlematch_snd.h"

//外部公開
#include "wifibattlematch_core.h"

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(dpw_common);

//-------------------------------------
///	デバッグマクロ
//=====================================
#ifdef PM_DEBUG
//#define DEBUGWIN_USE

#if defined(DEBUG_ONLY_FOR_toru_nagihashi)||defined(DEBUG_ONLY_FOR_shimoyamada)
#define DEBUG_GPF_PASS
#endif

//#define DEBUG_DIRTYCHECK_PASS
//#define SAKE_REPORT_NONE          //レポートをしない

#ifndef SAKE_REPORT_NONE
#define SAKE_REPORT_HEAP_DIVIDE   //レポート用ヒープを切り分ける
#endif 

#define DEBUG_GPF_PASS
#endif //PM_DEBUG


//デバッグWINインクルード
#ifdef DEBUGWIN_USE
//デバッグ
#include "debug/debugwin_sys.h"
#endif//DEBUGWIN_USE

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	シンク
//=====================================
#define ENEMYDATA_WAIT_SYNC    (180)
#define MATCHING_MSG_WAIT_SYNC (120)

//-------------------------------------
///	シンク
//=====================================
#define WBM_RND_RATE_HEAP_SIZE  (0x30000)

//-------------------------------------
///	サブシーケンス戻り値
//=====================================
typedef enum
{
  WBM_RND_SUBSEQ_EVILCHECK_RET_SUCCESS  = 0,  //成功
  WBM_RND_SUBSEQ_EVILCHECK_RET_DARTY,         //１体でも不正ポケモンがいた
  WBM_RND_SUBSEQ_EVILCHECK_RET_NET_ERR,       //ネットエラー

  WBM_RND_SUBSEQ_RET_NONE  = 0xFFFF,          //なし
} WBM_RND_SUBSEQ_RET;


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
	GFL_FONT			                *p_font;

	//共通で使うキュー
	PRINT_QUE				              *p_que;

	//共通で使うメッセージ
	GFL_MSGDATA	          	    	*p_msg;

	//共通で使う単語
	WORDSET				              	*p_word;

	//上画面情報
	PLAYERINFO_WORK             	*p_playerinfo;

	//対戦者情報
	MATCHINFO_WORK              	*p_matchinfo;

  //待機アイコン
  WBM_WAITICON_WORK             *p_wait;

  //テキスト面
  WBM_TEXT_WORK                 *p_text;

  //リスト
  WBM_LIST_WORK                 *p_list;
  u32                           list_type;

  //ネット
  WIFIBATTLEMATCH_NET_WORK      *p_net;

  //メインシーケンス
  WBM_SEQ_WORK                  *p_seq;

  //サブシーケンス
  WBM_SEQ_WORK                  *p_subseq;
  WBM_RND_SUBSEQ_RET            subseq_ret;

  //引数
  WIFIBATTLEMATCH_CORE_PARAM    *p_param;

  //サーバーからの受信バッファ
  WIFIBATTLEMATCH_GDB_RND_SCORE_DATA  rnd_score;

  //不正チェックのときに使用するバッファ（それ以外は参照しない）
  WIFIBATTLEMATCH_NET_EVILCHECK_DATA  evilecheck_data;

  //GFPの状態
  DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA gpf_data;

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
static void WbmRndSeq_Rate_EndRec( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Rate_CupContinue( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Rate_CupEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//フリー処理
static void WbmRndSeq_Free_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Free_Matching( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Free_EndBattle( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Free_EndRec( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Free_CupContinue( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmRndSeq_Free_CupEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//エラー処理
static void WbmRndSeq_Err_ReturnLogin( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//切断
static void WbmRndSeq_DisConnectEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	サブシーケンス関数
//   Util_SubSeq_Start等を使い、シーケンス上で動くもの
//=====================================
static void WbmRndSubSeq_EvilCheck( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	便利
//=====================================
//自分のカード作成
static void Util_PlayerInfo_Create( WIFIBATTLEMATCH_RND_WORK *p_wk, WIFIBATTLEMATCH_CORE_RETMODE mode );
static void Util_PlayerInfo_Delete( WIFIBATTLEMATCH_RND_WORK *p_wk );
static BOOL Util_PlayerInfo_Move( WIFIBATTLEMATCH_RND_WORK *p_wk );
//対戦相手のカード作成
static void Util_MatchInfo_Create( WIFIBATTLEMATCH_RND_WORK *p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_enemy_data );
static void Util_MatchInfo_Delete( WIFIBATTLEMATCH_RND_WORK *p_wk );
static BOOL Util_MatchInfo_Main( WIFIBATTLEMATCH_RND_WORK *p_wk );
//スコアセーブ
static void Util_SaveRateScore( RNDMATCH_DATA *p_save, WIFIBATTLEMATCH_BTLRULE btl_rule, const WIFIBATTLEMATCH_RND_WORK *cp_wk );
static void Util_SaveFreeScore( RNDMATCH_DATA *p_rndmatch, WIFIBATTLEMATCH_BTLRULE btl_rule, BtlResult btl_result );
//選択肢作成
typedef enum
{ 
  UTIL_LIST_TYPE_YESNO,
  UTIL_LIST_TYPE_FREERATE,
}UTIL_LIST_TYPE;
static void Util_List_Create( WIFIBATTLEMATCH_RND_WORK *p_wk, UTIL_LIST_TYPE type );
static void Util_List_Delete( WIFIBATTLEMATCH_RND_WORK *p_wk );
static u32 Util_List_Main( WIFIBATTLEMATCH_RND_WORK *p_wk );
//マッチング用データ作成
static void Util_Matchkey_SetData( WIFIBATTLEMATCH_MATCH_KEY_DATA *p_data, const WIFIBATTLEMATCH_RND_WORK *cp_wk );
//自分のデータ作成
static void Util_InitMyData( WIFIBATTLEMATCH_ENEMYDATA *p_my_data, WIFIBATTLEMATCH_RND_WORK *p_wk );
static void Util_RenewalMyData( WIFIBATTLEMATCH_ENEMYDATA *p_my_data, WIFIBATTLEMATCH_RND_WORK *p_wk );
//サブシーケンス
static void Util_SubSeq_Start( WIFIBATTLEMATCH_RND_WORK *p_wk, WBM_SEQ_FUNCTION seq_function );
static WBM_RND_SUBSEQ_RET Util_SubSeq_Main( WIFIBATTLEMATCH_RND_WORK *p_wk );
//ポケモン証明
static BOOL Util_VerifyPokeData( WIFIBATTLEMATCH_ENEMYDATA *p_data, HEAPID heapID );

//-------------------------------------
///	デバッグ
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
const GFL_PROC_DATA	WifiBattleMatchRnd_ProcData =
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
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_CORE, 0x35000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_RND_WORK), HEAPID_WIFIBATTLEMATCH_CORE );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_RND_WORK) );	
  p_wk->p_param = p_param;

	//グラフィック設定
	p_wk->p_graphic	= WIFIBATTLEMATCH_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_WIFIBATTLEMATCH_CORE );

  //リソース読み込み
  p_wk->p_res     = WIFIBATTLEMATCH_VIEW_LoadResource( WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic ), WIFIBATTLEMATCH_VIEW_RES_MODE_RANDOM, HEAPID_WIFIBATTLEMATCH_CORE );

  //自分のデータを設定
  Util_InitMyData( p_param->p_player_data, p_wk );

	//共通モジュールの作成
	p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
												NARC_message_wifi_match_dat, HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_word	= WORDSET_CreateEx( WORDSET_DEFAULT_SETNUM, WORDSET_COUNTRY_BUFLEN, HEAPID_WIFIBATTLEMATCH_CORE );

	//モジュールの作成
  p_wk->p_net   = WIFIBATTLEMATCH_NET_Init( p_param->p_param->p_game_data, p_param->p_svl_result, HEAPID_WIFIBATTLEMATCH_CORE );
  p_wk->p_text  = WBM_TEXT_Init( BG_FRAME_M_TEXT, PLT_FONT_M, PLT_TEXT_M, CGR_OFS_M_TEXT, p_wk->p_que, p_wk->p_font, HEAPID_WIFIBATTLEMATCH_CORE );
  p_wk->p_seq   = WBM_SEQ_Init( p_wk, WbmRndSeq_Init, HEAPID_WIFIBATTLEMATCH_CORE );
  p_wk->p_subseq   = WBM_SEQ_Init( p_wk, NULL, HEAPID_WIFIBATTLEMATCH_CORE );

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

  WIFIBATTLEMATCH_NETICON_SetDraw( p_wk->p_net, TRUE );
	
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
	WIFIBATTLEMATCH_RND_WORK	      *p_wk	= p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_param_adrs;

  WIFIBATTLEMATCH_NETICON_SetDraw( p_wk->p_net, FALSE );

#ifdef DEBUGWIN_USE
  DEBUGWIN_RemoveGroup( 0 );
  DEBUGWIN_ExitProc();
#endif

  PMSND_PopBGM();

	//モジュールの破棄
  WBM_WAITICON_Exit( p_wk->p_wait );
  WBM_SEQ_Exit( p_wk->p_subseq );
  WBM_SEQ_Exit( p_wk->p_seq );
  WBM_TEXT_Exit( p_wk->p_text );
  if( p_wk->p_net )
  { 
    WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
  }
  Util_MatchInfo_Delete( p_wk );
  Util_PlayerInfo_Delete( p_wk );

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
  case WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE_ERR:
    if( p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_RATE )
    { 
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_EndBattle );
    }
    else if( p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_FREE )
    { 
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_EndBattle );
    }
    break;

  case WIFIBATTLEMATCH_CORE_MODE_ENDREC:
    if( p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_RATE )
    { 
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_EndRec );
    }
    else if( p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_FREE )
    { 
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_EndRec );
    }
    break;
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
    SEQ_START_MSG_INIT,

    SEQ_START_RECV_GPF,
    SEQ_WAIT_RECV_GPF,

#if 0
    SEQ_START_INIT,
    SEQ_WAIT_INIT,

    SEQ_START_RECVDATA_SAKE,
    SEQ_WAIT_RECVDATA_SAKE,
#endif
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
  case SEQ_START_MSG_INIT:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_000, WBM_TEXT_TYPE_WAIT );
    *p_seq = SEQ_START_RECV_GPF;
    break;


  case SEQ_START_RECV_GPF:
    WIFIBATTLEMATCH_NET_StartRecvGpfData( p_wk->p_net, HEAPID_WIFIBATTLEMATCH_CORE );
    *p_seq = SEQ_WAIT_RECV_GPF;
    break;

  case SEQ_WAIT_RECV_GPF:
    switch( WIFIBATTLEMATCH_NET_WaitRecvGpfData( p_wk->p_net ) )
    { 
    case WIFIBATTLEMATCH_RECV_GPFDATA_RET_UPDATE:
      break;
    case WIFIBATTLEMATCH_RECV_GPFDATA_RET_SUCCESS:
      WIFIBATTLEMATCH_NET_GetRecvGpfData( p_wk->p_net, &p_wk->gpf_data );
      *p_seq = SEQ_CHECK_GPF;
      break;
    case WIFIBATTLEMATCH_RECV_GPFDATA_RET_DIRTY:
      //もらえなかったので、クリア
      GFL_STD_MemClear( &p_wk->gpf_data, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA) );
      *p_seq = SEQ_CHECK_GPF;
      break;
    case WIFIBATTLEMATCH_RECV_GPFDATA_RET_ERROR:
      //エラー処理
      switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
      { 
      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT: //切断しログインからやり直し
        /* fallthrough */
      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:     //復帰地点まで戻る
        WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
        break;
      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:      //電源切断
        NetErr_DispCallFatal();
        break;
      }
      break;
    }
    break;
#if 0
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
      SAVE_CONTROL_WORK *p_save = GAMEDATA_GetSaveControlWork(p_param->p_param->p_game_data);
      if( WIFIBATTLEMATCH_NET_WaitInitialize( p_wk->p_net, p_save ) )
      { 
        *p_seq = SEQ_START_RECVDATA_SAKE;
      }
      else
      { 
        //エラー処理
        switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT: //切断しログインからやり直し
          /* fallthrough */
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:     //復帰地点まで戻る
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
          break;
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:      //電源切断
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
#endif
    //-------------------------------------
    /// GPFチェック
    //=====================================
  case SEQ_CHECK_GPF:
    WBM_TEXT_EndWait( p_wk->p_text );

#ifdef DEBUG_GPF_PASS
    { 
      *p_seq = SEQ_SELECT_MSG;
    }
#else
    { 
      if( p_wk->gpf_data.signin )
      { 
        *p_seq = SEQ_SELECT_MSG;
      }
      else
      { 
        p_param->retmode = WIFIBATTLEMATCH_CORE_RETMODE_FREE;
        WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_Start );
        break;
      }
    }
#endif
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
    Util_List_Create( p_wk, UTIL_LIST_TYPE_FREERATE );
    *p_seq     = SEQ_WAIT_SELECT_MODE;
    break;

  case SEQ_WAIT_SELECT_MODE:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
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
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_WAIT_CANCEL_SELECT;
    break;

  case SEQ_WAIT_CANCEL_SELECT:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_DisConnectEnd );
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
    SEQ_START_RATE_MSG,
    SEQ_START_RECVRATE_SAKE,
    SEQ_WAIT_RECVRATE_SAKE,
    SEQ_WAIT_CARDIN,
    SEQ_START_POKECHECK_SERVER,
    SEQ_WAIT_POKECHECK_SERVER,
    SEQ_START_DIRTY_MSG,
    SEQ_DIRTY_END,
    SEQ_END,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	      *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START_RATE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_018, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_RECVRATE_SAKE );
    break;

    //-------------------------------------
    ///	レーティングデータ受信
    //=====================================
  case SEQ_START_RECVRATE_SAKE:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_000, WBM_TEXT_TYPE_WAIT );
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_MYRECORD, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->rnd_score );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_WAIT_RECVRATE_SAKE );
    break;

  case SEQ_WAIT_RECVRATE_SAKE:
    { 
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net ) )
      { 

        //自分のレートを保存
        { 
          WIFIBATTLEMATCH_ENEMYDATA *p_player;
          p_player  = p_param->p_player_data;
          p_player->rate  = p_wk->rnd_score.single_rate;
        }

        //自分の情報を表示
        Util_PlayerInfo_Create( p_wk, WIFIBATTLEMATCH_CORE_RETMODE_RATE );

        *p_seq       = SEQ_WAIT_CARDIN;
      }
      else
      { 
        //エラー
        switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:       //戻る
          WBM_TEXT_EndWait( p_wk->p_text );
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Start );
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          WBM_TEXT_EndWait( p_wk->p_text );
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
          break;
        }
      }
    }
    break;


  case SEQ_WAIT_CARDIN:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
#ifdef DEBUG_DIRTYCHECK_PASS
      *p_seq       = SEQ_END;
#else
      *p_seq       = SEQ_START_POKECHECK_SERVER;
#endif //DEBUG_DIRTYCHECK_PASS
    }
    break;

    //-------------------------------------
    /// ポケモン不正チェック
    //=====================================
  case SEQ_START_POKECHECK_SERVER:
    Util_SubSeq_Start( p_wk, WbmRndSubSeq_EvilCheck );
    *p_seq       = SEQ_WAIT_POKECHECK_SERVER;
    break;

  case SEQ_WAIT_POKECHECK_SERVER:
    { 
      WBM_RND_SUBSEQ_RET ret = Util_SubSeq_Main( p_wk );
      if( ret == WBM_RND_SUBSEQ_EVILCHECK_RET_SUCCESS )
      { 
        WBM_TEXT_EndWait( p_wk->p_text );
        *p_seq       = SEQ_END;
      }
      else if( ret == WBM_RND_SUBSEQ_EVILCHECK_RET_DARTY )
      {   
        WBM_TEXT_EndWait( p_wk->p_text );
        *p_seq       = SEQ_START_DIRTY_MSG;
      }

      //エラー
      switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
      { 
      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:       //戻る
        WBM_TEXT_EndWait( p_wk->p_text );
        WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Start );
        break;

      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
        WBM_TEXT_EndWait( p_wk->p_text );
        WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
        break;
      }
    }
    break;
    
  case SEQ_START_DIRTY_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_ERR_00, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_DIRTY_END );
    break;

  case SEQ_DIRTY_END:
    WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_DisConnectEnd );
    break;

  case SEQ_END:
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
    SEQ_START_SENDDATA,
    SEQ_WAIT_SENDDATA,
    SEQ_CHECK_DARTYDATA,

    SEQ_OK_MATCHING_MSG,
    SEQ_OK_MATCHING_WAIT,
    SEQ_WAIT_MATCH_CARDIN,
    SEQ_CARDIN_WAIT,

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
    { 
      WIFIBATTLEMATCH_MATCH_KEY_DATA  data;
      Util_Matchkey_SetData( &data, p_wk );
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, TRUE );
      PMSND_PlaySE( WBM_SND_SE_MATCHING );
      WIFIBATTLEMATCH_NET_StartMatchMake( p_wk->p_net, WIFIBATTLEMATCH_TYPE_RNDRATE, TRUE, p_param->p_param->btl_rule, &data );
      *p_seq = SEQ_START_MATCHING_MSG;
    }
    break;

  case SEQ_START_MATCHING_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_009, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MATCHING;
    break;

  case SEQ_WAIT_MATCHING:
    //マッチングキャンセル処理
    if( WIFIBATTLEMATCH_NET_GetSeqMatchMake( p_wk->p_net ) < WIFIBATTLEMATCH_NET_SEQ_CONNECT_START )
    { 
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      { 
        *p_seq = SEQ_START_SELECT_CANCEL_MSG;
      }
    }

    //マッチング終了待ち
    if( WIFIBATTLEMATCH_NET_WaitMatchMake( p_wk->p_net ) )
    { 
      *p_seq = SEQ_START_SENDDATA;
    }

    //マッチングエラー
    switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
    { 
    case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:       //戻る
      PMSND_StopSE();
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Start );
      break;

    case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
      PMSND_StopSE();
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
      break;
    }

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
      WIFIBATTLEMATCH_ENEMYDATA *p_recv;
      if( WIFIBATTLEMATCH_NET_WaitEnemyData( p_wk->p_net, &p_recv ) )
      {   
        u32 dirty;
        WIFIBATTLEMATCH_ENEMYDATA *p_enemy_data = p_param->p_enemy_data;
        GFL_STD_MemCopy( p_recv, p_enemy_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
        dirty = WIFIBATTLEMATCH_DATA_ModifiEnemyData( p_enemy_data, HEAPID_WIFIBATTLEMATCH_CORE );

        if( dirty == 0 )
        { 
          SAVE_CONTROL_WORK *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
          WIFI_NEGOTIATION_SAVEDATA* pSV  = WIFI_NEGOTIATION_SV_GetSaveData(p_sv_ctrl);
          WIFI_NEGOTIATION_SV_SetFriend(pSV, (MYSTATUS*)p_param->p_enemy_data->mystatus );
        }

        *p_seq  = SEQ_CHECK_DARTYDATA;
      }
    }
    break;

  case SEQ_CHECK_DARTYDATA:
#ifdef DEBUG_DIRTYCHECK_PASS
    if( 1 )
#else
    if( Util_VerifyPokeData( p_param->p_enemy_data, HEAPID_WIFIBATTLEMATCH_CORE ) )
#endif
    { 
      //対戦者情報表示
      Util_MatchInfo_Create( p_wk, p_param->p_enemy_data );
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
      *p_seq  = SEQ_OK_MATCHING_MSG;
    }
    else
    { 
      //不正なのでマッチングに戻る
      WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE );
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_Start );
    }
    break;

  case SEQ_OK_MATCHING_MSG:
    PMSND_StopSE();
    PMSND_PlaySE( WBM_SND_SE_MATCHING_OK );

    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_010, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_OK_MATCHING_WAIT );
    break;

  case SEQ_OK_MATCHING_WAIT:
    if( p_wk->cnt++ > MATCHING_MSG_WAIT_SYNC )
    { 
      p_wk->cnt      = 0;
      GFL_BG_SetVisible( BG_FRAME_M_TEXT, FALSE );
      *p_seq      = SEQ_WAIT_MATCH_CARDIN;
    }
    break;
  case SEQ_WAIT_MATCH_CARDIN:
    if( Util_MatchInfo_Main( p_wk ) )
    { 
      *p_seq  = SEQ_CARDIN_WAIT;
    }
    break;
  case SEQ_CARDIN_WAIT:
    if( p_wk->cnt++ > ENEMYDATA_WAIT_SYNC )
    { 
      p_wk->cnt      = 0;
      *p_seq      = SEQ_END_MATCHING_MSG;
    }
    break;

  case SEQ_END_MATCHING_MSG:
    GFL_BG_SetVisible( BG_FRAME_M_TEXT, TRUE );
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
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_WAIT_SELECT_CANCEL;
    break;

  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0:
          WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
          PMSND_PlaySE( WBM_SND_SE_MATCHING );
          WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE );
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_CupEnd );
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
    SEQ_SC_HEAP_INIT,
    SEQ_START_MSG,
    SEQ_START_REPORT_ATLAS,
    SEQ_WAIT_REPORT_ATLAS,

    SEQ_SC_HEAP_EXIT,

    SEQ_START_RECVDATA_SAKE,
    SEQ_WAIT_RECVDATA_SAKE,

    SEQ_START_SAVE_MSG,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,

    SEQ_WAIT_CARDIN,
    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_SC_HEAP_INIT:
#ifdef SAKE_REPORT_HEAP_DIVIDE
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_SC, WBM_RND_RATE_HEAP_SIZE );
    DWC_RAPCOMMON_SetSubHeapID( DWC_ALLOCTYPE_GS, WBM_RND_RATE_HEAP_SIZE, HEAPID_WIFIBATTLEMATCH_SC );
#endif //SAKE_REPORT_HEAP_DIVIDE
    *p_seq  = SEQ_START_MSG;
    break;
    //-------------------------------------
    ///	Atlasへの書き込み
    //=====================================
  case SEQ_START_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_013, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_REPORT_ATLAS );
    break;
  case SEQ_START_REPORT_ATLAS:
#ifndef SAKE_REPORT_NONE
    WIFIBATTLEMATCH_SC_Start( p_wk->p_net, WIFIBATTLEMATCH_TYPE_RNDRATE, p_param->p_param->btl_rule, p_param->btl_result );
#endif
    *p_seq = SEQ_WAIT_REPORT_ATLAS;
    break;
  case SEQ_WAIT_REPORT_ATLAS:
    { 
#ifndef SAKE_REPORT_NONE
      if( WIFIBATTLEMATCH_SC_Process( p_wk->p_net ) )
#endif
      { 
        *p_seq = SEQ_SC_HEAP_EXIT;
      }

      //エラー
      switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
      { 
      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:     //戻る
        *p_seq = SEQ_START_SAVE_MSG;
        break;

      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
        WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
        break;
      }
    }
    break;

  case SEQ_SC_HEAP_EXIT:
#ifdef SAKE_REPORT_HEAP_DIVIDE
    DWC_RAPCOMMON_ResetSubHeapID();
    GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH_SC );
#endif //SAKE_REPORT_HEAP_DIVIDE
    *p_seq = SEQ_START_RECVDATA_SAKE;
    break;

    //-------------------------------------
    ///	Sakeから取得
    //=====================================
  case SEQ_START_RECVDATA_SAKE:
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_MYRECORD, WIFIBATTLEMATCH_GDB_GET_RND_SCORE, &p_wk->rnd_score );
    *p_seq = SEQ_WAIT_RECVDATA_SAKE;
    break;
  case SEQ_WAIT_RECVDATA_SAKE:
    { 
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net ) )
      { 
        //セーブデータ
        Util_SaveRateScore( p_param->p_rndmatch, p_param->p_param->btl_rule, p_wk );

        //対戦者情報表示
        Util_PlayerInfo_Create( p_wk, WIFIBATTLEMATCH_CORE_RETMODE_RATE );

        *p_seq       = SEQ_WAIT_CARDIN;
      }
      else
      { 
        //エラー
        switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
        { 
        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:       //もう一度
          *p_seq = SEQ_START_SAVE_MSG;
          break;

        case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
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
      *p_seq = SEQ_START_SAVE_MSG;
    }
    break;

    //-------------------------------------
    /// レーティングセーブ処理
    //=====================================
  case SEQ_START_SAVE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_017, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SAVE );
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
        p_param->result = WIFIBATTLEMATCH_CORE_RESULT_NEXT_REC;
        WBM_SEQ_End( p_seqwk ); 
        break;
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
 *	@brief  ランダムマッチ  レーティングモード録画からの戻り
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Rate_EndRec( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START:
    WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_CupContinue );
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
 *	@brief  ランダムマッチ  たいせんを続けますか？
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Rate_CupContinue( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_SELECT_CONTINUE_MSG,
    SEQ_START_SELECT_CONTINUE,
    SEQ_WAIT_SELECT_CONTINUE,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    /// 対戦続行確認
    //=====================================
  case SEQ_START_SELECT_CONTINUE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_015, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CONTINUE );
    break;
  case SEQ_START_SELECT_CONTINUE:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_WAIT_SELECT_CONTINUE;
    break;
  case SEQ_WAIT_SELECT_CONTINUE:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0:
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_Matching );
          break;
        case 1:
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_CupEnd );
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
 *	@brief  ランダムマッチ  たいかいを終了しますか？
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Rate_CupEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    /// ランダムマッチ終了確認
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_016, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CANCEL );
    break;

  case SEQ_START_SELECT_CANCEL:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_WAIT_SELECT_CANCEL;
    break;
  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0:
          WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE );
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_DisConnectEnd );
          break;
        case 1:
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Rate_CupContinue );
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
//=============================================================================
/**
 *      フリーモード
 */
//=============================================================================
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

    SEQ_START_MSG_EVILCHECK,

    SEQ_START_SUBSEQ_EVILCHECK,
    SEQ_WAIT_SUBSEQ_EVILCHECK,
    SEQ_START_DIRTY_MSG,
    SEQ_DIRTY_END,
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
#ifdef DEBUG_DIRTYCHECK_PASS
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_Matching );
#else
      *p_seq  = SEQ_START_MSG_EVILCHECK;
#endif 
    }
    break;

    //-------------------------------------
    /// 不正チェック
    //=====================================

  case SEQ_START_MSG_EVILCHECK:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_000, WBM_TEXT_TYPE_WAIT );
    *p_seq  = SEQ_START_SUBSEQ_EVILCHECK;
    break;

  case SEQ_START_SUBSEQ_EVILCHECK:
    Util_SubSeq_Start( p_wk, WbmRndSubSeq_EvilCheck );
    *p_seq  = SEQ_WAIT_SUBSEQ_EVILCHECK;
    break;

  case SEQ_WAIT_SUBSEQ_EVILCHECK:
    { 
      WBM_RND_SUBSEQ_RET ret = Util_SubSeq_Main( p_wk );
      if( ret == WBM_RND_SUBSEQ_EVILCHECK_RET_SUCCESS )
      { 
        WBM_TEXT_EndWait( p_wk->p_text );
        WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_Matching );
      }
      else if( ret == WBM_RND_SUBSEQ_EVILCHECK_RET_DARTY )
      {   
        WBM_TEXT_EndWait( p_wk->p_text );
        *p_seq       = SEQ_START_DIRTY_MSG;
      }

      //エラー
      switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
      { 
      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:       //戻る
        WBM_TEXT_EndWait( p_wk->p_text );
        WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Start );
        break;

      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
        WBM_TEXT_EndWait( p_wk->p_text );
        WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
        break;
      }
    }
    break;
    
  case SEQ_START_DIRTY_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_ERR_00, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_DIRTY_END;
    break;

  case SEQ_DIRTY_END:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    {
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_DisConnectEnd );
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
    SEQ_INIT,
    SEQ_START_MATCHING_MSG,
    SEQ_START_MATCHING,
    SEQ_WAIT_MATCHING,
    SEQ_START_CHECK_CHEAT,
    SEQ_WAIT_CHECK_CHEAT,
    SEQ_START_SENDDATA,
    SEQ_WAIT_SENDDATA,

    SEQ_CHECK_DARTYDATA,
    SEQ_OK_MATCHING_MSG,
    SEQ_OK_MATCHING_WAIT,
    SEQ_WAIT_MATCH_CARDIN,
    SEQ_CARDIN_WAIT,
    SEQ_END_MATCHING_MSG,
    SEQ_END_MATCHING,

    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;
  switch( *p_seq )
  { 
    //-------------------------------------
    ///	マッチング開始
    //=====================================
  case SEQ_INIT:
    { 
      Util_RenewalMyData( p_param->p_player_data, p_wk );
      *p_seq = SEQ_START_MATCHING_MSG;
    }
    break;

  case SEQ_START_MATCHING_MSG:
    PMSND_PlaySE( WBM_SND_SE_MATCHING );
    WBM_WAITICON_SetDrawEnable( p_wk->p_wait, TRUE );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_009, WBM_TEXT_TYPE_WAIT );
    *p_seq = SEQ_START_MATCHING;
    break;

  case SEQ_START_MATCHING:
    { 
      WIFIBATTLEMATCH_MATCH_KEY_DATA  data;
      Util_Matchkey_SetData( &data, p_wk );
      WIFIBATTLEMATCH_NET_StartMatchMake( p_wk->p_net, WIFIBATTLEMATCH_TYPE_RNDFREE, FALSE, p_param->p_param->btl_rule, &data ); 
    }
    *p_seq  = SEQ_WAIT_MATCHING;
    break;

  case SEQ_WAIT_MATCHING:
    //マッチングキャンセル
    if( WIFIBATTLEMATCH_NET_GetSeqMatchMake( p_wk->p_net ) < WIFIBATTLEMATCH_NET_SEQ_CONNECT_START )
    { 
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      { 
        WBM_TEXT_EndWait( p_wk->p_text );
        *p_seq = SEQ_START_SELECT_CANCEL_MSG;
      }
    }

    //マッチング終了待ち
    if( WIFIBATTLEMATCH_NET_WaitMatchMake( p_wk->p_net ) )
    { 
      *p_seq = SEQ_START_SENDDATA;
    }

    //マッチングエラー
    switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
    { 
    case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:       //戻る
      PMSND_StopSE();
      WBM_TEXT_EndWait( p_wk->p_text );
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Start );
      break;

    case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
      PMSND_StopSE();
      WBM_TEXT_EndWait( p_wk->p_text );
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Err_ReturnLogin );
      break;
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
      WIFIBATTLEMATCH_ENEMYDATA *p_recv;
      if( WIFIBATTLEMATCH_NET_WaitEnemyData( p_wk->p_net, &p_recv ) )
      { 
        u32 dirty;
        WIFIBATTLEMATCH_ENEMYDATA *p_enemy_data = p_param->p_enemy_data;
        GFL_STD_MemCopy( p_recv, p_enemy_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
  

        dirty = WIFIBATTLEMATCH_DATA_ModifiEnemyData( p_enemy_data, HEAPID_WIFIBATTLEMATCH_CORE );

        if( dirty == 0 )
        { 
          SAVE_CONTROL_WORK *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
          WIFI_NEGOTIATION_SAVEDATA* pSV  = WIFI_NEGOTIATION_SV_GetSaveData(p_sv_ctrl);
          WIFI_NEGOTIATION_SV_SetFriend(pSV, (MYSTATUS*)p_param->p_enemy_data->mystatus );
        }

        *p_seq       = SEQ_CHECK_DARTYDATA;
      }
    }
    break;

  case SEQ_CHECK_DARTYDATA:
    WBM_TEXT_EndWait( p_wk->p_text );
#ifdef DEBUG_DIRTYCHECK_PASS
    if( 1 )
#else
    if( Util_VerifyPokeData( p_param->p_enemy_data, HEAPID_WIFIBATTLEMATCH_CORE ) )
#endif
    { 
      Util_MatchInfo_Create( p_wk, p_param->p_enemy_data );
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
      *p_seq       = SEQ_OK_MATCHING_MSG;
    }
    else
    { 
      //不正なのでマッチングに戻る
      WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE );
      WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_Matching );
    }
    break;

  case SEQ_OK_MATCHING_MSG:
    PMSND_StopSE();
    PMSND_PlaySE( WBM_SND_SE_MATCHING_OK );

    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_010, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_OK_MATCHING_WAIT );
    break;

  case SEQ_OK_MATCHING_WAIT:
    if( p_wk->cnt++ > MATCHING_MSG_WAIT_SYNC )
    { 
      p_wk->cnt      = 0;
      GFL_BG_SetVisible( BG_FRAME_M_TEXT, FALSE );
      *p_seq      = SEQ_WAIT_MATCH_CARDIN;
    }
    break;

  case SEQ_WAIT_MATCH_CARDIN:
    if( Util_MatchInfo_Main( p_wk ) )
    { 
      *p_seq  = SEQ_CARDIN_WAIT;
    }
    break;

  case SEQ_CARDIN_WAIT:
    if( p_wk->cnt++ > ENEMYDATA_WAIT_SYNC )
    { 
      p_wk->cnt      = 0;
      *p_seq      = SEQ_END_MATCHING_MSG;
    }
    break;

  case SEQ_END_MATCHING_MSG:
    GFL_BG_SetVisible( BG_FRAME_M_TEXT, TRUE );
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
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_WAIT_SELECT_CANCEL;
    break;

  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0:
          WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
          WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE );
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_CupEnd );
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
    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,

    SEQ_START_CARD,
    SEQ_WAIT_CARD,

    SEQ_START_SAVE_MSG,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    /// 切断処理
    //=====================================
  case SEQ_START_DISCONNECT:
    *p_seq = SEQ_WAIT_DISCONNECT;
    break;

  case SEQ_WAIT_DISCONNECT:
    if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE ) )
    { 
      //スコアをセーブ
      Util_SaveFreeScore( p_param->p_rndmatch, p_param->p_param->btl_rule, p_param->btl_result );
      *p_seq  = SEQ_START_CARD;
    }
    break;

  case SEQ_START_CARD:
    Util_PlayerInfo_Create( p_wk, WIFIBATTLEMATCH_CORE_RETMODE_FREE );
    *p_seq  = SEQ_WAIT_CARD;
    break;

  case SEQ_WAIT_CARD:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq  = SEQ_START_SAVE_MSG;
    }
    break;

    //-------------------------------------
    /// セーブ処理
    //=====================================
  case SEQ_START_SAVE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_017, WBM_TEXT_TYPE_WAIT);
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SAVE );
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
        WBM_TEXT_EndWait( p_wk->p_text );
        p_param->result = WIFIBATTLEMATCH_CORE_RESULT_NEXT_REC;
        WBM_SEQ_End( p_seqwk ); 
        break;
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
 *	@brief  ランダムマッチ  フリーモードの録画終了後
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Free_EndRec( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_CARD,
    SEQ_WAIT_CARD,

    SEQ_START,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START_CARD:
    Util_PlayerInfo_Create( p_wk, WIFIBATTLEMATCH_CORE_RETMODE_FREE );
    *p_seq  = SEQ_WAIT_CARD;
    break;

  case SEQ_WAIT_CARD:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq  = SEQ_START;
    }
    break;

  case SEQ_START:
    WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_CupContinue );
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
 *	@brief  ランダムマッチ  たいせんをつづけますか？
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Free_CupContinue( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_SELECT_CONTINUE_MSG,
    SEQ_START_SELECT_CONTINUE,
    SEQ_WAIT_SELECT_CONTINUE,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    /// 対戦続行確認
    //=====================================
  case SEQ_START_SELECT_CONTINUE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_015, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CONTINUE );
    break;
  case SEQ_START_SELECT_CONTINUE:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_WAIT_SELECT_CONTINUE;
    break;
  case SEQ_WAIT_SELECT_CONTINUE:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0:
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_Matching );
          break;
        case 1:
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_CupEnd );
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
 *	@brief  ランダムマッチ  たいかいをしゅうりょうしますか？
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_Free_CupEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_SELECT_CANCEL,
    SEQ_WAIT_SELECT_CANCEL,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
    //-------------------------------------
    /// ランダムマッチ終了確認
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_016, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECT_CANCEL );
    break;
  case SEQ_START_SELECT_CANCEL:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_WAIT_SELECT_CANCEL;
    break;
  case SEQ_WAIT_SELECT_CANCEL:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0:
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_DisConnectEnd );
          break;
        case 1:
          WBM_SEQ_SetNext( p_seqwk, WbmRndSeq_Free_CupContinue );
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
  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  p_param->result = WIFIBATTLEMATCH_CORE_RESULT_ERR_NEXT_LOGIN;
  WBM_SEQ_End( p_seqwk );
  
}
//----------------------------------------------------------------------------
/**
 *	@brief  切断して終了
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSeq_DisConnectEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
#if 0 //@todo rateのみ行なう
    SEQ_START_SEND_SAKE_TIME,
    SEQ_WAIT_SEND_SAKE_TIME,
#endif 

    SEQ_END,
  };
  WIFIBATTLEMATCH_RND_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
#if 0
  case SEQ_START_SEND_SAKE_TIME:
    WIFIBATTLEMATCH_GDB_StartWrite( p_wk->p_net, WIFIBATTLEMATCH_GDB_WRITE_MYINFO, NULL );
    *p_seq  = SEQ_WAIT_SEND_SAKE_TIME;
    break;

  case SEQ_WAIT_SEND_SAKE_TIME:
    {
      if( WIFIBATTLEMATCH_GDB_ProcessWrite( p_wk->p_net ) )
      { 
        *p_seq  = SEQ_END;
      }
    //エラーがあろうとなかろうと終了へいく
#if 0
      switch( WIFIBATTLEMATCH_NET_CheckErrorRepairType( p_wk->p_net, FALSE ) )
      { 
      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し
      case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:       //戻る
        *p_seq  = SEQ_END;
        break;
      }
#endif
    }
    break;
#endif 

  case SEQ_END:
    WBM_SEQ_End( p_seqwk );
    break;
  }
}
//=============================================================================
/**
 *    サブシーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  切断して終了
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_peq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmRndSubSeq_EvilCheck( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START,
    SEQ_START_EVIL_CHECK,
    SEQ_WAIT_EVIL_CHECK,
    SEQ_END_CHECK,
    SEQ_END,
  };

  WIFIBATTLEMATCH_RND_WORK	  *p_wk	      = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param    = p_wk->p_param;
  WIFIBATTLEMATCH_ENEMYDATA   *p_my_data  = p_param->p_player_data;

  switch( *p_seq )
  { 
  case SEQ_START:
    p_wk->cnt = 0;
    GFL_STD_MemClear( &p_wk->evilecheck_data, sizeof(WIFIBATTLEMATCH_NET_EVILCHECK_DATA ));
    (*p_seq)++;
    break;

  case SEQ_START_EVIL_CHECK:
    { 
      WIFIBATTLEMATCH_NET_StartEvilCheck( p_wk->p_net, p_my_data->pokeparty, WIFIBATTLEMATCH_NET_EVILCHECK_TYPE_PARTY );
      (*p_seq)++;
      NAGI_Printf( "EvilCheck Start\n" );
    }
    break;

  case SEQ_WAIT_EVIL_CHECK:
    {
      WIFIBATTLEMATCH_NET_EVILCHECK_RET ret;
      ret = WIFIBATTLEMATCH_NET_WaitEvilCheck( p_wk->p_net, &p_wk->evilecheck_data );
      if( ret == WIFIBATTLEMATCH_NET_EVILCHECK_RET_SUCCESS )
      { 
        NAGI_Printf( "EvilCheck Success\n" );
        (*p_seq)  = SEQ_END_CHECK;
      }
      else if( ret == WIFIBATTLEMATCH_NET_EVILCHECK_RET_ERROR )
      { 
        NAGI_Printf( "EvilCheck Error\n" );
        p_wk->subseq_ret  = WBM_RND_SUBSEQ_EVILCHECK_RET_NET_ERR;
        *p_seq  = SEQ_END;
      }
    }
    break;

  case SEQ_END_CHECK:
    //データを格納
    OS_TPrintf( "ポケモン結果[不正番号%d]status=%d\n", p_wk->evilecheck_data.poke_result[0], p_wk->evilecheck_data.status_code );
    if( p_wk->evilecheck_data.status_code == 0 )
    { 
      //署名を保存
      GFL_STD_MemCopy( p_wk->evilecheck_data.sign, p_my_data->sign, NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN );

      NAGI_Printf( "EvilCheck 成功！！\n" );
      p_wk->subseq_ret  = WBM_RND_SUBSEQ_EVILCHECK_RET_SUCCESS;
      *p_seq  = SEQ_END;
    }
    else
    { 
      NAGI_Printf( "EvilCheck 不正！！\n" );
      p_wk->subseq_ret  = WBM_RND_SUBSEQ_EVILCHECK_RET_DARTY;
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    WBM_SEQ_End( p_seqwk );
    break;
  }


}
//=============================================================================
/**
 *    便利関数
 */
//=============================================================================
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
    info_setup.trainerID  = MyStatus_GetTrainerView( p_my );
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
    BOOL is_rate  = p_wk->p_param->retmode  == WIFIBATTLEMATCH_CORE_RETMODE_RATE;

    GFL_CLUNIT  *p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
    p_wk->p_matchinfo		= MATCHINFO_Init( cp_enemy_data, p_unit, p_wk->p_res, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, WIFIBATTLEMATCH_TYPE_RNDRATE, is_rate, HEAPID_WIFIBATTLEMATCH_CORE );
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
static void Util_SaveRateScore( RNDMATCH_DATA *p_save, WIFIBATTLEMATCH_BTLRULE btl_rule, const WIFIBATTLEMATCH_RND_WORK *cp_wk )
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

//----------------------------------------------------------------------------
/**
 *	@brief  セーブ  フリーモード用
 *
 *	@param	RNDMATCH_DATA *p_save セーブ
 *	@param	mode                  ルール
 *	@param	WIFIBATTLEMATCH_RND_WORK *cp_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_SaveFreeScore( RNDMATCH_DATA *p_rndmatch, WIFIBATTLEMATCH_BTLRULE btl_rule, BtlResult btl_result )
{ 
  switch( btl_result )
  {
  case BTL_RESULT_WIN:
  case BTL_RESULT_RUN_ENEMY:
    RNDMATCH_AddParam( p_rndmatch, btl_rule, RNDMATCH_PARAM_IDX_WIN );
    break;
  case BTL_RESULT_LOSE:
  case BTL_RESULT_RUN:
    RNDMATCH_AddParam( p_rndmatch, btl_rule, RNDMATCH_PARAM_IDX_LOSE );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk  ワーク
 *	@param	type                            リストの種類
 */
//-----------------------------------------------------------------------------
static void Util_List_Create( WIFIBATTLEMATCH_RND_WORK *p_wk, UTIL_LIST_TYPE type )
{ 
  if( p_wk->p_list == NULL )
  { 
    WBM_LIST_SETUP  setup;
    GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
    setup.p_msg   = p_wk->p_msg;
    setup.p_font  = p_wk->p_font;
    setup.p_que   = p_wk->p_que;
    setup.frm     = BG_FRAME_M_TEXT;
    setup.font_plt= PLT_FONT_M;
    setup.frm_plt = PLT_LIST_M;
    setup.frm_chr = CGR_OFS_M_LIST;

    switch( type )
    {
    case UTIL_LIST_TYPE_YESNO:
      setup.strID[0]= WIFIMATCH_TEXT_007;
      setup.strID[1]= WIFIMATCH_TEXT_008;
      setup.list_max= 2;
      break;

    case UTIL_LIST_TYPE_FREERATE:
      setup.strID[0]= WIFIMATCH_TEXT_003;
      setup.strID[1]= WIFIMATCH_TEXT_004;
      setup.strID[2]= WIFIMATCH_TEXT_005;
      setup.list_max= 3;
      break;
    }

    p_wk->list_type = type;
    p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト破棄
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_List_Delete( WIFIBATTLEMATCH_RND_WORK *p_wk )
{
  if( p_wk->p_list )
  { 
    WBM_LIST_Exit( p_wk->p_list );
    p_wk->p_list  = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リストメイン
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk ワーク
 *
 *	@return 選択したもの
 */
//-----------------------------------------------------------------------------
static u32 Util_List_Main( WIFIBATTLEMATCH_RND_WORK *p_wk )
{ 
  if( p_wk->p_list )
  { 
    u32 ret = WBM_LIST_Main( p_wk->p_list );

    if( p_wk->list_type == UTIL_LIST_TYPE_YESNO )
    { 
      if( ret == BMPMENULIST_CANCEL )
      { 
        ret = 1;
      }
    }
    else
    { 
      if( ret == BMPMENULIST_CANCEL )
      { 
        ret = BMPMENULIST_NULL;
      }
    }
    
    return  ret;
  }
  else
  { 
    NAGI_Printf( "List not exist !!!\n" );
    return WBM_LIST_SELECT_NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  マッチング用データ設定
 *
 *	@param	WIFIBATTLEMATCH_MATCH_KEY_DATA *p_data  データ
 *	@param	WIFIBATTLEMATCH_RND_WORK *cp_wk         ワーク
 */
//-----------------------------------------------------------------------------
static void Util_Matchkey_SetData( WIFIBATTLEMATCH_MATCH_KEY_DATA *p_data, const WIFIBATTLEMATCH_RND_WORK *cp_wk )
{ 
  GFL_STD_MemClear( p_data, sizeof(WIFIBATTLEMATCH_MATCH_KEY_DATA) );

  p_data->disconnect = cp_wk->rnd_score.disconnect;
  p_data->cup_no = 1;

  switch( cp_wk->p_param->p_param->btl_rule )
  { 
  case WIFIBATTLEMATCH_BTLRULE_SINGLE:  
    p_data->rate    = cp_wk->rnd_score.single_rate;
    break;

  case WIFIBATTLEMATCH_BTLRULE_DOUBLE:
    p_data->rate     = cp_wk->rnd_score.double_rate;
    break;

  case WIFIBATTLEMATCH_BTLRULE_TRIPLE:
    p_data->rate     = cp_wk->rnd_score.triple_rate;
    break;

  case WIFIBATTLEMATCH_BTLRULE_ROTATE:
    p_data->rate     = cp_wk->rnd_score.rot_rate;
    break;

  case WIFIBATTLEMATCH_BTLRULE_SHOOTER:
    p_data->rate     = cp_wk->rnd_score.shooter_rate;
    break;
  } 

}

//----------------------------------------------------------------------------
/**
 *	@brief  自分のデータを格納 
 *
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *p_my_data    データ
 *	@param	WIFIBATTLEMATCH_RND_WORK *cp_wk         ワーク
 */
//-----------------------------------------------------------------------------
static void Util_InitMyData( WIFIBATTLEMATCH_ENEMYDATA *p_my_data, WIFIBATTLEMATCH_RND_WORK *p_wk )
{ 
  GAMEDATA  *p_game_data  = p_wk->p_param->p_param->p_game_data;

  p_my_data->btl_server_version  = BTL_NET_SERVER_VERSION;
  {
    MYSTATUS  *p_my;
    p_my  = GAMEDATA_GetMyStatus(p_game_data);
    GFL_STD_MemCopy( p_my, p_my_data->mystatus, MyStatus_GetWorkSize() );
  }
  { 
    const MYPMS_DATA *cp_mypms;
    SAVE_CONTROL_WORK *p_sv;
    p_sv            = GAMEDATA_GetSaveControlWork(p_game_data);
    cp_mypms        = SaveData_GetMyPmsDataConst( p_sv );
    MYPMS_GetPms( cp_mypms, MYPMS_PMS_TYPE_INTRODUCTION, &p_my_data->pms );
  }
  { 
    if( p_wk->p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_RATE )
    { 
      p_my_data->win_cnt  = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, 
          RNDMATCH_TYPE_RATE_SINGLE + p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_WIN );
      p_my_data->lose_cnt = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, 
          RNDMATCH_TYPE_RATE_SINGLE + p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_LOSE );
    }
    else
    { 
      p_my_data->win_cnt  = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, 
          RNDMATCH_TYPE_FREE_SINGLE + p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_WIN );
      p_my_data->lose_cnt = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, 
          RNDMATCH_TYPE_FREE_SINGLE + p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_LOSE );
    }
  }
  {
    POKEPARTY *p_temoti;
    switch( p_wk->p_param->p_param->poke )
    {
    case WIFIBATTLEMATCH_POKE_TEMOTI:
      p_temoti = GAMEDATA_GetMyPokemon(p_game_data);
      GFL_STD_MemCopy( p_temoti, p_my_data->pokeparty, PokeParty_GetWorkSize() );
      break;

    case WIFIBATTLEMATCH_POKE_BTLBOX:
      { 
        SAVE_CONTROL_WORK*	p_sv	= GAMEDATA_GetSaveControlWork(p_game_data);
        BATTLE_BOX_SAVE *p_btlbox_sv = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
        p_temoti  = BATTLE_BOX_SAVE_MakePokeParty( p_btlbox_sv, GFL_HEAP_LOWID(HEAPID_WIFIBATTLEMATCH_SYS) );
        GFL_STD_MemCopy( p_temoti, p_my_data->pokeparty, PokeParty_GetWorkSize() );
        GFL_HEAP_FreeMemory( p_temoti );
      }
      break;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のデータ更新
 *
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *p_my_data  ワーク
 *	@param	*p_wk   ワーク
 */
//-----------------------------------------------------------------------------
static void Util_RenewalMyData( WIFIBATTLEMATCH_ENEMYDATA *p_my_data, WIFIBATTLEMATCH_RND_WORK *p_wk )
{ 
  { 
    if( p_wk->p_param->retmode == WIFIBATTLEMATCH_CORE_RETMODE_RATE )
    { 
      p_my_data->win_cnt  = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, 
          RNDMATCH_TYPE_RATE_SINGLE + p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_WIN );
      p_my_data->lose_cnt = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, 
          RNDMATCH_TYPE_RATE_SINGLE + p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_LOSE );
    }
    else
    { 
      p_my_data->win_cnt  = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, 
          RNDMATCH_TYPE_FREE_SINGLE + p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_WIN );
      p_my_data->lose_cnt = RNDMATCH_GetParam( p_wk->p_param->p_rndmatch, 
          RNDMATCH_TYPE_FREE_SINGLE + p_wk->p_param->p_param->btl_rule, RNDMATCH_PARAM_IDX_LOSE );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  サブシーケンス開始
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk  ワーク
 *	@param	seq_function                      サブシーケンス
 */
//-----------------------------------------------------------------------------
static void Util_SubSeq_Start( WIFIBATTLEMATCH_RND_WORK *p_wk, WBM_SEQ_FUNCTION seq_function )
{ 
  WBM_SEQ_SetNext( p_wk->p_subseq, seq_function );
}
//----------------------------------------------------------------------------
/**
 *	@brief  サブシーケンス中
 *
 *	@param	WIFIBATTLEMATCH_RND_WORK *p_wk  ワーク
 *	@return WBM_RND_SUBSEQ_RET_NONE動作中  それ以外のときはサブシーケンスが終了し戻り値を返している
 *
 *	@return 戻り値
 */
//-----------------------------------------------------------------------------
static WBM_RND_SUBSEQ_RET Util_SubSeq_Main( WIFIBATTLEMATCH_RND_WORK *p_wk )
{ 
  WBM_SEQ_Main( p_wk->p_subseq );
  if( WBM_SEQ_IsEnd( p_wk->p_subseq ) )
  {
    return p_wk->subseq_ret;
  }
  else
  { 
    return WBM_RND_SUBSEQ_RET_NONE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン証明
 *
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *p_my_data  証明したい相手のデータ
 *	@param	heapID  ヒープID
 *
 *	@return TRUEで証明  FALSEで不正
 */
//-----------------------------------------------------------------------------
static BOOL Util_VerifyPokeData( WIFIBATTLEMATCH_ENEMYDATA *p_data, HEAPID heapID )
{ 
  int i;
  void *p_buff;
  POKEPARTY *p_party  = (POKEPARTY *)p_data->pokeparty;
  const POKEMON_PARAM *cp_pp;
  BOOL ret;

  p_buff  = NHTTP_RAP_EVILCHECK_CreateVerifyPokeBuffer( POKETOOL_GetWorkSize(), PokeParty_GetPokeCount(p_party) , GFL_HEAP_LOWID(heapID) );

  for( i = 0; i < PokeParty_GetPokeCount( p_party ); i++ )
  { 
    cp_pp  = PokeParty_GetMemberPointer( p_party, i);
    NHTTP_RAP_EVILCHECK_AddPokeVerifyPokeBuffer( p_buff, cp_pp, POKETOOL_GetWorkSize(), i );
    OS_TPrintf( "monsno %d\n", PP_Get( cp_pp, ID_PARA_monsno, NULL) );
    OS_TPrintf( "rnd %d\n", PP_Get( cp_pp, ID_PARA_personal_rnd, NULL) );
    OS_TPrintf( "crc %d\n", GFL_STD_CrcCalc( cp_pp, POKETOOL_GetWorkSize() ) );
  }

  ret = NHTTP_RAP_EVILCHECK_VerifySign( p_buff, POKETOOL_GetWorkSize(), PokeParty_GetPokeCount(p_party) , p_data->sign, GFL_HEAP_LOWID(heapID) );

  NHTTP_RAP_EVILCHECK_DeleteVerifyPokeBuffer( p_buff );

  NAGI_Printf( "不正チェック通過？=[%d]\n", ret );
  NAGI_Printf( "パーティ=[%d]\n", PokeParty_GetPokeCount( p_party ) );
  
  { 
    int i;
    for( i = 0; i < NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN; i++ )
    { 
      OS_TPrintf( "%d ", p_data->sign[i] );
      if(i % 0x10 == 0xF )
      { 
        OS_TPrintf( "\n" );
      }
    }
  }


  return ret;

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
