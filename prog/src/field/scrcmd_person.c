//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_person.c
 *	@brief  人物状態
 *	@author	Toru=Nagihashi
 *	@data		2011.01.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/main.h"
#include "system/vm_cmd.h"
#include "gamesystem/game_data.h"

#include "script_def.h"
#include "scrcmd.h"
#include "script.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "savedata/person_state.h"

#include "scrcmd_person.h"
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
//--------------------------------------------------------------
/**
 * @brief 人物状態を設定
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPersonState_SetState( VMHANDLE *p_vm, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  u16 person    = VMGetU16( p_vm );
  u16 state     = VMGetU16( p_vm );

  PERSON_STATE *p_person_state  = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );

  PERSON_STATE_SetState( p_person_state, person, state );

  return VMCMD_RESULT_CONTINUE;
}
