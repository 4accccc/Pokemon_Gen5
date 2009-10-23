//======================================================================
/**
 * @file	event_battle.h
 * @brief	イベント：バトル呼び出し
 * @author	tamada GAMEFREAK inc.
 * @date	2009.01.19
 */
//======================================================================
#pragma once
#include <gflib.h>

#include "gamesystem/gamesystem.h"
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
extern GMEVENT * EVENT_WildPokeBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM* bp );
extern GMEVENT * EVENT_TrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, int tr_id );
