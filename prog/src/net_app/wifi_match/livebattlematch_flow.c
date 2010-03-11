//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		livebattlematch_flow.c
 *	@brief  ライブ大会フロー
 *	@author	Toru=Nagihashi
 *	@date		2010.03.06
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID
#include "poke_tool/pokeparty.h"
#include "gamesystem/btl_setup.h"
#include "print/str_tool.h"
#include "poke_tool/poke_regulation.h"
#include "sound/pm_sndsys.h"
#include "system/net_err.h"
#include "battle/btl_net.h" //BTL_NET_SERVER_VERSION

//プロセス


//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_live_match.h"
#include "font/font.naix"

//セーブデータ
#include "savedata/my_pms_data.h"
#include "savedata/battlematch_savedata.h"
#include "savedata/livematch_savedata.h"

//ネット
#include "net/network_define.h"
#include "net/dreamworld_netdata.h"

//WIFIBATTLEMATCHのモジュール
#include "wifibattlematch_util.h"
#include "wifibattlematch_graphic.h"
#include "wifibattlematch_view.h"
#include "livebattlematch_irc.h"

//外部公開
#include "livebattlematch_flow.h"

FS_EXTERN_OVERLAY( ui_common );

//-------------------------------------
///	DEBUG
//=====================================
#ifdef PM_DEBUG
#define DEBUG_REGULATIONCRC_PASS  //レギュレーションのCRCチェックを通過する
#define DEBUG_REGULATION_RECVCHECK_PASS  //レギュレーションの受信チェックを通過する
#define DEBUG_REGULATION_RECV_A_PASS  //レギュレーションの受信をAボタンで進む
#endif //PM_DEBUG

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

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ライブ大会ワーク
//=====================================
struct _LIVEBATTLEMATCH_FLOW_WORK
{ 
  //シーケンス制御モジュール
  WBM_SEQ_WORK                  *p_seq;

  //選択肢表示モジュール
  WBM_LIST_WORK                 *p_list;
  u32                           list_type;

  //ネットモジュール
  LIVEBATTLEMATCH_IRC_WORK      *p_irc;

	//上画面情報
	PLAYERINFO_WORK             	*p_playerinfo;

	//対戦者情報
	MATCHINFO_WORK              	*p_matchinfo;

  //バトルボックス
  WBM_BTLBOX_WORK               *p_btlbox;

  //待機アイコン
  WBM_WAITICON_WORK             *p_wait;

  //ライブ大会用メッセージ
  GFL_MSGDATA                   *p_msg;

  //選手証用メッセージ(wifi_matchと共用)
  GFL_MSGDATA                   *p_wifi_msg;

  //ポケモンのレベル・性別マーク表示のみで使用しているフォント
	GFL_FONT			                *p_small_font; 

  //単語登録
  WORDSET                       *p_word;

  //ポケパーティテンポラリ
  POKEPARTY                     *p_party;

  //ライブマッチセーブデータ
  LIVEMATCH_DATA                *p_livematch;

  //デジタル選手証  セーブデータ
  REGULATION_CARDDATA           *p_regulation;

  //デジタル選手証　受信バッファ
  REGULATION_CARDDATA           regulation_temp;

  //戻り値
  LIVEBATTLEMATCH_FLOW_RET      retcode;

  //引数
  LIVEBATTLEMATCH_FLOW_PARAM    param;

  //ヒープID
  HEAPID                        heapID;

  //汎用カウンタ
  u32                           cnt;

