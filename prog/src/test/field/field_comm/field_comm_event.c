//======================================================================
/*
 * @file	field_comm_event.c
 * @brief	通信会話処理
 * @author	ariizumi
 * @data	08/11/19
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field_comm_main.h"
#include "field_comm_event.h"


//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================
GMEVENT* FIELD_COMM_EVENT_StartTalk( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys );
static GMEVENT_RESULT FIELD_COMM_EVENT_StaltTalkEvent( GMEVENT *event , int *seq , void *work );

extern const int	FIELD_COMM_MAIN_GetWorkSize(void);
//--------------------------------------------------------------
//	話しかけるイベント開始	
//--------------------------------------------------------------
GMEVENT* FIELD_COMM_EVENT_StartTalk( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys )
{
	FIELD_COMM_MAIN * evtWork;
	GMEVENT * event;
	event = GMEVENT_Create(gameSys, NULL, FIELD_COMM_EVENT_StaltTalkEvent, FIELD_COMM_MAIN_GetWorkSize());
	evtWork = GMEVENT_GetEventWork(event);
	evtWork = commSys;
	return event;
}

//--------------------------------------------------------------
//	話しかけるイベント更新
//--------------------------------------------------------------
static GMEVENT_RESULT FIELD_COMM_EVENT_StaltTalkEvent( GMEVENT *event , int *seq , void *work )
{
	FIELD_COMM_MAIN *commSys = work;
	switch( *seq )
	{
	}
	return GMEVENT_RES_CONTINUE;
}

