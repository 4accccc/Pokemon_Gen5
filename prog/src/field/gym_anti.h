//======================================================================
/**
 * @file  gym_anti.h
 * @brief  ƒAƒ“ƒ`ƒWƒ€
 * @author  Saito
 */
//======================================================================
#pragma once
extern void GYM_ANTI_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_ANTI_End(FIELDMAP_WORK *fieldWork);
extern void GYM_ANTI_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *GYM_ANTI_SwOnOff(GAMESYS_WORK *gsys, const u8 inSwIdx, const u8 inSwState);
extern GMEVENT *GYM_ANTI_OpenDoor(GAMESYS_WORK *gsys, const u8 inDoorIdx);

