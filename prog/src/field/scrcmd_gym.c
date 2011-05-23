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
#include "gym_elec_sv.h"
#include "gym_normal.h"
#include "gym_anti.h"
#include "gym_insect.h"
#include "gym_ground.h"
#include "gym_ground_ent.h"
#include "gym_fly.h"
#include "gym_ice.h"
#include "gym_dragon.h"
#include "gym_poison.h"

//--電気--
//--------------------------------------------------------------
/**
 * @brief 電気ジム環境リカバリー
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymElec_Recover( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  GYM_ELEC_Recover( fieldWork );

  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief スポットライトを設定
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymElec_SetSpotLight( VMHANDLE *core, void *wk )
{
  u16 on_off;
  u16 objID;
  u16 type;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  on_off  = VMGetU16( core );
  objID   = VMGetU16( core );
  type    = VMGetU16( core );

  GYM_ELEC_SetSpotLight( fieldWork, on_off, objID, type );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief フェードアウトイベントを開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymElec_CallFadeoutEvent( VMHANDLE *core, void *wk )
{
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( wk );
  u8 phase  = VMGetU16( core );
  GMEVENT *event  = GYM_ELEC_CreateFadeoutEvent( gsys, phase );

  if( event == NULL )
  {
    return VMCMD_RESULT_CONTINUE;
  }
  else
  {
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( wk );
    SCRIPT_CallEvent( sc, event );
    //イベントコールするので、一度制御を返す
    return VMCMD_RESULT_SUSPEND;
  }
}

//--------------------------------------------------------------
/**
 * @brief 観客演出を設定
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymElec_SetEffect( VMHANDLE *core, void *wk )
{
  u16 type;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  type  = VMGetU16( core );

  switch( type )
  {
  case GYM_ELEC_EFFECT_FAN_NORMAL:
    GYM_ELEC_SetFanEffect( fieldWork, GYM_ELEC_FAN_NORMAL );
    break;
  case GYM_ELEC_EFFECT_FAN_TR_END:
    GYM_ELEC_SetFanEffect( fieldWork, GYM_ELEC_FAN_TR_END );
    break;
  case GYM_ELEC_EFFECT_FAN_BOSS_END:
    GYM_ELEC_SetFanEffect( fieldWork, GYM_ELEC_FAN_BOSS_END );
    break;
  case GYM_ELEC_EFFECT_DISPLAY_BOSS:
    GYM_ELEC_SetDisplay( fieldWork, GYM_ELEC_DISPLAY_BOSS );
    break;
  case GYM_ELEC_EFFECT_DISPLAY_HERO:
    GYM_ELEC_SetDisplay( fieldWork, GYM_ELEC_DISPLAY_HERO );
    break;
  case GYM_ELEC_EFFECT_LIGHTUP_BOSS_START:
    GYM_ELEC_SetFloorLight( fieldWork, GYM_ELEC_FLOORLIGHT_BOSS_START );
    break;
  case GYM_ELEC_EFFECT_LIGHTUP_BOSS_RECOVER:
    GYM_ELEC_SetFloorLight( fieldWork, GYM_ELEC_FLOORLIGHT_BOSS_RECOVER );
    break;
  case GYM_ELEC_EFFECT_FLASH_WHITE:
    GYM_ELEC_SetBrightness( fieldWork, 8 );
    break;
  case GYM_ELEC_EFFECT_FLASH_NORMAL:
    GYM_ELEC_SetBrightness( fieldWork, 0 );
    break;
  default:
    GF_ASSERT(0);
  }
  return VMCMD_RESULT_CONTINUE;
}

//--ノーマル--
#if 0
//--------------------------------------------------------------
/**
 * ノーマルジムギミック 
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymNormal_( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  return VMCMD_RESULT_CONTINUE;
}
#endif

//--アンチ--

#if 0
//--------------------------------------------------------------
/**
 * アンチジムギミック 
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymAnti_( VMHANDLE *core, void *wk )
{
  u8 sw_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  return VMCMD_RESULT_SUSPEND;

}
#endif

//--虫--
//--------------------------------------------------------------
/**
 * @brief   虫ジムギミック 繭オープンアニメコール 
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymInsect_CocconOpen( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  u16 idx = SCRCMD_GetVMWorkValue(core,work);
  GYM_INSECT_CocconOpen( fieldWork, idx );

  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief   虫ジムギミック 繭吸い込みアニメコール 
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymInsect_VacuumAnm( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  GYM_INSECT_VacuumAnmTaskAdd( fieldWork );

  return VMCMD_RESULT_CONTINUE;
}

//--地面--
//--------------------------------------------------------------
/**
 * @brief   地面ジムギミック　昇降用リフト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymGround_MvLift( VMHANDLE *core, void *wk )
{
  u16 lift_id;
  u16 way;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT *call_event;

  // リフトのID 0～3
  lift_id  = VMGetU16( core );
  way  = VMGetU16( core );

  call_event = GYM_GROUND_CreateLiftMoveEvent( gsys, lift_id, way );

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
 * @brief   地面ジムギミック　出口リフト移動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymGround_ExitLift( VMHANDLE *core, void *wk )
{
  u16 exit;
  BOOL exit_flg;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  exit = VMGetU16( core );

  if (exit) exit_flg = TRUE;
  else exit_flg = FALSE;

  call_event = GYM_GROUND_CreateExitLiftMoveEvent(gsys, exit_flg);
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
 * @brief   地面ジムエントランスギミック　出口リフト移動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymGroundEnt_ExitLift( VMHANDLE *core, void *wk )
{
  u16 exit;
  BOOL exit_flg;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  exit = VMGetU16( core );

  if (exit) exit_flg = TRUE;
  else exit_flg = FALSE;

  call_event = GYM_GROUND_ENT_CreateExitLiftMoveEvt(gsys, exit_flg);
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
 * @brief   地面ジム 入り口位置にリフトと自機をセット　
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymGround_SetEnterPos( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  GYM_GROUND_SetEnterPos(gsys);
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief   地面ジムエントランス 入り口位置にリフトと自機をセット　
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymGroundEnt_SetEnterPos( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  GYM_GROUND_ENT_SetEnterPos(gsys);
  return VMCMD_RESULT_CONTINUE;
}

//--飛行--
//--------------------------------------------------------------
/**
 * @brief   飛行ジムギミック 吹き飛ばしイベント
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymFly_Blow( VMHANDLE * core, void *wk )
{
  u16 obj_id;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  GMEVENT *call_event;

  // サングラスのOBJID
  obj_id  = VMGetU16( core );

  call_event = GYM_FLY_BlowEvent( gsys, obj_id );

  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--氷--
#if 0
//--------------------------------------------------------------
/**
 * 氷ジムギミック　
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymIce_( VMHANDLE *core, void *wk )
{
  u16 idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  return VMCMD_RESULT_SUSPEND;
}
#endif

//--ドラゴン--
#if 0
//--------------------------------------------------------------
/**
 * ドラゴンジムギミック　
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymDragon_( VMHANDLE *core, void *wk )
{
  u16 head;
  u16 arm;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  head = VMGetU16( core );
  arm = VMGetU16( core );

  call_event = GYM_DRAGON_CreateGmkEvt(gsys, head, arm);

  if (call_event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }

  SCRIPT_CallEvent( sc, call_event );
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}
#endif

//--毒--
//--------------------------------------------------------------
/**
 * @brief   毒ジムギミック スピーカーのアニメSTOP
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymPoison_AnmStop( VMHANDLE * core, void *wk )
{
  u16 idx;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  // ギミックID
  idx  = VMGetU16( core );

  // アニメSTOP
  GYM_POISON_AnmStop( gsys, idx );

  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief   毒ジムギミック ISSOBJのアニメSTOP
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymPoison_IssObjStop( VMHANDLE * core, void *wk )
{
  u16 obj_id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // OBJID
  obj_id  = VMGetU16( core );

  GYM_POISON_IssObjStop( gsys, obj_id );

  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

