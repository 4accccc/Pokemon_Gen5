//======================================================================
/**
 * @file  gym_fly.h
 * @bfief  îÚçsÉWÉÄ
 * @author  Saito
 */
//======================================================================
#ifndef __GYM_FLY_H__
#define __GYM_FLY_H__

extern void GYM_FLY_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_FLY_End(FIELDMAP_WORK *fieldWork);
extern void GYM_FLY_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *GYM_FLY_Shot(GAMESYS_WORK *gsys, const u8 inShotIdx);

#endif //__GYM_FLY_H__
