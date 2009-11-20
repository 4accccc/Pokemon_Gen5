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
#include "fieldmap_tcb.h"
#include "sound/pm_sndsys.h"
#include "field_task.h"
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_player_rot.h"
#include "field_task_player_fall.h"


//==========================================================================================
// ■定数
//========================================================================================== 
#define ZOOM_IN_DIST   (50 << FX32_SHIFT)   // カメラのズームイン距離
#define ZOOM_IN_FRAME  (60)   // ズームインに掛かるフレーム数
#define ZOOM_OUT_FRAME (60)   // ズームアウトに掛かるフレーム数


//==========================================================================================
// ■イベントワーク
//==========================================================================================
typedef struct
{
  // 動作フィールドマップ
  FIELDMAP_WORK* fieldmap;   

} EVENT_WORK;


//==========================================================================================
// ■プロトタイプ宣言
//========================================================================================== 
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Rotate( GMEVENT* event, int* seq, void* wk );   // 回転
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* wk );     // 落下


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

  // 作成したイベントを返す
  return event;
}


//========================================================================================== 
// ■非公開関数の定義
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( 回転 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Rotate( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq )
  {
  case 0:
    { // カメラ初期設定
      FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );
      FIELD_CAMERA_SetAngleLen( camera, FIELD_CAMERA_GetAngleLen( camera ) - ZOOM_IN_DIST );
    }
    // フェードイン開始
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 1 );
    ++( *seq );
    break;
  case 1:
    { // タスクの追加
      FIELD_TASK* rot;
      FIELD_TASK* zoom;
      FIELD_TASK_MAN* man;
      rot  = FIELD_TASK_PlayerRotate_SpeedDown( work->fieldmap, 120, 10 );
      zoom = FIELD_TASK_CameraLinearZoom( work->fieldmap, ZOOM_OUT_FRAME, ZOOM_IN_DIST );
      man  = FIELDMAP_GetTaskManager( work->fieldmap ); 
      FIELD_TASK_MAN_AddTask( man, rot, NULL );
      FIELD_TASK_MAN_AddTask( man, zoom, NULL );
    }
    ++( *seq );
    break;
  case 2:
    { // タスクの終了待ち
      FIELD_TASK_MAN* man;
      man  = FIELDMAP_GetTaskManager( work->fieldmap ); 
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        ++( *seq );
      }
    }
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 登場イベント処理関数( 落下 )
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_APPEAR_Fall( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  switch( *seq )
  {
  case 0:
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
    // フェードイン開始
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0 );
    ++( *seq );
    break;
  case 2:
    { // タスクの終了待ち
      FIELD_TASK_MAN* man;
      man = FIELDMAP_GetTaskManager( work->fieldmap ); 
      if( FIELD_TASK_MAN_IsAllTaskEnd(man) )
      {
        ++( *seq );
      }
    }
    break;
  case 3: 
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
