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
    GAMESYS_WORK* gameSystem, const CAM_SHAKE_PARAM* shakeParam )
{
  GMEVENT* event;
  CAM_SHAKE_WORK* work;

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
  FIELD_CAMERA * camera;
  BOOL end = FALSE;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  CAM_SHAKE_WORK *work = GMEVENT_GetEventWork(event);
  CAM_SHAKE_PARAM* param = &(work->shakeParam);

  camera = FIELDMAP_GetFieldCamera( fieldmap );
  switch(*seq){
  case 0:
    //トレース終了
    FIELD_CAMERA_StopTraceRequest(camera);
    (*seq)++;
    break;
  case 1:
    if ( !FIELD_CAMERA_CheckTrace(camera) )
    {
      //カメラのターゲットを取得
      work->WatchTarget = FIELD_CAMERA_GetWatchTarget(camera);
      //バインド解除
      FIELD_CAMERA_FreeTarget(camera);
      
      (*seq)++;
    }
    break;
  case 2:
    {
      //ターゲットを動かす
      int tmp;
      u16 rad;
      param->NowSync++;
      tmp = (param->NowSync * 0x10000) / param->Sync;
      rad = tmp;

      if ( param->SubStartTime != 0)
      {
        if ( param->TimeMax-param->Time >= param->SubStartTime )
        {
          param->SubMargineCount++;
          if (param->SubMargineCount>=param->SubMargine)
          {
            param->SubMargineCount = 0;
            param->Height -= param->SubH;
            param->Width -= param->SubW;
            if (param->Height <= 0) param->Height = 1;
            if (param->Width <= 0) param->Width = 1;
          }
        }
      }

      if (param->NowSync >= param->Sync)
      {
        param->Time--;
        param->NowSync = 0;
      }
      if (param->Time == 0)
      {
        param->NowSync = 0;
        end = TRUE;
      }

      if (end)
      {
        //カメラオフセットクリア
        {
          VecFx32 cam_ofs = {0,0,0};
          FIELD_CAMERA_SetCamPosOffset( camera, &cam_ofs );
        }
        //際バインド
        FIELD_CAMERA_BindTarget(camera, work->WatchTarget);
        //トレース再開
        FIELD_CAMERA_RestartTrace(camera);
        (*seq)++;
      }
      else
      {
        fx32 w, h;
        VecFx32 target;
        VecFx32 cam_ofs;
        w = param->Width * FX_SinIdx(rad);
        h = param->Height * FX_SinIdx(rad);
        target = *work->WatchTarget;
        target.x += w;
        target.y += h;
        cam_ofs.x = w;
        cam_ofs.y = h;
        cam_ofs.z = 0;

        //座標セット
        FIELD_CAMERA_SetTargetPos( camera, &target );
        FIELD_CAMERA_SetCamPosOffset( camera, &cam_ofs );
      }
    }
    break;
  case 3:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;

}
