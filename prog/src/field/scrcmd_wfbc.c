//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_wfbc.c
 *	@brief  WFBC用スクリプトコマンド郡
 *	@author	tomoya takahashi
 *	@date		2009.12.17
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "arc/arc_def.h"
#include "arc/script_message.naix"

#include "print/wordset.h"

#include "field/field_msgbg.h"

#include "field_comm/intrude_work.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "event_field_fade.h"

#include "fieldmap.h"
#include "field_wfbc.h"
#include "fldmmdl.h"
#include "field_status_local.h"

#include "scrcmd_wfbc.h"

#include "scrcmd_wfbc_define.h" 

#include "msg/msg_place_name.h"  // for MAPNAME_xxxx
#include "msg/script/msg_plc10.h"  
#include "msg/script/msg_bc10.h"  
#include "msg/script/msg_wc10.h"  

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// Palace通信会話イベント
static GMEVENT* EVENT_SetUp_WFBC_Palece_Talk( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
static GMEVENT_RESULT EVENT_WFBC_Palece_Talk( GMEVENT* p_event, int* p_seq, void* p_wk );



//----------------------------------------------------------------------------
/**
 *	@brief  WFBC　会話開始処理
 *	@return
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_TalkStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
  FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
  s16 gx, gy, gz;
  MMDL* p_frontmmdl;
  u32 objid;

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );


  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  objid = FIELD_WFBC_CORE_GetMMdlNpcID( p_frontmmdl );

  // 会話カウント　（1日1回）
  FIELD_WFBC_AddTalkPointPeople( p_wfbc, objid );
  
	return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  会話終了設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_TalkEnd( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
  FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
  GAMEDATA* p_gamedata = SCRCMD_WORK_GetGameData( work );
  FIELD_WFBC_EVENT* p_event = GAMEDATA_GetWFBCEventData( p_gamedata );
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
  s16 gx, gy, gz;
  MMDL* p_frontmmdl;
  u32 objid;

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );


  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  // 会話スコア加算
  objid = FIELD_WFBC_CORE_GetMMdlNpcID( p_frontmmdl );


  // Palaceなら連れて行く処理

  // 会話したので、色々なマスクをOFF
  if( FIELD_WFBC_IsBattlePeople( p_wfbc, objid ) )
  {
    // バトルもうできない
    FIELD_WFBC_SetBattlePeople( p_wfbc, objid );
    
    // 勝った数加算
    FIELD_WFBC_EVENT_AddBCNpcWinCount( p_event );
  }

	return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  前の街（BC）の履歴にある人の名前をワードセットに設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_SetRirekiPlayerName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
  FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
  WORDSET* pp_wordset = SCRIPT_GetWordSet( sc );
  s16 gx, gy, gz;
  MMDL* p_frontmmdl;
  u32 objid;
  const FIELD_WFBC_CORE_PEOPLE* cp_people_core;
  const FIELD_WFBC_PEOPLE* cp_people;
  u16 idx = SCRCMD_GetVMWorkValue( core, wk );  // 
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );


  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  // OBJID
  objid = FIELD_WFBC_CORE_GetMMdlNpcID( p_frontmmdl );

  // ワードセットに設定
  FIELD_WFBC_SetWordSetParentPlayer( p_wfbc, pp_wordset, objid, idx, heapID );

	return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  WFBC  各情報の取得
 *
 *  FIELD_INITの状態で呼ばれる可能性があるので、注意！！！
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_GetData( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA* p_gamedata = SCRCMD_WORK_GetGameData( work );
  FIELD_WFBC_CORE* p_wfbc_core = GAMEDATA_GetMyWFBCCoreData( p_gamedata );

  u16 define = SCRCMD_GetVMWorkValue( core, wk );  // 
  u16 *ret_wk	= SCRCMD_GetVMWork( core, work );
  
  // FIELD_INITが存在していればOKな処理
  if( define == WFBC_GET_PARAM_PEOPLE_NUM )
  {
    (*ret_wk) = FIELD_WFBC_CORE_GetPeopleNum( p_wfbc_core, MAPMODE_NORMAL );
  }
  else
  // FIELDMAP構築後である必要がある処理
  {
    SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
    FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
    FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
    FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
    const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
    FIELD_WFBC_EVENT* p_event = GAMEDATA_GetWFBCEventData( p_gamedata );
    s16 gx, gy, gz;
    MMDL* p_frontmmdl;
    u32 objid;
    u32 mmdlid;
    const FIELD_WFBC_CORE_PEOPLE* cp_people_core;
    const FIELD_WFBC_PEOPLE_DATA* cp_people_data;
    const FIELD_WFBC_PEOPLE* cp_people;

    // 目の前のグリッド取得
    FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );

    // 目の前にいる人物を検索
    p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

    // OBJID
    if( p_frontmmdl )
    {
      objid = FIELD_WFBC_CORE_GetMMdlNpcID( p_frontmmdl );
      mmdlid = MMDL_GetOBJID( p_frontmmdl );

      // 人物情報取得
      cp_people = FIELD_WFBC_GetPeople( p_wfbc, objid );
      if( cp_people )
      {
        cp_people_data = FIELD_WFBC_PEOPLE_GetPeopleData( cp_people );
        cp_people_core = FIELD_WFBC_PEOPLE_GetPeopleCore( cp_people );
      }
    }


    GF_ASSERT(define < WFBC_GET_PARAM_MAX);

    switch( define )
    {
    // バトルトレーナーID
    case WFBC_GET_PARAM_BATTLE_TRAINER_ID:
      (*ret_wk) = cp_people_data->btl_trdata;
      break;
    // 街にいきたがるか
    case WFBC_GET_PARAM_IS_TAKES_ID:
      (*ret_wk) = FIELD_WFBC_CORE_PEOPLE_IsMoodTakes( cp_people_core );
      break;
    // 前にいたBCの街の履歴があるか
    case WFBC_GET_PARAM_IS_RIREKI:
      (*ret_wk) = FIELD_WFBC_CORE_PEOPLE_IsParentIn( cp_people_core );
      break;
    // BC　NPC勝利数
    case WFBC_GET_PARAM_BC_NPC_WIN_NUM:
      (*ret_wk) = FIELD_WFBC_EVENT_GetBCNpcWinCount( p_event );
      break;
    // BC　NPC勝利目標数
    case WFBC_GET_PARAM_BC_NPC_WIN_TARGET:
      (*ret_wk) = FIELD_WFBC_EVENT_GetBCNpcWinTarget( p_event );
      break;
    // WF　村長イベント　ポケモンはいるか？
    case WFBC_GET_PARAM_WF_IS_POKECATCH:
      if( FIELD_WFBC_GetPeopleNum( p_wfbc ) == 0 ){
        (*ret_wk) = FALSE;
      }else{
        (*ret_wk) = TRUE;
      }
      break;
    // WF 村長イベント　ご褒美アイテムの取得
    case WFBC_GET_PARAM_WF_ITEM:
      (*ret_wk) = FIELD_WFBC_EVENT_GetWFPokeCatchEventItem( p_event );
      break;
    // WF 村長イベント　探すポケモンナンバーの取得
    case WFBC_GET_PARAM_WF_POKE:
      (*ret_wk) = FIELD_WFBC_EVENT_GetWFPokeCatchEventMonsNo( p_event );
      break;
    // 話し相手のOBJID（0～29）
    case WFBC_GET_PARAM_OBJ_ID:
      (*ret_wk) = objid;
      break;
    // BC　相手とバトル可能かチェック
    case WFBC_GET_PARAM_BC_CHECK_BATTLE:
      (*ret_wk) = FIELD_WFBC_IsBattlePeople( p_wfbc, objid );
      break;
    // 話し相手のMMDL OBJID (フィールド上の動作モデルを識別するID)
    case WFBC_GET_PARAM_MMDL_ID:
      (*ret_wk) = mmdlid;
      break;

    default:
      GF_ASSERT(0);
      break;
    }
  }
	return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  BC　NPC勝利イベント　勝利目標数を加算
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_AddBCNpcWinTarget( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMESYS_WORK* p_gamesys = FIELDMAP_GetGameSysWork( fparam->fieldMap );
  GAMEDATA* p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  FIELD_WFBC_EVENT* p_event = GAMEDATA_GetWFBCEventData( p_gamedata );

  FIELD_WFBC_EVENT_AddBCNpcWinTarget( p_event );
	return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WF  ターゲットポケモンを手持ちに持っているかチェック
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_CheckWFTargetPokemon( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA* p_gamedata = SCRCMD_WORK_GetGameData( work );
  FIELD_WFBC_CORE* p_wfbc_core = GAMEDATA_GetMyWFBCCoreData( p_gamedata );
  FIELD_WFBC_EVENT* p_event = GAMEDATA_GetWFBCEventData( p_gamedata );
  POKEPARTY * p_party = GAMEDATA_GetMyPokemon( p_gamedata );
  u16 *ret_if_1 = SCRCMD_GetVMWork( core, wk ); // 第1分岐のフラグ
  u16 *ret_if_2 = SCRCMD_GetVMWork( core, wk ); // 第2分岐のフラグ
  u16 *ret_temoti   = SCRCMD_GetVMWork( core, wk ); // 手持ちインデックス
  u32 max = PokeParty_GetPokeCount( p_party );
  int  i;
  POKEMON_PARAM * pp;
  u32 target_monsno;
  u32 monsno;
  u32 mons_getplace;
  u32 mons_year;
  u32 mons_month;
  u32 mons_day;
  BOOL tamago;
  RTCDate now_date;

  // 目的の情報を取得
  target_monsno = FIELD_WFBC_EVENT_GetWFPokeCatchEventMonsNo( p_event );
  GFL_RTC_GetDate( &now_date );

  //各条件の初期化
  *ret_if_1 = FALSE;
  *ret_if_2 = FALSE;
  *ret_temoti = 0xffff;
  
  // 手持ちの情報とチェック
  for( i=0; i<max; i++ )
  {
    POKEMON_PARAM * pp = PokeParty_GetMemberPointer( p_party, i );

    monsno    = PP_Get( pp, ID_PARA_monsno, NULL );
    mons_year = PP_Get( pp, ID_PARA_get_year, NULL );
    mons_month= PP_Get( pp, ID_PARA_get_month, NULL );
    mons_day  = PP_Get( pp, ID_PARA_get_day, NULL );
    mons_getplace = PP_Get( pp, ID_PARA_get_place, NULL );
    tamago    = PP_Get( pp, ID_PARA_tamago_flag, NULL );

    TOMOYA_Printf( "target monsno %d  == monsno %d\n", target_monsno, monsno );
    TOMOYA_Printf( "tamago %d\n", tamago );
    TOMOYA_Printf( "get_place %d == WF%d\n", mons_getplace, MAPNAME_WC10 );
    TOMOYA_Printf( "year%d month%d day%d == now year%d month%d day%d \n", 
        mons_year, mons_month, mons_day, now_date.year, now_date.month, now_date.day );
    
    if( (monsno == target_monsno) && (tamago == FALSE) && (mons_getplace == MAPNAME_WC10) )
    {
      *ret_if_1 = TRUE;
      if( (*ret_temoti) == 0xffff )
      {  // 最初のヒットのみ
        (*ret_temoti) = i;
      }

      
      // 日時も一致？
      if( (mons_year == now_date.year) && (mons_month == now_date.month) && (mons_day == now_date.day) )
      {
        *ret_if_2 = TRUE;
        (*ret_temoti) = i;
      }
    }
  }

	return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  自動配置オブジェクト スクリプト用メッセージ取得
 */
//-----------------------------------------------------------------------------
#define WFBC_SCRIPT_OBJ_MSG_NUM (4)
VMCMD_RESULT EvCmdWfbc_GetAutoNpcMessage( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
  FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
  WORDSET* pp_wordset = SCRIPT_GetWordSet( sc );
  s16 gx, gy, gz;
  MMDL* p_frontmmdl;
  u32 objid;
  const FIELD_WFBC_CORE_PEOPLE* cp_people_core;
  const FIELD_WFBC_PEOPLE* cp_people;
  u16 *ret = SCRCMD_GetVMWork( core, wk ); // 戻り値
  u16 msg_idx = SCRCMD_GetVMWorkValue( core, wk ); // メッセージインデックス

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );


  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );
  GF_ASSERT( p_frontmmdl );

  // OBJID
  objid = FIELD_WFBC_CORE_GetMMdlNpcID( p_frontmmdl );

  msg_idx -= 1;
  GF_ASSERT( msg_idx < WFBC_SCRIPT_OBJ_MSG_NUM );

  // メッセージIDの返す
  if( FIELD_WFBC_GetType(p_wfbc) == FIELD_WFBC_CORE_TYPE_BLACK_CITY )
  {
    *ret = bc10_01_01 + (WFBC_SCRIPT_OBJ_MSG_NUM*objid) + msg_idx;
  }
  else
  {
    *ret = wc10_01_01 + (WFBC_SCRIPT_OBJ_MSG_NUM*objid) + msg_idx;
  }


	return VMCMD_RESULT_CONTINUE;
}




