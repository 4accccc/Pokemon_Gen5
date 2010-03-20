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


//-------------------------------------
///	パレス　進入先の街などでのギミック
//=====================================
extern void PALACE_MAP_GMK_Setup(FIELDMAP_WORK *fieldWork);
extern void PALACE_MAP_GMK_End(FIELDMAP_WORK *fieldWork);
extern void PALACE_MAP_GMK_Move(FIELDMAP_WORK *fieldWork);

