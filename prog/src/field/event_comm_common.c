//==============================================================================
/**
 * @file    event_comm_common.c
 * @brief   ミッション：話しかけ、話しかけられイベント共通処理
 * @author  matsuda
 * @date    2010.02.19(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"

#include "script.h"
#include "event_fieldtalk.h"

#include "print/wordset.h"
#include "field/field_comm/intrude_work.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_mission_field.h"
#include "field/field_comm/intrude_message.h"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_monolith.h"
#include "msg/msg_mission_msg.h"
#include "field/field_comm/intrude_battle.h"
#include "field/event_fieldmap_control.h" //EVENT_FieldSubProc
#include "item/itemsym.h"
#include "event_intrude.h"
#include "event_comm_common.h"
#include "event_mission_normal.h"
#include "event_mission_talked.h"
#include "event_mission_talk.h"
#include "event_mission_battle.h"
#include "event_mission_help.h"
#include "event_mission_shop.h"
#include "event_mission_secretitem.h"
#include "event_comm_notarget.h"
#include "event_comm_sleep.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "poke_tool/status_rcv.h"




//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	
//--------------------------------------------------------------
typedef struct{
  COMMTALK_COMMON_EVENT_WORK ccew;    //共通イベントワーク
	u8 first_talk_seq;
	u8 mission_achieve_user;      ///<TRUE:ミッション達成者がいる
	u8 padding[2];
}EVENT_COMM_COMMON;

typedef struct{
  FIELDMAP_WORK *fieldWork;
  MMDL *fmmdl_player;
  u32 talk_net_id;
  HEAPID heap_id;
}EVENT_COMM_MANAGER;



//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _CommCommon_Init(GAMESYS_WORK *gsys, INTRUDE_COMM_SYS_PTR intcomm, COMMTALK_COMMON_EVENT_WORK *ccew, FIELDMAP_WORK *fieldWork, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id);
static GMEVENT_RESULT EventCommCommonTalk( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT EventCommCommonTalked( GMEVENT *event, int *seq, void *wk );
static void _EventChangeTalk(GMEVENT *event, EVENT_COMM_COMMON *talk, INTRUDE_COMM_SYS_PTR intcomm);
static void _EventChangeTalked(GMEVENT *event, EVENT_COMM_COMMON *talk, INTRUDE_COMM_SYS_PTR intcomm);
static INTRUDE_TALK_TYPE _IntrudeTalkTypeJudge(EVENT_COMM_COMMON *talk, INTRUDE_COMM_SYS_PTR intcomm);
static GMEVENT_RESULT EventCommCommonTalk_Manager( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT EventCommCommonTalked_Manager( GMEVENT *event, int *seq, void *wk );
static GMEVENT * EVENT_CommCommon_TalkEvent(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id);
static GMEVENT * EVENT_CommCommon_TalkedEvent(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id);


//==============================================================================
//  データ
//==============================================================================
///話しかけた：イベント切り替えテーブル
static const EVENT_COMM_FUNC EventCommFuncTalkTbl[] = {
  EVENTCHANGE_CommMissionBattle_MtoT_Talk,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_N
  EVENTCHANGE_CommMissionBattle_TtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_M
  EVENTCHANGE_CommMissionNormal_Talk,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_N

  EVENTCHANGE_CommMissionTalked_MtoT_Talk,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_N
  EVENTCHANGE_CommMissionTalked_TtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_M
  EVENTCHANGE_CommMissionNormal_Talk,  //INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_N

  EVENTCHANGE_CommMissionTalk_MtoT_Talk,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_N
  EVENTCHANGE_CommMissionTalk_TtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_M
  EVENTCHANGE_CommMissionNormal_Talk,  //INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_N

  EVENTCHANGE_CommMissionShop_MtoT_Talk,//INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talk,   //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,   //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_N
  EVENTCHANGE_CommMissionShop_TtoM_Talk,//INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_M
  EVENTCHANGE_CommMissionNormal_Talk,   //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_N

  EVENTCHANGE_CommMissionItem_MtoT_Talk,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_N
  EVENTCHANGE_CommMissionItem_TtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_M
  EVENTCHANGE_CommMissionNormal_Talk,  //INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_N

  EVENTCHANGE_CommMissionHelp_MtoT_Talk,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_N
  EVENTCHANGE_CommMissionHelp_TtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_M
  EVENTCHANGE_CommMissionNormal_Talk,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_N
};

///話しかけられた：イベント切り替えテーブル
static const EVENT_COMM_FUNC EventCommFuncTalkedTbl[] = {
  EVENTCHANGE_CommMissionBattle_TtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_N
  EVENTCHANGE_CommMissionBattle_MtoT_Talked,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_M
  EVENTCHANGE_CommMissionNormal_Talked,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_N

  EVENTCHANGE_CommMissionTalked_TtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_N
  EVENTCHANGE_CommMissionTalked_MtoT_Talked,  //INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_M
  EVENTCHANGE_CommMissionNormal_Talked,  //INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_N

  EVENTCHANGE_CommMissionTalk_TtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_N
  EVENTCHANGE_CommMissionTalk_MtoT_Talked,  //INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_M
  EVENTCHANGE_CommMissionNormal_Talked,  //INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_N

  EVENTCHANGE_CommMissionShop_TtoM_Talked,//INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talked,   //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,   //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_N
  EVENTCHANGE_CommMissionShop_MtoT_Talked,//INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_M
  EVENTCHANGE_CommMissionNormal_Talked,   //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_N

  EVENTCHANGE_CommMissionItem_TtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_N
  EVENTCHANGE_CommMissionItem_MtoT_Talked,  //INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_M
  EVENTCHANGE_CommMissionNormal_Talked,  //INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_N

  EVENTCHANGE_CommMissionHelp_TtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_T
  EVENTCHANGE_CommMissionEtc_MtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_M
  EVENTCHANGE_CommMissionEtc_MtoN_M,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_N
  EVENTCHANGE_CommMissionHelp_MtoT_Talked,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_M
  EVENTCHANGE_CommMissionNormal_Talked,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_N
};





//==================================================================
/**
 * 通信プレイヤー話しかけイベント（ここから状況毎にイベントが枝分かれしていく)
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   intcomm		
 * @param   fmmdl_player		
 * @param   talk_net_id		
 * @param   heap_id		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_CommCommon_Talk(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id)
{
	EVENT_COMM_MANAGER *manage;
	GMEVENT *event = NULL;
	
 	event = GMEVENT_Create(
    		gsys, NULL,	EventCommCommonTalk_Manager, sizeof(EVENT_COMM_MANAGER) );
  
	manage = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( manage, sizeof(EVENT_COMM_MANAGER) );
	
	manage->fieldWork = fieldWork;
	manage->fmmdl_player = fmmdl_player;
	manage->talk_net_id = talk_net_id;
	manage->heap_id = heap_id;

	return( event );
}

//--------------------------------------------------------------
/**
 * 通信プレイヤー話しかけられたイベント（イベント管理）
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventCommCommonTalk_Manager( GMEVENT *event, int *seq, void *wk )
{
	EVENT_COMM_MANAGER *manage = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
	
	switch( *seq ){
	case 0:
	  if(intcomm == NULL){
      return GMEVENT_RES_FINISH;
    }
    
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( manage->fieldWork ) );
    
    GMEVENT_CallEvent(event, 
      EVENT_CommCommon_TalkEvent(gsys, manage->fieldWork, intcomm, 
      manage->fmmdl_player, manage->talk_net_id, manage->heap_id) );
    (*seq)++;
    break;
  case 1:
 	  MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(manage->fieldWork));
 	  return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//==================================================================
/**
 * 通信プレイヤー話しかけイベント（ここから状況毎にイベントが枝分かれしていく)
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   intcomm		
 * @param   fmmdl_player		
 * @param   talk_net_id		
 * @param   heap_id		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
static GMEVENT * EVENT_CommCommon_TalkEvent(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id)
{
	EVENT_COMM_COMMON *talk;
	GMEVENT *event = NULL;
	
 	event = GMEVENT_Create(
    		gsys, NULL,	EventCommCommonTalk, sizeof(EVENT_COMM_COMMON) );
  
	talk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( talk, sizeof(EVENT_COMM_COMMON) );
	
	//共通イベント初期化処理
	_CommCommon_Init(gsys, intcomm, &talk->ccew, fieldWork, fmmdl_player, talk_net_id, heap_id);
  Intrude_InitTalkWork(intcomm, talk_net_id);
  Intrude_InitTalkCertification(intcomm);

  //会話タイプ決定
  talk->ccew.intrude_talk_type = _IntrudeTalkTypeJudge(talk, intcomm);

	return( event );
}

//==================================================================
/**
 * 通信プレイヤー話しかけられたイベント（ここから状況毎にイベントが枝分かれしていく)
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   intcomm		
 * @param   fmmdl_player		
 * @param   talk_net_id		
 * @param   heap_id		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_CommCommon_Talked(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id)
{
	EVENT_COMM_MANAGER *manage;
	GMEVENT *event = NULL;
	
 	event = GMEVENT_Create(
    		gsys, NULL,	EventCommCommonTalked_Manager, sizeof(EVENT_COMM_MANAGER) );
  
	manage = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( manage, sizeof(EVENT_COMM_MANAGER) );
	
	manage->fieldWork = fieldWork;
	manage->fmmdl_player = fmmdl_player;
	manage->talk_net_id = talk_net_id;
	manage->heap_id = heap_id;
  
  Intrude_ClearTalkedEventReserve(intcomm); //TalkEventが始まったので予約を解除
  
	return( event );
}

//--------------------------------------------------------------
/**
 * 通信プレイヤー話しかけられたイベント（イベント管理）
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventCommCommonTalked_Manager( GMEVENT *event, int *seq, void *wk )
{
	EVENT_COMM_MANAGER *manage = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
	
	switch( *seq ){
	case 0:
	  if(intcomm == NULL){
      return GMEVENT_RES_FINISH;
    }
    
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( manage->fieldWork ) );
    
    GMEVENT_CallEvent(event, 
      EVENT_CommCommon_TalkedEvent(gsys, manage->fieldWork, intcomm, 
      manage->fmmdl_player, manage->talk_net_id, manage->heap_id) );
    (*seq)++;
    break;
  case 1:
 	  MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(manage->fieldWork));
 	  return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//==================================================================
/**
 * 通信プレイヤー話しかけられたイベント（ここから状況毎にイベントが枝分かれしていく)
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   intcomm		
 * @param   fmmdl_player		
 * @param   talk_net_id		
 * @param   heap_id		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
static GMEVENT * EVENT_CommCommon_TalkedEvent(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id)
{
	EVENT_COMM_COMMON *talk;
	GMEVENT *event = NULL;
	
 	event = GMEVENT_Create(
    		gsys, NULL,	EventCommCommonTalked, sizeof(EVENT_COMM_COMMON) );
  
	talk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( talk, sizeof(EVENT_COMM_COMMON) );
	
	//共通イベント初期化処理
	_CommCommon_Init(gsys, intcomm, &talk->ccew, fieldWork, fmmdl_player, talk_net_id, heap_id);

  //相手の送信データによって会話タイプ決定
	talk->ccew.intrude_talk_type = intcomm->recv_talk_first_attack.talk_type;
	talk->ccew.mdata = intcomm->recv_talk_first_attack.mdata;
  
	return( event );
}

//--------------------------------------------------------------
/**
 * イベント共通Init処理
 *
 * @param   intcomm		
 * @param   ccew		
 * @param   fieldWork		
 * @param   fmmdl_player		
 * @param   talk_net_id		
 * @param   heap_id		
 */
