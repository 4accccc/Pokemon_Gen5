////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  フィールドマップTCB( カメラズーム動作 )
 * @file   fieldmap_tcb_camera_zoom.c
 * @author obata
 * @date   2009.08.26
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "include/field/fieldmap_proc.h"
#include "include/gamesystem/game_event.h"
#include "fieldmap_tcb.h"
#include "fieldmap.h"
#include "field_player.h"
#include "field/field_camera.h"


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
static void TCB_FUNC_LinearZoom( GFL_TCB* tcb, void* work ); 
static void TCB_FUNC_SharpZoom( GFL_TCB* tcb, void* work ); 

static void UpdateDist_Linear( ZOOM_WORK* p_work );
static void UpdateDist_Sharp( ZOOM_WORK* p_work );


//========================================================================================== 
/**
 * @brief 公開関数の定義
 */
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( 等速ズーム )
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
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_LinearZoom, work, 0 );
}

//------------------------------------------------------------------------------------------
/**
 * @brief タスクを追加する( 急発進ズーム )
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param dist     移動距離
 */
//------------------------------------------------------------------------------------------
void FIELDMAP_TCB_AddTask_CameraZoom_Sharp( FIELDMAP_WORK* fieldmap, int frame, fx32 dist )
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
  GFL_TCB_AddTask( tcbsys, TCB_FUNC_SharpZoom, work, 0 );
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
static void TCB_FUNC_LinearZoom( GFL_TCB* tcb, void* work )
{
  ZOOM_WORK* tcbwork = work;

  // カメラ距離を更新
  UpdateDist_Linear( tcbwork );

  // 一定回数動作したら終了
  ++(tcbwork->frame);
  if( tcbwork->endFrame < tcbwork->frame )
  {
    GFL_HEAP_FreeMemory( work );  // ワークを破棄
    GFL_TCB_DeleteTask( tcb );    // タスクを破棄(自殺)
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief TCB実行関数( 急発進ズーム )
 */
//------------------------------------------------------------------------------------------
static void TCB_FUNC_SharpZoom( GFL_TCB* tcb, void* work )
{
  ZOOM_WORK* tcbwork = work;

  // カメラ距離を更新
  UpdateDist_Sharp( tcbwork );

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
 * @brief カメラ距離を更新する( 線形補完 )
 *
 * @param p_work ズームTCBイベントワーク
 */
//-----------------------------------------------------------------------------------------------
static void UpdateDist_Linear( ZOOM_WORK* p_work )
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

//-----------------------------------------------------------------------------------------------
/**
 * @brief カメラ距離を更新する( 急発進 )
 *
 * @param p_work ズームTCBイベントワーク
 */
//-----------------------------------------------------------------------------------------------
static void UpdateDist_Sharp( ZOOM_WORK* p_work )
{
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_work->pFieldmap );

  // 動作期間中なら, 更新する
  if( p_work->frame <= p_work->endFrame)
  {
    float a, t, d1, d2, dist;

    // 2次関数 f(x) = √ax
    a = 1.0f;
    t = p_work->frame / (float)p_work->endFrame;
    t = sqrt( a * t );
    d1 = FX_FX32_TO_F32( p_work->startDist );
    d2 = FX_FX32_TO_F32( p_work->endDist );
    dist = (1-t) * d1 + ( t * d2);
    FIELD_CAMERA_SetAngleLen( p_camera, FX_F32_TO_FX32( dist ) ); 

    OBATA_Printf( "%d, \n", (int)t );
    OBATA_Printf( "%d, \n", (int)a );
    OBATA_Printf( "%d, \n", (int)d1 );
    OBATA_Printf( "%d, \n", (int)d2 );
    OBATA_Printf( "%d, \n", (int)dist );
  }
}
