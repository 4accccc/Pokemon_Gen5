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
#include "field_comm_menu.h"
#include "field_comm_func.h"
#include "field_comm_data.h"
#include "field_comm_event.h"

#include "msg/msg_d_field.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
struct _FIELD_COMM_EVENT
{
	FIELD_COMM_MAIN *commSys_;
};

//======================================================================
//	proto
//======================================================================
const int FIELD_COMM_EVENT_GetWorkSize(void);
void FIELD_COMM_EVENT_SetWorkData( FIELD_COMM_MAIN *commSys , FIELD_COMM_EVENT *commEvent );

GMEVENT* FIELD_COMM_EVENT_Start_StartCommNormal( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys );
GMEVENT_RESULT FIELD_COMM_EVENT_StartCommNormal( GMEVENT *event , int *seq , void *work );
GMEVENT* FIELD_COMM_EVENT_Start_StartCommInvasion( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys );
GMEVENT_RESULT FIELD_COMM_EVENT_StartCommInvasion( GMEVENT *event , int *seq , void *work );

GMEVENT* FIELD_COMM_EVENT_StartTalk( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys );
static GMEVENT_RESULT FIELD_COMM_EVENT_TalkEvent( GMEVENT *event , int *seq , void *work );
GMEVENT* FIELD_COMM_EVENT_StartTalkPartner( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys );
static GMEVENT_RESULT FIELD_COMM_EVENT_TalkEventPartner( GMEVENT *event , int *seq , void *work );

extern const int	FIELD_COMM_MAIN_GetWorkSize(void);
extern FIELD_COMM_FUNC* FIELD_COMM_MAIN_GetCommFuncWork( FIELD_COMM_MAIN *commSys );
extern FIELD_COMM_MENU** FIELD_COMM_MAIN_GetCommMenuWork( FIELD_COMM_MAIN *commSys );
extern const HEAPID FIELD_COMM_MAIN_GetHeapID( FIELD_COMM_MAIN *commSys );

const int FIELD_COMM_EVENT_GetWorkSize(void)
{
	return sizeof(FIELD_COMM_EVENT);
}
void FIELD_COMM_EVENT_SetWorkData( FIELD_COMM_MAIN *commSys , FIELD_COMM_EVENT *commEvent )
{
	commEvent->commSys_ = commSys;
}

