//======================================================================
/**
 * @file  gym_normal.c
 * @brief  ノーマルジム
 * @author  Saito
 */
//======================================================================

#include "fieldmap.h"
#include "gym_normal_sv.h"
#include "gym_normal.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_normal.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript

#include "../../../resource/fldmapdata/gimmick/gym_normal/gym_normal_local_def.h"

#define GYM_NORMAL_UNIT_IDX (0)
#define GYM_NORMAL_TMP_ASSIGN_ID  (1)

//ジム内部中の一時ワーク
typedef struct GYM_NORMAL_TMP_tag
{
  u32 dummy;
}GYM_NORMAL_TMP;

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_NORMAL_Setup(FIELDMAP_WORK *fieldWork)
{
  GYM_NORMAL_SV_WORK *gmk_sv_work;
  GYM_NORMAL_TMP *tmp;

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_NORMAL_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_NORMAL_TMP));

  tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_NORMAL_TMP_ASSIGN_ID);

  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_NORM );
  }
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_NORMAL_End(FIELDMAP_WORK *fieldWork)
{
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_NORMAL_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	        fieldWork     フィールドワークポインタ
 * @return        none
 */
//--------------------------------------------------------------
void GYM_NORMAL_Move(FIELDMAP_WORK *fieldWork)
{
}
