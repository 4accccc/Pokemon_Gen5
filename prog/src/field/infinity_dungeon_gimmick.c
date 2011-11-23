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
#include "fldmmdl.h"
#include "savedata/gimmickwork.h"
#include "savedata/infinity_dungeon_save.h"
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

// モデルの登録インデックス
#define MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID (0)
#define ELEVATOR_NUM (2)  // エレベータの設置数
#define BLACK_INFO_ZONE_ID (ZONE_ID_D31R0201) // ブラックの受付ゾーンID
#define WHITE_INFO_ZONE_ID (ZONE_ID_D31R0101) // ホワイトの受付ゾーンID

//-------------------------------------
///	モデル管理
//=====================================
typedef enum
{
  MODELID_ELEVATOR_1,// 通常
  MODELID_ELEVATOR_2,// 通常
  MODELID_ELEVATOR_3,// (右店頭用)
  MODELID_GATE,       // ゲート
  MODELID_MAX,
}MODELID;

//-------------------------------------
// エレベータ配置データ
//=====================================
typedef struct {
  u16       zone_id;
  u16       obj_id;
  s32 x;
  s32 y;
  s32 z;
}ELEVATOR_SET_DATA;

//-------------------------------------
// エレベータの場所テーブル
//=====================================
static const ELEVATOR_SET_DATA sc_elevator_pos_table[] = {
  {
    BLACK_INFO_ZONE_ID, // 受付（ブラック）
    MODELID_ELEVATOR_1,
    120 , 0 , 32,
  },
  {
    BLACK_INFO_ZONE_ID, // 受付（ブラック）
    MODELID_ELEVATOR_2,
    182 , 0 , 32,
  },
  {
    BLACK_INFO_ZONE_ID, // 受付（ブラック:点灯用）
    MODELID_ELEVATOR_3,
    182 , 0 , 32,
  },
  {
    ZONE_ID_D31R0401, // エレベーター部屋（ブラック）
    MODELID_ELEVATOR_1,
    104 , 0 , 0,
  },
  {
    WHITE_INFO_ZONE_ID, // 受付（ホワイト）
    MODELID_ELEVATOR_1,
    120 , 0 , 32,
  },
  {
    WHITE_INFO_ZONE_ID, // 受付（ホワイト）
    MODELID_ELEVATOR_2,
    184 , 0 , 32,
  },
  {
    WHITE_INFO_ZONE_ID, // 受付（ホワイト）
    MODELID_ELEVATOR_3,
    184 , 0 , 32,
  },
  {
    ZONE_ID_D31R0301, // エレベーター部屋（ホワイト）
    MODELID_ELEVATOR_1,
    104 , 0 , 0,
  },
};
#define ELEVATOR_DATA_TABLE_SIZE (NELEMS(sc_elevator_pos_table))

//-------------------------------------
///	ワーク
//=====================================

//リソースID
enum
{
  MODEL_RESID_ELEVATOR,       // エレベータ用モデル
  MODEL_RESID_ELEVATOR_ITA,   // エレベータアニメ
  MODEL_RESID_ELEVATOR_ICA,   // エレベータアニメ
  MODEL_RESID_ELEVATOR02,     // エレベータ用モデル(点灯用)
  MODEL_RESID_ELEVATOR02_ITA, // エレベータアニメ(点灯用)
  MODEL_RESID_ELEVATOR02_ICA, // エレベータアニメ(点灯用)
  MODEL_RESID_GATE,           // ゲートIMD
  MODEL_RESID_GATE_ICA,       // ゲートICA
  MODEL_RESID_GATE_ITA,       // ゲートITA
  MODEL_RESID_MAX,
};

//リソースデータ（黒）
static const GFL_G3D_UTIL_RES sc_g3d_util_res_b[MODEL_RESID_MAX] =
{
  // MODEL_RESID_ELEVATOR->エレベータデータ
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_b_nsbmd,
    GFL_G3D_UTIL_RESARC,
  },
  //MODEL_RESID_ELEVATOR_TA->エレベータアニメーション
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_b_nsbta, 
    GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_ELEVATOR_ICA->エレベータアニメーション
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_b_nsbca, 
    GFL_G3D_UTIL_RESARC
  },
  // MODEL_RESID_ELEVATOR->エレベータデータ(点灯用)
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_b02_nsbmd,
    GFL_G3D_UTIL_RESARC,
  },
  //MODEL_RESID_ELEVATOR_TA->エレベータアニメーション(点灯用)
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_b02_nsbta, 
    GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_ELEVATOR_ICA->エレベータアニメーション(点灯用)
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_b02_nsbca, 
    GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_GATE->ゲートモデル
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_gate_b_nsbmd, 
    GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_GATE_ICA->ゲートアニメーション(ICA)
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_gate_b_nsbca, 
    GFL_G3D_UTIL_RESARC
  },
};

