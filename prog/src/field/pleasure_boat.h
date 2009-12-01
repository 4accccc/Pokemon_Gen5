//======================================================================
/**
 * @file  pleasure_boat.h
 * @brief —V——‘D
 * @author  Saito
 */
//======================================================================

#pragma once

#include "pleasure_boat_ptr.h"
#include "gamesystem/gamesystem.h"    //for GMEVENT

extern PL_BOAT_WORK_PTR PL_BOAT_Init(void);
extern void PL_BOAT_End(PL_BOAT_WORK_PTR *work);
extern void PL_BOAT_Main(PL_BOAT_WORK_PTR work);
extern GMEVENT *PL_BOAT_CheckEvt(GAMESYS_WORK *gsys, PL_BOAT_WORK_PTR work);
extern void PL_BOAT_AddTimeEvt(PL_BOAT_WORK_PTR work, const int inAddSec);
extern int PL_BOAT_GetTrNum(PL_BOAT_WORK_PTR work, const int inSearchType);


