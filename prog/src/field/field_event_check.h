//======================================================================
/**
 * @file  field_event_check.h
 * @brief
 * @date  2009.06.09
 * @author  tamada GAME FREAK inc.
 */
//======================================================================
#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"

#include "field/fieldmap_proc.h"

extern GMEVENT * FIELD_EVENT_CheckNormal_Wrap( GAMESYS_WORK *gsys, void *work );
extern GMEVENT * FIELD_EVENT_CheckUnion( GAMESYS_WORK *gsys, void *work );
extern GMEVENT * FIELD_EVENT_CheckNoGrid( GAMESYS_WORK *gsys, void *work );
extern GMEVENT * FIELD_EVENT_CheckHybrid( GAMESYS_WORK *gsys, void *work );

extern void FIELD_EVENT_ChangeNaminoriStart( GMEVENT *event,
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );

extern BOOL FIELD_EVENT_Check2vs2Battle( GAMESYS_WORK *gsys );