  //ふりかえり用対戦録画があるか
  BOOL                          is_rec;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	シーケンス関数
//=====================================
static void SEQFUNC_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void SEQFUNC_RecvCard( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Register( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_StartCup( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Matching( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_BtlAfter( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_RecAfter( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//-------------------------------------
///	便利関数
//=====================================
//リスト
typedef enum
{ 
  LVM_MENU_TYPE_YESNO,
  LVM_MENU_TYPE_MENU,
  LVM_MENU_TYPE_END,
}LVM_MENU_TYPE;
static void UTIL_LIST_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk, LVM_MENU_TYPE type );
static void UTIL_LIST_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
static u32 UTIL_LIST_Main( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
//テキスト
static void UTIL_TEXT_Print( LIVEBATTLEMATCH_FLOW_WORK *p_wk, u32 strID );
static BOOL UTIL_TEXT_IsEnd( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
//フロー終了
static void UTIL_FLOW_End( LIVEBATTLEMATCH_FLOW_WORK *p_wk, LIVEBATTLEMATCH_FLOW_RET ret );
//自分のカード作成
static void UTIL_PLAYERINFO_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
static void UTIL_PLAYERINFO_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
static BOOL UTIL_PLAYERINFO_Move( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
static void UTIL_PLAYERINFO_RenewalData( LIVEBATTLEMATCH_FLOW_WORK *p_wk, PLAYERINFO_WIFI_UPDATE_TYPE type );
//対戦相手のカード作成
static void UTIL_MATCHINFO_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_enemy_data );
static void UTIL_MATCHINFO_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
static BOOL UTIL_MATCHINFO_Main( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
//バトルボックス
static void UTIL_BTLBOX_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
static void UTIL_BTLBOX_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
static BOOL UTIL_BTLBOX_MoveIn( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
static BOOL UTIL_BTLBOX_MoveOut( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
//ポケパーティを設定
static void UTIL_DATA_GetBtlBoxParty( LIVEBATTLEMATCH_FLOW_WORK *p_wk, POKEPARTY *p_dst );
//自分のデータ作成
static void UTIL_DATA_SetupMyData( WIFIBATTLEMATCH_ENEMYDATA *p_my_data, LIVEBATTLEMATCH_FLOW_WORK *p_wk );
//=============================================================================
/**
 *					データ
*/
//=============================================================================
//=============================================================================
/**
 *					外部公開
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会フロー  初期
 *
 *	@param	const LIVEBATTLEMATCH_FLOW_PARAM *cp_param  引数
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LIVEBATTLEMATCH_FLOW_WORK *LIVEBATTLEMATCH_FLOW_Init( const LIVEBATTLEMATCH_FLOW_PARAM *cp_param, HEAPID heapID )
{ 
  LIVEBATTLEMATCH_FLOW_WORK *p_wk;

  //WIFIBATTLEMATCH_VIEWのPLAYERINFO_LIVE_Initの中で使用するため
  GFL_OVERLAY_Load(  FS_OVERLAY_ID( ui_common ) );

  //ワーク作成
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(LIVEBATTLEMATCH_FLOW_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(LIVEBATTLEMATCH_FLOW_WORK) );
  p_wk->param   = *cp_param;
  p_wk->heapID  = heapID;
  p_wk->is_rec  = BattleRec_DataSetCheck();

  //リソース追加
  WIFIBATTLEMATCH_VIEW_LoadScreen( p_wk->param.p_view, WIFIBATTLEMATCH_VIEW_RES_MODE_LIVE, heapID );

  //共通モジュール作成
  p_wk->p_word  = WORDSET_Create( heapID );
  p_wk->p_msg       = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_live_match_dat, heapID );
  p_wk->p_wifi_msg  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_match_dat, heapID );

  //共通フォント作成
	p_wk->p_small_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_num_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

  //モジュール作成
  p_wk->p_seq = WBM_SEQ_Init( p_wk, SEQFUNC_Start, heapID );
  p_wk->p_irc = LIVEBATTLEMATCH_IRC_Init( p_wk->param.p_gamedata, heapID );

  //待機アイコン
  { 
    GFL_CLUNIT  *p_unit;
    p_unit  = WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->param.p_graphic );
    p_wk->p_wait  = WBM_WAITICON_Init( p_unit, p_wk->param.p_view, heapID );
    WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
  }

  //LIVE大会用レギュレーションセーブアドレス
  { 
    SAVE_CONTROL_WORK   *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
    REGULATION_SAVEDATA *p_reg_sv   = SaveData_GetRegulationSaveData( p_sv_ctrl );
    p_wk->p_regulation  = RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_LIVE );
  }

  //ライブマッチ用セーブデータ
  {
    SAVE_CONTROL_WORK   *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
    BATTLEMATCH_DATA    *p_btlmatch_sv  = SaveData_GetBattleMatch( p_sv_ctrl );
    p_wk->p_livematch = BATTLEMATCH_GetLiveMatch( p_btlmatch_sv );
  }

  //ポケパーティテンポラリ
  p_wk->p_party = PokeParty_AllocPartyWork( heapID );


  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会フロー  破棄
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIVEBATTLEMATCH_FLOW_Exit( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  GFL_HEAP_FreeMemory( p_wk->p_party );

  //モジュール破棄
  UTIL_PLAYERINFO_Delete( p_wk );
  UTIL_MATCHINFO_Delete( p_wk );
  WBM_WAITICON_Exit( p_wk->p_wait );
  LIVEBATTLEMATCH_IRC_Exit( p_wk->p_irc );
  WBM_SEQ_Exit( p_wk->p_seq);

  //共通モジュール破棄
  GFL_FONT_Delete( p_wk->p_small_font );
  GFL_MSG_Delete( p_wk->p_wifi_msg );
  GFL_MSG_Delete( p_wk->p_msg );
  WORDSET_Delete( p_wk->p_word );

  GFL_OVERLAY_Unload(  FS_OVERLAY_ID( ui_common ) );

  //ワーク破棄
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会フロー  メイン処理
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIVEBATTLEMATCH_FLOW_Main( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{
  //シーケンスメイン
  WBM_SEQ_Main( p_wk->p_seq );

  //ネットメイン
  LIVEBATTLEMATCH_IRC_Main( p_wk->p_irc );

  //待機アイコン
  WBM_WAITICON_Main( p_wk->p_wait );


  //文字表示
  if( p_wk->p_playerinfo )
  { 
    PLAYERINFO_PrintMain( p_wk->p_playerinfo, p_wk->param.p_que );
  }
  if( p_wk->p_matchinfo )
  { 
    MATCHINFO_PrintMain( p_wk->p_matchinfo, p_wk->param.p_que );
  }
  if( p_wk->p_btlbox )
  { 
    WBM_BTLBOX_PrintMain( p_wk->p_btlbox, p_wk->param.p_que );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会フロー  終了検知
 *
 *	@param	const LIVEBATTLEMATCH_FLOW_WORK *cp_wk  ワーク
 *
 *	@return 終了コード（詳細は列挙型を参照）
 */
//-----------------------------------------------------------------------------
LIVEBATTLEMATCH_FLOW_RET LIVEBATTLEMATCH_FLOW_IsEnd( const LIVEBATTLEMATCH_FLOW_WORK *cp_wk )
{ 
  if( WBM_SEQ_IsEnd( cp_wk->p_seq )  )
  { 
    return cp_wk->retcode;
  }
  return LIVEBATTLEMATCH_FLOW_RET_CONTINUE;
}

//=============================================================================
/**
 *  シーケンス関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	開始
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  LIVEBATTLEMATCH_FLOW_WORK *p_wk = p_wk_adrs;

  switch( p_wk->param.mode )
  { 
  case LIVEBATTLEMATCH_FLOW_MODE_START:  //最初から開始
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_RecvCard );
    break;
  case LIVEBATTLEMATCH_FLOW_MODE_MENU:    //メインメニューから開始
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_StartCup );
    break;
  case LIVEBATTLEMATCH_FLOW_MODE_BTL:    //バトル後から開始
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_BtlAfter );
    break;
  case LIVEBATTLEMATCH_FLOW_MODE_REC:    //録画後から開始
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_RecAfter );
    break;
  default:
    GF_ASSERT(0);
  }
  
}
//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_seqwk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  //終了
  WBM_SEQ_End( p_seqwk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	選手証受信シーケンス
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_RecvCard( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_CHECK_CARD,       //選手証の存在チェック
    SEQ_START_MSG_NOLOCK, //すでに選手証があって、バトルボックスがロックされていない
    SEQ_START_MSG_LOCK,   //すでに選手証があって、バトルボックスがロックされている

    SEQ_START_MSG_RECVCARD, //選手証受け取り
    SEQ_START_RECVCARD,
    SEQ_WAIT_RECVCARD,
    SEQ_WAIT_RECV_DEBUG,

    SEQ_START_CANCEL,   //キャンセル
    SEQ_WAIT_CANCEL,    //キャンセル
    SEQ_START_MSG_CANCEL,
    SEQ_START_LIST_CANCEL,
    SEQ_WAIT_LIST_CANCEL,

    SEQ_CHECK_RECVCARD, //受け取った選手証をチェック

    SEQ_START_MSG_SAVE, //選手証が正しかったのでサインアップしてセーブ
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,

    SEQ_START_MSG_DIRTY_CRC,    //不正なレギュレーションをうけとった CRC不正
    SEQ_START_MSG_DIRTY_VER,    //不正なレギュレーションをうけとった VERSION不正
    SEQ_START_MSG_RETIRE,   //リタイアした大会を受け取っていた,
    SEQ_START_MSG_FINISH,   //終了した大会を受け取っていた,

    SEQ_SUCCESS_END,      //成功したので次へ
    SEQ_DIRTY_END,        //失敗したので戻る

    SEQ_WAIT_MSG,       //メッセージ待ち
  };

  LIVEBATTLEMATCH_FLOW_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_CHECK_CARD:       //選手証の存在チェック
    {
      SAVE_CONTROL_WORK   *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
      BATTLE_BOX_SAVE     *p_btlbox_sv  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv_ctrl );
      switch( Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_STATUS) )
      { 
      case DREAM_WORLD_MATCHUP_NONE:    // 登録していない状態
      case DREAM_WORLD_MATCHUP_END:	    // 大会終了
      case DREAM_WORLD_MATCHUP_RETIRE:  // 途中で解除
        if( Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_CUPNO) == 0 )
        { 
          //大会がないので受信継続
          *p_seq  = SEQ_START_MSG_RECVCARD;
        }
        else
        { 
          //大会があるので、ロックに相応のメッセージを出してから受信へ
          if( BATTLE_BOX_SAVE_GetLockType( p_btlbox_sv, BATTLE_BOX_LOCK_BIT_LIVE ) )
          { 
            *p_seq  = SEQ_START_MSG_LOCK;
          }
          else
          { 
            *p_seq  = SEQ_START_MSG_NOLOCK;
          }
        }
        break;

      case DREAM_WORLD_MATCHUP_SIGNUP:	  // web登録した。DS->SAKEの接続はまだ
        //すでに受信しているため登録処理へ
        WBM_SEQ_SetNext( p_seqwk, SEQFUNC_Register );
        break;

      case DREAM_WORLD_MATCHUP_ENTRY:	  // 参加中
        //参加しているため大会メインへ
        WBM_SEQ_SetNext( p_seqwk, SEQFUNC_StartCup );
        break;
      }
    }
    break;
  case SEQ_START_MSG_NOLOCK: //すでに選手証があって、バトルボックスがロックされていない
    UTIL_TEXT_Print( p_wk, LIVE_STR_01 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_MSG_RECVCARD );
    break;
  case SEQ_START_MSG_LOCK:   //すでに選手証があって、バトルボックスがロックされている
    UTIL_TEXT_Print( p_wk, LIVE_STR_02 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_MSG_RECVCARD );
    break;

  case SEQ_START_MSG_RECVCARD: //選手証受け取り
    UTIL_TEXT_Print( p_wk, LIVE_STR_03 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_RECVCARD );
    break;
  case SEQ_START_RECVCARD:
    WBM_WAITICON_SetDrawEnable( p_wk->p_wait, TRUE );
    LIVEBATTLEMATCH_IRC_StartRecvRegulation( p_wk->p_irc, &p_wk->regulation_temp );
    *p_seq  = SEQ_WAIT_RECVCARD;
    break;
  case SEQ_WAIT_RECVCARD:
    //受信中
    if( LIVEBATTLEMATCH_IRC_WaitRecvRegulation( p_wk->p_irc ) )
    { 
      //@todo 言語コードが違う場合はまたもらいに行く

      //Regulation_PrintDebug( &p_wk->regulation_temp );
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
      *p_seq  = SEQ_CHECK_RECVCARD;
    }

    //エラー処理
    if( LIVEBATTLEMATCH_IRC_ERROR_REPAIR_DISCONNECT == LIVEBATTLEMATCH_IRC_CheckErrorRepairType( p_wk->p_irc ) )
    { 
      *p_seq  = SEQ_START_MSG_RECVCARD;
      break;
    }

    //Bキャンセル
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    { 
      *p_seq  = SEQ_START_CANCEL;
    }

#ifdef DEBUG_REGULATION_RECV_A_PASS
    //Aで進む
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    { 
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
      LIVEBATTLEMATCH_IRC_StartCancelRecvRegulation( p_wk->p_irc );
      *p_seq  = SEQ_WAIT_RECV_DEBUG;
    }
#endif
    break;

  case SEQ_WAIT_RECV_DEBUG:
    if( LIVEBATTLEMATCH_IRC_WaitCancelRecvRegulation( p_wk->p_irc ) )
    { 
      Regulation_SetDebugData( &p_wk->regulation_temp );
      *p_seq  = SEQ_CHECK_RECVCARD;
    }
    break;

  case SEQ_START_CANCEL:   //キャンセル
    LIVEBATTLEMATCH_IRC_StartCancelRecvRegulation( p_wk->p_irc );
    *p_seq  = SEQ_WAIT_CANCEL;
    break;
  case SEQ_WAIT_CANCEL:    //キャンセル
    if( LIVEBATTLEMATCH_IRC_WaitCancelRecvRegulation( p_wk->p_irc ) )
    { 
      *p_seq  = SEQ_START_MSG_CANCEL;
    }
    break;
  case SEQ_START_MSG_CANCEL:
    WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
    UTIL_TEXT_Print( p_wk, LIVE_STR_17 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_CANCEL );
    break;
  case SEQ_START_LIST_CANCEL:
    UTIL_LIST_Create( p_wk, LVM_MENU_TYPE_YESNO ); 
    *p_seq  = SEQ_WAIT_LIST_CANCEL;
    break;
  case SEQ_WAIT_LIST_CANCEL:
    { 
      const u32 select  = UTIL_LIST_Main( p_wk ); 
      if( select != WBM_LIST_SELECT_NULL )
      { 
        UTIL_LIST_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_DIRTY_END;
          break;

        case 1: //いいえ
          *p_seq  = SEQ_START_MSG_RECVCARD;
          break;
        }
      }
    }
    break;

  case SEQ_CHECK_RECVCARD: //受け取った選手証をチェック
#ifdef DEBUG_REGULATIONCRC_PASS
    if(1)
#else
    //CRCは正しいか
    if( Regulation_CheckCrc( &p_wk->regulation_temp ) )
#endif
    { 
      //VERSIONは正しいか
      if( Regulation_GetCardParam( &p_wk->regulation_temp, REGULATION_CARD_ROMVER) & (1<<GET_VERSION()) )
      { 
        //すでに受けとっているか
        if( Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_CUPNO) ==
            Regulation_GetCardParam( &p_wk->regulation_temp, REGULATION_CARD_CUPNO) )
        { 
#ifdef DEBUG_REGULATION_RECVCHECK_PASS
          { 
            *p_seq  = SEQ_START_MSG_SAVE;
            break;
          }
#endif 
          //同じなので同じ大会に登録しようとした
          switch( Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_STATUS) )
          {   
          case DREAM_WORLD_MATCHUP_RETIRE:  // 途中で解除
            *p_seq  = SEQ_START_MSG_RETIRE;
            break;

          case DREAM_WORLD_MATCHUP_END:	    // 大会終了
            *p_seq  = SEQ_START_MSG_FINISH;
            break;

          default:
            //基本ここにはこないが念のため
            //大会NOは同じだが参加中なので次へいく
            *p_seq  = SEQ_START_MSG_SAVE;
            break;
          }
        }
        else
        { 
          //大会NOが違うので、大丈夫
          *p_seq  = SEQ_START_MSG_SAVE;
        }
      }
      else
      { 
        //VERSIONが間違っていた
        OS_TPrintf( "バージョンが違うよ\n" );
        *p_seq  = SEQ_START_MSG_DIRTY_VER;
      }
    }
    else
    { 
      //CRCが間違っていた
      OS_TPrintf( "CRCが違うよ\n" );
      *p_seq  = SEQ_START_MSG_DIRTY_CRC;
    }
    break;

  case SEQ_START_MSG_SAVE: //選手証が正しかったのでサインアップしてセーブ
    UTIL_TEXT_Print( p_wk, LIVE_STR_09 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SAVE );
    break;
  case SEQ_START_SAVE: //選手証が正しかったのでサインアップしてセーブ
    { 
      SAVE_CONTROL_WORK   *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
      REGULATION_SAVEDATA *p_reg_sv   = SaveData_GetRegulationSaveData( p_sv_ctrl );
      
      //開催状態をサインアップにしてからセーブ
      Regulation_SetCardParam( &p_wk->regulation_temp, REGULATION_CARD_STATUS, DREAM_WORLD_MATCHUP_SIGNUP );
      //新規大会なのでスコアもクリアする
      LIVEMATCH_Init( p_wk->p_livematch );

      OS_TPrintf( "セット前\n" );
      OS_TPrintf( "テンポラリ\n" );
      Regulation_PrintDebug( &p_wk->regulation_temp );
      OS_TPrintf( "セーブ\n" );
      Regulation_PrintDebug( p_wk->p_regulation );
      RegulationSaveData_SetRegulation(p_reg_sv, REGULATION_CARD_TYPE_LIVE, &p_wk->regulation_temp);
      OS_TPrintf( "セット後\n" );
      OS_TPrintf( "テンポラリ\n" );
      Regulation_PrintDebug( &p_wk->regulation_temp );
      OS_TPrintf( "セーブ\n" );
      Regulation_PrintDebug( p_wk->p_regulation );
    }

    GAMEDATA_SaveAsyncStart(p_wk->param.p_gamedata);
    *p_seq  = SEQ_WAIT_SAVE;
    break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_wk->param.p_gamedata);
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        *p_seq  = SEQ_SUCCESS_END;
        break;
      }
    } 
    break;

  case SEQ_START_MSG_DIRTY_CRC:    //不正なレギュレーションをうけとった
    UTIL_TEXT_Print( p_wk, LIVE_STR_22_1 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_DIRTY_END );
    break;

  case SEQ_START_MSG_DIRTY_VER:    //不正なレギュレーションをうけとった
    UTIL_TEXT_Print( p_wk, LIVE_STR_22_2 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_DIRTY_END );
    break;

  case SEQ_START_MSG_RETIRE:   //リタイアした大会を受け取っていた:
    UTIL_TEXT_Print( p_wk, LIVE_STR_04 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_DIRTY_END );
    break;
  case SEQ_START_MSG_FINISH:   //終了した大会を受け取っていた:
    UTIL_TEXT_Print( p_wk, LIVE_STR_05 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_DIRTY_END );
    break;

  case SEQ_SUCCESS_END:      //成功したので次へ

    OS_TPrintf( "セーブ後\n" );
    Regulation_PrintDebug( p_wk->p_regulation );
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_Register );
    break;
  case SEQ_DIRTY_END:        //失敗したので戻る
    UTIL_FLOW_End( p_wk, LIVEBATTLEMATCH_FLOW_RET_LIVEMENU );
    break;

  case SEQ_WAIT_MSG:       //メッセージ待ち
    if( UTIL_TEXT_IsEnd( p_wk ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	大会登録シーケンス
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Register( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_DRAW_CARD,  //選手証とバトルボックス表示
    SEQ_WAIT_DRAW_CARD,
    SEQ_START_MSG_REG,     //バトルボックスのポケモンを登録します
    SEQ_START_LIST_REG_YESNO,
    SEQ_WAIT_LIST_REG_YESNO,

    SEQ_START_MSG_CANCEL,     //参加ポケモンの登録をやめますか
    SEQ_START_LIST_CANCEL_YESNO,
    SEQ_WAIT_LIST_CANCEL_YESNO,

    SEQ_MOVE_BTLBOX,      //バトルボックス離脱
    SEQ_CHECK_REG,        //バトルボックスのポケモンがレギュレーションと一致するかチェック
    SEQ_START_MSG_DIRTY_REG,  //レギュレーションと一致しなかった
    SEQ_LOCK_BTLBOX,      //バトルボックスをロック

    SEQ_START_MSG_SAVE,     //登録完了セーブ
    SEQ_START_SAVE,        
    SEQ_WAIT_SAVE,

    SEQ_START_MSG_COMPLATE, //完了メッセージ

    SEQ_SUCCESS_END,      //成功したので次へ
    SEQ_DIRTY_MOVE_BTLBOX,//失敗したときのバトルボックス離脱
    SEQ_DIRTY_END,        //失敗したので戻る

    SEQ_WAIT_MSG,       //メッセージ待ち 
  };

  LIVEBATTLEMATCH_FLOW_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_DRAW_CARD:  //選手証とバトルボックス表示
    UTIL_PLAYERINFO_Create( p_wk );
    UTIL_PLAYERINFO_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_UNREGISTER );
    UTIL_BTLBOX_Create( p_wk );
    *p_seq  = SEQ_WAIT_DRAW_CARD;
    break;
  case SEQ_WAIT_DRAW_CARD:
    {
      BOOL ret = TRUE;
      ret &= UTIL_PLAYERINFO_Move( p_wk );
      ret &= UTIL_BTLBOX_MoveIn( p_wk );
      if( ret )
      { 
        *p_seq  = SEQ_START_MSG_REG;
      }
    }
    break;
  case SEQ_START_MSG_REG:     //バトルボックスのポケモンを登録します
    UTIL_TEXT_Print( p_wk, LIVE_STR_06 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_REG_YESNO );
    break;
  case SEQ_START_LIST_REG_YESNO:
    UTIL_LIST_Create( p_wk, LVM_MENU_TYPE_YESNO ); 
    *p_seq       = SEQ_WAIT_LIST_REG_YESNO;
    break;
  case SEQ_WAIT_LIST_REG_YESNO:
    { 
      const u32 select  = UTIL_LIST_Main( p_wk ); 
      if( select != WBM_LIST_SELECT_NULL )
      { 
        UTIL_LIST_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_MOVE_BTLBOX;
          break;

        case 1: //いいえ
          *p_seq  = SEQ_START_MSG_CANCEL;
          break;
        }
      }
    }
    break;

  case SEQ_START_MSG_CANCEL:     //参加ポケモンの登録をやめますか
    UTIL_TEXT_Print( p_wk, LIVE_STR_07 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_CANCEL_YESNO );
    break;
  case SEQ_START_LIST_CANCEL_YESNO:
    UTIL_LIST_Create( p_wk, LVM_MENU_TYPE_YESNO ); 
    *p_seq       = SEQ_WAIT_LIST_CANCEL_YESNO;
    break;
  case SEQ_WAIT_LIST_CANCEL_YESNO:
    { 
      const u32 select  = UTIL_LIST_Main( p_wk ); 
      if( select != WBM_LIST_SELECT_NULL )
      { 
        UTIL_LIST_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_DIRTY_MOVE_BTLBOX;
          break;

        case 1: //いいえ
          *p_seq  = SEQ_START_MSG_REG;
          break;
        }
      }
    }
    break;

  case SEQ_MOVE_BTLBOX:      //バトルボックス離脱
    if( UTIL_BTLBOX_MoveOut( p_wk ) )
    { 
      UTIL_BTLBOX_Delete( p_wk );
      *p_seq  = SEQ_CHECK_REG;
    }
    break;
  case SEQ_CHECK_REG:        //バトルボックスのポケモンがレギュレーションと一致するかチェック
    { 
      u32 failed_bit  = 0;
      REGULATION      *p_reg  = RegulationData_GetRegulation( p_wk->p_regulation );
      UTIL_DATA_GetBtlBoxParty( p_wk, p_wk->p_party );
      if( POKE_REG_OK == PokeRegulationMatchLookAtPokeParty(p_reg, p_wk->p_party, &failed_bit) )
      {
        OS_TPrintf( "バトルボックスのポケモンレギュレーションOK\n" );
        *p_seq  = SEQ_LOCK_BTLBOX;
      }
      else
      { 
        OS_TPrintf( "バトルボックスのポケモンレギュレーションNG!! 0x%x \n", failed_bit );
        *p_seq  = SEQ_START_MSG_DIRTY_REG;
      }
    }
    break;
  case SEQ_START_MSG_DIRTY_REG:  //レギュレーションと一致しなかった
    UTIL_TEXT_Print( p_wk, LIVE_STR_08 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_DIRTY_END );
    break;

  case SEQ_LOCK_BTLBOX:      //バトルボックスをロック
    { 
      //バトルボックスのロックと、ロック時のポケモン情報をセーブする
      SAVE_CONTROL_WORK   *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
      { 
        BATTLE_BOX_SAVE     *p_btlbox_sv  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv_ctrl ); 
        REGULATION_SAVEDATA *p_save = SaveData_GetRegulationSaveData( p_sv_ctrl );
        REGULATION_VIEWDATA *p_view_sv =  RegulationSaveData_GetRegulationView( p_save, BATTLE_BOX_LOCK_BIT_LIVE );
        POKEPARTY *p_party  = BATTLE_BOX_SAVE_MakePokeParty( p_btlbox_sv, GFL_HEAP_LOWID(HEAPID_WIFIBATTLEMATCH_SYS) );

        BATTLE_BOX_SAVE_OnLockFlg( p_btlbox_sv, BATTLE_BOX_LOCK_BIT_LIVE );


        RegulationView_SetEazy( p_view_sv, p_party );

        GFL_HEAP_FreeMemory( p_party );
        
      }
    }
    *p_seq  = SEQ_START_MSG_SAVE;
    break;

  case SEQ_START_MSG_SAVE:     //登録完了セーブ
    UTIL_PLAYERINFO_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_LOCK );
    UTIL_TEXT_Print( p_wk, LIVE_STR_09 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SAVE ); 
    break;
  case SEQ_START_SAVE:
    //開催状態を参加中にしてからセーブ
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_STATUS, DREAM_WORLD_MATCHUP_ENTRY );
    GAMEDATA_SaveAsyncStart(p_wk->param.p_gamedata);
    *p_seq  = SEQ_WAIT_SAVE;
    break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_wk->param.p_gamedata);
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        *p_seq  = SEQ_START_MSG_COMPLATE;
        break;
      }
    } 
    break;

  case SEQ_START_MSG_COMPLATE: //完了メッセージ
    UTIL_TEXT_Print( p_wk, LIVE_STR_10 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_SUCCESS_END ); 
    break;

  case SEQ_SUCCESS_END:      //成功したので次へ
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_StartCup );
    break;
    
  case SEQ_DIRTY_MOVE_BTLBOX: //失敗したときのバトルボックス離脱
    if( UTIL_BTLBOX_MoveOut( p_wk ) )
    { 
      UTIL_BTLBOX_Delete( p_wk );
      *p_seq  = SEQ_DIRTY_END;
    }
    break;

  case SEQ_DIRTY_END:        //失敗したので戻る
    UTIL_FLOW_End( p_wk, LIVEBATTLEMATCH_FLOW_RET_LIVEMENU );
    break;

  case SEQ_WAIT_MSG:       //メッセージ待ち 
    if( UTIL_TEXT_IsEnd( p_wk ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief	大会開始シーケンス
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_StartCup( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_DRAW_CARD,  //選手証表示
    SEQ_WAIT_DRAW_CARD,

    SEQ_CHECK_ERR,      //エラーをして戻ってきたか？
    SEQ_START_MSG_ERR,
    
    SEQ_START_MSG_MENU,   //メインメニュー
    SEQ_START_LIST_MENU,
    SEQ_WAIT_LIST_MENU,

    SEQ_START_MSG_UNREG,  //参加を解除する
    SEQ_START_LIST_UNREG,
    SEQ_WAIT_LIST_UNREG,
    SEQ_START_MSG_DECIDE, //本当に～
    SEQ_START_LIST_DECIDE,
    SEQ_WAIT_LIST_DECIDE,
    SEQ_START_MSG_SAVE,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,
    SEQ_START_MSG_UNLOCK,
  
    SEQ_START_MSG_LOOKBACK, //振り返る

    SEQ_LOOKBACK_END,     //振り返るために終了
    SEQ_MATCHING_END,     //マッチングへ
    SEQ_UNREGISTER_END,   //解除して終了した

    SEQ_WAIT_MSG,         //メッセージ待ち 
  };

  LIVEBATTLEMATCH_FLOW_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_DRAW_CARD:  //選手証表示
    UTIL_DATA_SetupMyData( p_wk->param.p_player_data, p_wk );

    if( p_wk->p_playerinfo )
    { 
      *p_seq  = SEQ_START_MSG_MENU;
    }
    else
    { 
      UTIL_PLAYERINFO_Create( p_wk );
      UTIL_PLAYERINFO_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_LOCK );
      *p_seq  = SEQ_WAIT_DRAW_CARD;
    }
		break;
  case SEQ_WAIT_DRAW_CARD:
    if( UTIL_PLAYERINFO_Move( p_wk ) )
    { 
      *p_seq  = SEQ_CHECK_ERR;
    }
		break;

  case SEQ_CHECK_ERR:      //エラーをして戻ってきたか？
    if( p_wk->param.p_btl_score->is_error )
    { 
      *p_seq  = SEQ_START_MSG_ERR;
    }
    else
    { 
      *p_seq  = SEQ_START_MSG_MENU;
    }
    break;

  case SEQ_START_MSG_ERR:
    UTIL_TEXT_Print( p_wk, LIVE_ERR_01 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_MSG_MENU );
    break;
    
  case SEQ_START_MSG_MENU:   //メインメニュー
    UTIL_TEXT_Print( p_wk, LIVE_STR_11 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_MENU );
		break;
  case SEQ_START_LIST_MENU:
    UTIL_LIST_Create( p_wk, LVM_MENU_TYPE_MENU ); 
    *p_seq       = SEQ_WAIT_LIST_MENU;
		break;
  case SEQ_WAIT_LIST_MENU:
    { 
      const u32 select  = UTIL_LIST_Main( p_wk ); 
      if( select != WBM_LIST_SELECT_NULL )
      { 
        UTIL_LIST_Delete( p_wk );
        if( p_wk->is_rec  && !p_wk->param.p_btl_score->is_error )
        { 
          switch( select )
          { 
          case 0: //たいせんする
            *p_seq  = SEQ_MATCHING_END;
            break;

          case 1: //ふりかえる
            *p_seq  = SEQ_START_MSG_LOOKBACK;
            break;

          case 2: //参加解除する
            *p_seq  = SEQ_START_MSG_UNREG;
            break;
          }
        }
        else
        { 
          switch( select )
          { 
          case 0: //たいせんする
            *p_seq  = SEQ_MATCHING_END;
            break;

          case 1: //参加解除する
            *p_seq  = SEQ_START_MSG_UNREG;
            break;
          }
        }
      }
    }
		break;

  case SEQ_START_MSG_UNREG:  //参加を解除する
    UTIL_TEXT_Print( p_wk, LIVE_STR_13 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_UNREG );
		break;
  case SEQ_START_LIST_UNREG:
    UTIL_LIST_Create( p_wk, LVM_MENU_TYPE_YESNO ); 
    *p_seq  = SEQ_WAIT_LIST_UNREG;
		break;
  case SEQ_WAIT_LIST_UNREG:
    { 
      const u32 select  = UTIL_LIST_Main( p_wk ); 
      if( select != WBM_LIST_SELECT_NULL )
      { 
        UTIL_LIST_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_START_MSG_DECIDE;
          break;

        case 1: //いいえ
          *p_seq  = SEQ_START_MSG_MENU;
          break;
        }
      }
    }
		break;
  case SEQ_START_MSG_DECIDE: //本当に～
    UTIL_TEXT_Print( p_wk, LIVE_STR_14 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_DECIDE );
		break;
  case SEQ_START_LIST_DECIDE:
    UTIL_LIST_Create( p_wk, LVM_MENU_TYPE_YESNO ); 
    *p_seq  = SEQ_WAIT_LIST_DECIDE;
		break;
  case SEQ_WAIT_LIST_DECIDE:
    { 
      const u32 select  = UTIL_LIST_Main( p_wk ); 
      if( select != WBM_LIST_SELECT_NULL )
      { 
        UTIL_LIST_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          *p_seq  = SEQ_START_MSG_SAVE;
          break;

        case 1: //いいえ
          *p_seq  = SEQ_START_MSG_MENU;
          break;
        }
      }
    }
		break;
  case SEQ_START_MSG_SAVE:
    UTIL_TEXT_Print( p_wk, LIVE_STR_09 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SAVE ); 
		break;
  case SEQ_START_SAVE:
    //解除してからセーブ
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_STATUS, DREAM_WORLD_MATCHUP_RETIRE );
    { 
      SAVE_CONTROL_WORK   *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
      BATTLE_BOX_SAVE     *p_btlbox_sv  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv_ctrl ); 
      BATTLE_BOX_SAVE_OffLockFlg( p_btlbox_sv, BATTLE_BOX_LOCK_BIT_LIVE );
    }

    GAMEDATA_SaveAsyncStart(p_wk->param.p_gamedata);
    *p_seq  = SEQ_WAIT_SAVE;
		break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_wk->param.p_gamedata);
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        *p_seq  = SEQ_START_MSG_UNLOCK;
        break;
      }
    } 
		break;
  case SEQ_START_MSG_UNLOCK:
    UTIL_PLAYERINFO_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_UNLOCK );
    UTIL_TEXT_Print( p_wk, LIVE_STR_15 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_UNREGISTER_END );
		break;
  
  case SEQ_START_MSG_LOOKBACK: //振り返る
    UTIL_TEXT_Print( p_wk, LIVE_STR_12 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_LOOKBACK_END );
		break;

  case SEQ_LOOKBACK_END:     //振り返るために終了
    UTIL_FLOW_End( p_wk, LIVEBATTLEMATCH_FLOW_RET_BTLREC );
		break;
  case SEQ_MATCHING_END:     //マッチングへ
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_Matching );
		break;
  case SEQ_UNREGISTER_END:   //解除して終了した
    UTIL_FLOW_End( p_wk, LIVEBATTLEMATCH_FLOW_RET_LIVEMENU );
		break;

  case SEQ_WAIT_MSG:         //メッセージ待ち 
    if( UTIL_TEXT_IsEnd( p_wk ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    } 
		break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	マッチング処理シーケンス
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Matching( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_MSG_MATCHING, //マッチング開始
    SEQ_START_CONNECT,      //接続開始
    SEQ_WAIT_CONNECT,       

    SEQ_START_CANCEL_MATCHING,
    SEQ_WAIT_CANCEL_MATCHING,
    SEQ_START_MSG_CANCEL_MATCHING, //マッチングやめる
    SEQ_START_LIST_CANCEL_MATCHING,
    SEQ_START_LSIT_CANCEL_MATCHING,

    SEQ_SEND_ENEMYDATA,      //情報交換
    SEQ_RECV_ENEMYDATA,      //情報交換
    SEQ_CHECK_MATCH,      //マッチングした
    SEQ_START_DISCONNECT_NOCUP,  //大会が違うため切断
    SEQ_WAIT_DISCONNECT_NOCUP,
    SEQ_START_MSG_NOCUP,  //大会が違うメッセージ
    SEQ_START_MSG_OK,     //マッチング相手が見つかった！
    SEQ_START_MSG_CNT,
    SEQ_START_DRAW_OTHERCARD,
    SEQ_WAIT_DRAW_OTHERCARD,
    SEQ_WAIT_SYNC,
    SEQ_START_MSG_WAIT,
    SEQ_START_TIMING,
    SEQ_WAIT_TIMING,

    SEQ_CHECK_SHOW,       //みせあいはあるか
    SEQ_LIST_END,         //リストへ

    SEQ_WAIT_MSG,         //メッセージ待ち 
  };

  LIVEBATTLEMATCH_FLOW_WORK *p_wk = p_wk_adrs;


  //エラー処理
  if( LIVEBATTLEMATCH_IRC_ERROR_REPAIR_DISCONNECT == LIVEBATTLEMATCH_IRC_CheckErrorRepairType( p_wk->p_irc ) )
  { 
    p_wk->param.p_btl_score->is_error = TRUE;
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_StartCup );
  }

  //シーケンス
  switch( *p_seq )
  { 
  case SEQ_START_MSG_MATCHING: //マッチング開始
    UTIL_TEXT_Print( p_wk, LIVE_STR_16 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_CONNECT ); 
		break;
  case SEQ_START_CONNECT:
    if( LIVEBATTLEMATCH_IRC_StartConnect( p_wk->p_irc ) )
    { 
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, TRUE );

      *p_seq  = SEQ_WAIT_CONNECT;
    }
    break;
  case SEQ_WAIT_CONNECT:
    if( LIVEBATTLEMATCH_IRC_WaitConnect( p_wk->p_irc ) )
    { 
      *p_seq  = SEQ_SEND_ENEMYDATA;
    }

    //キャンセル処理
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    { 
      *p_seq  = SEQ_START_CANCEL_MATCHING;
    }
    break;

  case SEQ_START_CANCEL_MATCHING:
    LIVEBATTLEMATCH_IRC_StartCancelConnect( p_wk->p_irc );
    *p_seq  = SEQ_WAIT_CANCEL_MATCHING;
		break;
  case SEQ_WAIT_CANCEL_MATCHING:
    if( LIVEBATTLEMATCH_IRC_WaitCancelConnect( p_wk->p_irc ) )
    { 
      *p_seq  = SEQ_START_MSG_CANCEL_MATCHING;
    }
		break;
  case SEQ_START_MSG_CANCEL_MATCHING: //マッチングやめる
    UTIL_TEXT_Print( p_wk, LIVE_STR_17 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_CANCEL_MATCHING );
		break;
  case SEQ_START_LIST_CANCEL_MATCHING:
    UTIL_LIST_Create( p_wk, LVM_MENU_TYPE_YESNO ); 
    *p_seq       = SEQ_START_LSIT_CANCEL_MATCHING;
    break;
  case SEQ_START_LSIT_CANCEL_MATCHING:
    { 
      const u32 select  = UTIL_LIST_Main( p_wk ); 
      if( select != WBM_LIST_SELECT_NULL )
      { 
        UTIL_LIST_Delete( p_wk );
        switch( select )
        { 
        case 0: //はい
          WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
          WBM_SEQ_SetNext( p_seqwk, SEQFUNC_StartCup );
          break;

        case 1: //いいえ
          *p_seq  = SEQ_START_CONNECT;
          break;
        }
      }
    }
    break;

  case SEQ_SEND_ENEMYDATA:      //情報交換
    if( LIVEBATTLEMATCH_IRC_StartEnemyData( p_wk->p_irc, p_wk->param.p_player_data ) )
    { 
      *p_seq  = SEQ_RECV_ENEMYDATA;
    }
    break;
  case SEQ_RECV_ENEMYDATA:      //情報交換
    { 
      WIFIBATTLEMATCH_ENEMYDATA *p_buff_adrs; 
      if( LIVEBATTLEMATCH_IRC_WaitEnemyData( p_wk->p_irc, &p_buff_adrs ) )
      { 
        GFL_STD_MemCopy( p_buff_adrs, p_wk->param.p_enemy_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
        *p_seq  = SEQ_CHECK_MATCH;
      }
    }
    break;
  case SEQ_CHECK_MATCH:      //マッチングした
    if( p_wk->param.p_player_data->wificup_no == p_wk->param.p_enemy_data->wificup_no )
    { 
      OS_TPrintf( "自分の大会 %d 相手の大会 %d\n", p_wk->param.p_player_data->wificup_no, p_wk->param.p_enemy_data->wificup_no );
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
      *p_seq  = SEQ_START_MSG_OK;
    }
    else
    { 
      WBM_WAITICON_SetDrawEnable( p_wk->p_wait, FALSE );
      *p_seq  = SEQ_START_DISCONNECT_NOCUP;
    }
		break;

  case SEQ_START_DISCONNECT_NOCUP:  //大会が違うため切断
    if( LIVEBATTLEMATCH_IRC_StartDisConnect(p_wk->p_irc))
    { 
      *p_seq  = SEQ_WAIT_DISCONNECT_NOCUP;
    }
    break;
  case SEQ_WAIT_DISCONNECT_NOCUP:
    if( LIVEBATTLEMATCH_IRC_WaitDisConnect(p_wk->p_irc))
    { 
      *p_seq  = SEQ_START_MSG_NOCUP;
    }
    break;
  case SEQ_START_MSG_NOCUP:  //大会が違うメッセージ
    UTIL_TEXT_Print( p_wk, LIVE_STR_18 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_MSG_MATCHING );
		break;
  case SEQ_START_MSG_OK:     //マッチング相手が見つかった！
    UTIL_TEXT_Print( p_wk, LIVE_STR_19 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_MSG_CNT );
		break;
  case SEQ_START_MSG_CNT:
    if( p_wk->cnt++ > MATCHING_MSG_WAIT_SYNC )
    { 
      p_wk->cnt      = 0;
      GFL_BG_SetVisible( BG_FRAME_M_TEXT, FALSE );
      *p_seq      = SEQ_START_DRAW_OTHERCARD;
    }
    break;
  case SEQ_START_DRAW_OTHERCARD:
    UTIL_MATCHINFO_Create( p_wk, p_wk->param.p_enemy_data );
    *p_seq  = SEQ_WAIT_DRAW_OTHERCARD;
		break;
  case SEQ_WAIT_DRAW_OTHERCARD:
    if( UTIL_MATCHINFO_Main( p_wk ) )
    { 
      *p_seq  = SEQ_WAIT_SYNC;
    }
		break;
  case SEQ_WAIT_SYNC:
    if( p_wk->cnt++ > ENEMYDATA_WAIT_SYNC )
    {  
      *p_seq  = SEQ_START_MSG_WAIT;
    }
		break;

  case SEQ_START_MSG_WAIT:
    GFL_BG_SetVisible( BG_FRAME_M_TEXT, TRUE );
    UTIL_TEXT_Print( p_wk, LIVE_STR_20 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_TIMING );
		break;
    
  case SEQ_START_TIMING:
    GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(),71, WB_NET_IRC_BATTLE );
    *p_seq  = SEQ_WAIT_TIMING;
    break;

  case SEQ_WAIT_TIMING:
    if( GFL_NET_HANDLE_IsTimeSync( GFL_NET_HANDLE_GetCurrentHandle(),71, WB_NET_IRC_BATTLE) )
    { 
      *p_seq  = SEQ_CHECK_SHOW;
    }
    break;

  case SEQ_CHECK_SHOW:        //見せ合いはあるか
    { 
      REGULATION      *p_reg  = RegulationData_GetRegulation( p_wk->p_regulation );
      if( Regulation_GetParam( p_reg, REGULATION_SHOW_POKE) )
      { 
        //あり
        *p_seq  = SEQ_LIST_END;
      }
      else
      { 
        //なし
        *p_seq  = SEQ_LIST_END;
      }
    }
    break;

  case SEQ_LIST_END:         //リストへ
    UTIL_FLOW_End( p_wk, LIVEBATTLEMATCH_FLOW_RET_BATTLE );
		break;

  case SEQ_WAIT_MSG:         //メッセージ待ち 
    if( UTIL_TEXT_IsEnd( p_wk ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
		break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	戦闘後シーケンス
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_BtlAfter( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_DISCONNECT,
    SEQ_WAIT_DISCONNECT,
    SEQ_START_MSG_SAVE,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,
    SEQ_REC_END,  //録画へ

    SEQ_WAIT_MSG,         //メッセージ待ち 
  };

  LIVEBATTLEMATCH_FLOW_WORK *p_wk = p_wk_adrs;

  //エラー処理
  if( LIVEBATTLEMATCH_IRC_ERROR_REPAIR_DISCONNECT == LIVEBATTLEMATCH_IRC_CheckErrorRepairType( p_wk->p_irc ) )
  { 
    p_wk->param.p_btl_score->is_error = TRUE;
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_StartCup );
  }

  switch( *p_seq )
  {
  case SEQ_START_DISCONNECT:
    if( LIVEBATTLEMATCH_IRC_StartDisConnect(p_wk->p_irc))
    { 
      *p_seq  = SEQ_WAIT_DISCONNECT;
    }
    break;
  case SEQ_WAIT_DISCONNECT:
    if( LIVEBATTLEMATCH_IRC_WaitDisConnect(p_wk->p_irc))
    { 
      *p_seq  = SEQ_START_MSG_SAVE;
    }
    break;
  case SEQ_START_MSG_SAVE:
    UTIL_TEXT_Print( p_wk, LIVE_STR_09 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SAVE );
		break;
  case SEQ_START_SAVE:

    //対戦相手の情報セーブ
    { 
      u8 idx  = LIVEMATCH_DATA_GetMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_BTLCNT );
      LIVEMATCH_DATA_SetFoeMacAddr( p_wk->p_livematch, idx, p_wk->param.p_enemy_data->mac_address );
      LIVEMATCH_DATA_SetFoeParam( p_wk->p_livematch, idx, LIVEMATCH_FOEDATA_PARAM_REST_POKE, p_wk->param.p_btl_score->enemy_rest_poke );
      LIVEMATCH_DATA_SetFoeParam( p_wk->p_livematch, idx, LIVEMATCH_FOEDATA_PARAM_REST_HP, p_wk->param.p_btl_score->enemy_rest_hp );

      OS_TPrintf( "▼対戦相手のデータ\n" );
      OS_TPrintf( "mac_addr [%s]\n", p_wk->param.p_enemy_data->mac_address );
      OS_TPrintf( "rest_poke[%d]\n", p_wk->param.p_btl_score->enemy_rest_poke );
      OS_TPrintf( "rest_hp  [%d]\n", p_wk->param.p_btl_score->enemy_rest_hp );
    }

    //自分のスコアセーブ
    { 
      u8  win  = 0;
      u8  lose  = 0;
      switch( p_wk->param.p_btl_score->result )
      { 
      case BTL_RESULT_RUN_ENEMY:
        /* fallthrough */
      case BTL_RESULT_WIN:
        win = 1;
        break;
      case BTL_RESULT_RUN:
        /* fallthrough */
      case BTL_RESULT_LOSE:
        lose  = 1;
        break;
      case BTL_RESULT_DRAW:
        break;
      default:
        GF_ASSERT_MSG(0, "バトル結果不正値 %d\n", p_wk->param.p_btl_score->result );
      }

      LIVEMATCH_DATA_AddMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_WIN,    win );
      LIVEMATCH_DATA_AddMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_LOSE,   lose );
      LIVEMATCH_DATA_AddMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_BTLCNT, 1 );
    }

    GAMEDATA_SaveAsyncStart(p_wk->param.p_gamedata);
    *p_seq  = SEQ_WAIT_SAVE;
		break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_wk->param.p_gamedata);
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        *p_seq  = SEQ_REC_END;
        break;
      }
    } 
		break;
  case SEQ_REC_END:  //録画へ
    UTIL_FLOW_End( p_wk, LIVEBATTLEMATCH_FLOW_RET_REC );
		break;

  case SEQ_WAIT_MSG:         //メッセージ待ち 
    if( UTIL_TEXT_IsEnd( p_wk ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
		break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	録画後シーケンス
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_RecAfter( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_CHECK_LAST,   //最後のふりかえりかをチェック
    SEQ_CHECK_BTLCNT, //規定回数戦った
    SEQ_START_MSG_SAVE,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,
    SEQ_START_MSG_UNLOCK,
    SEQ_START_MSG_LOOKBACK,
    SEQ_START_LIST_LOOKBACK,
    SEQ_WAIT_LIST_LOOKBACK,
    SEQ_START_MSG_END,

    SEQ_LOOKBACK_END, //振り返る
    SEQ_END,      //対戦終了
    SEQ_RETURN,   //まだ戦える

    SEQ_WAIT_MSG,         //メッセージ待ち 
  };
  LIVEBATTLEMATCH_FLOW_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_CHECK_LAST:  //最後の振り返りかをチェック
    if( Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_STATUS) == DREAM_WORLD_MATCHUP_END )
    { 
      *p_seq  = SEQ_START_MSG_LOOKBACK;
    }
    else
    { 
      *p_seq  = SEQ_CHECK_BTLCNT;
    }
    break;

  case SEQ_CHECK_BTLCNT: //規定回数戦った
    {
      REGULATION  *p_reg  = RegulationData_GetRegulation( p_wk->p_regulation );
      u32 now = LIVEMATCH_DATA_GetMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_BTLCNT );
      u32 max = Regulation_GetParam( p_reg, REGULATION_BTLCOUNT);
      if( now >= max )
      { 
        *p_seq  = SEQ_START_MSG_SAVE;
      }
      else
      { 
        *p_seq  = SEQ_RETURN;
      }
    }
		break;
  case SEQ_START_MSG_SAVE:
    UTIL_TEXT_Print( p_wk, LIVE_STR_09 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_SAVE );
    break;
  case SEQ_START_SAVE:
    //開催状態を終了にしてからセーブ
    Regulation_SetCardParam( p_wk->p_regulation, REGULATION_CARD_STATUS, DREAM_WORLD_MATCHUP_END );

    //ロック解除してからセーブ
    { 
      SAVE_CONTROL_WORK   *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
      BATTLE_BOX_SAVE     *p_btlbox_sv  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv_ctrl ); 
      BATTLE_BOX_SAVE_OffLockFlg( p_btlbox_sv, BATTLE_BOX_LOCK_BIT_LIVE );
    }
    GAMEDATA_SaveAsyncStart(p_wk->param.p_gamedata);
    *p_seq  = SEQ_WAIT_SAVE;
		break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT result;
      result = GAMEDATA_SaveAsyncMain(p_wk->param.p_gamedata);
      switch(result){
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
      case SAVE_RESULT_NG:
        *p_seq  = SEQ_START_MSG_UNLOCK;
        break;
      }
    } 
		break;

