#pragma once

typedef struct _FIELD_SETUP FIELD_SETUP;

extern BOOL CalcSetGroundMove( const FLDMAPPER* g3Dmapper, FLDMAPPER_GRIDINFODATA* gridInfoData, 
								VecFx32* pos, VecFx32* vecMove, fx32 speed );
extern GFL_G3D_CAMERA* GetG3Dcamera( FIELD_SETUP* gs );
extern FLDMAPPER* GetFieldG3Dmapper( FIELD_SETUP* gs );
extern GFL_BBDACT_SYS* GetBbdActSys( FIELD_SETUP* gs );
extern GFL_G3D_LIGHTSET* GetG3Dlightset( FIELD_SETUP* gs );

#define MV_SPEED		(2*FX32_ONE)
#define RT_SPEED		(FX32_ONE/8)
#define	CAMERA_TARGET_HEIGHT	(4)//(8)
