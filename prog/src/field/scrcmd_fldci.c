//======================================================================
/**
 * @file  scrcmd_fldci.c
 * @brief  フィールドカットインスクリプトコマンド用関数
 * @author  Nozomu Satio
 *
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

#include "scrcmd_fldci.h"
#include "fld3d_ci.h"

//--------------------------------------------------------------
/**
 * フィールドカットイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFldCi_CallCutin( VMHANDLE *core, void *wk )
{
  u16 cutin_no;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(fieldWork);

  cutin_no = VMGetU16( core );

  //カットインイベントをコール
  {
    GMEVENT *call_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    call_event = FLD3D_CI_CreateCutInEvt(gsys, ptr, cutin_no);
    SCRIPT_CallEvent( sc, call_event );
  }
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ポケモンカットイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFldCi_CallPokeCutin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*    work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(fieldWork);

  u16 pos = SCRCMD_GetVMWorkValue( core, work );
  
  //カットインイベントをコール
  {
    GMEVENT *call_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    call_event = FLD3D_CI_CreatePokeCutInEvtTemoti( gsys, ptr, pos );
    SCRIPT_CallEvent( sc, call_event );
  }
  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * NPC空を飛ぶカットイン
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFldCi_CallNpcFlyCutin( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*    work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(fieldWork);

  u16 obj_id = SCRCMD_GetVMWorkValue( core, work );
  
  //カットインイベントをコール
  {
    GMEVENT *call_event;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    call_event = FLD3D_CI_CreateNpcFlyCutInEvt( gsys, ptr, obj_id );
    SCRIPT_CallEvent( sc, call_event );
  }
  
  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}


