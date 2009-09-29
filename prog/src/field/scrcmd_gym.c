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

#include "gym_init.h"
#include "gym_elec.h"
#include "gym_normal.h"


//--電気--
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

//--ノーマル--
//--------------------------------------------------------------
/**
 * ノーマルジムギミックロック解除
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymNormal_Unrock( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  GYM_NORMAL_Unrock(fieldWork);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ノーマルジムギミックロック解除チェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymNormal_CheckUnrock( VMHANDLE *core, void *wk )
{
  u16 *ret;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  ret = SCRCMD_GetVMWork( core, work );

  if ( GYM_NORMAL_CheckRock(fieldWork) ){
    *ret = 1;
  }else{
    *ret = 0;
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ノーマルジムギミック本棚移動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymNormal_MoveWall( VMHANDLE *core, void *wk )
{
  u8 wall_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  wall_idx = VMGetU16( core );

  call_event = GYM_NORMAL_MoveWall(gsys, wall_idx);

  SCRIPT_EntryNextEvent( sc, call_event );
  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;

}

//--アンチ--
//--------------------------------------------------------------
/**
 * アンチジムギミック　床スイッチオン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymAnti_SwOn( VMHANDLE *core, void *wk )
{
  u8 sw_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  sw_idx = VMGetU16( core );
/**
  call_event = GYM_NORMAL_MoveWall(gsys, sw_idx);

  SCRIPT_EntryNextEvent( sc, call_event );
*/  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * アンチジムギミック　ドアオープン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymAnti_OpenDoor( VMHANDLE *core, void *wk )
{
  u8 door_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  door_idx = VMGetU16( core );
/**
  call_event = GYM_NORMAL_MoveWall(gsys, door_idx);

  SCRIPT_EntryNextEvent( sc, call_event );
*/  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;

}


