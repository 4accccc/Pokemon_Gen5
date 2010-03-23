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


extern void PALACE_GMK_StartBarrierEffect( FIELDMAP_WORK *fieldWork, const VecFx32* cp_pos );
extern BOOL PALACE_GMK_IsBarrierEffect( const FIELDMAP_WORK * fieldWork );