//--------------------------------------------------------------
static void _CommCommon_Init(GAMESYS_WORK *gsys, INTRUDE_COMM_SYS_PTR intcomm, COMMTALK_COMMON_EVENT_WORK *ccew, FIELDMAP_WORK *fieldWork, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id)
{
	//共通イベントワークセットアップ
	ccew->heapID = heap_id;
	ccew->fmmdl_player = fmmdl_player;
	ccew->fmmdl_talk = CommPlayer_GetMmdl(intcomm->cps, talk_net_id);
	ccew->fieldWork = fieldWork;
	ccew->talk_netid = talk_net_id;
	ccew->disguise_talk_type = MISSION_FIELD_GetTalkType(intcomm, talk_net_id);
  MyStatus_Copy( Intrude_GetMyStatus(GAMESYSTEM_GetGameData(gsys),talk_net_id), &ccew->talk_myst );
	
	if(MISSION_RecvCheck(&intcomm->mission) == TRUE){
    GFL_STD_MemCopy(MISSION_GetRecvData(&intcomm->mission), &ccew->mdata, sizeof(MISSION_DATA));
  }
  
  IntrudeEventPrint_SetupFieldMsg(&ccew->iem, gsys);

	//侵入システムのアクションステータスを更新
	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_TALK);
	Intrude_InitTalkCertification(intcomm);
	
	intcomm->mission_battle_timing_bit = 0;
}

