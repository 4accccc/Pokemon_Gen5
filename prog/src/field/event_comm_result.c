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
#include "event_comm_talk.h"
#include "field/field_comm/intrude_main.h"
#include "field/field_comm/intrude_comm_command.h"
#include "field/field_comm/bingo_system.h"
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
#include "event_comm_result.h"
#include "sound/pm_sndsys.h"
#include "savedata/intrude_save_field.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h"

#include "poke_tool/status_rcv.h"



//==============================================================================
//  定数定義
//==============================================================================
///ミッション達成で上がるレベル数
#define MISSION_ACHIEVE_ADD_LEVEL   (1)


//======================================================================
//	typedef struct
//======================================================================
//--------------------------------------------------------------
///	COMMTALK_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
	HEAPID heapID;
	FIELDMAP_WORK *fieldWork;
  
	INTRUDE_EVENT_MSGWORK iem;
	BOOL error;
	
	INTRUDE_EVENT_DISGUISE_WORK iedw;
	COMMTALK_COMMON_EVENT_WORK ccew;
	
	MISSION_RESULT mresult;
	u16 title_msgid;
	u16 explain_msgid;
	BOOL mission_result;    ///<TRUE:自分がミッション達成者
	s32 point;
	
	u8 send_occupy_req;
}COMMTALK_EVENT_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT CommMissionResultEvent( GMEVENT *event, int *seq, void *wk );



