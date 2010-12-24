//======================================================================
/**
 * @file  gym_anti.c
 * @brief  アンチジム
 * @author  Saito
 */
//======================================================================
#include "fieldmap.h"
#include "gym_anti_sv.h"
#include "gym_anti.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_anti.naix"

#include "../../../resource/fldmapdata/gimmick/gym_anti/gym_anti_local_def.h"


#define GYM_ANTI_UNIT_IDX (0)
#define GYM_ANTI_TMP_ASSIGN_ID  (1)

//ジム内部中の一時ワーク
typedef struct GYM_ANTI_TMP_tag
{
  u32 dummy;
}GYM_ANTI_TMP;

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ANTI_Setup(FIELDMAP_WORK *fieldWork)
{
  GYM_ANTI_SV_WORK *gmk_sv_work;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ANTI );
  }

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_ANTI_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_ANTI_TMP));

}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	    fieldWork   フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_ANTI_End(FIELDMAP_WORK *fieldWork)
{
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_ANTI_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork   フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ANTI_Move(FIELDMAP_WORK *fieldWork)
{
}
