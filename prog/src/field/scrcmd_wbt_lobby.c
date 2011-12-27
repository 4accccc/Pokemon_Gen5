//============================================================================================
/**
 * @file  scrcmd_wbt_lobby.c
 * @author  tamada GAMEFREAK inc.
 * @date  2011.02.21
 * @brief WBT用スクリプトコマンド実装：ロビー専用
 */
//============================================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "script_local.h" //SCRIPT_GetEvent

#include "scrcmd_wbt.h"

#include "fldmmdl.h"
#include "fieldmap.h" //FIELDMAP_GetMMdlSys

#include "field/wbt.h"
#include "wbt_system.h"

#include "wbt_setup.h"
#include "wbt_tool.h"
#include "event_wbt.h"
#include "wbt_gimmick.h"

#include "system/main.h"
#include "arc/message.naix"
#include "msg/script/msg_wbt_lobby.h" //msg_wbt_info_ng_

#include "scrcmd_fld_battle.h"  //BtlPartySelectCheck,SCR_FLD_BATTLE_GetIsCallBtlPartySelect

#include "event_wbt_wifi.h"
#include "savedata/bsubway_savedata.h"

#include "savedata/save_tbl.h"  //GMDATA_ID_BSUBWAY_SCOREDATA
#include "field/zonedata.h" //ZONEDATA_GetPlaceNameID

