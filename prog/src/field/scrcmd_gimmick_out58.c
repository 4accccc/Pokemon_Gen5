//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_gimmick_out58.c
 *	@brief  スクリプトコマンド  C14ギミック
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.11.12
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"

// mine
#include "scrcmd_gimmick_out58.h"
#include "field_gimmick_c14.h"

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
 *	@brief  水しぶき再生コマンド
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdC14GMK_Splash( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  int gx, gy, gz;

  gx = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  gy = SCRCMD_GetVMWorkValue( core, work );  // コマンド第2引数
  gz = SCRCMD_GetVMWorkValue( core, work );  // コマンド第3引数

  // 水しぶきアニメ再生
  C14_GIMMICK_AnmStart( gsys, gx, gy, gz );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  水からジャンプコマンド
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdC14GMK_ObjWaterJump( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT *event;

  u16 obj_id;
  int gx, gz;
  u16 dir;

  obj_id  = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  gx      = SCRCMD_GetVMWorkValue( core, work );  // コマンド第2引数
  gz      = SCRCMD_GetVMWorkValue( core, work );  // コマンド第3引数
  dir     = SCRCMD_GetVMWorkValue( core, work );  // コマンド第4引数

  // 水ジャンプイベント作成
  event = C14_GIMMICK_ObjWaterJump( gsys, obj_id, gx, gz, dir );

  if( event == NULL ) {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
//=============================================================================
/**
 *					static関数
*/
//=============================================================================


