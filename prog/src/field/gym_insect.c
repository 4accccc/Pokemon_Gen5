//======================================================================
/**
 * @file  gym_insect.c
 * @brief  虫ジム
 * @author  Saito
 * @date  09.11.05
 */
//======================================================================

#include "fieldmap.h"
#include "gym_insect_sv.h"
#include "gym_insect.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_insect.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "script.h"     //for SCRIPT_CallScript
#include "../../../resource/fldmapdata/script/c03gym0101_def.h"  //for SCRID_～

#include "../../../resource/fldmapdata/gimmick/gym_insect/gym_insect_local_def.h"

#include "sound/pm_sndsys.h"
#include "gym_insect_se_def.h"

#include "event_fldmmdl_control.h"  //for EVENT_ObjAnime

#define GYM_INSECT_UNIT_IDX (0)
#define GYM_INSECT_TMP_ASSIGN_ID  (1)

//ジム内部中の一時ワーク
typedef struct GYM_INSECT_TMP_tag
{
  u32 dummy;
}GYM_INSECT_TMP;

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
  GYM_INSECT_SV_WORK *gmk_sv_work;
  GYM_INSECT_TMP *tmp;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldWork, GYM_INSECT_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_INSECT_TMP));
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_End(FIELDMAP_WORK *fieldWork)
{
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_INSECT_Move(FIELDMAP_WORK *fieldWork)
{
  GYM_INSECT_SV_WORK *gmk_sv_work;
  GYM_INSECT_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }
}
