//======================================================================
/**
 * @file  scrcmd_record.c
 * @brief  レコード加算スクリプトコマンド用関数
 * @author  Nozomu Satio
 *
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_record.h"

//--------------------------------------------------------------
/**
 * レコード加算
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdAddRecord( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  u16 val = SCRCMD_GetVMWorkValue( core, work );

  {
    RECORD * rec;
    SAVE_CONTROL_WORK *sv;
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    sv = GAMEDATA_GetSaveControlWork(gamedata);
    rec = SaveData_GetRecord(sv);

    RECORD_Add(rec, id, val);
  }

  return VMCMD_RESULT_CONTINUE;
}

