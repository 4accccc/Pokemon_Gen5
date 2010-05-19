////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ギミック処理(ポケモンリーグフロント下)
 * @file   field_gimmick_league_front02.c
 * @author obata
 * @date   2009.12.01
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include "fieldmap.h"
#include "gmk_tmp_wk.h"
#include "field_gimmick_league_front02.h"
#include "field_gimmick_def.h"  // for FLD_GIMMICK_C09P02
#include "fld_exp_obj.h"        // for FLD_EXP_OBJ

#include "gamesystem/gamesystem.h"
#include "savedata/gimmickwork.h"  // for GIMMICKWORK
#include "system/ica_anime.h"      // for ICA_ANIME
#include "field/field_const.h"     // for FIELD_CONST_GRID_SIZE

#include "arc/league_front.naix" // for NARC_xxxx


//==========================================================================================
// ■定数
//==========================================================================================

// ギミックワークのアサインID
#define GIMMICK_WORK_ASSIGN_ID (1)

// ギミックデータのアーカイブID
#define ARCID (ARCID_LEAGUE_FRONT_GIMMICK)  

// ギミックワークのデータインデックス
typedef enum {
  GIMMICKWORK_DATA_NUM,
  GIMMICKWORK_DATA_MAX = GIMMICKWORK_DATA_NUM - 1
} GIMMICKWORK_DATA_INDEX;


//==========================================================================================
// ■3Dリソース
//==========================================================================================
//----------
// リソース
//----------
typedef enum {
  RES_LIFT_NSBMD,             // リフトのモデル
  RES_LIFT_NSBTA,             // リフトのテクスチャアニメーション
  RES_LIFT_EFFECT_NSBMD,      // リフト稼動エフェクトのモデル
  RES_LIFT_EFFECT_NSBTA,      // リフト稼動エフェクトのテクスチャアニメーション
  RES_LIFT_EFFECT_NSBMA,      // リフト稼動エフェクトのマテリアルアニメーション
  RES_NUM
} RES_INDEX;

static const GFL_G3D_UTIL_RES res_table[ RES_NUM ] = 
{
  {ARCID, NARC_league_front_pl_ele_01_nsbmd,     GFL_G3D_UTIL_RESARC}, // [ RES_LIFT_NSBMD ]
  {ARCID, NARC_league_front_pl_ele_01_nsbta,     GFL_G3D_UTIL_RESARC}, // [ RES_LIFT_NSBTA ]
  {ARCID, NARC_league_front_pl_efect_01_nsbmd,   GFL_G3D_UTIL_RESARC}, // [ RES_LIFT_EFFECT_NSBMD ]
  {ARCID, NARC_league_front_pl_efect_01_nsbta,   GFL_G3D_UTIL_RESARC}, // [ RES_LIFT_EFFECT_NSBTA ]
  {ARCID, NARC_league_front_pl_efect_01_nsbma,   GFL_G3D_UTIL_RESARC}, // [ RES_LIFT_EFFECT_NSBMA ]
}; 
//------------------------
// アニメーション(リフト)
//------------------------
static const GFL_G3D_UTIL_ANM anm_table_lift[ LIFT_ANM_NUM ] = 
{
  {RES_LIFT_NSBTA,  0}, 
}; 
//----------------------------------------
// アニメーション(リフトの稼動エフェクト)
//----------------------------------------
static const GFL_G3D_UTIL_ANM anm_table_lift_effect[ LIFT_EFFECT_ANM_NUM ] = 
{
  {RES_LIFT_EFFECT_NSBTA,  0}, 
  {RES_LIFT_EFFECT_NSBMA, 0},
}; 
//-------------
// オブジェクト
//-------------
static const GFL_G3D_UTIL_OBJ obj_table[ LF02_EXOBJ_NUM ] = 
{
  {RES_LIFT_NSBMD,        0, RES_LIFT_NSBMD,        anm_table_lift,        LIFT_ANM_NUM},
  {RES_LIFT_EFFECT_NSBMD, 0, RES_LIFT_EFFECT_NSBMD, anm_table_lift_effect, LIFT_EFFECT_ANM_NUM},
}; 
//----------
// ユニット
//----------
static const GFL_G3D_UTIL_SETUP unit[ LF02_EXUNIT_NUM ] = 
{ 
  {res_table, RES_NUM, obj_table, LF02_EXOBJ_NUM},
};


//==========================================================================================
// ■各オブジェの配置
//========================================================================================== 
// リフト座標
#define LIFT_POS_X_GRID (30)
#define LIFT_POS_Z_GRID (81)
#define LIFT_POS_X ((LIFT_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIFT_POS_Y (2 << FX32_SHIFT)
#define LIFT_POS_Z ((LIFT_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)

// 配置座標
static const VecFx32 obj_pos[ LF02_EXOBJ_NUM ] = 
{
  {LIFT_POS_X, LIFT_POS_Y, LIFT_POS_Z},
  {LIFT_POS_X, LIFT_POS_Y, LIFT_POS_Z},
};

//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct { 

  HEAPID heapID;  // 使用するヒープID

} LF02WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void Save( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx, u32 data );
static u32 Load( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx );
static void InitGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap );
static void LoadGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap );
static void SaveGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap );


