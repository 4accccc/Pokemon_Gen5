//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		infinity_dungeon_gimmick.c
 *	@brief  無限ダンジョン用ギミック
 *	@author	Masayuki Onoue
 *	@date		2011.03.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "fieldmap.h"
#include "savedata/gimmickwork.h"
#include "infinity_dungeon_gimmick.h"
#include "gmk_tmp_wk.h"
#include "arc/fieldmap/infinity_dungeon_gmk.naix" 
#include "field_g3d_mapper.h"
#include "fieldmap/zone_id.h"
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

// ギミックワークのアサインID
#define GIMMICK_WORK_ASSIGN_ID (0)

// モデルの登録インデックス
#define MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID (0)

//-------------------------------------
///	モデル管理
//=====================================
//モデルID
typedef enum
{
  MODELID_ELEVATOR_1,
  MODELID_ELEVATOR_2,
  MODELID_MAX,
}MODELID;

//-------------------------------------
// エレベータ配置データ
//=====================================
typedef struct {
  u16       zone_id;
  u16       obj_id;
  s32 gx;
  s32 gy;
  s32 gz;
}ELEVATOR_SET_DATA;

#define ELEVATOR_DATA_TABLE_SIZE (6)
//-------------------------------------
// エレベータの場所テーブル
//=====================================
static const ELEVATOR_SET_DATA sc_elevator_pos_table[ELEVATOR_DATA_TABLE_SIZE] = {
  {
    ZONE_ID_D31R0201, // 受付（ブラック）
    MODELID_ELEVATOR_1,
    7 , 0 , 0,
  },
  {
    ZONE_ID_D31R0201, // 受付（ブラック）
    MODELID_ELEVATOR_2,
    11 , 0 , 0,
  },
  {
    ZONE_ID_D31R0401, // エレベーター部屋（ブラック）
    MODELID_ELEVATOR_1,
    6 , 0 , 1,
  },
  {
    ZONE_ID_D31R0101, // 受付（ホワイト）
    MODELID_ELEVATOR_1,
    7 , 0 , 0,
  },
  {
    ZONE_ID_D31R0101, // 受付（ホワイト）
    MODELID_ELEVATOR_2,
    11 , 0 , 0,
  },
  {
    ZONE_ID_D31R0301, // エレベーター部屋（ホワイト）
    MODELID_ELEVATOR_1,
    6 , 0 , 1,
  },
};

//-------------------------------------
///	ワーク
//=====================================

//リソースID
enum
{
  MODEL_RESID_ELEVATOR,       // エレベータ用モデル
  MODEL_RESID_ELEVATOR_ICA,   // エレベータアニメ
  MODEL_RESID_MAX,
};

//リソースデータ
static const GFL_G3D_UTIL_RES sc_g3d_util_res[MODEL_RESID_MAX] =
{
  // MODEL_RESID_ELEVATOR->エレベータデータ
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_nsbmd,
    GFL_G3D_UTIL_RESARC,
  },
  //MODEL_RESID_ELEVATOR_TA->エレベータアニメーション
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_nsbta, 
    GFL_G3D_UTIL_RESARC
  },
};

// アニメ設定
#define ANIMEID_ELEVATOR_MAX  (1)
static const GFL_G3D_UTIL_ANM sc_g3d_elevator_anm[ANIMEID_ELEVATOR_MAX] = 
{
  { MODEL_RESID_ELEVATOR_ICA, 0 }
};

//3Dオブジェクト設定
static const GFL_G3D_UTIL_OBJ sc_g3d_util_obj[MODELID_MAX] =
{
    // エレベータ
  {
    MODEL_RESID_ELEVATOR, 	//モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_ELEVATOR, 	//テクスチャリソースID
		sc_g3d_elevator_anm,			    //アニメテーブル(複数指定のため)
		ANIMEID_ELEVATOR_MAX,	        //アニメリソース数
  },
  // エレベータ
  {
    MODEL_RESID_ELEVATOR, 	//モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_ELEVATOR, 	//テクスチャリソースID
		sc_g3d_elevator_anm,			    //アニメテーブル(複数指定のため)
		ANIMEID_ELEVATOR_MAX,	        //アニメリソース数
  },
};

//全情報設定
static const GFL_G3D_UTIL_SETUP sc_g3d_util_setup =
{
  sc_g3d_util_res,  //リソーステーブル
  MODEL_RESID_MAX,  //リソース最大数
  sc_g3d_util_obj,  //オブジェクトテーブル
  MODELID_MAX,      //オブジェクト数
};


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	モデルワーク
//=====================================
typedef struct
{
  s32                 unitID;
  MODELID             modelID;
  EXP_OBJ_ANM_CNT_PTR p_anm_cnt;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
} MODEL;

