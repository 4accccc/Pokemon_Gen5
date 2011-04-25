//============================================================================================
/**
 * @file  scrcmd_wbt.c
 * @author  tamada GAMEFREAK inc.
 * @date  2011.02.21
 * @brief WBT用スクリプトコマンド実装
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

#include "savedata/medalbox.h"

#include "fldmmdl.h"
#include "fieldmap.h" //FIELDMAP_GetMMdlSys
#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"

#include "field/wbt.h"
#include "wbt_system.h"

#include "wbt_setup.h"
#include "wbt_tool.h"
#include "event_wbt.h"
#include "wbt_gimmick.h"

#include "system/main.h"
#include "arc/message.naix"
#include "msg/msg_wbt_system.h"

#include "app/wbt_list.h"

#include "scrcmd_menuwin.h" //EvCmdBalloonWin_Write
#include "arc/message.naix" //NARC_message_wbt_trmsg_dat


//============================================================================================
//============================================================================================
SDK_COMPILER_ASSERT( SCR_WBT_RULE_NULL == WBTRULE_NULL );
SDK_COMPILER_ASSERT( SCR_WBT_RULE_SINGLE == WBTRULE_SINGLE );
SDK_COMPILER_ASSERT( SCR_WBT_RULE_DOUBLE == WBTRULE_DOUBLE );
SDK_COMPILER_ASSERT( SCR_WBT_RULE_TRIPLE == WBTRULE_TRIPLE );
SDK_COMPILER_ASSERT( SCR_WBT_RULE_ROTATION == WBTRULE_ROTATION );

SDK_COMPILER_ASSERT( SCR_WBT_TYPE_NULL == WBTTYPE_NULL );
SDK_COMPILER_ASSERT( SCR_WBT_TYPE_ALL == WBTTYPE_ALL );
SDK_COMPILER_ASSERT( SCR_WBT_TYPE_CHAMPION == WBTTYPE_CHAMPION );
SDK_COMPILER_ASSERT( SCR_WBT_TYPE_POKETYPE == WBTTYPE_POKETYPE );
SDK_COMPILER_ASSERT( SCR_WBT_TYPE_DOWNLOAD == WBTTYPE_DOWNLOAD );

SDK_COMPILER_ASSERT( SCR_WBT_STAGE_RECEPTION == WBTSTAGE_RECEPTION );
SDK_COMPILER_ASSERT( SCR_WBT_STAGE_ENTRY == WBTSTAGE_ENTRY );
SDK_COMPILER_ASSERT( SCR_WBT_STAGE_1ST == WBTSTAGE_1ST );
SDK_COMPILER_ASSERT( SCR_WBT_STAGE_2ND == WBTSTAGE_2ND );
SDK_COMPILER_ASSERT( SCR_WBT_STAGE_FINAL == WBTSTAGE_FINAL );
SDK_COMPILER_ASSERT( SCR_WBT_STAGE_WIN == WBTSTAGE_WIN );

SDK_COMPILER_ASSERT( WBT_TRMSG_1ST_FIGHT_START == SCR_WBT_TRMSG_1ST );
SDK_COMPILER_ASSERT( WBT_TRMSG_2ND_FIGHT_START == SCR_WBT_TRMSG_2ND );
SDK_COMPILER_ASSERT( WBT_TRMSG_3RD_FIGHT_START == SCR_WBT_TRMSG_3RD );
SDK_COMPILER_ASSERT( WBT_TRMSG_FIGHT_LOSE == SCR_WBT_TRMSG_LOSE );
SDK_COMPILER_ASSERT( WBT_TRMSG_FIGHT_WIN == SCR_WBT_TRMSG_WIN );
SDK_COMPILER_ASSERT( WBT_TRMSG_PLAYER_WIN == SCR_WBT_TRMSG_PLAYER_WIN );
SDK_COMPILER_ASSERT( WBT_TRMSG_PLAYER_LOSE == SCR_WBT_TRMSG_PLAYER_LOSE );
SDK_COMPILER_ASSERT( WBT_TRMSG_ANNOUNCE == SCR_WBT_TRMSG_ANNOUNCE );

SDK_COMPILER_ASSERT( BTL_RESULT_LOSE == SCR_WBT_BATTLE_RESULT_LOSE );
SDK_COMPILER_ASSERT( BTL_RESULT_WIN == SCR_WBT_BATTLE_RESULT_WIN );
SDK_COMPILER_ASSERT( BTL_RESULT_DRAW == SCR_WBT_BATTLE_RESULT_DRAW );

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static WBTSYS * getWBTSys( SCRCMD_WORK * work )
{
  GAMEDATA*          gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS ** wbtsys_work = GAMEDATA_GetWBTSysPointer( gamedata );
  if ( *wbtsys_work == NULL )
  {
    OS_Printf("WBTシステムが起動していません！！\n");
    GF_ASSERT(0);
  }
  return *wbtsys_work;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void incVictoryCount( WBTSYS * wbtsys, SCRCMD_WORK * work )
{
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
  WBTWIN_COUNT id;

  switch ( WBTSYS_GetType( wbtsys ) )
  {
  case WBTTYPE_NULL:
  case WBTTYPE_DOWNLOAD:
    //何もしない
    return;
  case WBTTYPE_ALL:
    id = WBTWIN_COUNT_ALLTOURNAMENT;
    break;
  case WBTTYPE_CHAMPION:
    id = WBTWIN_COUNT_CHAMPION;
    break;
  case WBTTYPE_POKETYPE:
    id = WBTSYS_GetPokeType( wbtsys );
    break;
  }
  WBTSAVE_IncWinCount( wbtsave, id );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 calcTotalVictory( SCRCMD_WORK * work )
{
  u32 count = 0;

  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
  u32 id;
  for ( id = 0; id < WBTWIN_COUNT_MAX; id ++ )
  {
    count += WBTSAVE_GetWinCount( wbtsave, id );
    if ( count >= WBTWIN_MAX )
    {
      count = WBTWIN_MAX;
      break;
    }
  }
  return count;
}
//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTシステム開始
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemStart( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS ** wbtsys_work;

  TAMADA_Printf("EvCmdWBTSystemStart\n");
  wbtsys_work = GAMEDATA_GetWBTSysPointer( gamedata );
  GF_ASSERT( *wbtsys_work == NULL );
  *wbtsys_work = WBTSYS_Create( HEAPID_PROC );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTシステム終了
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemEnd( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS ** wbtsys_work;

  TAMADA_Printf("EvCmdWBTSystemEnd\n");
  wbtsys_work = GAMEDATA_GetWBTSysPointer( gamedata );
  GF_ASSERT( *wbtsys_work != NULL );
  WBTSYS_Delete( *wbtsys_work );
  *wbtsys_work = NULL;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：状態設定
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemSetStage( VMHANDLE * core, void * wk )
{
  u16 stage = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );

  TAMADA_Printf("EvCmdWBTSystemSetStage(%d)\n", stage);
  WBTSYS_SetStage( wbtsys, stage );

  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：エントリー
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemEntry( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );

  GAMEDATA* gamedata = SCRCMD_WORK_GetGameData( wk );
  WBTSYS_MakeMatch( wbtsys, gamedata );
  TAMADA_Printf("EvCmdWBTSystemEntry\n");

  return VMCMD_RESULT_CONTINUE;
}

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トーナメント開催状態の取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTCheckTournament( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  u16 type = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  SCRCMD_WORK   *work = wk;
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );

  switch ( type )
  {
  case WBTTYPE_NULL:
  case WBTTYPE_ALL:
    *ret_wk = TRUE; //いつだってオールトーナメントは開催する
    break;
  case WBTTYPE_POKETYPE:
    *ret_wk = WBTTOOL_GetStatus( gamedata, WBT_COND_ID_ENABLE_POKETYPE_TOURNAMENT );
    break;
  case WBTTYPE_CHAMPION:
    *ret_wk = WBTTOOL_GetStatus( gamedata, WBT_COND_ID_ENABLE_CHAMPION_TOURNAMENT );
    break;
  case WBTTYPE_DOWNLOAD:
    *ret_wk = WBTTOOL_GetStatus( gamedata, WBT_COND_ID_ENABLE_DOWNLOAD_TOURNAMENT );
    break;
  }
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
  WBTSYS * wbtsys = getWBTSys( wk );
  const REGULATION * reg = WBTSYS_GetRegulation( wbtsys );
  GAMEDATA* gamedata = SCRCMD_WORK_GetGameData( wk );
  POKEPARTY * party = GAMEDATA_GetMyPokemon( gamedata );
  BOOL result;
  //仮
  TAMADA_Printf("EvCmdWBTCheckBattleRule\n");
  result = PokeRegulationMatchPartialPokeParty( reg, party );
  if ( result == POKE_REG_OK )
  {
    *ret_wk = TRUE;
  } else {
    *ret_wk = FALSE;
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTトーナメントのルールセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetBattleRule( VMHANDLE * core, void * wk )
{
  u16 rule = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );
  WBTSYS_SetRule( wbtsys, rule );
  TAMADA_Printf("EvCmdWBTSetBattleRule <= %d\n", rule );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTトーナメントのルール取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetBattleRule( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );
  *ret_wk = WBTSYS_GetRule( wbtsys );
  TAMADA_Printf("EvCmdWBTGetBattleRule = %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTトーナメントのタイプセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetTournamentType( VMHANDLE * core, void * wk )
{
  u16 type = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );
  WBTSYS_SetType( wbtsys, type );
  TAMADA_Printf("EvCmdWBTSetTournamentType <= %d\n", type);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTトーナメントのタイプ取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetTournamentType( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );
  *ret_wk = WBTSYS_GetType( wbtsys );
  TAMADA_Printf("EvCmdWBTGetTournamentType == %d\n", *ret_wk);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTタイプトーナメントのポケモンタイプセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetTournamentPokeType( VMHANDLE * core, void * wk )
{
  u16 type = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );
  WBTSYS_SetPokeType( wbtsys, type );
  TAMADA_Printf("EvCmdWBTSetTournamentPokeType <= %d\n", type);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTタイプトーナメントのポケモンタイプ取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetTournamentPokeType( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );
  *ret_wk = WBTSYS_GetPokeType( wbtsys );
  TAMADA_Printf("EvCmdWBTGetTournamentPokeType == %d\n", *ret_wk);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：優勝回数のインクリメント
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTIncVictoryCount( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  WBTSYS * wbtsys = getWBTSys( wk );
  WBTSTAGE stage = WBTSYS_GetStage( wbtsys );

  TAMADA_Printf("EvCmdWBTIncVictoryCount\n");
  GF_ASSERT( stage == WBTSTAGE_WIN );
  incVictoryCount( wbtsys, work );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：優勝回数の取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetVictoryCount( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  u32 count = 0;

  count = calcTotalVictory( work );
  *ret_wk = count;

  TAMADA_Printf("EvCmdWBTGetVictoryCount == %d\n", *ret_wk );
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
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );

  //仮
  *ret_wk = 3;
  TAMADA_Printf("EvCmdWBTGetDownLoadCount == %d\n", *ret_wk );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ダウンロードデータ名セット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetDownLoadName( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
  WBTSYS * wbtsys = getWBTSys( wk );

  u8 idx = VMGetU8(core);
  u16 dl_number = SCRCMD_GetVMWorkValue( core, work );

  TAMADA_Printf("EvCmdWBTGetDownLoadName\n");
  //仮データセット
  WORDSET_RegisterNumber( wordset, idx, dl_number, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTDLデータのナンバーセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetDLNumber( VMHANDLE * core, void * wk )
{
  u16 number = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );
  //numberをWBT_SYSTEMにセット
  WBTSYS_SetSelectDLData( wbtsys, number );
  TAMADA_Printf("EvCmdWBTSetDLNumber <= %d\n", number );
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
  WBTSYS * wbtsys = getWBTSys( wk );
  *ret_wk = WBTSYS_GetSelectDLData( wbtsys );
  TAMADA_Printf("EvCmdWBTGetDLNumber == %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//============================================================================================
//
//
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBT対戦トレーナーの動作モデルコード取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetEnemyMMdlCode( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = WBTSYS_GetOpponentMMdlObjCode( wbtsys );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBT対戦トレーナーのメッセージ表示
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTTrainerMsg( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  SCRIPT_WORK * sc = SCRCMD_WORK_GetScriptWork( work );
  WBTSYS * wbtsys = getWBTSys( wk );
  u16 type = SCRCMD_GetVMWorkValue( core, wk );
  u16 obj_id = SCRCMD_GetVMWorkValue( core, wk );
  BOOL result; 

  const WBTTRAINER * opponent = WBTSYS_GetOpponentTrainer( wbtsys );
  u16 msg_id = WBTTRAINER_GetID( opponent ) * WBT_TRMSG_TYPE_NUM;
  STRBUF *msgbuf = SCRIPT_GetMsgBuffer( sc );

  switch ( type )
  {
  default:
    GF_ASSERT( 0 );
  case WBT_TRMSG_1ST_FIGHT_START:
  case WBT_TRMSG_2ND_FIGHT_START:
  case WBT_TRMSG_3RD_FIGHT_START:
  case WBT_TRMSG_PLAYER_WIN:
  case WBT_TRMSG_PLAYER_LOSE:
    msg_id += type;
  };
  TAMADA_Printf("EvCmdWBTTrainerMsg(type = %d, obj_id =%d )\n", type, obj_id );
  {
    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );

    GFL_MSGDATA * msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
        NARC_message_wbt_trmsg_dat, SCRCMD_WORK_GetHeapID( work ) );
    GFL_MSG_GetString( msgData, msg_id, tmpbuf );
    GFL_MSG_Delete( msgData );

    WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
  }

  //トレーナーに合わせたメッセージを表示する
  return EvCmdBalloonWin_Write( core, wk, msgbuf, obj_id,
      SCRCMD_MSGWIN_DEFAULT, TALKMSGWIN_TYPE_NORMAL );
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTトレーナー対戦呼び出し
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTTrainerBattle( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*          gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS * wbtsys = getWBTSys( wk );
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  GMEVENT * next_event;
  TAMADA_Printf("EvCmdWBTTrainerBattle\n");
  GAMEDATA_SetLastBattleResult( gamedata, BTL_RESULT_WIN );

  //対戦結果を受け取るワークを設定する
  WBTSYS_SetScriptAnswerWork( wbtsys, ret_wk );

  //ここからトレーナーバトルイベントを呼び出す
  next_event = WBT_EVENT_TrainerBattle( wbtsys, gsys );
  SCRIPT_CallEvent( sc, next_event );
  return VMCMD_RESULT_SUSPEND;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：前口上メッセージを表示する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTAnnounce( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  STRBUF *msgbuf = SCRIPT_GetMsgBuffer( sc );
  u16 up_down = SCRCMD_GetVMWorkValue( core, wk );

  WBTSYS * wbtsys = getWBTSys( wk );
  const WBTTRAINER * opponent = WBTSYS_GetOpponentTrainer( wbtsys );
  u16 msg_id = WBTTRAINER_GetID( opponent ) * WBT_TRMSG_TYPE_NUM;
  msg_id += WBT_TRMSG_ANNOUNCE;

  TAMADA_Printf("EvCmdWBTAnnounce\n");
  {

    WORDSET *wordset = SCRIPT_GetWordSet( sc );
    STRBUF *tmpbuf = SCRIPT_GetMsgTempBuffer( sc );

    GFL_MSGDATA * msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
        NARC_message_wbt_trmsg_dat, SCRCMD_WORK_GetHeapID( work ) );
    GFL_MSG_GetString( msgData, msg_id, tmpbuf );
    GFL_MSG_Delete( msgData );

    WORDSET_ExpandStr( wordset, msgbuf, tmpbuf );
  }
  return EvCmdplainWin_Write( core, work, msgbuf, TALKMSGWIN_TYPE_NORMAL, up_down );
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トーナメント名のセット
 * @todo  ダウンロードデータへの対応
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetTournamentName( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
  WBTSYS * wbtsys = getWBTSys( wk );
  u8 idx = VMGetU8(core);
  WBTTYPE wbttype;
  u16 msg_id;

  TAMADA_Printf("EvCmdWBTSetTournamentName\n");
  wbttype = WBTSYS_GetType(wbtsys);
  {
    STRBUF * strbuf = SCRIPT_GetMsgTempBuffer( sc );
    WBTTOOL_SetTournamentName( wbttype, strbuf );
    WORDSET_RegisterWord( wordset, idx, strbuf, PM_NEUTRAL, TRUE, PM_LANG );
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：対戦者名のセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetEnemyName( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
  WBTSYS * wbtsys = getWBTSys( wk );
  u8 idx = VMGetU8(core);
  TAMADA_Printf("EvCmdWBTSetEnemyName\n");
  {
    u8 sex = WBTSYS_GetOpponentSex( wbtsys );
    STRBUF * strbuf = SCRIPT_GetMsgTempBuffer( sc );
    WBTSYS_GetOpponentName( wbtsys, strbuf );
    WORDSET_RegisterWord( wordset, idx, strbuf, sex, TRUE, PM_LANG );
  }
  return VMCMD_RESULT_CONTINUE;
}

//============================================================================================
//
//    WBT用コマンド：別アプリ呼び出し
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トーナメントリストの表示
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTShowTornamentList( VMHANDLE * core, void * wk )
{
  u16 list_type = SCRCMD_GetVMWorkValue( core, wk );
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK  *gsys = SCRIPT_GetGameSysWork( sc );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  WBTSYS * wbtsys = getWBTSys( wk );

  GMEVENT * next_event;
  WBTLIST_PARAM * p_param;

  p_param = WBTLIST_PARAM_Create( heapID, gsys );
  WBTLIST_PARAM_Setup( gsys, wbtsys, p_param );

  next_event = WBT_EVENT_ShowTournamentList( gsys, fieldmap, p_param, FALSE );
  SCRIPT_CallEvent( sc, next_event );
  OS_Printf( "WBT List Start\n" );

  return VMCMD_RESULT_SUSPEND;
}

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
  WBTSYS * wbtsys = getWBTSys( wk );

  u8 buf = FALSE;
  TAMADA_Printf("EvCmdWBTSelectPokemon\n");
  SCRIPT_CallEvent( sc, WBT_EVENT_SetSelectPokeList( wbtsys, gsys, buf ) );
  WBTSYS_SetScriptAnswerWork( wbtsys, ret_wk );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static GMEVENT * createDummyEvent( HEAPID heapID, GAMESYS_WORK * gsys )
{
  GMEVENT * new_event;
  WBTLIST_PARAM * wbtlist_param;
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  wbtlist_param = WBTLIST_PARAM_Create( heapID, gsys );
  {
    GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
    WBTSYS * wbtsys;

    wbtsys = WBTSYS_Create( heapID );
    WBTSYS_SetType( wbtsys, WBTTYPE_ALL );
    WBTSYS_SetRule( wbtsys, WBTRULE_SINGLE );
    WBTSYS_SetStage( wbtsys, WBTSTAGE_ENTRY );
    WBTSYS_MakeMatch( wbtsys, gamedata );

    WBTLIST_PARAM_Setup( gsys, wbtsys, wbtlist_param );
    //wbtlist_param->debug_value = debug_value;

    WBTSYS_Delete( wbtsys );
  }
  new_event = WBT_EVENT_ShowTournamentList( gsys, fieldmap, wbtlist_param, TRUE );
  return new_event;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTDLデータのナンバー選択
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSelectDLData( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  HEAPID      heap_id = SCRCMD_WORK_GetHeapID( work );
  GMEVENT * new_event = createDummyEvent( heap_id, gsys );
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );

  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  *ret_wk = 0;  //仮：選択せず
  //仮：ここで別画面アプリ呼び出し
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

  wr_param = WIN_RECORD_PARAM_Create( heap_id, gsys );
  new_event = WIN_RECORD_EVENT_Show( gsys, fieldmap, wr_param, TRUE );
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
  GMEVENT * new_event = createDummyEvent( heap_id, gsys );
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );

  //仮：ここで別画面アプリ呼び出し
  TAMADA_Printf("EvCmdWBTECheckerDownLoad\n");
  SCRIPT_CallEvent( sc, new_event );
  return VMCMD_RESULT_SUSPEND;
}


//============================================================================================
//
//      WBT用コマンド：ギミック関連
//
//============================================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック背景ビジョン変更
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKVisionChange( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA     *gdata = SCRCMD_WORK_GetGameData( work );
  HEAPID      heap_id = SCRCMD_WORK_GetHeapID( work );
  SCRIPT_WORK*     sc = SCRCMD_WORK_GetScriptWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  WBTSYS * wbtsys = getWBTSys( wk );
  STRBUF * strbuf1;
  STRBUF * strbuf2;   // プレイヤー名専用

  u8 type = SCRCMD_GetVMWorkValue( core, wk );    // ビジョン種類

  switch( type ){
  // VSビジョン
  // strbuf1 = 左上の名前
  // strbuf2 = 右下の名前
  case SCR_WBT_VISION_VS:
    strbuf1 = SCRIPT_GetMsgTempBuffer( sc );
    WBTSYS_GetOpponentName( wbtsys, strbuf1 );

    // プレイヤー名作成
    {
      MYSTATUS* myStatus;
      myStatus = GAMEDATA_GetMyStatus( gdata );
      strbuf2  = MyStatus_CreateNameString( myStatus, heap_id );
    }
    break;

  // 優勝ビジョン   プレイヤー勝ち
  // strbuf1 = 優勝者の名前
  // strbuf2 = 未使用
  case SCR_WBT_VISION_WIN_PLAYER:
    // プレイヤー名コピー
    {
      MYSTATUS* myStatus;
      myStatus = GAMEDATA_GetMyStatus( gdata );
      strbuf1 = SCRIPT_GetMsgTempBuffer( sc );
      MyStatus_CopyNameString( myStatus, strbuf1 );
    }
    strbuf2 = NULL;
    break;
  // 優勝ビジョン   敵が勝ち
  // strbuf1 = 優勝者の名前
  // strbuf2 = 未使用
  case SCR_WBT_VISION_WIN_ENEMY:
    strbuf1 = SCRIPT_GetMsgTempBuffer( sc );
    WBTSYS_GetOpponentName( wbtsys, strbuf1 );
    strbuf2 = NULL;
    break;

  default:
    strbuf1 = NULL;
    strbuf2 = NULL;
    break;
  }

  // ギミック変更
  WBT_GIMMICK_VisionChange( fieldWork, type, strbuf1, strbuf2 );

  // strbuf2はプレイヤー名で作成しているかもしれないので破棄
  if( strbuf2 ){ GFL_STR_DeleteBuffer( strbuf2 ); }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック通路エフェクト開始
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKEffectStart( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  u8 type = SCRCMD_GetVMWorkValue( core, wk );
  u8 pos = SCRCMD_GetVMWorkValue( core, wk );

  // ギミック開始
  WBT_GIMMICK_EffectStart( fieldWork, type, pos );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック優勝エフェクト開始
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKWinEffectStart( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  // ギミック開始
  WBT_GIMMICK_WinEffectStart( fieldWork );

  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//======================================================================
