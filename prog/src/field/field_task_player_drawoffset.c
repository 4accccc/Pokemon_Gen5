////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク(自機の描画オフセット移動)
 * @file   field_task_player_drawoffset.h
 * @author obata
 * @date   2009.11.18
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "field_task.h"
#include "field_task_player_drawoffset.h"
#include "field_player.h"


//========================================================================================== 
// ■タスクワーク
//========================================================================================== 
typedef struct
{
  u8             seq;       // シーケンス
  FIELDMAP_WORK* fieldmap;  // 動作対象のフィールドマップ
  u16            nowFrame;  // 動作フレーム数
  u16            endFrame;  // 最大フレーム数
  VecFx32        vecMove;   // 移動ベクトル 

} TASK_WORK;


//========================================================================================== 
// ■非公開関数のプロトタイプ宣言
//========================================================================================== 
static void CalcDrawOffset( VecFx32* now, u16 nowFrame, VecFx32* max, u16 maxFrame );
static FIELD_TASK_RETVAL UpdateDrawOffset( void* wk );


//------------------------------------------------------------------------------------------
/**
 * @brief プレイヤーの描画オフセット移動タスクを作成する
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param move     移動ベクトル
 *
 * @return 作成したフィールドタスク
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_TransDrawOffset( FIELDMAP_WORK* fieldmap, int frame, const VecFx32* vec )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), UpdateDrawOffset );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq      = 0;
  work->fieldmap = fieldmap;
  work->nowFrame = 0;
  work->endFrame = frame;
  VEC_Set( &work->vecMove, vec->x, vec->y, vec->z );

  return task;
}


//========================================================================================== 
// ■非公開関数
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける描画オフセット値を計算する
 *
 * @param now       計算結果の格納先
 * @param nowFrame  現在のフレーム数
 * @param max       最大移動ベクトル
 * @param maxFrame  最大フレーム数
 */
//------------------------------------------------------------------------------------------
static void CalcDrawOffset( VecFx32* now, u16 nowFrame, VecFx32* max, u16 maxFrame )
{
  // 線形補間
  fx32 rate = FX_F32_TO_FX32((float)nowFrame / (float)maxFrame);
  now->x = FX_Mul( max->x, rate );
  now->y = FX_Mul( max->y, rate );
  now->z = FX_Mul( max->z, rate );
} 


//========================================================================================== 
// ■タスク処理関数
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 描画オフセット更新処理
 */
//------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL UpdateDrawOffset( void* wk )
{
  TASK_WORK*      work = (TASK_WORK*)wk;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  VecFx32       offset;

  switch( work->seq )
  {
  case 0:
    // 自機の描画オフセットを更新
    work->nowFrame++;
    CalcDrawOffset( &offset, work->nowFrame, &work->vecMove, work->endFrame );
    MMDL_SetVectorDrawOffsetPos( mmdl, &offset );
    // 終了チェック
    if( work->endFrame <= work->nowFrame )
    { 
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE; 
} 
