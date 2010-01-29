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

typedef struct FACEUP_WORK_tag * FACEUP_WK_PTR;

extern void FLD_FACEUP_Start(const int inTypeNo, FIELDMAP_WORK * fieldmap);
extern void FLD_FACEUP_End(FIELDMAP_WORK * fieldmap);
extern void FLD_FACEUP_Change(void);
