//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_wifi.c
 *	@brief	WIFIのバトルマッチコア画面  WIFIシーケンス
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
#include "system/bmp_winframe.h"
#include "pokeicon/pokeicon.h"
#include "gamesystem/game_data.h"
#include "system/net_err.h"
#include "poke_tool/poke_regulation.h"

//	アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "wifimatch_gra.naix"
#include "msg/msg_wifi_match.h"

//	文字表示
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"

//  セーブデータ
#include "savedata/dreamworld_data.h"
#include "savedata/regulation.h"
#include "savedata/battle_box_save.h"
#include "savedata/my_pms_data.h"

//WIFIバトルマッチのモジュール
#include "wifibattlematch_graphic.h"
#include "wifibattlematch_view.h"
#include "wifibattlematch_net.h"
#include "wifibattlematch_util.h"

//外部公開
#include "wifibattlematch_core.h"

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(dpw_common);

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define GPF_FLAG_ON             //GPFフラグを強制ONにする
//#define DEBUG_REGULATION_DATA   //レギュレーションデータを作成する
//#define REGULATION_CHECK_ON     //パーティのレギュレーションチェックを強制ONにする
#endif //PM_DEBUG


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	サブシーケンスの戻り値
//=====================================
typedef enum
{
  //大会期間チェックの戻り値
  WBM_WIFI_SUBSEQ_UNREGISTER_RET_TRUE  = 0,//登録解除した
  WBM_WIFI_SUBSEQ_UNREGISTER_RET_FALSE,   //解除しなかった
  WBM_WIFI_SUBSEQ_UNREGISTER_RET_NONE,   //登録されていない

  //大会期間チェックの戻り値
  WBM_WIFI_SUBSEQ_CUPDATE_RET_TIMESAFE  = 0,//期間内
  WBM_WIFI_SUBSEQ_CUPDATE_RET_TIMEOVER,     //期間をすぎた


  WBM_WIFI_SUBSEQ_RET_NONE  = 0xFFFF,       //なし
} WBM_WIFI_SUBSEQ_RET;

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
///	メインワーク
//=====================================
typedef struct 
{
	//グラフィック設定
	WIFIBATTLEMATCH_GRAPHIC_WORK	*p_graphic;

  //リソース読み込み
  WIFIBATTLEMATCH_VIEW_RESOURCE *p_res;

	//共通で使うフォント
	GFL_FONT			            *p_font;

  //ポケモンのレベル・性別マーク表示のみで使用しているフォント
	GFL_FONT			            *p_small_font;

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

  //バトルボックス
  WBM_BTLBOX_WORK           *p_btlbox;

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

  //サブシーケンス
  WBM_SEQ_WORK              *p_subseq;
  WBM_WIFI_SUBSEQ_RET       subseq_ret;

  //GPFサーバーから落としてきた選手証データ
  DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA gpf_data;

  //SAKEから落としてきたレーティング＆ポケモンデータ
  WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA   sake_data;

  //引数
  WIFIBATTLEMATCH_CORE_PARAM  *p_param;

  //ウエイト
  u32 cnt;

} WIFIBATTLEMATCH_WIFI_WORK;

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_WIFI_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_WIFI_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_WIFI_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	WIFI大会シーケンス関数
//=====================================
static void WbmWifiSeq_Init( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_Join( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_RecvDigCard( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_CheckDigCard( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_Register( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_Matching( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_EndBattle( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_CupEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_DisConnextEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSeq_Err_ReturnLogin( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	サブシーケンス関数（シーケンス上で動くもの）
//=====================================
static void WbmWifiSubSeq_CheckDate( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void WbmWifiSubSeq_UnRegister( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	モジュールを使いやすくまとめたもの
//=====================================
//プレイヤー情報
static void Util_PlayerInfo_Create( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
static void Util_PlayerInfo_Delete( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
static BOOL Util_PlayerInfo_Move( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
static void Util_PlayerInfo_RenewalData( WIFIBATTLEMATCH_WIFI_WORK *p_wk, PLAYERINFO_WIFI_UPDATE_TYPE type );
//対戦相手情報
static void Util_MatchInfo_Create( WIFIBATTLEMATCH_WIFI_WORK *p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_enemy_data );
static void Util_MatchInfo_Delete( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
static BOOL Util_MatchInfo_Main( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
//バトルボックス
static void Util_BtlBox_Create( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
static void Util_BtlBox_Delete( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
static BOOL Util_BtlBox_MoveIn( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
static BOOL Util_BtlBox_MoveOut( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
//選択肢
typedef enum
{ 
  UTIL_LIST_TYPE_YESNO,
  UTIL_LIST_TYPE_JOIN,
  UTIL_LIST_TYPE_DECIDE,
  UTIL_LIST_TYPE_CUPMENU,
}UTIL_LIST_TYPE;
static void Util_List_Create( WIFIBATTLEMATCH_WIFI_WORK *p_wk, UTIL_LIST_TYPE type );
static void Util_List_Delete( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
static u32 Util_List_Main( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
//サブシーケンス
static void Util_SubSeq_Start( WIFIBATTLEMATCH_WIFI_WORK *p_wk, WBM_SEQ_FUNCTION seq_function );
static WBM_WIFI_SUBSEQ_RET Util_SubSeq_Main( WIFIBATTLEMATCH_WIFI_WORK *p_wk );
//自分のデータ作成
static void Util_SetupMyData( WIFIBATTLEMATCH_ENEMYDATA *p_my_data, const WIFIBATTLEMATCH_WIFI_WORK *cp_wk );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	WifiBattleMatchWifi_ProcData =
{	
	WIFIBATTLEMATCH_WIFI_PROC_Init,
	WIFIBATTLEMATCH_WIFI_PROC_Main,
	WIFIBATTLEMATCH_WIFI_PROC_Exit,
};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	コアプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_WIFI_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	WIFIBATTLEMATCH_WIFI_WORK	*p_wk;
  WIFIBATTLEMATCH_CORE_PARAM *p_param  = p_param_adrs;


	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_CORE, 0x30000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_WIFI_WORK), HEAPID_WIFIBATTLEMATCH_CORE );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_WIFI_WORK) );
  p_wk->p_param = p_param;

	//グラフィック設定
	p_wk->p_graphic	= WIFIBATTLEMATCH_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_WIFIBATTLEMATCH_CORE );

  //リソース読み込み
  p_wk->p_res     = WIFIBATTLEMATCH_VIEW_LoadResource( WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic ), WIFIBATTLEMATCH_MODE_WIFI, HEAPID_WIFIBATTLEMATCH_CORE );

	//共通モジュールの作成
	p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WIFIBATTLEMATCH_CORE );
	p_wk->p_small_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_small_gftr,
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
    p_wk->p_text  = WBM_TEXT_Init( BG_FRAME_M_TEXT, PLT_FONT_M, PLT_TEXT_M, CGR_OFS_M_TEXT, p_wk->p_que, p_wk->p_font, HEAPID_WIFIBATTLEMATCH_CORE );
    p_wk->p_seq   = WBM_SEQ_Init( p_wk, WbmWifiSeq_Init, HEAPID_WIFIBATTLEMATCH_CORE );
    p_wk->p_subseq   = WBM_SEQ_Init( p_wk, NULL, HEAPID_WIFIBATTLEMATCH_CORE );

    { 
      GFL_CLUNIT  *p_unit;
      p_unit  = WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
      p_wk->p_wait  = WBM_WAITICON_Init( p_unit, p_wk->p_res, HEAPID_WIFIBATTLEMATCH_CORE );
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
    }
	}

  PMSND_PlayBGM( SEQ_BGM_WCS );
	

#ifdef DEBUG_REGULATION_DATA
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
    Regulation_SetDebugData( SaveData_GetRegulationCardData(p_sv) );
  }
#endif //DEBUG_REGULATION_DATA

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	コアプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_WIFI_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{

	WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	= p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_param_adrs;

	//モジュールの破棄
  WBM_WAITICON_Exit( p_wk->p_wait );
  WBM_SEQ_Exit( p_wk->p_subseq );
  WBM_SEQ_Exit( p_wk->p_seq );
  WBM_TEXT_Exit( p_wk->p_text );
  if( p_wk->p_net )
  { 
    WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
  }
  Util_BtlBox_Delete( p_wk );
  Util_MatchInfo_Delete( p_wk );
  Util_PlayerInfo_Delete( p_wk );

	//共通モジュールの破棄
	WORDSET_Delete( p_wk->p_word );
	GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_small_font );
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
static GFL_PROC_RESULT WIFIBATTLEMATCH_WIFI_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_MAIN,
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
  };


	WIFIBATTLEMATCH_WIFI_WORK	*p_wk	    = p_wk_adrs;
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

  //NET
  WIFIBATTLEMATCH_NET_Main( p_wk->p_net );

  //文字表示
  if( p_wk->p_playerinfo )
  { 
    PLAYERINFO_PrintMain( p_wk->p_playerinfo, p_wk->p_que );
  }
  if( p_wk->p_matchinfo )
  { 
    MATCHINFO_PrintMain( p_wk->p_matchinfo, p_wk->p_que );
  }
  if( p_wk->p_btlbox )
  { 
    WBM_BTLBOX_PrintMain( p_wk->p_btlbox, p_wk->p_que );
  }

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *  WIFIシーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会  初期化処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_Init( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  NAGI_Printf( " mode%d ret%d\n", p_param->mode, p_param->retmode );
  switch( p_param->mode )
  { 
  case WIFIBATTLEMATCH_CORE_MODE_START:
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_Join );
    break;

  case WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE:
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_EndBattle );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会  参加選択処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_Join( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_MSG,    //WIFIバトル大会に参加しますか？
    SEQ_START_JOIN_LIST,  //参加しますか？
    SEQ_WAIT_JOIN_LIST,   //待ち
    SEQ_START_INFO_MSG,    //説明文
    SEQ_START_CANCEL_MSG,   //大会へ参加するのをやめる？
    SEQ_START_CANCEL_LIST,
    SEQ_WAIT_CANCEL_LIST,
    SEQ_JOIN,

    SEQ_WAIT_MSG, //メッセージ待ち
  };
  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START_MSG:    //WIFIバトル大会に参加しますか？
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_01, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_JOIN_LIST );
    break;
  case SEQ_START_JOIN_LIST:  //参加しますか？
    Util_List_Create( p_wk, UTIL_LIST_TYPE_JOIN );
    *p_seq     = SEQ_WAIT_JOIN_LIST;
    break;
  case SEQ_WAIT_JOIN_LIST:   //待ち
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //さんかする
          WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_RecvDigCard );
          break;
        case 1: //せつめいをきく
          *p_seq = SEQ_START_INFO_MSG;
          break;
        case 2: //やめる
          *p_seq = SEQ_START_CANCEL_MSG;
          break;
        }
      }
    }
    break;
  case SEQ_START_INFO_MSG:    //説明文
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_02, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_MSG );
    break;
  case SEQ_START_CANCEL_MSG:   //大会へ参加するのをやめる？
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_03, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_CANCEL_LIST );
    break;
  case SEQ_START_CANCEL_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_WAIT_CANCEL_LIST;
    break;
  case SEQ_WAIT_CANCEL_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
          WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_DisConnextEnd );
          break;
        case 1: //いいえ
          *p_seq = SEQ_START_MSG;
          break;
        }
      }
    }
    break;

  case SEQ_WAIT_MSG: //メッセージ待ち
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会  サーバ上のデジタル選手証データをダウンロード処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_RecvDigCard( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_SEARCH_MSG,
    SEQ_START_NET_INIT,
    SEQ_WAIT_NET_INIT,
    SEQ_CHECK_GPF,

    SEQ_START_NONE_MSG,
    SEQ_WAIT_NONE_MSG,

    SEQ_START_DOWNLOAD_GPF_DATA,
    SEQ_WAIT_DOWNLOAD_GPF_DATA,
    SEQ_START_DOWNLOAD_SAKE_DATA,
    SEQ_WAIT_DOWNLOAD_SAKE_DATA,

    SEQ_END
  };

  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START_SEARCH_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_04, WBM_TEXT_TYPE_STREAM );
    *p_seq  = SEQ_CHECK_GPF;
    break;
    
  case SEQ_CHECK_GPF:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      DREAMWORLD_SAVEDATA *p_dream = DREAMWORLD_SV_GetDreamWorldSaveData( p_sv );
