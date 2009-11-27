//======================================================================
/**
 * @file  pleasure_boat.h
 * @brief —V——‘D
 * @author  Saito
 */
//======================================================================

#pragma once

typedef struct PL_BOAT_WORK_tag * PL_BOAT_WORK_PTR;

extern PL_BOAT_WORK_PTR PL_BOAT_Init(void);
extern void PL_BOAT_End(PL_BOAT_WORK_PTR *work);
extern void PL_BOAT_Main(PL_BOAT_WORK_PTR work);
extern BOOL PL_BOAT_Check(PL_BOAT_WORK_PTR work);
extern void PL_BOAT_FlushTime(PL_BOAT_WORK_PTR work);

