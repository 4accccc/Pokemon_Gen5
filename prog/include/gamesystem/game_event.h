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
	GMEVENT_RES_CONTINUE = 0, ///<イベント継続中
	GMEVENT_RES_FINISH,       ///<イベント終了
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

//-----------------------------------------------------------------------------
/*
 * @brief Overlayイベントコール　イベント生成コールバック関数型
 */
//-----------------------------------------------------------------------------
typedef GMEVENT* (*GMEVENT_CREATE_CB_FUNC)( GAMESYS_WORK* gsys, void* work );

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
//------------------------------------------------------------------
extern GMEVENT_RESULT GMEVENT_Run(GMEVENT * event);
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void GMEVENT_Delete(GMEVENT * event);

//=============================================================================
//=============================================================================
//------------------------------------------------------------------
/**
 * @brief	イベント生成
 * @param	gsys
 * @param	parent		親イベントへのポインタ
 * @param	event_func	イベント制御関数へのポインタ
 * @param	work		イベント制御関数の使用するワークへのポインタ
 * @return	GMEVENT	生成したイベントへのポインタ
 *
 * @note
 * 引数parentは常時NULLを引き渡している。
 * いらないのかも…
 */
//------------------------------------------------------------------
extern GMEVENT * GMEVENT_Create(GAMESYS_WORK * gsys,
		GMEVENT * parent, GMEVENT_FUNC event_func, u32 work_size);


//------------------------------------------------------------------
//------------------------------------------------------------------
extern void GMEVENT_Change(GMEVENT * event, GMEVENT_FUNC next_func, u32 work_size);

//------------------------------------------------------------------
//  イベントからイベントへの切り替え
//------------------------------------------------------------------
extern void GMEVENT_ChangeEvent(GMEVENT * now_event, GMEVENT * next_event);

//------------------------------------------------------------------
//  イベントからイベントの呼び出し
//------------------------------------------------------------------
extern void GMEVENT_CallEvent(GMEVENT * parent, GMEVENT * child);

//------------------------------------------------------------------
//  イベントからプロセスの呼び出し
//------------------------------------------------------------------
extern void GMEVENT_CallProc( GMEVENT * parent, 
    FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * pwk);

//------------------------------------------------------------------
/**
 * @brief OverlayイベントCall→オーバーレイ解放
 * @param ov_id       呼び出すイベントのオーバーレイID指定
 * @param cb_event_create_func  内部でCallしたいイベントを生成するCallBack指定
 * @param cb_work イベント生成コールバックに引き渡すワーク 
 * 
 * 内部で指定のオーバーレイに配置されたイベントをCallし、
 * 終了後にオーバーレイ解放を行って終了するイベントを生成する
 */
//------------------------------------------------------------------
extern GMEVENT* GMEVENT_CreateOverlayEventCall( GAMESYS_WORK* gsys, 
    FSOverlayID ov_id, GMEVENT_CREATE_CB_FUNC cb_event_create_func, void* work );

//=============================================================================
//=============================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern GMEVENT * GMEVENT_GetParentEvent(GMEVENT * event);
extern GAMESYS_WORK * GMEVENT_GetGameSysWork(GMEVENT * event);
extern int * GMEVENT_GetSequenceWork(GMEVENT * event);
extern void * GMEVENT_GetEventWork(GMEVENT * event);



#endif /* __GAME_EVENT_H__ */
