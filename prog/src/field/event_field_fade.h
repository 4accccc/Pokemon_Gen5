//============================================================================================
/**
 * @file	event_field_fade.h
 * @brief	イベント：フィールドフェード制御ツール
 * @author	tamada GAMEFREAK inc.
 * @date	2008.12.10
 *
 * 2009.12.22 tamada event_fieldmap_controlから分離
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"


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


