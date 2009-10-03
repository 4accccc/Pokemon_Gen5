//======================================================================
/**
 * @file  gym_anti.h
 * @brief  ƒAƒ“ƒ`ƒWƒ€
 * @author  Saito
 */
//======================================================================
#ifndef __GYM_ANTI_H__
#define __GYM_ANTI_H__

extern void GYM_ANTI_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_ANTI_End(FIELDMAP_WORK *fieldWork);
extern void GYM_ANTI_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *GYM_ANTI_SwOn(GAMESYS_WORK *gsys, const u8 inSwIdx);
extern GMEVENT *GYM_ANTI_OpenDoor(GAMESYS_WORK *gsys, const u8 inDoorIdx);

#endif //__GYM_ANTI_H__
