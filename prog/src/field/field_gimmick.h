//============================================================================================
/**
 * @file	field_gimmick.h
 * @brief	マップ固有の仕掛けを発動するかどうかを判定するソース
 * @author	saito
 * @date	2006.02.09
 *
 */
//============================================================================================
#pragma once

#include "field/fieldmap_proc.h"

typedef void (*FLD_GMK_SETUP_FUNC)(FIELDMAP_WORK *);
typedef void (*FLD_GMK_END_FUNC)(FIELDMAP_WORK *);
typedef void (*FLD_GMK_MOVE_FUNC)(FIELDMAP_WORK *);
typedef BOOL (*FLD_GMK_HIT_CHECK)(FIELDMAP_WORK * , const int, const int, const fx32, BOOL* );

typedef struct {
  u32                 gimmick_id;
  FLD_GMK_SETUP_FUNC  setup_func;
  FLD_GMK_END_FUNC    end_func;
  FLD_GMK_MOVE_FUNC   move_func;
  FSOverlayID         ov_id;
}FLD_GMK_TABLE;

extern void FLDGMK_SetUpFieldGimmick(FIELDMAP_WORK *fieldWork);
extern void FLDGMK_EndFieldGimmick(FIELDMAP_WORK *fieldWork);
extern void FLDGMK_MoveFieldGimmick(FIELDMAP_WORK *fieldWork);
#if 0
extern BOOL FLDGMK_FieldGimmickHitCheck(FIELDSYS_WORK *fsys,
										const int inGridX, const int inGridZ,
										const fx32 inHeight,
										BOOL *outHit);
#endif

extern BOOL FLDGMK_GimmickCodeCheck(FIELDMAP_WORK *fieldWork, int gimmick_id);

