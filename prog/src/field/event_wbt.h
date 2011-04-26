//======================================================================
/**
 * @file  event_wbt.h
 * @brief  バトルサブウェイ　イベント
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "gamesystem/game_event.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
FS_EXTERN_OVERLAY( notwifi_event_wbt );

extern GMEVENT * WBT_EVENT_SetSelectPokeList( GAMESYS_WORK * gsys, void * parent_work );

extern GMEVENT * WBT_EVENT_ShowTournamentList( GAMESYS_WORK * gsys, void * parent_work );

extern GMEVENT * WBT_EVENT_TrainerBattle( GAMESYS_WORK *gsys, void * parent_work );

//
extern GMEVENT * WIN_RECORD_EVENT_Show( GAMESYS_WORK * gsys, void * parent_work );

//
extern GMEVENT * T_DOWNLOAD_EVENT_Show( GAMESYS_WORK * gsys, void * parent_work );


