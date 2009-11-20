/////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク( 画面フェード )
 * @file   field_task_fade.c
 * @author obata
 * @date   2009.11.20
 */
/////////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "field_task.h"
#include "field_task_fade.h"


//===============================================================================================
// ■タスクワーク
//===============================================================================================
typedef struct
{
  // 動作対象フィールドマップ
  FIELDMAP_WORK* fieldmap;

  int mode;      // フェードモード（GFL_FADE_MASTER_BRIGHT_BLACKOUT/_WHITEOUT)
  int startEvy;  // スタート輝度（0～16）
  int endEvy;    // エンド輝度（0～16）
  int wait;      // フェードスピード

} TASK_WORK;


//===============================================================================================
// ■非公開関数のプロトタイプ宣言
//===============================================================================================
static FIELD_TASK_RETVAL FadeRequest( void* wk );


//-----------------------------------------------------------------------------------------------
/**
 * @brief フィールドタスクを作成する( 画面フェードリクエスト処理 )
 *
 * @param fieldmap  動作対象フィールドマップ
 * @param mode	    フェードモード（GFL_FADE_MASTER_BRIGHT_BLACKOUT/_WHITEOUT)
 * @param	start_evy	スタート輝度（0～16）
 * @param	end_evy		エンド輝度（0～16）
 * @param	wait		  フェードスピード
 *
 * @return 作成したタスク
 */
//-----------------------------------------------------------------------------------------------
FIELD_TASK* FIELD_TASK_Fade( FIELDMAP_WORK* fieldmap, 
                             int mode, int start_evy, int end_evy, int wait )
{
  FIELD_TASK* task;
  TASK_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(TASK_WORK), FadeRequest );
  // 初期化
  work = FIELD_TASK_GetWork( task );
  work->fieldmap = fieldmap;
  work->mode     = mode;
  work->startEvy = start_evy;
  work->endEvy   = end_evy;
  work->wait     = wait; 
  return task;
}


//===============================================================================================
// ■タスク処理関数
//===============================================================================================

//-----------------------------------------------------------------------------------------------
/**
 * @brief 画面フェードリクエスト
 */
//-----------------------------------------------------------------------------------------------
static FIELD_TASK_RETVAL FadeRequest( void* wk )
{ 
  TASK_WORK* work = (TASK_WORK*)wk;

  // リクエスト
  GFL_FADE_SetMasterBrightReq( work->mode, work->startEvy, work->endEvy, work->wait );
  return FIELD_TASK_RETVAL_FINISH;
}
