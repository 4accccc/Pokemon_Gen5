//======================================================================
/**
 * @file  scrcmd_wbt.c
 * @author  tamada GAMEFREAK inc.
 * @date  2011.02.21
 * @brief
 */
//======================================================================

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

#include "scrcmd_medal.h"

#include "savedata/medalbox.h"

#include "fldmmdl.h"
#include "fieldmap.h" //FIELDMAP_GetMMdlSys
#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"

#include "field/wbt.h"
#include "wbt_system.h"
#include "event_wbt.h"

#include "scrcmd_wbt.h"

#include "system/main.h"

#include "app/wbt_list.h"
//======================================================================
//======================================================================
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

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static inline WBTSYS * getWBTSys( SCRCMD_WORK * work )
{
  GAMEDATA*          gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSYS ** wbtsys_work = GAMEDATA_GetWBTSysPointer( gamedata );
  GF_ASSERT( *wbtsys_work != NULL );
  return *wbtsys_work;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
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
 * @brief スクリプトコマンド：
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

//======================================================================
//======================================================================
FS_EXTERN_OVERLAY(namein);
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トーナメントリストの表示
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTShowTornamentList( VMHANDLE * core, void * wk )
{
  u16 list_type = SCRCMD_GetVMWorkValue( core, wk );
  //仮：ここでトーナメント表を表示
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK  *gsys = SCRIPT_GetGameSysWork( sc );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  WBTSYS * wbtsys = getWBTSys( wk );

  GMEVENT * next_event;
  WBTLIST_PARAM * p_param;

  p_param = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBTLIST_PARAM) );
  p_param->gamesys  = gsys;

  next_event = EVENT_FieldSubProcNoFade_Callback(
      gsys, fieldmap, FS_OVERLAY_ID(namein), &WBTListProcData, p_param,
      NULL, p_param );
  SCRIPT_CallEvent( sc, next_event );
  OS_Printf( "WBT List Start\n" );

  return VMCMD_RESULT_SUSPEND;
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
  GFL_HEAP_FreeMemory( (void*)reg );
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

//======================================================================
//======================================================================
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
 * @brief スクリプトコマンド：WBTDLデータのナンバー選択
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSelectDLData( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  //仮：ここで別画面アプリ呼び出し
  TAMADA_Printf("EvCmdWBTSelectDLData\n");
  return VMCMD_RESULT_SUSPEND;
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

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSelectPokemon( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );

  u8 buf = TRUE;
  TAMADA_Printf("EvCmdWBTSelectPokemon\n");
  //仮：ここからポケモン選択イベントを呼び出す
  SCRIPT_CallEvent( sc, WBT_EVENT_SetSelectPokeList( NULL, gsys, buf ) );

  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTTrainerMsg( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  u16 type = SCRCMD_GetVMWorkValue( core, wk );
  u16 obj_id = SCRCMD_GetVMWorkValue( core, wk );
  //仮：トレーナーに合わせたメッセージを表示する
  TAMADA_Printf("EvCmdWBTTrainerMsg(type = %d, obj_id =%d )\n", type, obj_id );
  return VMCMD_RESULT_SUSPEND;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTTrainerBattle( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  u16 num = SCRCMD_GetVMWorkValue( core, wk );
  //仮：ここからトレーナーバトルイベントを呼び出す
  TAMADA_Printf("EvCmdWBTTrainerBattle( num = %d)\n", num);
  return VMCMD_RESULT_SUSPEND;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTAnnounce( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  //仮：前口上メッセージを表示する
  TAMADA_Printf("EvCmdWBTAnnounce\n");
  return VMCMD_RESULT_SUSPEND;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetTournamentName( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  u8 idx = VMGetU8(core);
  TAMADA_Printf("EvCmdWBTSetTournamentName\n");
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSetEnemyName( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  u8 idx = VMGetU8(core);
  TAMADA_Printf("EvCmdWBTSetEnemyName\n");
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetVictoryCount( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  WBTSYS * wbtsys = getWBTSys( wk );

  //仮
  *ret_wk = 1;
  TAMADA_Printf("EvCmdWBTGetVictoryCount == %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//======================================================================