//リソースデータ（白）
static const GFL_G3D_UTIL_RES sc_g3d_util_res_w[MODEL_RESID_MAX] =
{
  // MODEL_RESID_ELEVATOR->エレベータデータ
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_w_nsbmd,
    GFL_G3D_UTIL_RESARC,
  },
  //MODEL_RESID_ELEVATOR_TA->エレベータアニメーション
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_w_nsbta, 
    GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_ELEVATOR_ICA->エレベータアニメーション
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_w_nsbca, 
    GFL_G3D_UTIL_RESARC
  },
  // MODEL_RESID_ELEVATOR->エレベータデータ(点灯用)
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_w02_nsbmd,
    GFL_G3D_UTIL_RESARC,
  },
  //MODEL_RESID_ELEVATOR_TA->エレベータアニメーション(点灯用)
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_w02_nsbta, 
    GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_ELEVATOR_ICA->エレベータアニメーション(点灯用)
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_elevator_w02_nsbca, 
    GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_GATE->ゲートモデル
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_gate_w_nsbmd, 
    GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_GATE_ICA->ゲートアニメーション(ICA)
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_gate_w_nsbca, 
    GFL_G3D_UTIL_RESARC
  },
  //MODEL_RESID_GATE_ICA->ゲートアニメーション(ITA)
  {
    ARCID_INFINITY_DUNGEON_GIMMICK_DATA,
    NARC_infinity_dungeon_gmk_gate_w_nsbta, 
    GFL_G3D_UTIL_RESARC
  },
};

// アニメ設定
static const GFL_G3D_UTIL_ANM sc_g3d_elevator_anm[] = 
{
  { MODEL_RESID_ELEVATOR_ICA, 0 },
  { MODEL_RESID_ELEVATOR_ITA, 0 }
};
#define ANIMEID_ELEVATOR_MAX  (NELEMS(sc_g3d_elevator_anm))

// アニメ設定
static const GFL_G3D_UTIL_ANM sc_g3d_elevator_anm02[] = 
{
  { MODEL_RESID_ELEVATOR02_ICA, 0 },
  { MODEL_RESID_ELEVATOR02_ITA, 0 }
};
#define ANIMEID_ELEVATOR02_MAX  (NELEMS(sc_g3d_elevator_anm02))

// アニメ設定
static const GFL_G3D_UTIL_ANM sc_g3d_gate_b[] = 
{
  { MODEL_RESID_GATE_ICA, 0 },
};
#define ANIMEID_GATE_B_MAX  (NELEMS(sc_g3d_gate_b))

// アニメ設定
static const GFL_G3D_UTIL_ANM sc_g3d_gate_w[] = 
{
  { MODEL_RESID_GATE_ICA, 0 },
  { MODEL_RESID_GATE_ITA, 0 },
};
#define ANIMEID_GATE_W_MAX  (NELEMS(sc_g3d_gate_w))

//3Dオブジェクト設定(ブラック)
static const GFL_G3D_UTIL_OBJ sc_g3d_util_obj_b[MODELID_MAX] =
{
  // エレベータ(左)
  {
    MODEL_RESID_ELEVATOR, 	//モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_ELEVATOR, 	//テクスチャリソースID
		sc_g3d_elevator_anm,			    //アニメテーブル(複数指定のため)
		ANIMEID_ELEVATOR_MAX,	        //アニメリソース数
  },
  // エレベータ(右)
  {
    MODEL_RESID_ELEVATOR, 	//モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_ELEVATOR, 	//テクスチャリソースID
		sc_g3d_elevator_anm,			    //アニメテーブル(複数指定のため)
		ANIMEID_ELEVATOR_MAX,	        //アニメリソース数
  },
  // エレベータ(右点灯用)
  {
    MODEL_RESID_ELEVATOR02, 	//モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_ELEVATOR02, 	//テクスチャリソースID
		sc_g3d_elevator_anm02,			    //アニメテーブル(複数指定のため)
		ANIMEID_ELEVATOR02_MAX,	        //アニメリソース数
  },
  // ゲート
  {
    MODEL_RESID_GATE, 	    //モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_GATE, 	    //テクスチャリソースID
		sc_g3d_gate_b,			    //アニメテーブル(複数指定のため)
		ANIMEID_GATE_B_MAX,	        //アニメリソース数
  },
};

