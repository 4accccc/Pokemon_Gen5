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
//-----------------------------------------------------------------------------
/**
 * @brief	イベント制御関数の戻り値定義
 */
//-----------------------------------------------------------------------------
typedef enum {
	GMEVENT_RES_CONTINUE = 0,
	GMEVENT_RES_FINISH,
}GMEVENT_RESULT;
//-----------------------------------------------------------------------------
/**
 * @brief	イベント制御関数の型定義
 */
//-----------------------------------------------------------------------------
typedef GMEVENT_RESULT (*GMEVENT_FUNC)(GMEVENT *, int *, void *);

//-----------------------------------------------------------------------------
/**
 * @brief	イベントチェック関数の型定義
 */
//-----------------------------------------------------------------------------
typedef GMEVENT * (*EVCHECK_FUNC)(GAMESYS_WORK *, void*);

//=============================================================================
//
//	関数外部参照
//
//=============================================================================

extern BOOL GAMESYSTEM_EVENT_Main(GAMESYS_WORK * gsys);
extern BOOL GAMESYSTEM_EVENT_IsExists(GAMESYS_WORK * gsys);
extern BOOL GAMESYSTEM_EVENT_CheckSet(GAMESYS_WORK * gsys, EVCHECK_FUNC ev_check, void * context);

//本体はgamesystem/gamesystem.cにあるので注意！
extern void GAMESYSTEM_EVENT_EntryCheckFunc(GAMESYS_WORK * gsys,
		EVCHECK_FUNC evcheck_func, void * context);

//------------------------------------------------------------------
/**
 * @brief	イベント生成
 * @param	gsys
 * @param	parent		親イベントへのポインタ
 * @param	event_func	イベント制御関数へのポインタ
 * @param	work		イベント制御関数の使用するワークへのポインタ
 * @return	GMEVENT	生成したイベントへのポインタ
 *
 */
//------------------------------------------------------------------
extern GMEVENT * GMEVENT_Create(GAMESYS_WORK * gsys,
		GMEVENT * parent, GMEVENT_FUNC event_func, u32 work_size);
//------------------------------------------------------------------
//------------------------------------------------------------------
extern GMEVENT_RESULT GMEVENT_Run(GMEVENT * event);
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void GMEVENT_Delete(GMEVENT * event);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void GMEVENT_Change(GMEVENT * event, GMEVENT_FUNC next_func, u32 work_size);
extern void GMEVENT_ChangeEvent(GMEVENT * now_event, GMEVENT * next_event);
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void GMEVENT_CallEvent(GMEVENT * parent, GMEVENT * child);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern GMEVENT * GMEVENT_GetParentEvent(GMEVENT * event);
extern GAMESYS_WORK * GMEVENT_GetGameSysWork(GMEVENT * event);
extern int * GMEVENT_GetSequenceWork(GMEVENT * event);
extern void * GMEVENT_GetEventWork(GMEVENT * event);



#endif /* __GAME_EVENT_H__ */
