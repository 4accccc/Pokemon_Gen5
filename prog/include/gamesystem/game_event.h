//============================================================================================
/**
 * @file	game_event.h
 * @brief	イベント制御用ヘッダ
 * @author	tamada
 * @date	2008.10.30	DPから移植（original 2005.07.22)
 */
//============================================================================================

#ifndef	__GAME_EVENT_H__
#define	__GAME_EVENT_H__

#include <gflib.h>
#include "gamesystem/gamesystem.h"

//=============================================================================
//
//	型定義
//
//=============================================================================
typedef enum {
	GMEVENT_RES_CONTINUE = 0,
	GMEVENT_RES_FINISH,
}GMEVENT_RESULT;
//-----------------------------------------------------------------------------
/**
 * @brief	フィールドイベント制御関数の型定義
 */
//-----------------------------------------------------------------------------
typedef GMEVENT_RESULT (*GMEVENT_FUNC)(GMEVENT *, int *, void *);

//=============================================================================
//
//	関数外部参照
//
//=============================================================================

extern BOOL GAMESYSTEM_EVENT_Main(GAMESYS_WORK * gsys);
extern BOOL GAMESYSTEM_EVENT_IsExists(GAMESYS_WORK * gsys);
extern GMEVENT * GAMESYSTEM_EVENT_Set(GAMESYS_WORK * repw, GMEVENT_FUNC event_func, u32 work_size);
extern void GMEVENT_Change(GMEVENT * event, GMEVENT_FUNC next_func, u32 work_size);
extern GMEVENT * GMEVENT_Call(GMEVENT * parent, GMEVENT_FUNC sub_func, u32 work_size);

extern GAMESYS_WORK * GMEVENT_GetGameSysWork(GMEVENT * event);
extern int * GMEVENT_GetSequenceWork(GMEVENT * event);
extern void * GMEVENT_GetEventWork(GMEVENT * event);



#endif /* __GAME_EVENT_H__ */