  case SEQ_START_MSG_UNLOCK:
    UTIL_PLAYERINFO_RenewalData( p_wk, PLAYERINFO_WIFI_UPDATE_TYPE_UNLOCK );
    UTIL_TEXT_Print( p_wk, LIVE_STR_21 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_MSG_LOOKBACK );
    break;
    
  case SEQ_START_MSG_LOOKBACK:
    UTIL_TEXT_Print( p_wk, LIVE_STR_23 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_LOOKBACK );
    break;
  case SEQ_START_LIST_LOOKBACK:
    UTIL_LIST_Create( p_wk, LVM_MENU_TYPE_END );
    *p_seq  = SEQ_WAIT_LIST_LOOKBACK;
    break;

  case SEQ_WAIT_LIST_LOOKBACK:
    { 
      const u32 select  = UTIL_LIST_Main( p_wk ); 
      if( select != WBM_LIST_SELECT_NULL )
      { 
        UTIL_LIST_Delete( p_wk );
        if( p_wk->is_rec  && !p_wk->param.p_btl_score->is_error )
        { 
          switch( select )
          { 
          case 0: //ふりかえる
            *p_seq  = SEQ_LOOKBACK_END;
            break;

          case 1: //おわる
            *p_seq  = SEQ_START_MSG_END;
            break;
          }
        }
        else
        { 
          switch( select )
          { 
          case 0: //おわる
            *p_seq  = SEQ_START_MSG_END;
            break;
          }
        }
      }
    }
    break;

