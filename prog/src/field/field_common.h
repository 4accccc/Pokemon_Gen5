
#pragma once

#include "gamesystem/gamesystem.h"
#include "field/fieldmap.h"

extern BOOL CalcSetGroundMove( const FLDMAPPER* g3Dmapper, FLDMAPPER_GRIDINFODATA* gridInfoData, 
								VecFx32* pos, VecFx32* vecMove, fx32 speed );
extern FLDMAPPER* GetFieldG3Dmapper( FIELDMAP_WORK * fieldWork );
extern GFL_BBDACT_SYS* GetBbdActSys( FIELDMAP_WORK * fieldWork );

#define MV_SPEED		(2*FX32_ONE)
#define RT_SPEED		(FX32_ONE/8)
