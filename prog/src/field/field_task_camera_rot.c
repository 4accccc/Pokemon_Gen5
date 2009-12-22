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
#define PI (0x8000)   // π[rad]

// 回転タイプ
typedef enum {
  CAMERA_ROT_TYPE_YAW,    // ヨー回転
  CAMERA_ROT_TYPE_PITCH,  // ピッチ回転
} CAMERA_ROT_TYPE;


//==========================================================================================
// ■タスクワーク
//==========================================================================================
typedef struct
{
  u8              seq;         // シーケンス
  FIELDMAP_WORK*  fieldmap;    // 処理対象のフィールドマップ
  CAMERA_ROT_TYPE type;        // 回転タイプ
  u16             frame;       // 経過フレーム数
  u16             endFrame;    // 終了フレーム
  u32             startAngle;  // 回転角初期値(2πラジアンを65536分割した値を単位とする数)
  u32             endAngle;    // 回転角最終値(2πラジアンを65536分割した値を単位とする数)

} ROT_WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void SetupAngle( ROT_WORK* work );
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
  FIELD_CAMERA* cam = FIELDMAP_GetFieldCamera( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(ROT_WORK), RotateCamera );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq        = 0;
  work->fieldmap   = fieldmap;
  work->type       = CAMERA_ROT_TYPE_YAW;
  work->frame      = 0;
  work->endFrame   = frame;
  work->endAngle   = angle;
  
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
  FIELD_CAMERA* cam = FIELDMAP_GetFieldCamera( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(ROT_WORK), RotateCamera );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq        = 0;
  work->fieldmap   = fieldmap;
  work->type       = CAMERA_ROT_TYPE_PITCH;
  work->frame      = 0;
  work->endFrame   = frame;
  work->endAngle   = angle;
  
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
static void SetupAngle( ROT_WORK* work )
{
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  // 初期角度・最終角度を設定
  switch( work->type )
  {
  case CAMERA_ROT_TYPE_YAW:
    work->startAngle = FIELD_CAMERA_GetAngleYaw( camera );
    break;
  case CAMERA_ROT_TYPE_PITCH:
    work->startAngle = FIELD_CAMERA_GetAnglePitch( camera );
    break;
  }

  // 正の方向に回したとき, 180度以上の回転が必要になる場合, 負の方向に回転させる必要がある.
  // 回転の方向を逆にするために, 小さい方の角度を360度分の下駄を履かせる.
  if( work->startAngle < work->endAngle )
  {
    if( PI < (work->endAngle - work->startAngle) )
    {
      work->startAngle += 2*PI;
    }
  }
  else if( work->endAngle < work->startAngle )
  {
    if( PI < (work->startAngle - work->endAngle) )
    {
      work->endAngle += 2*PI;
    }
  } 
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
  float t;
  u16 angle;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );
  FIELD_CAMERA_MODE cam_mode;

  // 更新
  work->frame++;
  if( work->frame <= work->endFrame )
  { 
    // 角度を算出
    //  計算式f(t)=-2t^3+3t^2 
    t     = work->frame / (float)work->endFrame;
    t     = t*t*( -2*t + 3 );
    angle = (u16)( ( (1-t) * work->startAngle ) + ( t * work->endAngle ) );

    // タイプに応じた更新を行う
    cam_mode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    switch( work->type )
    {
    case CAMERA_ROT_TYPE_YAW:    FIELD_CAMERA_SetAngleYaw( camera, angle );    
                                 OBATA_Printf( "TASK-CAM-ROT: yaw = %x\n", angle );
                                 break;
    case CAMERA_ROT_TYPE_PITCH:  FIELD_CAMERA_SetAnglePitch( camera, angle );  
                                 OBATA_Printf( "TASK-CAM-ROT: pitch = %x\n", angle );
                                 break;
    }
    FIELD_CAMERA_ChangeMode( camera, cam_mode );
  }
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

  switch( work->seq )
  {
  case 0:
    SetupAngle( work );
    work->seq++;
    break;
  case 1:
    UpdateAngle( work );
    if( work->endFrame <= work->frame )
    {
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}
