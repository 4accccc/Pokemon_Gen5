//============================================================================================
/**
 *	@file		scrcmd_pokewood.c
 *	@brief  ポケウッドスクリプトコマンド
 *	@author	tamada  GAME FREAK inc.
 *  @date   2011.06.30
 *
 */
//============================================================================================

#include "gflib.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"


#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"

#include "fieldmap_func.h"

#include "scrcmd_pokewood.h"
#include "event_pokewood.h"


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド録画
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodRec( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMAPFUNC_SYS* p_funcsys = FIELDMAP_GetFldmapFuncSys( p_fieldmap );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodShow( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド台本画面呼び出し
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodBook( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  

  return VMCMD_RESULT_CONTINUE;
}



