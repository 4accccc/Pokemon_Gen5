//======================================================================
/**
 * @file  field_gimmick_r04d03.c
 * @brief  4番道路+リゾートデザート ギミック
 * @author  Saito
 */
//======================================================================

#include <gflib.h>
#include "fieldmap.h" 
#include "field_gimmick_r04d03.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "sound/pm_sndsys.h"
#include "system/gfl_use.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "arc/arc_def.h"
#include "arc/h01.naix"
#include "gmk_tmp_wk.h"

#include "field_gimmick_r04d03_se_def.h"

#include "../../../resource/fldmapdata/gimmick/r04d03/gimmick_se_range_def.h"

#define EXPOBJ_UNIT_IDX (0)
#define ARCID (ARCID_H01_GIMMICK) // ギミックデータのアーカイブID
#define R04D03_TMP_ASSIGN_ID  (1)

#define TRAILER_TAIL_OFS (6*FIELD_CONST_GRID_FX32_SIZE)
#define TRAILER_Y (FIELD_CONST_GRID_FX32_SIZE*5)
#define TRAILER_RIGHT_X (454*FIELD_CONST_GRID_FX32_SIZE)
#define TRAILER_LEFT_X (375*FIELD_CONST_GRID_FX32_SIZE)
#define TRAILER_TAIL_RIGHT_X  (TRAILER_RIGHT_X+TRAILER_TAIL_OFS)
#define TRAILER_TAIL_LEFT_X   (TRAILER_LEFT_X-TRAILER_TAIL_OFS)


#define TRAILER1_Z ((526+2)*FIELD_CONST_GRID_FX32_SIZE)
#define TRAILER2_Z ((521+2)*FIELD_CONST_GRID_FX32_SIZE)


#define TRAILER_SPEED (FIELD_CONST_GRID_FX32_SIZE)

#define TRAILER_MAX (2)

#define SE_BAND_Z_MIN ( SE_BAND_Z_MIN_GRID * FIELD_CONST_GRID_FX32_SIZE )   //SEの聞こえる幅Ｚ最小値
#define SE_BAND_Z_MAX ( SE_BAND_Z_MAX_GRID * FIELD_CONST_GRID_FX32_SIZE )   //SEの聞こえる幅Ｚ最大値
#define SE_RANGE_X ( SE_RANGE_X_GRID * FIELD_CONST_GRID_FX32_SIZE )      //SEの聞こえる距離



//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct GNK_OBJ_tag
{
  int Frame;
  int Wait;
  BOOL SeFlg;
}GMK_OBJ;

typedef struct
{ 
  int Frame[TRAILER_MAX];
  GMK_OBJ GmkObj[TRAILER_MAX];
} GMK_WORK;

//==========================================================================================
// ■3Dリソース
//==========================================================================================
// リソース
typedef enum {
  RES_TRAILER_HEAD_NSBMD,  // トレーラー(前)のモデル
  RES_TRAILER_TAIL_NSBMD,  // トレーラー(後)のモデル
  RES_NUM
} RES_INDEX;

static const GFL_G3D_UTIL_RES res_table[RES_NUM] = 
{
  { ARCID, NARC_h01_trailer_01a_nsbmd, GFL_G3D_UTIL_RESARC },  // トレーラー(前)のモデル
  { ARCID, NARC_h01_trailer_01b_nsbmd, GFL_G3D_UTIL_RESARC },  // トレーラー(後)のモデル
};

// オブジェクト
typedef enum {
  OBJ_TRAILER_1_HEAD,  // トレーラー1(前) 左から右
  OBJ_TRAILER_1_TAIL,  // トレーラー1(後) 左から右
  OBJ_TRAILER_2_HEAD,  // トレーラー2(前) 右から左
  OBJ_TRAILER_2_TAIL,  // トレーラー2(後) 右から左
  OBJ_NUM
} OBJ_INDEX;

static const GFL_G3D_UTIL_OBJ obj_table[OBJ_NUM] = 
{
  { RES_TRAILER_HEAD_NSBMD, 0, RES_TRAILER_HEAD_NSBMD, NULL, 0 },  // トレーラー1(前)
  { RES_TRAILER_TAIL_NSBMD, 0, RES_TRAILER_TAIL_NSBMD, NULL, 0 },  // トレーラー1(後)
  { RES_TRAILER_HEAD_NSBMD, 0, RES_TRAILER_HEAD_NSBMD, NULL, 0 },  // トレーラー2(前)
  { RES_TRAILER_TAIL_NSBMD, 0, RES_TRAILER_TAIL_NSBMD, NULL, 0 },  // トレーラー2(後)
};

