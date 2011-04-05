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

#include "scrcmd_wbt.h"

#include "savedata/medalbox.h"

#include "fldmmdl.h"
#include "fieldmap.h" //FIELDMAP_GetMMdlSys
#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"

#include "field/wbt.h"
#include "wbt_system.h"
#include "wbt_system_local.h"
#include "event_wbt.h"
#include "wbt_gimmick.h"


#include "system/main.h"
#include "arc/message.naix"
#include "msg/msg_wbt_system.h"

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
  //仮
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

  p_param = WBTLIST_PARAM_Create( heapID, gsys );
  WBTLIST_PARAM_Setup_Debug( p_param, list_type, gsys );

  next_event = WBT_EVENT_ShowTournamentList( gsys, fieldmap, p_param, FALSE );
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

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBT対戦トレーナーの動作モデルコード取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetEnemyMMdlCode( VMHANDLE * core, void * wk )
{
  WBTSYS * wbtsys = getWBTSys( wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = WBTTOOL_GetEnemyTrainerMMdlCode( wbtsys );
  return VMCMD_RESULT_CONTINUE;
}

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
static void wordsetRegisterArc( SCRCMD_WORK * work, u32 bufID, u32 arc_id, u32 msg_id )
{
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
  HEAPID        heapID = SCRCMD_WORK_GetHeapID( work );
  GFL_MSGDATA     *man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, arc_id, heapID );
  if( man )
  {
    STRBUF * strbuf = SCRIPT_GetMsgTempBuffer( sc );
    GFL_MSG_GetString( man, msg_id, strbuf );
    WORDSET_RegisterWord( wordset, bufID, strbuf, PM_NEUTRAL, TRUE, PM_LANG );
    GFL_MSG_Delete( man );
  }
}

//--------------------------------------------------------------
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
    u8 sex = WBTTOOL_GetEnemyTrainerSex( wbtsys );
    STRBUF * strbuf = SCRIPT_GetMsgTempBuffer( sc );
    WBTTOOL_SetEnemyTrainerName( wbtsys, strbuf );
    WORDSET_RegisterWord( wordset, idx, strbuf, sex, TRUE, PM_LANG );
  }
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

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック背景ビジョン初期化
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKVisionInit( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  u8 type = SCRCMD_GetVMWorkValue( core, wk );

  // ギミック開始
  WBT_GIMMICK_VisionInit( fieldWork, type );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド:WBTギミック背景ビジョン変更
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGMKVisionChange( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK   *work = wk;
  GAMESYS_WORK  *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  u8 type = SCRCMD_GetVMWorkValue( core, wk );

  // ギミック変更
  WBT_GIMMICK_VisionChange( fieldWork, type );

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