//3Dオブジェクト設定(ホワイト)
static const GFL_G3D_UTIL_OBJ sc_g3d_util_obj_w[MODELID_MAX] =
{
  // エレベータ(左)
  {
    MODEL_RESID_ELEVATOR, 	//モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_ELEVATOR, 	//テクスチャリソースID
		sc_g3d_elevator_anm,			    //アニメテーブル(複数指定のため)
		ANIMEID_ELEVATOR_MAX,	        //アニメリソース数
  },
  // エレベータ(右)
  {
    MODEL_RESID_ELEVATOR, 	//モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_ELEVATOR, 	//テクスチャリソースID
		sc_g3d_elevator_anm,			    //アニメテーブル(複数指定のため)
		ANIMEID_ELEVATOR_MAX,	        //アニメリソース数
  },
  // エレベータ(右点灯用)
  {
    MODEL_RESID_ELEVATOR02, 	//モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_ELEVATOR02, 	//テクスチャリソースID
		sc_g3d_elevator_anm02,			    //アニメテーブル(複数指定のため)
		ANIMEID_ELEVATOR02_MAX,	        //アニメリソース数
  },
  // ゲート
  {
    MODEL_RESID_GATE, 	    //モデルリソースID
		0, 							        //モデルデータID(リソース内部INDEX)
		MODEL_RESID_GATE, 	    //テクスチャリソースID
		sc_g3d_gate_w,			    //アニメテーブル(複数指定のため)
		ANIMEID_GATE_W_MAX,	        //アニメリソース数
  },
};
// ブラック全情報設定
static const GFL_G3D_UTIL_SETUP sc_g3d_util_setup_b =
{
  sc_g3d_util_res_b,  //リソーステーブル
  MODEL_RESID_MAX,  //リソース最大数
  sc_g3d_util_obj_b,  //オブジェクトテーブル
  MODELID_MAX,      //オブジェクト数
};

// ホワイト全情報設定
static const GFL_G3D_UTIL_SETUP sc_g3d_util_setup_w =
{
  sc_g3d_util_res_w,  //リソーステーブル
  MODEL_RESID_MAX,  //リソース最大数
  sc_g3d_util_obj_w,  //オブジェクトテーブル
  MODELID_MAX,      //オブジェクト数
};

//-------------------------------------
///	エレベータアニメーションフレーム定義
//=====================================
typedef enum
{
  ELEVATOR_START_FRAME_OPEN   = 0,
  ELEVATOR_END_FRAME_OPEN     = 14,
  ELEVATOR_START_FRAME_CLOSE  = 15,
  ELEVATOR_END_FRAME_CLOSE    = 29,
}ELEVATOR_ANIME_FRAME;


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
///	エレベータギミックワーク
//=====================================
typedef struct {
  u32  zone_id;
  FLD_EXP_OBJ_CNT_PTR   p_ex_obj;
  FIELDMAP_WORK         *p_fieldmap;
  MODEL                 model[MODELID_MAX]; // モデル
  INFINITYDUNGEON_SAVE  *p_save;
  FIELD_WFBC_CORE_TYPE  wfbc_type;
}INF_ELEVATOR_WK;

//-------------------------------------
///	ギミックワーク
//=====================================
typedef struct {
  INF_ELEVATOR_WK       elev_wk;
  INF_DUNGEON_DATA      *p_dungeon_data;
}INF_GIMMICK_WK;

//-------------------------------------
///	エレベータ開閉イベントワーク
//=====================================
typedef struct {
  FIELDMAP_WORK   *p_fieldmap;
  INF_GIMMICK_WK  *p_gimmick_wk;
  u8              model_no;
  BOOL            is_open;
}INF_ELEVTER_EVENT_WK;

