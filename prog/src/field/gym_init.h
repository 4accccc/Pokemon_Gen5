//======================================================================
/**
 * @file  gym_init.h
 * @brief  ÉWÉÄèâä˙âª
 * @author  Saito
 */
//======================================================================
#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"

extern void GYM_INIT_Elec(GAMESYS_WORK *gsys, u16 seq );
extern void GYM_INIT_Normal(GAMESYS_WORK *gsys, const BOOL inOpened);
extern void GYM_INIT_Anti(GAMESYS_WORK *gsys, const int inCurtain);
extern void GYM_INIT_Fly(GAMESYS_WORK *gsys);
extern void GYM_INIT_Insect(GAMESYS_WORK *gsys, u16 **sw_work);
extern void GYM_INIT_Dragon(GAMESYS_WORK *gsys);

