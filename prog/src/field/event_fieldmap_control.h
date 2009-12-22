//============================================================================================
/**
 * @file	event_fieldmap_control.h
 * @brief	イベント：フィールドマップ制御ツール
 * @author	tamada GAMEFREAK inc.
 * @date	2008.12.10
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "app/pokelist.h" // PLIST_DATA
#include "app/p_status.h" // PSTATUS_DATA

//============================================================================================
// ■フェード処理
//============================================================================================
//  2009.12.22  event_field_fadeとして独立させた
#include "event_field_fade.h"

//============================================================================================
// ■フィールドマップの開始/終了
//============================================================================================

//------------------------------------------------------------------
/**
 * @brief	フィールドマップ開始イベントの生成
 * @param	gsys		GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @return	GMEVENT		生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldOpen(GAMESYS_WORK *gsys);

//------------------------------------------------------------------
/**
 * @brief	フィールドマップ終了イベント生成
 * @param	gsys		GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @return	GMEVENT		生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldClose(GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap);


//============================================================================================
// ■サブプロセス呼び出し
//============================================================================================

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
extern GMEVENT * EVENT_FieldSubProc( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * proc_work);

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
extern GMEVENT * EVENT_FieldSubProcNoFade( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * proc_work); 

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
extern GMEVENT * EVENT_FieldSubProc_Callback(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * proc_work,
    void (*callback)(void*), void* callback_work );

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
extern GMEVENT * EVENT_FieldSubProcNoFade_Callback(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * proc_work,
    void (*callback)(void*), void* callback_work );


//============================================================================================
// ■手持ちポケモン選択
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ポケモン選択イベント
 * @param	gsys		    GAMESYS_WORKへのポインタ
 * @param	fieldmap	  フィールドマップワークへのポインタ
 * @param list_data   ポケモンリスト画面    
 * @param status_data ポケモンステータス画面
 * @return	GMEVENT		生成したイベントへのポインタ
 *
 * フェードアウト→フィールドマップ終了→サブプロセス呼び出し
 * →フィールドマップ再開→フェードインを処理する
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_PokeSelect( 
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, 
    PLIST_DATA* list_data, PSTATUS_DATA* status_data );
