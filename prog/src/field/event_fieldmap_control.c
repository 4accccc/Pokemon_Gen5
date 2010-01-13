//============================================================================================
/**
 * @file	event_fieldmap_control.c
 * @brief	イベント：フィールドマップ制御ツール
 * @author	tamada GAMEFREAK inc.
 * @date	2008.12.10
 */
//============================================================================================

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap_call.h"  //FIELDMAP_IsReady,FIELDMAP_ForceUpdate
#include "field/fieldmap.h"

#include "./event_fieldmap_control.h"
#include "./event_fieldmap_control_local.h"
#include "./event_gamestart.h"

#include "system/main.h"      // HEAPID_PROC


FS_EXTERN_OVERLAY(pokelist);
FS_EXTERN_OVERLAY(poke_status);



//============================================================================================
// ■フィールドマップの開始/終了
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	FIELDMAP_WORK * fieldmap;
	GAMEDATA * gamedata;

  BOOL unload_fieldinit;
}FIELD_OPENCLOSE_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT FieldCloseEvent(GMEVENT * event, int * seq, void *work)
{
	FIELD_OPENCLOSE_WORK * focw = work;
	GAMESYS_WORK * gsys = focw->gsys;
	FIELDMAP_WORK * fieldmap = focw->fieldmap;
	switch (*seq) {
	case 0:
		FIELDMAP_Close(fieldmap);
		(*seq) ++;
		break;
	case 1:
		if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL) break;
     
    if( focw->unload_fieldinit ){
      // フィールド初期化用オーバーレイ破棄
      GAMESTART_OVERLAY_FIELD_INIT_UnLoad();
    }
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldClose(GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldCloseEvent, sizeof(FIELD_OPENCLOSE_WORK));
	FIELD_OPENCLOSE_WORK * focw = GMEVENT_GetEventWork(event);
	focw->gsys = gsys;
	focw->fieldmap = fieldmap;
	focw->gamedata = GAMESYSTEM_GetGameData(gsys);
  focw->unload_fieldinit = TRUE;
	return event;
}

//------------------------------------------------------------------
// event_mapchange専用
//------------------------------------------------------------------
GMEVENT * EVENT_FieldClose_FieldProcOnly(GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldCloseEvent, sizeof(FIELD_OPENCLOSE_WORK));
	FIELD_OPENCLOSE_WORK * focw = GMEVENT_GetEventWork(event);
	focw->gsys = gsys;
	focw->fieldmap = fieldmap;
	focw->gamedata = GAMESYSTEM_GetGameData(gsys);
  focw->unload_fieldinit = FALSE;
	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT FieldOpenEvent(GMEVENT * event, int *seq, void*work)
{
	FIELD_OPENCLOSE_WORK * focw = work;
	GAMESYS_WORK * gsys = focw->gsys;
	FIELDMAP_WORK * fieldmap;	// = focw->fieldmap;
	switch(*seq) {
	case 0:
		if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL) break;
#if 0
		GFL_FADE_SetMasterBrightReq(
				GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,
        16, 16, 0);
#endif
		GAMESYSTEM_CallFieldProc(gsys);
		(*seq) ++;
		break;
	case 1:
		fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
		if (FIELDMAP_IsReady(fieldmap) == FALSE) break;
		(*seq) ++;
		break;
	case 2:
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldOpen(GAMESYS_WORK *gsys)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldOpenEvent, sizeof(FIELD_OPENCLOSE_WORK));
	FIELD_OPENCLOSE_WORK * focw = GMEVENT_GetEventWork(event);
	focw->gsys = gsys;
	focw->fieldmap = NULL;
	focw->gamedata = GAMESYSTEM_GetGameData(gsys);

  // フィールド初期化用オーバーレイ読み込み
  GAMESTART_OVERLAY_FIELD_INIT_Load();

	return event;
}

//------------------------------------------------------------------
// event_mapchange専用
//------------------------------------------------------------------
GMEVENT * EVENT_FieldOpen_FieldProcOnly(GAMESYS_WORK *gsys)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldOpenEvent, sizeof(FIELD_OPENCLOSE_WORK));
	FIELD_OPENCLOSE_WORK * focw = GMEVENT_GetEventWork(event);
	focw->gsys = gsys;
	focw->fieldmap = NULL;
	focw->gamedata = GAMESYSTEM_GetGameData(gsys);

	return event;
}



//============================================================================================
// ■サブプロセス呼び出し
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct
{
	GAMESYS_WORK*             gsys;
	FIELDMAP_WORK*        fieldmap;
	FSOverlayID              ov_id;
	const GFL_PROC_DATA* proc_data;
	void*                proc_work;

} CHANGE_SAMPLE_WORK;
//------------------------------------------------------------------
/**
 * @brief イベント処理関数
 */
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEvent( GMEVENT* event, int* seq, void* work )
{
	CHANGE_SAMPLE_WORK* csw = work;
	GAMESYS_WORK*      gsys = csw->gsys;

	switch( *seq )
  {
	case 0:
    { // フェードアウト
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, csw->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
		(*seq) ++;
		break;
	case 1:
    { // サブプロセス呼び出しイベント
      GMEVENT* ev_sub;
      ev_sub = EVENT_FieldSubProcNoFade( csw->gsys, csw->fieldmap, 
                                        csw->ov_id, csw->proc_data, csw->proc_work );
      GMEVENT_CallEvent( event, ev_sub );
    }
		(*seq) ++;
		break;
	case 2:
    { // フェードイン
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, csw->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
		(*seq) ++;
		break;
	case 3:
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}
//------------------------------------------------------------------
/**
 * @brief イベント処理関数(フェードなし)
 */
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEvent_NoFade(GMEVENT * event, int * seq, void * work)
{
	CHANGE_SAMPLE_WORK* csw = work;
	GAMESYS_WORK*      gsys = csw->gsys;

	switch( *seq )
  {
	case 0:  // フィールドマップ破棄
		GMEVENT_CallEvent( event, EVENT_FieldClose(gsys, csw->fieldmap) );
		(*seq) ++;
		break;
	case 1:  // サブプロセス呼び出し
		GAMESYSTEM_CallProc( gsys, csw->ov_id, csw->proc_data, csw->proc_work );
		(*seq) ++;
		break;
	case 2:  // サブプロセス終了待ち
		if( GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL ) break;
		(*seq) ++;
		break;
	case 3:  // フィールドマップ再開
		GMEVENT_CallEvent( event, EVENT_FieldOpen(gsys) );
		(*seq) ++;
		break;
	case 4:
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}


//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出しイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	ov_id		  遷移するサブプロセスのオーバーレイ指定
 * @param	proc_data	遷移するサブプロセスのプロセスデータへのポインタ
 * @param	proc_work	遷移するサブプロセスで使用するワークへのポインタ
 * @return 生成したイベントへのポインタ
 *
 * フェードアウト→フィールドマップ終了→サブプロセス呼び出し
 * →フィールドマップ再開→フェードインを処理する
 */
//------------------------------------------------------------------
GMEVENT* EVENT_FieldSubProc( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap,
		                         FSOverlayID ov_id, 
                             const GFL_PROC_DATA* proc_data, void* proc_work )
{
	GMEVENT* event;
	CHANGE_SAMPLE_WORK* csw; 

  // 生成
  event = GMEVENT_Create( gsys, NULL, GameChangeEvent, sizeof(CHANGE_SAMPLE_WORK) );
  // 初期化
	csw = GMEVENT_GetEventWork( event );
	csw->gsys      = gsys;
	csw->fieldmap  = fieldmap;
	csw->ov_id     = ov_id;
	csw->proc_data = proc_data;
	csw->proc_work = proc_work;
	return event;
}

//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出しイベント生成(フェードなし)
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	ov_id		  遷移するサブプロセスのオーバーレイ指定
 * @param	proc_data	遷移するサブプロセスのプロセスデータへのポインタ
 * @param	proc_work	遷移するサブプロセスで使用するワークへのポインタ
 * @return 生成したイベントへのポインタ
 *
 * フィールドマップ終了 → サブプロセス呼び出し → フィールドマップ再開
 */
//------------------------------------------------------------------
GMEVENT* EVENT_FieldSubProcNoFade( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap,
		                               FSOverlayID ov_id, 
                                   const GFL_PROC_DATA* proc_data, void* proc_work )
{
	GMEVENT* event;
	CHANGE_SAMPLE_WORK* csw; 

  // 生成
  event = GMEVENT_Create( gsys, NULL, GameChangeEvent_NoFade, sizeof(CHANGE_SAMPLE_WORK) );
  // 初期化
	csw = GMEVENT_GetEventWork( event );
	csw->gsys      = gsys;
	csw->fieldmap  = fieldmap;
	csw->ov_id     = ov_id;
	csw->proc_data = proc_data;
	csw->proc_work = proc_work;
	return event;
}


//============================================================================================
//
//	イベント：別画面呼び出し(コールバック呼び出し付き)
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct
{
	GAMESYS_WORK*             gsys;
	FIELDMAP_WORK*        fieldmap;
	FSOverlayID              ov_id;
	const GFL_PROC_DATA* proc_data;
	void*                proc_work;
  void ( *callback )( void* );      // コールバック関数
  void*            callback_work;   // コールバック関数の引数

} SUBPROC_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEvent_Callback(GMEVENT * event, int * seq, void * work)
{
	SUBPROC_WORK * spw = work;
	GAMESYS_WORK *gsys = spw->gsys;

	switch(*seq) 
  {
	case 0:
    { // サブプロセス呼び出しイベント
      GMEVENT* ev_sub;
      ev_sub = EVENT_FieldSubProc( spw->gsys, spw->fieldmap, 
                                  spw->ov_id, spw->proc_data, spw->proc_work );
      GMEVENT_CallEvent( event, ev_sub );
    }
		(*seq) ++;
		break;
	case 1: // コールバック関数呼び出し
    if( spw->callback ) spw->callback( spw->callback_work );
		(*seq) ++;
    break;
  case 2:  // コールバックワーク破棄
    if( spw->callback_work ){ GFL_HEAP_FreeMemory( spw->callback_work ); }
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
} 
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEventNoFade_Callback(GMEVENT * event, int * seq, void * work)
{
	SUBPROC_WORK * spw = work;
	GAMESYS_WORK *gsys = spw->gsys;

	switch(*seq) 
  {
	case 0:
    { // サブプロセス呼び出しイベント
      GMEVENT* ev_sub;
      ev_sub = EVENT_FieldSubProcNoFade( spw->gsys, spw->fieldmap, 
                                        spw->ov_id, spw->proc_data, spw->proc_work );
      GMEVENT_CallEvent( event, ev_sub );
    }
		(*seq) ++;
		break;
	case 1: // コールバック関数呼び出し
    if( spw->callback ) spw->callback( spw->callback_work );
		(*seq) ++;
    break;
  case 2:  // コールバックワーク破棄
    if( spw->callback_work ){ GFL_HEAP_FreeMemory( spw->callback_work ); }
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}


//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出しイベント生成(コールバック関数付き)
 * @param	gsys		      GAMESYS_WORKへのポインタ
 * @param	fieldmap	    フィールドマップワークへのポインタ
 * @param	ov_id		      遷移するサブプロセスのオーバーレイ指定
 * @param	proc_data	    遷移するサブプロセスのプロセスデータへのポインタ
 * @param	proc_work	    遷移するサブプロセスで使用するワークへのポインタ
 * @param callback      コールバック関数
 * @param callback_work コールバック関数にわたすポインタ
 * @return	GMEVENT		生成したイベントへのポインタ
 *
 * フェードアウト→フィールドマップ終了→サブプロセス呼び出し
 * →フィールドマップ再開→フェードインを処理する
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldSubProc_Callback(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * proc_work,
    void (*callback)(void*), void* callback_work )
{
	GMEVENT *     event = GMEVENT_Create(gsys, NULL, GameChangeEvent_Callback, sizeof(SUBPROC_WORK));
	SUBPROC_WORK *  spw = GMEVENT_GetEventWork(event);
	spw->gsys          = gsys;
	spw->fieldmap      = fieldmap;
	spw->ov_id         = ov_id;
	spw->proc_data     = proc_data;
	spw->proc_work     = proc_work;
  spw->callback      = callback;
  spw->callback_work = callback_work;
	return event;
}

//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出しイベント生成(フェードなし, コールバック関数付き)
 * @param	gsys		      GAMESYS_WORKへのポインタ
 * @param	fieldmap	    フィールドマップワークへのポインタ
 * @param	ov_id		      遷移するサブプロセスのオーバーレイ指定
 * @param	proc_data	    遷移するサブプロセスのプロセスデータへのポインタ
 * @param	proc_work	    遷移するサブプロセスで使用するワークへのポインタ
 * @param callback      コールバック関数
 * @param callback_work コールバック関数にわたすポインタ
 * @return	GMEVENT		生成したイベントへのポインタ
 *
 * フェードアウト→フィールドマップ終了→サブプロセス呼び出し
 * →フィールドマップ再開→フェードインを処理する
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldSubProcNoFade_Callback(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * proc_work,
    void (*callback)(void*), void* callback_work )
{
	GMEVENT*     event = GMEVENT_Create(gsys, NULL, GameChangeEventNoFade_Callback, sizeof(SUBPROC_WORK));
	SUBPROC_WORK*  spw = GMEVENT_GetEventWork(event);
	spw->gsys          = gsys;
	spw->fieldmap      = fieldmap;
	spw->ov_id         = ov_id;
	spw->proc_data     = proc_data;
	spw->proc_work     = proc_work;
  spw->callback      = callback;
  spw->callback_work = callback_work;
	return event;
}



//============================================================================================
//
//	イベント：ポケモン選択
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	FIELDMAP_WORK * fieldmap;
  PLIST_DATA* plData;
  PSTATUS_DATA* psData;
}POKE_SELECT_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_PokeSelect(GMEVENT * event, int * seq, void * work)
{
	POKE_SELECT_WORK * psw = work;
	GAMESYS_WORK *gsys = psw->gsys;

  // シーケンス定義
  enum
  {
    SEQ_CALL_POKELIST = 0,
    SEQ_WAIT_POKELIST,
    SEQ_EXIT_POKELIST,
    SEQ_CALL_POKESTATUS,
    SEQ_WAIT_POKESTATUS,
    SEQ_END,
  };

	switch( *seq ) 
  {
	case SEQ_CALL_POKELIST: //// ポケモンリスト呼び出し
		GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(pokelist), &PokeList_ProcData, psw->plData);
    *seq = SEQ_WAIT_POKELIST;
		break;
	case SEQ_WAIT_POKELIST: //// プロック終了待ち
		if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL) break;
    *seq = SEQ_EXIT_POKELIST;
		break;
  case SEQ_EXIT_POKELIST: //// ポケモンリストの終了状態で分岐
    if( psw->plData->ret_mode == PL_RET_NORMAL )   // 選択終了
    {
      *seq = SEQ_END;
    }
    else if( psw->plData->ret_mode == PL_RET_STATUS )  //「つよさをみる」を選択
    {
      psw->psData->pos = psw->plData->ret_sel;   // 表示するデータ位置 = 選択ポケ
      *seq = SEQ_CALL_POKESTATUS;
    }
    else  // 未対応な項目を選択
    {
      OBATA_Printf( "----------------------------------------------------\n" );
      OBATA_Printf( "ポケモン選択イベント: 未対応な項目が選択されました。\n" );
      OBATA_Printf( "----------------------------------------------------\n" );
      *seq = SEQ_END;
    }
    break;
  case SEQ_CALL_POKESTATUS: //// ポケモンステータス呼び出し
		GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(poke_status), &PokeStatus_ProcData, psw->psData);
    *seq = SEQ_WAIT_POKESTATUS;
		break;
	case SEQ_WAIT_POKESTATUS: //// プロック終了待ち
		if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL) break;
    psw->plData->ret_sel = psw->psData->pos;  // ステータス画面終了時のポケモンにカーソルを合わせる
    *seq = SEQ_CALL_POKELIST;
		break;
	case SEQ_END: //// イベント終了
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_PokeSelect( 
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    PLIST_DATA* list_data, PSTATUS_DATA* status_data )
{
	GMEVENT* event;
	POKE_SELECT_WORK* psw;

  // イベント生成
  event = GMEVENT_Create(gsys, NULL, EVENT_FUNC_PokeSelect, sizeof(POKE_SELECT_WORK));
  psw   = GMEVENT_GetEventWork(event);
	psw->gsys      = gsys;
	psw->fieldmap  = fieldmap;
  psw->plData    = list_data;
  psw->psData    = status_data;
	return event;
}