//==========================================================================================
// ■ギミック登録関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  LF02WORK* work;
  HEAPID heap_id;

  // ギミック管理ワーク作成
  heap_id = FIELDMAP_GetHeapID( fieldmap );
  work    = (LF02WORK*)GMK_TMP_WK_AllocWork( fieldmap, GIMMICK_WORK_ASSIGN_ID,  heap_id, sizeof(LF02WORK) );
  work->heapID = heap_id; 

  // ギミック初期化
  InitGimmick( work, fieldmap );
  LoadGimmick( work, fieldmap ); 
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  LF02WORK* work;

  // セーブ
  work = (LF02WORK*)GMK_TMP_WK_GetWork( fieldmap, GIMMICK_WORK_ASSIGN_ID );
  SaveGimmick( work, fieldmap );

  // ギミック管理ワーク破棄
  GMK_TMP_WK_FreeWork( fieldmap, GIMMICK_WORK_ASSIGN_ID );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 動作関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  // 拡張オブジェクトのアニメ再生
  {
    FLD_EXP_OBJ_CNT_PTR exobj_cnt;
    exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
    FLD_EXP_OBJ_PlayAnime( exobj_cnt );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト本体のアニメーションを開始する
 *
 * @param fieldmap
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_StartLiftAnime( FIELDMAP_WORK* fieldmap )
{
  int i;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;

  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  for( i=0; i<LIFT_ANM_NUM; i++ )
  {
    FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT, i, TRUE );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト本体のアニメーションを停止する
 *
 * @param fieldmap
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_StopLiftAnime( FIELDMAP_WORK* fieldmap )
{
  int i;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;

  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  for( i=0; i<LIFT_ANM_NUM; i++ )
  {
    FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT, i, FALSE );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト稼動エフェクトを消去する
 *
 * @param fieldmap
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_HideLiftEffect( FIELDMAP_WORK* fieldmap )
{
  int i;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;

  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  FLD_EXP_OBJ_SetVanish( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT_EFFECT, TRUE );

  for( i=0; i<LIFT_EFFECT_ANM_NUM; i++ )
  {
    FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT_EFFECT, i,  FALSE );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト稼動エフェクトを表示する
 *
 * @param fieldmap
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_ShowLiftEffect( FIELDMAP_WORK* fieldmap )
{
  int i;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;

  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  FLD_EXP_OBJ_SetVanish( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT_EFFECT, FALSE );

  for( i=0; i<LIFT_EFFECT_ANM_NUM; i++ )
  {
    FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT_EFFECT, i,  TRUE );
  }

  {
    EXP_OBJ_ANM_CNT_PTR anime_controller;
    anime_controller = FLD_EXP_OBJ_GetAnmCnt( 
        exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT_EFFECT, LIFT_EFFECT_ANM_TA );
    FLD_EXP_OBJ_ChgAnmLoopFlg( anime_controller, FALSE );
    anime_controller = FLD_EXP_OBJ_GetAnmCnt( 
        exobj_cnt, LF02_EXUNIT_GIMMICK, LF02_EXOBJ_LIFT_EFFECT, LIFT_EFFECT_ANM_MA );
    FLD_EXP_OBJ_ChgAnmLoopFlg( anime_controller, FALSE );
  }
}



//==========================================================================================
// ■ 非公開関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックワークへのセーブ処理
 *
 * @param fieldmap フィールドマップ
 * @param idx      格納先インデックス
 * @param data     格納する実データ
 */
//------------------------------------------------------------------------------------------
static void Save( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx, u32 data )
{
  GAMESYS_WORK*   gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*      gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK* gmkwork = GAMEDATA_GetGimmickWork( gdata );
  u32*       save_work = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_C09P02 );

  // ギミックワークに保存
  save_work[idx] = data;
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックワークからのロード処理
 *
 * @param fieldmap フィールドマップ
 * @param idx      取得データのインデックス
 *
 * @return 指定したインデックスに格納した値
 */
//------------------------------------------------------------------------------------------
static u32 Load( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx )
{
  GAMESYS_WORK*   gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*      gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK* gmkwork = GAMEDATA_GetGimmickWork( gdata );
  u32*       save_work = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_C09P02 );
  u32              val = 0;

  // ギミックワークから取得
  val = save_work[idx];

  return val;
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックを初期化する
 *
 * @param work     初期化対象ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void InitGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap )
{
  int i;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;

  // 拡張オブジェクトのユニットを追加
  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  FLD_EXP_OBJ_AddUnitByHandle( exobj_cnt, &unit[LF02_EXUNIT_GIMMICK], LF02_EXUNIT_GIMMICK );

  // 各オブジェの配置
  {
    int obj_idx;
    GFL_G3D_OBJSTATUS* objstatus;
    for( obj_idx=0; obj_idx<LF02_EXOBJ_NUM; obj_idx++ )
    {
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF02_EXUNIT_GIMMICK, obj_idx );
      VEC_Set( &objstatus->trans, obj_pos[obj_idx].x, obj_pos[obj_idx].y, obj_pos[obj_idx].z );
    }
  }

  // リフトを初期化
  LEAGUE_FRONT_02_GIMMICK_StopLiftAnime( fieldmap );

  // リフトの稼動エフェクトを初期化
  LEAGUE_FRONT_02_GIMMICK_HideLiftEffect( fieldmap );
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックの状態を復帰する
 *
 * @param work     復帰対象ギミックの管理ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void LoadGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap )
{
} 

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックの状態を保存する
 *
 * @param work     保存対象ギミックの管理ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void SaveGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap )
{
}
