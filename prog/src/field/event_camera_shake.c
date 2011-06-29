/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  カメラ振動イベント
 * @file   event_camera_shake.c
 * @author obata
 * @date   2010.04.17
 *
 * ※scrcmd_camera.c より移動
 */
/////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/game_data.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "field_camera.h"
#include "event_camera_shake.h"


//===============================================================================
// ■イベントワーク
//===============================================================================
typedef struct CAM_SHAKE_WORK_tag {

  FIELD_TASK* task;
  CAM_SHAKE_PARAM shakeParam;
  const VecFx32 *WatchTarget;

} CAM_SHAKE_WORK;


//===============================================================================
//===============================================================================
static GMEVENT_RESULT CameraShakeEvt( GMEVENT* event, int* seq, void* wk );


//===============================================================================
// ■public functions
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief カメラ揺れイベントを生成する
 *
 * @param gameSystem
 * @param shakeParam 振動パラメータ
 *
 * @return 生成したイベント
 */
//-------------------------------------------------------------------------------
GMEVENT* EVENT_ShakeCamera( 
    GAMESYS_WORK* gameSystem, CAM_SHAKE_PARAM* shakeParam )
{
  GMEVENT* event;
  CAM_SHAKE_WORK* work;

  // デフォルトパラメータを設定
  shakeParam->WaitStartFrame = 0;
  shakeParam->ShakeType      = CAM_SHAKE_TYPE_ALL;

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, CameraShakeEvt, sizeof(CAM_SHAKE_WORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event ); 
  work->shakeParam = *shakeParam;

  return event;
}


//===============================================================================
// ■private functions
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief カメラ揺れイベント
 * @param   event　　　       イベントポインタ
 * @param   seq　　　　       シーケンス
 * @param   wk　　　　        ワークポインタ
 * @return  GMEVENT_RESULT    GMEVENT_RES_FINISHでイベント終了
 */
//-------------------------------------------------------------------------------
static GMEVENT_RESULT CameraShakeEvt( GMEVENT* event, int* seq, void* wk )
{ 
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  CAM_SHAKE_WORK *work = GMEVENT_GetEventWork(event);
  CAM_SHAKE_PARAM* p_param = &(work->shakeParam);

  switch(*seq){
  // フィールドタスク呼び出し
  case 0:
    {
      FIELD_TASK_MAN* taskMan;
      work->task = FIELD_TASK_ShakeCamera( fieldmap, p_param );
      taskMan = FIELDMAP_GetTaskManager( fieldmap );
      FIELD_TASK_MAN_AddTask( taskMan, work->task, NULL );
      (*seq)++; // フィールドタスク終了待ちへ
    }
    break;
  case 1:
    {
      if( FIELD_TASK_IsEnd( work->task ) == TRUE )
      {
        // カメラ振動タスクが終了していたらイベント終了
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

