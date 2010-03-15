//======================================================================
/**
 * @file  sp_poke_gimmick.h
 * @brief  配布系ポケモンギミック
 * @author  Saito
 */
//======================================================================
#pragma once

typedef enum
{
  BALL_ANM_TYPE_OUT = 0,
  BALL_ANM_TYPE_IN
}BALL_ANM_TYPE;

extern void SPPOKE_GMK_Setup(FIELDMAP_WORK *fieldWork);
extern void SPPOKE_GMK_End(FIELDMAP_WORK *fieldWork);
extern void SPPOKE_GMK_Move(FIELDMAP_WORK *fieldWork);

extern GMEVENT *SPPOKE_GMK_MoveBall(
    GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType,
    const VecFx32 *inStart, const VecFx32 *inEnd ,
    const fx32 inHeight, const u32 inSync);

extern void SPPOKE_GMK_StartBallAnm( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType, const VecFx32 *inPos );

extern GMEVENT *SPPOKE_GMK_WaitPokeAppear( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType );

extern GMEVENT *SPPOKE_GMK_WaitBallAnmEnd( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType );
