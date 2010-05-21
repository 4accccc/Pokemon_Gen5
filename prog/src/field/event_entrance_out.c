/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file   event_entrance_out.c
 * @breif  出入口からの退出時のイベント作成
 * @author obata
 * @date   2009.08.18
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/game_event.h"
#include "gamesystem/iss_sys.h"
#include "gamesystem/iss_dungeon_sys.h"
#include "field/eventdata_type.h"
#include "field/fieldmap_proc.h"
#include "field/location.h" 
#include "field/eventdata_sxy.h"
#include "field/zonedata.h"            // for ZONEDATA_GetBGMID
#include "sound/pm_sndsys.h"
#include "sound/bgm_info.h"

#include "field_sound.h" 
#include "event_appear.h"              // for EVENT_APPEAR_xxxx
#include "event_fieldmap_control.h"    // for EVENT_FieldFadeIn
#include "event_entrance_effect.h"     // for EVENT_FieldDoorOutAnime
#include "event_fldmmdl_control.h"     // for EVENT_PlayerOneStepAnime
#include "field_place_name.h"          // for FIELD_PLACE_NAME_DisplayOnStanderdRule
#include "fieldmap.h"                  // for FIELDMAP_GetPlaceNameSys

#include "event_entrance_out.h"



//=======================================================================================
// ■定数
//=======================================================================================
// イベント処理関数のタイプ
typedef enum {
  EVENTFUNC_TYPE_NONE,    // ドアなし
  EVENTFUNC_TYPE_DOOR,    // ドアあり
  EVENTFUNC_TYPE_STEP,    // 階段
  EVENTFUNC_TYPE_WARP,    // ワープ
  EVENTFUNC_TYPE_SPx,     // 特殊
  EVENTFUNC_TYPE_INTRUDE, // 侵入
  EVENTFUNC_TYPE_NUM,
} EVENTFUNC_TYPE;


//=======================================================================================
// ■イベントワーク
//=======================================================================================
typedef struct {

  ENTRANCE_EVDATA* evdata; // 出入り口イベントの共通ワーク

} EVENT_WORK;


