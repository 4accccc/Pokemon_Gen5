////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク(カメラの注視点補正座標の移動処理)
 * @file   field_task_target_offset.h
 * @author obata
 * @date   2009.11.24
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "field_task.h"
#include "field_task_target_offset.h"

#include "field_camera.h"


//===============================================================================================
// ■タスクワーク
//===============================================================================================
typedef struct
{
  u8                  seq;  // シーケンス
  FIELDMAP_WORK* fieldmap;  // 動作対象フィールドマップ
  int               frame;  // 現在のフレーム数
  int            endFrame;  // 最終フレーム数
  VecFx32     startOffset;  // 移動開始位置
  VecFx32       endOffset;  // 最終位置

} TASK_WORK;


//===============================================================================================
// ■非公開関数のプロトタイプ宣言
//===============================================================================================
static FIELD_TASK_RETVAL UpdateOffset( void* wk );


//------------------------------------------------------------------------------------------
/**
 * @brief カメラの注視点補正座標移動タスクを作成する
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param offset   最終的なオフセット値
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_CameraTargetOffset( FIELDMAP_WORK* fieldmap, 
                                                  int frame, 
                                                  const VecFx32* offset )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), UpdateOffset );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq      = 0;
  work->fieldmap = fieldmap;
  work->frame    = 0;
  work->endFrame = frame;
  VEC_Set( &work->endOffset, offset->x, offset->y, offset->z );

  return task;
}


//===============================================================================================
// ■タスク処理関数
//===============================================================================================

//-----------------------------------------------------------------------------------------------
/**
 * @brief 自機の平行移動処理
 */
//-----------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL UpdateOffset( void* wk )
{
  TASK_WORK*      work = (TASK_WORK*)wk;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( work->seq )
  {
  case 0:
    // 初期オフセットを取得
    FIELD_CAMERA_GetTargetOffset( camera, &work->startOffset );
    work->seq++;
    break;
  case 1:
    // オフセットを更新
    work->frame++;
    {
      VecFx32 v0, v1, v2;
      fx32 w1 = FX_F32_TO_FX32( (float)work->frame / (float)work->endFrame );
      fx32 w0 = FX32_ONE - w1;
      GFL_CALC3D_VEC_MulScalar( &work->startOffset, w0, &v0 );
      GFL_CALC3D_VEC_MulScalar( &work->endOffset,   w1, &v1 );
      VEC_Add( &v0, &v1, &v2 );
      FIELD_CAMERA_SetTargetOffset( camera, &v2 );
    }
    // 終了チェック
    if( work->endFrame <= work->frame )
    {
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}

