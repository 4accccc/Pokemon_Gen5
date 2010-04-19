//======================================================================
/**
 * @file  legend_gmk.h
 * @brief  伝説ポケモンギミック
 * @author  Saito
 */
//======================================================================
#pragma once

extern void LEGEND_GMK_Setup(FIELDMAP_WORK *fieldWork);
extern void LEGEND_GMK_End(FIELDMAP_WORK *fieldWork);
extern void LEGEND_GMK_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *LEGEND_GMK_Start( GAMESYS_WORK *gsys );

