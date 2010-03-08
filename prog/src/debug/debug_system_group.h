//======================================================================
/**
 * @file	debug_system_group.h
 * @brief	デバッグシステムのデフォルトグループ登録
 * @author	ariizumi
 * @data	09/05/08
 */
//======================================================================

#ifndef DEBUG_SYSTEM_GROUP_H__
#define DEBUG_SYSTEM_GROUP_H__

#define DEBUGWIN_GROUPID_SYSTEM (200)
#define DEBUGWIN_GROUPID_RTC (201)
#define DEBUGWIN_GROUPID_DEBUG_FLG (202)
#define DEBUGWIN_GROUPID_PDW (203)

void DEBUGWIN_AddSystemGroup( const HEAPID heapId );
void DEBUGWIN_RemoveSystemGroup(void);

#endif //DEBUG_SYSTEM_GROUP_H__
