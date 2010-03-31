/////////////////////////////////////////////////////////////////////
/**
 * @brief  特殊出入り口のカメラ演出データ
 * @file   entrance_camera.c
 * @author obata
 * @date   2010.02.16
 */
/////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "entrance_camera.h"

#include "fieldmap.h"
#include "field_task.h"  
#include "field_task_manager.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_target_offset.h"

#include "field/eventdata_type.h"  // for EXIT_TYPE_xxxx
#include "arc/arc_def.h"           // for ARCID_xxxx
#include "../../resource/fldmapdata/entrance_camera/entrance_camera.naix"  // for NARC_xxxx

// デバッグ出力スイッチ
#define DEBUG_PRINT_ON


//-------------------------------------------------------------------
/**
 * @brief 指定した出入り口タイプに該当するデータを取得する
 *
 * @param dest     取得したデータの格納先
 * @param exitType データ取得対象の出入り口タイプ
 */
//-------------------------------------------------------------------
void ENTRANCE_CAMERA_LoadData( ENTRANCE_CAMERA* dest, EXIT_TYPE exitType )
{
  u32 datID;

  // EXIT_TYPE_SPxx以外はダメ
  GF_ASSERT( EXIT_TYPE_SP1 <= exitType );
  GF_ASSERT( exitType < EXIT_TYPE_MAX );

  // 読み込むデータを決定
  datID = exitType - EXIT_TYPE_SP1;

  // 読み込み
  GFL_ARC_LoadData( dest, ARCID_ENTRANCE_CAMERA, datID );

#ifdef DEBUG_PRINT_ON
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: frame   = %d\n", dest->frame );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: pitch   = %x\n", dest->pitch );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: yaw     = %x\n", dest->yaw );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: length  = %x\n", dest->length );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: offsetX = %x\n", dest->targetOffsetX );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: offsetY = %x\n", dest->targetOffsetX );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: offsetZ = %x\n", dest->targetOffsetX );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: validFlag_IN  = %x\n", dest->validFlag_IN );
  OBATA_Printf( "ENTRANCE-CAMERA-SETTINGS: validFlag_OUT = %x\n", dest->validFlag_OUT );
#endif
}


//-------------------------------------------------------------------
/**
 * @brief 特殊出入り口に入る時のカメラ動作タスクを登録する
 *
 * @param fieldmap
 * @param data     適用するカメラ動作データ
 */
//-------------------------------------------------------------------
void ENTRANCE_CAMERA_AddDoorInTask( 
    FIELDMAP_WORK* fieldmap, const ENTRANCE_CAMERA* data )
{
  GF_ASSERT( data->validFlag_IN ); // 入る時に有効なデータではない

  {
    u16 frame;
    u16 pitch, yaw;
    fx32 length;
    VecFx32 targetOffset;

    // 各パラメータ取得
    frame  = data->frame;
    pitch  = data->pitch;
    yaw    = data->yaw;
    length = data->length << FX32_SHIFT;
    VEC_Set( &targetOffset, 
        data->targetOffsetX << FX32_SHIFT,
        data->targetOffsetY << FX32_SHIFT,
        data->targetOffsetZ << FX32_SHIFT );

    // タスク登録
    {
      FIELD_TASK_MAN* taskMan;
      FIELD_TASK* zoomTaks;
      FIELD_TASK* pitchTask;
      FIELD_TASK* yawTask;
      FIELD_TASK* targetOffsetTask;

      // タスクを生成
      zoomTaks         = FIELD_TASK_CameraLinearZoom  ( fieldmap, frame, length );
      pitchTask        = FIELD_TASK_CameraRot_Pitch   ( fieldmap, frame, pitch );
      yawTask          = FIELD_TASK_CameraRot_Yaw     ( fieldmap, frame, yaw );
      targetOffsetTask = FIELD_TASK_CameraTargetOffset( fieldmap, frame, &targetOffset );

      // タスクを登録
      taskMan = FIELDMAP_GetTaskManager( fieldmap );
      FIELD_TASK_MAN_AddTask( taskMan, zoomTaks, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, pitchTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, yawTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, targetOffsetTask, NULL );
    }
  }
}

//-------------------------------------------------------------------
/**
 * @brief 特殊出入り口から出る時のカメラ動作タスクを登録する
 *
 * @param fieldmap
 * @param data     適用するカメラ動作データ
 */
//-------------------------------------------------------------------
void ENTRANCE_CAMERA_AddDoorOutTask( 
    FIELDMAP_WORK* fieldmap, const ENTRANCE_CAMERA* data )
{
  FIELD_CAMERA* camera;

  GF_ASSERT( data->validFlag_OUT ); // 出る時に有効なデータではない

  camera = FIELDMAP_GetFieldCamera( fieldmap );

  {
    u16 frame;
    u16 pitch, yaw;
    fx32 length;
    VecFx32 targetOffset;

    // 各パラメータ取得
    {
      FLD_CAMERA_PARAM defaultParam; 

      // デフォルトパラメータを取得
      FIELD_CAMERA_GetInitialParameter( camera, &defaultParam );

      frame  = data->frame;
      pitch  = defaultParam.Angle.x;
      yaw    = defaultParam.Angle.y;
      length = defaultParam.Distance << FX32_SHIFT;
      VEC_Set( &targetOffset, 
          defaultParam.Shift.x, defaultParam.Shift.y, defaultParam.Shift.z );
    }

    // タスク登録
    {
      FIELD_TASK_MAN* taskMan;
      FIELD_TASK* zoomTaks;
      FIELD_TASK* pitchTask;
      FIELD_TASK* yawTask;
      FIELD_TASK* targetOffsetTask;

      // タスクを生成
      zoomTaks         = FIELD_TASK_CameraLinearZoom  ( fieldmap, frame, length );
      pitchTask        = FIELD_TASK_CameraRot_Pitch   ( fieldmap, frame, pitch );
      yawTask          = FIELD_TASK_CameraRot_Yaw     ( fieldmap, frame, yaw );
      targetOffsetTask = FIELD_TASK_CameraTargetOffset( fieldmap, frame, &targetOffset );

      // タスクを登録
      taskMan = FIELDMAP_GetTaskManager( fieldmap );
      FIELD_TASK_MAN_AddTask( taskMan, zoomTaks, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, pitchTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, yawTask, NULL );
      FIELD_TASK_MAN_AddTask( taskMan, targetOffsetTask, NULL );
    }
  }
}

//-------------------------------------------------------------------
/**
 * @brief 特殊出入り口から出る時のカメラ動作タスクのための初期化を行う
 *
 * @param fieldmap
 * @param data     適用するカメラ動作データ
 */
//-------------------------------------------------------------------
void ENTRANCE_CAMERA_PrepareForDoorOut( 
    FIELDMAP_WORK* fieldmap, const ENTRANCE_CAMERA* data )
{
  FIELD_CAMERA* camera;
  VecFx32 targetOffset;

  GF_ASSERT( data->validFlag_OUT ); // 出る時に有効なデータではない

  camera = FIELDMAP_GetFieldCamera( fieldmap );

  VEC_Set( &targetOffset, 
      data->targetOffsetX << FX32_SHIFT,
      data->targetOffsetY << FX32_SHIFT,
      data->targetOffsetZ << FX32_SHIFT );

  FIELD_CAMERA_SetAnglePitch( camera, data->pitch );
  FIELD_CAMERA_SetAngleYaw( camera, data->yaw );
  FIELD_CAMERA_SetAngleLen( camera, data->length << FX32_SHIFT );
  FIELD_CAMERA_SetTargetOffset( camera, &targetOffset );
} 
