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
#include "field/event_colosseum_battle.h"
#include "colosseum.h"
#include "colosseum_tool.h"
#include "app\pms_input.h"
#include "app\pms_select.h"
#include "net_app/pokemontrade.h"


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

static BOOL SubEvent_TrainerCard(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq);
static BOOL SubEvent_Trade(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq);
static BOOL SubEvent_ColosseumWarp(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq);
static BOOL SubEvent_ColosseumWarpMulti(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq);
static BOOL SubEvent_UnionWarp(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq);
static BOOL SubEvent_Pokelist(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq);
static BOOL SubEvent_Battle(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq);
static BOOL SubEvent_Chat(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq);


//--------------------------------------------------------------
//  オーバーレイ定義
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(pokelist);
FS_EXTERN_OVERLAY(poke_status);
FS_EXTERN_OVERLAY(pokemon_trade);


//==============================================================================
//  データ
//==============================================================================
///サブPROC実行時にplay_category値のテーブル
static const struct{
  BOOL (*sub_event)(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq);  ///<実行関数
  u8 play_category;       ///<サブPROC実行中のplay_category値
  u8 after_play_category; ///<サブPROC終了後に設定するplay_category値
  u8 padding[2];
}SubProc_PlayCategoryTbl[] = {
  {//UNION_SUBPROC_ID_NULL
    NULL,
    UNION_PLAY_CATEGORY_UNION, 
    UNION_PLAY_CATEGORY_UNION,
  },
  {//UNION_SUBPROC_ID_TRAINERCARD
    SubEvent_TrainerCard,
    UNION_PLAY_CATEGORY_TRAINERCARD, 
    UNION_PLAY_CATEGORY_TALK,
  },
  {//UNION_SUBPROC_ID_TRADE
    SubEvent_Trade,
    UNION_PLAY_CATEGORY_TRADE, 
    UNION_PLAY_CATEGORY_TALK,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_50_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_50
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_FREE_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_FREE
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_STANDARD_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_STANDARD
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_DOUBLE_50_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_DOUBLE_50
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_DOUBLE_FREE_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_DOUBLE_FREE
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_DOUBLE_STANDARD_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_DOUBLE_STANDARD
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_TRIPLE_50_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_TRIPLE_50
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_TRIPLE_FREE_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_TRIPLE_FREE
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_TRIPLE_STANDARD_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_TRIPLE_STANDARD
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_ROTATION_50_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_ROTATION_50
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_ROTATION_FREE_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_ROTATION_FREE
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_ROTATION_STANDARD_SHOOTER
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD_SHOOTER, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD_SHOOTER,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_ROTATION_STANDARD
    SubEvent_ColosseumWarp,
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD, 
    UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_WARP_MULTI
    SubEvent_ColosseumWarpMulti,
    UNION_PLAY_CATEGORY_COLOSSEUM_MULTI, 
    UNION_PLAY_CATEGORY_COLOSSEUM_MULTI,
  },
  {//UNION_SUBPROC_ID_UNION_WARP
    SubEvent_UnionWarp,
    UNION_PLAY_CATEGORY_MAX,  //MAX=変更しない
    UNION_PLAY_CATEGORY_MAX,
  },
  {//UNION_SUBPROC_ID_POKELIST
    SubEvent_Pokelist,
    UNION_PLAY_CATEGORY_MAX,  //MAX=変更しない
    UNION_PLAY_CATEGORY_MAX,
  },
  {//UNION_SUBPROC_ID_BATTLE
    SubEvent_Battle,
    UNION_PLAY_CATEGORY_MAX,  //MAX=変更しない
    UNION_PLAY_CATEGORY_MAX,
  },
  {//UNION_SUBPROC_ID_COLOSSEUM_TRAINERCARD
    SubEvent_TrainerCard,
    UNION_PLAY_CATEGORY_MAX,  //MAX=変更しない
    UNION_PLAY_CATEGORY_MAX,
  },
  {//UNION_SUBPROC_ID_CHAT
    SubEvent_Chat,
    UNION_PLAY_CATEGORY_MAX,  //MAX=変更しない
    UNION_PLAY_CATEGORY_MAX,
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
GMEVENT * UnionSubProc_Create(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldWork, UNION_SYSTEM_PTR unisys)
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
#include "system/main.h"
static GMEVENT_RESULT UnionSubProc_GameChangeEvent(GMEVENT * event, int * seq, void * work)
{
	UNION_SUBPROC_EVENT_WORK *subev = work;
	GAMESYS_WORK *gsys = subev->gsys;
	UNION_SYSTEM_PTR unisys = subev->unisys;
	UNION_SUB_PROC *subproc = &unisys->subproc;
  GMEVENT *child_event = NULL;
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  BOOL next;
  
	switch(*seq) {
	case 0:
	  OS_TPrintf("GMEVENT サブPROC呼び出し id = %d\n", subproc->id);
    UnionMsg_AllDel(unisys);

    if(SubProc_PlayCategoryTbl[subproc->id].play_category != UNION_PLAY_CATEGORY_MAX){
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_PLAY_CATEGORY, 
        (void*)SubProc_PlayCategoryTbl[subproc->id].play_category);
    }
    OS_TPrintf("play_category = %d\n", situ->play_category);

    (*seq)++;
    //break through
    
	case 1:
	  next = SubProc_PlayCategoryTbl[subproc->id].sub_event(
	      gsys, unisys, subev->fieldWork, subproc->parent_work, event, &child_event, &subproc->seq);
	  if(child_event != NULL){
      GMEVENT_CallEvent(event, child_event);
    }
    else if(next == TRUE){
		  (*seq)++;
		}
		break;
	
	case 2:
    if(SubProc_PlayCategoryTbl[subproc->id].after_play_category != UNION_PLAY_CATEGORY_MAX){
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_PLAY_CATEGORY, 
        (void*)SubProc_PlayCategoryTbl[subproc->id].after_play_category);
    }
    OS_TPrintf("after play_category = %d\n", situ->play_category);

    subproc->id = UNION_SUBPROC_ID_NULL;
    subproc->parent_work = NULL;
    subproc->seq = 0;
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
  unisys->subproc.seq = 0;
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


//==============================================================================
//  サブイベント実行関数
//==============================================================================
//--------------------------------------------------------------
/**
 * イベント：トレーナーカード
 *
 * @param   gsys		
 * @param   unisys		
 * @param   fieldWork		
 * @param   pwk		
 * @param   seq		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static BOOL SubEvent_TrainerCard(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq)
{
  switch(*seq){
  case 0:
    *child_event = EVENT_FieldSubProc(
  	    gsys, fieldWork, TRCARD_OVERLAY_ID, &TrCardSysCommProcData, pwk);
    break;
  default:
    return TRUE;
  }
  
  (*seq)++;
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント：ポケモン交換
 *
 * @param   gsys		
 * @param   unisys		
 * @param   fieldWork		
 * @param   pwk		
 * @param   seq		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static BOOL SubEvent_Trade(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq)
{
  switch(*seq){
  case 0:
    *child_event = EVENT_FieldSubProc(
  	    gsys, fieldWork, FS_OVERLAY_ID(pokemon_trade), &PokemonTradeProcData, pwk);
    break;
  default:
    return TRUE;
  }
  
  (*seq)++;
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント：コロシアムへワープ
 *
 * @param   gsys		
 * @param   unisys		
 * @param   fieldWork		
 * @param   pwk		
 * @param   seq		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static BOOL SubEvent_ColosseumWarp(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq)
{
  switch(*seq){
  case 0:
    {
      VecFx32 pos;
      
      pos.x = (88 + 16*GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())) << FX32_SHIFT;
      pos.y = 0;
      pos.z = 136 << FX32_SHIFT;
      *child_event = EVENT_ChangeMapPos(gsys, fieldWork, ZONE_ID_CLOSSEUM, &pos, 0);
    }
    break;
  default:
    return TRUE;
  }
  
  (*seq)++;
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント：マルチ用のコロシアムへワープ
 *
 * @param   gsys		
 * @param   unisys		
 * @param   fieldWork		
 * @param   pwk		
 * @param   seq		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static BOOL SubEvent_ColosseumWarpMulti(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq)
{
  switch(*seq){
  case 0:
    {
      VecFx32 pos;
      
      pos.x = (88 + 16*GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())) << FX32_SHIFT;
      pos.y = 0;
      pos.z = 136 << FX32_SHIFT;
      *child_event = EVENT_ChangeMapPos(gsys, fieldWork, ZONE_ID_CLOSSEUM02, &pos,0);
    }
    break;
  default:
    return TRUE;
  }
  
  (*seq)++;
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント：ユニオンルームへワープ
 *
 * @param   gsys		
 * @param   unisys		
 * @param   fieldWork		
 * @param   pwk		
 * @param   seq		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static BOOL SubEvent_UnionWarp(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq)
{
  switch(*seq){
  case 0:
    {
      VecFx32 pos;
      
      pos.x = 168 << FX32_SHIFT;
      pos.y = 0;
      pos.z = 232 << FX32_SHIFT;
      *child_event = EVENT_ChangeMapPos(gsys, fieldWork, ZONE_ID_UNION, &pos,0);
    }
    break;
  default:
    return TRUE;
  }
  
  (*seq)++;
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント：ポケモンリスト呼び出し
 *
 * @param   gsys		
 * @param   unisys		
 * @param   fieldWork		
 * @param   pwk		
 * @param   seq		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static BOOL SubEvent_Pokelist(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq)
{
  UNION_SUBPROC_PARENT_POKELIST *parent_list = pwk;
  PLIST_DATA *plist = &parent_list->plist;
  PSTATUS_DATA *pstatus = &parent_list->pstatus;
  
  enum{
    SEQ_FADEOUT,
    SEQ_FIELD_CLOSE,
    SEQ_POKELIST,
    SEQ_POKELIST_WAIT,
    SEQ_STATUS,
    SEQ_STATUS_WAIT,
    SEQ_FIELD_OPEN,
    SEQ_FADEIN,
    SEQ_FINISH,
  };
  
	switch(*seq) {
	case SEQ_FADEOUT:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut(gsys, fieldWork, 
                                      FIELD_FADE_BLACK, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(parent_event, fade_event);
    }
		(*seq) ++;
		break;
	case SEQ_FIELD_CLOSE:
		GMEVENT_CallEvent(parent_event, EVENT_FieldClose(gsys, fieldWork));
		(*seq) ++;
		break;
	case SEQ_POKELIST:
    OS_TPrintf("ポケモンリスト呼び出し\n");
		GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(pokelist), &PokeList_ProcData, plist);
		(*seq) ++;
		break;
	case SEQ_POKELIST_WAIT:
		if(GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      if(unisys->colosseum_sys != NULL){
        plist->comm_selected_num 
          = ColosseumTool_ReceiveCheck_PokeListSeletedNum(unisys->colosseum_sys);
        OS_TPrintf("selected_num = %d\n", plist->comm_selected_num);
      }
      break;
    }
    if(plist->ret_mode == PL_RET_STATUS){
      *seq = SEQ_STATUS;
    }
    else{
      *seq = SEQ_FIELD_OPEN;
    }
    break;
  case SEQ_STATUS:
    OS_TPrintf("ポケモンステータス呼び出し\n");
    pstatus->ppd = (void*)plist->pp;
    pstatus->cfg = plist->cfg;
    pstatus->game_data = GAMESYSTEM_GetGameData( gsys );

    pstatus->ppt = PST_PP_TYPE_POKEPARTY;
    pstatus->max = PokeParty_GetPokeCount( plist->pp );
    pstatus->mode = PST_MODE_NORMAL;
    pstatus->pos = plist->ret_sel;
    pstatus->page = PPT_INFO;
    
		GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(poke_status), &PokeStatus_ProcData, pstatus);
		(*seq) ++;
		break;
	case SEQ_STATUS_WAIT:
		if(GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    plist->ret_sel = pstatus->pos;
    (*seq) = SEQ_POKELIST;
    break;
	case SEQ_FIELD_OPEN:
		GMEVENT_CallEvent(parent_event, EVENT_FieldOpen(gsys));
		(*seq) ++;
		break;
	case SEQ_FADEIN:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, fieldWork, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(parent_event, fade_event);
    }
		(*seq) ++;
		break;
	case SEQ_FINISH:
  default:
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント：戦闘画面呼び出し
 *
 * @param   gsys		
 * @param   unisys		
 * @param   fieldWork		
 * @param   pwk		
 * @param   seq		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static BOOL SubEvent_Battle(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  
  switch(*seq){
  case 0:
    *child_event = EVENT_ColosseumBattle(gsys, fieldWork, situ->play_category, pwk);
    break;
  default:
    return TRUE;
  }
  
  (*seq)++;
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント：簡易会話入力画面
 *
 * @param   gsys		
 * @param   unisys		
 * @param   fieldWork		
 * @param   pwk		
 * @param   seq		
 *
 * @retval  GMEVENT *		
 */
//--------------------------------------------------------------
static BOOL SubEvent_Chat(GAMESYS_WORK *gsys, UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, void *pwk, GMEVENT * parent_event, GMEVENT **child_event, u8 *seq)
{
  switch(*seq){
  case 0:
    *child_event = EVENT_FieldSubProc(
  	    gsys, fieldWork, FS_OVERLAY_ID(pmsinput), &ProcData_PMSSelect, pwk);
    break;
  default:
    return TRUE;
  }
  
  (*seq)++;
  return FALSE;
}

