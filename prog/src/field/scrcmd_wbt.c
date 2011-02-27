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

#include "wbt.h"
#include "event_wbt.h"

#include "scrcmd_wbt.h"

#include "system/main.h"

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemStart( VMHANDLE * core, void * wk )
{
  //仮：
  WBTSYS * sys = WBTSYS_Create( HEAPID_PROC );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTSystemEnd( VMHANDLE * core, void * wk )
{
  //仮：
  WBTSYS_Delete( NULL );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トーナメントリストの表示
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTShowTornamentList( VMHANDLE * core, void * wk )
{
  u16 list_type = SCRCMD_GetVMWorkValue( core, wk );
  //仮：ここでトーナメント表を表示
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

  //仮
  *ret_wk = TRUE;
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
  //仮
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
  *ret_wk = SCR_WBT_RULE_SINGLE;  //仮
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
  //仮
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
  *ret_wk = SCR_WBT_TYPE_ALL; //仮
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

  //仮
  *ret_wk = 3;

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

  u8 idx = VMGetU8(core);
  u16 dl_number = SCRCMD_GetVMWorkValue( core, work );

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
  //仮：ここで別画面アプリ呼び出し
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
  //仮：numberをWBT_SYSTEMにセット
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
  *ret_wk = 0;  //仮
  return VMCMD_RESULT_CONTINUE;
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
  u16 type = SCRCMD_GetVMWorkValue( core, wk );
  u16 obj_id = SCRCMD_GetVMWorkValue( core, wk );
  //仮：トレーナーに合わせたメッセージを表示する
  return VMCMD_RESULT_SUSPEND;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTTrainerBattle( VMHANDLE * core, void * wk )
{
  u16 num = SCRCMD_GetVMWorkValue( core, wk );
  //仮：ここからトレーナーバトルイベントを呼び出す
  return VMCMD_RESULT_SUSPEND;
}
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTAnnounce( VMHANDLE * core, void * wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  //仮：前口上メッセージを表示する
  return VMCMD_RESULT_SUSPEND;
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

  u8 buf = TRUE;
  //仮：ここからポケモン選択イベントを呼び出す
  SCRIPT_CallEvent( sc, WBT_EVENT_SetSelectPokeList( NULL, gsys, buf ) );

  return VMCMD_RESULT_SUSPEND;
}

