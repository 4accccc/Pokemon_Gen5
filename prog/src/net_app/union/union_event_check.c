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
#include "net_app/union/union_subproc.h"


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ユニオンイベント：通信イベント起動チェック
 *
 * @param   gsys		
 * @param   fieldWork		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * UnionEvent_CommCheck( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  UNION_SYSTEM_PTR unisys = GameCommSys_GetAppWork(game_comm);
  GMEVENT *new_event = NULL;
  UNION_SUB_PROC *subproc = &unisys->subproc;
  
  if(subproc->id != UNION_SUBPROC_ID_NULL && subproc->active == FALSE){
    new_event = UnionSubProc_Create(gsys, fieldWork, unisys);
  }
  return new_event;
}

//==================================================================
/**
 * ユニオンイベント：話しかけチェック(ボタンが押されたときだけチェックが入る)
 *
 * @param   gsys		
 * @param   fieldWork		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * UnionEvent_TalkCheck( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  return NULL;
}

