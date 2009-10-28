//======================================================================
/**
 * @file  musical_event.c
 * @brief ミュージカル管理イベント
 * @author  ariizumi
 * @data  09/09/08
 *
 * モジュール名：MUSICAL_
 */
//======================================================================
#pragma once

typedef struct _MUSICAL_EVENT_WORK MUSICAL_EVENT_WORK;

extern GMEVENT* MUSICAL_CreateEvent( GAMESYS_WORK * gsys , GAMEDATA *gdata , const BOOL isComm );

extern const u8 MUSICAL_EVENT_GetSelfIndex( MUSICAL_EVENT_WORK *evWork );
extern const u8 MUSICAL_EVENT_GetPosIndex( MUSICAL_EVENT_WORK *evWork , const u8 pos );
extern const u8 MUSICAL_EVENT_GetMaxCondition( MUSICAL_EVENT_WORK *evWork );
extern STRBUF* MUSICAL_EVENT_CreateStr_ProgramTitle( MUSICAL_EVENT_WORK *evWork , HEAPID heapId );
