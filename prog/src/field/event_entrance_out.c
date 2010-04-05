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
#include "gamesystem/game_event.h"
#include "field/eventdata_type.h"
#include "field/fieldmap_proc.h"
#include "field/location.h"

#include "event_entrance_out.h"

#include "field/eventdata_sxy.h"
#include "field/zonedata.h"            // for ZONEDATA_GetBGMID
#include "event_appear.h"              // for EVENT_APPEAR_xxxx
#include "event_fieldmap_control.h"    // for EVENT_FieldFadeIn
#include "event_entrance_effect.h"     // for EVENT_FieldDoorOutAnime
#include "event_fldmmdl_control.h"     // for EVENT_PlayerOneStepAnime
#include "field_place_name.h"          // for FIELD_PLACE_NAME_Display
#include "fieldmap.h"                  // for FIELDMAP_GetPlaceNameSys

#include "sound/pm_sndsys.h"
#include "sound/bgm_info.h"
#include "field_sound.h" 
#include "../../resource/sound/bgm_info/iss_type.h"

#include "field_task.h"  
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_target_offset.h"


//=======================================================================================
// ■定数
//=======================================================================================
// イベント処理関数のタイプ
typedef enum {
  EVENTFUNC_TYPE_NONE, // ドアなし
  EVENTFUNC_TYPE_DOOR, // ドアあり
  EVENTFUNC_TYPE_STEP, // 階段
  EVENTFUNC_TYPE_WARP, // ワープ
  EVENTFUNC_TYPE_SPx,  // 特殊
  EVENTFUNC_TYPE_NUM,
} EVENTFUNC_TYPE;


