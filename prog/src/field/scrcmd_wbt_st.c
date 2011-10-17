//============================================================================================
/**
 * @file  scrcmd_wbt_st.c
 * @author  tamada GAMEFREAK inc.
 * @date  2011.10.17
 * @brief WBT用スクリプトコマンド実装（常駐用）
 */
//============================================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"


#include "script.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "script_local.h" //SCRIPT_GetEvent

#include "scrcmd_wbt.h"
//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：対戦済みフラグを取得する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWBTGetTrainerFlag( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
  u16 flag_id  = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  *ret_wk = WBTSAVE_GetFlag( wbtsave, flag_id );
  TAMADA_Printf("EvCmdWBTGetTrainerFlag(%d) = %d\n", flag_id, *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

