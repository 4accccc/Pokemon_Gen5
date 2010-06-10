//==============================================================================
/**
 * @file    event_mission_normal.c
 * @brief   ミッション：その他会話
 * @author  matsuda
 * @date    2010.04.19(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/main.h"

#include "message.naix"

#include "script.h"
#include "event_fieldtalk.h"

#include "print/wordset.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/intrude_battle.h"
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
#include "event_comm_notarget.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "poke_tool/status_rcv.h"



//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	EVENT_MISSION_NOTARGET
//--------------------------------------------------------------
typedef struct
{
  COMMTALK_COMMON_EVENT_WORK ccew;
	BOOL error;
	u16 msg_id;

	INTRUDE_TALK_STATUS answer_status;
	INTRUDE_BATTLE_PARENT ibp;
}EVENT_MISSION_NOTARGET;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _CommonEventChangeSet(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew, const u16 *msg_tbl);
static GMEVENT_RESULT CommMissionEtc_CommonEvent( GMEVENT *event, int *seq, void *wk );


//==============================================================================
//  データ
//==============================================================================
///戦闘状態の相手に話しかけた(ミッション実行者側)
ALIGN4 static const u16 Talk_M_Battle[TALK_TYPE_MAX] = {
  mis_btl_01_m1,
  mis_btl_01_m2,
  mis_btl_01_m3,
  mis_btl_01_m4,
  mis_btl_01_m5,
};

///話しかけが出来ない相手に話しかけた(ミッション実行者側)
ALIGN4 static const u16 Talk_M_NotTalk[TALK_TYPE_MAX] = {
  mis_std_01_m1,
  mis_std_01_m2,
  mis_std_01_m3,
  mis_std_01_m4,
  mis_std_01_m5,
};

///ミッション参加者同士の話しかけ(話しかけた側)
ALIGN4 static const u16 Talk_MissionMission[MISSION_TYPE_MAX][TALK_TYPE_MAX] = {
  {//MISSION_TYPE_VICTORY
    mis_m03_06_m1,
    mis_m03_06_m2,
    mis_m03_06_m3,
    mis_m03_06_m4,
    mis_m03_06_m5,
  },
  {//MISSION_TYPE_SKILL
    mis_m01_04_m1,
    mis_m01_04_m2,
    mis_m01_04_m3,
    mis_m01_04_m4,
    mis_m01_04_m5,
  },
  {//MISSION_TYPE_BASIC
    mis_m02_04_m1,
    mis_m02_04_m2,
    mis_m02_04_m3,
    mis_m02_04_m4,
    mis_m02_04_m5,
  },
  {//MISSION_TYPE_ATTRIBUTE
    mis_m05_01_m1,
    mis_m05_01_m2,
    mis_m05_01_m3,
    mis_m05_01_m4,
    mis_m05_01_m5,
  },
  {//MISSION_TYPE_ITEM
    mis_m06_06_m1,
    mis_m06_06_m2,
    mis_m06_06_m3,
    mis_m06_06_m4,
    mis_m06_06_m5,
  },
  {//MISSION_TYPE_PERSONALITY
    mis_m04_05_m1,
    mis_m04_05_m2,
    mis_m04_05_m3,
    mis_m04_05_m4,
    mis_m04_05_m5,
  },
};

///ミッション参加者同士の話しかけ(話しかけられた側)
ALIGN4 static const u16 Talked_MissionMission[MISSION_TYPE_MAX][TALK_TYPE_MAX] = {
  {//MISSION_TYPE_VICTORY
    mis_m03_06_t1,
    mis_m03_06_t2,
    mis_m03_06_t3,
    mis_m03_06_t4,
    mis_m03_06_t5,
  },
  {//MISSION_TYPE_SKILL
    mis_m01_04_t1,
    mis_m01_04_t2,
    mis_m01_04_t3,
    mis_m01_04_t4,
    mis_m01_04_t5,
  },
  {//MISSION_TYPE_BASIC
    mis_m02_04_t1,
    mis_m02_04_t2,
    mis_m02_04_t3,
    mis_m02_04_t4,
    mis_m02_04_t5,
  },
  {//MISSION_TYPE_ATTRIBUTE
    mis_m05_02_t1,
    mis_m05_02_t2,
    mis_m05_02_t3,
    mis_m05_02_t4,
    mis_m05_02_t5,
  },
  {//MISSION_TYPE_ITEM
    mis_m06_06_t1,
    mis_m06_06_t2,
    mis_m06_06_t3,
    mis_m06_06_t4,
    mis_m06_06_t5,
  },
  {//MISSION_TYPE_PERSONALITY
    mis_m04_05_t1,
    mis_m04_05_t2,
    mis_m04_05_t3,
    mis_m04_05_t4,
    mis_m04_05_t5,
  },
};

///傍観者とミッション参加者の会話(ミッション参加者側)
ALIGN4 static const u16 Talk_M_Boukansya[TALK_TYPE_MAX] = {
  mis_nom_01_m1,
  mis_nom_01_m2,
  mis_nom_01_m3,
  mis_nom_01_m4,
  mis_nom_01_m5,
};

///傍観者とミッション参加者の会話(傍観者側)
ALIGN4 static const u16 Talk_N_Boukansya[TALK_TYPE_MAX] = {
  mis_nom_01_t1,
  mis_nom_01_t2,
  mis_nom_01_t3,
  mis_nom_01_t4,
  mis_nom_01_t5,
};


//--------------------------------------------------------------
/**
 * イベント切り替え：共通処理
 *
 * @param   event		
 * @param   ccew		
 * @param   msg_tbl		メッセージIDのテーブル
 */
