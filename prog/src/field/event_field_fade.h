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

// フェードの種類指定
typedef enum {
	FIELD_FADE_BLACK,  // 輝度フェード(ブラック)
	FIELD_FADE_WHITE,  // 輝度フェード(ホワイト)
  FIELD_FADE_CROSS,  // クロスフェード
  FIELD_FADE_SEASON, // 季節フェード
} FIELD_FADE_TYPE;

// フェード処理完了待ちフラグ
typedef enum{
  FIELD_FADE_WAIT,    // フェード完了を待つ
  FIELD_FADE_NO_WAIT, // フェード完了を待たない
} FIELD_FADE_WAIT_FLAG;

// BG初期化をイベント内で行うかどうか
typedef enum{
  FIELD_FADE_BG_INIT,       // 初期化する
  FIELD_FADE_NOT_BG_INIT,   // 初期化しない
} FIELD_FADE_BG_INIT_FLAG;


//============================================================================================
// ■基本フェード関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief	フェードアウトイベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 * @param	fadeType		       フェードの種類指定
 * @param fadeFinishWaitFlag フェード完了を待つかどうか(輝度フェード時のみ有効)
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
extern GMEVENT* EVENT_FieldFadeOut( GAMESYS_WORK *gameSystem, FIELDMAP_WORK * fieldmap, 
                                    FIELD_FADE_TYPE fadeType, 
                                    FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag );

// クロスフェード
#define EVENT_FieldFadeOut_Cross( gsys, fieldmap ) \
        EVENT_FieldFadeOut( gsys, fieldmap, FIELD_FADE_CROSS, 0 )
// 輝度フェード(ブラック)
#define EVENT_FieldFadeOut_Black( gsys, fieldmap, wait ) \
        EVENT_FieldFadeOut( gsys, fieldmap, FIELD_FADE_BLACK, wait )
// 輝度フェード(ホワイト)
#define EVENT_FieldFadeOut_White( gsys, fieldmap, wait ) \
        EVENT_FieldFadeOut( gsys, fieldmap, FIELD_FADE_WHITE, wait )
// 季節フェード
#define EVENT_FieldFadeOut_Season( gsys, fieldmap, wait ) \
        EVENT_FieldFadeOut( gsys, fieldmap, FIELD_FADE_BLACK, wait )


//--------------------------------------------------------------------------------------------
/**
 * @brief	フェードイン イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap 
 * @param	fadeType		       フェードの種類指定
 * @param fadeFinishWaitFlag フェード完了を待つかどうか ( 輝度フェード時のみ有効 )
 * @param BGInitFlag         BG初期化を内部でおこなうかどうか ( 輝度フェード時のみ有効 )
 * @param startSeason        最初に表示する季節 ( 季節フェード時のみ有効 )
 * @param endSeason          最後に表示する季節 ( 季節フェード時のみ有効 )
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
extern GMEVENT* EVENT_FieldFadeIn( GAMESYS_WORK *gameSystem, FIELDMAP_WORK * fieldmap, 
                                   FIELD_FADE_TYPE fadeType, 
                                   FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag, 
                                   FIELD_FADE_BG_INIT_FLAG BGInitFlag,
                                   u8 startSeason, u8 endSeason );

// クロスフェード
#define EVENT_FieldFadeIn_Cross( gsys, fieldmap ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_CROSS, 0, 0, 0, 0 )
// 輝度フェード(ブラック)
#define EVENT_FieldFadeIn_Black( gsys, fieldmap, wait ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_BLACK, wait, TRUE, 0, 0 )
// 輝度フェード(ホワイト)
#define EVENT_FieldFadeIn_White( gsys, fieldmap, wait ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_WHITE, wait, TRUE, 0, 0 )
// 季節フェード
#define EVENT_FieldFadeIn_Season( gsys, fieldmap, startSeason, endSeason ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_SEASON, 0, 0, startSeason, endSeason )
// メニュー状態フェード（Yボタンメニューを出したままなど）
#define EVENT_FieldFadeIn_Menu( gsys, fieldmap, wait ) \
        EVENT_FieldFadeIn( gsys, fieldmap, FIELD_FADE_BLACK, wait, FALSE, 0, 0 )


//============================================================================================
// ■例外的なフェード関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief	空を飛ぶ 輝度フェードアウト イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 * @param	fadeType	         フェードの種類指定 ( FIELD_FADE_BLACK or FIELD_FADE_WHITE )
 * @param fadeFinishWaitFlag フェード完了を待つかどうか
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
extern GMEVENT* EVENT_FlySkyBrightOut( GAMESYS_WORK *gameSystem, FIELDMAP_WORK * fieldmap, 
                                       FIELD_FADE_TYPE fadeType, FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag );

//--------------------------------------------------------------------------------------------
/**
 * @brief	空を飛ぶ 輝度フェードイン イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 * @param	fadeType		       フェードの種類指定 ( FADE_TYPE_BLACK or FADE_TYPE_WHITE )
 * @param fadeFinishWaitFlag フェード完了を待つかどうか
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
extern GMEVENT* EVENT_FlySkyBrightIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, 
                                      FIELD_FADE_TYPE fadeType, FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag );

//--------------------------------------------------------------------------------------------
/**
 * @brief	デバッグ用 即時アウト イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
extern GMEVENT* DEBUG_EVENT_QuickFadeOut( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap );

//--------------------------------------------------------------------------------------------
/**
 * @brief	デバッグ用 即時アウト
 *
 * @param	gameSystem
 * @param	fieldmap
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
extern GMEVENT* DEBUG_EVENT_QuickFadeIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap );
