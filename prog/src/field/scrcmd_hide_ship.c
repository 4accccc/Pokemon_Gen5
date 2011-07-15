//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_hide_ship.c
 *	@brief  アジト船スクリプトコマンド
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.14
 *
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

#include "scrcmd_hide_ship.h"

#include "field_gimmick_hide_ship_engine.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン押下
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdHideShip_Button( VMHANDLE *core, void *wk )
{
  int id;
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // ボタンID
  id  = VMGetU16( core );

  event = HIDE_SHIP_Button( gsys, id );

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゲートの状態セット
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdHideShip_GateSet( VMHANDLE *core, void *wk )
{
  int id;
  int state;
  BOOL  flag;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // ゲートID
  id  = VMGetU16( core );
  // 解除しているか
  state  = SCRCMD_GetVMWorkValue( core, work );
  if( state == 1 ){ flag = FALSE; }   // 解除した
  else            { flag = TRUE;  }   // 解除していない

  HIDE_SHIP_GateSet( gsys, id, flag );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゲート解除アニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdHideShip_GateOff( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // ゲートID
  id  = VMGetU16( core );

  HIDE_SHIP_GateOff( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================


