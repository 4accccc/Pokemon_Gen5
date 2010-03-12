//======================================================================
/**
 * @file  scrcmd_startmenu.c
 * @brief  スタートメニュー関連
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

#include "scrcmd_startmenu.h"

//--------------------------------------------------------------
/**
 * スタートメニュー項目開示フラグ取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetStartMenuFlg( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work     = (SCRCMD_WORK*)wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( wk );
  
  u16 *type = SCRCMD_GetVMWorkValue( core, work );
  u16 *flg = SCRCMD_GetVMWork( core, work );
  {
    MISC *misc = GAMEDATA_GetMiscWork(gdata);
    (*flg) = MISC_GetStartMenuFlag( misc, type );
  }

  return VMCMD_RESULT_CONTINUE;
}

