//======================================================================
/**
 * @file  gym_normal.h
 * @bfief  ÉmÅ[É}ÉãÉWÉÄ
 * @author  Saito
 */
//======================================================================

#ifndef __GYM_NORMAL_H__
#define __GYM_NORMAL_H__

#include "field/fieldmap_proc.h"

extern void GYM_NORMAL_SetupRoom1(FIELDMAP_WORK *fieldWork);
extern void GYM_NORMAL_SetupRoom2(FIELDMAP_WORK *fieldWork);
extern void GYM_NORMAL_End(FIELDMAP_WORK *fieldWork);
extern void GYM_NORMAL_Move(FIELDMAP_WORK *fieldWork);
/**
extern GMEVENT *GYM_ELEC_ChangePoint(GAMESYS_WORK *gsys, const u8 inLeverIdx);
extern GMEVENT *GYM_ELEC_CreateMoveEvt(GAMESYS_WORK *gsys);
*/
#endif  //__GYM_ELEC_H__
