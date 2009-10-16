//======================================================================
/**
 * @file  scrcmd_gym_init.c
 * @brief  スクリプトコマンド：ジム初期化関連
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

FS_EXTERN_OVERLAY(field_gym_init);


//--電気--
//--------------------------------------------------------------
/**
 * 電気ジム初期化
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymElec_Init( VMHANDLE *core, void *wk )
{
  u16 evt1,evt2;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  evt1 = VMGetU16( core );
  evt2 = VMGetU16( core );

  GFL_OVERLAY_Load( FS_OVERLAY_ID(field_gym_init) );		//オーバーレイロード
  GYM_INIT_Elec(gsys, evt1, evt2);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(field_gym_init));		//オーバーレイアンロード

  return VMCMD_RESULT_CONTINUE;
}

//--ノーマル--
//--------------------------------------------------------------
/**
 * ノーマルジム初期化
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymNormal_Init( VMHANDLE *core, void *wk )
{
  u8 room_no;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  room_no = VMGetU16( core );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(field_gym_init) );		//オーバーレイロード
  GYM_INIT_Normal(gsys);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(field_gym_init));		//オーバーレイアンロード

  return VMCMD_RESULT_CONTINUE;
}

//--アンチ--
//--------------------------------------------------------------
/**
 * アンチジム初期化
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGymAnti_Init( VMHANDLE *core, void *wk )
{
  GFL_OVERLAY_Load( FS_OVERLAY_ID(field_gym_init) );		//オーバーレイロード
//  GYM_INIT_Normal(gsys, room_no);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(field_gym_init));		//オーバーレイアンロード

  return VMCMD_RESULT_CONTINUE;
}
