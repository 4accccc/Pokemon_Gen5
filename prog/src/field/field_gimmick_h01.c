#include <gflib.h>
#include "fieldmap.h"
#include "field_gimmick_h01.h"
#include "arc/fieldmap/buildmodel_outdoor.naix"
#include "savedata/gimmickwork.h"
#include "field_gimmick_def.h"


//======================================================================
/**
 * @breif 定数
 */
//======================================================================
//------------
// トレーラー
//------------ 
// トレーラの移動範囲
#define TRAILER_RAIL_Z_MIN (1200 << FX32_SHIFT)
#define TRAILER_RAIL_Z_MAX (4230 << FX32_SHIFT)

#define TRAILER_RAIL_X_01 (1756 << FX32_SHIFT)
#define TRAILER_RAIL_X_02 (1895 << FX32_SHIFT)
#define TRAILER_RAIL_Y    (490 << FX32_SHIFT) 

// トレーラーの移動速度
#define TRAILER_SPEED_MAX (8 << FX32_SHIFT)
#define TRAILER_SPEED_MIN (5 << FX32_SHIFT)

//-----
// 船
//-----
// 船の移動範囲
#define SHIP_RAIL_X_MIN (600 << FX32_SHIFT) 
#define SHIP_RAIL_X_MAX (3000 << FX32_SHIFT) 
#define SHIP_RAIL_Y     (-35 << FX32_SHIFT)
#define SHIP_RAIL_Z     (2386 << FX32_SHIFT)  

// 船の移動速度
#define SHIP_SPEED_MAX (3 << FX32_SHIFT)
#define SHIP_SPEED_MIN (1 << FX32_SHIFT)

//-----------------------
// ユニット・インデックス
//-----------------------
enum
{
  UNIT_IDX_TRAILER_01,
  UNIT_IDX_TRAILER_02,
  UNIT_IDX_SHIP,
  UNIT_IDX_MAX
};

//-----------
// GFL_UTIL
//-----------
// 3Dリソース
static const GFL_G3D_UTIL_RES res_table[] = 
{
  // トレーラー
  { ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_trailer_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_trailer_01_nsbmd, GFL_G3D_UTIL_RESARC },
  // 船
  { ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_h01_ship_nsbmd, GFL_G3D_UTIL_RESARC },
};

// 3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ obj_table[] = 
{
  // トレーラー01
  {
    0,    // モデルリソースID
    0,    // モデルデータID(リソース内部INDEX)
    0,    // テクスチャリソースID
    NULL, // アニメテーブル(複数指定のため)
    0,    // アニメリソース数
  },
  // トレーラー02
  {
    0,    // モデルリソースID
    0,    // モデルデータID(リソース内部INDEX)
    0,    // テクスチャリソースID
    NULL, // アニメテーブル(複数指定のため)
    0,    // アニメリソース数
  },
  // 船
  {
    0,    // モデルリソースID
    0,    // モデルデータID(リソース内部INDEX)
    0,    // テクスチャリソースID
    NULL, // アニメテーブル(複数指定のため)
    0,    // アニメリソース数
  },
};

// G3D_UTIL_SETUP
GFL_G3D_UTIL_SETUP setup0 = {
  &res_table[ UNIT_IDX_TRAILER_01 ], 1,
  &obj_table[ UNIT_IDX_TRAILER_01 ], 1, 
}; 

GFL_G3D_UTIL_SETUP setup1 = {
  &res_table[ UNIT_IDX_TRAILER_02 ], 1,
  &obj_table[ UNIT_IDX_TRAILER_02 ], 1, 
}; 
GFL_G3D_UTIL_SETUP setup2 = {
  &res_table[ UNIT_IDX_SHIP ], 1,
  &obj_table[ UNIT_IDX_SHIP ], 1, 
}; 

//======================================================================
/**
 * @brief ギミック登録関数
 */
//======================================================================

//--------------------------------------------------------------------
/**
 * @brief セットアップ関数
 */
