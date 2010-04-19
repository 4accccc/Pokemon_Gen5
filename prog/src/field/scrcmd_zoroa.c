//======================================================================
/**
 * @file  scrcmd_zoroa.c
 * @brief  ゾロアスクリプト
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"
#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_zoroa.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "sp_poke_gimmick.h"

//--------------------------------------------------------------
/**
 * ボールアニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdChangeZoroa( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  VecFx32 pos;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  u16 before = SCRCMD_GetVMWorkValue( core, work );
  u16 after = SCRCMD_GetVMWorkValue( core, work );

  //開始地点は対象位置
  {
    MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
    MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, before );
    MMDL_GetVectorPos( mmdl, &pos );
  }

  event = SPPOKE_GMK_SmokeAnm( gsys, &pos, before, after);

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
