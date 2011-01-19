//======================================================================
/**
 * @file  gym_inssect.h
 * @brief  ’ŽƒWƒ€
 * @author Miyuki Iwasawa
 * @date  11.01.19
 */
//======================================================================
#pragma once

extern void GYM_INSECT_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_INSECT_End(FIELDMAP_WORK *fieldWork);
extern void GYM_INSECT_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *GYM_INSECT_CreateGateInEvent(GAMESYS_WORK *gsys);

