/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー リスト画面の復帰データ
 * @file   research_list_recovery.h
 * @author obata
 * @date   2010.05.21
 */
///////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <gflib.h>

typedef struct _RESEARCH_RADAR_LIST_RECOVERY_DATA RRL_RECOVERY_DATA;


extern RRL_RECOVERY_DATA* RRL_RECOVERY_CreateData( HEAPID heapID );
extern void RRL_RECOVERY_DeleteData( RRL_RECOVERY_DATA* data );
extern void RRL_RECOVERY_InitData( RRL_RECOVERY_DATA* data );

extern u8 RRL_RECOVERY_GetTopicCursorPos( const RRL_RECOVERY_DATA* data );
extern int RRL_RECOVERY_GetScrollCursorPos( const RRL_RECOVERY_DATA* data );
extern int RRL_RECOVERY_GetScrollValue( const RRL_RECOVERY_DATA* data );

extern void RRL_RECOVERY_SetTopicCursorPos( RRL_RECOVERY_DATA* data, u8 topicCursorPos );
extern void RRL_RECOVERY_SetScrollCursorPos( RRL_RECOVERY_DATA* data, int scrollCursorPos );
extern void RRL_RECOVERY_SetScrollValue( RRL_RECOVERY_DATA* data, int scrollValue );
