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
  // 出入り口タイプに対応するカメラデータのインデックス
  const ARCDATID dataID[ EXIT_TYPE_MAX ] = 
  {
    0, 0, 0, 0, 0, 0, 0,  // EXIT_TYPE_NONE - EXIT_TYPE_INTRUDE
    NARC_entrance_camera_exit_type_sp1_bin,    // EXIT_TYPE_SP1
    NARC_entrance_camera_exit_type_sp2_bin,    // EXIT_TYPE_SP2
    NARC_entrance_camera_exit_type_sp3_bin,    // EXIT_TYPE_SP3
    NARC_entrance_camera_exit_type_sp4_bin,    // EXIT_TYPE_SP4
    NARC_entrance_camera_exit_type_sp5_bin,    // EXIT_TYPE_SP5
    NARC_entrance_camera_exit_type_sp6_bin,    // EXIT_TYPE_SP6
    NARC_entrance_camera_exit_type_sp7_bin,    // EXIT_TYPE_SP7
    NARC_entrance_camera_exit_type_sp8_bin,    // EXIT_TYPE_SP8
    NARC_entrance_camera_exit_type_sp9_bin,    // EXIT_TYPE_SP9
    NARC_entrance_camera_exit_type_sp10_bin,   // EXIT_TYPE_SP10
    NARC_entrance_camera_exit_type_sp11_bin,   // EXIT_TYPE_SP11
    NARC_entrance_camera_exit_type_sp12_bin,   // EXIT_TYPE_SP12
    NARC_entrance_camera_exit_type_sp13_bin,   // EXIT_TYPE_SP13
    NARC_entrance_camera_exit_type_sp14_bin,   // EXIT_TYPE_SP14
    NARC_entrance_camera_exit_type_sp15_bin,   // EXIT_TYPE_SP15
    NARC_entrance_camera_exit_type_sp16_bin,   // EXIT_TYPE_SP16
    NARC_entrance_camera_exit_type_sp17_bin,   // EXIT_TYPE_SP17
    NARC_entrance_camera_exit_type_sp18_bin,   // EXIT_TYPE_SP18
    NARC_entrance_camera_exit_type_sp19_bin,   // EXIT_TYPE_SP19
    NARC_entrance_camera_exit_type_sp20_bin,   // EXIT_TYPE_SP20
    NARC_entrance_camera_exit_type_sp21_bin,   // EXIT_TYPE_SP21
    NARC_entrance_camera_exit_type_sp22_bin,   // EXIT_TYPE_SP22
    NARC_entrance_camera_exit_type_sp23_bin,   // EXIT_TYPE_SP23
    NARC_entrance_camera_exit_type_sp24_bin,   // EXIT_TYPE_SP24
    NARC_entrance_camera_exit_type_sp25_bin,   // EXIT_TYPE_SP25
    NARC_entrance_camera_exit_type_sp26_bin,   // EXIT_TYPE_SP26
    NARC_entrance_camera_exit_type_sp27_bin,   // EXIT_TYPE_SP27
    NARC_entrance_camera_exit_type_sp28_bin,   // EXIT_TYPE_SP28
    NARC_entrance_camera_exit_type_sp29_bin,   // EXIT_TYPE_SP29
    NARC_entrance_camera_exit_type_sp30_bin,   // EXIT_TYPE_SP30
    NARC_entrance_camera_exit_type_sp31_bin,   // EXIT_TYPE_SP31
    NARC_entrance_camera_exit_type_sp32_bin,   // EXIT_TYPE_SP32
    NARC_entrance_camera_exit_type_sp33_bin,   // EXIT_TYPE_SP33
    NARC_entrance_camera_exit_type_sp34_bin,   // EXIT_TYPE_SP34
    NARC_entrance_camera_exit_type_sp35_bin,   // EXIT_TYPE_SP35
    NARC_entrance_camera_exit_type_sp36_bin,   // EXIT_TYPE_SP36
    NARC_entrance_camera_exit_type_sp37_bin,   // EXIT_TYPE_SP37
    NARC_entrance_camera_exit_type_sp38_bin,   // EXIT_TYPE_SP38
    NARC_entrance_camera_exit_type_sp39_bin,   // EXIT_TYPE_SP39
    NARC_entrance_camera_exit_type_sp40_bin,   // EXIT_TYPE_SP40
    NARC_entrance_camera_exit_type_sp41_bin,   // EXIT_TYPE_SP41
    NARC_entrance_camera_exit_type_sp42_bin,   // EXIT_TYPE_SP42
    NARC_entrance_camera_exit_type_sp43_bin,   // EXIT_TYPE_SP43
    NARC_entrance_camera_exit_type_sp44_bin,   // EXIT_TYPE_SP44
    NARC_entrance_camera_exit_type_sp45_bin,   // EXIT_TYPE_SP45
    NARC_entrance_camera_exit_type_sp46_bin,   // EXIT_TYPE_SP46
    NARC_entrance_camera_exit_type_sp47_bin,   // EXIT_TYPE_SP47
    NARC_entrance_camera_exit_type_sp48_bin,   // EXIT_TYPE_SP48
    NARC_entrance_camera_exit_type_sp49_bin,   // EXIT_TYPE_SP49
    NARC_entrance_camera_exit_type_sp50_bin,   // EXIT_TYPE_SP50
  };

  GF_ASSERT( EXIT_TYPE_SP1 <= exitType );
  GF_ASSERT( exitType < EXIT_TYPE_MAX );

  // 読み込み
  GFL_ARC_LoadData( dest, ARCID_ENTRANCE_CAMERA, dataID[ exitType ] );

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






