// セットアップ情報
static GFL_G3D_UTIL_SETUP setup = { res_table, RES_NUM, obj_table, OBJ_NUM };


static void InitWork( GMK_WORK* work, FIELDMAP_WORK* fieldmap );

//------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void R04D03_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  u32* gmk_save;  // ギミックの実セーブデータ
  GMK_WORK* work;  // ギミック管理ワーク
  HEAPID                heapID = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  GAMESYS_WORK*            gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*               gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*          gmkwork = GAMEDATA_GetGimmickWork(gdata);

  //汎用ワーク確保
  GMK_TMP_WK_AllocWork
      (fieldmap, R04D03_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(fieldmap), sizeof(GMK_WORK));
  work = GMK_TMP_WK_GetWork(fieldmap, R04D03_TMP_ASSIGN_ID);
  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &setup, EXPOBJ_UNIT_IDX );

  // ギミック管理ワークを初期化 
  InitWork( work, fieldmap );
/**
  // ロード
  LoadGimmick( work, fieldmap );

  // ギミック管理ワークのアドレスを保存
  gmk_save    = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  gmk_save[0] = (int)work;
*/
  // DEBUG:
  NOZOMU_Printf( "GIMMICK: set up\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void R04D03_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
/**
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  GMK_WORK*         work = (GMK_WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス
  // セーブ
  SaveGimmick( work, fieldmap );

*/
  //ユニット破棄
  FLD_EXP_OBJ_DelUnit( exobj_cnt, EXPOBJ_UNIT_IDX );  
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldmap, R04D03_TMP_ASSIGN_ID);
  // DEBUG:
  NOZOMU_Printf( "GIMMICK: end\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 動作関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void R04D03_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  int i;
  int *frame;
  VecFx32 pos;
  fx32 tr1_x, tr2_x;

  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  GMK_WORK* work = GMK_TMP_WK_GetWork(fieldmap, R04D03_TMP_ASSIGN_ID);
  GMK_OBJ *gmkobj;

  gmkobj = &work->GmkObj[0];
  gmkobj->Frame++;
  frame = &gmkobj->Frame;
  { // トレーラー1(前)
    GFL_G3D_OBJSTATUS* status;
    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_1_HEAD );
    status->trans.x = TRAILER_LEFT_X + TRAILER_SPEED * (*frame);
    if ( status->trans.x >= TRAILER_RIGHT_X)
    {
      (*frame) = 0;
      work->GmkObj[0].SeFlg = FALSE;
    }
    tr1_x = status->trans.x;
  }
  { // トレーラー1(後)
    GFL_G3D_OBJSTATUS* status;
    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_1_TAIL );
    status->trans.x = TRAILER_TAIL_LEFT_X + TRAILER_SPEED * (*frame);
    if ( status->trans.x >= TRAILER_RIGHT_X) (*frame) = 0;
  }

  gmkobj = &work->GmkObj[1];
  gmkobj->Frame++;
  frame = &gmkobj->Frame;
  { // トレーラー2(前)
    GFL_G3D_OBJSTATUS* status;
    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_2_HEAD );
    status->trans.x = TRAILER_RIGHT_X - TRAILER_SPEED * (*frame);
    if ( status->trans.x <= TRAILER_LEFT_X)
    {
      (*frame) = 0;
      work->GmkObj[1].SeFlg = FALSE;
    }
    tr2_x = status->trans.x;
  }
  { // トレーラー2(後)
    GFL_G3D_OBJSTATUS* status;
    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_2_TAIL );
    status->trans.x = TRAILER_TAIL_RIGHT_X - TRAILER_SPEED * (*frame);
    if ( status->trans.x <= TRAILER_LEFT_X) (*frame) = 0;
  }

  //自機の位置座表を取得
  {
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
    FIELD_PLAYER_GetPos( fld_player, &pos );
  }

  //SEを鳴らすＺバンド幅に自機がいるか？
  if ( (SE_BAND_Z_MIN<=pos.z)&&(pos.z<=SE_BAND_Z_MAX) )      //いる場合
  {
    SEPLAYER_ID p_id;
    fx32 diff;
    FIELD_SOUND *fs = GAMEDATA_GetFieldSound( gdata );
    diff = pos.x - tr1_x;
    //一台目のトレーラーが自機よりも右にいて、なおかつ、ＳＥ再生距離にいてまだＳＥならしてないか？
    if( (0 <= diff)&&(diff <= SE_RANGE_X)&&(!work->GmkObj[0].SeFlg) )   //YES
    {
      //プレーヤー空きチェック
      p_id = PMSND_GetSE_DefaultPlayerID( FLD_GMK_R04D03_LR_SE );
      if ( !PMSND_CheckPlaySE_byPlayerID( p_id ) )
      {
        work->GmkObj[0].SeFlg = TRUE;
        FSND_PlayEnvSE( fs, FLD_GMK_R04D03_LR_SE );    //左から右
      }
    }
    diff = tr2_x - pos.x;
    //二台目のトレーラーが自機よりも左にいて、なおかつ、ＳＥ再生距離にいてまだＳＥならしてないか？
    if( (0 <= diff)&&(diff <= SE_RANGE_X)&&(!work->GmkObj[1].SeFlg) )   //YES
    {
      //プレーヤー空きチェック
      p_id = PMSND_GetSE_DefaultPlayerID( FLD_GMK_R04D03_RL_SE );
      if ( !PMSND_CheckPlaySE_byPlayerID( p_id ) )
      {
        work->GmkObj[1].SeFlg = TRUE;
        FSND_PlayEnvSE( fs, FLD_GMK_R04D03_RL_SE );    //右から左
      }
    }
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミック管理ワークを初期化する
 *
 * @param work     初期化対象ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void InitWork( GMK_WORK* work, FIELDMAP_WORK* fieldmap )
{
  int i;
  HEAPID                heapID = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  // オブジェクトを作成
  { // トレーラー1(前)
    GFL_G3D_OBJSTATUS* status;
    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_1_HEAD );
    status->trans.x = TRAILER_LEFT_X;
    status->trans.y = TRAILER_Y;
    status->trans.z = TRAILER1_Z;
    //90°回転
    {
      fx32 sin = FX_SinIdx(0x4000);
      fx32 cos = FX_CosIdx(0x4000);
      MTX_RotY33( &status->rotate, sin, cos );
    }
    //カリングする
    FLD_EXP_OBJ_SetCulling(exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_1_HEAD, TRUE);
  }  
  { // トレーラー1(後)
    GFL_G3D_OBJSTATUS* status;
    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_1_TAIL );
    status->trans.x = TRAILER_TAIL_LEFT_X;
    status->trans.y = TRAILER_Y;
    status->trans.z = TRAILER1_Z;
    //90°回転
    {
      fx32 sin = FX_SinIdx(0x4000);
      fx32 cos = FX_CosIdx(0x4000);
      MTX_RotY33( &status->rotate, sin, cos );
    }
    //カリングする
    FLD_EXP_OBJ_SetCulling(exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_1_TAIL, TRUE);
  }
  { // トレーラー2(前)
    GFL_G3D_OBJSTATUS* status;
    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_2_HEAD );
    status->trans.x = TRAILER_RIGHT_X;
    status->trans.y = TRAILER_Y;
    status->trans.z = TRAILER2_Z;
    //270°回転
    {
      fx32 sin = FX_SinIdx(0x4000*3);
      fx32 cos = FX_CosIdx(0x4000*3);
      MTX_RotY33( &status->rotate, sin, cos );
    }
    //カリングする
    FLD_EXP_OBJ_SetCulling(exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_2_HEAD, TRUE);
  }  
  { // トレーラー2(後)
    GFL_G3D_OBJSTATUS* status;
    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_2_TAIL );
    status->trans.x = TRAILER_TAIL_RIGHT_X;
    status->trans.y = TRAILER_Y;
    status->trans.z = TRAILER2_Z;
    //270°回転
    {
      fx32 sin = FX_SinIdx(0x4000*3);
      fx32 cos = FX_CosIdx(0x4000*3);
      MTX_RotY33( &status->rotate, sin, cos );
    }
    //カリングする
    FLD_EXP_OBJ_SetCulling(exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TRAILER_2_TAIL, TRUE);
  }
}

