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
#include "field/fieldmap.h"

extern GMEVENT * DEBUG_EVENT_SetFirstMapIn(GAMESYS_WORK * gsys, GAME_INIT_WORK * game_init_work);
extern GMEVENT * DEBUG_EVENT_ChangeMap(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap, u16 mapid);
extern GMEVENT * DEBUG_EVENT_ChangeMapPos(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap,
		u16 mapid, const VecFx32 * pos);
extern GMEVENT * DEBUG_EVENT_ChangeToNextMap(GAMESYS_WORK * gsys, FIELD_MAIN_WORK *fieldmap);
extern void DEBUG_EVENT_ChangeEventMapChange( GAMESYS_WORK *gsys, GMEVENT *event,FIELD_MAIN_WORK *fieldmap, ZONEID zone_id );
extern GMEVENT * DEBUG_EVENT_FieldSample(GAMESYS_WORK * gsys, FIELD_MAIN_WORK *fieldmap);