#include "scrcmd_menuwin.h" //EvCmdBmpMenuStart

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static BSUBWAY_SCOREDATA * getBSubwayScoreData( SCRCMD_WORK * work )
{
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK * save_ctrl = GAMEDATA_GetSaveControlWork( gamedata );
  BSUBWAY_SCOREDATA * bsw_score = SaveControl_DataPtrGet( save_ctrl, GMDATA_ID_BSUBWAY_SCOREDATA );
  return bsw_score;
}

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：優勝時のごほうびBP数を取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetReward( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  u8 count;
  count = WBTSYS_GetRewardCount( wbtsys );

  if ( count > 0 )
  { //ごほうびのBPを足す
    BSUBWAY_SCOREDATA * bsw_score = getBSubwayScoreData( work );
    BSUBWAY_SCOREDATA_AddBattlePoint( bsw_score, count );

    {
      // レコード加算(勝ち取ったBP)
      GAMEDATA* gamedata = SCRCMD_WORK_GetGameData( work );
      RECORD*   record_sv =
                  SaveData_GetRecord( GAMEDATA_GetSaveControlWork(gamedata) );
      RECORD_Add( record_sv ,RECID_WIN_BP ,count );
    }
  }
  *ret_wk = count;
  TAMADA_Printf("EvCmdWBTGetReward == %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トーナメント開催状態の取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTCheckCupEnable( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u16 wbtcup = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  SCRCMD_WORK   *work = wk;
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );

  //受付があっていて、開催可能な場合のみTRUE
  if ( WBTTOOL_CheckReceptionID( wbtcup, WBTSYS_GetReceptionID( wbtsys ) ) == TRUE
      && WBTTOOL_CheckCupEnable( gamedata, wbtcup ) == TRUE )
  {
    *ret_wk = TRUE;
  }
  else
  {
    *ret_wk = FALSE;
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トーナメント解禁条件チェック
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTCheckCupUnlock( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  GAMEDATA* gamedata = SCRCMD_WORK_GetGameData( wk );

  u16 wbtcup = SCRCMD_GetVMWorkValue( core, wk );
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );

  //現在のトーナメントに勝ったら解禁されるものだけチェック
  *ret_wk = WBTTOOL_CheckCupUnlock( gamedata, WBTSYS_GetWBTCup( wbtsys ), wbtcup );
  //TAMADA_Printf("EvCmdWBTCheckCupUnlock(%d) == %d\n", wbtcup, *ret_wk);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ルール準拠の手持ちか？チェック
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTCheckBattleRule( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  const REGULATION * reg = WBTSYS_GetRegulation( wbtsys );
  GAMEDATA* gamedata = SCRCMD_WORK_GetGameData( wk );
  POKEPARTY * party = WBTSYS_GetUsePokeParty( wbtsys, gamedata );
  POKE_REG_RETURN_ENUM result;

  result = PokeRegulationMatchPartialPokeParty( reg, party );
  if ( result == POKE_REG_OK )
  {
    *ret_wk = TRUE;
  } else {
    *ret_wk = FALSE;
  }
#ifdef  PM_DEBUG
  WBTSYS_DEBUG_PrintRegulation( reg );
  TAMADA_Printf("EvCmdWBTCheckBattleRule() = %d\n", result);
#endif
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：現在のレギュレーションIDを取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetRegulationID( VMHANDLE * core, void *wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  *ret_wk = WBTSYS_GetRegulationID( wbtsys );
  TAMADA_Printf("EvCmdWBTGetRegulationID = %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：レギュレーション判定時にポケモン選択が発生するか
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTBtlUtilCheckPartySelect( VMHANDLE * core, void *wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  //結果を返すワーク

  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  const REGULATION* regu = WBTSYS_GetRegulation( wbtsys );

#ifdef  PM_DEBUG
  WBTSYS_DEBUG_PrintRegulation( regu );
#endif

  // レギュレーションチェック時にポケモン選択が必要かを調べる
  (*ret_wk) = SCR_FLD_BATTLE_GetIsCallBtlPartySelect( gsys, TRUE, regu, heapID );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ダウンロードレギュレーションの判定
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTBtlUtilPartySelect( VMHANDLE * core, void *wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  u16* ret_wk = SCRCMD_GetVMWork(core,work);  //結果を返すワーク
  
  {
    GMEVENT* event;
    HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
    const REGULATION* regu = WBTSYS_GetRegulation( wbtsys );
#ifdef  PM_DEBUG
    WBTSYS_DEBUG_PrintRegulation( regu );
#endif

    event = BtlPartySelectCheck( gsys, TRUE, regu, ret_wk, heapID );

    if(event == NULL){
      return VMCMD_RESULT_CONTINUE;
    }
    SCRIPT_CallEvent( sc, event );
  }
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：現在のレギュレーションでのNGメッセージIDを取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetNGMessageID( VMHANDLE * core, void * wk )
{
  static const u16 reg_msg_tbl[] = {
    REG_WBT_SINGLE,             msg_wbt_info_ng_all_single,
    REG_WBT_DOUBLE,             msg_wbt_info_ng_all_double,
    REG_WBT_TRIPLE,             msg_wbt_info_ng_all_triple,
    REG_WBT_ROTATION,           msg_wbt_info_ng_all_rotation,
    REG_WBT_SUBWAY_SINGLE,      msg_wbt_info_ng_subway_single,
    REG_WBT_SUBWAY_DOUBLE,      msg_wbt_info_ng_subway_double,
    REG_WBT_SUBWAY_TRIPLE,      msg_wbt_info_ng_subway_triple,
    REG_WBT_SUBWAY_ROTATION,    msg_wbt_info_ng_subway_rotation,
    REG_WBT_EVENT_SINGLE,       msg_wbt_info_ng_all_single,
  };

  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u32 msg_id = msg_wbt_info_ng_all_single;

  if ( WBTSYS_GetWBTCup( wbtsys ) == WBTCUP_DOWNLOAD )
  { //ダウンロードトーナメントは説明しきれないので簡潔に
    msg_id = msg_wbt_info_ng_download;
  }
  else
  { //通常のトーナメントは設定レギュレーションのIDでメッセージを振り分ける
    int i;
    u32 reg_id = WBTSYS_GetRegulationID( wbtsys );
    for ( i = 0; i < NELEMS(reg_msg_tbl); i += 2 )
    {
      if ( reg_msg_tbl[i+0] == reg_id )
      {
        msg_id = reg_msg_tbl[i+1];
        break;
      }
    }
    if ( WBTSYS_GetWBTCup(wbtsys) == WBTCUP_POKETYPE )
    {
      msg_id += 4;
    }
  }
  *ret_wk = msg_id;
  TAMADA_Printf("EvCmdWBTGetNGMessageID = %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTバトルボックス使用のセットアップ
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetupBattleBox( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );
  u16 use_mode = SCRCMD_GetVMWorkValue( core, wk );
  BOOL use_flag = FALSE;

  switch ( use_mode )
  {
  default:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case SCR_BTL_PARTY_SELECT_CANCEL:
  case SCR_BTL_PARTY_SELECT_TEMOTI:
  case SCR_BTL_PARTY_SELECT_NG:
    use_flag = FALSE;
    break;
  case SCR_BTL_PARTY_SELECT_BTLBOX:
    use_flag = TRUE;
    break;
  }
  WBTSYS_SetBattleBoxUse( wbtsys, use_flag );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：レンタルトーナメント用の手持ち生成
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetupRentalParty( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );
  const MYSTATUS * myStatus = GAMEDATA_GetMyStatus( gamedata );
  u16 place_id = ZONEDATA_GetPlaceNameID( SCRCMD_WORK_GetZoneID( wk ) );
  u16 rental_type  = SCRCMD_GetVMWorkValue( core, wk );

  WBTSYS_SetRentalType( wbtsys, rental_type );
  WBTSYS_SetupMyRentalParty( wbtsys, rental_type, myStatus, place_id );
  TAMADA_Printf("EvCmdWBTSetupRentalParty\n");
  return VMCMD_RESULT_CONTINUE;
}

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBT対戦者（ロビー出現）の動作モデルコードを取得する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetLobbyMMdlCode( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
  u16 * ret_wk1 = SCRCMD_GetVMWork( core, wk );
  u16 * ret_wk2 = SCRCMD_GetVMWork( core, wk );
  u16 * ret_wk3 = SCRCMD_GetVMWork( core, wk );
  u16 tr_id1 = WBTSAVE_GetTrainerID( wbtsave, SCR_WBT_LOBBY_OBJ_1 );
  u16 tr_id2 = WBTSAVE_GetTrainerID( wbtsave, SCR_WBT_LOBBY_OBJ_2 );
  u16 tr_id3 = WBTSAVE_GetTrainerID( wbtsave, SCR_WBT_LOBBY_OBJ_3 );
  const WBTROM_TRDATA * rom_trdata;

  rom_trdata = WBTTOOL_CreateRomTrainerData( SCRCMD_WORK_GetHeapID( work ) );
  *ret_wk1 = 0;
  *ret_wk2 = 0;
  *ret_wk3 = 0;
  if ( WBTTOOL_IsVaridRomTrainerID(rom_trdata, tr_id1) 
      && WBTTOOL_GetRomTrainerPriority( rom_trdata, tr_id1 ) >= WBT_TRPRI_LEADER )
  {
    *ret_wk1 = WBTTOOL_GetRomTrainerMMdlID( rom_trdata, tr_id1 );
  }
  if ( WBTTOOL_IsVaridRomTrainerID(rom_trdata, tr_id2)
      && WBTTOOL_GetRomTrainerPriority( rom_trdata, tr_id2 ) >= WBT_TRPRI_LEADER )
  {
    *ret_wk2 = WBTTOOL_GetRomTrainerMMdlID( rom_trdata, tr_id2 );
  }
  if ( WBTTOOL_IsVaridRomTrainerID(rom_trdata, tr_id3)
      && WBTTOOL_GetRomTrainerPriority( rom_trdata, tr_id3 ) >= WBT_TRPRI_LEADER )
  {
    *ret_wk3 = WBTTOOL_GetRomTrainerMMdlID( rom_trdata, tr_id3 );
  }
  WBTTOOL_DeleteRomTrainerData( (void*)rom_trdata );
  TAMADA_Printf("EvCmdWBTGetLobbyMMdlCode() = %d,%d,%d\n", *ret_wk1, *ret_wk2, *ret_wk3 );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBT対戦者（ロビー出現）のIDを取得する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetLobbyTrainerID( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
  u16 pos_id   = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = WBTSAVE_GetTrainerID( wbtsave, pos_id );
  TAMADA_Printf("EvCmdWBTGetLobbyTrainerID(%d) = %d\n", pos_id, *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}


//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ダウンロードデータ数取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetDownLoadCount( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  GAMEDATA* gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = WBTSYS_GetDownLoadCount( wbtsys, gamedata );
  TAMADA_Printf("EvCmdWBTGetDownLoadCount == %d\n", *ret_wk );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTDLデータのナンバーセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetDLData( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  GAMEDATA* gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  u16 number = SCRCMD_GetVMWorkValue( core, wk );

  //numberをWBT_SYSTEMにセット
  WBTSYS_SetSelectDLData( wbtsys, number );
  if ( WBTSYS_GetWBTCup(wbtsys) == WBTCUP_DOWNLOAD )
  { //ダウンロードデータを読み込み
    WBTSYS_LoadDLData( wbtsys, gamedata );
  }
  else
  {
    GF_ASSERT( 0 );
  }
  TAMADA_Printf("EvCmdWBTSetDLData <= %d\n", number );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTDLデータのナンバー取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetDLNumber( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  *ret_wk = WBTSYS_GetSelectDLData( wbtsys );
  TAMADA_Printf("EvCmdWBTGetDLNumber == %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//============================================================================================
//
//    WBT用コマンド：別アプリ呼び出し
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：参加ポケモンを選択する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSelectPokemon( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  GMEVENT * event;

  event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_wbt), WBT_EVENT_SetSelectPokeList, (void *)wbtsys );
  TAMADA_Printf("EvCmdWBTSelectPokemon\n");
  SCRIPT_CallEvent( sc, event );
  WBTSYS_SetScriptAnswerWork( wbtsys, ret_wk );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTDLデータのナンバー選択
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSelectDLData( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  HEAPID      heap_id = SCRCMD_WORK_GetHeapID( work );
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GMEVENT * new_event;
	T_DOWNLOAD_PARAM * td_param;

  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  u16 * cur_pos = SCRCMD_GetVMWork( core, wk );

  td_param = T_DOWNLOAD_PARAM_Create( heap_id, gsys, T_DOWNLOAD_STATUS_JOIN, ret_wk, cur_pos );
	new_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_wbt), T_DOWNLOAD_EVENT_Show, (void *)td_param );
  TAMADA_Printf("EvCmdWBTSelectDLData\n");
  SCRIPT_CallEvent( sc, new_event );
  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：対エントリーチェッカー（優勝記録確認画面）の呼び出し
 */
//----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTECheckerRecord( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  HEAPID      heap_id = SCRCMD_WORK_GetHeapID( work );
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GMEVENT * new_event;
  WIN_RECORD_PARAM * wr_param;

  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  wr_param = WIN_RECORD_PARAM_Create( heap_id, gsys, ret_wk );
  new_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_wbt), WIN_RECORD_EVENT_Show, (void*)wr_param );
  //結果を受け取るワークを設定する
  //WBTSYS_SetScriptAnswerWork( wbtsys, ret_wk );
  TAMADA_Printf("EvCmdWBTECheckerRecord\n");
  SCRIPT_CallEvent( sc, new_event );
  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：エントリーチェッカー（ダウンロード画面）の呼び出し
 */
//----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTECheckerDownLoad( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  HEAPID      heap_id = SCRCMD_WORK_GetHeapID( work );
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GMEVENT * new_event;
	T_DOWNLOAD_PARAM * td_param;

  u16 app_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

	new_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(event_wbt_download), T_DOWNLOAD_EVENT_ShowWifi, NULL );
  //結果を受け取るワークを設定する
  //WBTSYS_SetScriptAnswerWork( wbtsys, ret_wk );
  TAMADA_Printf("EvCmdWBTECheckerDownLoad\n");
  SCRIPT_CallEvent( sc, new_event );
  return VMCMD_RESULT_SUSPEND;
}

