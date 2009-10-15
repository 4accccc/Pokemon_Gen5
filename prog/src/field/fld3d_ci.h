//======================================================================
/**
 * @file	fld3d_ci.h
 * @brief	フィールド3Dカットイン
 * @author	Saito
 *
 */
//======================================================================
#ifndef __FLD3D_CI_H__
#define __FLD3D_CI_H__

#include <gflib.h>
#include "fld_particle.h"
#include "gamesystem/gamesystem.h"

typedef struct FLD3D_CI_tag * FLD3D_CI_PTR;

extern FLD3D_CI_PTR FLD3D_CI_Init(const HEAPID inHeapID, FLD_PRTCL_SYS_PTR inPrtclSysPtr);
extern void FLD3D_CI_End(FLD3D_CI_PTR ptr);
extern void FLD3D_CI_Draw( FLD3D_CI_PTR ptr );
extern void FLD3D_CI_CallCutIn( GAMESYS_WORK *gsys, FLD3D_CI_PTR ptr, const u8 inCutInNo );

#endif  //__FLD3D_CI_H__
