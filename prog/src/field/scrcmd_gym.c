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
#include "gym_ice.h"
#include "gym_dragon.h"

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
#if 0
//--------------------------------------------------------------
/**
 * 虫ジムギミック　
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymInsect_( VMHANDLE *core, void *wk )
{
  u16 *sw_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  return VMCMD_RESULT_SUSPEND;
}
#endif

//--地面--
#if 0
//--------------------------------------------------------------
/**
 * 地面ジムギミック　リフト移動
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymGround_( VMHANDLE *core, void *wk )
{
  u16 lift_idx;
  GMEVENT *call_event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  return VMCMD_RESULT_SUSPEND;
}
#endif

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

