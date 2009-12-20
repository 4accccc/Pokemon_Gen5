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
// ■フェードパラメータ
//============================================================================================

//------------------------------------------------------------------
/**
 * @brief	イベントでのフェード種類指定
 *
 * 今のところダミーで設定している。速度なども指定するかも？
 */
//------------------------------------------------------------------
typedef enum {
	FIELD_FADE_BLACK,  // 輝度フェード(ブラック)
	FIELD_FADE_WHITE,  // 輝度フェード(ホワイト)
  FIELD_FADE_CROSS,  // クロスフェード
} FIELD_FADE_TYPE;

//------------------------------------------------------------------
/**
 * @brief	イベントでの季節表示の有無指定フラグ
 */
//------------------------------------------------------------------
typedef enum{
  FIELD_FADE_SEASON_ON,   // 季節表示を許可する
  FIELD_FADE_SEASON_OFF,  // 季節表示を許可しない
} FIELD_FADE_SEASON_FLAG;

//------------------------------------------------------------------
/**
 * @brief	輝度フェードの完了待ちフラグ
 */
//------------------------------------------------------------------
typedef enum{
  FIELD_FADE_WAIT,    // フェード完了を待つ
  FIELD_FADE_NO_WAIT, // フェード完了を待たない
} FIELD_FADE_WAIT_TYPE;


//============================================================================================
// ■基本フェード関数
//============================================================================================

//------------------------------------------------------------------
/**
 * @brief	フェードアウトイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか(輝度フェード時のみ有効)
 * @return	GMEVENT	生成したイベントへのポインタ*/
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldFadeOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                     FIELD_FADE_TYPE type, 
                                     FIELD_FADE_WAIT_TYPE wait );
// クロスフェード
#define EVENT_FieldFadeOut_Cross( gsys, fieldmap ) \
        EVENT_FieldFadeOut( gsys, fieldmap, FIELD_FADE_CROSS, 0 )
// 輝度フェード(ブラック)
#define EVENT_FieldFadeOut_Black( gsys, fieldmap, wait ) \
        EVENT_FieldFadeOut( gsys, fieldmap, FIELD_FADE_BLACK, wait )
// 輝度フェード(ホワイト)
#define EVENT_FieldFadeOut_White( gsys, fieldmap, wait ) \
        EVENT_FieldFadeOut( gsys, fieldmap, FIELD_FADE_WHITE, wait )

//------------------------------------------------------------------
/**
 * @brief	フェードインイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param season    季節表示を許可するかどうか(輝度フェード時のみ有効)
 * @param wait      フェード完了を待つかどうか(輝度フェード時のみ有効)
 * @param bg_init_flag  BG初期化を内部でおこなうかどうか(輝度フェード時のみ有効)
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldFadeIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                    FIELD_FADE_TYPE type, 
                                    FIELD_FADE_SEASON_FLAG season,
                                    FIELD_FADE_WAIT_TYPE wait,
                                    BOOL bg_init_flag );
// クロスフェード
#define EVENT_FieldFadeIn_Cross( gsys, fieldmap ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_CROSS, 0, 0, TRUE )
// 輝度フェード(ブラック)
#define EVENT_FieldFadeIn_Black( gsys, fieldmap, wait ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_BLACK, FIELD_FADE_SEASON_OFF, wait, TRUE )
// 輝度フェード(ホワイト)
#define EVENT_FieldFadeIn_White( gsys, fieldmap, wait ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_WHITE, FIELD_FADE_SEASON_OFF, wait, TRUE )
// 季節フェード
#define EVENT_FieldFadeIn_Season( gsys, fieldmap ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_BLACK, FIELD_FADE_SEASON_ON, 0, TRUE )
// メニュー状態フェード（Yボタンメニューを出したままなど）
#define EVENT_FieldFadeIn_Menu( gsys, fieldmap, wait ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_BLACK, FIELD_FADE_SEASON_OFF, wait, FALSE )


//============================================================================================
// ■例外的なフェード関数
//============================================================================================

//------------------------------------------------------------------
/**
 * @brief	空を飛ぶブライトネスアウトイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FlySkyBrightOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                       FIELD_FADE_TYPE type, FIELD_FADE_WAIT_TYPE wait );

//------------------------------------------------------------------
/**
 * @brief	空を飛ぶブライトネスインイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FlySkyBrightIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                    FIELD_FADE_TYPE type, FIELD_FADE_WAIT_TYPE wait );

//------------------------------------------------------------------
/**
 * @brief	デバッグ用 即時アウト
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT* DEBUG_EVENT_QuickFadeOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap); 

//------------------------------------------------------------------
/**
 * @brief	デバッグ用 即時イン
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
extern GMEVENT* DEBUG_EVENT_QuickFadeIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap); 


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
