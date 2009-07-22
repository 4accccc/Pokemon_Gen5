//==============================================================================
/**
 * @file    union_subproc.c
 * @brief   ユニオン：サブPROC呼び出し制御
 * @author  matsuda
 * @date    2009.07.16(木)
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
#include "net_app/union/union_msg.h"
#include "field/event_mapchange.h"
#include "fieldmap/zone_id.h"


//==============================================================================
//  構造体定義
//==============================================================================
///ユニオン：サブPROC呼び出しイベント用ワーク構造体
typedef struct{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldWork;
  UNION_SYSTEM_PTR unisys;
}UNION_SUBPROC_EVENT_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT UnionSubProc_GameChangeEvent(GMEVENT * event, int * seq, void * work);


//==============================================================================
//  データ
//==============================================================================
///サブPROC実行時にplay_category値のテーブル
static const struct{
  u8 play_category;       ///<サブPROC実行中のplay_category値
  u8 after_play_category; ///<サブPROC終了後に設定するplay_category値
  u8 padding[2];
}SubProc_PlayCategoryTbl[] = {
  {//UNION_SUBPROC_ID_NULL
    UNION_PLAY_CATEGORY_UNION, 
    UNION_PLAY_CATEGORY_UNION,
  },
  {//UNION_SUBPROC_ID_TRAINERCARD
    UNION_PLAY_CATEGORY_TRAINERCARD, 
    UNION_PLAY_CATEGORY_TALK,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_50
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_FREE
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_STANDARD
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP
    UNION_PLAY_CATEGORY_COLOSSEUM_MULTI, 
    UNION_PLAY_CATEGORY_COLOSSEUM_MULTI,
  },
};
SDK_COMPILER_ASSERT(UNION_SUBPROC_ID_MAX == NELEMS(SubProc_PlayCategoryTbl));


//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * サブPROC呼び出しイベント生成
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   unisys		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
GMEVENT * UnionSubProc_Create(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldWork, UNION_SYSTEM_PTR unisys)
{
	GMEVENT *event;
	UNION_SUBPROC_EVENT_WORK *subev;
	
	event = GMEVENT_Create(gsys, NULL, 
	  UnionSubProc_GameChangeEvent, sizeof(UNION_SUBPROC_EVENT_WORK));

	subev = GMEVENT_GetEventWork(event);
	subev->gsys = gsys;
	subev->fieldWork = fieldWork;
	subev->unisys = unisys;

  unisys->subproc.active = TRUE;
	return event;
}

//--------------------------------------------------------------
/**
 * サブPROC呼び出しイベント
 *
 * @param   event		
 * @param   seq		
 * @param   work		
 *
 * @retval  GMEVENT_RESULT		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT UnionSubProc_GameChangeEvent(GMEVENT * event, int * seq, void * work)
{
	UNION_SUBPROC_EVENT_WORK *subev = work;
	GAMESYS_WORK *gsys = subev->gsys;
	UNION_SYSTEM_PTR unisys = subev->unisys;
	UNION_SUB_PROC *subproc = &unisys->subproc;
  GMEVENT *child_event = NULL;
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  
	switch(*seq) {
	case 0:
	  OS_TPrintf("GMEVENT サブPROC呼び出し id = %d\n", subproc->id);
    UnionMsg_AllDel(unisys);
    
	  switch(subproc->id){
	  case UNION_SUBPROC_ID_TRAINERCARD:
  	  child_event = EVENT_FieldSubProc(
  	    gsys, subev->fieldWork, TRCARD_OVERLAY_ID, &TrCardSysCommProcData, subproc->parent_work);
  	  break;
  	case UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_50:
  	case UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_FREE:
  	case UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_STANDARD:
      {
        PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gsys );
        VecFx32 pos;
        
        pos.x = (88 + 16*GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())) << FX32_SHIFT;
        pos.y = 0;
        pos.z = 136 << FX32_SHIFT;
        child_event = DEBUG_EVENT_ChangeMapPos(gsys, subev->fieldWork, ZONE_ID_CLOSSEUM, &pos, 0);
      }
      break;
  	case UNION_SUBPROC_ID_COLOSSEUM_WARP_MULTI:
      {
        PLAYER_WORK *plWork = GAMESYSTEM_GetMyPlayerWork( gsys );
        VecFx32 pos;
        
        pos.x = (88 + 16*GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())) << FX32_SHIFT;
        pos.y = 0;
        pos.z = 136 << FX32_SHIFT;
        child_event = DEBUG_EVENT_ChangeMapPos(gsys, subev->fieldWork, ZONE_ID_CLOSSEUM02, &pos,0);
      }
      break;
  	default:
  	  GF_ASSERT(0); //不明なサブPROC ID
  	  break;
  	}
    GMEVENT_CallEvent(event, child_event);
    
    UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_PLAY_CATEGORY, 
      (void*)SubProc_PlayCategoryTbl[subproc->id].play_category);
    OS_TPrintf("play_category = %d\n", situ->play_category);
		(*seq) ++;
		break;
	
	case 1:
    UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_PLAY_CATEGORY, 
      (void*)SubProc_PlayCategoryTbl[subproc->id].after_play_category);
    OS_TPrintf("after play_category = %d\n", situ->play_category);

    subproc->id = UNION_SUBPROC_ID_NULL;
    subproc->parent_work = NULL;
    subproc->active = FALSE;
	  OS_TPrintf("GMEVENT サブPROC終了\n");
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
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
void UnionSubProc_EventSet(UNION_SYSTEM_PTR unisys, UNION_SUBPROC_ID sub_proc_id, void *parent_wk)
{
  GF_ASSERT(unisys->subproc.id == UNION_SUBPROC_ID_NULL && unisys->subproc.active == FALSE);
  
  unisys->subproc.id = sub_proc_id;
  unisys->subproc.parent_work = parent_wk;
}

//==================================================================
/**
 * サブPROCが実行中か調べる
 *
 * @param   unisys		
 *
 * @retval  TRUE:実行中。　FALSE:何も実行していない
 */
//==================================================================
BOOL UnionSubProc_IsExits(UNION_SYSTEM_PTR unisys)
{
  if(unisys->subproc.id == UNION_SUBPROC_ID_NULL && unisys->subproc.active == FALSE){
    return FALSE;
  }
  return TRUE;
}

