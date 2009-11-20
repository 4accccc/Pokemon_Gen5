////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク(自機の落下処理)
 * @file   field_task_player_fall.c
 * @author obata
 * @date   2009.11.18
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "field_task.h"
#include "field_task_player_fall.h"
#include "sound/pm_sndsys.h"
#include "field_player.h"
#include "fldeff_kemuri.h"


//========================================================================================== 
// ■タスクワーク
//========================================================================================== 
typedef struct
{
  u8             seq;       // シーケンス
  FIELDMAP_WORK* fieldmap;  // 動作対象のフィールドマップ
  u16            frame;     // 動作フレーム数
  u16            endFrame;  // 最大フレーム数
  float          moveDist;  // 移動距離

} TASK_WORK;


//========================================================================================== 
// ■非公開関数のプロトタイプ宣言
//========================================================================================== 
static void CalcDrawOffset( VecFx32* vec, float frame, float max_frame, float max_y );
static FIELD_TASK_RETVAL FallPlayer( void* wk );


//------------------------------------------------------------------------------------------
/**
 * @brief プレイヤーの落下処理タスクを作成する
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param frame    タスク動作フレーム数
 * @param dist     移動距離
 *
 * @return 作成したフィールドタスク
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_PlayerFall( FIELDMAP_WORK* fieldmap, int frame, int dist )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), FallPlayer );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->seq      = 0;
  work->fieldmap = fieldmap;
  work->frame    = 0;
  work->endFrame = frame;
  work->moveDist = dist;

  return task;
}


//========================================================================================== 
// ■非公開関数
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief y座標の描画オフセットを算出する( 落下 ) 
 *
 * @param vec       計算結果の格納先
 * @param frame     現在のフレーム数
 * @param max_frame 最大フレーム数
 * @param max_y     最大オフセット
 */
//------------------------------------------------------------------------------------------
static void CalcDrawOffset( VecFx32* vec, float frame, float max_frame, float max_y )
{
  // y = ax^2
  float x = frame;
  float a = max_y / (max_frame * max_frame);
  float y = max_y - (a * x * x);

  vec->x = 0;
  vec->y = FX_F32_TO_FX32( y );
  vec->z = 0;
} 


//========================================================================================== 
// ■タスク処理関数
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 自機の落下処理
 */
//------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL FallPlayer( void* wk )
{
  TASK_WORK*      work = (TASK_WORK*)wk;
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  MMDL*           mmdl = FIELD_PLAYER_GetMMdl( player ); 
  VecFx32       offset;

  switch( work->seq )
  {
  case 0:
    // 主人公の向きを設定
    MMDL_SetAcmd( mmdl, AC_DIR_D ); 
    // 描画オフセット初期化
    CalcDrawOffset( &offset, work->frame, work->endFrame, work->moveDist );
    MMDL_SetVectorDrawOffsetPos( mmdl, &offset );
    // SE再生(落下音)
    PMSND_PlaySE( SEQ_SE_FLD_16 );
    work->seq++;
    break;
  case 1:
    // 自機の描画オフセットを更新
    work->frame++;
    CalcDrawOffset( &offset, work->frame, work->endFrame, work->moveDist );
    MMDL_SetVectorDrawOffsetPos( mmdl, &offset );
    if( work->endFrame <= work->frame )  // 落下終了
    { 
      // 砂埃を出す
      FLDEFF_CTRL*  fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
      FLDEFF_KEMURI_SetMMdl( mmdl, fectrl );
      // SE再生(着地音)
      PMSND_PlaySE( SEQ_SE_FLD_17 );
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE; 
} 
