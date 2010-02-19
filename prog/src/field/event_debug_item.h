//======================================================================
/**
 * @file	  event_debug_item.h
 * @brief	  デバッグアイテムイベント
 * @author	k.ohno
 * @date	  2009.06.30
 */
//======================================================================

#pragma once



FS_EXTERN_OVERLAY( debug_item );

extern GMEVENT* EVENT_DebugItemMake( GAMESYS_WORK * gsys, void * work );
//extern GMEVENT* EVENT_DebugItemMake( GAMESYS_WORK * gsys, HEAPID heapID);