//==================================================================
/**
 * イベント共通Finish処理
 *
 * @param   intcomm		
 */
//==================================================================
void EVENT_CommCommon_Finish(INTRUDE_COMM_SYS_PTR intcomm, COMMTALK_COMMON_EVENT_WORK *ccew)
{
  IntrudeEventPrint_ExitMenu(&ccew->iem);
  IntrudeEventPrint_ExitFieldMsg(&ccew->iem);
	if(intcomm != NULL){
  	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_FIELD);
    Intrude_InitTalkWork(intcomm, INTRUDE_NETID_NULL);
  	Intrude_InitTalkCertification(intcomm);
  }
}


//--------------------------------------------------------------
/**
 * 通信プレイヤー話しかけイベント（ここから状況毎にイベントが枝分かれしていく)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventCommCommonTalk( GMEVENT *event, int *seq, void *wk )
{
	EVENT_COMM_COMMON *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_INIT,
    SEQ_SEND_ACHIEVE_CHECK,
    SEQ_WAIT_ACHIEVE_CHECK,
    SEQ_FIRST_TALK,
    SEQ_TALK_OK,
    SEQ_WAIT_RECV_CERTIFICATION,
    SEQ_CHANGE_EVENT,
    SEQ_TALK_CANCEL,
    SEQ_LASTKEY_FINISH,
    SEQ_FINISH,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) <= SEQ_CHANGE_EVENT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_LASTKEY_FINISH;
      return GMEVENT_RES_CONTINUE;
    }
  }

	switch( *seq ){
	case SEQ_INIT:
	  {
      MISSION_TYPE mission_type = MISSION_GetMissionType(&intcomm->mission);
      if(intcomm->intrude_status[talk->ccew.talk_netid].detect_fold == TRUE){
        //話しかけた相手は蓋を閉じている場合は、相手との会話を成立させずに専用イベントを起動
        COMMTALK_COMMON_EVENT_WORK *temp_ccew;
        
        //EventChangeでワークが解放されるので、引き渡す為、テンポラリにコピーする
        temp_ccew = GFL_HEAP_AllocClearMemory(
          GFL_HEAP_LOWID(talk->ccew.heapID), sizeof(COMMTALK_COMMON_EVENT_WORK));
        GFL_STD_MemCopy(&talk->ccew, temp_ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));


        if(MISSION_GetResultData(&intcomm->mission) == NULL 
            && MISSION_RecvCheck(&intcomm->mission) == TRUE
            && MISSION_GetMissionEntry(&intcomm->mission) == TRUE
            && MISSION_CheckMissionTargetNetID(&intcomm->mission, talk->ccew.talk_netid) == TRUE){
          //蓋を閉じている相手はターゲットで自分はミッション実施者
          EVENTCHANGE_CommMissionSleep_MtoT_Talk(event, temp_ccew);
        }
        else{
          EVENTCHANGE_CommMissionEtc_MtoT_NotTalk_Talk(event, temp_ccew);
        }

        GFL_HEAP_FreeMemory(temp_ccew);
        break;
      }
      else if(intcomm->intrude_status[talk->ccew.talk_netid].action_status !=INTRUDE_ACTION_FIELD){
        //話しかけられる状態ではないため、相手との会話の成立を確認せずに専用イベントを起動
        COMMTALK_COMMON_EVENT_WORK *temp_ccew;
        
        //EventChangeでワークが解放されるので、引き渡す為、テンポラリにコピーする
        temp_ccew = GFL_HEAP_AllocClearMemory(
          GFL_HEAP_LOWID(talk->ccew.heapID), sizeof(COMMTALK_COMMON_EVENT_WORK));
        GFL_STD_MemCopy(&talk->ccew, temp_ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));

        if(intcomm->intrude_status[talk->ccew.talk_netid].action_status == INTRUDE_ACTION_BATTLE){
          if(mission_type == MISSION_TYPE_PERSONALITY){
            //戦闘手助け
            EVENTCHANGE_CommMissionHelp_TtoM_Battle(event, temp_ccew);
          }
          else{
            //戦闘中だから話しかけられない
            EVENTCHANGE_CommMissionEtc_MtoT_Battle_Talk(event, temp_ccew);
          }
        }
        else{ //戦闘以外の話しかけられる状態ではない
          EVENTCHANGE_CommMissionEtc_MtoT_NotTalk_Talk(event, temp_ccew);
        }

        GFL_HEAP_FreeMemory(temp_ccew);
        break;
      }
    }
    (*seq)++;
    break;
  case SEQ_SEND_ACHIEVE_CHECK:    //認証前にミッション達成者の確認
	  if(IntrudeSend_MissionCheckAchieveUser(intcomm) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_WAIT_ACHIEVE_CHECK:
    {
      MISSION_ACHIEVE_USER ans = MISSION_GetAnswerAchieveUser(&intcomm->mission);
      switch(ans){
      case MISSION_ACHIEVE_USER_USE:  //達成者がいる
        talk->mission_achieve_user = TRUE;
        (*seq)++;
        break;
      case MISSION_ACHIEVE_USER_NONE:
        (*seq)++;
        break;
      }
    }
    break;
	case SEQ_FIRST_TALK:
	  {
      FIRST_TALK_RET first_ret = EVENT_INTRUDE_FirstTalkSeq(intcomm, &talk->ccew, &talk->first_talk_seq);
      switch(first_ret){
      case FIRST_TALK_RET_OK:
        Intrude_ClearTalkAnswer(intcomm);
        *seq = SEQ_TALK_OK;
        break;
      case FIRST_TALK_RET_NG:
        *seq = SEQ_FINISH;
        break;
      case FIRST_TALK_RET_CANCEL:
        *seq = SEQ_TALK_CANCEL;
        break;
      }
    }
    break;

  case SEQ_TALK_OK:
    if(IntrudeSend_TalkCertificationParam(talk->ccew.talk_netid, talk->mission_achieve_user) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_WAIT_RECV_CERTIFICATION:
    {
      INTRUDE_TALK_CERTIFICATION *certifi = Intrude_GetTalkCertification(intcomm);
      if(certifi != NULL){
        if(certifi->mission_achieve_user != talk->mission_achieve_user
            || talk->mission_achieve_user == TRUE){
          //相手と自分の情報が違うor既に達成者がいる場合は通常会話にする
          talk->ccew.intrude_talk_type = INTRUDE_TALK_TYPE_NORMAL;
        }
        (*seq)++;
      }
    }
    break;
  
  case SEQ_CHANGE_EVENT:    //各イベントへ枝分かれ
    _EventChangeTalk(event, talk, intcomm);
    return GMEVENT_RES_CONTINUE;
    
	case SEQ_TALK_CANCEL:   //会話キャンセル
	  *seq = SEQ_FINISH;
	  break;
	case SEQ_LASTKEY_FINISH:
	  if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_FINISH;
    }
    break;
	case SEQ_FINISH:   //終了処理
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 通信プレイヤー話しかけられたイベント（ここから状況毎にイベントが枝分かれしていく)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT EventCommCommonTalked( GMEVENT *event, int *seq, void *wk )
{
	EVENT_COMM_COMMON *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_INIT,
    SEQ_PLAYER_DIR_CHANGE,
    SEQ_PLAYER_DIR_CHANGE_WAIT,
    SEQ_SEND_ACHIEVE_CHECK,
    SEQ_WAIT_ACHIEVE_CHECK,
    SEQ_SEND_ANSWER,
    SEQ_WAIT_TALK_ANSWER,
    SEQ_ANSWER_SEND_WAIT,
    SEQ_WAIT_RECV_CERTIFICATION,
    SEQ_EVENT_CHANGE,
    SEQ_LASTKEY_FINISH,
    SEQ_FINISH,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_LASTKEY_FINISH){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_LASTKEY_FINISH;
      return GMEVENT_RES_CONTINUE;
    }
  }

	switch( *seq ){
	case SEQ_INIT:
    Intrude_ClearTalkAnswer(intcomm);
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( talk->ccew.fieldWork ) );

    //話しかけられました。返事を返しています
    {
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gamedata, intcomm->talk.talk_netid);
      WORDSET_RegisterPlayerName( talk->ccew.iem.wordset, 0, target_myst );
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_001);
    }

	  (*seq)++;
	  break;

  case SEQ_PLAYER_DIR_CHANGE: //話しかけ相手に振り向く
    if(MMDL_CheckPossibleAcmd(talk->ccew.fmmdl_player) == TRUE){
      u16 target_dir, anmcmd;
      
      target_dir = intcomm->intrude_status[talk->ccew.talk_netid].player_pack.dir;
      if(target_dir == DIR_LEFT){
        anmcmd = AC_DIR_R;
      }
      else if(target_dir == DIR_RIGHT){
        anmcmd = AC_DIR_L;
      }
      else if(target_dir == DIR_DOWN){
        anmcmd = AC_DIR_U;
      }
      else{
        anmcmd = AC_DIR_D;
      }
      MMDL_SetAcmd(talk->ccew.fmmdl_player, anmcmd);
      (*seq)++;
    }
    break;
  case SEQ_PLAYER_DIR_CHANGE_WAIT: //振り向き終了待ち
    if(MMDL_EndAcmd(talk->ccew.fmmdl_player) == TRUE){ //アニメコマンド終了待ち
	    (*seq)++;
  	}
	  break;

	case SEQ_SEND_ACHIEVE_CHECK:  //認証前にミッション達成確認
	  if(IntrudeSend_MissionCheckAchieveUser(intcomm) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_WAIT_ACHIEVE_CHECK:
    {
      MISSION_ACHIEVE_USER ans = MISSION_GetAnswerAchieveUser(&intcomm->mission);
      switch(ans){
      case MISSION_ACHIEVE_USER_USE:  //達成者がいる
        talk->mission_achieve_user = TRUE;
        (*seq)++;
        break;
      case MISSION_ACHIEVE_USER_NONE:
        (*seq)++;
        break;
      }
    }
    break;

	case SEQ_SEND_ANSWER:   //認証の返事を返す
	  if(IntrudeSend_TalkAnswer(intcomm, intcomm->talk.talk_netid, 
	      intcomm->talk.talk_status, intcomm->talk.talk_rand) == TRUE){
      (*seq)++;
    }
    break;
	case SEQ_WAIT_TALK_ANSWER:    //話しかけに応答した事の結果を相手から待つ
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      INTRUDE_TALK_STATUS talk_status = Intrude_GetTalkAnswer(intcomm);
      BOOL talk_ng = FALSE;
      
      switch(talk_status){
      case INTRUDE_TALK_STATUS_OK:
        *seq = SEQ_ANSWER_SEND_WAIT;
        break;
      case INTRUDE_TALK_STATUS_NULL:
        break;
      case INTRUDE_TALK_STATUS_NG:
      case INTRUDE_TALK_STATUS_CANCEL:
      default:
        talk_ng = TRUE;
        break;
      }
      
      if(talk_ng == TRUE){
        //会話が切れました
        IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
        *seq = SEQ_LASTKEY_FINISH;
	    }
	  }
	  break;

  case SEQ_ANSWER_SEND_WAIT:
    if(IntrudeSend_TalkCertificationParam(intcomm->talk.talk_netid, talk->mission_achieve_user) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_WAIT_RECV_CERTIFICATION:
    {
      INTRUDE_TALK_CERTIFICATION *certifi = Intrude_GetTalkCertification(intcomm);
      if(certifi != NULL){
        if(certifi->mission_achieve_user != talk->mission_achieve_user
            || talk->mission_achieve_user == TRUE){
          //相手と自分の情報が違うor既に達成者がいる場合は通常会話にする
          talk->ccew.intrude_talk_type = INTRUDE_TALK_TYPE_NORMAL;
        }
        *seq = SEQ_EVENT_CHANGE;
      }
    }
    break;
  
  case SEQ_EVENT_CHANGE:   //各イベントへ枝分かれ
    _EventChangeTalked(event, talk, intcomm);
  	return GMEVENT_RES_CONTINUE;
    
	case SEQ_LASTKEY_FINISH:
	  if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_FINISH;
    }
    break;
	case SEQ_FINISH:   //終了処理
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
   	  MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(talk->ccew.fieldWork));
    	//共通Finish処理
    	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
      return GMEVENT_RES_FINISH;
    }
    break;
  }
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 話しかけた：イベント切り替え
 *
 * @param   event		
 * @param   talk		
 * @param   intcomm		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static void _EventChangeTalk(GMEVENT *event, EVENT_COMM_COMMON *talk, INTRUDE_COMM_SYS_PTR intcomm)
{
  COMMTALK_COMMON_EVENT_WORK *temp_ccew;
  
  //EventChangeでワークが解放されるので、引き渡す為、テンポラリにコピーする
  temp_ccew = GFL_HEAP_AllocClearMemory(
    GFL_HEAP_LOWID(talk->ccew.heapID), sizeof(COMMTALK_COMMON_EVENT_WORK));
  GFL_STD_MemCopy(&talk->ccew, temp_ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));


  OS_TPrintf("intrude_talk_type = %d\n", talk->ccew.intrude_talk_type);
  if(talk->ccew.intrude_talk_type == INTRUDE_TALK_TYPE_NORMAL){  //通常会話
    EVENTCHANGE_CommMissionNormal_Talk(event, temp_ccew);
  }
  else if(talk->ccew.intrude_talk_type == INTRUDE_TALK_TYPE_MISSION_N_to_M){  //共通：傍観者＞ミッション実施者
    EVENTCHANGE_CommMissionEtc_MtoN_N(event, temp_ccew);
  }
  else if(talk->ccew.intrude_talk_type == INTRUDE_TALK_TYPE_MISSION_N_to_T){  //共通：傍観者＞ターゲット
    EVENTCHANGE_CommMissionNormal_Talk(event, temp_ccew);
  }
  else if(talk->ccew.intrude_talk_type >= INTRUDE_TALK_TYPE_MISSION_OFFSET_START){
    GF_ASSERT(EventCommFuncTalkTbl[talk->ccew.intrude_talk_type - INTRUDE_TALK_TYPE_MISSION_VICTORY_START] != NULL);
    EventCommFuncTalkTbl[talk->ccew.intrude_talk_type - INTRUDE_TALK_TYPE_MISSION_VICTORY_START](event, temp_ccew);
  }
  else{
    GF_ASSERT(0);
    //通常会話を起動しておく
    EVENTCHANGE_CommMissionNormal_Talk(event, temp_ccew);
  }


  GFL_HEAP_FreeMemory(temp_ccew);
}

//--------------------------------------------------------------
/**
 * 話しかけられた：イベント切り替え
 *
 * @param   event		
 * @param   talk		
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void _EventChangeTalked(GMEVENT *event, EVENT_COMM_COMMON *talk, INTRUDE_COMM_SYS_PTR intcomm)
{
  COMMTALK_COMMON_EVENT_WORK *temp_ccew;
  
  //EventChangeでワークが解放されるので、引き渡す為、テンポラリにコピーする
  temp_ccew = GFL_HEAP_AllocClearMemory(
    GFL_HEAP_LOWID(talk->ccew.heapID), sizeof(COMMTALK_COMMON_EVENT_WORK));
  GFL_STD_MemCopy(&talk->ccew, temp_ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));


  OS_TPrintf("intrude_talk_type = %d\n", talk->ccew.intrude_talk_type);
  if(talk->ccew.intrude_talk_type == INTRUDE_TALK_TYPE_NORMAL){  //通常会話
    EVENTCHANGE_CommMissionNormal_Talked(event, temp_ccew);
  }
  else if(talk->ccew.intrude_talk_type == INTRUDE_TALK_TYPE_MISSION_N_to_M){  //共通：傍観者＞ミッション実施者
    EVENTCHANGE_CommMissionEtc_MtoN_N(event, temp_ccew);
  }
  else if(talk->ccew.intrude_talk_type == INTRUDE_TALK_TYPE_MISSION_N_to_T){  //共通：傍観者＞ターゲット
    EVENTCHANGE_CommMissionNormal_Talked(event, temp_ccew);
  }
  else if(talk->ccew.intrude_talk_type >= INTRUDE_TALK_TYPE_MISSION_OFFSET_START){
    GF_ASSERT(EventCommFuncTalkedTbl[talk->ccew.intrude_talk_type - INTRUDE_TALK_TYPE_MISSION_VICTORY_START] != NULL);
    EventCommFuncTalkedTbl[talk->ccew.intrude_talk_type - INTRUDE_TALK_TYPE_MISSION_VICTORY_START](event, temp_ccew);
  }
  else{
    GF_ASSERT(0);
    //通常会話を起動しておく
    EVENTCHANGE_CommMissionNormal_Talked(event, temp_ccew);
  }


  GFL_HEAP_FreeMemory(temp_ccew);
}

//--------------------------------------------------------------
/**
 * 話しかけタイプを決定する
 *
 * @param   talk		
 * @param   intcomm		
 */
