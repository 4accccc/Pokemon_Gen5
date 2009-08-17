
#pragma once
#include "gflib.h"
#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"

extern GMEVENT* EVENT_CameraRotateLeftDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
extern GMEVENT* EVENT_CameraRotateRightDoorIn( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
extern GMEVENT* EVENT_CameraRotateLeftDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
extern GMEVENT* EVENT_CameraRotateRightDoorOut( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap );
