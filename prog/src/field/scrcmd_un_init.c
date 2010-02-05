//======================================================================
/**
 * @file  scrcmd_un_init.c
 * @brief  スクリプトコマンド：国連関連
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

#include "savedata/wifihistory.h"
#include "scrcmd_un.h"
#include "united_nations.h" 

//--------------------------------------------------------------
/**
 * 部屋のOBJセットアップのためにOBJCODEを返す
 * ゾーンチェンジ時に呼ぶので常駐に置く
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_GetRoomObjCode( VMHANDLE *core, void *wk )
{
  u8 obj_idx;
  u16 *ret;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  UNSV_WORK *unsv_work = GAMEDATA_GetUnsvWorkPtr(gdata);

  obj_idx = VMGetU16( core ); //0～4
  ret = SCRCMD_GetVMWork( core, work );

  *ret = UN_GetRoomObjCode(unsv_work, obj_idx);

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 部屋の人数を取得
 * ゾーンチェンジ時に呼ぶので常駐に置く
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdUn_GetRoomObjNum( VMHANDLE *core, void *wk )
{
  u16 *obj_num;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata =  GAMESYSTEM_GetGameData(gsys);
  UNSV_WORK *unsv_work = GAMEDATA_GetUnsvWorkPtr(gdata);

  obj_num = SCRCMD_GetVMWork( core, work );
  *obj_num = UN_GetRoomObjNum(unsv_work);

  return VMCMD_RESULT_CONTINUE;
}
