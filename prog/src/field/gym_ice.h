//======================================================================
/**
 * @file  gym_ice.h
 * @brief  •XƒWƒ€
 * @author  Saito
 */
//======================================================================
#pragma once

extern void GYM_ICE_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_ICE_End(FIELDMAP_WORK *fieldWork);
extern void GYM_ICE_Move(FIELDMAP_WORK *fieldWork);
extern GMEVENT *GYM_ICE_CreateSwEvt(GAMESYS_WORK *gsys, const int inTargetIdx);
extern GMEVENT *GYM_ICE_CreateWallEvt(GAMESYS_WORK *gsys, const int inTargetIdx);

