//////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  キュー
 * @file   queue.c
 * @author obata
 * @date   2010.02.20
 */
//////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "queue.h"


//====================================================================================
// ■キュー
//====================================================================================
struct _QUEUE
{
  int* data;  // データ配列
  int  size;  // 配列サイズ
  int  head;  // 先頭要素のインデックス
  int  tail;  // 末尾要素のインデックス( 追加位置 ) 
};


//====================================================================================
// ■外部非公開関数
//====================================================================================
static void InitQueue( QUEUE* queue ); // キューを初期化する 
static void CreateDataArea( QUEUE* queue, int size, HEAPID heapID ); // データ領域を作成する
static void DeleteDataArea( QUEUE* queue ); // データ領域を破棄する
static void EnQueue( QUEUE* queue, int val ); // エンキューする
static int DeQueue( QUEUE* queue ); // デキューする
static void ClearQueue( QUEUE* queue ); // キューを空にする
static BOOL QueueIsEmpty( const QUEUE* queue ); // キューが空かどうかを判定する
static BOOL QueueIsFull( const QUEUE* queue ); // キューが満タンかどうかを判定する
static int GetDataNum( const QUEUE* queue ); // 保持しているデータの個数を取得する
static int PeekData( const QUEUE* queue, int no ); // データを覗き見る

//====================================================================================
// □外部公開関数
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief キューを作成する
 *
 * @param queueSize キューサイズ
 * @param heapID    使用するヒープID
 *
 * @return 生成したキュー
 */
//------------------------------------------------------------------------------------
QUEUE* QUEUE_Create( int queueSize, HEAPID heapID )
{
  QUEUE* queue;

  // 生成
  queue = GFL_HEAP_AllocMemory( heapID, sizeof(QUEUE) );

  // 初期化
  InitQueue( queue );
  CreateDataArea( queue, queueSize, heapID );

  return queue;
}

//------------------------------------------------------------------------------------
/**
 * @brief キューを破棄する
 *
 * @param queue
 */
//------------------------------------------------------------------------------------
void QUEUE_Delete( QUEUE* queue )
{
  DeleteDataArea( queue );
  GFL_HEAP_FreeMemory( queue );
}

//------------------------------------------------------------------------------------
/**
 * @brief エンキューする
 *
 * @param queue
 * @param val   追加する値
 */
//------------------------------------------------------------------------------------
void QUEUE_EnQueue( QUEUE* queue, int val )
{
  EnQueue( queue, val );
}

//------------------------------------------------------------------------------------
/**
 * @brief デキューする
 *
 * @param queue
 *
 * @return キューから取り出した値
 */
//------------------------------------------------------------------------------------
int QUEUE_DeQueue( QUEUE* queue )
{
  return DeQueue( queue );
}

//------------------------------------------------------------------------------------
/**
 * @brief キューを空にする
 *
 * @param queue
 */
//------------------------------------------------------------------------------------
void QUEUE_Clear( QUEUE* queue )
{
  ClearQueue( queue );
}

//------------------------------------------------------------------------------------
/**
 * @brief キューが空かどうかを判定する
 *
 * @param queue
 *
 * @return キューが空なら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------------------
BOOL QUEUE_IsEmpty( const QUEUE* queue )
{
  return QueueIsEmpty( queue );
}

//------------------------------------------------------------------------------------
/**
 * @brief キューが満タンかどうかを判定する
 *
 * @param queue
 *
 * @return キューが満タンなら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------------------
BOOL QUEUE_IsFull( const QUEUE* queue )
{
  return QueueIsFull( queue );
}

//------------------------------------------------------------------------------------
/**
 * @brief 保持しているデータの個数を取得する
 *
 * @param queue
 */
//------------------------------------------------------------------------------------
int QUEUE_GetDataNum( const QUEUE* queue )
{
  return GetDataNum( queue );
}

//------------------------------------------------------------------------------------
/**
 * @brief データを覗き見る
 *
 * @param queue
 * @param no    何番目のデータを見るか
 *
 * @return 指定した位置にあるデータ
 */
//------------------------------------------------------------------------------------
int QUEUE_PeekData( const QUEUE* queue, int no )
{
  return PeekData( queue, no );
}


