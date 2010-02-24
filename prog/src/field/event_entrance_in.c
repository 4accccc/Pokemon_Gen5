/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file   event_entrance_in.c
 * @breif  出入口への進入時のイベント作成
 * @author obata
 * @date   2009.08.18
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////

#include "gamesystem/game_event.h"
#include "field/eventdata_type.h"
#include "field/fieldmap_proc.h"
#include "field/location.h"

#include "event_entrance_in.h"

#include "entrance_camera_settings.h"  // for ENTRANCE_CAMERA_SETTINGS
#include "event_fieldmap_control.h"    // for EVENT_FieldFadeOut_xxxx
#include "event_entrance_effect.h"     // for EVENT_FieldDoorInAnime
#include "event_fldmmdl_control.h"     // for EVENT_PlayerOneStepAnime
#include "event_disappear.h"           // for EVENT_DISAPPEAR_xxxx

#include "sound/pm_sndsys.h"
#include "field_sound.h"
#include "sound/bgm_info.h"
#include "../../resource/sound/bgm_info/iss_type.h"

#include "fieldmap.h"
#include "field_task.h"  
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_target_offset.h"


//=======================================================================================
// ■イベント ワーク
//=======================================================================================
typedef struct
{
  GAMESYS_WORK*            gameSystem;
  GAMEDATA*                gameData;
  FIELDMAP_WORK*           fieldmap;
  LOCATION                 nextLocation;       // 遷移先指定
  EXIT_TYPE                exitType;           // 出入り口タイプ
  BOOL                     seasonDisplayFlag;  // 季節表示を行うかどうか
  ENTRANCE_CAMERA_SETTINGS cameraSettings;     // 特殊出入り口のカメラ設定データ
  FIELD_FADE_TYPE          fadeOutType;        // 季節表示がない場合のF/Oタイプ
  BOOL                     BGMFadeWaitFlag;    // BGM のフェード完了を待つかどうか

} EVENT_WORK;


//=======================================================================================
// ■非公開関数のプロトタイプ宣言
//======================================================================================= 

