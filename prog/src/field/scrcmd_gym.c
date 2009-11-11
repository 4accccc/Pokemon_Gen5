//======================================================================
/**
 * @file  scrcmd_gym.c
 * @brief  スクリプトコマンド：ジム関連
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
#include "gym_anti.h"
#include "gym_insect.h"
#include "gym_ground.h"

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
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );
  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 電気ジムカプセルトレーナー処理ご処理
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymElec_SetTrEncFlg( VMHANDLE *core, void *wk )
{
  u8 cap_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  cap_idx = VMGetU16( core );
  GYM_ELEC_SetTrEncFlg(gsys, cap_idx);

  return VMCMD_RESULT_CONTINUE;
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
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );
  
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
  call_event = GYM_ANTI_SwOnOff(gsys, sw_idx, 1);
  
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * アンチジムギミック　床スイッチオフ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymAnti_SwOff( VMHANDLE *core, void *wk )
{
  u8 sw_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  sw_idx = VMGetU16( core );
  call_event = GYM_ANTI_SwOnOff(gsys, sw_idx, 0);
  
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );  
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
  call_event = GYM_ANTI_OpenDoor(gsys, door_idx);
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;

}

//--虫--
//--------------------------------------------------------------
/**
 * 虫ジムギミック　床スイッチオン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymInsect_SwOn( VMHANDLE *core, void *wk )
{
  u16 *sw_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  sw_idx = SCRCMD_GetVMWork( core, work );
  call_event = GYM_INSECT_CreateSwitchEvt(gsys, *sw_idx);
  
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 虫ジムギミック　ポールアニメ起動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymInsect_PoleOn( VMHANDLE *core, void *wk )
{
  u16 *pl_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  pl_idx = SCRCMD_GetVMWork( core, work );
  call_event = GYM_INSECT_CreatePoleEvt(gsys, *pl_idx);
  
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 虫ジムギミック　トレーナートラップ起動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymInsect_TrTrapOn( VMHANDLE *core, void *wk )
{
  u16 tr_evt_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  tr_evt_idx = VMGetU16( core );

  GF_ASSERT( tr_evt_idx < 2 );
  call_event = GYM_INSECT_CreateTrTrapEvt(gsys, tr_evt_idx);
  
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--地面--
//--------------------------------------------------------------
/**
 * 地面ジムギミック　リフト移動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymGround_MvLift( VMHANDLE *core, void *wk )
{
  u16 lift_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  lift_idx = VMGetU16( core );

  call_event = GYM_GROUND_CreateLiftMoveEvt(gsys, lift_idx);
  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}
