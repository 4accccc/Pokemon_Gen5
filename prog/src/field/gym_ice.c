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

//#include "../../../resource/fldmapdata/gimmick/gym_ground/gym_ice_local_def.h"

//#include "sound/pm_sndsys.h"
//#include "gym_ice_se_def.h"

#define GYM_ICE_UNIT_IDX (0)
#define GYM_ICE_TMP_ASSIGN_ID  (1)

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define WALL_NUM_MAX    (3)
#define WALL_ANM_NUM  (1)

//ジム内部中の一時ワーク
typedef struct GYM_ICE_TMP_tag
{
  int dummy;
}GYM_ICE_TMP;

static void SetupWallAnm(GYM_ICE_SV_WORK *gmk_sv_work, FLD_EXP_OBJ_CNT_PTR ptr, const int inWallIdx);

//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_ICE_Setup(FIELDMAP_WORK *fieldWork)
{
  int i;
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

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit(ptr, &Setup, GYM_ICE_UNIT_IDX );

  //座標セット
  for (i=0;i<WALL_NUM_MAX;i++)
  {
    int j;
    int idx = OBJ_WALL_1 + i;
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(ptr, GYM_ICE_UNIT_IDX, idx);
    status->trans = WallPos[i];

    //カリングする
    FLD_EXP_OBJ_SetCulling(ptr, GYM_ICE_UNIT_IDX, idx, TRUE);

    for (j=0;j<WALL_ANM_NUM;j++)
    {
      //1回再生設定
      EXP_OBJ_ANM_CNT_PTR anm;
      anm = FLD_EXP_OBJ_GetAnmCnt( ptr, GYM_ICE_UNIT_IDX, idx, j);
      FLD_EXP_OBJ_ChgAnmLoopFlg(anm, 0);
      //アニメ停止
      FLD_EXP_OBJ_ChgAnmStopFlg(anm, 1);
    }
    
    SetupWallAnm(gmk_sv_work, ptr, i);
  }
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
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ICE_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GYM_ICE_TMP_ASSIGN_ID);
  //ＯＢＪ解放
  FLD_EXP_OBJ_DelUnit( ptr, GYM_ICE_UNIT_IDX );
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
  int i;
  GYM_ICE_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GYM_ICE_TMP *tmp = GMK_TMP_WK_GetWork(fieldWork, GYM_GROUND_TMP_ASSIGN_ID);
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_ICE );
  }
  
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( ptr );
}


//--------------------------------------------------------------
/**
 * 壁アニメ準備
 * @param   gmk_sv_work   ギミックセーブワーク
 * @param   ptr       拡張ＯＢＪコントロールポインタ
 * @param	  inWallIdx 壁インデックス
 * @return  none
 */
//--------------------------------------------------------------
static void SetupWallAnm(GYM_ICE_SV_WORK *gmk_sv_work, FLD_EXP_OBJ_CNT_PTR ptr, const int inWallIdx)
{
}


