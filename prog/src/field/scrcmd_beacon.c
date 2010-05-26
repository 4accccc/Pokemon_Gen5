//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：ビーコン通信/Gパワー関連
 * @file   scrcmd_beacon.c
 * @author iwasawa
 * @date   2010.02.15
 */
//////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/main.h"

#include "gamesystem/game_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_event.h"
#include "gamesystem/g_power.h"

#include "event_gpower.h"

#include "script_def.h"
#include "scrcmd.h"
#include "script.h"
#include "script_local.h"

#include "scrcmd_beacon.h"

//--------------------------------------------------------------------
/**
 * @brief ビーコン送信リクエスト呼び出し 
 *
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT:
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdBeaconSetRequest( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*   work     = (SCRCMD_WORK*)wk;
  u16            beacon_id  = SCRCMD_GetVMWorkValue( core, work );
  u16            value      = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*  gsys     = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SCRIPT_WORK*   sc       = SCRCMD_WORK_GetScriptWork( work );

  switch( beacon_id ){
  case SCR_BEACON_SET_REQ_ITEM_GET: //アイテム拾った
    GAMEBEACON_Set_SpItemGet( value );
    break;
  case SCR_BEACON_SET_REQ_FSKILL_USE: //フィールド技を使った
    GAMEBEACON_Set_FieldSkill( value );
    break;
  case SCR_BEACON_SET_REQ_UNION_COUNTER: //ユニオンカウンター処理開始
    GAMEBEACON_Set_UnionIn();
    break;
  case SCR_BEACON_SET_REQ_TRIAL_HOUSE_START: //トライアルハウス挑戦開始
    GAMEBEACON_Set_TrialHouse();
    break;
  case SCR_BEACON_SET_REQ_TRIAL_HOUSE_RANK: //トライアルハウスランクセット
    GAMEBEACON_Set_TrialHouseRank( value );
    break;
  case SCR_BEACON_SET_REQ_POKE_SHIFTER_START: //ポケシフター挑戦開始
    GAMEBEACON_Set_PokeShifter();
    break;
  case SCR_BEACON_SET_REQ_SUBWAY_START: //サブウェイ挑戦開始
    GAMEBEACON_Set_Subway();
    break;
  case SCR_BEACON_SET_REQ_SUBWAY_WIN: //サブウェイ勝利
    GAMEBEACON_Set_SubwayStraightVictories( value );
    break;
  case SCR_BEACON_SET_REQ_SUBWAY_TROPHY_GET:  //サブウェイトロフィーゲット
    GAMEBEACON_Set_SubwayTrophyGet();
    break;

  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief フィニッシュ待ちのGパワーチェック 
 *
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT:
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdCheckGPowerFinish( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*   work     = (SCRCMD_WORK*)wk;
  u16*           ret_power  = SCRCMD_GetVMWork( core, work );
  u16*           ret_next   = SCRCMD_GetVMWork( core, work );

  *ret_power = GPOWER_Get_FinishWaitID();

  if((*ret_power) != GPOWER_ID_NULL){
    POWER_CONV_DATA * p_data = GPOWER_PowerData_LoadAlloc( GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
    GPOWER_Set_Finish( *ret_power, p_data );
    GPOWER_PowerData_Unload( p_data );
  
    //次があるかチェックしておく
    *ret_next = (GPOWER_Get_FinishWaitID() != GPOWER_ID_NULL);
  }else{
    *ret_next = FALSE;
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief Gパワーフィールドエフェクト
 *
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT:
 *
 * @note  EVENT_GPOWERオーバーレイがロードされていないと使えません
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGPowerUseEffect( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*   work     = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   sc       = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*  gsys     = SCRCMD_WORK_GetGameSysWork( work );

  SCRIPT_CallEvent( sc, EVENT_GPowerUseEffect( gsys ) );

  return VMCMD_RESULT_SUSPEND;
}