//--------------------------------------------------------------
//	通信開始イベント	
//--------------------------------------------------------------
GMEVENT* FIELD_COMM_EVENT_Start_StartCommNormal( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys )
{
	GF_ASSERT(NULL);
	return NULL;
}
GMEVENT_RESULT FIELD_COMM_EVENT_StartCommNormal( GMEVENT *event , int *seq , void *work )
{
	FIELD_COMM_EVENT *evtWork = work;
	FIELD_COMM_MAIN *commSys = evtWork->commSys_;
	switch( *seq )
	{
	case 0:
		FIELD_COMM_MAIN_InitStartCommMenu( commSys );
		*seq += 1;
		break;
	case 1:
		if( FIELD_COMM_MAIN_LoopStartCommMenu( commSys ) == TRUE ){
			*seq += 1;
		}
		break;
	case 2:
		FIELD_COMM_MAIN_TermStartCommMenu( commSys );
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
//	侵入開始イベント	
//--------------------------------------------------------------
GMEVENT* FIELD_COMM_EVENT_Start_StartCommInvasion( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys )
{
	GF_ASSERT(NULL);
	return NULL;
}
GMEVENT_RESULT FIELD_COMM_EVENT_StartCommInvasion( GMEVENT *event , int *seq , void *work )
{
	FIELD_COMM_EVENT *evtWork = work;
	FIELD_COMM_MAIN *commSys = evtWork->commSys_;
	switch( *seq )
	{
	case 0:
		FIELD_COMM_MAIN_InitStartInvasionMenu( commSys );
		*seq += 1;
		break;
	case 1:
		if( FIELD_COMM_MAIN_LoopStartInvasionMenu( commSys ) == TRUE ){
			*seq += 1;
		}
		break;
	case 2:
		FIELD_COMM_MAIN_TermStartInvasionMenu( commSys );
		*seq += 1;
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}


//--------------------------------------------------------------
//	話しかけるイベント開始	
//--------------------------------------------------------------
GMEVENT* FIELD_COMM_EVENT_StartTalk( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys )
{
	FIELD_COMM_EVENT *evtWork;
	GMEVENT *event;
	event = GMEVENT_Create(gameSys, NULL, FIELD_COMM_EVENT_TalkEvent, sizeof(FIELD_COMM_EVENT));
	evtWork = GMEVENT_GetEventWork(event);
	evtWork->commSys_ = commSys;

	//最初のパケット送信をすでにしてしまう
	FIELD_COMM_MAIN_StartTalk( commSys );
	return event;
}

//--------------------------------------------------------------
//	話しかけるイベント更新
//--------------------------------------------------------------
static GMEVENT_RESULT FIELD_COMM_EVENT_TalkEvent( GMEVENT *event , int *seq , void *work )
{
	FIELD_COMM_EVENT *evtWork = work;
	FIELD_COMM_MAIN *commSys = evtWork->commSys_;
	FIELD_COMM_FUNC *commFunc = FIELD_COMM_MAIN_GetCommFuncWork( commSys );
	FIELD_COMM_MENU **commMenu = FIELD_COMM_MAIN_GetCommMenuWork( commSys );
	const F_COMM_TALK_STATE talkState = FIELD_COMM_DATA_GetTalkState( FCD_SELF_INDEX );
	switch( *seq )
	{
	case 0:
		if( talkState == FCTS_UNPOSSIBLE )
		{
			//キャンセルされた
			FIELD_COMM_DATA_SetTalkState( FCD_SELF_INDEX , FCTS_NONE );
			return GMEVENT_RES_FINISH;
		}
		else if( talkState == FCTS_ACCEPT )
		{
			//OK
			FIELD_COMM_DATA_SetTalkState( FCD_SELF_INDEX , FCTS_TALKING );
			
			*commMenu = FIELD_COMM_MENU_InitCommMenu( FIELD_COMM_MAIN_GetHeapID( commSys ) );
			FIELD_COMM_MENU_InitBG_MsgPlane( *commMenu );
			FIELD_COMM_MENU_OpenMessageWindow( FCM_BGPLANE_MSG_WINDOW , *commMenu );
			FIELD_COMM_MENU_SetMessage( DEBUG_FIELD_C_STR02 , *commMenu );

			*seq = 10;
		}
		break;
		//TODO 以下の処理、とりあえず親子一緒なのでまとめる？
	case 10:
		FIELD_COMM_MENU_UpdateMessageWindow( *commMenu );
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
		{
			//終了待ちへ
			FIELD_COMM_MENU_SetMessage( DEBUG_FIELD_C_STR04 , *commMenu );
			FIELD_COMM_DATA_SetTalkState( FCD_SELF_INDEX , FCTS_WAIT_END );
			*seq += 1;
		}
		break;
	case 11:
		{
			u8 partnerID;
			FIELD_COMM_FUNC_GetTalkParterData_ID( &partnerID , commFunc );
			FIELD_COMM_MENU_UpdateMessageWindow( *commMenu );
			if( FIELD_COMM_DATA_GetTalkState( partnerID ) == FCTS_WAIT_END )
			{
				FIELD_COMM_MENU_CloseMessageWindow( *commMenu );
				FIELD_COMM_MENU_TermBG_MsgPlane( *commMenu );
				FIELD_COMM_MENU_TermCommMenu( *commMenu );
				FIELD_COMM_DATA_SetTalkState( FCD_SELF_INDEX , FCTS_NONE );
				return GMEVENT_RES_FINISH;
			}
		}
		break;
	}
	//OS_TPrintf("TALK!\n");
	//return GMEVENT_RES_FINISH;
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
//	話しかけられるイベント開始	
//--------------------------------------------------------------
GMEVENT* FIELD_COMM_EVENT_StartTalkPartner( GAMESYS_WORK *gameSys , FIELD_COMM_MAIN *commSys )
{
	FIELD_COMM_EVENT *evtWork;
	GMEVENT *event;
	event = GMEVENT_Create(gameSys, NULL, FIELD_COMM_EVENT_TalkEventPartner, sizeof(FIELD_COMM_EVENT));
	evtWork = GMEVENT_GetEventWork(event);
	evtWork->commSys_ = commSys;

	FIELD_COMM_MAIN_StartTalkPartner( commSys );
	return event;
}

//--------------------------------------------------------------
//	話しかけられるイベント	
//--------------------------------------------------------------
static GMEVENT_RESULT FIELD_COMM_EVENT_TalkEventPartner( GMEVENT *event , int *seq , void *work )
{
	FIELD_COMM_EVENT *evtWork = work;
	FIELD_COMM_MAIN *commSys = evtWork->commSys_;
	FIELD_COMM_FUNC *commFunc = FIELD_COMM_MAIN_GetCommFuncWork( commSys );
	FIELD_COMM_MENU **commMenu = FIELD_COMM_MAIN_GetCommMenuWork( commSys );
	switch( *seq )
	{
	case 0:
		//OK
		FIELD_COMM_DATA_SetTalkState( FCD_SELF_INDEX , FCTS_TALKING );
		
		*commMenu = FIELD_COMM_MENU_InitCommMenu( FIELD_COMM_MAIN_GetHeapID( commSys ) );
		FIELD_COMM_MENU_InitBG_MsgPlane( *commMenu );
		FIELD_COMM_MENU_OpenMessageWindow( FCM_BGPLANE_MSG_WINDOW , *commMenu );
		FIELD_COMM_MENU_SetMessage( DEBUG_FIELD_C_STR03 , *commMenu );
		*seq = 10;
		break;

		//TODO 以下の処理、とりあえず親子一緒なのでまとめる？
	case 10:
		FIELD_COMM_MENU_UpdateMessageWindow( *commMenu );
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
		{
			//終了待ちへ
			FIELD_COMM_MENU_SetMessage( DEBUG_FIELD_C_STR04 , *commMenu );
			FIELD_COMM_DATA_SetTalkState( FCD_SELF_INDEX , FCTS_WAIT_END );
			*seq += 1;
		}
		break;
	case 11:
		{
			u8 partnerID;
			FIELD_COMM_FUNC_GetTalkParterData_ID( &partnerID , commFunc );
			FIELD_COMM_MENU_UpdateMessageWindow( *commMenu );
			if( FIELD_COMM_DATA_GetTalkState( partnerID ) == FCTS_WAIT_END )
			{
				FIELD_COMM_MENU_CloseMessageWindow( *commMenu );
				FIELD_COMM_MENU_TermBG_MsgPlane( *commMenu );
				FIELD_COMM_MENU_TermCommMenu( *commMenu );
				FIELD_COMM_DATA_SetTalkState( FCD_SELF_INDEX , FCTS_NONE );
				return GMEVENT_RES_FINISH;
			}
		}
		break;
	}
	//OS_TPrintf("TALKED!\n");
	//return GMEVENT_RES_FINISH;
	return GMEVENT_RES_CONTINUE;
}