//--------------------------------------------------------------
static INTRUDE_TALK_TYPE _IntrudeTalkTypeJudge(EVENT_COMM_COMMON *talk, INTRUDE_COMM_SYS_PTR intcomm)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork(talk->ccew.fieldWork) );
  NetID my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  MISSION_TYPE mission_type = MISSION_GetMissionType(&intcomm->mission);
  INTRUDE_TALK_TYPE intrude_talk_type = INTRUDE_TALK_TYPE_NORMAL;
  
  //ミッション実施中である
  if(MISSION_GetResultData(&intcomm->mission) == NULL 
      && MISSION_RecvCheck(&intcomm->mission) == TRUE){
    //ミッション実施者である
    if(MISSION_GetMissionEntry(&intcomm->mission) == TRUE){
    	//話しかけた相手がミッションターゲットの相手である
    	if(MISSION_CheckMissionTargetNetID(&intcomm->mission, talk->ccew.talk_netid) == TRUE){
        intrude_talk_type = INTRUDE_TALK_TYPE_MISSION_OFFSET_M_to_T;
      }
      //話しかけた相手もミッション実施者である
      else if(intcomm->intrude_status[talk->ccew.talk_netid].mission_entry == TRUE){
        intrude_talk_type = INTRUDE_TALK_TYPE_MISSION_OFFSET_M_to_M;
      }
      else{         //話しかけた相手は傍観者である
        intrude_talk_type = INTRUDE_TALK_TYPE_MISSION_OFFSET_M_to_N;
      }
    }
    //自分がターゲットである
    else if(MISSION_CheckMissionTargetNetID(&intcomm->mission, my_net_id) == TRUE){
      //話しかけた相手がミッション実施者である
      if(intcomm->intrude_status[talk->ccew.talk_netid].mission_entry == TRUE){
        intrude_talk_type = INTRUDE_TALK_TYPE_MISSION_OFFSET_T_to_M;
      }
      else{   //話しかけた相手は傍観者
        intrude_talk_type = INTRUDE_TALK_TYPE_MISSION_OFFSET_T_to_N;
      }
    }
    //自分は傍観者である
    else{
      //相手はミッション実施者である
      if(intcomm->intrude_status[talk->ccew.talk_netid].mission_entry == TRUE){
        intrude_talk_type = INTRUDE_TALK_TYPE_MISSION_N_to_M;
      }
      //相手はターゲットである
      else if(MISSION_CheckMissionTargetNetID(&intcomm->mission, talk->ccew.talk_netid) == TRUE){
        intrude_talk_type = INTRUDE_TALK_TYPE_MISSION_N_to_T;
      }
      else{ //相手も傍観者である
        intrude_talk_type = INTRUDE_TALK_TYPE_NORMAL;
      }
    }
  }
  else{ //ミッションは実施していない
    intrude_talk_type = INTRUDE_TALK_TYPE_NORMAL;
  }
  
  if(intrude_talk_type >= INTRUDE_TALK_TYPE_MISSION_OFFSET_START){
    switch(mission_type){ //ミッション毎のオフセットを足しこむ
    case MISSION_TYPE_VICTORY:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_VICTORY_START;
      break;
    case MISSION_TYPE_SKILL:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_SKILL_START;
      break;
    case MISSION_TYPE_BASIC:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_BASIC_START;
      break;
    case MISSION_TYPE_ATTRIBUTE:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_START;
      break;
    case MISSION_TYPE_ITEM:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_ITEM_START;
      break;
    case MISSION_TYPE_PERSONALITY:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_PERSONALITY_START;
      break;
    }
    intrude_talk_type -= INTRUDE_TALK_TYPE_MISSION_OFFSET_START;
  }
  
  return intrude_talk_type;
}

