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
#include "app/trainer_card.h"
#include "field/event_fieldmap_control.h"


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
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  UNION_SYSTEM_PTR unisys = GameCommSys_GetAppWork(game_comm);
  GMEVENT *new_event = NULL;
  UNION_SUB_PROC *subproc = &unisys->subproc;
  
  switch(subproc->id){
  case UNION_SUBPROC_ID_TRAINERCARD:
    new_event = EVENT_FieldSubProc(gsys, fieldWork, TRCARD_OVERLAY_ID, 
      &TrCardSysCommProcData, subproc->parent_work);
    break;
  }
  
  subproc->id = UNION_SUBPROC_ID_NULL;
  subproc->parent_work = NULL;
  return new_event;
  
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
    UNION_MYSITU_PARAM_IDX_CALLING_PC, unisys->receive_beacon[obj_id].mac_address);
  OS_TPrintf("ターゲット発見! obj_id = %d, gx=%d, gz=%d\n", obj_id, check_gx, check_gz);
#endif
  return NULL;
}

//==================================================================
/**
 * サブPROC呼び出し設定
 *
 * @param   unisys		
 * @param   sub_proc_id		
 * @param   parent_wk		
 */
//==================================================================
void UnionEvent_SubProcSet(UNION_SYSTEM_PTR unisys, UNION_SUBPROC_ID sub_proc_id, void *parent_wk)
{
  GF_ASSERT(unisys->subproc.id == UNION_SUBPROC_ID_NULL);
  
  unisys->subproc.id = sub_proc_id;
  unisys->subproc.parent_work = parent_wk;
}

//==================================================================
/**
 * サブPROC呼び出しIDを取得
 *
 * @param   unisys		
 *
 * @retval  UNION_SUBPROC_ID		
 */
//==================================================================
UNION_SUBPROC_ID UnionEvent_SubProcGet(UNION_SYSTEM_PTR unisys)
{
  return unisys->subproc.id;
}