//----------------------------------------------------------------------------
/**
 *	@brief  進入用会話処理起動
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_StartPalaceTalk( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  // イベント起動
  SCRIPT_CallEvent( scw, EVENT_SetUp_WFBC_Palece_Talk(gsys, fieldWork) );

	return VMCMD_RESULT_SUSPEND;
}


//-----------------------------------------------------------------------------
/**
 *    private関数
 */
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
/**
 *    Palace時の会話
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	イベントシーケンス
//=====================================
enum {
  WFBC_PALACE_TALK_SEQ_IS_OBJID_ON,   // 親の世界に目の前の人がいるのかチェック
  WFBC_PALACE_TALK_SEQ_WAIL_OBJID_ON, // 親の世界に目の前の人がいるのかチェック待ち
  WFBC_PALACE_TALK_SEQ_TALK_01,       // おれ、別VERSIONの街にいくんだいいでしょう。
  WFBC_PALACE_TALK_SEQ_IS_MYROMVER,   // 自分ROMバージョンをチェック
  WFBC_PALACE_TALK_SEQ_TALK_02,       // この街はつまらない・・・
  WFBC_PALACE_TALK_SEQ_IS_MYWFBC_CAN_TAKE, // 自分の世界に持ってこれるかチェック
  WFBC_PALACE_TALK_SEQ_TALK_03,       // たのしそうだな
  WFBC_PALACE_TALK_SEQ_TALK_04,       // つれていって！　YES/NO表示
  WFBC_PALACE_TALK_SEQ_TALK_05,       // つれだしてよー。　NOの場合
  WFBC_PALACE_TALK_SEQ_IS_OBJID_ON01, // もう一度親の世界にいるかチェック
  WFBC_PALACE_TALK_SEQ_WAIT_OBJID_ON01,// もう一度親の世界にいるかチェック
  WFBC_PALACE_TALK_SEQ_TALK_06,       // やっぱり別の街にいく！
  WFBC_PALACE_TALK_SEQ_TALK_08,       // やったー！（人移動＆移動したことを親に通知）
  WFBC_PALACE_TALK_SEQ_MAP_RESET_FADE_OUT,        // マップの再構築 フェードアウト
  WFBC_PALACE_TALK_SEQ_MAP_RESET_PEOPLE_CHANGE,   // マップの再構築 フェードアウトウエイト　＆　つれてきた人を消す処理
  WFBC_PALACE_TALK_SEQ_MAP_RESET_FADE_INWAIT,        // マップの再構築 フェードアウト


  WFBC_PALACE_TALK_SEQ_TALK_WAIT,   //メッセージ完了まち　

  WFBC_PALACE_TALK_SEQ_END,
} ;

//-------------------------------------
///	会話処理内　シーケンス
//=====================================
enum {
  WFBC_PALACE_TALKSYS_SEQ_PRINT,
  WFBC_PALACE_TALKSYS_SEQ_YESNO_START,
  WFBC_PALACE_TALKSYS_SEQ_YESNO_WAIT,
  WFBC_PALACE_TALKSYS_SEQ_CLOSE,
  WFBC_PALACE_TALKSYS_SEQ_CLOSE_WAIT,
} ;
enum{
  WFBC_PALACE_TALKMODE_NORMAL,
  WFBC_PALACE_TALKMODE_YESNO,

  WFBC_PALACE_TALKMODE_MAX,
};


//-------------------------------------
///	Palace時の会話ワーク
//=====================================
typedef struct {
  GAMESYS_WORK* p_gsys;
  FIELDMAP_WORK* p_fieldmap;

  HEAPID heapID;
  
  FLDMSGBG*         p_fmb;
  FLDTALKMSGWIN *   p_talkwin;
  FLDMENUFUNC *     p_yesno;
  GFL_MSGDATA*      p_msgdata;
  WORDSET*          p_wordset;
  STRBUF*           p_strbuf;
  STRBUF*           p_strbuf_tmp;

  FIELD_WFBC* p_wfbc;
  const FIELD_WFBC_PEOPLE* cp_people;
  MMDL*       p_mmdl;
  VecFx32     pos;

  WFBC_COMM_DATA* p_commsys;

  FIELD_WFBC_CORE* p_my;
  FIELD_WFBC_CORE* p_oya;

  u16 npc_id;
  u16 yes_next_seq;
  u16 no_next_seq;

  u16 talk_seq;
  u8  is_yes;
  u8  mode;
  u8  pad[2];
} WFBC_PALACE_TALK_WK;


//-------------------------------------
///	会話メッセージ情報
//=====================================
static const u16 sc_WFBC_PALACE_TALK_START_IDX[FIELD_WFBC_CORE_TYPE_MAX] = 
{
  plbc10_01_01,
  plwc10_01_01,
};
#define WFBC_PALACE_TALK_NPC_MSG_NUM  (8)

// private
static void EVENT_WFBC_WFBC_TalkStart( WFBC_PALACE_TALK_WK* p_wk, u16 palace_msg_idx, int* p_seq, u16 next_seq );
static void EVENT_WFBC_WFBC_TalkStartYesNo( WFBC_PALACE_TALK_WK* p_wk, u16 palace_msg_idx, int* p_seq, u16 yes_next_seq, u16 no_next_seq );
static BOOL EVENT_WFBC_WFBC_TalkMain( WFBC_PALACE_TALK_WK* p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  WFBC　Palace時　の会話
 *
 *	@param	p_gsys      システム
 *	@param	p_fieldmap  フィールドマップ
 */
