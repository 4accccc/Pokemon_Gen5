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

//#define DEBUG_EVENT_ON

typedef enum {
  TRANS_TYPE_PLUS = 0,
  TRANS_TYPE_MINUS,
}TRANS_TYPE;

//========================================================================================== 
// ■タスクワーク
//========================================================================================== 
typedef struct
{
  u8             seq;       // シーケンス
  FIELDMAP_WORK* fieldmap;  // 動作対象のフィールドマップ
  TRANS_TYPE    transType;
  u16            nowFrame;  // 動作フレーム数
  u16            endFrame;  // 最大フレーム数
  VecFx32        vecMove;   // 移動ベクトル 
  VecFx32        basePos;
  MMDL*          mmdl;      // 操作対象の動作モデル

} TASK_WORK;

//========================================================================================== 
// ■非公開関数のプロトタイプ宣言
//========================================================================================== 
static void CalcDrawOffset( VecFx32* now, u16 nowFrame, VecFx32* max, u16 maxFrame );
static FIELD_TASK_RETVAL UpdateDrawOffset( void* wk );
static FIELD_TASK_RETVAL UpdatePosOffset( void* wk );


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
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldmap );
  return FIELD_TASK_TransDrawOffsetEX( fieldmap, frame, vec, FIELD_PLAYER_GetMMdl( player ) );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 指定した動作モデルのオフセット移動タスクを作成する
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param move     移動ベクトル
 * @param mmdl     操作対象の動作モデル
 *
 * @return 作成したフィールドタスク
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_TransDrawOffsetEX( FIELDMAP_WORK* fieldmap, int frame, const VecFx32* vec, MMDL* mmdl )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // タスクを生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), UpdateDrawOffset );

  // タスクワークを初期化
  work = FIELD_TASK_GetWork( task );
  work->seq      = 0;
  work->fieldmap = fieldmap;
  work->mmdl     = mmdl;
  MMDL_GetVectorPos( mmdl, &work->basePos );

  if (frame >= 0 ) {
    work->transType = TRANS_TYPE_PLUS;
    work->nowFrame = 0;
    work->endFrame = frame;
  } else {
    work->transType = TRANS_TYPE_MINUS;
    work->nowFrame = 0;
    work->endFrame = -frame;
  }
  VEC_Set( &work->vecMove, vec->x, vec->y, vec->z );

  return task;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 指定した動作モデルの移動タスクを作成する
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param move     移動ベクトル
 * @param mmdl     操作対象の動作モデル
 *
 * @return 作成したフィールドタスク
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_TransPos( FIELDMAP_WORK* fieldmap, int frame, const VecFx32* vec, MMDL* mmdl )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // タスクを生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), UpdatePosOffset );

  // タスクワークを初期化
  work = FIELD_TASK_GetWork( task );
  work->seq      = 0;
  work->fieldmap = fieldmap;
  work->mmdl     = mmdl;
  MMDL_GetVectorPos( mmdl, &work->basePos );

  if (frame >= 0 ) {
    work->transType = TRANS_TYPE_PLUS;
    work->nowFrame = 0;
    work->endFrame = frame;
  } else {
    work->transType = TRANS_TYPE_MINUS;
    work->nowFrame = 0;
    work->endFrame = -frame;
  }
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
#ifdef DEBUG_EVENT_ON
  OS_Printf("NOW/MAX=%03d/%03d\n", nowFrame, maxFrame );
#endif
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
  TASK_WORK* work = (TASK_WORK*)wk;
  MMDL*      mmdl = work->mmdl;
  VecFx32    offset;

  switch( work->seq )
  {
  case 0:
    // 自機の描画オフセットを更新
    work->nowFrame++;
    if (work->transType == TRANS_TYPE_PLUS)
    {
      CalcDrawOffset( &offset, work->nowFrame, &work->vecMove, work->endFrame );
    } else if (work->transType == TRANS_TYPE_MINUS ) {
      u16 now = work->endFrame - work->nowFrame;
      CalcDrawOffset( &offset, now, &work->vecMove, work->endFrame );
    }
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
//------------------------------------------------------------------------------------------
/**
 * @brief 描画オフセット更新処理
 */
//------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL UpdatePosOffset( void* wk )
{
  TASK_WORK* work = (TASK_WORK*)wk;
  MMDL*      mmdl = work->mmdl;
  VecFx32    offset;

  switch( work->seq )
  {
  case 0:
    // 自機の描画オフセットを更新
    work->nowFrame++;
    if (work->transType == TRANS_TYPE_PLUS)
    {
      CalcDrawOffset( &offset, work->nowFrame, &work->vecMove, work->endFrame );
    } else if (work->transType == TRANS_TYPE_MINUS ) {
      u16 now = work->endFrame - work->nowFrame;
      CalcDrawOffset( &offset, now, &work->vecMove, work->endFrame );
    }
    VEC_Add( &offset, &work->basePos, &offset );
    MMDL_SetVectorPos( mmdl, &offset );
    // 終了チェック
    if( work->endFrame <= work->nowFrame )
    { 
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE; 
} 
