//======================================================================
/**
 * @file	event_mapchange.h
 * @brief
 * @date	2008.11.05
 * @author	tamada GAME FREAK inc.
 */
//======================================================================
#pragma once
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"

extern GMEVENT * DEBUG_EVENT_SetFirstMapIn(GAMESYS_WORK * gsys, GAME_INIT_WORK * game_init_work);
extern GMEVENT * DEBUG_EVENT_ChangeToNextMap(GAMESYS_WORK * gsys);
extern GMEVENT * DEBUG_EVENT_FieldSample(GAMESYS_WORK * gsys);
