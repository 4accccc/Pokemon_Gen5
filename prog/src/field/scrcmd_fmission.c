//==============================================================================
/**
 * @file    scrcmd_fmission.c
 * @brief   スクリプトコマンド：フェスミッション関連(フィールド常駐)
 * @author  iwasawa 
 * @date    2011.02.27
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
#include "field_comm\intrude_main.h"
#include "field_comm\intrude_comm_command.h"
#include "field/field_comm/intrude_monolith.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "field/field_comm/intrude_field.h"
#include "field\field_comm\intrude_work.h"
#include "field/field_comm/intrude_mission_field.h"

#include "gamesystem/link_festival.h"

#include "message.naix"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_msg.h"

#include "event_intrude.h"
#include "event_intrude_subscreen.h"
#include "fieldmap/zone_id.h"
#include "field/zonedata.h"


//==================================================================
/**
 * フェスミッション　指定イベント起動時スコアアップ監視
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdFMissionEventClearCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  
  u16 ev_id = SCRCMD_GetVMWorkValue( core, wk );

  GAMEBEACON_Set_FMissionEventClear( ev_id );

  return VMCMD_RESULT_SUSPEND;
}

