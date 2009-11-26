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
/**
 * @brief 別プロセス呼び出しイベント用ワーク定義
 */
//------------------------------------------------------------------
typedef struct _CALL_PROC_EVENT_WORK{
  FSOverlayID ov_id;
  const GFL_PROC_DATA * proc_data;
  void * pwk;
}CPEW;

//------------------------------------------------------------------
/**
 * @brief 別プロセス呼び出しイベント
 */
//------------------------------------------------------------------
static GMEVENT_RESULT callProcEvent( GMEVENT * event, int *seq, void * work)
{
  CPEW * cpew = (CPEW *)work;
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  switch(*seq)
  {
  case 0:
    GAMESYSTEM_CallProc(gsys, cpew->ov_id, cpew->proc_data, cpew->pwk);
    (*seq)++;
    break;
  case 1:
    if( GAMESYSTEM_IsProcExists( gsys ) == GFL_PROC_MAIN_NULL )
    {
      return GMEVENT_RES_FINISH;
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}
//------------------------------------------------------------------
/**
 * @brief 別プロセス呼び出し→ウェイト
 * @param parent     現在のイベント
 * @param ov_id     呼び出すプロセスのオーバーレイID指定
 * @param proc_data 呼び出すプロセスのPROC_DATAへのポインタ
 * @param pwk       プロセスに引き渡すワーク
 *
 * 内部でプロセス呼び出し→終了待ちをおこなうイベントを生成し、
 * そのイベントをCallする
 */
//------------------------------------------------------------------
void GMEVENT_CallProc( GMEVENT * parent, 
    FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * pwk)
{
  CPEW * cpew;
  GMEVENT * proc_event;
  proc_event = GMEVENT_Create( parent->gsys, NULL, callProcEvent, sizeof(CPEW) );
  cpew = GMEVENT_GetEventWork( proc_event );
  cpew->ov_id = ov_id;
  cpew->proc_data = proc_data;
  cpew->pwk = pwk;
  GMEVENT_CallEvent( parent, proc_event );
}

//------------------------------------------------------------------
/**
 * @brief オーバレイ配置の別イベント呼び出しイベント用ワーク定義
 */
//------------------------------------------------------------------
typedef struct _CALL_OVERLAY_EVENT_WORK{
  FSOverlayID ov_id;
  const GMEVENT* call_event;
  const GFL_PROC_DATA * proc_data;
  void * pwk;
}COEW;

//------------------------------------------------------------------
/**
 * @brief オーバーレイ配置の別イベント呼び出しイベント
 */
//------------------------------------------------------------------
static GMEVENT_RESULT callOverlayEvent( GMEVENT * event, int *seq, void * work)
{
  COEW * coew = (COEW *)work;
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork(event);
  switch(*seq)
  {
  case 0:
    GMEVENT_CallEvent( event, (GMEVENT*)coew->call_event );
    (*seq)++;
    break;
  case 1:
    GFL_OVERLAY_Unload( coew->ov_id );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

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
GMEVENT* GMEVENT_CreateOverlayEventCall( GAMESYS_WORK* gsys, 
    FSOverlayID ov_id, GMEVENT_CREATE_CB_FUNC cb_event_create_func, void* work )
{
  COEW * coew;
  GMEVENT *event;
  event = GMEVENT_Create(gsys, NULL, callOverlayEvent, sizeof(COEW) );

  coew = GMEVENT_GetEventWork( event );
  coew->ov_id = ov_id;

  GFL_OVERLAY_Load( ov_id );
  coew->call_event = (cb_event_create_func)( gsys, work );

  return event;
}

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