//=======================================================================================
// ■非公開関数のプロトタイプ宣言
//======================================================================================= 
// 各EXIT_TYPEごとのイベント
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeNone( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeDoor( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeStep( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeWarp( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSPx( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeIntrude( GMEVENT* event, int* seq, void* wk );
// BGM
static void StartStandByBGM( EVENT_WORK* work );
static void CallBGMFadeInEvent( EVENT_WORK* work, GMEVENT* parentEvent );
// 画面のフェードイン
static void CallFadeInEvent( const EVENT_WORK* work, GMEVENT* parentEvent );
// 地名表示
static void PutPlaceNameRequest( EVENT_WORK* work );


//=======================================================================================
// ■公開関数の定義
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @brief 出入口退出イベントを作成する
 *
 * @param evdata 出入り口イベント共通ワーク
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_EntranceOut( ENTRANCE_EVDATA* evdata )
{
  GMEVENT* event;
  EVENT_WORK* work; 
  EVENTFUNC_TYPE funcType;

  // イベントテーブル
  const GMEVENT_FUNC eventFuncTable[ EVENTFUNC_TYPE_NUM ] = 
  {
    EVENT_FUNC_EntranceOut_ExitTypeNone,   // EVENTFUNC_TYPE_NONE ドアなし
    EVENT_FUNC_EntranceOut_ExitTypeDoor,   // EVENTFUNC_TYPE_DOOR ドアあり
    EVENT_FUNC_EntranceOut_ExitTypeStep,   // EVENTFUNC_TYPE_STEP 階段
    EVENT_FUNC_EntranceOut_ExitTypeWarp,   // EVENTFUNC_TYPE_WARP ワープ
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    // EVENTFUNC_TYPE_SPx, 特殊
    EVENT_FUNC_EntranceOut_ExitTypeIntrude, // EVENTFUNC_TYPE_INTRUDE 侵入
  };

  // イベント処理関数を決定
  switch( evdata->exit_type_out ) {
  case EXIT_TYPE_NONE:    funcType = EVENTFUNC_TYPE_NONE;    break;
  case EXIT_TYPE_MAT:     funcType = EVENTFUNC_TYPE_NONE;    break;
  case EXIT_TYPE_STAIRS:  funcType = EVENTFUNC_TYPE_STEP;    break;
  case EXIT_TYPE_DOOR:    funcType = EVENTFUNC_TYPE_DOOR;    break;
  case EXIT_TYPE_WALL:    funcType = EVENTFUNC_TYPE_STEP;    break;
  case EXIT_TYPE_WARP:    funcType = EVENTFUNC_TYPE_WARP;    break;
  case EXIT_TYPE_INTRUDE: funcType = EVENTFUNC_TYPE_INTRUDE; break;
  default:                funcType = EVENTFUNC_TYPE_SPx;     break;
  }

  // イベント作成
  event = GMEVENT_Create( evdata->gameSystem, evdata->parentEvent, 
      eventFuncTable[ funcType ], sizeof( EVENT_WORK ) );

  // イベント・ワークを初期化
  work = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->evdata = evdata;

  return event;
}


//=======================================================================================
// ■非公開関数の定義
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @breif ドアなし時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeNone( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*      work       = wk;
  ENTRANCE_EVDATA* evdata     = work->evdata;

  switch( *seq ) {
  case 0:
    // BGM再生開始
    if( evdata->BGM_standby_flag ) {
      StartStandByBGM( work );
    }
    // 画面フェード開始
    CallFadeInEvent( work, event );
    (*seq)++;
    break;

  case 1:
    PutPlaceNameRequest( work ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ドアあり時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeDoor( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*      work   = wk;
  ENTRANCE_EVDATA* evdata = work->evdata;

  switch( *seq ) {
  case 0:
    // ドア退出イベント
    GMEVENT_CallEvent( event, EVENT_FieldDoorOutAnime( evdata ) );
    (*seq)++;
    break;

  case 1:
    PutPlaceNameRequest( work ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH; // イベント終了
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 階段の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeStep( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*      work       = wk;
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;
	FIELDMAP_WORK*   fieldmap   = evdata->fieldmap;

  switch( *seq ) {
  case 0: 
    // 画面フェードイン開始
    CallFadeInEvent( work, event );
    (*seq)++;
    break;

  case 1:  
    // BGM 操作
    if( evdata->BGM_standby_flag ) {
      StartStandByBGM( work );
    }
    else if( evdata->BGM_fadeout_flag ) {
      CallBGMFadeInEvent( work, event );
    }
    (*seq)++;
    break;

  case 2:
    // 自機の一歩移動アニメ
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gameSystem, fieldmap) );
    (*seq)++;
    break;

  case 3:
    PutPlaceNameRequest( work ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH; // イベント終了
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ワープ時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeWarp( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*      work       = wk;
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;
	FIELDMAP_WORK*   fieldmap   = evdata->fieldmap;

  switch( *seq ) {
  case 0:
    // ワープ出現イベント
		GMEVENT_CallEvent(event, EVENT_APPEAR_Warp( NULL, gameSystem, fieldmap ) );
    (*seq)++;
    break;

  case 1:
    PutPlaceNameRequest( work ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH; // イベント終了
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 退出イベント( SPx )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSPx( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*      work     = wk;
  ENTRANCE_EVDATA* evdata   = work->evdata;
	FIELDMAP_WORK*   fieldmap = evdata->fieldmap;
  FIELD_CAMERA*    camera   = FIELDMAP_GetFieldCamera( fieldmap );

  switch( *seq ) {
  case 0:
    // ドア退出イベント
    GMEVENT_CallEvent( event, EVENT_FieldDoorOutAnime( evdata ) );
    (*seq)++;
    break;

  case 1:
    PutPlaceNameRequest( work ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 

//---------------------------------------------------------------------------------------
/**
 * @breif 侵入時の退出イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeIntrude( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*      work       = wk;
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;
	FIELDMAP_WORK*   fieldmap   = evdata->fieldmap;

  switch( *seq ) {
  case 0:
    // BGM再生開始
    if( evdata->BGM_standby_flag ) {
      StartStandByBGM( work );
    }
    (*seq)++;
    break;

  case 1:
    // 画面フェード開始
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Cross( gameSystem, fieldmap ) );
    (*seq)++;
    break;

  case 2:
    PutPlaceNameRequest( work ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH; // イベント終了
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @brief スタンバイした BGM の再生を開始する
 *
 * @param work
 */
//---------------------------------------------------------------------------------------
static void StartStandByBGM( EVENT_WORK* work )
{
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMEDATA*        gameData   = evdata->gameData;
  FIELD_SOUND*     fieldSound = GAMEDATA_GetFieldSound( gameData );

  GF_ASSERT( evdata->BGM_standby_flag );

  FSND_PlayStartBGM( fieldSound, gameData );
}

//---------------------------------------------------------------------------------------
/**
 * @brief フェードアウトした BGM のフェードインイベントを呼び出す
 *
 * @param work
 * @param parentEvent
 */
//---------------------------------------------------------------------------------------
static void CallBGMFadeInEvent( EVENT_WORK* work, GMEVENT* parentEvent )
{
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;
	GAMEDATA*        gameData   = evdata->gameData;
  FIELD_SOUND*     fieldSound = GAMEDATA_GetFieldSound( gameData );

  GF_ASSERT( evdata->BGM_fadeout_flag );

  GMEVENT_CallEvent( parentEvent, 
      EVENT_FSND_FadeInBGM( gameSystem, FSND_FADE_FAST ) );
} 

//---------------------------------------------------------------------------------------
/**
 * @brief 画面のフェードインイベントを呼び出す
 *
 * @param work
 * @param parentEvent
 */
//---------------------------------------------------------------------------------------
static void CallFadeInEvent( const EVENT_WORK* work, GMEVENT* parentEvent )
{
  ENTRANCE_EVDATA* evdata     = work->evdata;
	GAMESYS_WORK*    gameSystem = evdata->gameSystem;
	FIELDMAP_WORK*   fieldmap   = evdata->fieldmap;

  // 季節フェード
  if( evdata->season_disp_flag ) { 
    GMEVENT_CallEvent( parentEvent, 
        EVENT_FieldFadeIn_Season( gameSystem, fieldmap, evdata->start_season, evdata->end_season ) );
  }
  // 基本フェード
  else {
    GMEVENT_CallEvent( parentEvent, 
        EVENT_FieldFadeIn( gameSystem, fieldmap, evdata->fadein_type, FIELD_FADE_WAIT, TRUE, 0, 0 ) );
  }
} 

//---------------------------------------------------------------------------------------
/**
 * @brief 地名表示のリクエストを発行する
 *
 * @param work
 */
//---------------------------------------------------------------------------------------
static void PutPlaceNameRequest( EVENT_WORK* work )
{
  ENTRANCE_EVDATA* evdata   = work->evdata;

  FIELD_PLACE_NAME_DisplayOnStanderdRule( 
      FIELDMAP_GetPlaceNameSys(evdata->fieldmap), evdata->nextLocation.zone_id );
}
