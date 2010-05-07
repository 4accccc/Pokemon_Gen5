////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク(カメラの回転処理)
 * @file   field_task_camera_rot.c
 * @author obata
 * @date   2009.11.18
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "field_task.h"
#include "field_task_camera_rot.h"
#include "field/field_camera.h"


//==========================================================================================
// ■定数
//==========================================================================================
//#define DEBUG_EVENT_ON
#define PI (0x8000)   // π[rad]

// 回転タイプ
typedef enum {
  CAMERA_ROT_TYPE_YAW,    // ヨー回転
  CAMERA_ROT_TYPE_PITCH,  // ピッチ回転
} CAMERA_ROT_TYPE;


//==========================================================================================
// ■タスクワーク
//==========================================================================================
typedef struct {

  FIELDMAP_WORK*  fieldmap;
  FIELD_CAMERA*   camera;

  CAMERA_ROT_TYPE rot_type;    // 回転タイプ
  u8              seq;         // シーケンス
  u16             now_frame;   // 経過フレーム数
  u16             end_frame;   // 終了フレーム
  u32             start_angle; // 回転角初期値(2πラジアンを65536分割した値を単位とする数)
  u32             end_angle;   // 回転角最終値(2πラジアンを65536分割した値を単位とする数)

} ROT_WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void SetupStartAngle( ROT_WORK* work );
static void UpdateAngle( ROT_WORK* work );
static FIELD_TASK_RETVAL RotateCamera( void* wk );


//------------------------------------------------------------------------------------------
/**
 * @brief カメラのヨー回転タスクを作成する
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param angle    変化角( 2πラジアンを65536分割した値を単位とする数 )
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_CameraRot_Yaw( FIELDMAP_WORK* fieldmap, int frame, u16 angle )
{
  ROT_WORK* work;
  FIELD_TASK* task;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // タスクを生成
  task = FIELD_TASK_Create( heap_id, sizeof(ROT_WORK), RotateCamera );

  // タスクワークを初期化
  work = FIELD_TASK_GetWork( task );
  work->fieldmap  = fieldmap;
  work->camera    = FIELDMAP_GetFieldCamera( fieldmap );
  work->seq       = 0;
  work->rot_type  = CAMERA_ROT_TYPE_YAW;
  work->now_frame = 0;
  work->end_frame = frame;
  work->end_angle = angle;
  
  return task;
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラのピッチ回転タスクを作成する
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param angle    変化角( 2πラジアンを65536分割した値を単位とする数 )
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_CameraRot_Pitch( FIELDMAP_WORK* fieldmap, int frame, u16 angle )
{
  ROT_WORK* work;
  FIELD_TASK* task;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // タスクを生成
  task = FIELD_TASK_Create( heap_id, sizeof(ROT_WORK), RotateCamera );

  // タスクワークを初期化
  work = FIELD_TASK_GetWork( task );
  work->fieldmap   = fieldmap;
  work->camera     = FIELDMAP_GetFieldCamera( fieldmap );
  work->seq        = 0;
  work->rot_type   = CAMERA_ROT_TYPE_PITCH;
  work->now_frame  = 0;
  work->end_frame  = frame;
  work->end_angle  = angle;
  
  return task;
}


//==========================================================================================
// ■非公開関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief 回転角度を設定する
 *
 * @param work 設定対象のワーク
 */
//------------------------------------------------------------------------------------------
static void SetupStartAngle( ROT_WORK* work )
{
  FIELD_CAMERA* camera = work->camera;
  u32 end_angle = work->end_angle;
  u32 start_angle;

  // 初期角度を設定
  switch( work->rot_type ) {
  default: GF_ASSERT(0);
  case CAMERA_ROT_TYPE_YAW:   start_angle = FIELD_CAMERA_GetAngleYaw( camera ); break;
  case CAMERA_ROT_TYPE_PITCH: start_angle = FIELD_CAMERA_GetAnglePitch( camera ); break;
  }

  // 正の方向に回したとき, 180度以上の回転が必要になる場合, 負の方向に回転させる必要がある.
  // 回転の方向を逆にするために, 小さい方の角度を360度分の下駄を履かせる.
  if( start_angle < end_angle ) {
    if( PI < (end_angle - start_angle) ) {
      start_angle += 2*PI;
    }
  }
  else if( end_angle < start_angle ) {
    if( PI < (start_angle - end_angle) ) {
      end_angle += 2*PI;
    }
  } 

  // ワークの値を更新
  work->start_angle = start_angle;
  work->end_angle   = end_angle;
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラアングルを更新する
 *
 * @param 更新対象のワーク
 */
//------------------------------------------------------------------------------------------
static void UpdateAngle( ROT_WORK* work )
{
  FIELD_CAMERA* camera = work->camera;
  FIELD_CAMERA_MODE camera_mode;

  // 更新
  work->now_frame++;

  if( work->now_frame <= work->end_frame ) { 
    float t;
    u16 angle;

    // 角度を算出
    t     = work->now_frame / (float)work->end_frame;
    t     = t*t*( -2*t + 3 );
    angle = (u16)( ( (1-t) * work->start_angle ) + ( t * work->end_angle ) );

    camera_mode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

    // タイプに応じた更新を行う
    switch( work->rot_type ) {
    case CAMERA_ROT_TYPE_YAW:    FIELD_CAMERA_SetAngleYaw( camera, angle );    break;
    case CAMERA_ROT_TYPE_PITCH:  FIELD_CAMERA_SetAnglePitch( camera, angle );  break;
    }

    FIELD_CAMERA_ChangeMode( camera, camera_mode ); 
  }

#ifdef DEBUG_EVENT_ON
  OBATA_Printf( "TASK-CAM-ROT: now_frame = %d, pitch = 0x%x\n", work->now_frame, angle ); 
#endif
}

//==========================================================================================
// ■タスク処理関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief カメラアングルを更新する
 */
//------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL RotateCamera( void* wk )
{
  ROT_WORK* work = (ROT_WORK*)wk;

  switch( work->seq ) {
  case 0:
    SetupStartAngle( work );
    work->seq++;
    break;

  case 1:
    UpdateAngle( work );
    if( work->end_frame <= work->now_frame ) {
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}
