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
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	イベントでのフェード種類指定
 *
 * 今のところダミーで設定している。速度なども指定するかも？
 */
//------------------------------------------------------------------
typedef enum {
	FIELD_FADE_BLACK = 0,
	FIELD_FADE_WHITE,
} FIELD_FADE_TYPE;

//------------------------------------------------------------------
/**
 * @brief	イベントでのフェード完了待ちの有無指定
 */
//------------------------------------------------------------------
typedef enum{
  FIELD_FADE_WAIT,    // フェード完了を待つ
  FIELD_FADE_NO_WAIT, // フェード完了を待たない
} FIELD_FADE_WAIT_TYPE;

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	フェードアウトイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldFadeOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                     FIELD_FADE_TYPE type, FIELD_FADE_WAIT_TYPE wait );

//------------------------------------------------------------------
/**
 * @brief	フェードインイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldFadeIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                    FIELD_FADE_TYPE type, FIELD_FADE_WAIT_TYPE wait );

//------------------------------------------------------------------
/**
 * @brief	ブライトネスアウトイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldBrightOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                     FIELD_FADE_TYPE type, FIELD_FADE_WAIT_TYPE wait );

//------------------------------------------------------------------
/**
 * @brief	ブライトネスインイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldBrightIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                    FIELD_FADE_TYPE type, FIELD_FADE_WAIT_TYPE wait );

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

//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出しイベント生成
 * @param	gsys		GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	ov_id		遷移するサブプロセスのオーバーレイ指定
 * @param	proc_data	遷移するサブプロセスのプロセスデータへのポインタ
 * @param	proc_work	遷移するサブプロセスで使用するワークへのポインタ
 * @return	GMEVENT		生成したイベントへのポインタ
 *
 * フェードアウト→フィールドマップ終了→サブプロセス呼び出し
 * →フィールドマップ再開→フェードインを処理する
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldSubProc(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
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
