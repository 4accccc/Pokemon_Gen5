////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク (カメラのターゲット座標)
 * @file   field_task_target_pos.c
 * @author obata
 * @date   2010.04.03
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "field_task.h"
#include "fieldmap.h"
#include "field/field_camera.h"
#include "field_task_target_pos.h"


//==========================================================================================
// ■タスクワーク
//==========================================================================================
typedef struct {

  u8             seq;       // シーケンス
  FIELDMAP_WORK* fieldmap;  // 処理対象のフィールドマップ
  u16            frame;     // 経過フレーム数
  u16            endFrame;  // 終了フレーム
  VecFx32        startPos;  // 初期座標
  VecFx32        endPos;    // 最終座標
  VecFx32        moveVec;   // 移動方向ベクトル
  FIELD_CAMERA_MODE cam_mode;

} TARGET_WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static FIELD_TASK_RETVAL TargetPosMove( void* wk );


//------------------------------------------------------------------------------------------
/**
 * @brief カメラを等速ズームさせるタスクを作成する
 *
 * @param fieldmap 処理対象のフィールドマップ
 * @param frame    ズーム完了までのフレーム数
 * @param dist     最終的な距離
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_CameraTargetMove( FIELDMAP_WORK* fieldmap, u16 frame, const VecFx32* startPos, const VecFx32* endPos )
{
  FIELD_TASK* task;
  TARGET_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TARGET_WORK), TargetPosMove );

  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq       = 0;
  work->fieldmap  = fieldmap;
  work->frame     = 0;
  work->endFrame  = frame;
  work->startPos = *startPos;
  work->endPos    = *endPos;
  VEC_Subtract( &work->endPos, &work->startPos, &work->moveVec );

  return task;
}

//==========================================================================================
// ■タスク処理関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief カメラの等速ズーム
 */
//------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL TargetPosMove( void* wk )
{
  TARGET_WORK*  work = (TARGET_WORK*)wk;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( work->seq ) {
  case 0:
    work->cam_mode = FIELD_CAMERA_GetMode( camera );
    FIELD_CAMERA_ChangeMode( camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    work->seq++;
    break;
  case 1:
    work->frame++;
    // 線形補完
    { 
      VecFx32 pos;
      float t = work->frame / (float)work->endFrame;
      VEC_MultAdd( FX_F32_TO_FX32(t), &work->moveVec, &work->startPos, &pos );
      FIELD_CAMERA_SetTargetPos( camera, &pos );
      OBATA_Printf( "TASK-TPOS: frame = %d, target = 0x%x, 0x%x, 0x%x\n", 
          work->frame, FX_Whole(pos.x), FX_Whole(pos.y), FX_Whole(pos.z) );
    }
    if( work->endFrame <= work->frame ) {
      FIELD_CAMERA_ChangeMode( camera, work->cam_mode );
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}
