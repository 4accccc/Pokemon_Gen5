//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_gimmick_sppoke.c
 *	@brief  スクリプトコマンド  特殊ポケギミック
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.18
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
#include "scrcmd_gimmick_sppoke.h"
#include "field_gimmick_rezi.h"
#include "sp_poke_gimmick.h"

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
 *	@brief  レジギミック  眼設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdRegiGMK_SetEye( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  u16 id;
  id = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数

  // 眼の設定
  REZI_GIMMICK_SetEye( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  レジギミック  ドア状態設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdRegiGMK_SetDoorState( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  u16 is_open;
  is_open = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数

  // ドア状態の設定
  REZI_GIMMICK_SetDoorState( gsys, is_open );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  レジギミック  ドアアニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdRegiGMK_AnmDoorStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  u16 is_open;
  is_open = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数

  // ドアアニメ再生
  REZI_GIMMICK_AnmDoorStart( gsys, is_open );

  return VMCMD_RESULT_CONTINUE;
}

//=============================================================================
/**
 *        【配布】ケルディオ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ケルディオ  思い出しイベント
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdKerudhioGMK_RememberEvent( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT *event;

  // 思い出しイベント作成
  event = KERUDHIO_GIMMICK_Event( gsys );

  if( event == NULL ) {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ケルディオ  エフェクト削除
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdKerudhioGMK_EffectDelete( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  // エフェクト削除
  KERUDHIO_GIMMICK_EffectDelete( gsys );

  return VMCMD_RESULT_CONTINUE;
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================


