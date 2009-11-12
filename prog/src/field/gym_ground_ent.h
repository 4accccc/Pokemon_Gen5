//======================================================================
/**
 * @file  gym_ground_ent.h
 * @brief  地面ジムエントランス
 * @author  Saito
 * @date  09.11.11
 */
//======================================================================

#pragma once

extern void GYM_GROUND_ENT_Setup(FIELDMAP_WORK *fieldWork);
extern void GYM_GROUND_ENT_End(FIELDMAP_WORK *fieldWork);
extern void GYM_GROUND_ENT_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *GYM_GROUND_ENT_CreateExitLiftMoveEvt(GAMESYS_WORK *gsys, const BOOL inExit);

