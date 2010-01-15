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

#include "print/wordset.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"
#include "field_wfbc.h"
#include "fldmmdl.h"

#include "scrcmd_wfbc.h"

#include "scrcmd_wfbc_define.h" 

#include "msg/msg_place_name.h"  // for MAPNAME_xxxx

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

  objid = MMDL_GetOBJID( p_frontmmdl );

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
  objid = MMDL_GetOBJID( p_frontmmdl );


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
  WORDSET** pp_wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
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
  objid = MMDL_GetOBJID( p_frontmmdl );

  // ワードセットに設定
  FIELD_WFBC_SetWordSetParentPlayer( p_wfbc, *pp_wordset, objid, idx, heapID );

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
      objid = MMDL_GetOBJID( p_frontmmdl );

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
    // 話し相手のOBJID（0～49）
    case WFBC_GET_PARAM_OBJ_ID:
      (*ret_wk) = objid;
      break;
    // BC　相手とバトル可能かチェック
    case WFBC_GET_PARAM_BC_CHECK_BATTLE:
      (*ret_wk) = FIELD_WFBC_IsBattlePeople( p_wfbc, objid );
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




