//////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  キュー
 * @file   queue.h
 * @author obata
 * @date   2010.02.20
 */
//////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>


//====================================================================================
// ■キューの不完全形
//====================================================================================
typedef struct _QUEUE QUEUE;


//====================================================================================
// ■キューの操作関数
//====================================================================================

// 生成・破棄
extern QUEUE* QUEUE_Create( int queueSize, HEAPID heapID );
extern void QUEUE_Delete( QUEUE* queue );

// エンキュー・デキュー
extern void QUEUE_EnQueue( QUEUE* queue, int val );
extern int QUEUE_DeQueue ( QUEUE* queue );

// クリア
extern void QUEUE_Clear( QUEUE* queue );

// チェック
extern BOOL QUEUE_IsEmpty( const QUEUE* queue ); // 空かどうか
extern BOOL QUEUE_IsFull ( const QUEUE* queue ); // 満タンかどうか

// データ取得
extern int QUEUE_GetDataNum( const QUEUE* queue ); // 保持しているデータの個数
extern int QUEUE_PeekData( const QUEUE* queue, int no ); // 指定した順番にあるデータ
