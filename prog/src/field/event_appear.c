////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  退場＆登場イベント
 * @file   event_appear.c
 * @author obata
 * @date   2009.08.28
 *
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include "include/field/fieldmap_proc.h"
#include "include/gamesystem/game_event.h"
#include "event_appear.h"
#include "fieldmap.h"
#include "field_player.h"
#include "sound/pm_sndsys.h"

#include "field_task.h"
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_player_rot.h"
#include "field_task_player_fall.h"
#include "field_task_wait.h"

#include "fldeff_kemuri.h"  // for FLDEFF_KEMURI_SetMMdl
#include "event_fieldmap_control.h"  // for EVENT_FieldFadeIn
#include "event_season_display.h"  // for GetSeasonDispEventFrame


//==========================================================================================
// ■定数
//========================================================================================== 
#define ZOOM_IN_DIST   (180 << FX32_SHIFT)   // カメラのズームイン距離
#define ZOOM_IN_FRAME  (60)   // ズームインに掛かるフレーム数
#define ZOOM_OUT_FRAME (60)   // ズームアウトに掛かるフレーム数


//==========================================================================================
// ■イベントワーク
//==========================================================================================
typedef struct
{ 
  FIELD_CAMERA_MODE cameraMode;   // イベント開始時のカメラモード
  FIELDMAP_WORK*      fieldmap;   // 動作フィールドマップ
  GAMESYS_WORK*           gsys;   // ゲームシステム

} EVENT_WORK;


