//======================================================================
/**
 * @file  gym_normal.h
 * @brief  ÉmÅ[É}ÉãÉWÉÄ
 * @author  Saito
 */
//======================================================================

#pragma once

#include "field/fieldmap_proc.h"  //for FIELDMAP_WORK

extern void GYM_NORMAL_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_NORMAL_End(FIELDMAP_WORK *fieldWork);
extern void GYM_NORMAL_Move(FIELDMAP_WORK *fieldWork);

extern void GYM_NORMAL_Unrock(FIELDMAP_WORK *fieldWork);
extern BOOL GYM_NORMAL_CheckRock(FIELDMAP_WORK *fieldWork);
extern GMEVENT * GYM_NORMAL_MoveWall(GAMESYS_WORK *gsys, const u8 inWallIdx);

