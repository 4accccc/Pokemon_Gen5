//======================================================================
/*
 * @file	field_comm_event.h
 * @brief	í êMâÔòbèàóù
 * @author	ariizumi
 * @data	08/11/19
 */
//======================================================================
#ifndef FIELD_COMM_EVENT_H__
#define FIELD_COMM_EVENT_H__

#include "field_comm_def.h"

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
extern const int FIELD_COMM_EVENT_GetWorkSize(void);
extern void FIELD_COMM_EVENT_SetWorkData( FIELD_COMM_MAIN *commSys , FIELDMAP_WORK *fieldWork , FIELD_COMM_EVENT *commEvent );

extern GMEVENT_RESULT FIELD_COMM_EVENT_StartCommNormal( GMEVENT *event , int *seq , void *work );
extern GMEVENT_RESULT FIELD_COMM_EVENT_StartCommInvasion( GMEVENT *event , int *seq , void *work );

extern GMEVENT* FIELD_COMM_EVENT_StartTalk( GAMESYS_WORK *gameSys ,FIELDMAP_WORK *fieldWork , FIELD_COMM_MAIN *commSys );
extern GMEVENT* FIELD_COMM_EVENT_StartTalkPartner( GAMESYS_WORK *gameSys ,FIELDMAP_WORK *fieldWork , FIELD_COMM_MAIN *commSys );


#endif //FIELD_COMM_EVENT_H__