//--------------------------------------------------------------------
void H01_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldmap ); 
  GAMESYS_WORK*      gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*      gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK*   sv = GAMEDATA_GetSaveControlWork( gamedata );
  GIMMICKWORK*    gmkwork = SaveData_GetGimmickWork( sv );
  fx32              pos[] = { TRAILER_RAIL_Z_MIN, TRAILER_RAIL_Z_MAX, SHIP_RAIL_X_MIN };

  // セーブデータをチェック
  if( GIMMICKWORK_GetAssignID( gmkwork ) == FLD_GIMMICK_H01 )
  {
    int work_adrs = (int)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
    pos[0] = *( (fx32*)( work_adrs + 0 ) );
    pos[1] = *( (fx32*)( work_adrs + 4 ) );
    pos[2] = *( (fx32*)( work_adrs + 8 ) );
  }

  // トレーラー01
  {
    GFL_G3D_OBJSTATUS status;
    VEC_Set( &status.trans, TRAILER_RAIL_X_01, TRAILER_RAIL_Y, pos[0] );
    VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
    MTX_RotY33( &status.rotate, FX_SinIdx( 32768 ), FX_CosIdx( 32768 ) );	  // y軸180度回転
    FLD_EXP_OBJ_AddUnit( ptr, &setup0, &status, UNIT_IDX_TRAILER_01 );
  } 
  // トレーラー02
  {
    GFL_G3D_OBJSTATUS status;
    VEC_Set( &status.trans, TRAILER_RAIL_X_02, TRAILER_RAIL_Y, pos[1] );
    VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
    MTX_Identity33( &status.rotate );
    FLD_EXP_OBJ_AddUnit( ptr, &setup1, &status, UNIT_IDX_TRAILER_02 );
  }
  // 船
  {
    GFL_G3D_OBJSTATUS status;
    VEC_Set( &status.trans, pos[2], SHIP_RAIL_Y, SHIP_RAIL_Z );
    VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
    MTX_Identity33( &status.rotate );
    FLD_EXP_OBJ_AddUnit( ptr, &setup2, &status, UNIT_IDX_SHIP );
  }
}

//--------------------------------------------------------------------
/**
 * @brief 終了関数
 */
//--------------------------------------------------------------------
void H01_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{

  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldmap ); 
  GAMESYS_WORK*      gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*      gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK*   sv = GAMEDATA_GetSaveControlWork( gamedata );
  GIMMICKWORK*    gmkwork = SaveData_GetGimmickWork( sv );
  int           work_adrs = 0;
  int           data_size = 0;

  // 仕掛け用ワークを初期化
  work_adrs = (int)GIMMICKWORK_Assign( gmkwork, FLD_GIMMICK_H01 );

  // データを保存
  {
    GFL_G3D_OBJSTATUS* status            = FLD_EXP_OBJ_GetUnitObjStatus( ptr, UNIT_IDX_TRAILER_01 );
    *( (fx32*)(work_adrs + data_size) )  = status->trans.z;
    data_size                           += sizeof( fx32 );
  }
  {
    GFL_G3D_OBJSTATUS* status            = FLD_EXP_OBJ_GetUnitObjStatus( ptr, UNIT_IDX_TRAILER_02 );
    *( (fx32*)(work_adrs + data_size) )  = status->trans.z;
    data_size                           += sizeof( fx32 );
  }
  {
    GFL_G3D_OBJSTATUS* status            = FLD_EXP_OBJ_GetUnitObjStatus( ptr, UNIT_IDX_SHIP );
    *( (fx32*)(work_adrs + data_size) )  = status->trans.x;
    data_size                           += sizeof( fx32 );
  } 
} 

//--------------------------------------------------------------------
/**
 * @brief 動作関数
 */
//--------------------------------------------------------------------
void H01_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldmap ); 

  // トレーラー01の動作
  { 
    GFL_G3D_OBJSTATUS* p_status = FLD_EXP_OBJ_GetUnitObjStatus( ptr, UNIT_IDX_TRAILER_01 );
    p_status->trans.z += TRAILER_SPEED_MAX;
    if( TRAILER_RAIL_Z_MAX < p_status->trans.z )
    {
      p_status->trans.z = TRAILER_RAIL_Z_MIN;
    }
  }
  // トレーラー02の動作
  { 
    GFL_G3D_OBJSTATUS* p_status = FLD_EXP_OBJ_GetUnitObjStatus( ptr, UNIT_IDX_TRAILER_02 );
    p_status->trans.z -= TRAILER_SPEED_MAX;
    if( p_status->trans.z < TRAILER_RAIL_Z_MIN )
    {
      p_status->trans.z = TRAILER_RAIL_Z_MAX;
    }
  }
  // 船の動作
  { 
    GFL_G3D_OBJSTATUS* p_status = FLD_EXP_OBJ_GetUnitObjStatus( ptr, UNIT_IDX_SHIP );
    p_status->trans.x += SHIP_SPEED_MAX;
    if( SHIP_RAIL_X_MAX < p_status->trans.x )
    {
      p_status->trans.x = SHIP_RAIL_X_MIN;
    }
  }
}
