//======================================================================
/**
 * @file  gym_init.h
 * @bfief  ÉWÉÄèâä˙âª
 * @author  Saito
 */
//======================================================================
#ifndef __GYM_INIT_H__

#define __GYM_INIT_H__

#include "field/fieldmap_proc.h"

extern void GYM_INIT_Elec(FIELDMAP_WORK *fieldWork);
extern void GYM_INIT_Normal(FIELDMAP_WORK *fieldWork, const u8 inRoomNo);

#endif  //__GYM_INIT_H__
