//============================================================================================
/**
 * @file	game_event.h
 * @brief	イベント制御用ヘッダ
 * @author	tamada
 * @date	2005.07.22
 */
//============================================================================================

#ifndef	__GAME_EVENT_H__
#define	__GAME_EVENT_H__

#include <gflib.h>
//#include "field_common.h"
//#include "fieldobj.h"
//#include "system/procsys.h"
#include "gamesystem/gamesystem.h"

//=============================================================================
//
//	型定義
//
//=============================================================================
//-----------------------------------------------------------------------------
/**
 * @brief	フィールドイベント制御関数の型定義
 */
//-----------------------------------------------------------------------------
typedef BOOL (*GMEVENT_FUNC)(GMEVENT_CONTROL *, void *);

//=============================================================================
//
//	関数外部参照
//
//=============================================================================

extern GMEVENT_CONTROL * GMEVENT_Set(GAMESYS_WORK * repw, GMEVENT_FUNC event_func, void * work);
extern void GMEVENT_Change(GMEVENT_CONTROL * event, GMEVENT_FUNC next_func, void * work);
extern GMEVENT_CONTROL * GMEVENT_Call(GMEVENT_CONTROL * parent, GMEVENT_FUNC sub_func, void * work);

extern BOOL GMEVENT_Main(GAMESYS_WORK * repw);
extern BOOL GMEVENT_IsExists(GAMESYS_WORK * gsys);

extern GAMESYS_WORK * GMEVENT_GetGameSysWork(GMEVENT_CONTROL * event);
extern int * GMEVENT_GetSequenceWork(GMEVENT_CONTROL * event);
extern void * GMEVENT_GetSpecialWork(GMEVENT_CONTROL * event);
extern void * GMEVENT_GetLocalWork(GMEVENT_CONTROL * event, int size);


//extern BOOL FieldEvent_Cmd_WaitSubProcEnd(GAMESYS_WORK * gsys);
//extern void FieldEvent_Cmd_SetMapProc(GAMESYS_WORK * gsys);
//extern BOOL FieldEvent_Cmd_WaitMapProcStart(GAMESYS_WORK * gsys);

//extern void EventCmd_CallSubProc(GMEVENT_CONTROL * event, const GFL_PROC_DATA * proc_data, void * param);


#endif /* __GAME_EVENT_H__ */
