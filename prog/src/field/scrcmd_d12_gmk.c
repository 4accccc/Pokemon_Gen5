//======================================================================
/**
 * @file  scrcmd_d12_gmk.c
 * @brief  スクリプトコマンド：D12ギミック
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

#include "scrcmd_d12_gmk.h"
#include "field_gimmick_d12.h"

FS_EXTERN_OVERLAY(fld_faceup);

static GMEVENT_RESULT EndEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * 顔アップ開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdD12Gmk_Vanish( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  D12_GIMMICK_Vanish( gsys );

  return VMCMD_RESULT_CONTINUE;
}


