//======================================================================
/**
 * @file  gym_ice.c
 * @brief  氷ジム
 * @author  Saito
 * @date  09.11.26
 */
//======================================================================

#include "fieldmap.h"
#include "gym_ice_sv.h"
#include "gym_ice.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/fieldmap/gym_ice.naix"
#include "system/main.h"    //for HEAPID_FIELDMAP

#include "../../../resource/fldmapdata/gimmick/gym_ice/gym_ice_local_def.h"

#include "sound/pm_sndsys.h"
#include "gym_ice_se_def.h"

#include "field/fieldmap_proc.h"    //for FLDMAP_CTRLTYPE
#include "fieldmap_ctrl_hybrid.h" //for FIELDMAP_CTRL_HYBRID

#define GYM_ICE_UNIT_IDX (0)
#define GYM_ICE_TMP_ASSIGN_ID  (1)

//ジム内部中の一時ワーク
typedef struct GYM_ICE_TMP_tag
{
  int dummy;
}GYM_ICE_TMP;

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ICE_Setup(FIELDMAP_WORK *fieldWork)
{
  GYM_ICE_SV_WORK *gmk_sv_work;
  GYM_ICE_TMP *tmp;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ICE );
  }

  //汎用ワーク確保
  tmp = GMK_TMP_WK_AllocWork
      (fieldWork, GYM_ICE_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_ICE_TMP));

}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ICE_End(FIELDMAP_WORK *fieldWork)
{
  GYM_ICE_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ICE_Move(FIELDMAP_WORK *fieldWork)
{
  GYM_ICE_SV_WORK *gmk_sv_work;
  GYM_ICE_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ICE );
  }
}
