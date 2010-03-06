//======================================================================
/**
 * @file scr_pdw.c
 * @brief スクリプトコマンド　PDW関連
 * @author	Ariizumi Nobuhiko
 * @date	10/03/06
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "sound/pm_sndsys.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "savedata/save_control.h"

#include "message.naix"
#include "msg/script/msg_pdw_scr.h"
#include "scrcmd_pdw.h"

#include "../../../resource/fldmapdata/script/pdw_scr_local.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// EVENT_MUSICAL_WORK
//--------------------------------------------------------------

//======================================================================
//  proto
//======================================================================

VMCMD_RESULT EvCmdPDW_CommonTools( VMHANDLE *core, void *wk );
VMCMD_RESULT EvCmdPDW_FurnitureTools( VMHANDLE *core, void *wk );
VMCMD_RESULT EvCmdPDW_SetFurnitureWord( VMHANDLE *core, void *wk );


//--------------------------------------------------------------
/**
 *  _PDW_COMMON_TOOLS PDW汎用ツール
 *  @param type  コマンド種類
 *  @param val1  取得用番号
 *  @param val2  取得用番号
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPDW_CommonTools( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );

  u16  type = SCRCMD_GetVMWorkValue( core, work );
  u16  val1 = SCRCMD_GetVMWorkValue( core, work );
  u16  val2 = SCRCMD_GetVMWorkValue( core, work );
  u16* ret_wk = SCRCMD_GetVMWork( core, work );
  
}


//--------------------------------------------------------------
/**
 *  _PDW_FURNITURE_TOOLS PDW家具メニュー：汎用ツール
 *  @param val1  取得用番号
 *  @param val2  取得用番号
 *  @param ret_val  戻り値
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPDW_FurnitureTools( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );

  u16  type = SCRCMD_GetVMWorkValue( core, work );
  u16  val1 = SCRCMD_GetVMWorkValue( core, work );
  u16  val2 = SCRCMD_GetVMWorkValue( core, work );
  u16* ret_wk = SCRCMD_GetVMWork( core, work );
  
}


//--------------------------------------------------------------
/**
 *  _PDW_SET_FURNITURE_TOOLS PDW家具メニュー：家具名セット
 * @param idx セットするタグナンバー
 * @param val セットする家具番号
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPDW_SetFurnitureWord( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );

  u16  idx = SCRCMD_GetVMWorkValue( core, work );
  u16  val = SCRCMD_GetVMWorkValue( core, work );
  
}
