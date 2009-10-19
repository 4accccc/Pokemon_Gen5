//======================================================================
/**
 * @file  gym_fly.h
 * @brief  îÚçsÉWÉÄ
 * @author  Saito
 */
//======================================================================
#pragma once

extern void GYM_FLY_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_FLY_End(FIELDMAP_WORK *fieldWork);
extern void GYM_FLY_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *GYM_FLY_CreateShotEvt(GAMESYS_WORK *gsys);

