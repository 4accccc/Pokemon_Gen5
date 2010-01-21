////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  電光掲示板を見るイベント
 * @file   event_look_elboard.c
 * @author obata
 * @date   2009.11.25
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "field_task.h"
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_target_offset.h"
#include "field_gimmick_gate.h"
#include "event_look_elboard.h"


//==========================================================================================
// ■定数
//==========================================================================================
// イベントシーケンス
enum
{
  SEQ_INIT,               // イベント初期化
  SEQ_CREATE_LOOK_TASK,   // カメラ移動タスクの作成
  SEQ_WAIT_CAMERA_SET,    // カメラの移動完了待ち
  SEQ_KEY_WAIT,           // キー入力待ち
  SEQ_CREATE_RESET_TASK,  // カメラリセット タスクの作成
  SEQ_WAIT_CAMERA_RESET,  // カメラリセット完了待ち
  SEQ_EXIT,               // イベント終了
};


//==========================================================================================
// ■イベントワーク
//==========================================================================================
typedef struct
{
  GAMESYS_WORK*  gameSystem;
  FIELDMAP_WORK* fieldmap; 

  u16 cameraAnimeFrame;  // カメラ移動に要するフレーム数

} EVENT_WORK;


//==========================================================================================
// ■イベント処理関数
//==========================================================================================
static GMEVENT_RESULT SeasonDisplay( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK*            work = (EVENT_WORK*)wk;
  FIELDMAP_WORK*     fieldmap = work->fieldmap;
  FIELD_CAMERA*        camera = FIELDMAP_GetFieldCamera( fieldmap );
  FIELD_TASK_MAN* taskManager = FIELDMAP_GetTaskManager( fieldmap );

  switch( *seq )
  {
  // イベント初期化
  case SEQ_INIT:
    ++(*seq);
    break;
  // カメラ移動タスクの作成
  case SEQ_CREATE_LOOK_TASK:
    {
      u32 pitch, yaw;
      fx32 length;
      VecFx32 targetOffset; 
      // カメラ最終パラメータ決定
      switch( GATE_GIMMICK_GetElboardDir(fieldmap) )
      {
      default:
        OBATA_Printf( "GIMMICK-GATE: 掲示板の向きがカメラアニメ未対応\n" );
        GF_ASSERT(0);
      case DIR_DOWN:
        pitch  = 0x0ee5;
        yaw    = 0;
        length = 0x0086 << FX32_SHIFT;
        VEC_Set( &targetOffset, 0, 0x001b<<FX32_SHIFT, 0xfff94000 );
        break;
      case DIR_RIGHT: 
        pitch  = 0x0ee5;
        yaw    = 0x3fff;
        length = 0x0086 << FX32_SHIFT;
        VEC_Set( &targetOffset, 0xfff94000, 0x001b<<FX32_SHIFT, 0 );
        break;
      } 
      // タスク登録
      {
        FIELD_TASK* zoomTask;
        FIELD_TASK* pitchTask;
        FIELD_TASK* yawTask;
        FIELD_TASK* targetOffsetTask;
        // 生成
        zoomTask  = FIELD_TASK_CameraLinearZoom( fieldmap, work->cameraAnimeFrame, length );
        pitchTask = FIELD_TASK_CameraRot_Pitch( fieldmap, work->cameraAnimeFrame, pitch );
        yawTask   = FIELD_TASK_CameraRot_Yaw( fieldmap, work->cameraAnimeFrame, yaw );
        targetOffsetTask = FIELD_TASK_CameraTargetOffset( fieldmap, work->cameraAnimeFrame, &targetOffset );
        // 登録
        FIELD_TASK_MAN_AddTask( taskManager, zoomTask, NULL );
        FIELD_TASK_MAN_AddTask( taskManager, pitchTask, NULL );
        FIELD_TASK_MAN_AddTask( taskManager, yawTask, NULL );
        FIELD_TASK_MAN_AddTask( taskManager, targetOffsetTask, NULL );
      }
    }
    ++(*seq);
    break;
  // カメラの移動完了待ち
  case SEQ_WAIT_CAMERA_SET:
    // 全タスク処理終了で次のシーケンスへ
    if( FIELD_TASK_MAN_IsAllTaskEnd(taskManager) ){ ++(*seq); } 
    break;
  // キー入力待ち
  case SEQ_KEY_WAIT:
    {
      int trg = GFL_UI_KEY_GetTrg();
      if( (trg & PAD_KEY_LEFT)  || (trg & PAD_KEY_RIGHT) ||
          (trg & PAD_KEY_DOWN)  || (trg & PAD_BUTTON_B) ){ ++(*seq); }
    }
    break;
  // カメラリセット タスクの作成
  case SEQ_CREATE_RESET_TASK:
    {
      u16 pitch, yaw;
      fx32 length;
      VecFx32 targetOffset = {0, 0, 0};
      FLD_CAMERA_PARAM defaultParam;
      // デフォルトパラメータ取得
      FIELD_CAMERA_GetInitialParameter( camera, &defaultParam );
      pitch  = defaultParam.Angle.x;
      yaw    = defaultParam.Angle.y;
      length = defaultParam.Distance << FX32_SHIFT;
      // タスク登録
      {
        FIELD_TASK* zoomTask;
        FIELD_TASK* pitchTask;
        FIELD_TASK* yawTask;
        FIELD_TASK* targetOffsetTask;
        zoomTask  = FIELD_TASK_CameraLinearZoom( fieldmap, work->cameraAnimeFrame, length );
        pitchTask = FIELD_TASK_CameraRot_Pitch( fieldmap, work->cameraAnimeFrame, pitch );
        yawTask   = FIELD_TASK_CameraRot_Yaw( fieldmap, work->cameraAnimeFrame, yaw );
        targetOffsetTask = FIELD_TASK_CameraTargetOffset( fieldmap, work->cameraAnimeFrame, &targetOffset );
        FIELD_TASK_MAN_AddTask( taskManager, zoomTask, NULL );
        FIELD_TASK_MAN_AddTask( taskManager, pitchTask, NULL );
        FIELD_TASK_MAN_AddTask( taskManager, yawTask, NULL );
        FIELD_TASK_MAN_AddTask( taskManager, targetOffsetTask, NULL );
      }
    }
    ++(*seq);
    break;
  // カメラリセット完了待ち
  case SEQ_WAIT_CAMERA_RESET:
    // 全タスク処理終了で次のシーケンスへ
    if( FIELD_TASK_MAN_IsAllTaskEnd(taskManager) ){ ++(*seq); } 
    break;
  // イベント終了
  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


//==========================================================================================
// ■外部公開関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief 電光掲示板を見るイベントを生成する
 *
 * @param gameSystem
 * @param fieldmap 
 * @param cameraAnimeFrame カメラの移動に要するフレーム数
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* EVENT_LookElboard( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, u16 cameraAnimeFrame )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // イベント生成
  event = GMEVENT_Create( gameSystem, NULL, SeasonDisplay, sizeof(EVENT_WORK) ); 
  // イベントワーク初期化
  work                   = GMEVENT_GetEventWork( event );
  work->gameSystem       = gameSystem;
  work->fieldmap         = fieldmap;
  work->cameraAnimeFrame = cameraAnimeFrame;
  return event;
} 
