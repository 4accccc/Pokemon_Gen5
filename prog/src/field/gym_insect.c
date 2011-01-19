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
#include "map_attr.h"
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

//フロア矩形
typedef struct FLOOR_RECT_tag
{
  u8 x;
  u8 z;
  u8 w;
  u8 h;
  fx32 height;
}FLOOR_RECT;

//ジム内部中の一時ワーク
typedef struct GYM_INSECT_TMP_tag
{
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  GIMMICKWORK* gmkwork;
  GYM_INSECT_SV_WORK* gmk_sv;

  FLD_EXP_OBJ_CNT_PTR exp_obj;
  FIELD_PLAYER *f_player;

}GYM_INSECT_TMP;


#include "gym_insect.cdat"

///=======================================================
//プロトタイプ
static void sub_SetExHeight( FIELDMAP_WORK* fieldWork );

#ifdef PM_DEBUG
static void deb_Move( GYM_INSECT_TMP* wk, FIELDMAP_WORK* fieldWork );
#endif  //PM_DEBUG

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
  GYM_INSECT_TMP *wk;

  //汎用ワーク確保
  wk = GMK_TMP_WK_AllocWork
      (fieldWork, GYM_INSECT_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldWork), sizeof(GYM_INSECT_TMP));
 
  wk->gsys = FIELDMAP_GetGameSysWork( fieldWork );
  wk->gdata = GAMESYSTEM_GetGameData( wk->gsys );
  wk->gmkwork = GAMEDATA_GetGimmickWork( wk->gdata );
  wk->gmk_sv = GIMMICKWORK_Get( wk->gmkwork, FLD_GIMMICK_GYM_INSECT );

  wk->exp_obj = FIELDMAP_GetExpObjCntPtr( fieldWork );
  wk->f_player = FIELDMAP_GetFieldPlayer( fieldWork );

  //拡張高さセット
  sub_SetExHeight( fieldWork );
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

#ifdef PM_DEBUG
  deb_Move( wk, fieldWork );
#endif  //PM_DEBUG

}



//===============================================================
///サブルーチン
//===============================================================
static void sub_SetExHeight( FIELDMAP_WORK* fieldWork )
{
  int i;
  FLDMAPPER* mapper = FIELDMAP_GetFieldG3Dmapper( fieldWork );
  EHL_PTR exHeightPtr = FLDMAPPER_GetExHegihtPtr( mapper );

  for(i = 0;i < FLOOR_RECT_NUM;i++){
    EXH_SetUpHeightData( i,
      DATA_FloorRect[i].x, DATA_FloorRect[i].z,
      DATA_FloorRect[i].w, DATA_FloorRect[i].h,
			DATA_FloorRect[i].height, FLOOR_RECT_ATTR, exHeightPtr );
  }
}



//===============================================================
///デバッグルーチン
//===============================================================
#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * 動作関数内デバッグ処理
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
static void deb_Move( GYM_INSECT_TMP* wk, FIELDMAP_WORK* fieldWork )
{
  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_L)
  {
    //自機の高さを変更
    {
      VecFx32 pos;
      FIELD_PLAYER_GetPos( wk->f_player, &pos );
      if (GFL_UI_KEY_GetCont() & PAD_BUTTON_START){
        if (GFL_UI_KEY_GetCont() & PAD_BUTTON_B){
          pos.y -=  10*FIELD_CONST_GRID_FX32_SIZE;
        }else{
          pos.y +=  10*FIELD_CONST_GRID_FX32_SIZE;
        }
      }else{
        if (GFL_UI_KEY_GetCont() & PAD_BUTTON_B){
          pos.y -=  FIELD_CONST_GRID_FX32_SIZE;
        }else{
          pos.y +=  FIELD_CONST_GRID_FX32_SIZE;
        }
      }
      FIELD_PLAYER_SetPos( wk->f_player, &pos );
    }
  }
}

#endif  //PM_DEBUG