#ifdef GPF_FLAG_ON
      *p_seq = SEQ_START_NET_INIT;
#else
      if( DREAMWORLD_SV_GetSignin( p_dream ) )
      { 
        *p_seq = SEQ_START_NET_INIT;
      }
      else
      { 
        *p_seq = SEQ_START_NONE_MSG;
      }
#endif
    }
    break;

  case SEQ_START_NONE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_05, WBM_TEXT_TYPE_STREAM );
    *p_seq  = SEQ_WAIT_NONE_MSG;
    break;

  case SEQ_WAIT_NONE_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    {
      WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_Join );
    }
    break;

  case SEQ_START_NET_INIT:
    WIFIBATTLEMATCH_NET_StartInitialize( p_wk->p_net );
    *p_seq  = SEQ_WAIT_NET_INIT;
    break;

  case SEQ_WAIT_NET_INIT:
    { 
      DWCGdbError error;
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      if( WIFIBATTLEMATCH_NET_WaitInitialize( p_wk->p_net, p_sv, &error ) )
      { 
        *p_seq = SEQ_START_DOWNLOAD_GPF_DATA;
      }
    }
    break;

  case SEQ_START_DOWNLOAD_GPF_DATA:
    { 
      MYSTATUS    *p_my;
      p_my  = GAMEDATA_GetMyStatus( p_wk->p_param->p_param->p_game_data); 
      WIFIBATTLEMATCH_NET_StartRecvGpfData( p_wk->p_net, p_my, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq = SEQ_WAIT_DOWNLOAD_GPF_DATA;
    }
    break;

  case SEQ_WAIT_DOWNLOAD_GPF_DATA:
    { 
      WIFIBATTLEMATCH_RECV_GPFDATA_RET  ret;
      NHTTPError  error;

      ret = WIFIBATTLEMATCH_NET_WaitRecvGpfData( p_wk->p_net, &error );
      if( ret == WIFIBATTLEMATCH_RECV_GPFDATA_RET_SUCCESS )
      { 
        WIFIBATTLEMATCH_NET_GetRecvGpfData( p_wk->p_net, &p_wk->gpf_data );
        *p_seq = SEQ_START_DOWNLOAD_SAKE_DATA;
      }
      else if( ret == WIFIBATTLEMATCH_RECV_GPFDATA_RET_ERROR )
      { 
        GF_ASSERT_MSG(0, "%d", error); //@todo
        *p_seq = SEQ_START_DOWNLOAD_SAKE_DATA;
      }
    }
    break;

  case SEQ_START_DOWNLOAD_SAKE_DATA:
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_WIFI_SCORE, &p_wk->sake_data );
    *p_seq = SEQ_WAIT_DOWNLOAD_SAKE_DATA;
    break;

  case SEQ_WAIT_DOWNLOAD_SAKE_DATA:
    { 
      DWCGdbError error;  //@todo　エラー処理
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &error ) )
      { 
        *p_seq = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_CheckDigCard );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会  デジタル選手証データチェック処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_CheckDigCard( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    //大会チェック
    SEQ_CHECK_DATA,
    SEQ_CHECK_STATUS,
    SEQ_START_END_MSG,
    SEQ_START_GIVEUP_MSG,
    SEQ_START_EXIT_MSG,
    SEQ_START_WRITE_GPF,
    SEQ_WAIT_WRITE_GPF,
    SEQ_START_WRITE_SAKE_DELETE_POKE,
    SEQ_WAIT_WRITE_SAKE_DELETE_POKE,

    //大会発見
    SEQ_START_DOWNLOAD_REG,
    SEQ_WAIT_DOWNLOAD_REG,
    SEQ_WAIT_DOWNLOAD_REG_SUCCESS,
    SEQ_WAIT_DOWNLOAD_REG_FAILURE,
    SEQ_START_SUBSEQ_CUPDATE,
    SEQ_WAIT_SUBSEQ_CUPDATE,
    SEQ_START_NOTCUP_MSG,

    //バトルボックスロック確認
    SEQ_CHECK_EXITS_REG,
    SEQ_CHECK_LOCK,

    SEQ_START_BOX_REWRITE_MSG,
    SEQ_START_BOX_REWRITE_YESNO,
    SEQ_SELECT_BOX_REWRITE_YESNO,      // 上書き確認
    SEQ_START_BOX_REWRITE_CANCEL_MSG,
    SEQ_START_BOX_REWRITE_CANCEL_YESNO,
    SEQ_SELECT_BOX_REWRITE_CANCEL_YESNO,

    SEQ_START_BOX_LOCK_MSG,           //  通常ロック確認
    SEQ_START_BOX_LOCK_YESNO,
    SEQ_SELECT_BOX_LOCK_YESNO,
    SEQ_START_BOX_LOCK_CANCEL_MSG,
    SEQ_START_BOX_LOCK_CANCEL_YESNO,
    SEQ_SELECT_BOX_LOCK_CANCEL_YESNO,

    SEQ_START_UPDATE_MSG,
    SEQ_START_SAVE_UPDATE,
    SEQ_WAIT_SAVE_UPDATE,
    SEQ_SEND_GPF_POKEMON,
    SEQ_WAIT_GPF_POKEMON,
    SEQ_SEND_GPF_CUPSTATUS,
    SEQ_WAIT_GPF_CUPSTATUS,
    SEQ_START_SAVE_NG_MSG,

    //大会未発見
    SEQ_CHECK_RATING,
    SEQ_START_RENEWAL_MSG,
    SEQ_START_SAVE_RENEWAL,
    SEQ_WAIT_SAVE_RENEWAL,
    
    SEQ_REGISTER_EXIT,  //選手証チェック＝１にして終了
    SEQ_ALREADY_EXIT,   //選手証チェック＝２にして終了
    SEQ_NG_EXIT,        //選手証チェック＝０にして終了

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  const REGULATION_CARDDATA *cp_reg_card  = SaveData_GetRegulationCardData(GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data ));

  switch( *p_seq )
  { 
    //-------------------------------------
    ///	大会と自分の状態をチェック
    //=====================================
  case SEQ_CHECK_DATA:
    //大会NOチェック
    if( p_wk->gpf_data.WifiMatchUpID == Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_CUPNO ) )
    { 
      //大会NOは一致

      //開催ワークチェック
      if( p_wk->gpf_data.WifiMatchUpState == Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_STATUS ) )
      { 
        //一致している
        NAGI_Printf( "大会ワーク一致\n" );
        *p_seq  = SEQ_CHECK_STATUS;
      }
      else
      { 
        //一致していない

        //４ならば、書き込みに行く
        //それ以外ならば上書き
        if( DREAM_WORLD_MATCHUP_RETIRE == Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_STATUS ) )
        { 
          NAGI_Printf( "大会ワーク一致してなく、リタイアしていた\n" );
          *p_seq  = SEQ_START_WRITE_GPF;
        }
        else
        { 
          NAGI_Printf( "大会ワーク一致してない\n" );
          *p_seq  = SEQ_CHECK_STATUS;
        }
      }
    }
    else
    { 
      //大会NO不一致

      //開催ワークチェック
      if( p_wk->gpf_data.WifiMatchUpState == DREAM_WORLD_MATCHUP_NONE )
      { 
        //未開催なので、取得へ
        NAGI_Printf( "大会NO不一致で、未開催\n" );
        *p_seq  = SEQ_START_DOWNLOAD_REG;
      }
      else
      { 
        //参加できる大会がないのでメッセージを出して終了
        NAGI_Printf( "参加できる大会がない\n" );
        *p_seq  = SEQ_START_EXIT_MSG;
      }
    }
    break;

  case SEQ_CHECK_STATUS:
    if( p_wk->gpf_data.WifiMatchUpState == DREAM_WORLD_MATCHUP_END )
    { 
      //大会が終了しているので
      //相応のメッセージをだして、大会見つからなかった、へ
      *p_seq  = SEQ_START_END_MSG;
    }
    else if( p_wk->gpf_data.WifiMatchUpState == DREAM_WORLD_MATCHUP_RETIRE )
    { 
      //この大会にギブアップしているので、
      //相応のメッセージをだして、大会見つからなかった、へ
      *p_seq  = SEQ_START_GIVEUP_MSG;
    }
    else
    { 
      //大会発見
      *p_seq  = SEQ_START_DOWNLOAD_REG;
    }
    break;

  case SEQ_START_END_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_07, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_CHECK_RATING );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_GIVEUP_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_07, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_CHECK_RATING );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_EXIT_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_06, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_NG_EXIT );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_WRITE_GPF:
    { 
      MYSTATUS    *p_my;
      DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA data;
      p_my  = GAMEDATA_GetMyStatus( p_wk->p_param->p_param->p_game_data); 
      GFL_STD_MemClear( &data, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA) );
      data.WifiMatchUpState = DREAM_WORLD_MATCHUP_RETIRE;
      WIFIBATTLEMATCH_NET_StartSendGpfData( p_wk->p_net, p_my, &data, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq  = SEQ_WAIT_WRITE_GPF;
    }
    break;

  case SEQ_WAIT_WRITE_GPF:
    { 
      WIFIBATTLEMATCH_SEND_GPFDATA_RET ret;
      NHTTPError  error;
      
      ret = WIFIBATTLEMATCH_NET_WaitSendGpfData( p_wk->p_net, &error );
      if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_SUCCESS )
      { 
        *p_seq  = SEQ_START_WRITE_SAKE_DELETE_POKE;
      }
      else if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR )
      { 
        //@todo
        GF_ASSERT_MSG( 0, "error%d", error );
        *p_seq  = SEQ_START_WRITE_SAKE_DELETE_POKE;
      }
    }
    break;
    
  case SEQ_START_WRITE_SAKE_DELETE_POKE:
    
    GFL_STD_MemClear( p_wk->sake_data.pokeparty, WIFIBATTLEMATCH_GDB_WIFI_POKEPARTY_SIZE );
    WIFIBATTLEMATCH_GDB_StartWrite( p_wk->p_net, WIFIBATTLEMATCH_GDB_WRITE_POKEPARTY, p_wk->sake_data.pokeparty );
    *p_seq  = SEQ_WAIT_WRITE_SAKE_DELETE_POKE;
    break;

  case SEQ_WAIT_WRITE_SAKE_DELETE_POKE:
    {
      DWCGdbError error;
      if( WIFIBATTLEMATCH_GDB_ProcessWrite( p_wk->p_net, &error ) )
      { 
        //@todo error

        *p_seq  = SEQ_START_GIVEUP_MSG;
      }
    }
    break;

    //-------------------------------------
    ///	大会が見つかったので、ダウンロード開始
    //=====================================
  case SEQ_START_DOWNLOAD_REG:
    WIFIBATTLEMATCH_NET_StartDownloadDigCard( p_wk->p_net, p_wk->gpf_data.WifiMatchUpID );
    *p_seq  = SEQ_WAIT_DOWNLOAD_REG;
    break;

  case SEQ_WAIT_DOWNLOAD_REG:
    { 
      WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET  ret;
      ret = WIFIBATTLEMATCH_NET_WaitDownloadDigCard( p_wk->p_net );
      if( ret == WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_SUCCESS )
      { 
        SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
        //取得成功
        WIFIBATTLEMATCH_NET_GetDownloadDigCard( p_wk->p_net, SaveData_GetRegulationCardData(p_sv) );
        *p_seq  =  SEQ_WAIT_DOWNLOAD_REG_SUCCESS;
      }
      else if( ret == WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_EMPTY )
      { 
        //取得失敗
        NAGI_Printf( "レギュレーションがなかったよ！\n" );
        *p_seq  =  SEQ_WAIT_DOWNLOAD_REG_FAILURE;
      }
      else if( ret == WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR )
      { 
        //@todo エラー処理
        GF_ASSERT( 0 );
        *p_seq  =  SEQ_WAIT_DOWNLOAD_REG_FAILURE;
      }
    }
    break;

  case SEQ_WAIT_DOWNLOAD_REG_FAILURE:
    if(DREAM_WORLD_MATCHUP_NONE == p_wk->gpf_data.WifiMatchUpState )
    { 
      *p_seq = SEQ_START_NOTCUP_MSG;
    }
    else
    { 
      *p_seq  = SEQ_START_SUBSEQ_CUPDATE;
    }
    break;

  case SEQ_START_SUBSEQ_CUPDATE:
    Util_SubSeq_Start( p_wk, WbmWifiSubSeq_CheckDate );
    *p_seq  = SEQ_WAIT_SUBSEQ_CUPDATE;
    break;

  case SEQ_WAIT_SUBSEQ_CUPDATE:
    { 
      WBM_WIFI_SUBSEQ_RET ret = Util_SubSeq_Main( p_wk );
      if( ret != WBM_WIFI_SUBSEQ_RET_NONE )
      { 
        *p_seq  = SEQ_CHECK_RATING;
      }
    }
    break;

  case SEQ_START_NOTCUP_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_09, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_NG_EXIT );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_WAIT_DOWNLOAD_REG_SUCCESS:
    if( DREAM_WORLD_MATCHUP_NONE == p_wk->gpf_data.WifiMatchUpState )
    { 
      *p_seq  = SEQ_CHECK_EXITS_REG;
    }
    else if( DREAM_WORLD_MATCHUP_SIGNUP == p_wk->gpf_data.WifiMatchUpState )
    { 
      *p_seq  = SEQ_REGISTER_EXIT;
    }
    else if( DREAM_WORLD_MATCHUP_ENTRY == p_wk->gpf_data.WifiMatchUpState )
    { 
      *p_seq  = SEQ_ALREADY_EXIT;
    }
    else
    { 
      *p_seq  = SEQ_NG_EXIT;
    }
    break;

    //-------------------------------------
    /// バトルボックスロック確認
    //=====================================
  case SEQ_CHECK_EXITS_REG:
    if( 0 == Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_CUPNO ) )
    { 
      //デジタル選手証のデータがない
      NAGI_Printf( "おとしてきたデジタル選手証のデータがないよ %d\n", Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_CUPNO ) );
      *p_seq  = SEQ_START_UPDATE_MSG;
    }
    else
    { 
      //デジタル選手証データがある
      *p_seq  = SEQ_CHECK_LOCK;
    }
    break;

  case SEQ_CHECK_LOCK:
    {
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
      if( BATTLE_BOX_SAVE_GetLockFlg( p_bbox_save ) )
      { 
        *p_seq  = SEQ_START_BOX_REWRITE_MSG;
      }
      else
      { 
        *p_seq  = SEQ_START_BOX_LOCK_MSG;
      }
    }
    break;

  //上書き確認
  case SEQ_START_BOX_REWRITE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_11, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_BOX_REWRITE_YESNO );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_BOX_REWRITE_YESNO:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq  = SEQ_SELECT_BOX_REWRITE_YESNO;
    break;

  case SEQ_SELECT_BOX_REWRITE_YESNO:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_START_UPDATE_MSG;
          break;
        case 1: //いいえ
          *p_seq  = SEQ_START_BOX_REWRITE_CANCEL_MSG;
          break;
        }
      }
    }
    break;

  case SEQ_START_BOX_REWRITE_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_12, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_BOX_REWRITE_CANCEL_YESNO );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_BOX_REWRITE_CANCEL_YESNO:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq  = SEQ_SELECT_BOX_REWRITE_CANCEL_YESNO;
    break;

  case SEQ_SELECT_BOX_REWRITE_CANCEL_YESNO:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_NG_EXIT;
          break;
        case 1: //いいえ
          *p_seq  = SEQ_START_BOX_REWRITE_MSG;
          break;
        }
      }
    }
    break;

  //ロック確認
  case SEQ_START_BOX_LOCK_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_10, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_BOX_LOCK_YESNO );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_BOX_LOCK_YESNO:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq  = SEQ_SELECT_BOX_LOCK_YESNO;
    break;

  case SEQ_SELECT_BOX_LOCK_YESNO:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_START_UPDATE_MSG;
          break;
        case 1: //いいえ
          *p_seq  = SEQ_START_BOX_LOCK_CANCEL_MSG;
          break;
        }
      }
    }
    break;

  case SEQ_START_BOX_LOCK_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_12, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_BOX_LOCK_CANCEL_YESNO );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_BOX_LOCK_CANCEL_YESNO:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq  = SEQ_SELECT_BOX_LOCK_CANCEL_YESNO;
    break;

  case SEQ_SELECT_BOX_LOCK_CANCEL_YESNO:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_NG_EXIT;
          break;
        case 1: //いいえ
          *p_seq  = SEQ_START_BOX_LOCK_MSG;
          break;
        }
      }
    }
    break;

  //更新処理
  case SEQ_START_UPDATE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_13, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SAVE_UPDATE );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_SAVE_UPDATE:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      SaveControl_SaveAsyncInit(p_sv);
      *p_seq  = SEQ_WAIT_SAVE_UPDATE;
    }
    break;

  case SEQ_WAIT_SAVE_UPDATE:
    {
      SAVE_RESULT ret;
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      ret = SaveControl_SaveAsyncMain(p_sv);
      if( ret == SAVE_RESULT_OK )
      { 
        *p_seq  = SEQ_SEND_GPF_POKEMON;
      }
      else if( ret == SAVE_RESULT_NG )
      { 
        *p_seq  = SEQ_START_SAVE_NG_MSG;
      }
    }
    break;

  case SEQ_SEND_GPF_POKEMON:
    //GPFサーバへポケモンを転送し、バトルポケモンを消す
    GFL_STD_MemClear( p_wk->sake_data.pokeparty, WIFIBATTLEMATCH_GDB_WIFI_POKEPARTY_SIZE );
    WIFIBATTLEMATCH_GDB_StartWrite( p_wk->p_net, WIFIBATTLEMATCH_GDB_WRITE_POKEPARTY, p_wk->sake_data.pokeparty );
    *p_seq  = SEQ_WAIT_GPF_POKEMON;
    break;
    
  case SEQ_WAIT_GPF_POKEMON:
    {
      DWCGdbError error;
      if( WIFIBATTLEMATCH_GDB_ProcessWrite( p_wk->p_net, &error ) )
      { 
        //@todo error
        *p_seq  = SEQ_SEND_GPF_CUPSTATUS;
      }
    }
    break;

  case SEQ_SEND_GPF_CUPSTATUS:
    //GPFサーバへ大会開催状況を転送する
    { 
      MYSTATUS    *p_my;
      DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA data;
      p_my  = GAMEDATA_GetMyStatus( p_wk->p_param->p_param->p_game_data); 
      GFL_STD_MemClear( &data, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA) );
      data.WifiMatchUpState = DREAM_WORLD_MATCHUP_SIGNUP;
      WIFIBATTLEMATCH_NET_StartSendGpfData( p_wk->p_net, p_my, &data, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq  = SEQ_WAIT_GPF_CUPSTATUS;
    }
    break;

  case SEQ_WAIT_GPF_CUPSTATUS:
    { 
      WIFIBATTLEMATCH_SEND_GPFDATA_RET ret;
      NHTTPError  error;
      
      ret = WIFIBATTLEMATCH_NET_WaitSendGpfData( p_wk->p_net, &error );
      if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_SUCCESS )
      { 
        *p_seq  = SEQ_REGISTER_EXIT;
      }
      else if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR )
      { 
        //@todo
        GF_ASSERT_MSG( 0, "error%d", error );
        *p_seq  = SEQ_REGISTER_EXIT;
      }
    }
    break;
    
  case SEQ_START_SAVE_NG_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_14, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_NG_EXIT );
    *p_seq  = SEQ_WAIT_MSG;
    break;

    //-------------------------------------
    /// 大会が見つからない
    //=====================================
  case SEQ_CHECK_RATING:
    if( 1 )  //レーティングチェック @todo
    { 
      //シャチとサーバーが一緒ならば
      *p_seq  = SEQ_START_RENEWAL_MSG;
    }
    else
    { 
      *p_seq  = SEQ_NG_EXIT;
    }
    break;

  case SEQ_START_RENEWAL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_08, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SAVE_RENEWAL );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_SAVE_RENEWAL:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      SaveControl_SaveAsyncInit(p_sv);
      *p_seq  = SEQ_WAIT_SAVE_RENEWAL;
    }
    break;

  case SEQ_WAIT_SAVE_RENEWAL:
    {
      SAVE_RESULT ret;
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      ret = SaveControl_SaveAsyncMain(p_sv);
      if( ret == SAVE_RESULT_OK )
      { 
        *p_seq  = SEQ_NG_EXIT;
      }
      else if( ret == SAVE_RESULT_NG )
      { 
        *p_seq  = SEQ_NG_EXIT;
      }
    }
    break;


  case SEQ_REGISTER_EXIT:  //選手証チェック＝１にして終了
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_Register );
    break;

  case SEQ_ALREADY_EXIT:   //選手証チェック＝２にして終了
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_Start );
    break;

  case SEQ_NG_EXIT:        //選手証チェック＝０にして終了
    p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_DisConnextEnd );
    break;


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
 *	@brief  WIFI大会  登録処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_Register( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_DRAW_PLAYERINFO,
    SEQ_WAIT_DRAW_PLAYERINFO,
    SEQ_START_REGISTER_POKE_MSG,
    SEQ_START_REGISTER_POKE_LIST,
    SEQ_SELECT_REGISTER_POKE_LIST,
    SEQ_START_REGISTER_POKE_CANCEL_MSG,
    SEQ_START_REGISTER_POKE_CANCEL_LIST,
    SEQ_SELECT_REGISTER_POKE_CANCEL_LIST,
    SEQ_START_REGISTER_CUP_CANCEL_MSG,
    SEQ_START_REGISTER_CUP_CANCEL_LIST,
    SEQ_SELECT_REGISTER_CUP_CANCEL_LIST,
    SEQ_WAIT_MOVEOUT_BTLBOX_END,
    
    SEQ_CHECK_REGULATION,
    SEQ_START_NG_REG_MSG,
    SEQ_WAIT_MOVEOUT_BTLBOX,
    SEQ_START_SEND_DATA_MSG,

    SEQ_START_SEND_CHECK_DIRTY_POKE,
    SEQ_START_DIRTY_POKE_MSG,
    SEQ_START_SEND_SAKE_POKE,
    SEQ_WAIT_SEND_SAKE_POKE,
    SEQ_START_INIT_SAKE_DATA,
    SEQ_WAIT_INIT_SAKE_DATA,
    SEQ_LOCK_POKE,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,
    SEQ_SEND_GPF_CUPSTATUS,
    SEQ_WAIT_GPF_CUPSTATUS,
    SEQ_START_OK_REGISTER_MSG,

    SEQ_NEXT,
    SEQ_DISCONNECT_END,
    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START_DRAW_PLAYERINFO:
    Util_PlayerInfo_Create( p_wk );
    Util_PlayerInfo_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_UNREGISTER );
    Util_BtlBox_Create( p_wk );
    *p_seq  = SEQ_WAIT_DRAW_PLAYERINFO;
    break;

  case SEQ_WAIT_DRAW_PLAYERINFO:
    {
      BOOL ret = TRUE;
      ret &= Util_PlayerInfo_Move( p_wk );
      ret &= Util_BtlBox_MoveIn( p_wk );
      if( ret )
      { 
        *p_seq  = SEQ_START_REGISTER_POKE_MSG;
      }
    }
    break;

  case SEQ_START_REGISTER_POKE_MSG: //バトルボックスのポケモンをとうろくしますか
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_15, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_REGISTER_POKE_LIST );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_REGISTER_POKE_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_DECIDE ); 
    *p_seq  = SEQ_SELECT_REGISTER_POKE_LIST;
    break;

  case SEQ_SELECT_REGISTER_POKE_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //けってい
          *p_seq  = SEQ_CHECK_REGULATION;
          break;

        case 1: //やめる
          *p_seq  = SEQ_START_REGISTER_POKE_CANCEL_MSG;
          break;
        }
      }
    }
    break;

    //-------------------------------------
    ///	キャンセル
    //=====================================
  case SEQ_START_REGISTER_POKE_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_16, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_REGISTER_POKE_CANCEL_LIST );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_REGISTER_POKE_CANCEL_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO ); 
    *p_seq  = SEQ_SELECT_REGISTER_POKE_CANCEL_LIST; 
    break;

  case SEQ_SELECT_REGISTER_POKE_CANCEL_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_START_REGISTER_CUP_CANCEL_MSG;
          break;

        case 1: //いいえ
          *p_seq  = SEQ_START_REGISTER_POKE_MSG;
          break;
        }
      }
    }
    break;

  case SEQ_START_REGISTER_CUP_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_17, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_REGISTER_CUP_CANCEL_LIST );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_REGISTER_CUP_CANCEL_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO ); 
    *p_seq  = SEQ_SELECT_REGISTER_CUP_CANCEL_LIST; 
    break;

  case SEQ_SELECT_REGISTER_CUP_CANCEL_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_WAIT_MOVEOUT_BTLBOX_END;
          break;

        case 1: //いいえ
          *p_seq  = SEQ_START_REGISTER_POKE_MSG;
          break;
        }
      }
    }
    break;

  case SEQ_WAIT_MOVEOUT_BTLBOX_END:
    if( Util_BtlBox_MoveOut( p_wk ) )
    { 
      Util_BtlBox_Delete( p_wk );
      *p_seq  = SEQ_DISCONNECT_END;
    }
    break;
    
    //-------------------------------------
    ///	登録
    //=====================================
  case SEQ_CHECK_REGULATION:
