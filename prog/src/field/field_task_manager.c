//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  フィールドタスク管理
 * @file   field_task_manager.c
 * @author obata
 * @date   2009.11.13
 */
//////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "field_task.h"
#include "field_task_manager.h"


//========================================================================================
// ■タスクマネージャ
//========================================================================================
struct _FIELD_TASK_MAN
{
  u8     maxTaskNum;  // タスクの最大管理数
  FIELD_TASK** task;  // タスク配列
  FIELD_TASK** trig;  // トリガタスク配列
};


//========================================================================================
// ■非公開関数のプロトタイプ宣言
//========================================================================================
static void RegistTask( FIELD_TASK_MAN* man, u8 idx, FIELD_TASK* task, FIELD_TASK* trig );
static void DeleteTask( FIELD_TASK_MAN* man, u8 idx );
static void RunTask( FIELD_TASK_MAN* man, u8 idx );
static void TrigBoot( FIELD_TASK_MAN* man, u8 idx );
static void RemoveTask( FIELD_TASK_MAN* man, u8 idx );


//========================================================================================
// ■作成・破棄
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief タスクマネージャを作成する
 *
 * @param max_task_num タスクの最大登録数
 * @param heap_id      使用するヒープID
 *
 * @return 作成したマネージャ
 */
//----------------------------------------------------------------------------------------
FIELD_TASK_MAN* FIELD_TASK_MAN_Create( u8 max_task_num, HEAPID heap_id )
{
  FIELD_TASK_MAN* man;
  man = GFL_HEAP_AllocMemory( heap_id, sizeof(FIELD_TASK_MAN) );
  man->maxTaskNum = max_task_num;
  man->task = GFL_HEAP_AllocClearMemory( heap_id, sizeof(FIELD_TASK*) * max_task_num );
  man->trig = GFL_HEAP_AllocClearMemory( heap_id, sizeof(FIELD_TASK*) * max_task_num );
  return man;
}

//----------------------------------------------------------------------------------------
/**
 * @brief タスクマネージャを破棄する
 *
 * @param man 破棄するマネージャ
 */
//----------------------------------------------------------------------------------------
void FIELD_TASK_MAN_Delete( FIELD_TASK_MAN* man )
{
  int i;

  // タスクを破棄
  for( i=0; i<man->maxTaskNum; i++ )
  {
    DeleteTask( man, i );
  }
  // タスク配列を破棄
  GFL_HEAP_FreeMemory( man->task );
  GFL_HEAP_FreeMemory( man->trig );
  // 本体を破棄
  GFL_HEAP_FreeMemory( man );
}


//========================================================================================
// ■動作
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief タスクマネージャを動かす
 *
 * @param man 動かすマネージャ
 */
//----------------------------------------------------------------------------------------
void FIELD_TASK_MAN_Main( FIELD_TASK_MAN* man )
{
  int i;

  // 全タスクを動かす
  for( i=0; i<man->maxTaskNum; i++ )
  {
    RunTask( man, i );
  }

  // トリガチェック
  for( i=0; i<man->maxTaskNum; i++ )
  {
    TrigBoot( man, i );
  } 

  // 終了したタスクを破棄
  for( i=0; i<man->maxTaskNum; i++ )
  {
    RemoveTask( man, i );
  }
}


//========================================================================================
// ■制御
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief タスクを登録する
 *
 * @param man  登録先マネージャ
 * @param task 登録するマネージャ
 * @param trig タスク実行のトリガとなるタスク(不要ならNULLを指定)
 */
//----------------------------------------------------------------------------------------
void FIELD_TASK_MAN_AddTask( FIELD_TASK_MAN* man, FIELD_TASK* task, FIELD_TASK* trig )
{
  int idx;

  // 空き要素を検索
  for( idx=0; idx<man->maxTaskNum; idx++ )
  {
    // 発見 ==> 登録 ==> 起動
    if( man->task[idx] == NULL )
    { 
      RegistTask( man, idx, task, trig );
      if( trig == NULL ) FIELD_TASK_Boot( task );  // トリガ設定無し==> 即起動
      return;
    }
  }

  // 空いていない
  OBATA_Printf( "==============================\n" );
  OBATA_Printf( "FIELD_TASK_MAN: task over flow\n" );
  OBATA_Printf( "==============================\n" );
}


