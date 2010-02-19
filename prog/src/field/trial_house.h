//======================================================================
/**
 * @file  trial_house.h
 * @brief  トライアルハウス
 * @author  Saito
 */
//======================================================================
#pragma once

#include "trial_house_ptr.h"
#include "savedata/bsubway_savedata.h"  //for BSWAY_PLAYMODE

extern TRIAL_HOUSE_WORK_PTR TRIAL_HOUSE_Start( void );
extern void TRIAL_HOUSE_End( TRIAL_HOUSE_WORK_PTR *ptr );
extern void TRIAL_HOUSE_SetPlayMode( TRIAL_HOUSE_WORK_PTR ptr, const BSWAY_PLAYMODE inPlayMode );

extern u16 TRIAL_HOUSE_GetTrainerOBJCode( TRIAL_HOUSE_WORK_PTR ptr );


