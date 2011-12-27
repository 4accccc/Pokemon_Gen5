//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_rndmatch.c
 *	@brief  ランダムマッチのスクリプト
 *	@author	Toru=Nagihashi
 *	@date		2011.11.01
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
#include "savedata/battlematch_savedata.h"
#include "savedata/bsubway_savedata.h"
#include "savedata/record.h"

//外部公開
#include "scrcmd_rndmatch.h"

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
 *	@brief  データを取得
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs        ワークアドレス
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdRndMatchGetData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  const BATTLEMATCH_DATA *cp_match_sv = SaveData_GetBattleMatchConst( p_sv );
  u16 param = SCRCMD_GetVMWorkValue( p_core, p_scrcmd );
  u16 *p_ret = SCRCMD_GetVMWork( p_core, p_scrcmd );

  switch( param )
  {
  case RNDMATCH_PARAM_POWER_OFF:
    *p_ret = BATTLEMATCH_IsPowerOff( cp_match_sv );
    break;
  case RNDMATCH_PARAM_SAME_DS:
    *p_ret = BATTLEMATCH_IsSameDS( cp_match_sv );
    break;
  case RNDMATCH_PARAM_PENALTY:
    *p_ret = BATTLEMATCH_IsEndPenaltyTime( cp_match_sv );
    break;
  case RNDMATCH_PARAM_BP:
    *p_ret = GAMEDATA_GetRndMaptchBp( p_gamedata );
    GAMEDATA_ResetRndMaptchBp( p_gamedata );
    break;
  case RNDMATCH_PARAM_ENTRY:
    *p_ret = BATTLEMATCH_GetRndmatch( cp_match_sv );
    break;
  default:
    GF_ASSERT(0);
  }
  NAGI_Printf( "RndMatchGet param%d ret=%d\n", param, *p_ret );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  チェック開始
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs        ワークアドレス
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdRndMatchStartCheck( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  BATTLEMATCH_DATA *p_match_sv = SaveData_GetBattleMatch( p_sv );

  BATTLEMATCH_StartPowerOffCheck( p_match_sv );
  GAMEDATA_ResetRndMaptchBp( p_gamedata );

  NAGI_Printf( "RndMatchStartCheck \n" );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  チェック終了
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs        ワークアドレス
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdRndMatchEndCheck( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  BATTLEMATCH_DATA *p_match_sv = SaveData_GetBattleMatch( p_sv );

  BATTLEMATCH_EndPowerOffCheck( p_match_sv );

  NAGI_Printf( "RndMatchEndCheck \n" );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを設定
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs        ワークアドレス
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdRndMatchSetData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  BATTLEMATCH_DATA *p_match_sv = SaveData_GetBattleMatch( p_sv );
  u16 param = SCRCMD_GetVMWorkValue( p_core, p_scrcmd );
  u16 value = SCRCMD_GetVMWorkValue( p_core, p_scrcmd );

  switch( param )
  {
  case RNDMATCH_PARAM_ENTRY:
    BATTLEMATCH_SetRndmatch( p_match_sv, value );
    break;
  case RNDMATCH_PARAM_POWER_OFF_CNT:
    RECORD_Inc( SaveData_GetRecord(p_sv), RECID_RNDMATCH_POWEROFF );
    break;
  default:
    GF_ASSERT(0);
  }
  NAGI_Printf( "RndMatchSetData param%d value%d\n", param, value );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ADD_BP
 *
 *	@param	VMHANDLE *p_core  VMハンドル
 *	@param	*p_wk_adrs        ワークアドレス
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdRndMatchAddBp( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_scrcmd = p_wk_adrs;
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_scrcmd );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_scrcmd );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  BATTLEMATCH_DATA *p_match_sv = SaveData_GetBattleMatch( p_sv );
  u16 param = SCRCMD_GetVMWorkValue( p_core, p_scrcmd );
  BSUBWAY_SCOREDATA *p_bsubway = GAMEDATA_GetBSubwayScoreData( p_gamedata );

  BSUBWAY_SCOREDATA_AddBattlePoint( p_bsubway, param );

  // レコード加算(勝ち取ったBP)
  if( param > 0 ){
    RECORD * record_sv =
      SaveData_GetRecord( GAMEDATA_GetSaveControlWork(p_gamedata) );
    RECORD_Add( record_sv ,RECID_WIN_BP ,param );
  }

  NAGI_Printf( "RndMatchAdBp param%d", param );

  return VMCMD_RESULT_CONTINUE;
}
