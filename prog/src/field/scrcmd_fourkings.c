//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_fourkings.c
 *	@brief  四天王スクリプトコマンド
 *	@author	tomoya takahashi
 *	@date		2009.11.25
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"

#include "scrcmd_fourkings.h"

#include "event_fourkings.h"

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


//======================================================================
//  四天王歩きイベント
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  四天王歩きイベント完了待ち
 *
 *	@param	core
 *	@param	*wk 
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_WalkEvent( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  u16          index = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数

  // イベントを呼び出す
  SCRIPT_CallEvent( scw, EVENT_FourKings_CircleWalk(gsys, fieldWork, index) );
  return VMCMD_RESULT_SUSPEND;
}


