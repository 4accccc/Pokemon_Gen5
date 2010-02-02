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

  GF_ASSERT( index > 0 );

  // ZONE ID の IDでわたってくる（1オリジン）ので、-1する
  index -= 1;

  // イベントを呼び出す
  SCRIPT_CallEvent( scw, EVENT_FourKings_CircleWalk(gsys, fieldWork, index) );
  return VMCMD_RESULT_SUSPEND;
}




//----------------------------------------------------------------------------
/**
 *	@brief  四天王頂上カメラの指定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_SetCameraParam( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  u16          index = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( fieldWork );

  GF_ASSERT( index > 0 );

  // ZONE ID の IDでわたってくる（1オリジン）ので、-1する
  index -= 1;


  // カメラの状態を頂上にする
  FIELD_CAMERA_SetCameraType( p_camera, EVENT_FourKings_GetCameraID( index ) );

  return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ゴースト雷音　設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_SetGhostSparkSound( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );

  // 雷音ON
  FIELDMAPFUNC_FourkingsGhostSparkSound( fieldWork );

  return VMCMD_RESULT_CONTINUE;
}



