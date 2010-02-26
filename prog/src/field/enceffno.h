//============================================================================================
/**
 * @file	enceffno.h
*/
//============================================================================================

#pragma once

#include "field/fieldmap_proc.h"
#include "encount_data.h"

extern void ENCEFFNO_GetWildEncEffNoBgmNo( const int inMonsNo, ENCOUNT_TYPE inEncType, FIELDMAP_WORK *fieldmap, int *outEffNo, u16 *outBgmNo );
extern void ENCEFFNO_GetTrEncEffNoBgmNo( const int inTrID, FIELDMAP_WORK *fieldmap, int *outEffNo, u16 *outBgmNo );