//-----------------------------------------------------------------------------
static GMEVENT* EVENT_SetUp_WFBC_Palece_Talk( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap )
{
  GMEVENT*              p_event;
  WFBC_PALACE_TALK_WK*   p_work;
  HEAPID heapID = FIELDMAP_GetHeapID( p_fieldmap );
  FLDMSGBG* p_msgbg = FIELDMAP_GetFldMsgBG( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  GAME_COMM_SYS_PTR p_game_comm = GAMESYSTEM_GetGameCommSysPtr(p_gsys);
  INTRUDE_COMM_SYS_PTR p_intcomm = GameCommSys_GetAppWork( p_game_comm );
  WFBC_COMM_DATA * p_wfbc_comm = Intrude_GetWfbcCommData( p_intcomm  );



  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, EVENT_WFBC_Palece_Talk, sizeof( WFBC_PALACE_TALK_WK ) );

  // イベントワークを初期化
  p_work                  = GMEVENT_GetEventWork( p_event );
  p_work->p_gsys          = p_gsys;
  p_work->p_fieldmap      = p_fieldmap;
  p_work->p_strbuf        = GFL_STR_CreateBuffer( 128, heapID );
  p_work->p_strbuf_tmp    = GFL_STR_CreateBuffer( 128, heapID );
  p_work->p_msgdata       = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_plc10_dat, heapID );
  p_work->p_wordset       = WORDSET_Create( heapID );
  p_work->p_fmb           = p_msgbg;
  p_work->p_commsys       = p_wfbc_comm;

  p_work->heapID          = GFL_HEAP_LOWID(heapID);

  // WFBC情報取得
  p_work->p_my   = GAMEDATA_GetWFBCCoreData( p_gdata, GAMEDATA_WFBC_ID_MINE );
  p_work->p_oya  = GAMEDATA_GetWFBCCoreData( p_gdata, GAMEDATA_WFBC_ID_COMM );

  // 目の前の人物をチェック
  {
    FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( p_fieldmap );
    FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
    FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
    const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
    GAMEDATA* p_gamedata = GAMESYSTEM_GetGameData( p_gsys );
    FIELD_WFBC_EVENT* p_event = GAMEDATA_GetWFBCEventData( p_gamedata );
    s16 gx, gy, gz;
    MMDL* p_frontmmdl;
    u32 objid;
    const FIELD_WFBC_PEOPLE* cp_people;

    // 目の前のグリッド取得
    FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );

    // 目の前にいる人物を検索
    p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );
    GF_ASSERT( p_frontmmdl );

    // 座標を取得
    MMDL_GetVectorPos( p_frontmmdl, &p_work->pos );

    // OBJID
    objid = FIELD_WFBC_CORE_GetMMdlNpcID( p_frontmmdl );

    // 人物情報取得
    cp_people = FIELD_WFBC_GetPeople( p_wfbc, objid );
    GF_ASSERT( cp_people );

    p_work->p_wfbc    = p_wfbc;
    p_work->cp_people = cp_people;
    p_work->p_mmdl    = p_frontmmdl;
    p_work->npc_id    = objid;

    // 自分のほうを向かせる
  }
  // 生成したイベントを返す
  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WFBC　Palece　会話イベント
 *
 *	@param	p_event   イベントシステム
 *	@param	p_seq     シーケンス
 *	@param	p_wk      ワーク
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_WFBC_Palece_Talk( GMEVENT* p_event, int* p_seq, void* p_wk )
{
  WFBC_PALACE_TALK_WK*   p_work = p_wk;

  switch( (*p_seq) )
  {
  // 親の世界に目の前の人がいるのかチェック
  case WFBC_PALACE_TALK_SEQ_IS_OBJID_ON:
    FIELD_WFBC_COMM_DATA_ClearReqAnsData( p_work->p_commsys );
    FIELD_WFBC_COMM_DATA_SetReqData( p_work->p_commsys, p_work->npc_id, FIELD_WFBC_COMM_NPC_REQ_THERE );

    (*p_seq) = WFBC_PALACE_TALK_SEQ_WAIL_OBJID_ON;
    break;

  // 親の世界に目の前の人がいるのかチェック待ち
  case WFBC_PALACE_TALK_SEQ_WAIL_OBJID_ON:
    if( FIELD_WFBC_COMM_DATA_WaitAnsData( p_work->p_commsys, p_work->npc_id ) )
    {
      if( FIELD_WFBC_GetAnsData( p_work->p_commsys ) == FIELD_WFBC_COMM_NPC_ANS_ON )
      {
        (*p_seq) = WFBC_PALACE_TALK_SEQ_IS_MYROMVER;
      }else{
        (*p_seq) = WFBC_PALACE_TALK_SEQ_TALK_01;
      }
    }
    break;

  // おれ、別VERSIONの街にいくんだいいでしょう。
  case WFBC_PALACE_TALK_SEQ_TALK_01:
    EVENT_WFBC_WFBC_TalkStart( p_work, 0, p_seq, WFBC_PALACE_TALK_SEQ_END );
    break;

  // 自分ROMバージョンをチェック
  case WFBC_PALACE_TALK_SEQ_IS_MYROMVER:
    if( p_work->p_my->type != p_work->p_oya->type )
    {
      // OK
      (*p_seq) = WFBC_PALACE_TALK_SEQ_IS_MYWFBC_CAN_TAKE;
    }
    else
    {
      // NG
      (*p_seq) = WFBC_PALACE_TALK_SEQ_TALK_02;
    }
    break;

  // この街はつまらない・・・
  case WFBC_PALACE_TALK_SEQ_TALK_02:
    EVENT_WFBC_WFBC_TalkStart( p_work, 1, p_seq, WFBC_PALACE_TALK_SEQ_END );
    break;

  // 自分の世界に持ってこれるかチェック
  case WFBC_PALACE_TALK_SEQ_IS_MYWFBC_CAN_TAKE:
    // 自分の世界に空きがあるか？
    if( FIELD_WFBC_CORE_GetPeopleNum( p_work->p_my, MAPMODE_NORMAL ) >= FIELD_WFBC_PEOPLE_MAX )
    {
      // 空きなし
      (*p_seq) = WFBC_PALACE_TALK_SEQ_TALK_03;
    }
    // その人がいなか？
    else if( FIELD_WFBC_CORE_IsOnNpcIDPeople( p_work->p_my, p_work->npc_id, MAPMODE_MAX ) )
    {
      // 空きなし
      (*p_seq) = WFBC_PALACE_TALK_SEQ_TALK_03;
    }
    else
    {
      // OK
      (*p_seq) = WFBC_PALACE_TALK_SEQ_TALK_04;
    }
    break;

  // たのしそうだな
  case WFBC_PALACE_TALK_SEQ_TALK_03:
    EVENT_WFBC_WFBC_TalkStart( p_work, 2, p_seq, WFBC_PALACE_TALK_SEQ_END );
    break;

  // つれていって！　YES/NO表示
  case WFBC_PALACE_TALK_SEQ_TALK_04:
    EVENT_WFBC_WFBC_TalkStartYesNo( p_work, 3, p_seq, WFBC_PALACE_TALK_SEQ_IS_OBJID_ON01, WFBC_PALACE_TALK_SEQ_TALK_05 );
    break;

  // つれだしてよー。　NOの場合
  case WFBC_PALACE_TALK_SEQ_TALK_05:
    EVENT_WFBC_WFBC_TalkStart( p_work, 4, p_seq, WFBC_PALACE_TALK_SEQ_END );
    break;

  // もう一度親の世界にいるかチェック
  case WFBC_PALACE_TALK_SEQ_IS_OBJID_ON01: 
    FIELD_WFBC_COMM_DATA_ClearReqAnsData( p_work->p_commsys );
    FIELD_WFBC_COMM_DATA_SetReqData( p_work->p_commsys, p_work->npc_id, FIELD_WFBC_COMM_NPC_REQ_WISH_TAKE );

    (*p_seq) = WFBC_PALACE_TALK_SEQ_WAIT_OBJID_ON01;
    break;

  // もう一度親の世界にいるかチェック
  case WFBC_PALACE_TALK_SEQ_WAIT_OBJID_ON01:
    if( FIELD_WFBC_COMM_DATA_WaitAnsData( p_work->p_commsys, p_work->npc_id ) )
    {
      if( FIELD_WFBC_GetAnsData( p_work->p_commsys ) == FIELD_WFBC_COMM_NPC_ANS_TAKE_OK )
      {
        (*p_seq) = WFBC_PALACE_TALK_SEQ_TALK_08;
      }else{
        (*p_seq) = WFBC_PALACE_TALK_SEQ_TALK_06;
      }
    }
    break;

  // やっぱり別の街にいく！
  case WFBC_PALACE_TALK_SEQ_TALK_06:       
    EVENT_WFBC_WFBC_TalkStart( p_work, 5, p_seq, WFBC_PALACE_TALK_SEQ_END );
    break;

  // やったー！（人移動＆移動したことを親に通知）
  case WFBC_PALACE_TALK_SEQ_TALK_08:       
    EVENT_WFBC_WFBC_TalkStart( p_work, 7, p_seq, WFBC_PALACE_TALK_SEQ_MAP_RESET_FADE_OUT );


    // 親に通知
    FIELD_WFBC_COMM_DATA_ClearReqAnsData( p_work->p_commsys );
    FIELD_WFBC_COMM_DATA_SetReqData( p_work->p_commsys, p_work->npc_id, FIELD_WFBC_COMM_NPC_REQ_TAKE );
    break;

  // マップの再構築 フェードアウト
  case WFBC_PALACE_TALK_SEQ_MAP_RESET_FADE_OUT:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, FIELD_FADE_WAIT);
    (*p_seq) = WFBC_PALACE_TALK_SEQ_MAP_RESET_PEOPLE_CHANGE;
    break;

  // マップの再構築 フェードアウトウエイト　＆　つれてきた人を消す処理
  case WFBC_PALACE_TALK_SEQ_MAP_RESET_PEOPLE_CHANGE:
    if( GFL_FADE_CheckFade() ){
      break;
    }

    {
      const MYSTATUS* cp_mystatus = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(p_work->p_gsys) );
      // 人を移し変える。
      FIELD_WFBC_COMM_DATA_Ko_ChangeNpc( p_work->p_commsys, p_work->p_my, p_work->p_oya, cp_mystatus, p_work->npc_id );
    }
    
    // その人を消す
    {
      FIELD_WFBC_DeletePeople( p_work->p_wfbc, p_work->npc_id );
      MMDL_Delete( p_work->p_mmdl );
    }
    
    // フェードイン
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, FIELD_FADE_WAIT);
    (*p_seq) = WFBC_PALACE_TALK_SEQ_MAP_RESET_FADE_INWAIT;
    break;

  case WFBC_PALACE_TALK_SEQ_MAP_RESET_FADE_INWAIT:        // マップの再構築 フェードアウト
    if( GFL_FADE_CheckFade() ){
      break;
    }
    (*p_seq) = WFBC_PALACE_TALK_SEQ_END;
    break;

  //メッセージ完了まち　
  case WFBC_PALACE_TALK_SEQ_TALK_WAIT:   
    if( EVENT_WFBC_WFBC_TalkMain( p_work ) )
    {
      if( p_work->is_yes ){
        *p_seq = p_work->yes_next_seq;
      }else{
        *p_seq = p_work->no_next_seq;
      }
    }
    break;

  // 終了
  case WFBC_PALACE_TALK_SEQ_END:  
    // 後始末
    {
      GFL_STR_DeleteBuffer( p_work->p_strbuf );
      GFL_STR_DeleteBuffer( p_work->p_strbuf_tmp );

      GFL_MSG_Delete( p_work->p_msgdata );
      WORDSET_Delete( p_work->p_wordset );
    }
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
    break;
  }

  return GMEVENT_RES_CONTINUE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  会話開始
 *
 *	@param	p_wk        ワーク
 *	@param	msg_id      メッセージID
 *	@param	p_seq       シーケンス変更ワーク
 *	@param	next_seq    次のシーケンス
 */