  case SEQ_START_MSG_END:
    UTIL_TEXT_Print( p_wk, LIVE_STR_24 );
    *p_seq       = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_END );
    break;
  case SEQ_LOOKBACK_END:      //振り返る
    UTIL_FLOW_End( p_wk, LIVEBATTLEMATCH_FLOW_RET_BTLREC );
		break;
  case SEQ_END:      //対戦終了
    UTIL_FLOW_End( p_wk, LIVEBATTLEMATCH_FLOW_RET_LIVEMENU );
		break;
  case SEQ_RETURN:   //まだ戦える
    WBM_SEQ_SetNext( p_seqwk, SEQFUNC_StartCup );
		break;

  case SEQ_WAIT_MSG:         //メッセージ待ち 
    if( UTIL_TEXT_IsEnd( p_wk ) )
    { 
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
		break;
  }
}


//=============================================================================
/**
 *  便利関数
 */
//=============================================================================
//-------------------------------------
///	リスト
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  リストを表示
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *	@param	type                                表示リストタイプ
 */
//-----------------------------------------------------------------------------
static void UTIL_LIST_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk, LVM_MENU_TYPE type )
{ 
  enum
  { 
    POS_CENTER,
    POS_RIGHT_DOWN,
  } pos;

  u8 x,y,w,h;

  WBM_LIST_SETUP  setup;
  GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
  setup.p_msg   = p_wk->p_msg;
  setup.p_font  = p_wk->param.p_font;
  setup.p_que   = p_wk->param.p_que;


  switch( type )
  { 
  case LVM_MENU_TYPE_YESNO:
    setup.strID[0]= LIVE_SELECT_00;
    setup.strID[1]= LIVE_SELECT_01;
    setup.list_max= 2;
    pos = POS_RIGHT_DOWN;
    break;

  case LVM_MENU_TYPE_MENU:
    if( p_wk->is_rec  && !p_wk->param.p_btl_score->is_error )
    { 
      setup.strID[0]= LIVE_SELECT_02;
      setup.strID[1]= LIVE_SELECT_03;
      setup.strID[2]= LIVE_SELECT_04;
      setup.list_max= 3;
    }
    else
    { 
      setup.strID[0]= LIVE_SELECT_02;
      setup.strID[1]= LIVE_SELECT_04;
      setup.list_max= 2;
    }
    pos = POS_CENTER;
    break;
  case LVM_MENU_TYPE_END:
    if( p_wk->is_rec  && !p_wk->param.p_btl_score->is_error )
    { 
      setup.strID[0]= LIVE_SELECT_03;
      setup.strID[1]= LIVE_SELECT_05;
      setup.list_max= 2;
    }
    else
    { 
      setup.strID[0]= LIVE_SELECT_05;
      setup.list_max= 1;
    }
    pos = POS_CENTER;
    break;
  }
  setup.frm     = BG_FRAME_M_TEXT;
  setup.font_plt= PLT_FONT_M;
  setup.frm_plt = PLT_LIST_M;
  setup.frm_chr = CGR_OFS_M_LIST;


  switch( pos )
  { 
  case POS_CENTER:
    w  = 28;
    h  = setup.list_max * 2;
    x  = 32 / 2  - w /2; 
    y  = (24-6) /2  - h /2;
    break;
  case POS_RIGHT_DOWN:
    w  = 12;
    h  = setup.list_max * 2;
    x  = 32 - w - 1; //1はフレーム分
    y  = 24 - h - 1 - 6; //１は自分のフレーム分と6はテキスト分
    break;
  }

  p_wk->p_list  = WBM_LIST_InitEx( &setup, x, y, w, h, p_wk->heapID );
  p_wk->list_type = type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  リストを削除
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_LIST_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  WBM_LIST_Exit( p_wk->p_list );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト処理
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *
 *	@retval 選択肢インデックス
 */
//-----------------------------------------------------------------------------
static u32 UTIL_LIST_Main( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  u32 ret = WBM_LIST_Main( p_wk->p_list );

  if( ret == WBM_LIST_SELECT_CALNCEL )
  { 
    if( p_wk->list_type == LVM_MENU_TYPE_YESNO )
    { 
      ret = 1;  //NOにする
    }
    else
    { 
      //キャンセルきかない
      ret = WBM_LIST_SELECT_NULL;
    }
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  テキスト文字表示
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *	@param	strID                               文字
 */
//-----------------------------------------------------------------------------
static void UTIL_TEXT_Print( LIVEBATTLEMATCH_FLOW_WORK *p_wk, u32 strID )
{ 
  WBM_TEXT_Print( p_wk->param.p_text, p_wk->p_msg, strID, WBM_TEXT_TYPE_STREAM );
}
//----------------------------------------------------------------------------
/**
 *	@brief  テキスト表示待ち
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *
 *	@return TRUE文字描画完了  FALSE文字描画中
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_TEXT_IsEnd( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  return WBM_TEXT_IsEnd( p_wk->param.p_text );
}
//----------------------------------------------------------------------------
/**
 *	@brief  フロー終了
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *	@param	ret                                 戻り値
 */
//-----------------------------------------------------------------------------
static void UTIL_FLOW_End( LIVEBATTLEMATCH_FLOW_WORK *p_wk, LIVEBATTLEMATCH_FLOW_RET ret )
{ 
  p_wk->retcode = ret;
  WBM_SEQ_SetNext( p_wk->p_seq, SEQFUNC_End );
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のカード作成
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk   ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_PLAYERINFO_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  if( p_wk->p_playerinfo == NULL )
  {
    MYSTATUS    *p_my;
    GFL_CLUNIT	*p_unit;

    //自分のデータを表示
    PLAYERINFO_LIVECUP_DATA info_setup;
    SAVE_CONTROL_WORK   *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
    BATTLE_BOX_SAVE     *p_btlbox_sv  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv_ctrl );
    REGULATION  *p_reg  = RegulationData_GetRegulation(p_wk->p_regulation);
    REGULATION_SAVEDATA *p_reg_sv   = SaveData_GetRegulationSaveData( p_sv_ctrl );
    REGULATION_VIEWDATA *p_reg_view = RegulationSaveData_GetRegulationView( p_reg_sv, REGULATION_CARD_TYPE_LIVE ); 

    p_my  = GAMEDATA_GetMyStatus( p_wk->param.p_gamedata); 
    p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->param.p_graphic );

    GFL_STD_MemClear( &info_setup, sizeof(PLAYERINFO_LIVECUP_DATA) );

    //Regulation_PrintDebug( p_wk->p_regulation );

    info_setup.win_cnt  = LIVEMATCH_DATA_GetMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_WIN );
    info_setup.lose_cnt = LIVEMATCH_DATA_GetMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_LOSE );
    info_setup.btl_cnt  = LIVEMATCH_DATA_GetMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_BTLCNT );
    info_setup.btl_max  = Regulation_GetParam( p_reg, REGULATION_BTLCOUNT );
    info_setup.trainerID  = MyStatus_GetTrainerView( p_my );

    GFL_STD_MemCopy( Regulation_GetCardCupNamePointer( p_wk->p_regulation ),
        info_setup.cup_name, 2*(WIFI_PLAYER_TIX_CUPNAME_MOJINUM + EOM_SIZE) );

    info_setup.start_date = GFDATE_Set( 
        Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_START_YEAR ),
        Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_START_MONTH ), 
        Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_START_DAY ),
          0);

    info_setup.end_date = GFDATE_Set( 
        Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_END_YEAR ),
        Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_END_MONTH ), 
        Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_END_DAY ),
          0);

    p_wk->p_playerinfo	= PLAYERINFO_LIVE_Init( &info_setup, p_my, p_unit, p_wk->param.p_view, p_wk->param.p_font, p_wk->param.p_que, p_wk->p_wifi_msg, p_wk->p_word, p_reg_view, p_wk->heapID );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のカード破棄
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_PLAYERINFO_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
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
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk   ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_PLAYERINFO_Move( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{
  return PLAYERINFO_MoveMain( p_wk->p_playerinfo );
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分のカードのデータ更新
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *	@param	type                            更新タイプ
 */
//-----------------------------------------------------------------------------
static void UTIL_PLAYERINFO_RenewalData( LIVEBATTLEMATCH_FLOW_WORK *p_wk, PLAYERINFO_WIFI_UPDATE_TYPE type )
{ 
  PLAYERINFO_LIVE_RenewalData( p_wk->p_playerinfo, type, p_wk->p_wifi_msg, p_wk->param.p_que, p_wk->param.p_font, p_wk->heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  対戦者のカードを作成
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk     ワーク
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *cp_enemy_data  対戦者情報
 */
//-----------------------------------------------------------------------------
static void UTIL_MATCHINFO_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk, const WIFIBATTLEMATCH_ENEMYDATA *cp_enemy_data )
{
  if( p_wk->p_matchinfo == NULL )
  { 
    GFL_CLUNIT  *p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->param.p_graphic );
    p_wk->p_matchinfo		= MATCHINFO_Init( cp_enemy_data, p_unit, p_wk->param.p_view, p_wk->param.p_font, p_wk->param.p_que, p_wk->p_wifi_msg, p_wk->p_word, WIFIBATTLEMATCH_TYPE_LIVECUP, FALSE, p_wk->heapID );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  対戦者のカードを破棄
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_MATCHINFO_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
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
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk   ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_MATCHINFO_Main( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{
  return MATCHINFO_MoveMain( p_wk->p_matchinfo );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックス  作成
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_BTLBOX_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{
  if( p_wk->p_btlbox == NULL )
  { 
    GFL_CLUNIT  *p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->param.p_graphic );
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
    BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );

#ifdef PM_DEBUG
    { 
      int i;
      POKEPARTY *p_party  = BATTLE_BOX_SAVE_MakePokeParty( p_bbox_save, p_wk->heapID );
      for( i = 0; i < PokeParty_GetPokeCount(p_party); i++ )
      { 
        POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer( p_party, i );
        if( PP_Get( p_pp, ID_PARA_poke_exist, NULL ) )
        { 
          NAGI_Printf( "monsno[%d]\n",  PP_Get( p_pp, ID_PARA_monsno, NULL ) );
          NAGI_Printf( "formno[%d]\n",  PP_Get( p_pp, ID_PARA_form_no, NULL ) );
          NAGI_Printf( "item[%d]\n",  PP_Get( p_pp, ID_PARA_item, NULL ) );
        }
      }
      GFL_HEAP_FreeMemory( p_party );
    }
#endif //PM_DEBUG

    p_wk->p_btlbox  = WBM_BTLBOX_Init( p_bbox_save, p_unit, p_wk->param.p_view, p_wk->p_small_font, p_wk->param.p_que, p_wk->p_wifi_msg, p_wk->heapID );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックス  破棄
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_BTLBOX_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{
  if( p_wk->p_btlbox )
  { 
    WBM_BTLBOX_Exit( p_wk->p_btlbox );
    p_wk->p_btlbox  = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックス  スライドイン
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_BTLBOX_MoveIn( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{
  return WBM_BTLBOX_MoveInMain( p_wk->p_btlbox );
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトルボックス  スライドアウト
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_BTLBOX_MoveOut( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{
  return WBM_BTLBOX_MoveOutMain( p_wk->p_btlbox );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケパーティテンポラリへバトルボックスのパーティをセット
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *	@param  POKEPARTY                       受け取り
 */
//-----------------------------------------------------------------------------
static void UTIL_DATA_GetBtlBoxParty( LIVEBATTLEMATCH_FLOW_WORK *p_wk, POKEPARTY *p_dst )
{ 
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->param.p_gamedata );
  BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
  POKEPARTY         *p_party  =  BATTLE_BOX_SAVE_MakePokeParty( p_bbox_save, p_wk->heapID );
  GFL_STD_MemCopy( p_party, p_dst, PokeParty_GetWorkSize() );
  GFL_HEAP_FreeMemory( p_party );
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のデータを格納
 *
 *	@param	WIFIBATTLEMATCH_ENEMYDATA *p_my_data  格納場所
 *	@param	*p_wk                                 ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_DATA_SetupMyData( WIFIBATTLEMATCH_ENEMYDATA *p_my_data, LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  GAMEDATA  *p_gamedata  = p_wk->param.p_gamedata;
  //SERVER
  p_my_data->btl_server_version  = BTL_NET_SERVER_VERSION;
  //MYSTATUS
  {
    MYSTATUS  *p_my;
    p_my  = GAMEDATA_GetMyStatus(p_gamedata);
    GFL_STD_MemCopy( p_my, p_my_data->mystatus, MyStatus_GetWorkSize() );
  }
  //MACアドレス
  { 
    OS_GetMacAddress( p_my_data->mac_address );
  }
  //PMS
  { 
    const MYPMS_DATA *cp_mypms;
    SAVE_CONTROL_WORK *p_sv;
    p_sv            = GAMEDATA_GetSaveControlWork(p_gamedata);
    cp_mypms        = SaveData_GetMyPmsDataConst( p_sv );
    MYPMS_GetPms( cp_mypms, MYPMS_PMS_TYPE_INTRODUCTION, &p_my_data->pms );
  }
  //パーティ
  {
    SAVE_CONTROL_WORK*	p_sv	= GAMEDATA_GetSaveControlWork(p_gamedata);
    BATTLE_BOX_SAVE *p_btlbox_sv = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
    POKEPARTY *p_temoti  = BATTLE_BOX_SAVE_MakePokeParty( p_btlbox_sv, GFL_HEAP_LOWID(HEAPID_WIFIBATTLEMATCH_SYS) );
    GFL_STD_MemCopy( p_temoti, p_my_data->pokeparty, PokeParty_GetWorkSize() );
    GFL_HEAP_FreeMemory( p_temoti );
  }
  //スコア
  { 
    p_my_data->win_cnt  = LIVEMATCH_DATA_GetMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_WIN );
    p_my_data->lose_cnt = LIVEMATCH_DATA_GetMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_LOSE );
    p_my_data->btl_cnt  = LIVEMATCH_DATA_GetMyParam( p_wk->p_livematch, LIVEMATCH_MYDATA_PARAM_BTLCNT );
  }
  //大会番号
  { 
    p_my_data->wificup_no = Regulation_GetCardParam( p_wk->p_regulation, REGULATION_CARD_STATUS);
  }
}
