//==============================================================================
/**
 * @file    scrcmd_intrude.c
 * @brief   スクリプトコマンド：侵入
 * @author  matsuda
 * @date    2009.10.26(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_intrude.h"

#include "field_comm\intrude_types.h"
#include "field_comm\intrude_work.h"
#include "field_comm\intrude_comm_command.h"
#include "field/field_comm/intrude_monolith.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "field/field_comm/intrude_field.h"

#include "event_symbol.h"
#include "symbol_map.h"

#include "message.naix"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_msg.h"


//==============================================================================
//  構造体定義
//==============================================================================
///イベント制御ワーク：ミッション開始
typedef struct{
  INTRUDE_EVENT_MSGWORK iem;  ///<プリント制御
  BOOL error;                 ///<TRUE:エラー発生
  u16 zone_id;                ///<ゾーンID
  u8 monolith_type;           ///<モノリスタイプ
  u8 padding;
}EVENT_MISSION_START;




//==================================================================
/**
 * パレス島の通信相手側にいるNPC動作モデルを登録
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudePalaceMmdlSetting( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  
  IntrudeField_PalaceMMdlAllAdd(fieldWork);
  
  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * パレス島の初期マップ連結セッティング
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdIntrudeConnectMapSetting( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm = NULL;
  
  if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_INVASION){
    intcomm = GameCommSys_GetAppWork(game_comm);
  }

  if(intcomm != NULL){
    IntrudeField_ConnectMapInit(fieldWork, gsys, intcomm);
    IntrudeField_ConnectMap(fieldWork, gsys, intcomm);
  }
  else{
    IntrudeField_ConnectMapNum(fieldWork, 1); //非通信時は1つ
  }
  
  MONOLITH_AddConnectAllMap(fieldWork);

  return VMCMD_RESULT_CONTINUE;
}