//=======================================================================================
// ■イベントワーク
//=======================================================================================
typedef struct {

  GAMESYS_WORK*     gameSystem;
  GAMEDATA*         gameData;
  FIELDMAP_WORK*    fieldmap;
  LOCATION          location;          // 遷移先指定
  u16               prevZoneID;        // 遷移前のゾーン
  EXIT_TYPE         exitType;          // 出入り口タイプ
  BOOL              seasonDisplayFlag; // 季節表示を行うかどうか
  u8                startSeason;       // 最初に表示する季節
  u8                endSeason;         // 最後に表示する季節
  FIELD_FADE_TYPE   fadeInType;        // 季節表示がない場合のF/Iタイプ

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


//=======================================================================================
// ■公開関数の定義
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @brief 出入口退出イベントを作成する
 *
 * @param parent            親イベント
 * @param gameSystem
 * @param gameData
 * @param fieldmap
 * @param location          遷移先指定
 * @param prevZoneID        遷移前のゾーン
 * @param seasonDisplayFlag 季節表示を行うかどうか
 * @param startSeason       最初に表示する季節
 * @param endtSeason        最後に表示する季節
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_EntranceOut( GMEVENT* parent, 
                            GAMESYS_WORK* gameSystem,
                            GAMEDATA* gameData, 
                            FIELDMAP_WORK* fieldmap, 
                            LOCATION location,
                            u16 prevZoneID,
                            BOOL seasonDisplayFlag,
                            u8 startSeason,
                            u8 endSeason )
{
  GMEVENT* event;
  EVENT_WORK* work; 
  EXIT_TYPE exitType; 
  EVENTFUNC_TYPE funcType;

  // イベントテーブル
  const GMEVENT_FUNC eventFuncTable[ EVENTFUNC_TYPE_NUM ] = 
  {
    EVENT_FUNC_EntranceOut_ExitTypeNone,   // EVENTFUNC_TYPE_NONE ドアなし
    EVENT_FUNC_EntranceOut_ExitTypeDoor,   // EVENTFUNC_TYPE_DOOR ドアあり
    EVENT_FUNC_EntranceOut_ExitTypeStep,   // EVENTFUNC_TYPE_STEP 階段
    EVENT_FUNC_EntranceOut_ExitTypeWarp,   // EVENTFUNC_TYPE_WARP ワープ
    EVENT_FUNC_EntranceOut_ExitTypeSPx,    // EVENTFUNC_TYPE_SPx, 特殊
  };

  // EXIT_TYPEを決定
  if( location.type == LOCATION_TYPE_DIRECT ) {
    exitType = EXIT_TYPE_NONE;
  }
  else {
    EVENTDATA_SYSTEM* eventData;
    const CONNECT_DATA* connectData;

    eventData   = GAMEDATA_GetEventData( gameData );
    connectData = EVENTDATA_GetConnectByID( eventData, location.exit_id );
    exitType    = CONNECTDATA_GetExitType( connectData );
  } 

  // イベント処理関数を決定
  switch( exitType ) {
  case EXIT_TYPE_NONE:    funcType = EVENTFUNC_TYPE_NONE; break;
  case EXIT_TYPE_MAT:     funcType = EVENTFUNC_TYPE_NONE; break;
  case EXIT_TYPE_STAIRS:  funcType = EVENTFUNC_TYPE_STEP; break;
  case EXIT_TYPE_DOOR:    funcType = EVENTFUNC_TYPE_DOOR; break;
  case EXIT_TYPE_WALL:    funcType = EVENTFUNC_TYPE_STEP; break;
  case EXIT_TYPE_WARP:    funcType = EVENTFUNC_TYPE_WARP; break;
  case EXIT_TYPE_INTRUDE: funcType = EVENTFUNC_TYPE_NONE; break;
  default:                funcType = EVENTFUNC_TYPE_SPx;  break;
  }

  // イベント作成
  event = GMEVENT_Create( gameSystem, parent, eventFuncTable[ funcType ], sizeof( EVENT_WORK ) );

  // イベント・ワークを初期化
  work                    = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gameSystem        = gameSystem;
  work->gameData          = gameData;
  work->fieldmap          = fieldmap;
  work->prevZoneID        = prevZoneID;
  work->location          = location;
  work->exitType          = exitType;
  work->seasonDisplayFlag = seasonDisplayFlag;
  work->startSeason       = startSeason;
  work->endSeason         = endSeason;
  work->fadeInType        = FIELD_FADE_GetFadeInType( prevZoneID, location.zone_id );

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
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
	GAMEDATA*      gameData   = work->gameData;
  FIELD_SOUND*   fieldSound = GAMEDATA_GetFieldSound( work->gameData );

  switch( *seq ) {
  // カメラのセットアップ
  case 0:
    (*seq)++;
    break;

  // BGM再生開始
  case 1:
    FSND_PlayStartBGM( fieldSound );
    (*seq)++;
    break;

  // 画面フェード開始
  case 2:
    if( work->seasonDisplayFlag ) { 
      // 季節フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn_Season( gameSystem, fieldmap, work->startSeason, work->endSeason ) );
    }
    else {
      // 基本フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn( gameSystem, fieldmap, work->fadeInType, FIELD_FADE_WAIT, TRUE, 0, 0 ) );
    }
    (*seq)++;
    break;

  // イベント終了
  case 3:
    FIELD_PLACE_NAME_Display( 
        FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id ); // 地名表示更新リクエスト
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
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;

  switch( *seq ) {
  case 0:
    // ドア退出イベント
    GMEVENT_CallEvent( event, 
        EVENT_FieldDoorOutAnime( gameSystem, fieldmap, TRUE, work->seasonDisplayFlag, 
                                 work->startSeason, work->endSeason, work->fadeInType,
                                 work->exitType ) );
    (*seq)++;
    break;

  // イベント終了
  case 1:
    FIELD_PLACE_NAME_Display( 
        FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH;
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
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
	GAMEDATA*      gameData   = work->gameData;
  FIELD_SOUND*   fieldSound = GAMEDATA_GetFieldSound( work->gameData );

  switch( *seq ) {
  // 画面フェードイン開始
  case 0: 
    if( work->seasonDisplayFlag ) { 
      // 季節フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn_Season( gameSystem, fieldmap, work->startSeason, work->endSeason ) );
    }
    else { 
      // クロスフェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeIn( gameSystem, fieldmap, work->fadeInType, FIELD_FADE_WAIT, TRUE, 0, 0 ) );
    }
    (*seq)++;
    break;

  // BGM 操作
  case 1:  
    { 
      BGM_INFO_SYS* bgm_info = GAMEDATA_GetBGMInfoSys( gameData );
      u8 season = GAMEDATA_GetSeasonID( gameData );
      u16 zone_id = FIELDMAP_GetZoneID( fieldmap );
      u32 bgm_now = ZONEDATA_GetBGMID( zone_id, season );
      u8 iss_type_now = BGM_INFO_GetIssType( bgm_info, bgm_now ); 
      // 現在のBGMがダンジョンISS
      if( iss_type_now == ISS_TYPE_DUNGEON ) {
        // フェードイン
        FIELD_SOUND* fieldSound = GAMEDATA_GetFieldSound( gameData );
        GMEVENT* fadeInEvent = EVENT_FSND_FadeInBGM( gameSystem, FSND_FADE_FAST );
        GMEVENT_CallEvent( event, fadeInEvent );
      }
      else {
        FSND_PlayStartBGM( fieldSound );
      }
    }
    (*seq)++;
    break;

  // 自機の一歩移動アニメ
  case 2:
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gameSystem, fieldmap) );
    (*seq)++;
    break;

  // イベント終了
  case 3:
    FIELD_PLACE_NAME_Display(
        FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH;
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
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
	GAMEDATA*      gameData   = work->gameData;

  switch( *seq ) {
  // ワープ出現イベント
  case 0:
		GMEVENT_CallEvent(event, EVENT_APPEAR_Warp( NULL, gameSystem, fieldmap ) );
    (*seq)++;
    break;

  // イベント終了
  case 1:
    FIELD_PLACE_NAME_Display( 
        FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id ); // 地名表示更新リクエスト
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

#if 0
//---------------------------------------------------------------------------------------
/**
 * @breif 退出イベント( SPx )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSPx( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
  FIELD_CAMERA*  camera     = FIELDMAP_GetFieldCamera( work->fieldmap );

  // 処理シーケンス
  enum {
    SEQ_SETUP_CAMERA,                   // カメラのセットアップ
    SEQ_LOAD_ENTRANCE_CAMERA_SETTINGS,  // カメラ演出データ取得
    SEQ_DOOR_OUT_ANIME,                 // ドア退出イベント
    SEQ_INIT_CAMERA_SETTINGS,           // カメラの初期状態を設定する
    SEQ_CREATE_CAMERA_EFFECT_TASK,      // カメラ演出タスクの作成
    SEQ_WAIT_CAMERA_EFFECT_TASK,        // カメラ演出タスク終了待ち
    SEQ_DISPLAY_PLACE_NAME,             // 地名表示更新リクエスト発行
    SEQ_RECOVER_CAMERA,                 // カメラの復帰
    SEQ_EXIT,                           // イベント終了
  };

  switch( *seq ) {
  // カメラのセットアップ
  case SEQ_SETUP_CAMERA:
    SetupCamera( work );
    *seq = SEQ_LOAD_ENTRANCE_CAMERA_SETTINGS;
    break;

  // カメラ演出データ取得
  case SEQ_LOAD_ENTRANCE_CAMERA_SETTINGS:
    // データ取得
    ENTRANCE_CAMERA_LoadData( &work->cameraSettings, work->exitType );

    // データが有効かどうか
    if( work->cameraSettings.validFlag_OUT ) {
      *seq = SEQ_INIT_CAMERA_SETTINGS;
    }
    else {
      *seq = SEQ_DOOR_OUT_ANIME;
    } 
    break;

  // カメラの初期状態を設定する
  case SEQ_INIT_CAMERA_SETTINGS:
    ENTRANCE_CAMERA_PrepareForDoorOut( fieldmap, &(work->cameraSettings) );
    *seq = SEQ_DOOR_OUT_ANIME;
    break;

  // ドア退出イベント
  case SEQ_DOOR_OUT_ANIME:
    GMEVENT_CallEvent( event, 
        EVENT_FieldDoorOutAnime( gameSystem, fieldmap, FALSE, 
                                 work->seasonDisplayFlag, work->startSeason, work->endSeason, work->fadeInType ) );

    if( work->cameraSettings.validFlag_OUT ) {
      *seq = SEQ_CREATE_CAMERA_EFFECT_TASK;
    }
    else {
      *seq = SEQ_DISPLAY_PLACE_NAME;
    } 
    break;

  // カメラ演出タスクの作成
  case SEQ_CREATE_CAMERA_EFFECT_TASK:
    ENTRANCE_CAMERA_AddDoorOutTask( fieldmap, &(work->cameraSettings) );
    *seq = SEQ_WAIT_CAMERA_EFFECT_TASK;
    break;

  // タスク終了待ち
  case SEQ_WAIT_CAMERA_EFFECT_TASK:
    {
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ) { 
        *seq = SEQ_DISPLAY_PLACE_NAME;
      }
    }
    break;

  // 地名表示更新リクエスト発行
  case SEQ_DISPLAY_PLACE_NAME:
    FIELD_PLACE_NAME_Display( FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id );
    *seq = SEQ_RECOVER_CAMERA;
    break;

  // カメラの復帰
  case SEQ_RECOVER_CAMERA:
    RecoverCamera( work );
    *seq = SEQ_EXIT;
    break;

  // イベント終了
  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
#endif
//---------------------------------------------------------------------------------------
/**
 * @breif 退出イベント( SPx )
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceOut_ExitTypeSPx( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
  FIELD_CAMERA*  camera     = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( *seq ) {
  // ドア退出イベント
  case 0:
    GMEVENT_CallEvent( event, 
        EVENT_FieldDoorOutAnime( gameSystem, fieldmap, FALSE, 
                                 work->seasonDisplayFlag, work->startSeason, work->endSeason, work->fadeInType, work->exitType ) );
    (*seq)++;
    break;

  // 地名表示更新リクエスト発行
  case 1:
    FIELD_PLACE_NAME_Display( FIELDMAP_GetPlaceNameSys(fieldmap), work->location.zone_id );
    (*seq)++;
    break;

  // イベント終了
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
