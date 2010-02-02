//==============================================================================
/**
 * @file    event_intrude_subscreen.c
 * @brief   侵入によるサブスクリーンイベント
 * @author  matsuda
 * @date    2009.10.18(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/field_subscreen.h"
#include "event_intrude_subscreen.h"
#include "fieldmap.h"
#include "event_mapchange.h"
#include "print/wordset.h"
#include "field_comm/intrude_types.h"
#include "field_comm/intrude_main.h"
#include "field_comm/intrude_work.h"
#include "field_comm/intrude_mission.h"
#include "field_comm/intrude_message.h"
#include "msg/msg_invasion.h"



//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT MissionPutEvent( GMEVENT *event, int *seq, void *wk );




//==============================================================================
//  ワープイベント
//==============================================================================
//==================================================================
/**
 * 侵入によるサブスクリーン切り替えイベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_IntrudeTownWarp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, int town_tblno)
{
  GMEVENT * event;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  ZONEID warp_zone_id;
  VecFx32 pos;
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  int palace_area;
  
  if(town_tblno == PALACE_TOWN_DATA_NULL){
    return NULL;
  }
  
  if(intcomm == NULL){
    palace_area = GAMEDATA_GetIntrudeMyID(GAMESYSTEM_GetGameData(gsys));
  }
  else{
    palace_area = intcomm->intrude_status_mine.palace_area;
  }
  
  warp_zone_id = Intrude_GetPalaceTownZoneID(town_tblno);
  Intrude_GetPalaceTownRandPos(town_tblno, &pos, palace_area);
  
  event = EVENT_ChangeMapPos(gsys, fieldWork, warp_zone_id, &pos, DIR_UP, FALSE);
  return event;
}

//==================================================================
/**
 * 侵入によるサブスクリーン切り替えイベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_IntrudePlayerWarp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, int player_netid)
{
  GMEVENT * event;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  ZONEID warp_zone_id;
  VecFx32 pos;
	INTRUDE_COMM_SYS_PTR intcomm;

  intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL || (intcomm->recv_profile & (1 << player_netid)) == 0){
    return NULL;
  }
  
  warp_zone_id = intcomm->intrude_status[player_netid].zone_id;
  pos = intcomm->intrude_status[player_netid].player_pack.pos;
  pos.z += 32 << FX32_SHIFT;
  event = EVENT_ChangeMapPos(gsys, fieldWork, warp_zone_id, &pos, DIR_UP, FALSE);
  return event;
}


//==============================================================================
//
//  現在のミッション表示イベント
//
//==============================================================================
//--------------------------------------------------------------
//  構造体定義
//--------------------------------------------------------------
typedef struct
{
	HEAPID heapID;
	FIELDMAP_WORK *fieldWork;
  
	INTRUDE_EVENT_MSGWORK iem;
	BOOL error;
}COMMTALK_EVENT_WORK;

//==================================================================
/**
 * 現在のミッション表示イベント起動   ※check　削除候補 2010.02.02(火)
 *
 * @param   gsys		
 * @param   fieldWork		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_IntrudeMissionPut(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, HEAPID heap_id)
{
	COMMTALK_EVENT_WORK *ftalk_wk;
	GMEVENT *event;
	
  event = GMEVENT_Create( gsys, NULL, MissionPutEvent, sizeof(COMMTALK_EVENT_WORK) );
	
	ftalk_wk = GMEVENT_GetEventWork( event );
	GFL_STD_MemClear( ftalk_wk, sizeof(COMMTALK_EVENT_WORK) );
	
	ftalk_wk->heapID = heap_id;
	ftalk_wk->fieldWork = fieldWork;
	
	return( event );
}

//--------------------------------------------------------------
/**
 * 現在のミッション表示イベント
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event talk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT MissionPutEvent( GMEVENT *event, int *seq, void *wk )
{
	COMMTALK_EVENT_WORK *talk = wk;
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	INTRUDE_COMM_SYS_PTR intcomm;
	enum{
    SEQ_INIT,
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_MSG_END_BUTTON_WAIT,
    SEQ_END,
  };
	
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(intcomm == NULL){
    if((*seq) > SEQ_INIT && (*seq) < SEQ_MSG_WAIT){
      IntrudeEventPrint_StartStream(&talk->iem, msg_invasion_mission000);
      *seq = SEQ_MSG_WAIT;
      talk->error = TRUE;
    }
  }

	switch( *seq ){
  case SEQ_INIT:
    IntrudeEventPrint_SetupFieldMsg(&talk->iem, gsys);
    (*seq)++;
    break;
  case SEQ_MSG_INIT:
    {
    #if 0
      GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gdata,intcomm->mission.data.target_netid);
      u16 msg_id;
      
      msg_id = MISSION_GetMissionMsgID(&intcomm->mission);
      if(talk->error == FALSE && intcomm->mission.data.mission_no != MISSION_NO_NULL){
        WORDSET_RegisterPlayerName( talk->iem.wordset, 0, target_myst );
      }

      IntrudeEventPrint_StartStream(&talk->iem, msg_id);
    #else
      IntrudeEventPrint_StartStreamMission(&talk->iem, intcomm);
    #endif
    }
		(*seq)++;
		break;
  case SEQ_MSG_WAIT:
    if(IntrudeEventPrint_WaitStream(&talk->iem) == TRUE){
      *seq = SEQ_MSG_END_BUTTON_WAIT;
    }
    break;
  case SEQ_MSG_END_BUTTON_WAIT:
    if(IntrudeEventPrint_LastKeyWait() == TRUE){
      *seq = SEQ_END;
    }
    break;
  case SEQ_END:
    IntrudeEventPrint_ExitFieldMsg(&talk->iem);
    return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}
