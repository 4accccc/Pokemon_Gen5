//======================================================================
/**
 * @file  gym_ground_ent.c
 * @brief  地面ジムエントランス
 * @author  Saito
 * @date  09.11.11
 */
//======================================================================

#include "fieldmap.h"
//#include "gym_ground_sv.h"
#include "gym_ground_ent.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_ground.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP

#include "sound/pm_sndsys.h"

#include "gym_ground_se_def.h"

#define GYM_GROUND_ENT_UNIT_IDX (0)
#define GYM_GROUND_ENT_TMP_ASSIGN_ID  (1)


//ジム内部中の一時ワーク
typedef struct GYM_GROUND_ENT_TMP_tag
{
  u32 dummy;
}GYM_GROUND_ENT_TMP;

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_ENT_Setup(FIELDMAP_WORK *fieldWork)
{
  GYM_GROUND_ENT_TMP *tmp;

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_GROUND_ENT_TMP));
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_ENT_End(FIELDMAP_WORK *fieldWork)
{
  GYM_GROUND_ENT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_GROUND_ENT_Move(FIELDMAP_WORK *fieldWork)
{
//  GYM_GROUND_ENT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_ENT_TMP_ASSIGN_ID);
}

