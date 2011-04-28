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

//-------------------------------------
///	モデル管理
//=====================================
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
  s32 x;
  s32 y;
  s32 z;
}ELEVATOR_SET_DATA;

#define ELEVATOR_DATA_TABLE_SIZE (6)
//-------------------------------------
// エレベータの場所テーブル
//=====================================
static const ELEVATOR_SET_DATA sc_elevator_pos_table[ELEVATOR_DATA_TABLE_SIZE] = {
  {
    ZONE_ID_D31R0201, // 受付（ブラック）
    MODELID_ELEVATOR_1,
    120 , 0 , 32,
  },
  {
    ZONE_ID_D31R0201, // 受付（ブラック）
    MODELID_ELEVATOR_2,
    182 , 0 , 32,
  },
  {
    ZONE_ID_D31R0401, // エレベーター部屋（ブラック）
    MODELID_ELEVATOR_1,
    104 , 0 , 0,
  },
  {
    ZONE_ID_D31R0101, // 受付（ホワイト）
    MODELID_ELEVATOR_1,
    120 , 0 , 32,
  },
  {
    ZONE_ID_D31R0101, // 受付（ホワイト）
    MODELID_ELEVATOR_2,
    184 , 0 , 32,
  },
  {
    ZONE_ID_D31R0301, // エレベーター部屋（ホワイト）
    MODELID_ELEVATOR_1,
    104 , 0 , 0,
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

// ブラック全情報設定
static const GFL_G3D_UTIL_SETUP sc_g3d_util_setup_b =
{
  sc_g3d_util_res_b,  //リソーステーブル
  MODEL_RESID_MAX,  //リソース最大数
  sc_g3d_util_obj,  //オブジェクトテーブル
  MODELID_MAX,      //オブジェクト数
};

// ホワイト全情報設定
static const GFL_G3D_UTIL_SETUP sc_g3d_util_setup_w =
{
  sc_g3d_util_res_w,  //リソーステーブル
  MODEL_RESID_MAX,  //リソース最大数
  sc_g3d_util_obj,  //オブジェクトテーブル
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
  FLD_EXP_OBJ_CNT_PTR p_ex_obj;
  FIELDMAP_WORK       *p_fieldmap;
  MODEL               model[MODELID_MAX]; // モデル
}INF_ELEVATOR_WK;

//-------------------------------------
///	ギミックワーク
//=====================================
typedef struct {
  INF_ELEVATOR_WK   elev_wk;
  INF_DUNGEON_DATA  *p_dungeon_data;
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

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void InitElevatorGimmicWork( INF_ELEVATOR_WK *p_wk ,FIELDMAP_WORK *p_fieldmap );
static void MapModelInit( INF_ELEVATOR_WK *p_wk );
static const ELEVATOR_SET_DATA* GetElevatorTable( INF_ELEVATOR_WK *p_wk );
static u32 GetDungeonColor();
static GMEVENT_RESULT ElevatorOpenEvent( GMEVENT *p_event, int *seq, void *wk );

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
  SAVE_CONTROL_WORK     *p_save_data;
  INFINITYDUNGEON_SAVE  *p_save;
  const HEAPID  heapID = FIELDMAP_GetHeapID( p_fieldmap );
  GAMESYS_WORK  *p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA      *p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  p_gmk_wk = GMK_TMP_WK_AllocWork(p_fieldmap, INF_GIMMICK_WORK_ASSIGN_ID, 
      heapID, sizeof(INF_GIMMICK_WK));

  // ワークの初期化
  InitElevatorGimmicWork( &p_gmk_wk->elev_wk , p_fieldmap );

  // 無限ダンジョンのデータを生成
  p_save_data = GAMEDATA_GetSaveControlWork( p_gdata );
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );

  // 無限ダンジョンワーク確保
  p_gmk_wk->p_dungeon_data = GFL_HEAP_AllocMemory( heapID , INF_GetDungeonDataSize() );

  INF_MakeDungeonData( p_gmk_wk->p_dungeon_data ,p_save ,heapID );

  // モデルの読み込み
  MapModelInit( &p_gmk_wk->elev_wk );

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
static void MapModelInit( INF_ELEVATOR_WK *p_wk )
{
  //読込み
  if( GetDungeonColor() == VERSION_B2 )
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
        p_status->trans.x = FX32_CONST(p_data->x);
        p_status->trans.y = FX32_CONST(p_data->y);
        p_status->trans.z = FX32_CONST(p_data->z);

        // 見える状態に
        FLD_EXP_OBJ_SetVanish( p_model->p_ex_obj,
            MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, FALSE );
        
        // フレームの初期化
        FLD_EXP_OBJ_ValidCntAnm(p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, 0, TRUE);

        // アニメーション取得
        exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, 0 );

        // アニメーション停止
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );

        // ループ再生
        //FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, FALSE );
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
static void InitElevatorGimmicWork( INF_ELEVATOR_WK *p_wk ,FIELDMAP_WORK *p_fieldmap )
{
  GFL_STD_MemFill(p_wk, 0, sizeof(INF_ELEVATOR_WK));
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
static u32 GetDungeonColor()
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
  fx32 start_frame ,end_frame;
  fx32 frame;
  EXP_OBJ_ANM_CNT_PTR exobj_anm;
  INF_ELEVTER_EVENT_WK *p_wk = wk;
  u8 mode_no = p_wk->model_no;
  MODEL *p_model = &p_wk->p_gimmick_wk->elev_wk.model[mode_no];
  u32 sound_id; 

  // アニメーションの取得
  exobj_anm = FLD_EXP_OBJ_GetAnmCnt( p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, 0 );

  // アニメーションフレーム定義
  if( p_wk->is_open == TRUE )
  {
    start_frame = ELEVATOR_START_FRAME_OPEN << FX32_SHIFT;
    end_frame = ELEVATOR_END_FRAME_OPEN << FX32_SHIFT;

    if( GetDungeonColor() == VERSION_B2 )
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
    
    if( GetDungeonColor() == VERSION_B2 )
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
      // 閉じた状態に初期化
      FLD_EXP_OBJ_SetObjAnmFrm(p_model->p_ex_obj, MODEL_MANAGER_FLD_EXP_OBJ_UNIT_ID, p_model->modelID, 0 , start_frame);
      FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
      
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
        // アニメーション停止
        FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, TRUE );
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

