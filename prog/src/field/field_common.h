
#pragma once

#include "gamesystem/gamesystem.h"
#include "field/fieldmap.h"

extern BOOL CalcSetGroundMove( const FLDMAPPER* g3Dmapper, FLDMAPPER_GRIDINFODATA* gridInfoData, 
								VecFx32* pos, VecFx32* vecMove, fx32 speed );
extern GFL_G3D_CAMERA* GetG3Dcamera( FIELD_MAIN_WORK * fieldWork );
extern FLDMAPPER* GetFieldG3Dmapper( FIELD_MAIN_WORK * fieldWork );
extern GFL_BBDACT_SYS* GetBbdActSys( FIELD_MAIN_WORK * fieldWork );

#define MV_SPEED		(2*FX32_ONE)
#define RT_SPEED		(FX32_ONE/8)
#define	CAMERA_TARGET_HEIGHT	(4)//(8)
