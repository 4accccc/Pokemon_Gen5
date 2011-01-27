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
//----------------------------------------------------------------------------
/**
 * @brief ローカルライブキャスターの呼び出し設定をONに
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPersonState_SetLTvtCall( VMHANDLE *p_vm, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  u16 person    = VMGetU16( p_vm );

  PERSON_STATE *p_person_state  = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );

  PERSON_STATE_SetLocalTvtCall( p_person_state, person );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 * @brief 隠しキャラから電話が掛かってくるかどうかチェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPersonState_CheckSecretCalled( VMHANDLE *p_vm, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  u16 *p_ret_wk = SCRCMD_GetVMWork( p_vm, p_wk_adrs );

  PERSON_STATE *p_person_state  = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );

  //条件
  //電話可能
  //１日１回だけ
  //デート待ち合わせしていない
  //１０％
  BOOL is_once  = PERSON_STATE_GetSecretOnceDayCallFlag( p_person_state );
  BOOL is_date  = PERSON_STATE_GetSecretDateEvent( p_person_state );
  BOOL is_data  = PERSON_STATE_GetLocalTvtCall( p_person_state, LOCAL_TVT_PERSON_SECRET );
  BOOL is_rnd  = GFUser_GetPublicRand(100) < 10 ? TRUE: FALSE;
  *p_ret_wk = !is_once && !is_date && is_data && is_rnd;

#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    *p_ret_wk = TRUE;
  }
#endif

  return VMCMD_RESULT_CONTINUE;
}