//-----------------------------------------------------------------------------
static void EVENT_WFBC_WFBC_TalkStart( WFBC_PALACE_TALK_WK* p_wk, u16 palace_msg_idx, int* p_seq, u16 next_seq )
{
  u16 msg_id;
  
  GF_ASSERT( p_wk->p_talkwin == NULL );

  msg_id = sc_WFBC_PALACE_TALK_START_IDX[ p_wk->p_oya->type ] + (p_wk->npc_id*WFBC_PALACE_TALK_NPC_MSG_NUM) + palace_msg_idx;

  TOMOYA_Printf( "print msg %d\n", msg_id );

  // ワードセットを使用して、文字列展開
  GFL_MSG_GetString( p_wk->p_msgdata, msg_id, p_wk->p_strbuf_tmp );

  WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_strbuf, p_wk->p_strbuf_tmp );

  // 描画開始
  p_wk->p_talkwin = FLDTALKMSGWIN_AddStrBuf( p_wk->p_fmb, FLDTALKMSGWIN_IDX_AUTO, &p_wk->pos, p_wk->p_strbuf, TALKMSGWIN_TYPE_NORMAL );


  (*p_seq) = WFBC_PALACE_TALK_SEQ_TALK_WAIT;
  p_wk->yes_next_seq = next_seq;

  p_wk->talk_seq = 0;
  p_wk->is_yes = TRUE;
  p_wk->mode  = WFBC_PALACE_TALKMODE_NORMAL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  会話開始
 *
 *	@param	p_wk        ワーク
 *	@param	msg_id      メッセージID
 *	@param	p_seq       シーケンス変更ワーク
 *	@param	yes_next_seqYESの場合のシーケンス
 *	@param	no_next_seq NOの場合のシーケンス
 */
