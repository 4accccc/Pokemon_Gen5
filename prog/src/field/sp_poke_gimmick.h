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

extern void SPPOKE_GMK_SetupTrio(FIELDMAP_WORK *fieldWork);
extern void SPPOKE_GMK_EndTrio(FIELDMAP_WORK *fieldWork);
extern void SPPOKE_GMK_MoveTrio(FIELDMAP_WORK *fieldWork);

extern void SPPOKE_GMK_SetupMerodhia(FIELDMAP_WORK *fieldWork);
extern void SPPOKE_GMK_EndMerodhia(FIELDMAP_WORK *fieldWork);
extern void SPPOKE_GMK_MoveMerodhia(FIELDMAP_WORK *fieldWork);

extern void SPPOKE_GMK_SetupZoroa(FIELDMAP_WORK *fieldWork);
extern void SPPOKE_GMK_EndZoroa(FIELDMAP_WORK *fieldWork);
extern void SPPOKE_GMK_MoveZoroa(FIELDMAP_WORK *fieldWork);

// swan追加
// 【配布】ケルディオ
extern void SPPOKE_GMK_SetupKerudhio( FIELDMAP_WORK *p_fieldmap );
extern void SPPOKE_GMK_EndKerudhio( FIELDMAP_WORK *p_fieldmap );
extern void SPPOKE_GMK_MoveKerudhio( FIELDMAP_WORK *p_fieldmap );
// ケルディオ演出イベント
extern GMEVENT* KERUDHIO_GIMMICK_Event( GAMESYS_WORK *gsys );
// ケルディオ エフェクト削除
extern void KERUDHIO_GIMMICK_EffectDelete( GAMESYS_WORK *gsys );


extern GMEVENT *SPPOKE_GMK_MoveBall(
    GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType,
    const VecFx32 *inStart, const VecFx32 *inEnd ,
    const fx32 inHeight, const u32 inSync);

extern void SPPOKE_GMK_StartBallAnm(  GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType,
                                      const VecFx32 *inPos );

extern GMEVENT *SPPOKE_GMK_WaitPokeAppear( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType );

extern GMEVENT *SPPOKE_GMK_WaitBallAnmEnd( GAMESYS_WORK *gsys, const BALL_ANM_TYPE inType );

extern GMEVENT *SPPOKE_GMK_SmokeAnm(  GAMESYS_WORK *gsys, const VecFx32 *inPos,
                                      const int inBefore, const int inAfter );
