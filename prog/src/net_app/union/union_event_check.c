//==============================================================================
/**
 * @file    union_event_check.c
 * @brief   ユニオン：イベント起動チェック
 * @author  matsuda
 * @date    2009.07.09(木)
 */
//==============================================================================
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_event.h"
#include "field/zonedata.h"
#include "field/fieldmap.h"
#include "net_app/union/union_main.h"
#include "net_app/union/union_types.h"
#include "net_app/union/union_event_check.h"
#include "union_local.h"


//==================================================================
/**
 * ユニオンイベント：話しかけチェック
 *
 * @param   gsys		
 * @param   fieldWork		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * UnionEvent_TalkCheck( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
#if 0
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  UNION_SYSTEM_PTR unisys = GameCommSys_GetAppWork(game_comm);
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldWork);
  MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( fieldWork );
  s16 check_gx, check_gz, check_gy;
  MMDL *target_pc;
  u16 obj_id;
  
  FIELD_PLAYER_GetFrontGridPos(player, &check_gx, &check_gy, &check_gz);
  target_pc = MMDLSYS_SearchGridPos(fldMdlSys, check_gx, check_gz, FALSE);
  if(target_pc == NULL){
    return NULL;
  }
  
  obj_id = MMDL_GetOBJID(target_pc);
  if(obj_id > UNION_RECEIVE_BEACON_MAX){
    GF_ASSERT(0);
    return NULL;
  }
  UnionMySituation_SetParam(unisys, 
    UNION_MYSITU_PARAM_IDX_CONNECT_MAC, unisys->receive_beacon[obj_id].mac_address);
  OS_TPrintf("ターゲット発見! obj_id = %d, gx=%d, gz=%d\n", obj_id, check_gx, check_gz);
#endif
  return NULL;
}
