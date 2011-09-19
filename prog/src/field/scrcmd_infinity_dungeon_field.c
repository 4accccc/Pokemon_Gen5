//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_infinity_dungeon_field.c
 *	@brief  無限ダンジョン常駐スクリプト
 *	@author	Masayuki Onoue
 *	@date		2011.09.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"


#include "savedata/infinity_dungeon_save.h"
#include "scrcmd_infinity_dungeon_field.h"

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンのステータス取得
 *
 *	@param	*core
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetStatus( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
   
  // WFBCの状態を調べる
  FIELD_WFBC_CORE_TYPE wfbc_type = GAMEDATA_GetMyWFBCType( p_gamedata );

  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  *p_ret_wk = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_save ,wfbc_type );

  return VMCMD_RESULT_CONTINUE;
}

