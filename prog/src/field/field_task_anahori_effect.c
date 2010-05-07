////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク ( あなほりエフェクト )
 * @file   field_task_anahori_effect.h
 * @author obata
 * @date   2010.05.03
 */
////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "field_task.h"
#include "field_player.h"
#include "fldeff_iwakudaki.h"
#include "field_task_anahori_effect.h"


//#define DEBUG_PRINT_ON


//========================================================================================== 
// ■タスクワーク
//========================================================================================== 
typedef struct {

  FIELDMAP_WORK* fieldmap;  // 動作対象のフィールドマップ
  FLDEFF_CTRL*   fectrl;    // フィールドエフェクト
  MMDL*          mmdl;      // 操作対象の動作モデル
  u8             seq;       // シーケンス
  u16            now_frame; // 動作フレーム数
  u16            end_frame; // 最終フレーム数
  u16            interval;  // エフェクト表示間隔

} TASK_WORK;


//========================================================================================== 
// ■prototype
//========================================================================================== 
static FIELD_TASK_RETVAL AnahoriEffectUpdate( void* wk );


//========================================================================================== 
// ■public function
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 穴掘りエフェクト表示タスクを生成する
 *
 * @param fieldmap タスク動作対象のフィールドマップ
 * @param mmdl     操作対象の動作モデル
 * @param frame    タスク動作フレーム数
 * @param interval エフェクト表示間隔
 *
 * @return 作成したフィールドタスク
 */
//------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_AnahoriEffect( FIELDMAP_WORK* fieldmap, MMDL* mmdl, int frame, int interval )
{
  HEAPID        heap_id = FIELDMAP_GetHeapID( fieldmap );
  FIELD_PLAYER* player  = FIELDMAP_GetFieldPlayer( fieldmap );

  FIELD_TASK* task;
  TASK_WORK* work;

  // タスクを生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), AnahoriEffectUpdate );

  // タスクワークを初期化
  work = FIELD_TASK_GetWork( task );
  work->fieldmap  = fieldmap;
  work->fectrl    = FIELDMAP_GetFldEffCtrl( fieldmap );
  work->mmdl      = FIELD_PLAYER_GetMMdl( player ); 
  work->seq       = 0;
  work->now_frame = 0;
  work->end_frame = frame;
  work->interval  = interval;

  return task;
}


//========================================================================================== 
// ■private function
//========================================================================================== 

//------------------------------------------------------------------------------------------
/**
 * @brief 穴掘りエフェクト タスク更新処理
 */
//------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL AnahoriEffectUpdate( void* wk )
{
  TASK_WORK* work = (TASK_WORK*)wk;

  switch( work->seq ) {
  case 0:
    // 岩砕きエフェクト表示
    if( work->now_frame % work->interval == 0 ) {
      FLDEFF_IWAKUDAKI_SetMMdl( work->mmdl, work->fectrl );
    }
    // 終了チェック
    if( work->end_frame <= work->now_frame++ ) { 
      return FIELD_TASK_RETVAL_FINISH;
    }
    break;
  }
  return FIELD_TASK_RETVAL_CONTINUE; 
} 