//====================================================================================
// □外部非公開関数
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief キューを初期化する 
 *
 * @param queue
 */
//------------------------------------------------------------------------------------
static void InitQueue( QUEUE* queue )
{
  queue->data = NULL;
  queue->head = 0;
  queue->tail = 0;
}

//------------------------------------------------------------------------------------
/**
 * @brief データ領域を作成する
 *
 * @param queue
 * @param size   キューサイズ
 * @param heapID 使用するヒープID
 */
//------------------------------------------------------------------------------------
static void CreateDataArea( QUEUE* queue, int size, HEAPID heapID )
{
  // 多重生成
  GF_ASSERT( queue->data == NULL );

  queue->data = GFL_HEAP_AllocMemory( heapID, sizeof(int) * size );
  queue->size = size;
}

//------------------------------------------------------------------------------------
/**
 * @brief データ領域を破棄する
 *
 * @param queue
 */
//------------------------------------------------------------------------------------
static void DeleteDataArea( QUEUE* queue )
{
  // 生成されていない
  GF_ASSERT( queue->data != NULL );

  GFL_HEAP_FreeMemory( queue->data );
}

//------------------------------------------------------------------------------------
/**
 * @brief エンキューする
 *
 * @param queue
 * @param val   追加する値
 */
//------------------------------------------------------------------------------------
static void EnQueue( QUEUE* queue, int val )
{
  // オーバーフロー
  GF_ASSERT( QueueIsFull(queue) == FALSE );

  // 値を追加
  queue->data[ queue->tail ] = val;

  // 末尾位置を更新
  queue->tail = ( queue->tail + 1 ) % queue->size;
}

//------------------------------------------------------------------------------------
/**
 * @brief デキューする
 *
 * @param queue
 *
 * @return キューから取り出した値
 */
//------------------------------------------------------------------------------------
static int DeQueue( QUEUE* queue )
{
  int val;

  // キューが空
  GF_ASSERT( QueueIsEmpty(queue) == FALSE );

  // 先頭要素を取得
  val = queue->data[ queue->head ];

  // 先頭位置を更新
  queue->head = ( queue->head + 1 ) % queue->size;

  return val;
}

//------------------------------------------------------------------------------------
/**
 * @brief キューを空にする
 *
 * @param queue
 */
//------------------------------------------------------------------------------------
static void ClearQueue( QUEUE* queue )
{
  queue->head = 0;
  queue->tail = 0;
}

//------------------------------------------------------------------------------------
/**
 * @brief キューが空かどうかを判定する
 *
 * @param queue
 *
 * @return キューが空なら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------------------
static BOOL QueueIsEmpty( const QUEUE* queue )
{
  // キューが生成されていない
  GF_ASSERT( queue->data );

  return ( queue->head == queue->tail );
}

//------------------------------------------------------------------------------------
/**
 * @brief キューが満タンかどうかを判定する
 *
 * @param queue
 *
 * @return キューが満タンなら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------------------
static BOOL QueueIsFull( const QUEUE* queue )
{
  int next;

  // キューが生成されていない
  GF_ASSERT( queue->data );

  next = ( queue->tail + 1 ) % queue->size; 
  return ( next == queue->head );
}

//------------------------------------------------------------------------------------
/**
 * @brief 保持しているデータの個数を取得する
 *
 * @param queue
 */
//------------------------------------------------------------------------------------
static int GetDataNum( const QUEUE* queue )
{
  int num;

  // キューが生成されていない
  GF_ASSERT( queue->data );

  num = queue->tail - queue->head;

  // tail の方が大きいインデックスにある場合
  if( num < 0 )
  {
    num += queue->size;
  }
  
  return num;
}

//------------------------------------------------------------------------------------
/**
 * @brief データを覗き見る
 *
 * @param queue
 * @param no    何番目のデータを見るか
 *
 * @return 指定した位置にあるデータ
 */
//------------------------------------------------------------------------------------
static int PeekData( const QUEUE* queue, int no )
{
  int index;

  // データ位置指定エラー
  GF_ASSERT( no < GetDataNum(queue) );

  // 指定された番号のデータのインデックスを計算
  index = ( queue->head + no ) % queue->size;

  return queue->data[ index ];
}
