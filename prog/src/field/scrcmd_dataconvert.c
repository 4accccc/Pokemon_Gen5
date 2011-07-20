//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_dataconvert.c
 *	@brief  データコンバート用スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@data		2011.07.20
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
#include "event_fieldmap_control.h"

//外部公開
#include "scrcmd_dataconvert.h"


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
 *	@brief  フラグを取得
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdDataConv_GetFlag( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  OUTSIDE_PERMANENT *p_per = SaveControl_GetOutsidePermanent( p_sv );
  DATA_CONVERT_SAVE *p_dataconv = OUTSIDE_PERMANENT_GetDataConvert( p_per );
  u16 flag    = SCRCMD_GetVMWorkValue( p_core, p_wk_adrs );
  u16 *p_ret  = SCRCMD_GetVMWork( p_core, p_scrcmd );
  *p_ret      = DATA_CONVERT_SAVE_GetFlag( p_dataconv, flag );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワークを取得
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdDataConv_GetWork( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  OUTSIDE_PERMANENT *p_per = SaveControl_GetOutsidePermanent( p_sv );
  DATA_CONVERT_SAVE *p_dataconv = OUTSIDE_PERMANENT_GetDataConvert( p_per );
  u16 wk    = SCRCMD_GetVMWorkValue( p_core, p_wk_adrs );
  u16 *p_ret  = SCRCMD_GetVMWork( p_core, p_scrcmd );
  *p_ret      = DATA_CONVERT_SAVE_GetWork( p_dataconv, wk );

  return VMCMD_RESULT_CONTINUE;
}
