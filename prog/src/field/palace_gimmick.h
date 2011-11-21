//======================================================================
/**
 * @file  palace_gimmick.h
 * @brief  パレスギミック
 * @author  Saito
 */
//======================================================================

extern void PALACE_GMK_Setup(FIELDMAP_WORK *fieldWork);
extern void PALACE_GMK_End(FIELDMAP_WORK *fieldWork);
extern void PALACE_GMK_Move(FIELDMAP_WORK *fieldWork);

// ミッション開始のエフェクト
extern void PALACE_GMK_EffectStart( GAMESYS_WORK *gsys );
// ミッション開始のエフェクト2つ目
extern void PALACE_GMK_EffectStart2( GAMESYS_WORK *gsys );