// 各EXIT_TYPEごとのイベント
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeNone( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeDoor( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeStep( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeWarp( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSPx( GMEVENT* event, int* seq, void* wk );


//=======================================================================================
// ■公開関数の定義
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @brief 出入口進入イベントを作成する
 *
 * @param parent            親イベント
 * @param gameSystem
 * @param gameData
 * @param fieldmap
 * @param nextLocation      遷移先指定
 * @param exitType          遷移タイプ指定
 * @param seasonDisplayFlag 季節表示を行うかどうか
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_EntranceIn( GMEVENT* parent, 
                           GAMESYS_WORK* gameSystem,
                           GAMEDATA* gameData, 
                           FIELDMAP_WORK* fieldmap, 
                           LOCATION nextLocation, 
                           EXIT_TYPE exitType,
                           BOOL seasonDisplayFlag )
{
  GMEVENT* event;
  EVENT_WORK* work;
  u16 prevZoneID, nextZoneID;
  FIELD_FADE_TYPE fadeOutType;

  // イベントテーブル
  const GMEVENT_FUNC eventFuncTable[] = 
  {
    EVENT_FUNC_EntranceIn_ExitTypeNone,   // EXIT_TYPE_NONE
    EVENT_FUNC_EntranceIn_ExitTypeNone,   // EXIT_TYPE_MAT
    EVENT_FUNC_EntranceIn_ExitTypeStep,   // EXIT_TYPE_STAIRS
    EVENT_FUNC_EntranceIn_ExitTypeDoor,   // EXIT_TYPE_DOOR
    EVENT_FUNC_EntranceIn_ExitTypeStep,   // EXIT_TYPE_WALL
    EVENT_FUNC_EntranceIn_ExitTypeWarp,   // EXIT_TYPE_WARP
    EVENT_FUNC_EntranceIn_ExitTypeNone,   // EXIT_TYPE_INTRUDE
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP1
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP2
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP3
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP4
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP5
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP6
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP7
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP8
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP9
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP10
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP11
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP12
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP13
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP14
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP15
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP16
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP17
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP18
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP19
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP20
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP21
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP22
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP23
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP24
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP25
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP26
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP27
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP28
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP29
    EVENT_FUNC_EntranceIn_ExitTypeSPx,    // EXIT_TYPE_SP30
  };

  // 基本フェードアウトの種類を決定
  prevZoneID  = FIELDMAP_GetZoneID( fieldmap );
  nextZoneID  = nextLocation.zone_id;
  fadeOutType = FIELD_FADE_GetFadeOutType( prevZoneID, nextZoneID );

  // イベント作成
  event = GMEVENT_Create( gameSystem, parent, eventFuncTable[ exitType ], sizeof( EVENT_WORK ) );

  // イベントワーク初期化
  work                    = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->gameSystem        = gameSystem;
  work->gameData          = gameData;
  work->fieldmap          = fieldmap;
  work->nextLocation      = nextLocation;
  work->exitType          = exitType;
  work->seasonDisplayFlag = seasonDisplayFlag;
  work->fadeOutType       = fadeOutType;

  return event;
}


//=======================================================================================
// ■非公開関数の定義
//=======================================================================================

//---------------------------------------------------------------------------------------
/**
 * @breif ドアなし時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeNone( GMEVENT * event, int *seq, void * wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
	GAMEDATA*      gameData   = work->gameData;
  FIELD_SOUND*   fieldSound = GAMEDATA_GetFieldSound( work->gameData );

  switch ( *seq )
  {
  // BGM再生準備
  case 0:
    { 
      u16 nowZoneID;

      nowZoneID = FIELDMAP_GetZoneID( fieldmap );
      FSND_StandByNextMapBGM( fieldSound, gameData, nowZoneID, work->nextLocation.zone_id );
    }
    (*seq)++;
    break;

  // SE 再生
  case 1:
    if( work->fadeOutType != FIELD_FADE_CROSS ) { PMSND_PlaySE( SEQ_SE_KAIDAN ); }
    (*seq)++; 
    break;

  // 画面フェードアウト
  case 2:
    if( work->seasonDisplayFlag )
    { // 季節フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeOut_Season( gameSystem, fieldmap, FIELD_FADE_WAIT ) );
    }
    else
    { // 基本フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeOut( gameSystem, fieldmap, work->fadeOutType, FIELD_FADE_WAIT ) );
    }
    (*seq)++;
    break;

  // イベント終了
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ドアあり時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeDoor( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // ドア進入イベント
    GMEVENT_CallEvent( event, 
        EVENT_FieldDoorInAnime( 
          gameSystem, fieldmap, &(work->nextLocation), TRUE, work->seasonDisplayFlag, work->fadeOutType ) );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif 階段の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeStep( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*     work       = wk;
	GAMESYS_WORK*   gameSystem = work->gameSystem;
	FIELDMAP_WORK*  fieldmap   = work->fieldmap;
	GAMEDATA*       gameData   = work->gameData;
  FIELD_SOUND*    fieldSound = GAMEDATA_GetFieldSound( work->gameData );

  switch ( *seq )
  {
  case 0:  
    // 自機前進
    GMEVENT_CallEvent( event, EVENT_PlayerOneStepAnime(gameSystem, fieldmap) );
    (*seq)++;
    break;
  case 1: 
    { // 現在のBGMがダンジョンISS && 次のBGMもダンジョンISS ==> BGMフェードアウト
      FIELD_SOUND*  fieldSound = GAMEDATA_GetFieldSound( gameData );
      BGM_INFO_SYS* BGMInfo    = GAMEDATA_GetBGMInfoSys( gameData );
      PLAYER_WORK*  player     = GAMEDATA_GetPlayerWork( gameData, 0 );
      u32 nextBGM = FSND_GetFieldBGM( gameData, work->nextLocation.zone_id );
      u32 nowBGM = PMSND_GetBGMsoundNo();
      u8 nextIssType = BGM_INFO_GetIssType( BGMInfo, nextBGM ); 
      u8 nowIssType = BGM_INFO_GetIssType( BGMInfo, nowBGM ); 
      if( ( nextIssType == ISS_TYPE_DUNGEON ) && ( nowIssType == ISS_TYPE_DUNGEON ) ) { 
        // BGM フェードアウト
        GMEVENT* fadeOutEvent = EVENT_FSND_FadeOutBGM( gameSystem, FSND_FADE_SHORT );
        GMEVENT_CallEvent( event, fadeOutEvent );
        work->BGMFadeWaitFlag = TRUE; // BGMフェードを待つ
      }
      else { 
        // BGM 再生準備
        u16 nowZoneID = FIELDMAP_GetZoneID( fieldmap );
        FSND_StandByNextMapBGM( fieldSound, gameData, nowZoneID, work->nextLocation.zone_id );
        work->BGMFadeWaitFlag = FALSE; // BGMフェードは待たない
      }
    }
    (*seq)++;
    break;
  case 2: 
    // BGMフェード完了待ち
    if( (work->BGMFadeWaitFlag == FALSE) || (PMSND_CheckFadeOnBGM() == FALSE) )
    { 
      // SE 再生
      if( work->fadeOutType != FIELD_FADE_CROSS ) { PMSND_PlaySE( SEQ_SE_KAIDAN ); }
      (*seq)++; 
    }
    break;
  case 3:
    // 画面フェードアウト
    if( work->seasonDisplayFlag )
    { // 季節フェード
      GMEVENT_CallEvent( event, 
          EVENT_FieldFadeOut_Season( gameSystem, fieldmap, FIELD_FADE_WAIT ) );
    }
    else
    { // 基本フェード
      GMEVENT_CallEvent( event,
          EVENT_FieldFadeOut( gameSystem, fieldmap, work->fadeOutType, FIELD_FADE_WAIT ) );
    }
    (*seq)++;
    break;
  case 4:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------------
/**
 * @breif ワープ時の進入イベント
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeWarp( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;

  switch( *seq )
  {
  case 0:
    // ワープ退出イベント
		GMEVENT_CallEvent( event, EVENT_DISAPPEAR_Warp( NULL, gameSystem, fieldmap ) );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 

//---------------------------------------------------------------------------------------
/**
 * @breif 進入イベント( SPx )
 *
 * ※カメラのアニメーション → ドア進入アニメ
 */
//---------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_EntranceIn_ExitTypeSPx( GMEVENT* event, int* seq, void* wk )
{
	EVENT_WORK*    work       = wk;
	GAMESYS_WORK*  gameSystem = work->gameSystem;
	FIELDMAP_WORK* fieldmap   = work->fieldmap;
  FIELD_CAMERA*  camera     = FIELDMAP_GetFieldCamera( work->fieldmap );

  // 処理シーケンス
  enum {
    SEQ_LOAD_ENTRANCE_CAMERA_SETTINGS,  // カメラ演出データ取得
    SEQ_CREATE_CAMERA_EFFECT_TASK,      // カメラ演出タスクの作成
    SEQ_WAIT_CAMERA_EFFECT_TASK,        // カメラ演出タスク終了待ち
    SEQ_CAMERA_STOP_TRACE_REQUEST,      // カメラの自機追従OFFリクエスト発行
    SEQ_WAIT_CAMERA_TRACE,              // カメラの自機追従処理の終了待ち
    SEQ_CAMERA_TRACE_OFF,               // カメラの自機追従OFF
    SEQ_DOOR_IN_ANIME,                  // ドア進入イベント
    SEQ_EXIT,                           // イベント終了
  };

  switch( *seq )
  {
  // カメラ演出データ取得
  case SEQ_LOAD_ENTRANCE_CAMERA_SETTINGS:
    // データ取得
    ENTRANCE_CAMERA_SETTINGS_LoadData( &work->cameraSettings, work->exitType );

    // データが有効かどうか
    if( work->cameraSettings.validFlag_IN )
    {
      *seq = SEQ_CREATE_CAMERA_EFFECT_TASK;
    }
    else
    {
      *seq = SEQ_DOOR_IN_ANIME;
    } 
    break;

  // カメラ演出タスクの作成
  case SEQ_CREATE_CAMERA_EFFECT_TASK:
    {
      u16 frame;
      u16 pitch, yaw;
      fx32 length;
      VecFx32 targetOffset;
      // 各パラメータ取得
      {
        frame  = work->cameraSettings.frame;
        pitch  = work->cameraSettings.pitch;
        yaw    = work->cameraSettings.yaw;
        length = work->cameraSettings.length << FX32_SHIFT;
        VEC_Set( &targetOffset, 
                 work->cameraSettings.targetOffsetX << FX32_SHIFT,
                 work->cameraSettings.targetOffsetY << FX32_SHIFT,
                 work->cameraSettings.targetOffsetZ << FX32_SHIFT );
      }
      // タスク登録
      {
        FIELD_TASK_MAN* taskMan;
        FIELD_TASK* zoomTaks;
        FIELD_TASK* pitchTask;
        FIELD_TASK* yawTask;
        FIELD_TASK* targetOffsetTask;
        // 生成
        zoomTaks         = FIELD_TASK_CameraLinearZoom  ( fieldmap, frame, length );
        pitchTask        = FIELD_TASK_CameraRot_Pitch   ( fieldmap, frame, pitch );
        yawTask          = FIELD_TASK_CameraRot_Yaw     ( fieldmap, frame, yaw );
        targetOffsetTask = FIELD_TASK_CameraTargetOffset( fieldmap, frame, &targetOffset );
        // 登録
        taskMan = FIELDMAP_GetTaskManager( fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, zoomTaks, NULL );
        FIELD_TASK_MAN_AddTask( taskMan, pitchTask, NULL );
        FIELD_TASK_MAN_AddTask( taskMan, yawTask, NULL );
        FIELD_TASK_MAN_AddTask( taskMan, targetOffsetTask, NULL );
      }
    }
    *seq = SEQ_WAIT_CAMERA_EFFECT_TASK;
    break;

  // タスク終了待ち
  case SEQ_WAIT_CAMERA_EFFECT_TASK:
    {
      FIELD_TASK_MAN* taskMan;
      taskMan = FIELDMAP_GetTaskManager( fieldmap );
      if( FIELD_TASK_MAN_IsAllTaskEnd(taskMan) ){ *seq = SEQ_CAMERA_STOP_TRACE_REQUEST; }
    }
    break;

  // カメラのトレース処理停止リクエスト発行
  case SEQ_CAMERA_STOP_TRACE_REQUEST:
    FIELD_CAMERA_StopTraceRequest( camera );
    *seq = SEQ_WAIT_CAMERA_TRACE;
    break;

  // カメラのトレース処理終了待ち
  case SEQ_WAIT_CAMERA_TRACE: 
    if( FIELD_CAMERA_CheckTrace( camera ) == FALSE ){ *seq = SEQ_CAMERA_TRACE_OFF; }
    break;

  // カメラのトレースOFF
  case SEQ_CAMERA_TRACE_OFF:
    FIELD_CAMERA_FreeTarget( camera );
    *seq = SEQ_DOOR_IN_ANIME;
    break;

  // ドア進入アニメ
  case SEQ_DOOR_IN_ANIME:
    GMEVENT_CallEvent( event, 
        EVENT_FieldDoorInAnime( 
          gameSystem, fieldmap, &work->nextLocation, FALSE, work->seasonDisplayFlag, work->fadeOutType ) );
    *seq = SEQ_EXIT;
    break;

  // イベント終了
  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