//========================================================================================
// ■取得
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief 保持タスクの数を取得する
 *
 * @param man タスクの数を調べるマネージャ
 *
 * @return 動作中タスクの数
 */
//----------------------------------------------------------------------------------------
u8 FIELD_TASK_MAN_GetTaskNum( const FIELD_TASK_MAN* man )
{
  int i;
  u8 num = 0;

  // 所持しているタスクの数をカウント
  for( i=0; i<man->maxTaskNum; i++ )
  {
    if( man->task[i] != NULL )
    {
      num++;
    }
  }
  return num;
}

//----------------------------------------------------------------------------------------
/**
 * @brief すべてのタスクが終了したかどうかを取得する
 *
 * @param man マネージャ
 *
 * @return 全タスクが終了していたら TRUE, そうでなければ FALSE
 */
//----------------------------------------------------------------------------------------
extern BOOL FIELD_TASK_MAN_IsAllTaskEnd( const FIELD_TASK_MAN* man )
{
  return (FIELD_TASK_MAN_GetTaskNum(man) == 0);
}


//========================================================================================
// ■非公開関数
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief タスクを登録する
 *
 * @param man 登録先マネージャ
 * @param idx 登録先インデックス
 * @param task 登録するタスク
 * @param trig 登録するタスクのトリガとなるタスク
 */
//----------------------------------------------------------------------------------------
static void RegistTask( FIELD_TASK_MAN* man, u8 idx, FIELD_TASK* task, FIELD_TASK* trig )
{
  // 指定インデックスは空いていない
  if( man->task[idx] != NULL ) return;

  // 登録
  man->task[idx] = task;
  man->trig[idx] = trig;
}

//----------------------------------------------------------------------------------------
/**
 * @brief タスクを破棄する
 *
 * @param man 破棄対象タスクを保持するマネージャ
 * @param idx 破棄するタスクを指定
 */
//----------------------------------------------------------------------------------------
static void DeleteTask( FIELD_TASK_MAN* man, u8 idx )
{
  // 指定タスクを持っていない
  if( man->task[idx] == NULL ) return;

  // 破棄
  FIELD_TASK_Delete( man->task[idx] );
  man->task[idx] = NULL;
  man->trig[idx] = NULL;
}

//----------------------------------------------------------------------------------------
/**
 * @brief タスクを動かす
 *
 * @param man 動かすタスクを保持するマネージャ
 * @param idx 動かすタスクを指定
 */
//----------------------------------------------------------------------------------------
static void RunTask( FIELD_TASK_MAN* man, u8 idx )
{
  // 指定タスクを持っていない
  if( man->task[idx] == NULL ) return;

  // 実行
  FIELD_TASK_Main( man->task[idx] );
}

//----------------------------------------------------------------------------------------
/**
 * @brief 指定タスクのトリガを調べて起動する
 *
 * @param man チェックするタスクを保持するマネージャ
 * @param idx チェック対象タスクを指定
 */
//----------------------------------------------------------------------------------------
static void TrigBoot( FIELD_TASK_MAN* man, u8 idx )
{
  // 指定タスクを持っていない
  if( man->task[idx] == NULL ) return; 
  // トリガを持っていない
  if( man->trig[idx] == NULL ) return;
  // 起動待ちでない
  {
    TASK_STATE state = FIELD_TASK_GetState( man->task[idx] );
    if( state != TASK_STATE_WAIT ) return;
  } 
  // トリガが引かれていない
  {
    TASK_STATE state = FIELD_TASK_GetState( man->trig[idx] );
    if( state != TASK_STATE_END ) return;
  }

  // 起動
  FIELD_TASK_Boot( man->task[idx] );
}  

//----------------------------------------------------------------------------------------
/**
 * @brief 指定タスクが終了していたら削除する
 *
 * @param man 削除するタスクを保持しているマネージャ
 * @param idx 削除するタスクを指定
 */
//----------------------------------------------------------------------------------------
static void RemoveTask( FIELD_TASK_MAN* man, u8 idx )
{
  // 指定タスクを持っていない
  if( man->task[idx] == NULL ) return;

  // 終了していたら破棄
  if( FIELD_TASK_IsEnd( man->task[idx] ) == TRUE )
  {
    DeleteTask( man, idx );
  }
}