//-------------------------------------
///	無限ダンジョンギミックワーク
//=====================================
typedef struct {
  u32  zone_id;
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
  FIELDMAP_WORK       *p_fieldmap;
  MODEL               model[MODELID_MAX]; // モデル
} INFINITY_DUNGEON_GIMMICK_WK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void WorkDataInit( INFINITY_DUNGEON_GIMMICK_WK *p_wk ,FIELDMAP_WORK *p_fieldmap );
static void MapModelInit( INFINITY_DUNGEON_GIMMICK_WK *p_wk );
static const ELEVATOR_SET_DATA* GetElevatorTable( INFINITY_DUNGEON_GIMMICK_WK *p_wk );
static u32 GetDungeonColor( INFINITY_DUNGEON_GIMMICK_WK *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンギミック初期化
 *
 *	@param	*p_fieldmap 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_Setup( FIELDMAP_WORK *p_fieldmap )
{
  INFINITY_DUNGEON_GIMMICK_WK *p_wk = NULL;
  const HEAPID heapID = FIELDMAP_GetHeapID( p_fieldmap );

  //汎用ワーク確保
  p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, GIMMICK_WORK_ASSIGN_ID, 
      heapID, sizeof(INFINITY_DUNGEON_GIMMICK_WK));

  // ワークの初期化
  WorkDataInit( p_wk , p_fieldmap );
  
  // モデルの読み込み
  MapModelInit( p_wk );

}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンギミック終了処理
 *
 *	@param	*p_fieldmap 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_End( FIELDMAP_WORK *p_fieldmap )
{
  INFINITY_DUNGEON_GIMMICK_WK *p_wk = NULL;
  p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GIMMICK_WORK_ASSIGN_ID);

  //破棄
  FLD_EXP_OBJ_DelUnit( p_wk->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID );
  
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, GIMMICK_WORK_ASSIGN_ID);
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンメイン処理
 *
 *	@param	*p_fieldmap 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_Move( FIELDMAP_WORK *p_fieldmap )
{
  INFINITY_DUNGEON_GIMMICK_WK *p_wk = NULL;
  p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GIMMICK_WORK_ASSIGN_ID);

  //通常のギミックモデル
  FLD_EXP_OBJ_PlayAnime( p_wk->p_ex_obj );
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップ上のモデルを初期化する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MapModelInit( INFINITY_DUNGEON_GIMMICK_WK *p_wk )
{
  //読込み
  FLD_EXP_OBJ_AddUnitByHandle(p_wk->p_ex_obj, &sc_g3d_util_setup, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID );
  
  //モデル初期化
  {
    int i;
    for( i = 0; i < MODELID_MAX; i++ )
    {
      MODEL *p_model    = &p_wk->model[i];
      p_model->unitID   = MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID;
      p_model->modelID  = i;
      p_model->p_ex_obj = p_wk->p_ex_obj;
      FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,  p_model->unitID, p_model->modelID, TRUE );
    }
  }

  {
    int i;
    const ELEVATOR_SET_DATA *p_elevator_table = GetElevatorTable( p_wk );
    for( i = 0 ; i < ELEVATOR_DATA_TABLE_SIZE ; i++ )
    {
      GFL_G3D_OBJSTATUS *p_status;
      const ELEVATOR_SET_DATA *p_data = &p_elevator_table[i];
      MODEL *p_model    = &p_wk->model[p_data->obj_id];
      if( p_wk->zone_id == p_data->zone_id )
      {
        EXP_OBJ_ANM_CNT_PTR exobj_anm;

        // ステータス取得
        p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
            MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID,
            p_model->modelID );

        // 位置を設定
        p_status->trans.x = FX32_CONST(p_data->gx*FIELD_CONST_GRID_SIZE);
        p_status->trans.y = FX32_CONST(p_data->gy*FIELD_CONST_GRID_SIZE);
        p_status->trans.z = FX32_CONST(p_data->gz*FIELD_CONST_GRID_SIZE);

        // 見える状態に
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
            MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, FALSE );
        
        // フレームの初期化
        FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, 0, TRUE);

        // アニメーション取得
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, 0 );

        // ループ再生
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, TRUE );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワークを初期化する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void WorkDataInit( INFINITY_DUNGEON_GIMMICK_WK *p_wk ,FIELDMAP_WORK *p_fieldmap )
{
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->zone_id     = FIELDMAP_GetZoneID( p_wk->p_fieldmap );
  p_wk->p_ex_obj    = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のダンジョンバージョンを返す
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u32 GetDungeonColor( INFINITY_DUNGEON_GIMMICK_WK *p_wk )
{
  return GET_VERSION();
}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベータテーブルを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const ELEVATOR_SET_DATA* GetElevatorTable( INFINITY_DUNGEON_GIMMICK_WK *p_wk )
{
  return sc_elevator_pos_table;
}

