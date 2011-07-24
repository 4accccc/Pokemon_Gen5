//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_walk_count.c
 *	@brief  歩数カウントスクリプトコマンド
 *	@author	Masayuki Onoue
 *	@date		2011.07.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd.h"
#include "savedata/situation.h"
#include "scrcmd_walk_count.h"

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
 *	@brief  歩数カウントスクリプト起動
 *
 *	@param	*core
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWalkCountStart( VMHANDLE *core, void *wk )
{
  SITUATION         *p_situation;
  SAVE_CONTROL_WORK *p_saveData;
  SCRCMD_WORK   *p_wk     = wk;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gsys );

  // 状況データ取得
  p_saveData  = GAMEDATA_GetSaveControlWork( p_gamedata );
  p_situation = SaveData_GetSituation( p_saveData );

  // 歩数カウンタ起動
  Situation_ScWalkCoutStart( p_situation );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  歩数カウントスクリプト終了
 *
 *	@param	*core
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWalkCountStop( VMHANDLE *core, void *wk )
{
  SITUATION         *p_situation;
  SAVE_CONTROL_WORK *p_saveData;
  SCRCMD_WORK   *p_wk     = wk;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gsys );

  // 状況データ取得
  p_saveData  = GAMEDATA_GetSaveControlWork( p_gamedata );
  p_situation = SaveData_GetSituation( p_saveData );

  // 歩数カウンタ終了
  Situation_ScWalkCoutStop( p_situation );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  歩数カウント取得
 *
 *	@param	*core
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetWalkCount( VMHANDLE *core, void *wk )
{
  SITUATION         *p_situation;
  SAVE_CONTROL_WORK *p_saveData;
  SCRCMD_WORK   *p_wk     = wk;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gsys );
  u16           *p_ret_wk	  = SCRCMD_GetVMWork( core, p_wk );

  // 状況データ取得
  p_saveData  = GAMEDATA_GetSaveControlWork( p_gamedata );
  p_situation = SaveData_GetSituation( p_saveData );

  // 歩数カウンタ起動
  (*p_ret_wk) = Situation_GetScWalkCout( p_situation );

  return VMCMD_RESULT_CONTINUE;
}

