//======================================================================
/**
 * @file  gym_fly.h
 * @brief  îÚçsÉWÉÄ
 * @author  Saito
 */
//======================================================================
#ifndef __GYM_FLY_H__
#define __GYM_FLY_H__

extern void GYM_FLY_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_FLY_End(FIELDMAP_WORK *fieldWork);
extern void GYM_FLY_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *GYM_FLY_CreateShotEvt(GAMESYS_WORK *gsys);

#endif //__GYM_FLY_H__