//--------------------------------------------------------------
static void _CommonEventChangeSet(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew, const u16 *msg_tbl)
{
 	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
  EVENT_MISSION_NOTARGET *talk;

  intcomm = Intrude_Check_CommConnect(game_comm);
  
  GMEVENT_Change(event, CommMissionEtc_CommonEvent, sizeof(EVENT_MISSION_NOTARGET));
	talk = GMEVENT_GetEventWork( event );
  GFL_STD_MemCopy(ccew, &talk->ccew, sizeof(COMMTALK_COMMON_EVENT_WORK));
	
  if(intcomm != NULL){  //このタイミングでintcommがNULLの場合は無いが一応
  	talk->msg_id = msg_tbl[ccew->disguise_talk_type];
    WORDSET_RegisterPlayerName( //イベント中にintcommに依存しなくて良いようにここでセット
      ccew->iem.wordset, 0, &ccew->talk_myst );
  }
  else{
    GF_ASSERT(0);
  }
}

//--------------------------------------------------------------
/**
 * ミッション：共通処理：talk->msg_idに設定されているメッセージを出力して終了
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionEtc_CommonEvent( GMEVENT *event, int *seq, void *wk )
{
	EVENT_MISSION_NOTARGET *talk = wk;
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
  if(intcomm == NULL){
    //intcommに依存していないのでチェックする必要はなし
  }

	switch( *seq ){
  case SEQ_MSG_INIT:
    IntrudeEventPrint_StartStream(&talk->ccew.iem, talk->msg_id);
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->ccew.iem) == TRUE){
  		(*seq)++;
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
//  戦闘状態の相手に話しかけた
//==============================================================================
//==================================================================
/**
 * イベント切り替え：戦闘状態の相手に話しかけた(自分がミッション実行者でターゲットに話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionEtc_MtoT_Battle_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  _CommonEventChangeSet(event, ccew, Talk_M_Battle);
}

//==================================================================
/**
 * イベント切り替え：話しかけが出来ない状態の相手に話しかけた(自分がミッション実行者でターゲットに話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionEtc_MtoT_NotTalk_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  _CommonEventChangeSet(event, ccew, Talk_M_NotTalk);
}

//==================================================================
/**
 * イベント切り替え：ミッション参加者同士の話しかけ(自分がミッション実行者でミッション実行者に話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionEtc_MtoM_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  MISSION_TYPE mission_type = ccew->mdata.cdata.type;
  
  if(mission_type >= MISSION_TYPE_MAX){
    mission_type = 0;
  }
  _CommonEventChangeSet(event, ccew, Talk_MissionMission[mission_type]);
}

//==================================================================
/**
 * イベント切り替え：ミッション参加者同士の話しかけ(自分がミッション実行者でミッション実行者に話しかけられた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionEtc_MtoM_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  MISSION_TYPE mission_type = ccew->mdata.cdata.type;
  
  if(mission_type >= MISSION_TYPE_MAX){
    mission_type = 0;
  }
  _CommonEventChangeSet(event, ccew, Talked_MissionMission[mission_type]);
}

//==================================================================
/**
 * イベント切り替え：傍観者がミッション参加者に話しかけた(自分がミッション実行者で傍観者に話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionEtc_MtoN_M(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  _CommonEventChangeSet(event, ccew, Talk_M_Boukansya);
}

//==================================================================
/**
 * イベント切り替え：傍観者がミッション参加者に話しかけた(自分が傍観者でミッション実行者に話しかけた)
 *
 * @param   event		
 * @param   ccew		
 */
//==================================================================
void EVENTCHANGE_CommMissionEtc_MtoN_N(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew)
{
  _CommonEventChangeSet(event, ccew, Talk_N_Boukansya);
}