//==========================================================================================
// ■プロトタイプ宣言
//========================================================================================== 
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Ananukenohimo( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Anawohoru( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Teleport( GMEVENT* event, int* seq, void* wk );


//========================================================================================== 
// ■公開関数
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( 落下 )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_Fall( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Fall, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap = fieldmap;
  work->gsys     = gsys;

  // 作成したイベントを返す
  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( あなぬけのヒモ )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_Ananukenohimo( GMEVENT* parent, 
                                     GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Ananukenohimo, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap = fieldmap;
  work->gsys     = gsys;

  // 作成したイベントを返す
  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( あなをほる )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_Anawohoru( GMEVENT* parent, 
                                 GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Anawohoru, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap = fieldmap;
  work->gsys     = gsys;

  // 作成したイベントを返す
  return event;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベントを作成する( テレポート )
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_APPEAR_Teleport( GMEVENT* parent, 
                                GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
  GMEVENT*   event;
  EVENT_WORK* work;

  // イベントを作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_APPEAR_Teleport, sizeof( EVENT_WORK ) );

  // イベントワークを初期化
  work           = (EVENT_WORK*)GMEVENT_GetEventWork( event );
  work->fieldmap = fieldmap;
  work->gsys     = gsys;

  // 作成したイベントを返す
  return event;
}


//========================================================================================== 
// ■非公開関数の定義
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( 落下 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work = (EVENT_WORK*)wk;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( *seq )
  {
  case 0:
    // カメラモードの設定
    work->cameraMode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    { // 主人公の描画オフセットの初期設定(画面外にいるように設定)
      VecFx32       offset = {0, 100<<FX32_SHIFT, 0};
      FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
      MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
      MMDL_SetVectorDrawOffsetPos( mmdl, &offset );
    }
    { // フェードイン
      GMEVENT* fadeInEvent;
      fadeInEvent = EVENT_FieldFadeIn_Black( work->gsys, work->fieldmap,  FIELD_FADE_NO_WAIT );
      GMEVENT_CallEvent( event, fadeInEvent );
    }
    { // タスクの追加
      FIELD_TASK* fall;
      FIELD_TASK_MAN* man;
      fall = FIELD_TASK_PlayerFall( work->fieldmap, 40, 250 );
      man  = FIELDMAP_GetTaskManager( work->fieldmap ); 
      FIELD_TASK_MAN_AddTask( man, fall, NULL );
    }
    ++( *seq );
    break;
  case 1:
    { // タスクの終了待ち
      FIELD_TASK_MAN* man;
      man = FIELDMAP_GetTaskManager( work->fieldmap ); 
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        ++( *seq );
      }
    }
    break;
  case 2:
    // カメラモードの復帰
    FIELD_CAMERA_ChangeMode( camera, work->cameraMode );
    ++( *seq );
    break;
  case 3: 
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( あなぬけのヒモ )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Ananukenohimo( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work = (EVENT_WORK*)wk;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( *seq )
  {
  case 0:
    // カメラモードの設定
    work->cameraMode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    // カメラ初期設定
    FIELD_CAMERA_SetAngleLen( camera, ZOOM_IN_DIST );
    { // フェードイン
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_White( work->gsys, work->fieldmap,  FIELD_FADE_NO_WAIT );
      GMEVENT_CallEvent( event, fade_event );
    }
    { // タスクの追加
      FLD_CAMERA_PARAM def_param; 
      fx32 val_dist;
      FIELD_TASK* rot1;
      FIELD_TASK* rot2;
      FIELD_TASK* zoom;
      FIELD_TASK_MAN* man;
      u32 fadein_frame;
      FIELD_CAMERA_GetInitialParameter( camera, &def_param );
      fadein_frame = GetSeasonDispEventFrame( work->gsys );  // 季節表示にかかる時間を取得
      val_dist = def_param.Distance << FX32_SHIFT;
      rot1 = FIELD_TASK_PlayerRotate( work->fieldmap, fadein_frame, fadein_frame/5 );
      rot2 = FIELD_TASK_PlayerRotate_SpeedDown( work->fieldmap, 60, 8 );
      zoom = FIELD_TASK_CameraLinearZoom( work->fieldmap, ZOOM_OUT_FRAME, val_dist );
      man  = FIELDMAP_GetTaskManager( work->fieldmap ); 
      FIELD_TASK_MAN_AddTask( man, rot1, NULL );
      FIELD_TASK_MAN_AddTask( man, rot2, rot1 );
      FIELD_TASK_MAN_AddTask( man, zoom, rot1 );
    }
    ++( *seq );
    break;
  case 1:
    { // タスクの終了待ち
      FIELD_TASK_MAN* man;
      man  = FIELDMAP_GetTaskManager( work->fieldmap ); 
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        ++( *seq );
      }
    }
    break;
  case 2:
    // カメラモードの復帰
    FIELD_CAMERA_ChangeMode( camera, work->cameraMode );
    ++( *seq );
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( あなをほる )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Anawohoru( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work = (EVENT_WORK*)wk;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( *seq )
  {
  case 0:
    // カメラモードの設定
    work->cameraMode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    // カメラ初期設定
    FIELD_CAMERA_SetAngleLen( camera, ZOOM_IN_DIST );
    { // フェードイン
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black( work->gsys, work->fieldmap,  FIELD_FADE_NO_WAIT );
      GMEVENT_CallEvent( event, fade_event );
    }
    { // タスクの追加
      FLD_CAMERA_PARAM def_param; 
      fx32 val_dist;
      FIELD_TASK* rot1;
      FIELD_TASK* rot2;
      FIELD_TASK* zoom;
      FIELD_TASK_MAN* man;
      u32 fadein_frame;
      FIELD_CAMERA_GetInitialParameter( camera, &def_param );
      fadein_frame = GetSeasonDispEventFrame( work->gsys );  // 季節表示にかかる時間を取得
      val_dist = def_param.Distance << FX32_SHIFT;
      rot1    = FIELD_TASK_PlayerRotate( work->fieldmap, fadein_frame, fadein_frame/5 );
      rot2    = FIELD_TASK_PlayerRotate_SpeedDown( work->fieldmap, 60, 8 );
      zoom    = FIELD_TASK_CameraLinearZoom( work->fieldmap, ZOOM_OUT_FRAME, val_dist );
      man     = FIELDMAP_GetTaskManager( work->fieldmap ); 
      FIELD_TASK_MAN_AddTask( man, rot1, NULL );
      FIELD_TASK_MAN_AddTask( man, rot2, rot1 );
      FIELD_TASK_MAN_AddTask( man, zoom, rot1 );
      // @todo SE
    }
    ++( *seq );
    break;
  case 1:
    { // タスクの終了待ち
      FIELD_TASK_MAN* man;
      man  = FIELDMAP_GetTaskManager( work->fieldmap ); 
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        ++( *seq );
      }
    }
    break;
  case 2:
    // カメラモードの復帰
    FIELD_CAMERA_ChangeMode( camera, work->cameraMode );
    ++( *seq );
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( テレポート )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Teleport( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*     work = (EVENT_WORK*)wk;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( *seq )
  {
  case 0:
    // カメラモードの設定
    work->cameraMode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    // カメラ初期設定
    FIELD_CAMERA_SetAngleLen( camera, ZOOM_IN_DIST );
    { // フェードイン
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black( work->gsys, work->fieldmap,  FIELD_FADE_NO_WAIT );
      GMEVENT_CallEvent( event, fade_event );
    }
    { // タスクの追加
      FLD_CAMERA_PARAM def_param; 
      fx32 val_dist;
      FIELD_TASK* rot1;
      FIELD_TASK* rot2;
      FIELD_TASK* zoom;
      FIELD_TASK_MAN* man;
      u32 fadein_frame;
      FIELD_CAMERA_GetInitialParameter( camera, &def_param );
      fadein_frame = GetSeasonDispEventFrame( work->gsys );  // 季節表示にかかる時間を取得
      val_dist = def_param.Distance << FX32_SHIFT;
      rot1    = FIELD_TASK_PlayerRotate( work->fieldmap, fadein_frame, fadein_frame/5 );
      rot2    = FIELD_TASK_PlayerRotate_SpeedDown( work->fieldmap, 60, 8 );
      zoom    = FIELD_TASK_CameraLinearZoom( work->fieldmap, ZOOM_OUT_FRAME, val_dist );
      man     = FIELDMAP_GetTaskManager( work->fieldmap ); 
      FIELD_TASK_MAN_AddTask( man, rot1, NULL );
      FIELD_TASK_MAN_AddTask( man, rot2, rot1 );
      FIELD_TASK_MAN_AddTask( man, zoom, rot1 );
    }
    ++( *seq );
    break;
  case 1:
    { // タスクの終了待ち
      FIELD_TASK_MAN* man;
      man  = FIELDMAP_GetTaskManager( work->fieldmap ); 
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        { // 煙エフェクト表示
          FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
          MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player );
          FLDEFF_CTRL*  fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
          FLDEFF_KEMURI_SetMMdl( mmdl, fectrl );
        }
        ++( *seq );
      }
    }
    break;
  case 2:
    // カメラモードの復帰
    FIELD_CAMERA_ChangeMode( camera, work->cameraMode );
    ++( *seq );
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
