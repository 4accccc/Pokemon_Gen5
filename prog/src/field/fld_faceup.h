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

typedef struct FACEUP_WORK_tag * FACEUP_WK_PTR;

extern GMEVENT *FLD_FACEUP_Start(const int inTypeNo, GAMESYS_WORK *gsys);
extern GMEVENT *FLD_FACEUP_End(GAMESYS_WORK *gsys);
extern void FLD_FACEUP_Change(void);
