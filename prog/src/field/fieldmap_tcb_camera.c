////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  フィールドマップTCB( カメラ動作 )
 * @file   fieldmap_tcb_camera.c
 * @author obata
 * @date   2009.08.26
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include "include/field/fieldmap_proc.h"
#include "include/gamesystem/game_event.h"
#include "fieldmap_tcb.h"
#include "fieldmap.h"
#include "field_player.h"
#include "field/field_camera.h"


////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ズーム
 */
////////////////////////////////////////////////////////////////////////////////////////////

//==========================================================================================
/**
 * @brief TCBワークエリア
 */
//==========================================================================================
typedef struct
{
  FIELDMAP_WORK* pFieldmap; // 処理対象のフィールドマップ
  u16            frame;     // 経過フレーム数
  u16            endFrame;  // 終了フレーム
  fx32           startDist; // カメラ距離初期値
  fx32           endDist;   // カメラ距離最終値
}
ZOOM_WORK;


//==========================================================================================
/**
 * @brief プロトタイプ宣言
 */
//========================================================================================== 
static void TCB_FUNC_Zoom( GFL_TCB* tcb, void* work ); 
static void UpdateDist( ZOOM_WORK* p_work );


//========================================================================================== 
/**
 * @brief 公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( ズーム )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param dist     移動距離
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_CameraZoom( FIELDMAP_WORK* fieldmap, int frame, fx32 dist )
{
  HEAPID      heap_id = FIELDMAP_GetHeapID( fieldmap );
  FIELD_CAMERA*   cam = FIELDMAP_GetFieldCamera( fieldmap );
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  ZOOM_WORK* work     = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( ZOOM_WORK ) ); 

  // TCBワーク初期化
  work->frame     = 0;
  work->endFrame  = frame;
  work->startDist = FIELD_CAMERA_GetAngleLen( cam );
  work->endDist   = FIELD_CAMERA_GetAngleLen( cam ) + dist;
  work->pFieldmap = fieldmap;

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_Zoom, work, 0 );
}


//========================================================================================== 
/**
 * @brief 非公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief TCB実行関数( ズーム )
 */
//------------------------------------------------------------------------------------------
static void TCB_FUNC_Zoom( GFL_TCB* tcb, void* work )
{
  ZOOM_WORK* tcbwork = work;

  // カメラ距離を更新
  UpdateDist( tcbwork );

  // 一定回数動作したら終了
  ++(tcbwork->frame);
  if( tcbwork->endFrame < tcbwork->frame )
  {
    GFL_HEAP_FreeMemory( work );  // ワークを破棄
    GFL_TCB_DeleteTask( tcb );    // タスクを破棄(自殺)
  }
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ距離を更新する
 *
 * @param p_work ズームTCBイベントワーク
 */
//-----------------------------------------------------------------------------------------------
static void UpdateDist( ZOOM_WORK* p_work )
{
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_work->pFieldmap );

  // 動作期間中なら, 更新する
  if( p_work->frame <= p_work->endFrame)
  {
    // 線形補完
    float    t = p_work->frame / (float)p_work->endFrame;
    float   d1 = FX_FX32_TO_F32( p_work->startDist );
    float   d2 = FX_FX32_TO_F32( p_work->endDist );
    float dist = (1-t) * d1 + ( t * d2);
    FIELD_CAMERA_SetAngleLen( p_camera, FX_F32_TO_FX32( dist ) ); 
  }
}


////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief カメラ回転
 */
////////////////////////////////////////////////////////////////////////////////////////////

//==========================================================================================
/**
 * @brief 定数
 */
//==========================================================================================
// π[rad]
#define PI (0x8000)   

// 回転タイプ
typedef enum 
{
  CAMERA_ROT_TYPE_YAW,    // ヨー回転
  CAMERA_ROT_TYPE_PITCH,  // ピッチ回転
}
CAMERA_ROT_TYPE;

//==========================================================================================
/**
 * @brief TCBワークエリア
 */
//==========================================================================================
typedef struct
{
  FIELDMAP_WORK*  pFieldmap;   // 処理対象のフィールドマップ
  CAMERA_ROT_TYPE type;        // 回転タイプ
  u16             frame;       // 経過フレーム数
  u16             endFrame;    // 終了フレーム
  u32             startAngle;  // 回転角初期値(2πラジアンを65536分割した値を単位とする数)
  u32             endAngle;    // 回転角最終値(2πラジアンを65536分割した値を単位とする数)
}
ROT_WORK;


//==========================================================================================
/**
 * @brief プロトタイプ宣言
 */
//========================================================================================== 
static void InitWork( 
    ROT_WORK* work, FIELDMAP_WORK* fieldmap, 
    CAMERA_ROT_TYPE type, u16 end_frame, u32 start_angle, u32 end_angle );
