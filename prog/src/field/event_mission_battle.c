//==============================================================================
/**
 * @file    event_mission_battle.c
 * @brief   ミッション：「ｘｘｘに勝負を挑め！」
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
#include "event_mission_battle.h"
#include "field/intrude_snd_def.h"
#include "event_comm_result.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "poke_tool/status_rcv.h"



//==============================================================================
//  定数定義
//==============================================================================
///バトルミッションで繰り出せる手持ちポケモン最大数
#define BATTLE_MISSION_POKE_MAX   (2)


//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	EVENT_MISSION_BATTLE
//--------------------------------------------------------------
typedef struct
{
  COMMTALK_COMMON_EVENT_WORK ccew;
	BOOL error;
	INTRUDE_BATTLE_PARENT ibp;
	int first_talked;
	BOOL success;
	BOOL cancel;
}EVENT_MISSION_BATTLE;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT CommMissionBattle_MtoT_Talk( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionBattle_TtoM_Talk( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionBattle_MtoT_Talked( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT CommMissionBattle_TtoM_Talked( GMEVENT *event, int *seq, void *wk );


//==============================================================================
//  データ
//==============================================================================
static const struct{
  u16 target_talk[TALK_TYPE_MAX];           ///<自分がターゲットで話しかけた
  u16 target_talked[TALK_TYPE_MAX];         ///<自分がターゲットで話しかけられた

  u16 target_battle_ok[TALK_TYPE_MAX];     ///<自分がターゲットで戦闘OK
  u16 target_battle_after[TALK_TYPE_MAX];  ///<自分がターゲットで戦闘後

  u16 target_battle_ng[TALK_TYPE_MAX];      ///<自分がターゲットで戦闘を断った

  u16 mission_talk[TALK_TYPE_MAX];          ///<自分がミッション実行者で話しかけた
  u16 mission_talked[TALK_TYPE_MAX];        ///<自分がミッション実行者で話しかけられた
  
  u16 mission_battle_ok[TALK_TYPE_MAX];     ///<自分がミッション実行者で戦闘OK
  u16 mission_battle_after[TALK_TYPE_MAX];  ///<自分がミッション実行者で戦闘後

  u16 mission_battle_ng[TALK_TYPE_MAX];     ///<自分がミッション実行者で戦闘を断られた
}MissionBattleMsgID = {
  { //自分がターゲットで話しかけた
    mis_m03_02_t1,
    mis_m03_02_t2,
    mis_m03_02_t3,
    mis_m03_02_t4,
    mis_m03_02_t5,
  },
  { //自分がターゲットで話しかけられた
    mis_m03_01_t1,
    mis_m03_01_t2,
    mis_m03_01_t3,
    mis_m03_01_t4,
    mis_m03_01_t5,
  },
  { //自分がターゲットで戦闘OK
    mis_m03_03_t1,
    mis_m03_03_t2,
    mis_m03_03_t3,
    mis_m03_03_t4,
    mis_m03_03_t5,
  },
  { //自分がターゲットで戦闘後
    mis_m03_04_t1,
    mis_m03_04_t2,
    mis_m03_04_t3,
    mis_m03_04_t4,
    mis_m03_04_t5,
  },
  { //自分がターゲットで戦闘を断った
    mis_m03_05_t1,
    mis_m03_05_t2,
    mis_m03_05_t3,
    mis_m03_05_t4,
    mis_m03_05_t5,
  },

  { //自分がミッション実行者で話しかけた
    mis_m03_01_m1,
    mis_m03_01_m2,
    mis_m03_01_m3,
    mis_m03_01_m4,
    mis_m03_01_m5,
  },
  { //自分がミッション実行者で話しかけられた
    mis_m03_02_m1,
    mis_m03_02_m2,
    mis_m03_02_m3,
    mis_m03_02_m4,
    mis_m03_02_m5,
  },
  { //自分がミッション実行者で戦闘OK
    mis_m03_03_m1,
    mis_m03_03_m2,
    mis_m03_03_m3,
    mis_m03_03_m4,
    mis_m03_03_m5,
  },
  { //自分がミッション実行者で戦闘後
    mis_m03_04_m1,
    mis_m03_04_m2,
    mis_m03_04_m3,
    mis_m03_04_m4,
    mis_m03_04_m5,
  },
  { //自分がミッション実行者で戦闘を断られた
    mis_m03_05_m1,
    mis_m03_05_m2,
    mis_m03_05_m3,
    mis_m03_05_m4,
    mis_m03_05_m5,
  },
};



//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：勝負を挑め  (自分がミッション実行者でターゲットに話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionBattle_MtoT_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  EVENT_MISSION_BATTLE *talk;
  const MISSION_TYPEDATA_VICTORY *d_vic = (void*)ccew->mdata.cdata.data;
  
  GMEVENT_Change(event, CommMissionBattle_MtoT_Talk, sizeof(EVENT_MISSION_BATTLE));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));

  talk->first_talked = FALSE;

  talk->ibp.gsys = gsys;
  talk->ibp.target_netid = ccew->talk_netid;
  talk->ibp.flat_level = d_vic->battle_level;
  talk->ibp.max_poke_num = BATTLE_MISSION_POKE_MAX;
}

//--------------------------------------------------------------
/**
 * ミッション：勝負を挑め (自分がミッション実行者でターゲットに話しかけた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionBattle_MtoT_Talk( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_BATTLE *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_BATTLE_OK,
    SEQ_SEND_ACHIEVE,
    SEQ_RECV_WAIT,
    SEQ_BATTLE_START_WAIT,
    SEQ_BATTLE_START_TIMING_WAIT,
    SEQ_BATTLE_START_LAST_TIMING,
    SEQ_BATTLE_START_LAST_TIMING_WAIT,
    SEQ_BATTLE_COMMON_EVENT,
    SEQ_BATTLE_AFTER_NEXT,
    SEQ_BATTLE_NG,
    SEQ_TALK_CANCEL,
    SEQ_LAST_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_LAST_MSG_WAIT){
      IntrudeEventPrint_SetupFieldMsg(&talk->ccew.iem, gsys); //途中のシーケンスで解放しているので
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_LAST_MSG_WAIT;
      talk->error = TRUE;
      return GMEVENT_RES_CONTINUE;
    }
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, &talk->ccew.talk_myst );
    WORDSET_RegisterNumber( talk->ccew.iem.wordset, 1, 
      talk->ibp.flat_level, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    {
      int msg_id;
      if(talk->first_talked == FALSE){
        msg_id = MissionBattleMsgID.mission_talk[talk->ccew.disguise_talk_type];
      }
      else{
        msg_id = MissionBattleMsgID.mission_talked[talk->ccew.disguise_talk_type];
      }
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_id);
    }
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      INTRUDE_TALK_STATUS talk_status = Intrude_GetTalkAnswer(intcomm);
      OS_TPrintf("menu_answer status = %d\n", talk_status);
      switch(talk_status){
      case INTRUDE_TALK_STATUS_BATTLE:
        *seq = SEQ_BATTLE_OK;
        break;
      case INTRUDE_TALK_STATUS_NG:
      case INTRUDE_TALK_STATUS_CANCEL:
        *seq = SEQ_BATTLE_NG;
        break;
      default:  //まだ返事が来ていない
        if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
          if(IntrudeSend_TalkCancel(intcomm->talk.talk_netid, intcomm->talk.talk_rand) == TRUE){
            Intrude_TalkRandClose(intcomm);
            *seq = SEQ_TALK_CANCEL;
          }
        }
        break;
      }
    }
    break;

  case SEQ_BATTLE_OK:
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionBattleMsgID.mission_battle_ok[talk->ccew.disguise_talk_type]);
  	(*seq) = SEQ_SEND_ACHIEVE;
		break;
	case SEQ_SEND_ACHIEVE:
    if(IntrudeSend_MissionAchieve(intcomm, &intcomm->mission) == TRUE){//ミッション達成を親に伝える
      (*seq)++;
    }
    break;
  case SEQ_RECV_WAIT:
		if(MISSION_GetAchieveAnswer(intcomm, &intcomm->mission) != MISSION_ACHIEVE_NULL){
      Intrude_SetTimeOutStopFlag(intcomm, TRUE);
      (*seq) = SEQ_BATTLE_START_WAIT;
    }
    break;
  case SEQ_BATTLE_START_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      if(IntrudeSend_MissionBattleTiming(intcomm, talk->ccew.talk_netid, MISSION_BATTLE_TIMING_BIT_FIRST) == TRUE){
        (*seq)++;
      }
    }
    break;
  case SEQ_BATTLE_START_TIMING_WAIT:
    if(Intrude_GetMissionBattleTimingNo(intcomm, MISSION_BATTLE_TIMING_BIT_FIRST, talk->ccew.talk_netid) == TRUE){
      (*seq) = SEQ_BATTLE_START_LAST_TIMING;
    }
    else{ //同期待ちBキャンセル
      if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
        if(IntrudeSend_TalkCancel(intcomm->talk.talk_netid, intcomm->talk.talk_rand) == TRUE){
          //ここでキャンセルしても相手には既にタイミングコマンドが届いている為、
          //先に進んでしまう。その為、通信切断へと処理を持っていく
          Intrude_TalkRandClose(intcomm);
          *seq = SEQ_TALK_CANCEL;
        }
      }
    }
    break;
  case SEQ_BATTLE_START_LAST_TIMING:    //キャンセルできない状態でもう一度同期
    if(IntrudeSend_MissionBattleTiming(intcomm, talk->ccew.talk_netid, MISSION_BATTLE_TIMING_BIT_SECOND) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_BATTLE_START_LAST_TIMING_WAIT:
    if(Intrude_GetMissionBattleTimingNo(intcomm, MISSION_BATTLE_TIMING_BIT_SECOND, talk->ccew.talk_netid) == TRUE){
      IntrudeEventPrint_ExitFieldMsg(&talk->ccew.iem);
      (*seq) = SEQ_BATTLE_COMMON_EVENT;
    }
    else if(Intrude_GetTalkAnswer(intcomm) == INTRUDE_TALK_STATUS_CANCEL){
      //相手からキャンセルが届いている
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_LAST_MSG_WAIT;
    }
    break;

  case SEQ_BATTLE_COMMON_EVENT:
    // 侵入通信対戦：共通イベント呼び出し
    talk->success = TRUE;
    GMEVENT_CallEvent(event, EVENT_IntrudeBattle_CallBattle( gsys, &talk->ibp ));
    (*seq)++;
    break;

  case SEQ_BATTLE_AFTER_NEXT:
    IntrudeEventPrint_SetupFieldMsg(&talk->ccew.iem, gsys);
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionBattleMsgID.mission_battle_after[talk->ccew.disguise_talk_type]);
    (*seq) = SEQ_LAST_MSG_WAIT;
    break;

  case SEQ_BATTLE_NG:
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionBattleMsgID.mission_battle_ng[talk->ccew.disguise_talk_type]);
    (*seq) = SEQ_LAST_MSG_WAIT;
    break;

	case SEQ_TALK_CANCEL:   //会話キャンセル
	  talk->cancel = TRUE;
	  *seq = SEQ_END;
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

    if(intcomm != NULL){
      Intrude_SetTimeOutStopFlag(intcomm, FALSE);
    }

    if(talk->success == TRUE){
      GMEVENT_ChangeEvent(event, EVENT_CommMissionResult(gsys, talk->success));
    }
    else if(talk->cancel == TRUE){
      return GMEVENT_RES_FINISH;    //キャンセルで終了
    }
    else{
      GMEVENT_ChangeEvent(
        event, EVENT_IntrudeForceWarpMyPalace(gsys, MISSION_FORCEWARP_MSGID_BATTLE_NG));
    }
    return GMEVENT_RES_CONTINUE;  //ChangeEventで終了するためFINISHしない
  }
	return GMEVENT_RES_CONTINUE;
}

//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：勝負を挑め  (自分がターゲットでミッション実行者に話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionBattle_TtoM_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  EVENT_MISSION_BATTLE *talk;
  const MISSION_TYPEDATA_VICTORY *d_vic = (void*)ccew->mdata.cdata.data;

  GMEVENT_Change(event, CommMissionBattle_TtoM_Talk, sizeof(EVENT_MISSION_BATTLE));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
  
  talk->first_talked = FALSE;

  talk->ibp.gsys = gsys;
  talk->ibp.target_netid = talk->ccew.talk_netid;
  talk->ibp.flat_level = d_vic->battle_level;
  talk->ibp.max_poke_num = BATTLE_MISSION_POKE_MAX;
}

//--------------------------------------------------------------
/**
 * ミッション：勝負を挑め (自分がターゲットでミッション実行者に話しかけた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionBattle_TtoM_Talk( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_BATTLE *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_BATTLE_YESNO_SELECT,
    SEQ_BATTLE_YES,
    SEQ_BATTLE_YES_MSG,
    SEQ_BATTLE_YES_WAIT,
    SEQ_BATTLE_START_TIMING_WAIT,
    SEQ_BATTLE_START_LAST_TIMING,
    SEQ_BATTLE_START_LAST_TIMING_WAIT,
    SEQ_BATTLE_COMMON_EVENT,
    SEQ_BATTLE_AFTER_NEXT,
    SEQ_BATTLE_NG,
    SEQ_BATTLE_NG_MSG,
    SEQ_TALK_CANCEL,   //会話キャンセル
    SEQ_LAST_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == FALSE){
      return GMEVENT_RES_CONTINUE;  //メッセージ描画中は待つ
    }
    if((*seq) < SEQ_LAST_MSG_WAIT){
      IntrudeEventPrint_SetupFieldMsg(&talk->ccew.iem, gsys); //途中のシーケンスで解放しているので
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_LAST_MSG_WAIT;
      talk->error = TRUE;
      return GMEVENT_RES_CONTINUE;
    }
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    WORDSET_RegisterPlayerName( 
      talk->ccew.iem.wordset, 0, &talk->ccew.talk_myst );
    WORDSET_RegisterNumber( talk->ccew.iem.wordset, 1, 
      talk->ibp.flat_level, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    {
      int msg_id;
      if(talk->first_talked == FALSE){
        msg_id = MissionBattleMsgID.target_talk[talk->ccew.disguise_talk_type];
      }
      else{
        msg_id = MissionBattleMsgID.target_talked[talk->ccew.disguise_talk_type];
      }
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_id);
    }
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      IntrudeEventPrint_SetupYesNo(&talk->ccew.iem, gsys);
      (*seq)++;
    }
    break;
  case SEQ_BATTLE_YESNO_SELECT:
    {
      FLDMENUFUNC_YESNO yesno = IntrudeEventPrint_SelectYesNo(&talk->ccew.iem);
      switch(yesno){
      case FLDMENUFUNC_YESNO_YES:
        IntrudeEventPrint_ExitMenu(&talk->ccew.iem);
        *seq = SEQ_BATTLE_YES;
        break;
      case FLDMENUFUNC_YESNO_NO:
        IntrudeEventPrint_ExitMenu(&talk->ccew.iem);
        *seq = SEQ_BATTLE_NG;
        break;
      }
    }
    break;
    
  case SEQ_BATTLE_YES:
    if(IntrudeSend_TalkAnswer(intcomm, intcomm->talk.talk_netid, 
        INTRUDE_TALK_STATUS_BATTLE, intcomm->talk.talk_rand) == TRUE){
      Intrude_SetTimeOutStopFlag(intcomm, TRUE);
      *seq = SEQ_BATTLE_YES_MSG;
    }
    break;
  case SEQ_BATTLE_YES_MSG:
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionBattleMsgID.target_battle_ok[talk->ccew.disguise_talk_type]);
    (*seq)++;
    break;
  case SEQ_BATTLE_YES_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
      if(IntrudeSend_MissionBattleTiming(intcomm, talk->ccew.talk_netid, MISSION_BATTLE_TIMING_BIT_FIRST) == TRUE){
        (*seq)++;
      }
    }
    break;
  case SEQ_BATTLE_START_TIMING_WAIT:
    if(Intrude_GetMissionBattleTimingNo(intcomm, MISSION_BATTLE_TIMING_BIT_FIRST, talk->ccew.talk_netid) == TRUE){
      (*seq) = SEQ_BATTLE_START_LAST_TIMING;
    }
    else{ //同期待ちBキャンセル
      if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
        if(IntrudeSend_TalkCancel(intcomm->talk.talk_netid, intcomm->talk.talk_rand) == TRUE){
          //ここでキャンセルしても相手には既にタイミングコマンドが届いている為、
          //先に進んでしまう。その為、通信切断へと処理を持っていく
          Intrude_TalkRandClose(intcomm);
          *seq = SEQ_TALK_CANCEL;
          GameCommSys_ExitReq(game_comm);
        }
      }
      else if(Intrude_GetTalkAnswer(intcomm) == INTRUDE_TALK_STATUS_CANCEL){
        //相手からキャンセルが届いている
        IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
        *seq = SEQ_LAST_MSG_WAIT;
      }
    }
    break;
  case SEQ_BATTLE_START_LAST_TIMING:    //キャンセルできない状態でもう一度同期
    if(IntrudeSend_MissionBattleTiming(intcomm, talk->ccew.talk_netid, MISSION_BATTLE_TIMING_BIT_SECOND) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_BATTLE_START_LAST_TIMING_WAIT:
    if(Intrude_GetMissionBattleTimingNo(intcomm, MISSION_BATTLE_TIMING_BIT_SECOND, talk->ccew.talk_netid) == TRUE){
      IntrudeEventPrint_ExitFieldMsg(&talk->ccew.iem);
      (*seq) = SEQ_BATTLE_COMMON_EVENT;
    }
    else if(Intrude_GetTalkAnswer(intcomm) == INTRUDE_TALK_STATUS_CANCEL){
      //相手からキャンセルが届いている
      IntrudeEventPrint_StartStream(&talk->ccew.iem, msg_intrude_004);
      *seq = SEQ_LAST_MSG_WAIT;
    }
    break;

  case SEQ_BATTLE_COMMON_EVENT:
    // 侵入通信対戦：共通イベント呼び出し
    GMEVENT_CallEvent(event, EVENT_IntrudeBattle_CallBattle( gsys, &talk->ibp ));
    (*seq)++;
    break;

  case SEQ_BATTLE_AFTER_NEXT:
    IntrudeEventPrint_SetupFieldMsg(&talk->ccew.iem, gsys);
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionBattleMsgID.target_battle_after[talk->ccew.disguise_talk_type]);
    (*seq) = SEQ_LAST_MSG_WAIT;
    break;

  case SEQ_BATTLE_NG:
    if(IntrudeSend_TalkAnswer(intcomm, intcomm->talk.talk_netid, 
        INTRUDE_TALK_STATUS_NG, intcomm->talk.talk_rand) == TRUE){
      //失敗なのでミッションを作り直す
      MISSION_LIST_Create_Type(
        GAMEDATA_GetMyOccupyInfo(GAMESYSTEM_GetGameData(gsys)), MISSION_TYPE_VICTORY);
      *seq = SEQ_BATTLE_NG_MSG;
    }
    break;
  case SEQ_BATTLE_NG_MSG:
    IntrudeEventPrint_StartStream(&talk->ccew.iem, 
      MissionBattleMsgID.target_battle_ng[talk->ccew.disguise_talk_type]);
    (*seq) = SEQ_LAST_MSG_WAIT;
    break;

	case SEQ_TALK_CANCEL:   //会話キャンセル
	  *seq = SEQ_END;
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
    if(intcomm != NULL){
      Intrude_SetTimeOutStopFlag(intcomm, FALSE);
    }
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：勝負を挑め  (自分がミッション実行者でターゲットから話しかけられた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionBattle_MtoT_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  const MISSION_TYPEDATA_VICTORY *d_vic = (void*)ccew->mdata.cdata.data;
  EVENT_MISSION_BATTLE *talk;
  
  GMEVENT_Change(event, CommMissionBattle_MtoT_Talk, sizeof(EVENT_MISSION_BATTLE));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));

  talk->first_talked = TRUE;

  talk->ibp.gsys = gsys;
  talk->ibp.target_netid = talk->ccew.talk_netid;
  talk->ibp.flat_level = d_vic->battle_level;
  talk->ibp.max_poke_num = BATTLE_MISSION_POKE_MAX;
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * イベント切り替え：勝負を挑め  (自分がターゲットでミッション実行者から話しかけられた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionBattle_TtoM_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  const MISSION_TYPEDATA_VICTORY *d_vic = (void*)ccew->mdata.cdata.data;
  EVENT_MISSION_BATTLE *talk;

  GMEVENT_Change(event, CommMissionBattle_TtoM_Talk, sizeof(EVENT_MISSION_BATTLE));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
  
  talk->first_talked = TRUE;

  talk->ibp.gsys = gsys;
  talk->ibp.target_netid = talk->ccew.talk_netid;
  talk->ibp.flat_level = d_vic->battle_level;
  talk->ibp.max_poke_num = BATTLE_MISSION_POKE_MAX;
}

