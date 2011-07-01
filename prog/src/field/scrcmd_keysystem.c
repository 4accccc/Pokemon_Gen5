//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_keysystem.h
 *	@brief  キーシステム用スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@data		2011.06.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
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
#include "savedata/outside_permanent.h"
#include "net_app/key_system.h"
#include "event_fieldmap_control.h"

//外部公開
#include "scrcmd_keysystem.h"

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
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  WFBCの種類を取得
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdKeySys_GetWfbc( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );

  u16 *p_ret = SCRCMD_GetVMWork( p_core, p_scrcmd );

  *p_ret = GAMEDATA_GetMyWFBCType( p_gamedata );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  難易度を取得
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdKeySys_GetDifficulty( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  OUTSIDE_PERMANENT *p_per = SaveControl_GetOutsidePermanent( p_sv );
  KEY_SYSTEM_SAVE *p_key = OUTSIDE_PERMANENT_GetKeySystem( p_per );
  u16 *p_ret = SCRCMD_GetVMWork( p_core, p_scrcmd );
  *p_ret = KEY_SYSTEM_SAVE_GetDifficult( p_key );

  return VMCMD_RESULT_CONTINUE;
}

static void EvCmdKeySys_CallKeyGetDemo_Callback( void* p_wk_adrs )
{
  GFL_HEAP_FreeMemory( p_wk_adrs );
}

//--------------------------------------------------------------
/**
 * @brief   キーシステムキー取得部分を呼び出します
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdKeySys_CallKeyGetDemo( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  u16 get_key = SCRCMD_GetVMWorkValue( p_core, p_wk_adrs );

  GMEVENT *p_event;
  KEY_SYSTEM_PARAM *p_param = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(KEY_SYSTEM_PARAM) );
  p_param->p_gamedata = p_gamedata;
  p_param->mode       = KEY_SYSTEM_MODE_GET;
  p_param->get_key    = get_key;

  p_event = EVENT_FieldSubProc_Callback( p_gamesys, p_fieldmap,
                                       FS_OVERLAY_ID(key_system), &KEY_SYSTEM_ProcData, p_param,
                                       EvCmdKeySys_CallKeyGetDemo_Callback, p_param );
  SCRIPT_CallEvent( p_sc, p_event );

  return VMCMD_RESULT_SUSPEND;
}


