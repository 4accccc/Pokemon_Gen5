//======================================================================
/**
 * @file  field_gimmick_d12.c
 * @brief  ビクティの島 ギミック
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "fieldmap.h" 
#include "field_gimmick_d12.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "system/gfl_use.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/arc_def.h"
#include "arc/fieldmap/gmk_d12.naix"
#include "field_gimmick_d12_sv.h"

#define EXPOBJ_UNIT_IDX (0)

#define OBJ_NUM (1)

#define SHIP_X  (FIELD_CONST_GRID_FX32_SIZE * 322 + (FIELD_CONST_GRID_FX32_SIZE/2))
#define SHIP_Y  (FX32_ONE * 8)
#define SHIP_Z  (FIELD_CONST_GRID_FX32_SIZE * 767 + (FIELD_CONST_GRID_FX32_SIZE/2))

//==========================================================================================
// ■3Dリソース
//==========================================================================================
// リソース
typedef enum {
  RES_SHIP_NSBMD,   //船のモデル
  RES_SHIP_NSBTA,   //船アニメ
} RES_INDEX;

static const GFL_G3D_UTIL_RES res_table[] = 
{
  { ARCID_GMK_D12, NARC_gmk_d12_c03_ship_03_nsbmd, GFL_G3D_UTIL_RESARC },  // 船のモデル
  { ARCID_GMK_D12, NARC_gmk_d12_c03_ship_01_nsbta, GFL_G3D_UTIL_RESARC },  // 船のアニメ
};

// オブジェクト
typedef enum {
  OBJ_SHIP,  // 船
} OBJ_INDEX;

//3Dアニメ
static const GFL_G3D_UTIL_ANM g3Dutil_anmTb[] = {
  { RES_SHIP_NSBTA,0 }, //アニメリソースID, アニメデータID(リソース内部INDEX)
};


static const GFL_G3D_UTIL_OBJ obj_table[OBJ_NUM] = 
{
  { RES_SHIP_NSBMD, 0, RES_SHIP_NSBMD, g3Dutil_anmTb, NELEMS(g3Dutil_anmTb) },  // 船
};

static const GFL_G3D_UTIL_SETUP Setup = {
  res_table,				//リソーステーブル
	NELEMS(res_table),		//リソース数
	obj_table,				//オブジェクト設定テーブル
	NELEMS(obj_table),		//オブジェクト数
};

//------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void D12_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  D12_SV_WORK *gmk_sv_work;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldmap ) );
    GIMMICKWORK *gmkwork = GAMEDATA_GetGimmickWork(gamedata);
    gmk_sv_work = GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_D12 );
  }

  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnitByHandle( exobj_cnt, &Setup, EXPOBJ_UNIT_IDX );

  {
    VecFx32 pos = { SHIP_X, SHIP_Y, SHIP_Z };
    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus(exobj_cnt, EXPOBJ_UNIT_IDX, 0);
    status->trans = pos;
    //カリングする
    FLD_EXP_OBJ_SetCulling(exobj_cnt, EXPOBJ_UNIT_IDX, 0, TRUE);
    //アニメ再生
    FLD_EXP_OBJ_ValidCntAnm(exobj_cnt, EXPOBJ_UNIT_IDX, 0, 0, TRUE);
    if (gmk_sv_work->Vanish)
    {
      FLD_EXP_OBJ_SetVanish(exobj_cnt, EXPOBJ_UNIT_IDX, 0, TRUE);
    }
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void D12_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  //ユニット破棄
  FLD_EXP_OBJ_DelUnit( exobj_cnt, EXPOBJ_UNIT_IDX );  
}

//------------------------------------------------------------------------------------------
/**
 * @brief 動作関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void D12_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  //アニメーション再生
  FLD_EXP_OBJ_PlayAnime( exobj_cnt );
}
