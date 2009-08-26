//============================================================================================
/**
 * @file	field_gimmick.h
 * @brief	マップ固有の仕掛けを発動するかどうかを判定するソース
 * @author	saito
 * @date	2006.02.09
 *
 */
//============================================================================================
#ifndef __FIELD_GIMMICK_H__
#define __FIELD_GIMMICK_H__

#include "field/fieldmap_proc.h"

typedef void (*FLD_GMK_SETUP_FUNC)(FIELDMAP_WORK *);
typedef void (*FLD_GMK_END_FUNC)(FIELDMAP_WORK *);
typedef void (*FLD_GMK_MOVE_FUNC)(FIELDMAP_WORK *);
typedef BOOL (*FLD_GMK_HIT_CHECK)(FIELDMAP_WORK * , const int, const int, const fx32, BOOL* );

extern void FLDGMK_SetUpFieldGimmick(FIELDMAP_WORK *fieldWork);
extern void FLDGMK_EndFieldGimmick(FIELDMAP_WORK *fieldWork);
extern void FLDGMK_MoveFieldGimmick(FIELDMAP_WORK *fieldWork);
#if 0
extern BOOL FLDGMK_FieldGimmickHitCheck(FIELDSYS_WORK *fsys,
										const int inGridX, const int inGridZ,
										const fx32 inHeight,
										BOOL *outHit);
#endif
#endif	//__FIELD_GIMMICK_H__

