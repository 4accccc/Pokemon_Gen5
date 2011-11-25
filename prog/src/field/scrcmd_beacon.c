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
  u16            value1     = SCRCMD_GetVMWorkValue( core, work );
  u16            value2     = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*  gsys     = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SCRIPT_WORK*   sc       = SCRCMD_WORK_GetScriptWork( work );
  u16            zone_id  = SCRCMD_WORK_GetZoneID( work );

  switch( beacon_id ){
  case SCR_BEACON_SET_REQ_ITEM_GET: //アイテム拾った
    GAMEBEACON_Set_SpItemGet( value1 );
    break;
  case SCR_BEACON_SET_REQ_FSKILL_USE: //フィールド技を使った
    GAMEBEACON_Set_FieldSkill( value1 );
    break;
  case SCR_BEACON_SET_REQ_UNION_COUNTER: //ユニオンカウンター処理開始
    GAMEBEACON_Set_UnionIn();
    break;
  case SCR_BEACON_SET_REQ_TRIAL_HOUSE_START: //トライアルハウス挑戦開始
    GAMEBEACON_Set_TrialHouse();
    break;
  case SCR_BEACON_SET_REQ_TRIAL_HOUSE_RANK: //トライアルハウスランクセット
    GAMEBEACON_Set_TrialHouseRank( value1 );
    break;
  case SCR_BEACON_SET_REQ_POKE_SHIFTER_START: //ポケシフター挑戦開始
    GAMEBEACON_Set_PokeShifter();
    break;
  case SCR_BEACON_SET_REQ_SUBWAY_START: //サブウェイ挑戦開始
    GAMEBEACON_Set_Subway();
    break;
  case SCR_BEACON_SET_REQ_SUBWAY_WIN: //サブウェイ勝利
    GAMEBEACON_Set_SubwayStraightVictories( value1 );
    break;
  case SCR_BEACON_SET_REQ_SUBWAY_TROPHY_GET:  //サブウェイトロフィーゲット
    GAMEBEACON_Set_SubwayTrophyGet();
    break;
  case SCR_BEACON_SET_REQ_WBT_ENTRY:  ///<WBTトーナメント挑戦開始
    GAMEBEACON_Set_WBTEntry( (WBTCUP)value1 );
    break;
  case SCR_BEACON_SET_REQ_WBT_WIN:  ///<WBTトーナメントX回戦勝利
    GAMEBEACON_Set_WBTWin( (WBTCUP)value1, (u8)value2 );
    break;
  case SCR_BEACON_SET_REQ_WBT_LOSE:  ///<WBTトーナメントX回戦敗退
    GAMEBEACON_Set_WBTLose( (WBTCUP)value1, (u8)value2 );
    break;
  case SCR_BEACON_SET_REQ_INF_DUNGEON_ENTRY: ///<無限ダンジョン挑戦
    GAMEBEACON_Set_InfinityDungeonEntry( (u8)value1, zone_id );
    break;
  case SCR_BEACON_SET_REQ_INF_DUNGEON_CLEAR:  ///<無限ダンジョンクリア
    GAMEBEACON_Set_InfinityDungeonClear( (u8)value1, zone_id );
    break;
  case SCR_BEACON_SET_REQ_POKEWOOD_FILMING:  ///<ポケウッド撮影開始
    GAMEBEACON_Set_PokewoodFilming( (u8)value1 );
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