//-------------------------------------
///	エレベータ開放イベントワーク
//=====================================
typedef struct {
  FIELDMAP_WORK   *p_fieldmap;
  INF_GIMMICK_WK  *p_gimmick_wk;
}INF_ELEVTER_LIFTBAN_EVENT_WK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void InitElevatorGimmicWork( INF_ELEVATOR_WK *p_wk ,FIELDMAP_WORK *p_fieldmap ,INFINITYDUNGEON_SAVE  *p_inf_save,FIELD_WFBC_CORE_TYPE  wfbc_type);
static void MapModelInit( INF_DUNGEON_DATA  *p_dungeon_data ,INF_ELEVATOR_WK *p_wk );
static const ELEVATOR_SET_DATA* GetElevatorTable( INF_ELEVATOR_WK *p_wk );
static GMEVENT_RESULT ElevatorOpenEvent( GMEVENT *p_event, int *seq, void *wk );
static GMEVENT_RESULT ElevatorLiftBanEvent( GMEVENT *p_event, int *seq, void *wk );

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
  MMDLSYS               *p_mmdlsys;
  INF_GIMMICK_WK        *p_gmk_wk = NULL;
  const HEAPID  heapID = FIELDMAP_GetHeapID( p_fieldmap );
  GAMESYS_WORK  *p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA      *p_gdata = GAMESYSTEM_GetGameData( p_gsys );
  SAVE_CONTROL_WORK     *p_save;
  INFINITYDUNGEON_SAVE  *p_inf_save;
  FIELD_WFBC_CORE_TYPE  wfbc_type;

  //汎用ワーク確保
  p_gmk_wk = GMK_TMP_WK_AllocWork(p_fieldmap, INF_GIMMICK_WORK_ASSIGN_ID, 
      heapID, sizeof(INF_GIMMICK_WK));
  
  // 無限ダンジョンのデータを取得
  p_save = GAMEDATA_GetSaveControlWork( p_gdata );
  p_inf_save  = SaveData_GetINFINITYDUNGEON( p_save );
   
  // WFBCの状態を調べる
  wfbc_type = GAMEDATA_GetMyWFBCType( p_gdata );

  // ワークの初期化
  InitElevatorGimmicWork( &p_gmk_wk->elev_wk , p_fieldmap ,p_inf_save ,wfbc_type );

  // 無限ダンジョンワーク確保
  p_gmk_wk->p_dungeon_data = GFL_HEAP_AllocMemory( heapID , INF_GetDungeonDataSize() );

  // ダンジョンデータの生成
  INF_MakeDungeonData( p_gmk_wk->p_dungeon_data ,p_gdata ,heapID );

  // モデルの読み込み
  MapModelInit( p_gmk_wk->p_dungeon_data, &p_gmk_wk->elev_wk );

  // ビルボードアクターのライトを設定する（鏡面床の影）
  p_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
  MMDLSYS_SetReflectionShadowLightMask( p_mmdlsys ,GFL_BBD_LIGHTMASK_3 );
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
  MMDLSYS               *p_mmdlsys;
  INF_GIMMICK_WK *p_wk = NULL;
  p_wk = GMK_TMP_WK_GetWork(p_fieldmap, INF_GIMMICK_WORK_ASSIGN_ID);

  // 破棄
  FLD_EXP_OBJ_DelUnit( p_wk->elev_wk.p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID );
  
  // ダンジョンデータを開放する
  GFL_HEAP_FreeMemory( p_wk->p_dungeon_data );

  // 汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, INF_GIMMICK_WORK_ASSIGN_ID);
  
  // ライトマスクを戻す
  p_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
  MMDLSYS_SetReflectionShadowLightMask( p_mmdlsys ,GFL_BBD_LIGHTMASK_0 );
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
  INF_GIMMICK_WK *p_wk = NULL;
  p_wk = GMK_TMP_WK_GetWork(p_fieldmap, INF_GIMMICK_WORK_ASSIGN_ID);

  //通常のギミックモデル
  FLD_EXP_OBJ_PlayAnime( p_wk->elev_wk.p_ex_obj );
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
static void MapModelInit( INF_DUNGEON_DATA  *p_dungeon_data ,INF_ELEVATOR_WK *p_wk )
{
  //読込み
  if( INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data ) == VERSION_B2 )
  {
    FLD_EXP_OBJ_AddUnitByHandle(p_wk->p_ex_obj, &sc_g3d_util_setup_b, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID );
  }
  else
  {
    FLD_EXP_OBJ_AddUnitByHandle(p_wk->p_ex_obj, &sc_g3d_util_setup_w, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID );
  }

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
    int i,n;
    const ELEVATOR_SET_DATA *p_elevator_table = GetElevatorTable( p_wk );
    for( i = 0 ; i < ELEVATOR_DATA_TABLE_SIZE ; i++ )
    {
      GFL_G3D_OBJSTATUS *p_status;
      const ELEVATOR_SET_DATA *p_data = &p_elevator_table[i];
      MODEL *p_model    = &p_wk->model[p_data->obj_id];
      if( p_wk->zone_id == p_data->zone_id )
      {

        // ステータス取得
        p_status = FLD_EXP_OBJ_GetUnitObjStatus( p_model->p_ex_obj,
            MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID,
            p_model->modelID );

        // 位置を設定
        p_status->trans.x = FX32_CONST(p_data->x);
        p_status->trans.y = FX32_CONST(p_data->y);
        p_status->trans.z = FX32_CONST(p_data->z);

        // 見える状態に
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
            MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, FALSE );
        
        // アニメーション初期化
        for( n = 0; n < ANIMEID_ELEVATOR_MAX; n ++ )
        {
          EXP_OBJ_ANM_CNT_PTR exobj_anm;
          FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, n, TRUE);
          exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, n );
          FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
        }
      }
    }
  }

  // 表示状態の設定（受付の時のみ行う）
  if( (p_wk->zone_id == BLACK_INFO_ZONE_ID) || (p_wk->zone_id == WHITE_INFO_ZONE_ID) )
  {
    u8 status = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_wk->p_save ,p_wk->wfbc_type);
    MODEL *p_model_on    = &p_wk->model[MODELID_ELEVATOR_2];
    MODEL *p_model_off    = &p_wk->model[MODELID_ELEVATOR_3];
    if( status <= INF_LEFT_ELEVATOR_AREA )
    {
      // 右側が使用可能な状態
      FLD_EXP_OBJ_SetVanish( p_model_on->p_ex_obj,
          MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model_on->modelID, TRUE );
      FLD_EXP_OBJ_SetVanish( p_model_off->p_ex_obj,
          MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model_off->modelID, FALSE );
    }
    else
    {
      // 右側が閉じている状態
      FLD_EXP_OBJ_SetVanish( p_model_on->p_ex_obj,
          MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model_on->modelID, FALSE );
      FLD_EXP_OBJ_SetVanish( p_model_off->p_ex_obj,
          MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model_off->modelID, TRUE );
    }
  }
  else
  {
    u8 now_floor  = INFINITYDUNGEON_SAVE_GetFloorID( p_wk->p_save );
    u8 now_room   = INFINITYDUNGEON_SAVE_GetRoomID( p_wk->p_save );
    // 受付以外の時に行う
    BOOL is_boss = INFINITY_DUNGEON_GetUpPosRoomTypeIsBoss( p_dungeon_data ,now_floor ,now_room) ;
 
    // 上の部屋がボス部屋の時のみ表示する
    if( is_boss )
    {
      u8 anm_idx = 0;
      u8 anm_cnt = 0;
      MODEL *p_model_gate    = &p_wk->model[MODELID_GATE];
      BOOL is_open = INFINITY_DUNGEON_GetGateOpen( p_dungeon_data, p_wk->p_save);
      
      // アニメーションを有効に
      FLD_EXP_OBJ_SetVanish( p_model_gate->p_ex_obj,
          MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model_gate->modelID, FALSE );

      // アニメーション数を取得
      if( INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data ) == VERSION_B2 )
      {
        anm_cnt = ANIMEID_GATE_B_MAX;
      }
      else
      {
        anm_cnt = ANIMEID_GATE_W_MAX;
      }

      // アニメーションを初期化
      for( anm_idx = 0; anm_idx < anm_cnt; anm_idx++ )
      {
        fx32 last_frame;
        EXP_OBJ_ANM_CNT_PTR exobj_anm;

        // アニメーションを有効に
        FLD_EXP_OBJ_ValidCntAnm(p_model_gate->p_ex_obj,
            MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model_gate->modelID, anm_idx, TRUE);
        
        // アニメーションを停止
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model_gate->p_ex_obj,
            MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model_gate->modelID, anm_idx );
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );

        // 開いている場合は最終フレームに進める
        if( is_open == TRUE )
        {
          last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exobj_anm );
          FLD_EXP_OBJ_SetObjAnmFrm( p_model_gate->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model_gate->modelID, anm_idx, last_frame );
        }
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
static void InitElevatorGimmicWork( INF_ELEVATOR_WK *p_wk ,FIELDMAP_WORK *p_fieldmap ,INFINITYDUNGEON_SAVE  *p_inf_save,FIELD_WFBC_CORE_TYPE  wfbc_type)
{
  GFL_STD_MemFill(p_wk, 0, sizeof(INF_ELEVATOR_WK));
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->zone_id     = FIELDMAP_GetZoneID( p_wk->p_fieldmap );
  p_wk->p_ex_obj    = FIELDMAP_GetExpObjCntPtr( p_wk->p_fieldmap );
  p_wk->p_save      = p_inf_save;
  p_wk->wfbc_type   = wfbc_type;
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
static const ELEVATOR_SET_DATA* GetElevatorTable( INF_ELEVATOR_WK *p_wk )
{
  return sc_elevator_pos_table;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンデータを取得する
 *
 *	@param	*p_fieldmap 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
INF_DUNGEON_DATA* INFINITY_DUNGEON_DataGet( FIELDMAP_WORK *p_fieldmap )
{
  INF_GIMMICK_WK *p_wk = NULL;
  p_wk = GMK_TMP_WK_GetWork(p_fieldmap, INF_GIMMICK_WORK_ASSIGN_ID);

  return p_wk->p_dungeon_data;
}


//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンのエレベータギミック起動
 *
 *	@param	*p_gsys
 *	@param	*p_fieldmap 
 *	@param	is_open      TRUE = 開く
 *	@param	elev_no     エレベータ番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT * INFINITY_DUNGEON_EVENT_ElevatorOpen(GAMESYS_WORK *p_gsys, FIELDMAP_WORK *p_fieldmap ,BOOL is_open ,int elev_no)
{
  GMEVENT   *event;
  INF_ELEVTER_EVENT_WK *p_wk;

  // イベント作成
  event = GMEVENT_Create(
      p_gsys, NULL, ElevatorOpenEvent, sizeof(INF_ELEVTER_EVENT_WK) );

  // ワークに値をセット
	p_wk = GMEVENT_GetEventWork( event );
  p_wk->p_fieldmap = p_fieldmap;
  p_wk->p_gimmick_wk = GMK_TMP_WK_GetWork(p_fieldmap, INF_GIMMICK_WORK_ASSIGN_ID);
  p_wk->model_no = elev_no;
  p_wk->is_open = is_open;

  return( event );
}


//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンのエレベータ開放ギミック起動
 *
 *	@param	*p_gsys
 *	@param	*p_fieldmap 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT * INFINITY_DUNGEON_EVENT_ElevatorLiftBan(GAMESYS_WORK *p_gsys, FIELDMAP_WORK *p_fieldmap )
{
  GMEVENT   *event;
  INF_ELEVTER_LIFTBAN_EVENT_WK *p_wk;

  // イベント作成
  event = GMEVENT_Create(
      p_gsys, NULL, ElevatorLiftBanEvent, sizeof(INF_ELEVTER_EVENT_WK) );

  // ワークに値をセット
	p_wk = GMEVENT_GetEventWork( event );
  p_wk->p_fieldmap = p_fieldmap;
  p_wk->p_gimmick_wk = GMK_TMP_WK_GetWork(p_fieldmap, INF_GIMMICK_WORK_ASSIGN_ID);

  return( event );
}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベータを開くイベント
 *
 *	@param	*p_event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ElevatorOpenEvent( GMEVENT *p_event, int *seq, void *wk )
{
  int n;
  fx32 start_frame ,end_frame;
  fx32 frame;
  EXP_OBJ_ANM_CNT_PTR exobj_anm;
  INF_ELEVTER_EVENT_WK *p_wk = wk;
  u8 mode_no = p_wk->model_no;
  MODEL *p_model = &p_wk->p_gimmick_wk->elev_wk.model[mode_no];
  INF_DUNGEON_DATA  *p_dungeon_data = p_wk->p_gimmick_wk->p_dungeon_data;
  u32 sound_id; 

  // アニメーションフレーム定義
  if( p_wk->is_open == TRUE )
  {
    start_frame = ELEVATOR_START_FRAME_OPEN << FX32_SHIFT;
    end_frame = ELEVATOR_END_FRAME_OPEN << FX32_SHIFT;

    if( INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data ) == VERSION_B2 )
    {
      sound_id = SEQ_SE_SW_MDUN_BL_01;  //エレベータの開く音
    }
    else
    {
      sound_id = SEQ_SE_SW_MDUN_WH_01;  //エレベータの開く音
    }
  }
  else
  {
    start_frame = ELEVATOR_START_FRAME_CLOSE << FX32_SHIFT;
    end_frame = ELEVATOR_END_FRAME_CLOSE << FX32_SHIFT;
    
    if( INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data ) == VERSION_B2 )
    {
      sound_id = SEQ_SE_SW_MDUN_BL_02;  //エレベータのしまる音
    }
    else
    {
      sound_id = SEQ_SE_SW_MDUN_WH_02;  //エレベータのしまる音
    }
  }

  // アニメーション
  switch( *seq )
  {
    // アニメーションの初期化
  case 0:
    {
      // アニメーション初期化
      for( n = 0; n < ANIMEID_ELEVATOR_MAX; n ++ )
      {
        // アニメーションの取得
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, n );

        // 閉じた状態に初期化
        FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, n , start_frame);
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
      }
      // 開閉音
      PMSND_PlaySE( sound_id );
      (*seq)++;

    }
    break;
    // アニメーションの終了待ち
  case 1:
    {
      frame = FLD_EXP_OBJ_GetObjAnmFrm(p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, 0 );
      if( frame >= end_frame && (PMSND_CheckPlaySE()))
      {
        for( n = 0; n < ANIMEID_ELEVATOR_MAX; n ++ )
        {
          // アニメーションの取得
          exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, n );

          // アニメーション停止
          FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
        }
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベータを開くイベント
 *
 *	@param	*p_event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ElevatorLiftBanEvent( GMEVENT *p_event, int *seq, void *wk )
{
  int n;
  EXP_OBJ_ANM_CNT_PTR exobj_anm;
  INF_ELEVTER_LIFTBAN_EVENT_WK *p_wk = wk;
  MODEL *p_model_off = &p_wk->p_gimmick_wk->elev_wk.model[MODELID_ELEVATOR_3];
  MODEL *p_model_on = &p_wk->p_gimmick_wk->elev_wk.model[MODELID_ELEVATOR_2];
  INF_DUNGEON_DATA  *p_dungeon_data = p_wk->p_gimmick_wk->p_dungeon_data;
  u32 sound_id; 


  if( INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data ) == VERSION_B2 )
  {
    sound_id = SEQ_SE_SW_MDUN_BL_02;  //エレベータのしまる音
  }
  else
  {
    sound_id = SEQ_SE_SW_MDUN_WH_02;  //エレベータのしまる音
  }

  // アニメーション
  switch( *seq )
  {
    // アニメーションの初期化
  case 0:
    {
      // アニメーション初期化
      for( n = 0; n < ANIMEID_ELEVATOR_MAX; n ++ )
      {
        // アニメーションの取得
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model_off->p_ex_obj, p_model_off->unitID, p_model_off->modelID, n );

        // 閉じた状態に初期化
        FLD_EXP_OBJ_SetObjAnmFrm(p_model_off->p_ex_obj, p_model_off->unitID, p_model_off->modelID, n , 0);
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
      }
      PMSND_PlaySE( sound_id );
      (*seq)++;

    }
    break;
    // アニメーションの終了待ち
  case 1:
    {
      // アニメーションの取得
      exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model_off->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model_off->modelID, 0 );
      if( (FLD_EXP_OBJ_ChkAnmEnd( exobj_anm ) == TRUE) && (PMSND_CheckPlaySE()))
      {
        FLD_EXP_OBJ_SetVanish( p_model_off->p_ex_obj,  p_model_off->unitID, p_model_off->modelID, TRUE );
        FLD_EXP_OBJ_SetVanish( p_model_on->p_ex_obj,  p_model_on->unitID, p_model_on->modelID, FALSE );
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

