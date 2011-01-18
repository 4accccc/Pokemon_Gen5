//======================================================================
/**
 * @file  gym_insect.c
 * @brief  虫ジム
 * @author Miyuki Iwasawa 
 * @date  11.01.18
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

///=======================================================
//プロトタイプ
static void deb_Move( GYM_INSECT_TMP* wk, GYM_INSECT_SV_WORK* gmk_sv_work, GAMEDATA* gdata, FIELDMAP_WORK* fieldWork );

///=======================================================
//メイン関数

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
  GYM_INSECT_TMP *wk;
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldWork );
  {
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gdata);
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
  GYM_INSECT_TMP *wk = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);

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
  GYM_INSECT_TMP *wk = GMK_TMP_WK_GetWork(fieldWork, GYM_INSECT_TMP_ASSIGN_ID);
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldWork ) );
  {
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gdata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_GYM_INSECT );
  }

#ifdef PM_DEBUG
  deb_Move( wk, gmk_sv_work, gdata, fieldWork );
#endif  //PM_DEBUG

}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * 動作関数内デバッグ処理
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static void deb_Move( GYM_INSECT_TMP* wk, GYM_INSECT_SV_WORK* gmk_sv_work, GAMEDATA* gdata, FIELDMAP_WORK* fieldWork )
{
  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_L)
  {
    //自機の高さを変更
    {
      VecFx32 pos;
      FIELD_PLAYER *fld_player;
      fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      FIELD_PLAYER_GetPos( fld_player, &pos );
      if (GFL_UI_KEY_GetCont() & PAD_BUTTON_START){
        pos.y +=  10*FIELD_CONST_GRID_FX32_SIZE;
      }else{
        pos.y +=  FIELD_CONST_GRID_FX32_SIZE;
      }
      if (GFL_UI_KEY_GetCont() & PAD_BUTTON_B){
        pos.y *= -1;
      }
      FIELD_PLAYER_SetPos( fld_player, &pos );
    }
  }
}

#endif  //PM_DEBUG

