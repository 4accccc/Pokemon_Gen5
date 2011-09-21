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

//デバッグ
#include "debug/debug_flg.h"
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
//--------------------------------------------------------------
/**
 * @brief 人物状態を取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPersonState_GetState( VMHANDLE *p_vm, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  u16 person    = VMGetU16( p_vm );
  u16 *p_ret_wk = SCRCMD_GetVMWork( p_vm, p_wk_adrs );

  PERSON_STATE *p_person_state  = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );

  *p_ret_wk = PERSON_STATE_GetState( p_person_state, person );

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

  PERSON_STATE_SetLocalTvtCall( p_person_state, person, TRUE );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 * @brief 隠しキャラから電話がかかってくるのが終了したかどうか
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPersonState_CheckSecretCallEnd( VMHANDLE *p_vm, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  u16 *p_ret_wk = SCRCMD_GetVMWork( p_vm, p_wk_adrs );
  u16 place    = VMGetU16( p_vm );

  PERSON_STATE *p_person_state  = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );

  int i;
  int cnt = 0;
  for( i = 0; i < PERSON_STATE_SECRET_CALLPLACE_MAX; i++ )
  {
    if( PERSON_STATE_GetCallPlaceFlag( p_person_state, i ) )
    {
      cnt++;
    }
  }

  *p_ret_wk = cnt >= 10;

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 * @brief 隠しキャラとのデートがあるかチェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPersonState_GetDateType( VMHANDLE *p_vm, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  u16 *p_ret_wk = SCRCMD_GetVMWork( p_vm, p_wk_adrs );

  PERSON_STATE *p_person_state  = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );

  *p_ret_wk = PERSON_STATE_GetSecretDateEvent( p_person_state );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 * @brief 隠しキャラとのデート終了
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPersonState_SetDateEnd( VMHANDLE *p_vm, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  PERSON_STATE  *p_person_state  = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );

  PERSON_STATE_SetDateEndFlag( p_person_state, TRUE );
  PERSON_STATE_ClearSecretDateEvent( p_person_state );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 * @brief 隠しキャラとの交換をおこなった
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPersonState_SetTrade( VMHANDLE *p_vm, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  PERSON_STATE  *p_person_state  = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(p_gamedata) );
  u16 monsno = SCRCMD_GetVMWorkValue( p_vm, p_wk_adrs );

  PERSON_STATE_SetTradeFlag( p_person_state, TRUE );
  PERSON_STATE_SetTradeMonsno( p_person_state, monsno );

  return VMCMD_RESULT_CONTINUE;
}
