//=============================================================================
/**
 * @file	game_event.c
 * @brief	イベント制御
 * @author	tamada
 * @date	2008.10.30	DPから移植（original 2005.07.22)
 */
//=============================================================================
#include <gflib.h>

#include "system/main.h"	//HEAPID参照のため

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

//=============================================================================
//=============================================================================
enum {
	HEAPID_LOCAL = HEAPID_PROC,
};
//-----------------------------------------------------------------------------
/**
 * ゲーム内イベントコントロール用ワーク定義
 */
//-----------------------------------------------------------------------------
struct _GMEVENT_CONTROL{
	GMEVENT * parent;	///<親（呼び出し元）のイベントへのポインタ
	GMEVENT_FUNC	func;	///<制御関数へのポインタ
	int seq;				///<シーケンスワーク
	void * work;			///<制御関数毎に固有ワークへのポインタ
	GAMESYS_WORK * gsys;	///<フィールド全体制御ワークへのポインタ（なるべく参照したくない）
};

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
 */
//------------------------------------------------------------------
GMEVENT * GMEVENT_Create(GAMESYS_WORK * gsys, GMEVENT * parent, GMEVENT_FUNC event_func, u32 work_size)
{
	GMEVENT * event;
	event = GFL_HEAP_AllocClearMemory(HEAPID_LOCAL, sizeof(GMEVENT));
	event->parent = parent;
	event->func = event_func;
	event->seq = 0;
	event->work = GFL_HEAP_AllocClearMemory(HEAPID_LOCAL, work_size);
	event->gsys = gsys;
	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT_RESULT GMEVENT_Run(GMEVENT * event)
{
	return event->func(event, &event->seq, event->work);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GMEVENT_Delete(GMEVENT * event)
{
	if (event->work) {
		GFL_HEAP_FreeMemory(event->work);
	}
	GFL_HEAP_FreeMemory(event);
}

//------------------------------------------------------------------
/**
 * @brief	イベント切替
 * @param	event		イベント制御ワークへのポインタ
 * @param	next_func	次のイベント制御関数へのポインタ
 * @param	work_size	次のイベント制御関数の使用するワークのサイズ
 */
//------------------------------------------------------------------
void GMEVENT_Change(GMEVENT * event, GMEVENT_FUNC next_func, u32 work_size)
{
	event->func = next_func;
	event->seq = 0;
	if (event->work) {
		GFL_HEAP_FreeMemory(event->work);
	}
	event->work = GFL_HEAP_AllocClearMemory(HEAPID_LOCAL, work_size);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GMEVENT_ChangeEvent(GMEVENT * now_event, GMEVENT * next_event)
{
	GAMESYS_WORK * gsys = now_event->gsys;
	next_event->parent = now_event->parent;
	GMEVENT_Delete(now_event);
	GAMESYSTEM_SetEvent(gsys, next_event);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GMEVENT_CallEvent(GMEVENT * parent, GMEVENT * child)
{
	child->parent = parent;
	GAMESYSTEM_SetEvent(parent->gsys, child);
}

//=============================================================================
//=============================================================================
//------------------------------------------------------------------
/**
 * @brief	イベント制御
 * @param	gsys
 * @retval	TRUE	イベント終了
 * @retval	FALSE	イベント継続中
 */
//------------------------------------------------------------------
BOOL GAMESYSTEM_EVENT_Main(GAMESYS_WORK * gsys)
{
	GMEVENT * event = GAMESYSTEM_GetEvent(gsys);
	if (event == NULL) {
		return FALSE;
	}
	while (GMEVENT_Run(event) == GMEVENT_RES_FINISH) {
		GMEVENT * parent = GMEVENT_GetParentEvent(event);
		GMEVENT_Delete(event);
		GAMESYSTEM_SetEvent(gsys, parent);
		event = parent;
		if (parent == NULL) {
			return TRUE;
		}
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	イベント起動中チェック
 * @param	gsys	フィールド制御ワークへのポインタ
 * @retval	TRUE	イベント起動中
 * @retval	FALSE	イベントなし
 */
//------------------------------------------------------------------
BOOL GAMESYSTEM_EVENT_IsExists(GAMESYS_WORK * gsys)
{
	return (GAMESYSTEM_GetEvent(gsys) != NULL);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL GAMESYSTEM_EVENT_CheckSet(GAMESYS_WORK * gsys, EVCHECK_FUNC ev_check, void * context)
{
	GMEVENT * event;
	if (GAMESYSTEM_GetEvent(gsys) != NULL){
		return FALSE;
	}
	if (ev_check == NULL) {
		return FALSE;
	}
	event = ev_check(gsys, context);
	if (event != NULL) {
		GAMESYSTEM_SetEvent(gsys, event);
		return TRUE;
	}
	return FALSE;
}

//=============================================================================
//
//	イベントコマンド
//
//=============================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------

#if 0
//extern BOOL FieldEvent_Cmd_WaitSubProcEnd(GAMESYS_WORK * gsys);
//extern void FieldEvent_Cmd_SetMapProc(GAMESYS_WORK * gsys);
//extern BOOL FieldEvent_Cmd_WaitMapProcStart(GAMESYS_WORK * gsys);

//extern void EventCmd_CallSubProc(GMEVENT * event, const GFL_PROC_DATA * proc_data, void * param);

//------------------------------------------------------------------
/**
 * @brief	サブプロセス動作終了待ち
 * @param	gsys	フィールド制御ワークへのポインタ
 * @retval	TRUE	サブプロセス動作中
 * @retval	FALSE	サブプロセス終了
 */
//------------------------------------------------------------------
BOOL FieldEvent_Cmd_WaitSubProcEnd(GAMESYS_WORK * gsys)
{
	if (GameSystem_CheckFieldProcExists(gsys) || GameSystem_CheckSubProcExists(gsys)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FieldEvent_Cmd_SetMapProc(GAMESYS_WORK * gsys)
{
	GameSystem_CreateFieldProc(gsys);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FieldEvent_Cmd_WaitMapProcStart(GAMESYS_WORK * gsys)
{
	if (GameSystem_CheckFieldMain(gsys)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

//=============================================================================
//
//=============================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	int seq;
	const GFL_PROC_DATA * proc_data;
	void * param;
}EV_SUBPROC_WORK;
//------------------------------------------------------------------
/**
 * @brief	サブイベント：サブプロセス呼び出し
 * @param	event		イベント制御ワークへのポインタ
 * @retval	TRUE		サブイベント終了
 * @retval	FALSE		サブイベント継続中
 */
//------------------------------------------------------------------
static BOOL GMEVENT_Sub_CallSubProc(GMEVENT * event)
{
	GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
	EV_SUBPROC_WORK * esw = GMEVENT_GetSpecialWork(event);
	switch (esw->seq) {
	case 0:
		GameSystem_StartSubProc(gsys, esw->proc_data, esw->param);
		esw->seq ++;
		break;
	case 1:
		if (FieldEvent_Cmd_WaitSubProcEnd(gsys)) {
			break;
		}
		GFL_HEAP_FreeMemory(esw);
		return TRUE;
	}
	return FALSE;
}
//------------------------------------------------------------------
/**
 * @brief	イベント擬似コマンド：サブプロセス呼び出し
 * @param	event		イベント制御ワークへのポインタ
 * @param	proc_data	プロセス定義データへのポインタ
 * @param	param		パラメータへのポインタ
 *
 * サブプロセスを呼び出して終了を待つ
 */
//------------------------------------------------------------------
void EventCmd_CallSubProc(GMEVENT * event, const GFL_PROC_DATA * proc_data, void * param)
{
	EV_SUBPROC_WORK * esw = GFL_HEAP_AllocMemory(HEAPID_LOCAL, sizeof(EV_SUBPROC_WORK));
	esw->seq = 0;
	esw->proc_data = proc_data;
	esw->param = param;
	FieldEvent_Call(event, GMEVENT_Sub_CallSubProc, esw);
}
#endif

//=============================================================================
//
//
//
//=============================================================================
//------------------------------------------------------------------
/**
 * @brief	ゲームシステム制御ワークへのポインタ取得
 * @param	event		イベント制御ワークへのポインタ
 * @return	GAMESYS_WORK	ゲームシステム制御ワークへのポインタ
 *
 * ゲームシステム制御ワークはポケモンゲーム内では最上位のワークエリアとなる。
 * この領域に直接参照することは「ゲームシステム内でのグローバル変数へのアクセス」と
 * 等しい。グローバル変数へのアクセスはなるべく避けるべきなので、この関数は
 * なるべく使用しないようにしたい。
 *
 * まずはプログラム構造を概念的に切り分けることを優先するため、
 * 基本的に必要な変数などへのアクセスは関数経由にする。
 * その上で不要なものは削除していき、速度的な問題がある場合はポインタ直接参照を
 * 考える。
 *
 */
//------------------------------------------------------------------
GAMESYS_WORK * GMEVENT_GetGameSysWork(GMEVENT * event)
{
	return event->gsys;
}

//------------------------------------------------------------------
/**
 * @brief	フィールドイベント個別のワークへのポインタ
 * @param	event		イベント制御ワークへのポインタ
 * @return	各イベント用ワーク
 */
//------------------------------------------------------------------
void * GMEVENT_GetEventWork(GMEVENT * event)
{
	return event->work;
}

//------------------------------------------------------------------
/**
 * @brief	シーケンスワークの取得
 * @param	event		イベント制御ワークへのポインタ
 * @return	シーケンスワークへのポインタ
 */
//------------------------------------------------------------------
int * GMEVENT_GetSequenceWork(GMEVENT * event)
{
	return &event->seq;
}
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
GMEVENT * GMEVENT_GetParentEvent(GMEVENT * event)
{
	return event->parent;
}

