///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー リスト画面の復帰データ
 * @file   research_list_recovery.c
 * @author obata
 * @date   2010.05.21
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "research_list_def.h"
#include "research_list_recovery.h"

#include "topic_id.h"  // for TOPIC_ID_xxxx


// リスト画面復帰データ
struct _RESEARCH_RADAR_LIST_RECOVERY_DATA {

  u8  topicCursorPos;  // TOPIC_ID_xxxx
  int scrollCursorPos;
  int scrollValue;
};


/**
 * @brief 復帰データのワークを生成する
 */
RRL_RECOVERY_DATA* RRL_RECOVERY_CreateData( HEAPID heapID )
{
  RRL_RECOVERY_DATA* data;

  data = GFL_HEAP_AllocClearMemory( heapID, sizeof(RRL_RECOVERY_DATA) ); 
  return data;
}

/**
 * @brief 復帰データのワークを破棄する
 */
void RRL_RECOVERY_DeleteData( RRL_RECOVERY_DATA* data )
{
  GFL_HEAP_FreeMemory( data );
}

/**
 * @brief 復帰データを初期化する
 */
void RRL_RECOVERY_InitData( RRL_RECOVERY_DATA* data )
{
  data->topicCursorPos  = TOPIC_ID_STYLE;
  data->scrollCursorPos = MIN_SCROLL_CURSOR_POS;
  data->scrollValue     = MIN_SCROLL_VALUE;
}

/**
 * @brief 調査項目カーソルの位置を取得する
 */
u8 RRL_RECOVERY_GetTopicCursorPos( const RRL_RECOVERY_DATA* data )
{
  return data->topicCursorPos;
}

/**
 * @brief スクロールカーソルの位置を取得する
 */
int RRL_RECOVERY_GetScrollCursorPos( const RRL_RECOVERY_DATA* data )
{
  return data->scrollCursorPos;
}

/**
 * @brief スクロール実効値を取得する
 */
int RRL_RECOVERY_GetScrollValue( const RRL_RECOVERY_DATA* data )
{
  return data->scrollValue;
}

/**
 * @brief 調査項目カーソルの位置を設定する
 */
void RRL_RECOVERY_SetTopicCursorPos( RRL_RECOVERY_DATA* data, u8 topicCursorPos )
{
  data->topicCursorPos = topicCursorPos;
}

/**
 * @brief スクロールカーソルの位置を設定する
 */
void RRL_RECOVERY_SetScrollCursorPos( RRL_RECOVERY_DATA* data, int scrollCursorPos )
{
  data->scrollCursorPos = scrollCursorPos;
}

/**
 * @brief スクロール実効値を設定する
 */
void RRL_RECOVERY_SetScrollValue( RRL_RECOVERY_DATA* data, int scrollValue )
{
  data->scrollValue = scrollValue;
}
