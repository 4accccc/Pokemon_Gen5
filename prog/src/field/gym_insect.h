//======================================================================
/**
 * @file  gym_inssect.h
 * @brief  ’ŽƒWƒ€
 * @author  Saito
 */
//======================================================================
#pragma once

#include "field/fieldmap_proc.h"  //for FIELDMAP_WORK

extern void GYM_INSECT_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_INSECT_End(FIELDMAP_WORK *fieldWork);
extern void GYM_INSECT_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT* GYM_INSECT_CreatePushWallEvt(GAMESYS_WORK *gsys, const int inX, const int inZ);

extern GMEVENT* GYM_INSECT_CreateSwitchEvt(GAMESYS_WORK *gsys, const int inSwIdx);
extern GMEVENT* GYM_INSECT_CreatePoleEvt(GAMESYS_WORK *gsys, const int inPoleIdx);
extern GMEVENT* GYM_INSECT_CreateTrTrapEvt( GAMESYS_WORK *gsys,
                                            const int inTrEvtIdx);
