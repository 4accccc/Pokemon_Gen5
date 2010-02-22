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
#include "event_comm_talk.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/bingo_system.h"
#include "field/field_comm/intrude_mission.h"
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
	u8 padding[3];
}EVENT_COMM_COMMON;



//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _CommCommon_Init(GAMESYS_WORK *gsys, INTRUDE_COMM_SYS_PTR intcomm, COMMTALK_COMMON_EVENT_WORK *ccew, FIELDMAP_WORK *fieldWork, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id);
static GMEVENT_RESULT EventCommCommonTalk( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT EventCommCommonTalked( GMEVENT *event, int *seq, void *wk );
static void _EventChangeTalk(GMEVENT *event, EVENT_COMM_COMMON *talk, INTRUDE_COMM_SYS_PTR intcomm);
static void _EventChangeTalked(GMEVENT *event, EVENT_COMM_COMMON *talk, INTRUDE_COMM_SYS_PTR intcomm);
static INTRUDE_TALK_TYPE _IntrudeTalkTypeJudge(EVENT_COMM_COMMON *talk, INTRUDE_COMM_SYS_PTR intcomm);


//==============================================================================
//  データ
//==============================================================================
///話しかけた：イベント切り替えテーブル
static const EVENT_COMM_FUNC EventCommFuncTalkTbl[] = {
  EVENTCHANGE_CommMissionBattle_MtoT_Talk,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_N
  EVENTCHANGE_CommMissionBattle_TtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_N

  EVENTCHANGE_CommMissionTalked_MtoT_Talk,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_N
  EVENTCHANGE_CommMissionTalked_TtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_N

  EVENTCHANGE_CommMissionTalk_MtoT_Talk,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_N
  EVENTCHANGE_CommMissionTalk_TtoM_Talk,  //INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_N
};

///話しかけられた：イベント切り替えテーブル
static const EVENT_COMM_FUNC EventCommFuncTalkedTbl[] = {
  EVENTCHANGE_CommMissionBattle_TtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_N
  EVENTCHANGE_CommMissionBattle_MtoT_Talked,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_N

  EVENTCHANGE_CommMissionTalked_TtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_N
  EVENTCHANGE_CommMissionTalked_MtoT_Talked,  //INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_N

  EVENTCHANGE_CommMissionTalk_TtoM_Talked,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_N
  EVENTCHANGE_CommMissionTalk_MtoT_Talked,  //INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_SIZE_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_OCCUR_T_to_N

  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_T
  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_N
  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_M
  NULL,  //INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_N
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
	EVENT_COMM_COMMON *talk;
	GMEVENT *event = NULL;
	
 	event = GMEVENT_Create(
    		gsys, NULL,	EventCommCommonTalk, sizeof(EVENT_COMM_COMMON) );
  
	talk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( talk, sizeof(EVENT_COMM_COMMON) );
	
	//共通イベント初期化処理
	_CommCommon_Init(gsys, intcomm, &talk->ccew, fieldWork, fmmdl_player, talk_net_id, heap_id);
  Intrude_InitTalkWork(intcomm, talk_net_id);

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
	
	if(MISSION_RecvCheck(&intcomm->mission) == TRUE){
    GFL_STD_MemCopy(MISSION_GetRecvData(&intcomm->mission), &ccew->mdata, sizeof(MISSION_DATA));
  }
  
  IntrudeEventPrint_SetupFieldMsg(&ccew->iem, gsys);

	//侵入システムのアクションステータスを更新
	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_TALK);
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
  IntrudeEventPrint_ExitFieldMsg(&ccew->iem);
	if(intcomm != NULL){
  	Intrude_SetActionStatus(intcomm, INTRUDE_ACTION_FIELD);
    Intrude_InitTalkWork(intcomm, INTRUDE_NETID_NULL);
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
    SEQ_FIRST_TALK,
    SEQ_TALK_OK,
    SEQ_TALK_CANCEL,
    SEQ_FINISH,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_TALK_OK){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_invasion_mission_sys002);
      *seq = SEQ_FINISH;
    }
  }

	switch( *seq ){
	case SEQ_FIRST_TALK:
	  {
      FIRST_TALK_RET first_ret = EVENT_INTRUDE_FirstTalkSeq(intcomm, &talk->ccew, &talk->first_talk_seq);
      switch(first_ret){
      case FIRST_TALK_RET_OK:
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

  case SEQ_TALK_OK:   //各イベントへ枝分かれ
    _EventChangeTalk(event, talk, intcomm);
    return GMEVENT_RES_CONTINUE;
    
	case SEQ_TALK_CANCEL:   //会話キャンセル
	  *seq = SEQ_FINISH;
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
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_INIT,
    SEQ_ANSWER_SEND_WAIT,
    SEQ_FINISH,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_FINISH){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_invasion_mission_sys002);
      *seq = SEQ_FINISH;
    }
  }

	switch( *seq ){
	case SEQ_INIT:    //返事を返す
	  if(IntrudeSend_TalkAnswer(
        intcomm, intcomm->talk.talk_netid, intcomm->talk.talk_status) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_ANSWER_SEND_WAIT:   //各イベントへ枝分かれ
    _EventChangeTalked(event, talk, intcomm);
  	return GMEVENT_RES_CONTINUE;
    
	case SEQ_FINISH:   //終了処理
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
    return GMEVENT_RES_FINISH;
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
  }
  else if(talk->ccew.intrude_talk_type == INTRUDE_TALK_TYPE_MISSION_N_to_T){  //共通：傍観者＞ターゲット
  }
  else if(talk->ccew.intrude_talk_type >= INTRUDE_TALK_TYPE_MISSION_OFFSET_START){
    GF_ASSERT(EventCommFuncTalkTbl[talk->ccew.intrude_talk_type - INTRUDE_TALK_TYPE_MISSION_VICTORY_START] != NULL);
    EventCommFuncTalkTbl[talk->ccew.intrude_talk_type - INTRUDE_TALK_TYPE_MISSION_VICTORY_START](event, temp_ccew);
  }
  else{
    GF_ASSERT(0);
    //通常会話を起動しておく
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
  }
  else if(talk->ccew.intrude_talk_type == INTRUDE_TALK_TYPE_MISSION_N_to_T){  //共通：傍観者＞ターゲット
  }
  else if(talk->ccew.intrude_talk_type >= INTRUDE_TALK_TYPE_MISSION_OFFSET_START){
    GF_ASSERT(EventCommFuncTalkedTbl[talk->ccew.intrude_talk_type - INTRUDE_TALK_TYPE_MISSION_VICTORY_START] != NULL);
    EventCommFuncTalkedTbl[talk->ccew.intrude_talk_type - INTRUDE_TALK_TYPE_MISSION_VICTORY_START](event, temp_ccew);
  }
  else{
    GF_ASSERT(0);
    //通常会話を起動しておく
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
  if(MISSION_RecvCheck(&intcomm->mission) == TRUE){
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
    case MISSION_TYPE_SIZE:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_SIZE_START;
      break;
    case MISSION_TYPE_ATTRIBUTE:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_START;
      break;
    case MISSION_TYPE_ITEM:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_ITEM_START;
      break;
    case MISSION_TYPE_OCCUR:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_OCCUR_START;
      break;
    case MISSION_TYPE_PERSONALITY:
      intrude_talk_type += INTRUDE_TALK_TYPE_MISSION_PERSONALITY_START;
      break;
    }
    intrude_talk_type -= INTRUDE_TALK_TYPE_MISSION_OFFSET_START;
  }
  
  return intrude_talk_type;
}