#ifdef REGULATION_CHECK_ON
    *p_seq  = SEQ_WAIT_MOVEOUT_BTLBOX;
#else
    {
      u32 failed_bit;
      SAVE_CONTROL_WORK *p_sv   = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      REGULATION        *p_reg  = SaveData_GetRegulation( p_sv,0 );
      BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
      POKEPARTY         *p_party=  BATTLE_BOX_SAVE_MakePokeParty( p_bbox_save, GFL_HEAP_LOWID(HEAPID_WIFIBATTLEMATCH_CORE) );

      if( POKE_REG_OK == PokeRegulationMatchLookAtPokeParty(p_reg, p_party, &failed_bit) )
      { 
        *p_seq  = SEQ_WAIT_MOVEOUT_BTLBOX;
        GFL_HEAP_FreeMemory( p_party );
      }
      else
      { 
        *p_seq  = SEQ_START_NG_REG_MSG;
        GFL_HEAP_FreeMemory( p_party );
      }
    }
#endif
    break;
  case SEQ_START_NG_REG_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_21, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_REGISTER_POKE_MSG );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_WAIT_MOVEOUT_BTLBOX:
    if( Util_BtlBox_MoveOut( p_wk ) )
    { 
      Util_BtlBox_Delete( p_wk );
      *p_seq  = SEQ_START_SEND_DATA_MSG;
    }
    break;

  case SEQ_START_SEND_DATA_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_18, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SEND_CHECK_DIRTY_POKE );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_SEND_CHECK_DIRTY_POKE:
    if( 1 )  //@todo  不正チェック
    { 
      *p_seq = SEQ_START_SEND_SAKE_POKE;
    }
    else
    { 
      *p_seq = SEQ_START_DIRTY_POKE_MSG;
    }
    break;

  case SEQ_START_DIRTY_POKE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_19, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_DISCONNECT_END );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_START_SEND_SAKE_POKE:
    {
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
      POKEPARTY         *p_party =  BATTLE_BOX_SAVE_MakePokeParty( p_bbox_save, HEAPID_WIFIBATTLEMATCH_CORE );

      GFL_STD_MemCopy( p_party, p_wk->sake_data.pokeparty, PokeParty_GetWorkSize() );
      GFL_HEAP_FreeMemory( p_party );
      WIFIBATTLEMATCH_GDB_StartWrite( p_wk->p_net, WIFIBATTLEMATCH_GDB_WRITE_POKEPARTY, p_wk->sake_data.pokeparty );
      *p_seq  = SEQ_WAIT_SEND_SAKE_POKE;
    }
    break;
    
  case SEQ_WAIT_SEND_SAKE_POKE:
    {
      DWCGdbError error;
      if( WIFIBATTLEMATCH_GDB_ProcessWrite( p_wk->p_net, &error ) )
      { 
        //@todo error
        *p_seq  = SEQ_LOCK_POKE;
      }
    }
    break;

  case SEQ_START_INIT_SAKE_DATA:
    {
      WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA data;
      GFL_STD_MemClear( &data, sizeof(WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA) );
      data.rate = 1500;
      WIFIBATTLEMATCH_GDB_StartWrite( p_wk->p_net, WIFIBATTLEMATCH_GDB_WRITE_WIFI_SCORE, &data );
      *p_seq  = SEQ_WAIT_INIT_SAKE_DATA;
    }
    break;

  case SEQ_WAIT_INIT_SAKE_DATA:
    {
      DWCGdbError error;
      if( WIFIBATTLEMATCH_GDB_ProcessWrite( p_wk->p_net, &error ) )
      { 
        //@todo error
        *p_seq  = SEQ_LOCK_POKE;
      }
    }
    break;

  case SEQ_LOCK_POKE:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
      BATTLE_BOX_SAVE_SetLockFlg( p_bbox_save, TRUE );
      *p_seq  = SEQ_START_SAVE;
    }
    break;

  case SEQ_START_SAVE:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      SaveControl_SaveAsyncInit(p_sv);
      *p_seq  = SEQ_WAIT_SAVE;
    }
    break;

  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT ret;
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      ret = SaveControl_SaveAsyncMain(p_sv);
      if( ret == SAVE_RESULT_OK )
      { 
        *p_seq  = SEQ_SEND_GPF_CUPSTATUS;
      }
      else if( ret == SAVE_RESULT_NG )
      { 
        *p_seq  = SEQ_DISCONNECT_END;
      }
    }
    break;

  case SEQ_SEND_GPF_CUPSTATUS:
    { 
      MYSTATUS    *p_my;
      DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA data;
      p_my  = GAMEDATA_GetMyStatus( p_wk->p_param->p_param->p_game_data); 
      GFL_STD_MemClear( &data, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA) );
      data.WifiMatchUpState = DREAM_WORLD_MATCHUP_ENTRY;
      WIFIBATTLEMATCH_NET_StartSendGpfData( p_wk->p_net, p_my, &data, HEAPID_WIFIBATTLEMATCH_CORE );
    }
    *p_seq  = SEQ_WAIT_GPF_CUPSTATUS;
    break;

  case SEQ_WAIT_GPF_CUPSTATUS:
    { 
      WIFIBATTLEMATCH_SEND_GPFDATA_RET ret;
      NHTTPError  error;
      
      ret = WIFIBATTLEMATCH_NET_WaitSendGpfData( p_wk->p_net, &error );
      if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_SUCCESS )
      { 
        *p_seq  = SEQ_START_OK_REGISTER_MSG;
      }
      else if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR )
      { 
        //@todo
        GF_ASSERT_MSG( 0, "error%d", error );
        *p_seq  = SEQ_START_OK_REGISTER_MSG;
      }
    }
    break;

  case SEQ_START_OK_REGISTER_MSG:
    Util_PlayerInfo_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_LOCK );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_20, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_NEXT );
    *p_seq  = SEQ_WAIT_MSG; 
    break;

  case SEQ_NEXT:
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_Start );
    break;

  case SEQ_DISCONNECT_END:
    p_param->result = WIFIBATTLEMATCH_CORE_RESULT_FINISH;
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_DisConnextEnd );
    break;

  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk ); //予約したシーケンス変数に飛ぶ
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会  大会開始処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_DRAW_PLAYERINFO,
    SEQ_WAIT_DRAW_PLAYERINFO,

    SEQ_START_CUP_MSG,
    SEQ_START_CUP_LIST,
    SEQ_SELECT_CUP_LIST,

    SEQ_NEXT_MATCHING,
    SEQ_NEXT_CUP_END,

    SEQ_START_SUBSEQ_UNREGISTER,
    SEQ_WAIT_SUBSEQ_UNREGISTER,
    SEQ_NEXT_DISCONNECT,

    SEQ_WAIT_MSG,
  };
  
  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START_DRAW_PLAYERINFO:
    if( p_wk->p_playerinfo == NULL )
    { 
      Util_PlayerInfo_Create( p_wk );
      Util_PlayerInfo_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_LOCK );
    }
    else
    { 
      *p_seq  = SEQ_START_CUP_MSG;
    }
    break;

  case SEQ_WAIT_DRAW_PLAYERINFO:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq  = SEQ_START_CUP_MSG;
    }
    break;

  case SEQ_START_CUP_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_22, WBM_TEXT_TYPE_STREAM );
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_CUP_LIST );
    *p_seq  = SEQ_WAIT_MSG; 
    break;

  case SEQ_START_CUP_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_CUPMENU );
    *p_seq  = SEQ_SELECT_CUP_LIST;
    break;

  case SEQ_SELECT_CUP_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //たいせんする
          *p_seq = SEQ_NEXT_MATCHING;
          break;
        case 1: //さんかかいじょ
          *p_seq = SEQ_START_SUBSEQ_UNREGISTER;
          break;
        case 2: //やめる
          *p_seq = SEQ_NEXT_CUP_END;
          break;
        }
      }
    }
    break;

  case SEQ_NEXT_MATCHING:
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_Matching );
    break;

  case SEQ_NEXT_CUP_END:
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_CupEnd );
    break;
    
  case SEQ_START_SUBSEQ_UNREGISTER:
    Util_SubSeq_Start( p_wk, WbmWifiSubSeq_UnRegister );
    *p_seq  = SEQ_WAIT_SUBSEQ_UNREGISTER;
    break;
  case SEQ_WAIT_SUBSEQ_UNREGISTER:
    { 
      WBM_WIFI_SUBSEQ_RET ret = Util_SubSeq_Main( p_wk );
      if( ret != WBM_WIFI_SUBSEQ_RET_NONE )
      { 
        *p_seq  = SEQ_NEXT_DISCONNECT;
      }
    }
    break;
  case SEQ_NEXT_DISCONNECT:
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_DisConnextEnd );
    break;

  case SEQ_WAIT_MSG:
    if( WBM_TEXT_IsEnd( p_wk->p_text ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk ); //予約したシーケンス変数に飛ぶ
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会  マッチング処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_Matching( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  { 
    //大会期間チェック
    SEQ_START_SUBSEQ_CUPDATE,
    SEQ_WAIT_SUBSEQ_CUPDATE,
    SEQ_NEXT_START,
    SEQ_RECV_SAKE_DATA,
    SEQ_WAIT_SAKE_DATA,

    //マッチング開始
    SEQ_START_MATCH_MSG,
    SEQ_START_MATCH,
    SEQ_WAIT_MATCHING,
    SEQ_START_SENDDATA,
    SEQ_WAIT_SENDDATA,
    SEQ_CHECK_YOU_CUPNO,
    SEQ_CHECK_YOU_REGULATION,
    SEQ_START_SEND_CHECK_DIRTY_POKE,
    SEQ_SEND_DIRTY_CHECK_DATA,
    SEQ_RECV_DIRTY_CHECK_DATA,
    SEQ_CHECK_DIRTY,
    SEQ_START_BADWORD,
    SEQ_WAIT_BADWORD,
    SEQ_START_OK_MATCHING_MSG,
    SEQ_START_DRAW_MATCHINFO,
    SEQ_WAIT_MOVEIN_MATCHINFO,
    SEQ_WAIT_CNT,
    SEQ_END_MATCHING,

    //キャンセル処理
    SEQ_START_SELECT_CANCEL_MSG,
    SEQ_START_CANCEL_LIST,
    SEQ_SELECT_CANCEL_LIST,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;


  switch( *p_seq )
  { 
    //-------------------------------------
    ///	大会期間チェック
    //=====================================
  case SEQ_START_SUBSEQ_CUPDATE:
    Util_SubSeq_Start( p_wk, WbmWifiSubSeq_CheckDate );
    *p_seq  = SEQ_WAIT_SUBSEQ_CUPDATE;
    break;
  case SEQ_WAIT_SUBSEQ_CUPDATE:
    { 
      WBM_WIFI_SUBSEQ_RET ret = Util_SubSeq_Main( p_wk );
      if( ret == WBM_WIFI_SUBSEQ_CUPDATE_RET_TIMESAFE )
      { 
        *p_seq  = SEQ_RECV_SAKE_DATA;
      }
      else if( ret == WBM_WIFI_SUBSEQ_CUPDATE_RET_TIMESAFE )
      { 
        *p_seq  = SEQ_NEXT_START;
      }
    }
    break;
  case SEQ_NEXT_START:
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_Start );
    break;
  case SEQ_RECV_SAKE_DATA:
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_WIFI_SCORE, &p_wk->sake_data );
    *p_seq  = SEQ_WAIT_SAKE_DATA;
    break;
    
  case SEQ_WAIT_SAKE_DATA:
    { 
      DWCGdbError error;  //@todo　エラー処理
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &error ) )
      { 
        //自分のデータ作成
        WIFIBATTLEMATCH_ENEMYDATA *p_my_data = p_param->p_player_data;
        Util_SetupMyData( p_my_data, p_wk );
        *p_seq  = SEQ_START_MATCH_MSG;
      }
    }
    break;

    //-------------------------------------
    ///	マッチング開始
    //=====================================
  case SEQ_START_MATCH_MSG:
    WBM_WAITICON_SetDrawEnable( p_wk->p_wait, TRUE );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_30, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_MATCH );
    break;
  case SEQ_START_MATCH:
    {
      const REGULATION_CARDDATA *cp_reg_card  = SaveData_GetRegulationCardData(GAMEDATA_GetSaveControlWork( p_wk->p_param->p_param->p_game_data ));
      WIFIBATTLEMATCH_MATCH_KEY_DATA  data;
      GFL_STD_MemClear( &data, sizeof(WIFIBATTLEMATCH_MATCH_KEY_DATA) );
      data.rate = p_wk->sake_data.rate;
      data.disconnect = p_wk->sake_data.disconnect;
      data.cup_no = Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_CUPNO );
      WIFIBATTLEMATCH_NET_StartMatchMake( p_wk->p_net, p_param->p_param->mode, FALSE, p_param->p_param->btl_rule, &data ); 
      *p_seq  = SEQ_WAIT_MATCHING;
    }
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
  case SEQ_START_SENDDATA:
    { 
      if( WIFIBATTLEMATCH_NET_StartEnemyData( p_wk->p_net, p_param->p_player_data ) )
      {  
        *p_seq  = SEQ_WAIT_SENDDATA;
      }
    }
    break;
  case SEQ_WAIT_SENDDATA:
    {
      WIFIBATTLEMATCH_ENEMYDATA *p_recv;
      if( WIFIBATTLEMATCH_NET_WaitEnemyData( p_wk->p_net, &p_recv ) )
      { 
        GFL_STD_MemCopy( p_recv, p_param->p_enemy_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
        WIFIBATTLEMATCH_DATA_ModifiEnemyData( p_param->p_enemy_data, HEAPID_WIFIBATTLEMATCH_CORE );  //不正データ対策
        *p_seq  = SEQ_CHECK_YOU_CUPNO;
      }
    }
    break;
  case SEQ_CHECK_YOU_CUPNO:
    WIFIBATTLEMATCH_DATA_DebugPrint( p_param->p_player_data );
    WIFIBATTLEMATCH_DATA_DebugPrint( p_param->p_enemy_data );
    if( p_param->p_player_data->wificup_no == p_param->p_enemy_data->wificup_no )
    { 
      *p_seq  = SEQ_CHECK_YOU_REGULATION;
    }
    else
    {
      //大会番号がことなっていたらマッチングに戻る
      if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE ) )
      { 
        *p_seq  = SEQ_START_MATCH;
      }
    }
    break;
  case SEQ_CHECK_YOU_REGULATION:
    {
      u32 failed_bit;
      SAVE_CONTROL_WORK *p_sv   = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      REGULATION        *p_reg  = SaveData_GetRegulation( p_sv,0 );
      POKEPARTY         *p_party=  (POKEPARTY*)p_param->p_enemy_data->pokeparty;
      //相手のポケモンをレギュレーションチェックにかける
      if( POKE_REG_OK == PokeRegulationMatchLookAtPokeParty(p_reg, p_party, &failed_bit)  )
      { 
        NAGI_Printf( "相手のポケモンはレギュレーションOK\n" );
        *p_seq = SEQ_START_SEND_CHECK_DIRTY_POKE;
      }
      else
      { 
        //不正カウンタアップ //@todo
        NAGI_Printf( "!!相手のポケモンはレギュレーションNG!! 0x%x\n", failed_bit );
        *p_seq = SEQ_SEND_DIRTY_CHECK_DATA;
      }
    }
    break;
  case SEQ_START_SEND_CHECK_DIRTY_POKE:
    //不正判定サーバへポケモンを送る ふせいならば不正カウンタアップ
    if( 1 ) //@todo
    { 
      *p_seq  = SEQ_SEND_DIRTY_CHECK_DATA;
    }
    break;
  case SEQ_SEND_DIRTY_CHECK_DATA:
    if( 1 ) //@todo 相手へ不正カウンタ送信
    { 
      *p_seq  = SEQ_RECV_DIRTY_CHECK_DATA;
    }
    break;
  case SEQ_RECV_DIRTY_CHECK_DATA:
    if( 1 ) //@todo　 不正カウンタ受信まち
    { 
      *p_seq  = SEQ_CHECK_DIRTY;
    }
    break;
  case SEQ_CHECK_DIRTY:
    if( 1 )  //@todo 相手の不正カウンタチェック
    { 
      *p_seq  = SEQ_START_BADWORD;
    }
    else
    { 
      //不正だったらマッチングに戻る
      if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE ) )
      { 
        *p_seq  = SEQ_START_MATCH;
      }
    }
    break;

  case SEQ_START_BADWORD:
    WIFIBATTLEMATCH_NET_StartBadWord( p_wk->p_net, p_param->p_enemy_data->name, PERSON_NAME_SIZE+EOM_SIZE );
    *p_seq  = SEQ_WAIT_BADWORD;
    break;
  case SEQ_WAIT_BADWORD:
    { 
      BOOL ret;
      BOOL is_badword;
      
      ret = WIFIBATTLEMATCH_NET_WaitBadWord( p_wk->p_net, &is_badword );
      if( ret )
      { 
        if( is_badword )
        { 
          WIFIBATTLEMATCH_DATA_ModifiName( p_param->p_enemy_data, HEAPID_WIFIBATTLEMATCH_CORE );
          NAGI_Printf( "わるもしでした\n" );
        }

        *p_seq  = SEQ_START_OK_MATCHING_MSG;
      }
    }
    break;
  case SEQ_START_OK_MATCHING_MSG:
    WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_32, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_DRAW_MATCHINFO ); 
    break;
  case SEQ_START_DRAW_MATCHINFO:
    //対戦者情報表示
    Util_MatchInfo_Create( p_wk, p_param->p_enemy_data );
    *p_seq  = SEQ_WAIT_MOVEIN_MATCHINFO;
    break;
  case SEQ_WAIT_MOVEIN_MATCHINFO:
    if( Util_MatchInfo_Main( p_wk ) )
    {
      *p_seq  = SEQ_WAIT_CNT;
    }
    break;
  case SEQ_WAIT_CNT:
    if( p_wk->cnt++ > ENEMYDATA_WAIT_SYNC )
    { 
      p_wk->cnt = 0;
      *p_seq  = SEQ_END_MATCHING;
    }
    break;
  case SEQ_END_MATCHING:
    p_param->result = WIFIBATTLEMATCH_CORE_RESULT_NEXT_BATTLE;
    WBM_SEQ_End( p_seqwk );
    break;

    //-------------------------------------
    ///	キャンセル処理
    //=====================================
  case SEQ_START_SELECT_CANCEL_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_31, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_CANCEL_LIST ); 
    break;
  case SEQ_START_CANCEL_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq  = SEQ_SELECT_CANCEL_LIST;
    break;
  case SEQ_SELECT_CANCEL_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_CupEnd );
          break;
        case 1: //いいえ
          *p_seq  = SEQ_START_MATCH_MSG;
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
 *	@brief  WIFI大会  戦闘後処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_EndBattle( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_NET_MSG,
    SEQ_START_REPORT_ATLAS,
    SEQ_WAIT_REPORT_ATLAS,
    SEQ_WAIT_DISCONNECT,

    SEQ_START_REPORT_MSG,
    SEQ_START_RECVDATA_SAKE,
    SEQ_WAIT_RECVDATA_SAKE,
    SEQ_START_RECVDATA_GPF,
    SEQ_WAIT_RECVDATA_GPF,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,
    SEQ_START_CARDIN,
    SEQ_WAIT_CARDIN,

    //録画確認
    SEQ_START_SELECT_BTLREC_MSG,
    SEQ_START_SELECTBTLREC,
    SEQ_WAIT_SELECTBTLREC,

    //大会期間チェック
    SEQ_START_SUBSEQ_CUPDATE,
    SEQ_WAIT_SUBSEQ_CUPDATE,

    //続行確認
    SEQ_START_SELECT_CONTINUE_MSG,
    SEQ_START_SELECT_CONTINUE,
    SEQ_WAIT_SELECT_CONTINUE,

    //共通
    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START_NET_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_34, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_REPORT_ATLAS );
    break;
  case SEQ_START_REPORT_ATLAS:
    WIFIBATTLEMATCH_SC_Start( p_wk->p_net, p_param->p_param->mode, p_param->p_param->btl_rule, p_param->btl_result );
    *p_seq = SEQ_WAIT_REPORT_ATLAS;
    break;
  case SEQ_WAIT_REPORT_ATLAS:
    { 
      DWCScResult result;
      if( WIFIBATTLEMATCH_SC_Process( p_wk->p_net, &result ) )
      { 
        NAGI_Printf( "送ったよ！%d\n", result );
        *p_seq = SEQ_WAIT_DISCONNECT;
      }
    }
    break;

  case SEQ_WAIT_DISCONNECT:
    if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk->p_net, TRUE ) )
    { 
      *p_seq = SEQ_START_REPORT_MSG;
    }
    break;

  case SEQ_START_REPORT_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_33, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_RECVDATA_SAKE );
    break;

  case SEQ_START_RECVDATA_SAKE:
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_GET_WIFI_SCORE, &p_wk->sake_data ); 
    *p_seq       = SEQ_WAIT_RECVDATA_SAKE;
    break;

  case SEQ_WAIT_RECVDATA_SAKE:
    { 
      DWCGdbError error;  //@todo　エラー処理
      if( WIFIBATTLEMATCH_GDB_Process( p_wk->p_net, &error ) )
      { 
        *p_seq        = SEQ_START_RECVDATA_GPF;
      }
    }
    break;

  case SEQ_START_RECVDATA_GPF:
    { 
      MYSTATUS    *p_my;
      p_my  = GAMEDATA_GetMyStatus( p_wk->p_param->p_param->p_game_data); 
      WIFIBATTLEMATCH_NET_StartRecvGpfData( p_wk->p_net, p_my, HEAPID_WIFIBATTLEMATCH_CORE );
      *p_seq        = SEQ_WAIT_RECVDATA_GPF;
    }
    break;
  case SEQ_WAIT_RECVDATA_GPF:
    { 
      WIFIBATTLEMATCH_RECV_GPFDATA_RET  ret;
      NHTTPError  error;

      ret = WIFIBATTLEMATCH_NET_WaitRecvGpfData( p_wk->p_net, &error );
      if( ret == WIFIBATTLEMATCH_RECV_GPFDATA_RET_SUCCESS )
      { 
        WIFIBATTLEMATCH_NET_GetRecvGpfData( p_wk->p_net, &p_wk->gpf_data );
        *p_seq        = SEQ_START_SAVE;
      }
      else if( ret == WIFIBATTLEMATCH_RECV_GPFDATA_RET_ERROR )
      { 
        GF_ASSERT_MSG(0, "%d", error); //@todo
        *p_seq        = SEQ_START_SAVE;
      }
    }
    break;
  case SEQ_START_SAVE:
    GAMEDATA_SaveAsyncStart(p_param->p_param->p_game_data);
    *p_seq  = SEQ_WAIT_SAVE;
    break;

  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_param->p_param->p_game_data);
      if( result == SAVE_RESULT_OK )
      { 
        *p_seq  = SEQ_START_CARDIN;
      }
      else if(result == SAVE_RESULT_NG)
      {
        WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_DisConnextEnd );
      }
    }
    break;
  case SEQ_START_CARDIN:
    Util_PlayerInfo_Create( p_wk );
    *p_seq  = SEQ_WAIT_CARDIN;
    break;
  case SEQ_WAIT_CARDIN:
    {
      BOOL ret;
      ret = Util_PlayerInfo_Move( p_wk );
      if( ret )
      { 
        *p_seq  = SEQ_START_SELECT_BTLREC_MSG;
      }
    }
    break;
    //-------------------------------------
    ///	録画確認
    //=====================================
  case SEQ_START_SELECT_BTLREC_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_TEXT_014, WBM_TEXT_TYPE_STREAM );
    *p_seq = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SELECTBTLREC ); 
    break;
  case SEQ_START_SELECTBTLREC:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_WAIT_SELECTBTLREC;   
    break;
  case SEQ_WAIT_SELECTBTLREC:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0:
          //@todo
          *p_seq = SEQ_START_SUBSEQ_CUPDATE;
          break;
        case 1:
          *p_seq = SEQ_START_SUBSEQ_CUPDATE;
          break;
        }
      }
    }
    break;

  case SEQ_START_SUBSEQ_CUPDATE:
    Util_SubSeq_Start( p_wk, WbmWifiSubSeq_CheckDate );
    *p_seq  = SEQ_WAIT_SUBSEQ_CUPDATE;
    break;
  case SEQ_WAIT_SUBSEQ_CUPDATE:
    { 
      WBM_WIFI_SUBSEQ_RET ret = Util_SubSeq_Main( p_wk );
      if( ret == WBM_WIFI_SUBSEQ_CUPDATE_RET_TIMESAFE )
      { 
        *p_seq = SEQ_START_SELECT_CONTINUE_MSG;
      }
      else if( ret == WBM_WIFI_SUBSEQ_CUPDATE_RET_TIMEOVER )
      { 
        WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_DisConnextEnd );
      }
    }
    break;
    //-------------------------------------
    ///	続行確認
    //=====================================
  case SEQ_START_SELECT_CONTINUE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_24, WBM_TEXT_TYPE_STREAM );
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
          WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_Matching );
          break;
        case 1:
          WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_CupEnd );
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
 *	@brief  WIFI大会  大会終了処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_CupEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_END_MSG,
    SEQ_START_END_LIST,
    SEQ_SELECT_END_LIST,

    SEQ_START_CONFIRM_MSG,
    SEQ_START_CONFIRM_LIST,
    SEQ_SELECT_CONFIRM_LIST,

    SEQ_START_SUBSEQ_CUPDATE,
    SEQ_WAIT_SUBSEQ_CUPDATE,

    SEQ_SEND_SAKE_LOGINTIME,
    SEQ_END,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START_END_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_23, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_END_LIST );
    break;
    
  case SEQ_START_END_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq  = SEQ_SELECT_END_LIST;
    break;

  case SEQ_SELECT_END_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_START_SUBSEQ_CUPDATE;
          break;
        case 1: //いいえ
          *p_seq  = SEQ_START_CONFIRM_MSG;
          break;
        }
      }
    }
    break;

  case SEQ_START_CONFIRM_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_24, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_CONFIRM_LIST );
    break;

  case SEQ_START_CONFIRM_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq  = SEQ_SELECT_CONFIRM_LIST;
    break;

  case SEQ_SELECT_CONFIRM_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_Matching );
          break;
        case 1: //いいえ
          *p_seq  = SEQ_START_END_MSG;
          break;
        }
      }
    }
    break;

  case SEQ_START_SUBSEQ_CUPDATE:
    Util_SubSeq_Start( p_wk, WbmWifiSubSeq_CheckDate );
    *p_seq  = SEQ_WAIT_SUBSEQ_CUPDATE;
    break;

  case SEQ_WAIT_SUBSEQ_CUPDATE:
    { 
      WBM_WIFI_SUBSEQ_RET ret = Util_SubSeq_Main( p_wk );
      if( ret != WBM_WIFI_SUBSEQ_RET_NONE )
      { 
        *p_seq  = SEQ_SEND_SAKE_LOGINTIME;
      }
    }
    break;

  case SEQ_SEND_SAKE_LOGINTIME:
    //@todo ログイン時間書き込み
    *p_seq  = SEQ_END;
    break;

  case SEQ_END:
    WBM_SEQ_SetNext( p_seqwk, WbmWifiSeq_DisConnextEnd );
    break;

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
 *	@brief  WIFI大会  切断処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_DisConnextEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  { 
    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,
    SEQ_END,
  };

  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_START_DISCONNECT:
    if( GFL_NET_IsInit())
    { 
      WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
      p_wk->p_net = NULL;
      GFL_NET_Exit( NULL );
      *p_seq  = SEQ_WAIT_DISCONNECT;
    }
    else
    { 
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_WAIT_DISCONNECT:
    if( !GFL_NET_IsInit() )
    { 
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    WBM_SEQ_End( p_seqwk );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会  エラー処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSeq_Err_ReturnLogin( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  p_param->result = WIFIBATTLEMATCH_CORE_RESULT_ERR_NEXT_LOGIN;
  WBM_SEQ_End( p_seqwk );
}
//=============================================================================
/**
///	サブシーケンス関数（メインシーケンス上で動くシーケンス処理）
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  WIFI大会  大会期間をチェック
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSubSeq_CheckDate( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_CHECK_CUP_STATUS,

    SEQ_CHECK_DATE,
    SEQ_START_DATE_MSG,
    SEQ_CHECK_LOCK,
    SEQ_UNLOCK,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,
    SEQ_START_UNLOCK_MSG,
    SEQ_SEND_GPF_CUPSTATUS,
    SEQ_WAIT_GPF_CUPSTATUS,
    SEQ_END,

    SEQ_WAIT_MSG,
  };

  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_CHECK_CUP_STATUS:
    if( p_wk->gpf_data.WifiMatchUpState == DREAM_WORLD_MATCHUP_NONE )
    { 
      *p_seq  = SEQ_END;
    }
    else
    { 
      *p_seq  = SEQ_CHECK_DATE;
    }
    break;

  case SEQ_CHECK_DATE:
    { 
      u8 start_year;
      u8 start_month;
      u8 start_day;
      u8 end_year;
      u8 end_month;
      u8 end_day;
      RTCDate now_date;
      RTCTime time;
      BOOL    ret;
      SAVE_CONTROL_WORK *p_sv   = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      REGULATION_CARDDATA*p_reg = SaveData_GetRegulationCardData( p_sv );
      ret = DWC_GetDateTime( &now_date, &time );

      start_year  = Regulation_GetCardParam( p_reg, REGULATION_CARD_START_YEAR );
      start_month = Regulation_GetCardParam( p_reg, REGULATION_CARD_START_MONTH );
      start_day   = Regulation_GetCardParam( p_reg, REGULATION_CARD_START_DAY );

      end_year    = Regulation_GetCardParam( p_reg, REGULATION_CARD_END_YEAR );
      end_month   = Regulation_GetCardParam( p_reg, REGULATION_CARD_END_MONTH );
      end_day     = Regulation_GetCardParam( p_reg, REGULATION_CARD_END_DAY );

      if( ret &&
          ( start_year <= now_date.year && now_date.year <= end_year &&
            start_month <= now_date.month && now_date.month <= end_month &&
            start_day <= now_date.day && now_date.day <= end_day ) )
      { 
        p_wk->subseq_ret  = WBM_WIFI_SUBSEQ_CUPDATE_RET_TIMESAFE;
        *p_seq  = SEQ_END;
      }
      else
      { 
        p_wk->subseq_ret  = WBM_WIFI_SUBSEQ_CUPDATE_RET_TIMEOVER;
        *p_seq  = SEQ_START_DATE_MSG;
      }
    }
    break;

  case SEQ_START_DATE_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_25, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_CHECK_LOCK );
    break;

  case SEQ_CHECK_LOCK:
    {
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
      if( BATTLE_BOX_SAVE_GetLockFlg( p_bbox_save ) )
      {
        *p_seq  = SEQ_UNLOCK;
      }
      else
      { 
        *p_seq  = SEQ_END;
      }
    }
    break;

  case SEQ_UNLOCK:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
      BATTLE_BOX_SAVE_SetLockFlg( p_bbox_save, FALSE );
      *p_seq  = SEQ_START_SAVE;
    }
    break;

  case SEQ_START_SAVE:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      SaveControl_SaveAsyncInit(p_sv);
      *p_seq  = SEQ_WAIT_SAVE;
    }
    break;

  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT ret;
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      ret = SaveControl_SaveAsyncMain(p_sv);
      if( ret == SAVE_RESULT_OK )
      { 
        *p_seq  = SEQ_START_UNLOCK_MSG;
      }
      else if( ret == SAVE_RESULT_NG )
      { 
        *p_seq  = SEQ_END;
      }
    }
    break;

  case SEQ_START_UNLOCK_MSG:
    Util_PlayerInfo_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_UNLOCK );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_26, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_SEND_GPF_CUPSTATUS );
    break;

  case SEQ_SEND_GPF_CUPSTATUS:
    { 
      MYSTATUS    *p_my;
      DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA data;
      p_my  = GAMEDATA_GetMyStatus( p_wk->p_param->p_param->p_game_data); 
      GFL_STD_MemClear( &data, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA) );
      data.WifiMatchUpState = DREAM_WORLD_MATCHUP_END;
      WIFIBATTLEMATCH_NET_StartSendGpfData( p_wk->p_net, p_my, &data, HEAPID_WIFIBATTLEMATCH_CORE ); 
      *p_seq  = SEQ_WAIT_GPF_CUPSTATUS;
    }
    break;
    
 case SEQ_WAIT_GPF_CUPSTATUS:
    { 
      WIFIBATTLEMATCH_SEND_GPFDATA_RET ret;
      NHTTPError  error;
      
      ret = WIFIBATTLEMATCH_NET_WaitSendGpfData( p_wk->p_net, &error );
      if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_SUCCESS )
      { 
        *p_seq  = SEQ_END;
      }
      else if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR )
      { 
        //@todo
        GF_ASSERT_MSG( 0, "error%d", error );
        *p_seq  = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    WBM_SEQ_End( p_seqwk );
    break;

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
 *	@brief  WIFI大会  登録解除処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void WbmWifiSubSeq_UnRegister( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  {
    SEQ_CHECK_CUP_STATUS,
    SEQ_START_NOT_CUP_MSG,
    SEQ_START_UNREGISTER_MSG,
    SEQ_START_UNREGISTER_LIST,
    SEQ_SELECT_UNREGISTER_LIST,
    SEQ_START_CONFIRM_MSG,
    SEQ_START_CONFIRM_LIST,
    SEQ_SELECT_CONFIRM_LIST,
    SEQ_UNLOCK,
    SEQ_SEND_GPF_CUPSTATUS,
    SEQ_WAIT_GPF_CUPSTATUS,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,
    SEQ_START_UNLOCK_MSG,
    SEQ_END,
    SEQ_WAIT_MSG,

  };

  WIFIBATTLEMATCH_WIFI_WORK	  *p_wk	    = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_wk->p_param;

  switch( *p_seq )
  { 
  case SEQ_CHECK_CUP_STATUS:
    if( p_wk->gpf_data.WifiMatchUpState == DREAM_WORLD_MATCHUP_ENTRY )
    { 
      p_wk->subseq_ret  = WBM_WIFI_SUBSEQ_UNREGISTER_RET_FALSE;
      *p_seq  = SEQ_START_UNREGISTER_MSG;
    }
    else
    { 
      p_wk->subseq_ret  = WBM_WIFI_SUBSEQ_UNREGISTER_RET_NONE;
      *p_seq  = SEQ_START_NOT_CUP_MSG;
    }
    break;
  case SEQ_START_NOT_CUP_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_27, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_END );
    break;
  case SEQ_START_UNREGISTER_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_28, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_UNREGISTER_LIST );
    break;
  case SEQ_START_UNREGISTER_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_SELECT_UNREGISTER_LIST;
    break;
  case SEQ_SELECT_UNREGISTER_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_START_CONFIRM_MSG;
          break;
        case 1: //いいえ
          *p_seq  = SEQ_END;
          break;
        }
      }
    }
    break;
  case SEQ_START_CONFIRM_MSG:
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_29, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_CONFIRM_LIST );
    break;
  case SEQ_START_CONFIRM_LIST:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq     = SEQ_SELECT_CONFIRM_LIST;
    break;
  case SEQ_SELECT_CONFIRM_LIST:
    {
      const u32 select  = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_UNLOCK;
          break;
        case 1: //いいえ
          *p_seq  = SEQ_END;
          break;
        }
      }
    }
    break;
  case SEQ_UNLOCK:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
      BATTLE_BOX_SAVE_SetLockFlg( p_bbox_save, FALSE );
      *p_seq  = SEQ_SEND_GPF_CUPSTATUS;
    }
    break;
  case SEQ_SEND_GPF_CUPSTATUS:
    { 
      MYSTATUS    *p_my;
      DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA data;
      p_my  = GAMEDATA_GetMyStatus( p_wk->p_param->p_param->p_game_data); 
      GFL_STD_MemClear( &data, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA) );
      data.WifiMatchUpState = DREAM_WORLD_MATCHUP_RETIRE;
      WIFIBATTLEMATCH_NET_StartSendGpfData( p_wk->p_net, p_my, &data, HEAPID_WIFIBATTLEMATCH_CORE ); 
      *p_seq  = SEQ_WAIT_GPF_CUPSTATUS;
    }
    break;

  case SEQ_WAIT_GPF_CUPSTATUS:
    { 
      WIFIBATTLEMATCH_SEND_GPFDATA_RET ret;
      NHTTPError  error;
      
      ret = WIFIBATTLEMATCH_NET_WaitSendGpfData( p_wk->p_net, &error );
      if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_SUCCESS )
      { 
        *p_seq  = SEQ_START_SAVE;
      }
      else if( ret == WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR )
      { 
        //@todo
        GF_ASSERT_MSG( 0, "error%d", error );
        *p_seq  = SEQ_START_SAVE;
      }
    } 
    break;

  case SEQ_START_SAVE:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      SaveControl_SaveAsyncInit(p_sv);
      *p_seq  = SEQ_WAIT_SAVE;
    }
    break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT ret;
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_param->p_param->p_game_data );
      ret = SaveControl_SaveAsyncMain(p_sv);
      if( ret == SAVE_RESULT_OK )
      { 
        p_wk->subseq_ret  = WBM_WIFI_SUBSEQ_UNREGISTER_RET_TRUE;
        *p_seq  = SEQ_START_UNLOCK_MSG;
      }
      else if( ret == SAVE_RESULT_NG )
      { 
        *p_seq  = SEQ_END;
      }
    }
    break;
  case SEQ_START_UNLOCK_MSG:
    Util_PlayerInfo_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_UNLOCK );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, WIFIMATCH_WIFI_STR_26, WBM_TEXT_TYPE_STREAM );
    *p_seq       = SEQ_END;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_SEND_GPF_CUPSTATUS );
    break;
  case SEQ_END:
    WBM_SEQ_End( p_seqwk );
    break;

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
 *  便利
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を表示
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_PlayerInfo_Create( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{ 
  if( p_wk->p_playerinfo == NULL )
  {
    MYSTATUS    *p_my;
    GFL_CLUNIT	*p_unit;
    SAVE_CONTROL_WORK *p_sv;
    BATTLE_BOX_SAVE   *p_bbox_save;

    PLAYERINFO_WIFICUP_DATA info_setup;

    const REGULATION_CARDDATA *cp_reg_card  = SaveData_GetRegulationCardData(GAMEDATA_GetSaveControlWork( p_wk->p_param->p_param->p_game_data ));


    p_my  = GAMEDATA_GetMyStatus( p_wk->p_param->p_param->p_game_data); 
    p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
    p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_param->p_game_data );
    p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );


    //自分のデータを表示
    GFL_STD_MemClear( &info_setup, sizeof(PLAYERINFO_WIFICUP_DATA) );
    GFL_STD_MemCopy( Regulation_GetCardCupNamePointer( cp_reg_card ), info_setup.cup_name, 2*(WIFI_PLAYER_TIX_CUPNAME_MOJINUM + EOM_SIZE) );

    info_setup.start_date = GFDATE_Set( 
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_START_YEAR ),
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_START_MONTH ), 
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_START_DAY ),
          0);

    info_setup.end_date = GFDATE_Set( 
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_END_YEAR ),
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_END_MONTH ), 
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_END_DAY ),
          0);

    info_setup.trainerID  = MyStatus_GetTrainerView( p_my );

    info_setup.rate = p_wk->sake_data.rate;
    info_setup.btl_cnt = p_wk->sake_data.win + p_wk->sake_data.lose;

    p_wk->p_playerinfo	= PLAYERINFO_WIFI_Init( &info_setup, FALSE, p_my, p_unit, p_wk->p_res, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, p_bbox_save, FALSE, HEAPID_WIFIBATTLEMATCH_CORE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を破棄
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_PlayerInfo_Delete( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{ 
  if( p_wk->p_playerinfo )
  { 
    PLAYERINFO_WIFI_Exit( p_wk->p_playerinfo );
    p_wk->p_playerinfo  = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のカードをスライドイン
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk  ワーク
 *
 *	@return TRUEで完了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL Util_PlayerInfo_Move( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{ 
  return PLAYERINFO_MoveMain( p_wk->p_playerinfo );
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のカードのデータを更新
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk
 *	@param	type 
 */
//-----------------------------------------------------------------------------
static void Util_PlayerInfo_RenewalData( WIFIBATTLEMATCH_WIFI_WORK *p_wk, PLAYERINFO_WIFI_UPDATE_TYPE type )
{ 
  PLAYERINFO_WIFI_RenewalData( p_wk->p_playerinfo, type, p_wk->p_msg, p_wk->p_que, p_wk->p_font, HEAPID_WIFIBATTLEMATCH_CORE );

}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者の情報を表示
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_MatchInfo_Create( WIFIBATTLEMATCH_WIFI_WORK *p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_enemy_data )
{ 
  if( p_wk->p_matchinfo == NULL )
  { 
    GFL_CLUNIT  *p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
    p_wk->p_matchinfo		= MATCHINFO_Init( cp_enemy_data, p_unit, p_wk->p_res, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, p_wk->p_param->p_param->mode, HEAPID_WIFIBATTLEMATCH_CORE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者の情報を破棄
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_MatchInfo_Delete( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
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
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk  ワーク
 *
 *	@return TRUEで完了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL Util_MatchInfo_Main( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{
  return MATCHINFO_MoveMain( p_wk->p_matchinfo );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックスを表示
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_BtlBox_Create( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{ 
  if( p_wk->p_btlbox == NULL )
  { 
    GFL_CLUNIT  *p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_param->p_game_data );
    BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
    p_wk->p_btlbox  = WBM_BTLBOX_Init( p_bbox_save, p_unit, p_wk->p_res, p_wk->p_small_font, p_wk->p_que, p_wk->p_msg, HEAPID_WIFIBATTLEMATCH_CORE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックスを表示
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_BtlBox_Delete( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{ 
  if( p_wk->p_btlbox )
  { 
    WBM_BTLBOX_Exit( p_wk->p_btlbox );
    p_wk->p_btlbox  = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックスを移動
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 *	@retval TRUE移動完了 FALSE移動中
 */
//-----------------------------------------------------------------------------
static BOOL Util_BtlBox_MoveIn( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{ 
  return WBM_BTLBOX_MoveInMain( p_wk->p_btlbox );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックスを移動
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 *	@retval TRUE移動完了 FALSE移動中
 */
//-----------------------------------------------------------------------------
static BOOL Util_BtlBox_MoveOut( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{ 
  return WBM_BTLBOX_MoveOutMain( p_wk->p_btlbox );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk  ワーク
 *	@param	type                            リストの種類
 */
//-----------------------------------------------------------------------------
static void Util_List_Create( WIFIBATTLEMATCH_WIFI_WORK *p_wk, UTIL_LIST_TYPE type )
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
      setup.strID[0]= WIFIMATCH_WIFI_SELECT_04;
      setup.strID[1]= WIFIMATCH_WIFI_SELECT_05;
      setup.list_max= 2;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      break;

    case UTIL_LIST_TYPE_JOIN:
      setup.strID[0]= WIFIMATCH_WIFI_SELECT_01;
      setup.strID[1]= WIFIMATCH_WIFI_SELECT_02;
      setup.strID[2]= WIFIMATCH_WIFI_SELECT_03;
      setup.list_max= 3;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      break;

    case UTIL_LIST_TYPE_DECIDE:
      setup.strID[0]= WIFIMATCH_WIFI_SELECT_06;
      setup.strID[1]= WIFIMATCH_WIFI_SELECT_07;
      setup.list_max= 2;
      p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_WIFIBATTLEMATCH_CORE );
      break;

    case UTIL_LIST_TYPE_CUPMENU:
      setup.strID[0]= WIFIMATCH_WIFI_SELECT_08;
      setup.strID[1]= WIFIMATCH_WIFI_SELECT_09;
      setup.strID[2]= WIFIMATCH_WIFI_SELECT_10;
      setup.list_max= 3;
      p_wk->p_list  = WBM_LIST_InitEx( &setup, 32/2 - 26/2, (24-6)/2 - 3*2/2, 26, 3*2, HEAPID_WIFIBATTLEMATCH_CORE );
      break;
    }

  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト破棄
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_List_Delete( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
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
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 *
 *	@return 選択したもの
 */
//-----------------------------------------------------------------------------
static u32 Util_List_Main( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{ 
  if( p_wk->p_list )
  { 
    return WBM_LIST_Main( p_wk->p_list );
  }
  else
  { 
    NAGI_Printf( "List not exist !!!\n" );
    return WBM_LIST_SELECT_NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  サブシーケンススタート
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk   ワーク
 *	@param	seq_function                      サブシーケンス
 */
//-----------------------------------------------------------------------------
static void Util_SubSeq_Start( WIFIBATTLEMATCH_WIFI_WORK *p_wk, WBM_SEQ_FUNCTION seq_function )
{ 
  WBM_SEQ_SetNext( p_wk->p_subseq, seq_function );
}

//----------------------------------------------------------------------------
/**
 *	@brief  サブシーケンスメイン
 *
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *p_wk ワーク
 *
 *	@return WBM_WIFI_SUBSEQ_RET_NONE動作中  それ以外のときはサブシーケンスが終了し戻り値を返している
 */
//-----------------------------------------------------------------------------
static WBM_WIFI_SUBSEQ_RET Util_SubSeq_Main( WIFIBATTLEMATCH_WIFI_WORK *p_wk )
{ 
  WBM_SEQ_Main( p_wk->p_subseq );
  if( WBM_SEQ_IsEnd( p_wk->p_subseq ) )
  {
    return p_wk->subseq_ret;
  }
  else
  { 
    return WBM_WIFI_SUBSEQ_RET_NONE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分のデータを格納
 *	        SAKE GPFからデータをもらってきていること
 *
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *p_my_data  データ格納先
 *	@param	WIFIBATTLEMATCH_WIFI_WORK *cp_wk      ワーク
 */
//-----------------------------------------------------------------------------
static void Util_SetupMyData( WIFIBATTLEMATCH_ENEMYDATA *p_my_data, const WIFIBATTLEMATCH_WIFI_WORK *cp_wk )
{ 
  GFL_STD_MemClear( p_my_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );

  {
    MYSTATUS  *p_my;
    p_my  = GAMEDATA_GetMyStatus(cp_wk->p_param->p_param->p_game_data);
    STRTOOL_Copy( MyStatus_GetMyName(p_my), p_my_data->name, PERSON_NAME_SIZE+EOM_SIZE );
    p_my_data->sex           = MyStatus_GetMySex(p_my);
    p_my_data->trainer_view  = MyStatus_GetTrainerView(p_my);

    p_my_data->nation  = MyStatus_GetMyNation(p_my); 
    p_my_data->area    = MyStatus_GetMyArea(p_my); 
  }
  { 
    const MYPMS_DATA *cp_mypms;
    SAVE_CONTROL_WORK *p_sv;
    p_sv            = GAMEDATA_GetSaveControlWork(cp_wk->p_param->p_param->p_game_data);
    cp_mypms        = SaveData_GetMyPmsDataConst( p_sv );
    MYPMS_GetPms( cp_mypms, MYPMS_PMS_TYPE_INTRODUCTION, &p_my_data->pms );
  }
  p_my_data->win_cnt    = cp_wk->sake_data.win;
  p_my_data->lose_cnt   = cp_wk->sake_data.lose;
  p_my_data->rate       = cp_wk->sake_data.rate;

  { 
    const REGULATION_CARDDATA *cp_reg_card  = SaveData_GetRegulationCardData(GAMEDATA_GetSaveControlWork( cp_wk->p_param->p_param->p_game_data ));
    p_my_data->wificup_no      = Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_CUPNO );
  }

  GFL_STD_MemCopy( cp_wk->sake_data.pokeparty, p_my_data->pokeparty, PokeParty_GetWorkSize() );
}
