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

#include "fldmmdl.h"
#include "fieldmap.h" //FIELDMAP_GetMMdlSys

#include "field/wbt.h"
#include "wbt_system.h"

#include "wbt_setup.h"
#include "wbt_tool.h"
#include "event_wbt.h"

#include "system/main.h"
#include "arc/message.naix"

#include "app/wbt_list.h"

#include "field/eventwork.h"  //EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  //SYS_FLAG_WBT_TUTORIAL

//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
SDK_COMPILER_ASSERT( SCR_WBT_CUP_NULL == WBTCUP_NULL );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_CHAMPION == WBTCUP_CHAMPION );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_POKETYPE == WBTCUP_POKETYPE );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_DOWNLOAD == WBTCUP_DOWNLOAD );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_HODOMOE == WBTCUP_HODOMOE );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_ISSYU == WBTCUP_ISSYU );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_KANTO == WBTCUP_KANTO );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_JOUTO == WBTCUP_JOUTO );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_HOUEN == WBTCUP_HOUEN );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_SINOU == WBTCUP_SINOU );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_WORLD == WBTCUP_WORLD );
SDK_COMPILER_ASSERT( SCR_WBT_CUP_HODOMOE_EVENT == WBTCUP_HODOMOE_EVENT );

SDK_COMPILER_ASSERT( SCR_WBT_RULE_SINGLE == WBTRULE_SINGLE );
SDK_COMPILER_ASSERT( SCR_WBT_RULE_DOUBLE == WBTRULE_DOUBLE );
SDK_COMPILER_ASSERT( SCR_WBT_RULE_TRIPLE == WBTRULE_TRIPLE );
SDK_COMPILER_ASSERT( SCR_WBT_RULE_ROTATION == WBTRULE_ROTATION );
SDK_COMPILER_ASSERT( SCR_WBT_RULE_NULL == WBTRULE_NULL );

SDK_COMPILER_ASSERT( SCR_WBT_RECEPTION1 == WBTRECEPT_1 );
SDK_COMPILER_ASSERT( SCR_WBT_RECEPTION2 == WBTRECEPT_2 );

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

SDK_COMPILER_ASSERT( WBTMIX_TARGET_MINE1 == SCR_WBT_MIX_TARGET_MINE1 );
SDK_COMPILER_ASSERT( WBTMIX_TARGET_MINE2 == SCR_WBT_MIX_TARGET_MINE2 );
SDK_COMPILER_ASSERT( WBTMIX_TARGET_ENEMY1 == SCR_WBT_MIX_TARGET_ENEMY1 );
SDK_COMPILER_ASSERT( WBTMIX_TARGET_ENEMY2 == SCR_WBT_MIX_TARGET_ENEMY2 );

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
WBTSYS * WBT_SCR_getWBTSys( SCRCMD_WORK * work )
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
/**
 * @brief  対応したトーナメントの優勝数をインクリメントする
 */