//-----------------------------------------------------------------------------
static void EVENT_WFBC_WFBC_TalkStartYesNo( WFBC_PALACE_TALK_WK* p_wk, u16 palace_msg_idx, int* p_seq, u16 yes_next_seq, u16 no_next_seq )
{
  u16 msg_id;
  
  GF_ASSERT( p_wk->p_talkwin == NULL );

  msg_id = sc_WFBC_PALACE_TALK_START_IDX[ p_wk->p_oya->type ] + (p_wk->npc_id*WFBC_PALACE_TALK_NPC_MSG_NUM) + palace_msg_idx;

  // ワードセットを使用して、文字列展開
  GFL_MSG_GetString( p_wk->p_msgdata, msg_id, p_wk->p_strbuf_tmp );

  WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_strbuf, p_wk->p_strbuf_tmp );

  // 描画開始
  p_wk->p_talkwin = FLDTALKMSGWIN_AddStrBuf( p_wk->p_fmb, FLDTALKMSGWIN_IDX_AUTO, &p_wk->pos, p_wk->p_strbuf, TALKMSGWIN_TYPE_NORMAL );


  (*p_seq) = WFBC_PALACE_TALK_SEQ_TALK_WAIT;
  p_wk->yes_next_seq = yes_next_seq;
  p_wk->no_next_seq = no_next_seq;

  p_wk->talk_seq = 0;
  p_wk->is_yes = TRUE;
  p_wk->mode  = WFBC_PALACE_TALKMODE_YESNO;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ完了まち
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL EVENT_WFBC_WFBC_TalkMain( WFBC_PALACE_TALK_WK* p_wk )
{
  switch( p_wk->talk_seq )
  {
  case WFBC_PALACE_TALKSYS_SEQ_PRINT:
    {
      BOOL result;
      result = FLDTALKMSGWIN_Print( p_wk->p_talkwin );
      if( result )
      {
        if( p_wk->mode == WFBC_PALACE_TALKMODE_NORMAL ){
          // クローズへ
          p_wk->talk_seq = WFBC_PALACE_TALKSYS_SEQ_CLOSE;
        }else{
          // YESNOチェックへ
          p_wk->talk_seq = WFBC_PALACE_TALKSYS_SEQ_YESNO_START;
        }
      }
    }
    break;
    
  //YESNOチェック
  case WFBC_PALACE_TALKSYS_SEQ_YESNO_START:
    p_wk->p_yesno = FLDMENUFUNC_AddYesNoMenu( p_wk->p_fmb, FLDMENUFUNC_YESNO_YES );
    p_wk->talk_seq ++;
    break;

  //YESNOチェックウエイト
  case WFBC_PALACE_TALKSYS_SEQ_YESNO_WAIT:
    {
      FLDMENUFUNC_YESNO result = FLDMENUFUNC_ProcYesNoMenu( p_wk->p_yesno );
      
      if( result != FLDMENUFUNC_YESNO_NULL )
      {
        if( result == FLDMENUFUNC_YESNO_YES ){
          p_wk->is_yes = TRUE;
        }else{
          p_wk->is_yes = FALSE;
        }
        p_wk->talk_seq = WFBC_PALACE_TALKSYS_SEQ_CLOSE;
        FLDMENUFUNC_DeleteMenu( p_wk->p_yesno );
        p_wk->p_yesno = NULL;
      }
    }
    break;

  //クローズ
  case WFBC_PALACE_TALKSYS_SEQ_CLOSE:
    FLDTALKMSGWIN_StartClose(p_wk->p_talkwin);
    p_wk->talk_seq ++;
    break;

  // 終了
  case WFBC_PALACE_TALKSYS_SEQ_CLOSE_WAIT:
    if(FLDTALKMSGWIN_WaitClose(p_wk->p_talkwin))
    {
      p_wk->p_talkwin = NULL;
      return TRUE;
    }
    break;

  default:
    GF_ASSERT(0);
    break;
  }
  return FALSE;
}