static void TCB_FUNC_RotCamera( GFL_TCB* tcb, void* work ); 
static void UpdateAngle( ROT_WORK* p_work );


//========================================================================================== 
/**
 * @brief 公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( カメラ ヨー回転 )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param angle    最終的な角度( 2πラジアンを65536分割した値を単位とする数 )
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_CameraRotate_Yaw( FIELDMAP_WORK* fieldmap, int frame, u16 angle )
{
  HEAPID      heap_id = FIELDMAP_GetHeapID( fieldmap );
  FIELD_CAMERA*  cam  = FIELDMAP_GetFieldCamera( fieldmap );
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  ROT_WORK*      work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( ROT_WORK ) ); 

  // TCBワーク初期化
  InitWork( work, fieldmap, CAMERA_ROT_TYPE_YAW, frame, FIELD_CAMERA_GetAngleYaw( cam ), angle );

  // TCBを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_RotCamera, work, 0 );
}

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( カメラ ピッチ回転 )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param angle    最終的な角度( 2πラジアンを65536分割した値を単位とする数 )
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_CameraRotate_Pitch( FIELDMAP_WORK* fieldmap, int frame, u16 angle )
{
  HEAPID      heap_id = FIELDMAP_GetHeapID( fieldmap );
  FIELD_CAMERA*  cam  = FIELDMAP_GetFieldCamera( fieldmap );
  GFL_TCBSYS*  tcbsys = FIELDMAP_GetFieldmapTCBSys( fieldmap );
  ROT_WORK*      work = GFL_HEAP_AllocMemoryLo( heap_id, sizeof( ROT_WORK ) ); 

  // TCBワーク初期化
  InitWork( work, fieldmap, CAMERA_ROT_TYPE_PITCH, frame, FIELD_CAMERA_GetAnglePitch( cam ), angle );

  // タスクを追加
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_RotCamera, work, 0 );
}


//========================================================================================== 
/**
 * @brief 非公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief ワークを初期化する
 * 
 * @param work         初期化するワーク
 * @param fieldmap     動作対象のフィールドマップ
 * @param type         回転タイプ
 * @param end_frame    最終フレーム数
 * @param start_angle  初期角度( 2πラジアンを65536分割した値を単位とする数 )
 * @param end_angle    最終角度( 2πラジアンを65536分割した値を単位とする数 )
 */
//------------------------------------------------------------------------------------------
static void InitWork( 
    ROT_WORK* work, FIELDMAP_WORK* fieldmap, 
    CAMERA_ROT_TYPE type, u16 end_frame, u32 start_angle, u32 end_angle )
{
  work->pFieldmap  = fieldmap;
  work->type       = type;
  work->frame      = 0;
  work->endFrame   = end_frame;
  work->startAngle = start_angle;
  work->endAngle   = end_angle;

  // 正の方向に回したとき, 180度以上の回転が必要になる場合, 負の方向に回転させる必要がある.
  // 回転の方向を逆にするために, 小さい方の角度を360度分の下駄を履かせる.
  if( start_angle < end_angle )
  {
    if( PI < (end_angle - start_angle) )
    {
      work->startAngle += 2*PI;
    }
  }
  else if( end_angle < start_angle )
  {
    if( PI < (start_angle - end_angle) )
    {
      work->endAngle += 2*PI;
    }
  } 
}

//------------------------------------------------------------------------------------------
/**
 * @brief TCB実行関数( カメラ回転 )
 */
//------------------------------------------------------------------------------------------
static void TCB_FUNC_RotCamera( GFL_TCB* tcb, void* work )
{
  ROT_WORK* tcbwork = work;

  // カメラ位置を更新
  UpdateAngle( tcbwork );

  // 一定回数動作したら終了
  ++(tcbwork->frame);
  if( tcbwork->endFrame < tcbwork->frame )
  {
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラアングルを更新する
 *
 * @param p_work イベントワーク
 */
//-----------------------------------------------------------------------------------------------
static void UpdateAngle( ROT_WORK* p_work )
{
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_work->pFieldmap );
  u16           angle;

  // 動作中なら, 更新する
  if(p_work->frame <= p_work->endFrame )
  {
    // 角度を算出
    //  計算式f(t)=-2t^3+3t^2 
    float t = p_work->frame / (float)p_work->endFrame;
    t       = t*t*( -2*t + 3 );
    angle     = (u16)( ( (1-t) * p_work->startAngle ) + ( t * p_work->endAngle ) );

    // タイプに応じた更新を行う
    switch( p_work->type )
    {
    case CAMERA_ROT_TYPE_YAW:     FIELD_CAMERA_SetAngleYaw( p_camera, angle );    break;
    case CAMERA_ROT_TYPE_PITCH:   FIELD_CAMERA_SetAnglePitch( p_camera, angle );  break;
    }
  }
}
