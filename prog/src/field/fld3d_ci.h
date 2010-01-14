//======================================================================
/**
 * @file	fld3d_ci.h
 * @brief	フィールド3Dカットイン
 * @author	Saito
 *
 */
//======================================================================
#pragma once

#include <gflib.h>
#include "fld_particle.h"
#include "gamesystem/gamesystem.h"

#ifdef PM_DEBUG
#include "field_camera.h"
#include "field_player.h"
#include "field_nogrid_mapper.h"
#endif

typedef struct FLD3D_CI_tag * FLD3D_CI_PTR;

extern FLD3D_CI_PTR FLD3D_CI_Init(const HEAPID inHeapID, FLD_PRTCL_SYS_PTR inPrtclSysPtr);
extern void FLD3D_CI_End(FLD3D_CI_PTR ptr);
extern void FLD3D_CI_Draw( FLD3D_CI_PTR ptr );

extern GMEVENT *FLD3D_CI_CreateCutInEvt(GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, const u8 inCutInNo);
extern GMEVENT *FLD3D_CI_CreatePokeCutInEvt( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr,
                                      const int inMonsNo, const int inFormNo,
                                      const int inSex, const int inRare, const BOOL inEgg );
extern GMEVENT *FLD3D_CI_CreatePokeCutInEvtTemoti( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, u8 pos );
extern GMEVENT *FLD3D_CI_CreateEncCutInEvt( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, const int inEndCutinNo );

#ifdef PM_DEBUG
extern void FLD3D_CI_FlySkyCameraDebug(
    GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, FIELD_CAMERA *camera, FIELD_PLAYER * player, FLDNOGRID_MAPPER *mapper);
extern void FLD3D_CI_CallCutIn( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, const u8 inCutInNo );
extern void FLD3D_CI_CallPokeCutIn( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr );
#endif