//--------------------------------------------------------------
static void incVictoryCount( WBTSYS * wbtsys, SCRCMD_WORK * work )
{
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
  WBTWIN_COUNT id;
  id = WBTTOOL_WBTCupToWinCount( WBTSYS_GetWBTCup( wbtsys ), WBTSYS_GetPokeType( wbtsys ) );
  if ( id < WBTWIN_COUNT_MAX )
  {
    WBTSAVE_IncWinCount( wbtsave, id );
  }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 calcTotalVictory( SCRCMD_WORK * work, WBTCUP wbtcup, PokeType poketype )
{
  u32 count = 0;

  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
  u32 id_start, id_end;

  if ( wbtcup == WBTCUP_NULL || wbtcup >= WBTCUP_MAX )
  {
    GF_ASSERT( wbtcup < WBTCUP_MAX );
    id_start = 0;
    id_end = WBTWIN_COUNT_MAX;
  }
  else if ( wbtcup == WBTCUP_POKETYPE )
  {
    if ( poketype == POKETYPE_NULL )
    {
      id_start = 0;
      id_end = WBTWIN_COUNT_POKETYPE_AKU + 1;
    }
    else
    {
      id_start = poketype;
      id_end = id_start + 1;
    }
  }
  else
  {
    id_start = WBTTOOL_WBTCupToWinCount( wbtcup, 0 );
    id_end = id_start + 1;
  }

  for ( ; id_start < id_end; id_start ++ )
  {
    count += WBTSAVE_GetWinCount( wbtsave, id_start );
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
  EVENTWORK * evwork = GAMEDATA_GetEventWork( gamedata );
  WBTSYS ** wbtsys_work;
  BOOL isTutorlal;

  isTutorlal = EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_WBT_TUTORIAL );
  TAMADA_Printf("EvCmdWBTSystemStart\n");
  wbtsys_work = GAMEDATA_GetWBTSysPointer( gamedata );
  GF_ASSERT( *wbtsys_work == NULL );
  *wbtsys_work = WBTSYS_Create( HEAPID_PROC, isTutorlal );

  //バトルボックスにポケモンがいる場合のセットアップ
  WBTSYS_SetupBattleBox( *wbtsys_work, gamedata );

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
 * @brief スクリプトコマンド：WBTシステム起動状態の取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemCheckEnable( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*       work     = (SCRCMD_WORK*)wk;
  GAMEDATA*          gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS ** wbtsys_work = GAMEDATA_GetWBTSysPointer( gamedata );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  if ( *wbtsys_work != NULL )
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
 * @brief スクリプトコマンド：状態セット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemSetStage( VMHANDLE * core, void * wk )
{
  u16 stage = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  TAMADA_Printf("EvCmdWBTSystemSetStage(%d)\n", stage);
  WBTSYS_SetStage( wbtsys, stage );

  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：状態取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemGetStage( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  *ret_wk = WBTSYS_GetStage( wbtsys );
  TAMADA_Printf("EvCmdWBTSystemGetStage(%d)\n", *ret_wk);

  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：エントリー
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemEntry( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  GAMEDATA* gamedata = SCRCMD_WORK_GetGameData( wk );

  WBTSYS_SetupMyParty( wbtsys, gamedata );
  WBTSYS_MakeMatch( wbtsys, gamedata );
  TAMADA_Printf("EvCmdWBTSystemEntry\n");

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
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
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
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  *ret_wk = WBTSYS_GetRule( wbtsys );
  TAMADA_Printf("EvCmdWBTGetBattleRule = %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTトーナメントのタイプセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetWBTCup( VMHANDLE * core, void * wk )
{
  u16 wbtcup = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  WBTSYS_SetWBTCup( wbtsys, wbtcup );
  TAMADA_Printf("EvCmdWBTSetWBTCup <= %d\n", wbtcup);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTトーナメントのタイプ取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetWBTCup( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  *ret_wk = WBTSYS_GetWBTCup( wbtsys );
  TAMADA_Printf("EvCmdWBTGetWBTCup == %d\n", *ret_wk);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTタイプトーナメントのポケモンタイプセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetTournamentPokeType( VMHANDLE * core, void * wk )
{
  u16 poketype = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  WBTSYS_SetPokeType( wbtsys, poketype );
  TAMADA_Printf("EvCmdWBTSetTournamentPokeType <= %d\n", poketype);
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
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  *ret_wk = WBTSYS_GetPokeType( wbtsys );
  TAMADA_Printf("EvCmdWBTGetTournamentPokeType == %d\n", *ret_wk);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBT受付のIDセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetReceptionID( VMHANDLE * core, void * wk )
{
  u16 reception_id = SCRCMD_GetVMWorkValue( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  WBTSYS_SetReceptionID( wbtsys, reception_id );
  TAMADA_Printf("EvCmdWBTSetReceptionID == %d\n", reception_id);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBT受付のID取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetReceptionID( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  *ret_wk = WBTSYS_GetReceptionID( wbtsys );
  TAMADA_Printf("EvCmdWBTGetReceptionID == %d\n", *ret_wk);
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
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
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
  u16   wbtcup = SCRCMD_GetVMWorkValue( core, wk );
  u16 poketype = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  u32 count = 0;

  count = calcTotalVictory( work, wbtcup, poketype );
  *ret_wk = count;

  TAMADA_Printf("EvCmdWBTGetVictoryCount(%d) == %d\n", wbtcup, *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：手持ち最大数を取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetPartyMax( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = WBTSYS_GetPartyMax( wbtsys );
  TAMADA_Printf("EvCmdWBTGetPartyMax=%d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トーナメント名のセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetTournamentName( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );
  u8 idx = VMGetU8(core);
  WBTCUP wbtcup = SCRCMD_GetVMWorkValue( core, wk );
  u16 msg_id;

  {
    STRBUF * strbuf = SCRIPT_GetMsgTempBuffer( sc );
    WBTSYS_GetTournamentName( wbtsys, wbtcup, strbuf );
    WORDSET_RegisterWord( wordset, idx, strbuf, PM_NEUTRAL, TRUE, PM_LANG );
  }
  TAMADA_Printf("EvCmdWBTSetTournamentName(%d,%d)\n", idx, wbtcup);
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
  WBTSYS * wbtsys = WBT_SCR_getWBTSys( wk );

  GMEVENT * next_event;
  WBTLIST_PARAM * p_param;

  p_param = WBTLIST_PARAM_Create( heapID, gsys );
  WBTLIST_PARAM_Setup( gsys, wbtsys, p_param );

  next_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_wbt), WBT_EVENT_ShowTournamentList, (void *)p_param );
  SCRIPT_CallEvent( sc, next_event );
  OS_Printf( "WBT List Start\n" );

  return VMCMD_RESULT_SUSPEND;
}

