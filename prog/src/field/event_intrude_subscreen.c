//==============================================================================
/**
 * @file    event_intrude_subscreen.c
 * @brief   侵入によるサブスクリーン切り替えイベント
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
//  構造体定義
//==============================================================================
///イベント管理ワーク
typedef struct{
  GMEVENT *gmEvent;
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldWork;
  
  FIELD_SUBSCREEN_MODE change_mode;
  u8 seq;
  u8 padding[3];
}CISS_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GMEVENT_RESULT ChangeIntrudeSubScreenEvent( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT MissionPutEvent( GMEVENT *event, int *seq, void *wk );



//==============================================================================
//  イベント
//==============================================================================
//==================================================================
/**
 * 侵入によるサブスクリーン切り替えイベント起動
 *
 * @param   gsys		
 * @param   fieldWork		
 * @param   change_mode		切り替え後のサブスクリーンモード
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_ChangeIntrudeSubScreen(
  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_SUBSCREEN_MODE change_mode)
{
  CISS_WORK *ciss;
  GMEVENT * event;
  FIELD_SUBSCREEN_WORK *subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
  
  event = GMEVENT_Create(
    gsys, NULL, ChangeIntrudeSubScreenEvent, sizeof(CISS_WORK));
  
  ciss = GMEVENT_GetEventWork(event);
  MI_CpuClear8( ciss, sizeof(CISS_WORK) );
  
  ciss->gsys = gsys;
  ciss->gmEvent = event;
  ciss->fieldWork = fieldWork;
  ciss->change_mode = change_mode;
  
  return event;
}

//--------------------------------------------------------------
/**
 * イベント：侵入によるサブスクリーン切り替え
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ChangeIntrudeSubScreenEvent( GMEVENT *event, int *seq, void *wk )
{
  CISS_WORK *ciss = wk;
  switch (ciss->seq){
  case 0:
    if( FIELD_SUBSCREEN_CanChange( FIELDMAP_GetFieldSubscreenWork(ciss->fieldWork) ) == TRUE )
    {
      MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( ciss->fieldWork );
      GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
      MMDLSYS_PauseMoveProc( fldMdlSys );
      GAMEDATA_SetSubScreenType( gamedata , FMIT_POKEMON );
      FIELD_SUBSCREEN_Change(FIELDMAP_GetFieldSubscreenWork(ciss->fieldWork), ciss->change_mode);
      ciss->seq++;
    }
    break;
  case 1:
    if( FIELD_SUBSCREEN_CanChange( FIELDMAP_GetFieldSubscreenWork(ciss->fieldWork) ) == TRUE )
    {
      GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork( event );
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
      GAMEDATA_SetSubScreenMode(gamedata, ciss->change_mode);
      ciss->seq++;
    }
    break;
  case 2:
    {
      MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( ciss->fieldWork );
      MMDLSYS_ClearPauseMoveProc( fldMdlSys );
    }
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;

}


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
GMEVENT * EVENT_IntrudeTownWarp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT * event;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  int town_tblno;
  ZONEID warp_zone_id;
  VecFx32 pos;
  
  town_tblno = Intrude_GetWarpTown(game_comm);
  if(town_tblno == PALACE_TOWN_DATA_NULL){
    return NULL;
  }
  
  warp_zone_id = Intrude_GetPalaceTownZoneID(town_tblno);
  Intrude_GetPalaceTownRandPos(town_tblno, &pos);
  
  event = EVENT_ChangeMapPos(gsys, fieldWork, warp_zone_id, &pos, DIR_UP);
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
 * 現在のミッション表示イベント起動
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
      GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
      MYSTATUS *target_myst = GAMEDATA_GetMyStatusPlayer(gdata,intcomm->mission.data.target_netid);
      u16 msg_id;
      
      msg_id = MISSION_GetMissionMsgID(&intcomm->mission);
      if(talk->error == FALSE && intcomm->mission.data.mission_no != MISSION_NO_NULL){
        WORDSET_RegisterPlayerName( talk->iem.wordset, 0, target_myst );
      }

      IntrudeEventPrint_StartStream(&talk->iem, msg_id);
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
