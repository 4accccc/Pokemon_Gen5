//======================================================================
/**
 * @file  scrcmd_gym.c
 * @bfief  スクリプトコマンド：ジム関連
 * @author  Saito
 */
//======================================================================
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

#include "event_fieldmap_control.h"

#include "field_sound.h"

#include "scrcmd_gym.h"

#include "gym_elec.h"
#include "gym_init.h"

//--------------------------------------------------------------
/**
 * 電気ジム初期化
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymElec_Init( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  GYM_INIT_Elec(fieldWork);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 電気ジムスイッチ押下
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymElec_PushSw( VMHANDLE *core, void *wk )
{
  u8 sw_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  sw_idx = VMGetU16( core );

  call_event = GYM_ELEC_ChangePoint(gsys, sw_idx);

  SCRIPT_EntryNextEvent( sc, call_event );
  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}


