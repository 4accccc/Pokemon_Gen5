/////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク(自機の平行移動処理)
 * @file   field_task_player_trans.c
 * @author obata
 * @date   2009.11.18
 */
/////////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "field_task.h"
#include "field_task_player_trans.h"


//===============================================================================================
// ■タスクワーク
//===============================================================================================
typedef struct
{
  u8                  seq;  // シーケンス
  FIELDMAP_WORK* fieldmap;  // 動作対象フィールドマップ
  int               frame;  // 現在のフレーム数
  int            endFrame;  // 最終フレーム数
  VecFx32        startPos;  // 移動開始位置
  VecFx32          endPos;  // 最終位置

} TASK_WORK;


//===============================================================================================
// ■非公開関数のプロトタイプ宣言
//===============================================================================================
static FIELD_TASK_RETVAL TransPlayer( void* wk );


//-----------------------------------------------------------------------------------------------
/**
 * @brief プレイヤーの移動タスクを作成する
 *
 * @param fieldmap 動作対象フィールドマップ
 * @param frame    動作フレーム数
 * @param pos      移動先座標
 */
//-----------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_TransPlayer( FIELDMAP_WORK* fieldmap, int frame, VecFx32* pos )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), TransPlayer );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq      = 0;
  work->fieldmap = fieldmap;
  work->frame    = 0;
  work->endFrame = frame;
  VEC_Set( &work->endPos, pos->x, pos->y, pos->z );

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
static FIELD_TASK_RETVAL TransPlayer( void* wk )
{
  TASK_WORK*      work = (TASK_WORK*)wk;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player );

  switch( work->seq )
  {
  case 0:
    FIELD_PLAYER_GetPos( player, &work->startPos );
    work->seq++;
    break;
  case 1:
    // 自機の座標を更新
    work->frame++;
    {
      VecFx32 v0, v1, v2;
      fx32 w1 = FX_F32_TO_FX32( (float)work->frame / (float)work->endFrame );
      fx32 w0 = FX32_ONE - w1;
      GFL_CALC3D_VEC_MulScalar( &work->startPos, w0, &v0 );
      GFL_CALC3D_VEC_MulScalar( &work->endPos,   w1, &v1 );
      VEC_Add( &v0, &v1, &v2 );
      FIELD_PLAYER_SetPos( player, &v2 );   // 移動
      MMDL_UpdateCurrentHeight( mmdl );   // 接地
    }
    if( work->endFrame <= work->frame )
    {
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE;
}
