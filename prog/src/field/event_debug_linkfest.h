//===============================================================================
/*
 *  @file event_debug_livecomm.h
 *  @brief  リンクフェスティバル デバッグメニューイベント
 *  @author Miyuki Iwasawa
 *  @date   10.04.19
 */
//===============================================================================

#pragma once

#ifdef PM_DEBUG

FS_EXTERN_OVERLAY( d_linkfest );

extern GMEVENT* DEBUG_EVENT_LinkFest( GAMESYS_WORK* gsys, void* parent_work );
extern GMEVENT* DEBUG_EVENT_LinkFestFromField( GAMESYS_WORK* gsys, void* parent_work );

#endif  //PM_DEBUG