//==================================================================
/**
 * ミッション結果通知イベント起動(自分が達成ではなく通信相手が達成した結果が届いた)
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   intcomm		      侵入システムワークへのポインタ
 * @param   fmmdl_player		自機動作モデル
 * @param   heap_id		      ヒープID
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_CommMissionResult(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, HEAPID heap_id)
{
	COMMTALK_EVENT_WORK *talk;
	GMEVENT *event;
	
	event = GMEVENT_Create(
 		gsys, NULL,	CommMissionResultEvent, sizeof(COMMTALK_EVENT_WORK) );

	talk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( talk, sizeof(COMMTALK_EVENT_WORK) );
	
	talk->heapID = heap_id;
	talk->fieldWork = fieldWork;
	
  IntrudeEventPrint_SetupFieldMsg(&talk->iem, gsys);

  //イベント中、エラーになっても構わないようにintcommから必要なパラメータをここで全て取得
  //  ※BGMのPush,Popがある為、途中でイベントをやめるにはそこら辺のケアが必要になる
  {
    const MISSION_RESULT *mresult = MISSION_GetResultData(&intcomm->mission);
    
    GF_ASSERT(mresult != NULL);
    talk->mresult = *mresult;
    talk->title_msgid = msg_mistype_000 + mresult->mission_data.cdata.type;
    talk->explain_msgid = mresult->mission_data.cdata.msg_id_contents_monolith;
    
    talk->mission_result = MISSION_CheckResultMissionMine(intcomm, &intcomm->mission);
    talk->point = MISSION_GetResultPoint(intcomm, &intcomm->mission);
  }
  
	return( event );
}

//--------------------------------------------------------------
/**
 * ミッション結果通知イベント(自分が達成ではなく通信相手が達成した結果が届いた)
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT CommMissionResultEvent( GMEVENT *event, int *seq, void *wk )
{
	COMMTALK_EVENT_WORK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
  INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_INIT,
    SEQ_RESULT_KEY_WAIT,
    SEQ_POINT_GET_CHECK,   //報酬ゲットしたか
    SEQ_POINT_GET,         //報酬ゲット
    SEQ_POINT_GET_MSG_WAIT,
    SEQ_LEVELUP_MSG,
    SEQ_LEVELUP_MSG_WAIT,
    SEQ_POINT_GET_MSG_END_BUTTON_WAIT,
    SEQ_MISSION_FAIL,    //ミッション失敗
    SEQ_DISGUISE_START,  //変装戻す
    SEQ_DISGUISE_MAIN,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  // ※BGMのPush,Popがある為、途中でイベントは停止しない
  //   その為、intcommがNULLかどうかは、その使用ポイントでそれぞれNULL判定をして使用すること！！
  //   ※check　マスターアップ前にintcomm使用箇所でNULLチェックしているか調査する

	switch( *seq ){
  case SEQ_INIT:
    IntrudeEventPrint_SetupExtraMsgWin(&talk->iem, gsys, 1, 4, 32-2, 16);
    IntrudeEventPrint_PrintExtraMsgWin_MissionMono(&talk->iem, talk->title_msgid, 8, 0);
    IntrudeEventPrint_PrintExtraMsgWin_MissionMono(&talk->iem, talk->explain_msgid, 8, 16);
    (*seq)++;
    break;
  case SEQ_RESULT_KEY_WAIT:
    if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL)){
      IntrudeEventPrint_ExitExtraMsgWin(&talk->iem);
      (*seq) = SEQ_POINT_GET_CHECK;
    }
    break;

  case SEQ_POINT_GET_CHECK:   //報酬ゲットしたか
    if(talk->mission_result == TRUE){   //成功
      IntrudeEventPrint_Print(&talk->iem, msg_invasion_mission_sys004, 0, 0);
      GMEVENT_CallEvent(event, EVENT_FSND_PushPlayJingleBGM(gsys, SEQ_ME_MISSION_CLEAR ));
      *seq = SEQ_POINT_GET;
    }
    else{   //失敗
      IntrudeEventPrint_Print(&talk->iem, msg_invasion_mission_sys002, 0, 0);
      GMEVENT_CallEvent(event, EVENT_FSND_PushPlayJingleBGM(gsys, SEQ_ME_MISSION_FAILED ));
      *seq = SEQ_MISSION_FAIL;
    }
    break;
  case SEQ_POINT_GET:         //報酬ゲット
    if( PMSND_CheckPlayBGM() == FALSE ){
      GMEVENT_CallEvent(event, EVENT_FSND_PopBGM(gsys, FSND_FADE_NONE, FSND_FADE_SHORT));

      { //パレス球
        MYITEM_PTR myitem = GAMEDATA_GetMyItem(gdata);
        u16 now_num = MYITEM_GetItemNum( myitem, ITEM_PARESUDAMA, talk->heapID);
        int add_num = talk->point;
        
        if(MYITEM_GetItemMax( ITEM_PARESUDAMA ) < now_num + add_num){
          add_num = ITEM_PARESUDAMA - now_num;
        }
        MYITEM_AddItem(myitem, ITEM_PARESUDAMA, add_num, talk->heapID);
      }
      { //白黒レベル
        OCCUPY_INFO *occupy = GAMEDATA_GetMyOccupyInfo(gdata);
        if(talk->mresult.mission_data.monolith_type == MONOLITH_TYPE_BLACK){
          INTRUDE_OCCUPY_FIELD_LevelUpBlack(occupy, MISSION_ACHIEVE_ADD_LEVEL);
        }
        else{
          INTRUDE_OCCUPY_FIELD_LevelUpWhite(occupy, MISSION_ACHIEVE_ADD_LEVEL);
        }
      }

      WORDSET_RegisterNumber( talk->iem.wordset, 0, talk->point, 
        3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      IntrudeEventPrint_StartStream(&talk->iem, msg_invasion_mission_sys003);

      (*seq)++;
    }
    break;
  case SEQ_POINT_GET_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->iem) == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_LEVELUP_MSG:
    WORDSET_RegisterNumber( talk->iem.wordset, 0, MISSION_ACHIEVE_ADD_LEVEL, 
      3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    if(talk->mresult.mission_data.monolith_type == MONOLITH_TYPE_BLACK){
      IntrudeEventPrint_StartStream(&talk->iem, msg_invasion_mission_clear_03);
    }
    else{
      IntrudeEventPrint_StartStream(&talk->iem, msg_invasion_mission_clear_02);
    }
    (*seq)++;
    break;
  case SEQ_LEVELUP_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->iem) == TRUE){
      *seq = SEQ_POINT_GET_MSG_END_BUTTON_WAIT;
    }
    break;

  case SEQ_POINT_GET_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      if(MISSION_AddPoint(gdata, &talk->mresult, talk->point) == TRUE){
        talk->send_occupy_req = TRUE;
      }
      (*seq) = SEQ_DISGUISE_START;
    }
    break;

  case SEQ_MISSION_FAIL:    //ミッション失敗
    if( PMSND_CheckPlayBGM() == FALSE ){
      GMEVENT_CallEvent(event, EVENT_FSND_PopBGM(gsys, FSND_FADE_NONE, FSND_FADE_SHORT));
      *seq = SEQ_DISGUISE_START;
    }
    break;
    
  case SEQ_DISGUISE_START:  //変装戻す
    IntrudeEvent_Sub_DisguiseEffectSetup(&talk->iedw, gsys, talk->fieldWork, 
      DISGUISE_NO_NORMAL, 0, 0);
    (*seq)++;
    break;
  case SEQ_DISGUISE_MAIN:
    if(IntrudeEvent_Sub_DisguiseEffectMain(&talk->iedw, intcomm) == TRUE){
      (*seq)++;
    }
    break;

  case SEQ_END:
    IntrudeEventPrint_ExitFieldMsg(&talk->iem);

  #if 0
    //※check　ミッションが一つしかないので、ここで全回復
    if(intcomm->mission.data.target_info.net_id == GAMEDATA_GetIntrudeMyID(gdata)){
      STATUS_RCV_PokeParty_RecoverAll(GAMEDATA_GetMyPokemon(gdata));
    }
  #endif
    if(intcomm != NULL){
      if(talk->send_occupy_req == TRUE){
        intcomm->send_occupy = TRUE;
      }
      MISSION_Init(&intcomm->mission);
    }

    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

