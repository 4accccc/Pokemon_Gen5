//======================================================================
/**
 * @file  gym_fly.c
 * @brief  飛行ジム
 * @author  Saito
 */
//======================================================================
#include "fieldmap.h"
#include "gym_fly_sv.h"
#include "gym_fly.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"
#include "system/ica_anime.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_fly.naix"

#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/c06gym0101_def.h"  //for SCRID_～

#include "../../../resource/fldmapdata/gimmick/gym_fly/gym_fly_local_def.h"

#include "sound/pm_sndsys.h"

#include "gym_fly_se_def.h"


#define GYM_FLY_UNIT_IDX (0)
#define GYM_FLY_TMP_ASSIGN_ID  (1)

//ジム内部中の一時ワーク
typedef struct GYM_FLY_TMP_tag
{
  u32 dummy;
}GYM_FLY_TMP;

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	    fieldWork     フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_FLY_Setup(FIELDMAP_WORK *fieldWork)
{
  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_FLY_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_FLY_TMP));
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param   fieldWork     フィールドワークポインタ
 * @return  none
 */
//--------------------------------------------------------------
void GYM_FLY_End(FIELDMAP_WORK *fieldWork)
{
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_FLY_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_FLY_Move(FIELDMAP_WORK *fieldWork)
{
}
