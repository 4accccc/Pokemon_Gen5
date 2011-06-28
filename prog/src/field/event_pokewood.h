//======================================================================
/**
 * @file   event_pokewood.h
 * @brief  ポケウッド　イベント
 * @date  2011.04.27
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
typedef struct{
  int type;         // 録画・再生指定
  int senario_no;   // シナリオNO
}POKEWOOD_EVENT_PARAM;

//======================================================================
//  extern
//======================================================================
FS_EXTERN_OVERLAY( notwifi_event_pokewood );

extern void POKEWOOD_EVENT_setupPokemon( const MYSTATUS * mystat, POKEPARTY * party, HEAPID heapID, const s32 scenario_id, const s32 mode );

extern GMEVENT * POKEWOOD_EVENT_Rec( GAMESYS_WORK *gsys, void * parent_work );
extern GMEVENT * POKEWOOD_EVENT_Show( GAMESYS_WORK *gsys, void * parent_work );


extern GMEVENT * PKWD_COUNT_EVENT_Show( GAMESYS_WORK * gsys, void * parent_work );
extern GMEVENT * PKWD_BOOK_EVENT_Show(  GAMESYS_WORK * gsys, void * parent_work );

