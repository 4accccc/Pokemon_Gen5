//==============================================================================
/**
 * @file    event_mission_talk.c
 * @brief   ミッション：話しかけろ
 * @author  matsuda
 * @date    2010.02.21(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

#include "message.naix"

#include "script.h"
#include "event_fieldtalk.h"

#include "print/wordset.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/intrude_mission.h"
#include "field/field_comm/intrude_message.h"
#include "field\field_comm\intrude_mission_field.h"
#include "field\field_comm\intrude_work.h"
#include "msg/msg_invasion.h"
#include "msg/msg_mission_monolith.h"
#include "msg/msg_mission_msg.h"
#include "field/field_comm/intrude_battle.h"
#include "field/event_fieldmap_control.h" //EVENT_FieldSubProc
#include "item/itemsym.h"
#include "event_intrude.h"
#include "event_comm_common.h"
#include "event_mission_talk.h"
#include "event_gpower.h"
#include "event_comm_result.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "poke_tool/status_rcv.h"



//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	EVENT_MISSION_TALK
//--------------------------------------------------------------
typedef struct
{
  COMMTALK_COMMON_EVENT_WORK ccew;
	BOOL error;
	BOOL recv_achieve;
	TALK_TYPE talk_type;
}EVENT_MISSION_TALK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT CommMissionTalk_MtoT_Talk( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionTalk_TtoM_Talk( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionTalk_MtoT_Talked( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionTalk_TtoM_Talked( GMEVENT *event, int *seq, void *wk );


//==============================================================================
//  データ
//==============================================================================
static const struct{
  u16 target_talk[TALK_TYPE_MAX];           ///<自分がターゲットで話しかけた
  
  u16 target_talked[TALK_TYPE_MAX];         ///<自分がターゲットで話しかけられた
  u16 target_talked_get[TALK_TYPE_MAX];    ///<自分がターゲットでアイテムをもらった

  u16 mission_talked[TALK_TYPE_MAX];        ///<自分がミッション実行者で話しかけられた

  u16 mission_talk[TALK_TYPE_MAX];          ///<自分がミッション実行者で話しかけた
  u16 mission_talk_second[TALK_TYPE_MAX];   ///<自分がミッション実行者で話しかけた(二言目)
  u16 mission_talk_item[TALK_TYPE_MAX];     ///<自分がミッション実行者でアイテムをあげた
}MissionTalkMsgID = {
  { //自分がターゲットで話しかけた
    mis_m02_02_t1,
    mis_m02_02_t2,
    mis_m02_02_t3,
    mis_m02_02_t4,
    mis_m02_02_t5,
  },
  { //自分がターゲットで話しかけられた
    mis_m02_01_t1,
    mis_m02_01_t2,
    mis_m02_01_t3,
    mis_m02_01_t4,
    mis_m02_01_t5,
  },
  { //自分がターゲットでアイテムをもらった
    mis_m02_03_t1,
    mis_m02_03_t2,
    mis_m02_03_t3,
    mis_m02_03_t4,
    mis_m02_03_t5,
  },
  { //自分がミッション実行者で話しかけられた
    mis_m02_02_m1,
    mis_m02_02_m2,
    mis_m02_02_m3,
    mis_m02_02_m4,
    mis_m02_02_m5,
  },
  { //自分がミッション実行者で話しかけた
    mis_m02_01_m1,
    mis_m02_01_m2,
    mis_m02_01_m3,
    mis_m02_01_m4,
    mis_m02_01_m5,
  },
  { //自分がミッション実行者で話しかけた(長いのでメッセージ分割)
    mis_m02_01_m1,
    mis_m02_01_m2,
    mis_m02_01_m3,
    mis_m02_01_m4_01,
    mis_m02_01_m5_01,
  },
  { //自分がミッション実行者でアイテムをあげた
    mis_m02_03_m1,
    mis_m02_03_m2,
    mis_m02_03_m3,
    mis_m02_03_m4,
    mis_m02_03_m5,
  },
};



//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：話し掛けろ  (自分がミッション実行者でターゲットに話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionTalk_MtoT_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_TALK *talk;
  
  GMEVENT_Change(event, CommMissionTalk_MtoT_Talk, sizeof(EVENT_MISSION_TALK));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：話し掛けろ (自分がミッション実行者でターゲットに話しかけた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionTalk_MtoT_Talk( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_TALK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_SEND_ACHIEVE,
    SEQ_RECV_WAIT,    //通信はここまで。ここから下は通信不要
    SEQ_MSG_INIT,
    SEQ_MSG_SECOND,
    SEQ_MSG_WAIT,
    SEQ_LAST_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL && (*seq) <= SEQ_RECV_WAIT){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_LAST_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_LAST_MSG_WAIT;
      talk->error = TRUE;
      return GMEVENT_RES_CONTINUE;
    }
  }

	switch( *seq ){
	case SEQ_SEND_ACHIEVE:
    if(IntrudeSend_MissionAchieve(intcomm, &intcomm->mission) == TRUE){//ミッション達成を親に伝える
      (*seq)++;
    }
    break;
  case SEQ_RECV_WAIT:
		if(MISSION_GetAchieveAnswer(intcomm, &intcomm->mission) != MISSION_ACHIEVE_NULL){
      talk->recv_achieve = TRUE;
      (*seq)++;
    }
    break;
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( talk->ccew.iem.wordset, 0, &talk->ccew.talk_myst );
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 1, GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(gsys) ) );
    {
      const MISSION_TYPEDATA_BASIC *d_basic = (void*)talk->ccew.mdata.cdata.data;
      WORDSET_RegisterGPowerName( talk->ccew.iem.wordset, 2, d_basic->gpower_id );
    }
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionTalkMsgID.mission_talk[talk->ccew.disguise_talk_type]);
		(*seq)++;
		break;
  case SEQ_MSG_SECOND:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, 
        MissionTalkMsgID.mission_talk_second[talk->ccew.disguise_talk_type]);
  		(*seq)++;
  	}
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, 
        MissionTalkMsgID.mission_talk_item[talk->ccew.disguise_talk_type]);
      PMSND_PlaySE( SE_POWER_USE );
  		(*seq)++;
  	}
		break;
  case SEQ_LAST_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);

    GMEVENT_ChangeEvent(event, EVENT_CommMissionResult(gsys, talk->recv_achieve));
    return GMEVENT_RES_CONTINUE;  //ChangeEventで終了するためFINISHしない
  }
	return GMEVENT_RES_CONTINUE;
}

//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：話し掛けろ  (自分がターゲットでミッション実行者に話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionTalk_TtoM_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_TALK *talk;

  GMEVENT_Change(event, CommMissionTalk_TtoM_Talk, sizeof(EVENT_MISSION_TALK));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：話し掛けろ (自分がターゲットでミッション実行者に話しかけた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionTalk_TtoM_Talk( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_TALK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(0){  //intcomm不使用の為、エラーチェックの必要無し   intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_MSG_WAIT;
      talk->error = TRUE;
      return GMEVENT_RES_CONTINUE;
    }
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, &talk->ccew.talk_myst );
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 1, GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(gsys) ) );
    {
      const MISSION_TYPEDATA_BASIC *d_basic = (void*)talk->ccew.mdata.cdata.data;
      WORDSET_RegisterGPowerName( talk->ccew.iem.wordset, 2, d_basic->gpower_id );
    }
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionTalkMsgID.target_talk[talk->ccew.disguise_talk_type]);
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}


//======================================================================
//	
//======================================================================
//==================================================================
/**
 * イベント切り替え：話し掛けろ (自分がミッション実行者でターゲットから話しかけられた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionTalk_MtoT_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_TALK *talk;

  GMEVENT_Change(event, CommMissionTalk_MtoT_Talked, sizeof(EVENT_MISSION_TALK));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：話し掛けろ (自分がミッション実行者でターゲットから話しかけられた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionTalk_MtoT_Talked( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_TALK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(0){  //intcomm不使用の為、エラーチェックの必要無し   intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_MSG_WAIT;
      talk->error = TRUE;
      return GMEVENT_RES_CONTINUE;
    }
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, &talk->ccew.talk_myst );
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 1, GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(gsys) ) );
    {
      const MISSION_TYPEDATA_BASIC *d_basic = (void*)talk->ccew.mdata.cdata.data;
      WORDSET_RegisterGPowerName( talk->ccew.iem.wordset, 2, d_basic->gpower_id );
    }
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionTalkMsgID.mission_talked[talk->ccew.disguise_talk_type]);
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}



//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：話し掛けろ (自分がターゲットでミッション実行者から話しかけられた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionTalk_TtoM_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  EVENT_MISSION_TALK *talk;

  GMEVENT_Change(event, CommMissionTalk_TtoM_Talked, sizeof(EVENT_MISSION_TALK));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
}

//--------------------------------------------------------------
/**
 * ミッション：話し掛けろ (自分がターゲットでミッション実行者から話しかけられた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionTalk_TtoM_Talked( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_TALK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_GPOWER,
    SEQ_GPOWER_WAIT,
    SEQ_AFTER_MSG_INIT,
    SEQ_AFTER_MSG_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(0){  //intcomm不使用の為、エラーチェックの必要無し   intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_GPOWER){
      IntrudeEventPrint_SetupFieldMsg(&talk->ccew.iem, gsys); //途中のシーケンスで解放しているので
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_END;
      talk->error = TRUE;
      return GMEVENT_RES_CONTINUE;
    }
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, &talk->ccew.talk_myst );
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 1, GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(gsys) ) );
    {
      const MISSION_TYPEDATA_BASIC *d_basic = (void*)talk->ccew.mdata.cdata.data;
      WORDSET_RegisterGPowerName( talk->ccew.iem.wordset, 2, d_basic->gpower_id );
    }
    talk->talk_type = talk->ccew.disguise_talk_type;
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionTalkMsgID.target_talked[talk->talk_type]);
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
  		(*seq)++;
  	}
		break;

  case SEQ_GPOWER:
    //外部イベントを起動する為ウィンドウ類のみ先行して削除
    IntrudeEventPrint_ExitFieldMsg(&talk->ccew.iem);

    {
      GPOWER_EFFECT_PARAM param;
      GMEVENT* call_event;
      const MISSION_TYPEDATA_BASIC *d_basic = (void*)talk->ccew.mdata.cdata.data;
      param.g_power = d_basic->gpower_id;
      param.mine_f = FALSE;
      call_event = GMEVENT_CreateOverlayEventCall( gsys,
          FS_OVERLAY_ID( event_gpower), EVENT_GPowerEffectStart, &param );
      GMEVENT_CallEvent(event, call_event );
    }
    (*seq)++;
    break;
  case SEQ_GPOWER_WAIT:
    *seq = SEQ_AFTER_MSG_INIT;
		break;

  case SEQ_AFTER_MSG_INIT:
    IntrudeEventPrint_SetupFieldMsg(&talk->ccew.iem, gsys);
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, &talk->ccew.talk_myst );
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionTalkMsgID.target_talked_get[talk->talk_type]);
  	(*seq)++;
		break;
  case SEQ_AFTER_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      if(IntrudeEventPrint_LastKeyWait() == TRUE){
        (*seq)++;
      }
    }
    break;

  case SEQ_END:
  	//共通Finish処理
  	EVENT_CommCommon_Finish(intcomm, &talk->ccew);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

