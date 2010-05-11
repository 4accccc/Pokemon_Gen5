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

extern GMEVENT *LEGEND_GMK_MoveBall(  GAMESYS_WORK *gsys, const VecFx32 *inStart, const VecFx32 *inEnd,
                                      const fx32 inHeight, const u32 inSync );
extern void LEGEND_GMK_StartBallAnm( GAMESYS_WORK *gsys, const VecFx32 *inPos );
extern GMEVENT *LEGEND_GMK_WaitPokeAppear( GAMESYS_WORK *gsys );
extern GMEVENT *LEGEND_GMK_WaitBallAnmEnd( GAMESYS_WORK *gsys );

