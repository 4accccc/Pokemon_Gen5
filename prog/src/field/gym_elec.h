//======================================================================
/**
 * @file  gym_elec.h
 * @brief  “d‹CƒWƒ€
 * @author  Saito
 * @date  09.09.02
 */
//======================================================================

#ifndef __GYM_ELEC_H__
#define __GYM_ELEC_H__

#include "field/fieldmap_proc.h"  //for FIELDMAP_WORK

extern void GYM_ELEC_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_ELEC_End(FIELDMAP_WORK *fieldWork);
extern void GYM_ELEC_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *GYM_ELEC_ChangePoint(GAMESYS_WORK *gsys, const u8 inLeverIdx);
extern GMEVENT *GYM_ELEC_CreateMoveEvt(GAMESYS_WORK *gsys);

#endif  //__GYM_ELEC_H__
