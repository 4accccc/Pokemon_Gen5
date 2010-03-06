////////////////////////////////////////////////////////////////////////////
/**
 * @brief  キュー
 * @file   queue.h
 * @author obata
 * @date   2010.02.20
 */
////////////////////////////////////////////////////////////////////////////
#include <gflib.h>


//==========================================================================
// ■キューの不完全型
//==========================================================================
typedef struct _QUEUE QUEUE;


//==========================================================================
// ■キューの操作関数
//========================================================================== 
// キューを生成する
extern QUEUE* QUEUE_Create( int queueSize, HEAPID heapID );
// キューを破棄する
extern void QUEUE_Delete( QUEUE* queue ); 
// データをエンキューする
extern void QUEUE_EnQueue( QUEUE* queue, int val );
// データをデキューする
extern int QUEUE_DeQueue ( QUEUE* queue ); 
// キューをクリアする
extern void QUEUE_Clear( QUEUE* queue ); 
// キューの状態をチェックする
extern BOOL QUEUE_IsEmpty( const QUEUE* queue ); // 空かどうか
extern BOOL QUEUE_IsFull ( const QUEUE* queue ); // 満タンかどうか
extern int QUEUE_GetDataNum( const QUEUE* queue ); // 保持しているデータの個数
// 指定した順番にあるデータを見る
extern int QUEUE_PeekData( const QUEUE* queue, int no );
