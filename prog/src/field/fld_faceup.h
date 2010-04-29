//======================================================================
/*
 * @file	fld_faceup.h
 * @brief	ŠçƒAƒbƒv
 * @author saito
 */
//======================================================================

#pragma once

#include <gflib.h>
#include "field/fieldmap_proc.h"
#include "gamesystem/gamesystem.h"

#include "fld_faceup_ptr.h"
#include "scrcmd_work.h"    //for SCRCMD_WORK


extern GMEVENT *FLD_FACEUP_Start( const int inBackNo, const int inCharNo,
                                  GAMESYS_WORK *gsys, SCRCMD_WORK * scrCmdWork );
extern GMEVENT *FLD_FACEUP_End(GAMESYS_WORK *gsys);
extern void FLD_FACEUP_Release( FIELDMAP_WORK *fieldmap );
