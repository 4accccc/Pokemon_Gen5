//======================================================================
/**
 * @file  scrcmd_t01_gmk.c
 * @brief  スクリプトコマンド：Ｔ01ギミック
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

#include "scrcmd_t01_gmk.h"
#include "field_gimmick_t01.h"

//--------------------------------------------------------------
/**
 * ギミック発動トリガーセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdT01Gmk_Start( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  T01_GIMMICK_Start( gsys );

  return VMCMD_RESULT_CONTINUE;
}


