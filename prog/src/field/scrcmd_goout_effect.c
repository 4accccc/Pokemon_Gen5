//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_goout_effect.c
 *	@brief
 *	@author		Toru=Nagihashi
 *	@date		2011.10.27
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

//外部公開
#include "scrcmd_goout_effect.h"


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  GooutEffectの演出表示リクエスト
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs  スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGooutEffect_Request( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );

  GAMEDATA_SetGooutEffectRequest( p_gamedata, TRUE );

  return VMCMD_RESULT_CONTINUE;
}
