////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ギミック処理(ポケモンリーグフロント上)
 * @file field_gimmick_league_front01.c
 * @author obata
 * @date 2009.12.01
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include "fieldmap.h"
#include "field_gimmick_league_front01.h"
#include "gmk_tmp_wk.h"
#include "field_gimmick_def.h"  // for FLD_GIMMICK_C09P01
#include "fld_exp_obj.h"        // for FLD_EXP_OBJ
#include "field/eventwork.h"    // for EVENTWORK_

#include "gamesystem/gamesystem.h"
#include "savedata/gimmickwork.h"  // for GIMMICKWORK
#include "system/ica_anime.h"      // for ICA_ANIME
#include "field/field_const.h"     // for FIELD_CONST_GRID_SIZE

#include "arc/league_front.naix"                               // for NARC_xxxx
#include "../../../resource/fldmapdata/flagwork/flag_define.h" // for SYS_FLAG_BIGFOUR_xxxx


//==========================================================================================
// ■定数
//==========================================================================================
// ギミックワークのアサインID
#define GIMMICK_WORK_ASSIGN_ID (0)

// ギミックデータのアーカイブID
#define ARCID (ARCID_LEAGUE_FRONT_GIMMICK)  

// ギミックワークのデータインデックス
typedef enum{
  GIMMICKWORK_DATA_NUM,
  GIMMICKWORK_DATA_MAX = GIMMICKWORK_DATA_NUM - 1
} GIMMICKWORK_DATA_INDEX;


//==========================================================================================
// ■3Dリソース
//==========================================================================================
//----------
// リソース
//----------
typedef enum{
  RES_LIFT_NSBMD,             // リフトのモデル
  RES_LIFT_ON_NSBTA,          // リフトのita ( ON )
  RES_LIGHT_FIGHT_NSBMD,      // ライト(格闘)    モデル
  RES_LIGHT_FIGHT_OFF_NSBTA,  // ライト(格闘)    OFF
  RES_LIGHT_FIGHT_ON_NSBTA,   // ライト(格闘)    ON
  RES_LIGHT_EVIL_NSBMD,       // ライト(悪)      モデル
  RES_LIGHT_EVIL_OFF_NSBTA,   // ライト(悪)      OFF
  RES_LIGHT_EVIL_ON_NSBTA,    // ライト(悪)      ON
  RES_LIGHT_GHOST_NSBMD,      // ライト(ゴースト)モデル
  RES_LIGHT_GHOST_OFF_NSBTA,  // ライト(ゴースト)OFF
  RES_LIGHT_GHOST_ON_NSBTA,   // ライト(ゴースト)ON
  RES_LIGHT_ESPER_NSBMD,      // ライト(エスパー)モデル
  RES_LIGHT_ESPER_OFF_NSBTA,  // ライト(エスパー)OFF
  RES_LIGHT_ESPER_ON_NSBTA,   // ライト(エスパー)ON
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[RES_NUM] = 
{
  {ARCID, NARC_league_front_pl_ele_00_nsbmd,     GFL_G3D_UTIL_RESARC},  // リフトのモデル
  {ARCID, NARC_league_front_pl_ele_00_on_nsbta,  GFL_G3D_UTIL_RESARC},  // リフトのita ( ON )
  {ARCID, NARC_league_front_pl_lite1_nsbmd,      GFL_G3D_UTIL_RESARC},  // ライト(格闘)    モデル
  {ARCID, NARC_league_front_pl_lite1_off_nsbta,  GFL_G3D_UTIL_RESARC},  // ライト(格闘)    OFF
  {ARCID, NARC_league_front_pl_lite1_on_nsbta,   GFL_G3D_UTIL_RESARC},  // ライト(格闘)    ON
  {ARCID, NARC_league_front_pl_lite2_nsbmd,      GFL_G3D_UTIL_RESARC},  // ライト(悪)      モデル
  {ARCID, NARC_league_front_pl_lite2_off_nsbta,  GFL_G3D_UTIL_RESARC},  // ライト(悪)      OFF
  {ARCID, NARC_league_front_pl_lite2_on_nsbta,   GFL_G3D_UTIL_RESARC},  // ライト(悪)      ON
  {ARCID, NARC_league_front_pl_lite3_nsbmd,      GFL_G3D_UTIL_RESARC},  // ライト(ゴースト)モデル
  {ARCID, NARC_league_front_pl_lite3_off_nsbta,  GFL_G3D_UTIL_RESARC},  // ライト(ゴースト)OFF
  {ARCID, NARC_league_front_pl_lite3_on_nsbta,   GFL_G3D_UTIL_RESARC},  // ライト(ゴースト)ON
  {ARCID, NARC_league_front_pl_lite4_nsbmd,      GFL_G3D_UTIL_RESARC},  // ライト(エスパー)モデル
  {ARCID, NARC_league_front_pl_lite4_off_nsbta,  GFL_G3D_UTIL_RESARC},  // ライト(エスパー)OFF
  {ARCID, NARC_league_front_pl_lite4_on_nsbta,   GFL_G3D_UTIL_RESARC},  // ライト(エスパー)ON
}; 
//------------------------
// アニメーション(リフト)
//------------------------
typedef enum {
  LIFT_ANM_ON_TA,  // テクスチャ アニメーション ( ON )
  LIFT_ANM_NUM
} LIFT_ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table_lift[LIFT_ANM_NUM] = 
{
  {RES_LIFT_ON_NSBTA,  0},  // テクスチャ アニメーション ( ON )
}; 
//------------------------
// アニメーション(ライト)
//------------------------
// 格闘
typedef enum {
  LIGHT_FIGHT_ANM_OFF,  // OFF
  LIGHT_FIGHT_ANM_ON,   // ON
  LIGHT_FIGHT_ANM_NUM
} LIGHT_FIGHT_ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table_light_fight[LIGHT_FIGHT_ANM_NUM] = 
{
  {RES_LIGHT_FIGHT_OFF_NSBTA, 0},
  {RES_LIGHT_FIGHT_ON_NSBTA,  0},
};
// 悪
typedef enum{
  LIGHT_EVIL_ANM_OFF,  // OFF
  LIGHT_EVIL_ANM_ON,   // ON
  LIGHT_EVIL_ANM_NUM
} LIGHT_EVIL_ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table_light_evil[LIGHT_EVIL_ANM_NUM] = 
{
  {RES_LIGHT_EVIL_OFF_NSBTA, 0},
  {RES_LIGHT_EVIL_ON_NSBTA,  0},
};
// ゴースト
typedef enum{
  LIGHT_GHOST_ANM_OFF,  // OFF
  LIGHT_GHOST_ANM_ON,   // ON
  LIGHT_GHOST_ANM_NUM
} LIGHT_GHOST_ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table_light_ghost[LIGHT_GHOST_ANM_NUM] = 
{
  {RES_LIGHT_GHOST_OFF_NSBTA, 0},
  {RES_LIGHT_GHOST_ON_NSBTA,  0},
};
// エスパー
typedef enum{
  LIGHT_ESPER_ANM_OFF,  // OFF
  LIGHT_ESPER_ANM_ON,   // ON
  LIGHT_ESPER_ANM_NUM
} LIGHT_ESPER_ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table_light_esper[LIGHT_ESPER_ANM_NUM] = 
{
  {RES_LIGHT_ESPER_OFF_NSBTA, 0},
  {RES_LIGHT_ESPER_ON_NSBTA,  0},
};

//-------------
// オブジェクト
//-------------
static const GFL_G3D_UTIL_OBJ obj_table[ LF01_EXOBJ_NUM ] = 
{
  {RES_LIFT_NSBMD,        0, RES_LIFT_NSBMD,        anm_table_lift,        LIFT_ANM_NUM},
  {RES_LIGHT_FIGHT_NSBMD, 0, RES_LIGHT_FIGHT_NSBMD, anm_table_light_fight, LIGHT_FIGHT_ANM_NUM},
  {RES_LIGHT_EVIL_NSBMD,  0, RES_LIGHT_EVIL_NSBMD,  anm_table_light_evil,  LIGHT_EVIL_ANM_NUM},
  {RES_LIGHT_GHOST_NSBMD, 0, RES_LIGHT_GHOST_NSBMD, anm_table_light_ghost, LIGHT_GHOST_ANM_NUM},
  {RES_LIGHT_ESPER_NSBMD, 0, RES_LIGHT_ESPER_NSBMD, anm_table_light_esper, LIGHT_ESPER_ANM_NUM},
}; 
//----------
// ユニット
//----------
static const GFL_G3D_UTIL_SETUP unit[ LF01_EXUNIT_NUM ] = 
{ 
  {res_table, RES_NUM, obj_table, LF01_EXOBJ_NUM},
};


//==========================================================================================
// ■各オブジェの配置
//========================================================================================== 
// リフト座標
#define LIFT_POS_X_GRID (32)
#define LIFT_POS_Y_GRID (0)
#define LIFT_POS_Z_GRID (42)
#define LIFT_POS_X ((LIFT_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIFT_POS_Y ((LIFT_POS_Y_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIFT_POS_Z ((LIFT_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT) 
// ライト座標(格闘)
#define LIGHT_FIGHT_POS_X_GRID (34)
#define LIGHT_FIGHT_POS_Y_GRID (0)
#define LIGHT_FIGHT_POS_Z_GRID (44)
#define LIGHT_FIGHT_POS_X ((LIGHT_FIGHT_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_FIGHT_POS_Y ((LIGHT_FIGHT_POS_Y_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_FIGHT_POS_Z ((LIGHT_FIGHT_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
// ライト座標(悪)
#define LIGHT_EVIL_POS_X_GRID (28)
#define LIGHT_EVIL_POS_Y_GRID (0)
#define LIGHT_EVIL_POS_Z_GRID (39)
#define LIGHT_EVIL_POS_X ((LIGHT_EVIL_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_EVIL_POS_Y ((LIGHT_EVIL_POS_Y_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_EVIL_POS_Z ((LIGHT_ESPER_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
// ライト座標(ゴースト)
#define LIGHT_GHOST_POS_X_GRID (28)
#define LIGHT_GHOST_POS_Y_GRID (0)
#define LIGHT_GHOST_POS_Z_GRID (44)
#define LIGHT_GHOST_POS_X ((LIGHT_GHOST_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_GHOST_POS_Y ((LIGHT_GHOST_POS_Y_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_GHOST_POS_Z ((LIGHT_GHOST_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
// ライト座標(エスパー)
#define LIGHT_ESPER_POS_X_GRID (34)
#define LIGHT_ESPER_POS_Y_GRID (0)
#define LIGHT_ESPER_POS_Z_GRID (39)
#define LIGHT_ESPER_POS_X ((LIGHT_ESPER_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_ESPER_POS_Y ((LIGHT_ESPER_POS_Y_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_ESPER_POS_Z ((LIGHT_ESPER_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)

// 配置座標
static const VecFx32 obj_pos[ LF01_EXOBJ_NUM ] = 
{
  {LIFT_POS_X,        LIFT_POS_Y,        LIFT_POS_Z},
  {LIGHT_FIGHT_POS_X, LIGHT_FIGHT_POS_Y, LIGHT_FIGHT_POS_Z},
  {LIGHT_EVIL_POS_X,  LIGHT_EVIL_POS_Y,  LIGHT_EVIL_POS_Z},
  {LIGHT_GHOST_POS_X, LIGHT_GHOST_POS_Y, LIGHT_GHOST_POS_Z},
  {LIGHT_ESPER_POS_X, LIGHT_ESPER_POS_Y, LIGHT_ESPER_POS_Z},
};


//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct
{ 
  HEAPID heapID;  // 使用するヒープID

} LF01WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void Save( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx, u32 data );
static u32 Load( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx );
static void InitGimmick( LF01WORK* work, FIELDMAP_WORK* fieldmap );
static void LoadGimmick( LF01WORK* work, FIELDMAP_WORK* fieldmap );
static void SaveGimmick( LF01WORK* work, FIELDMAP_WORK* fieldmap );


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
void LEAGUE_FRONT_01_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  LF01WORK* work;
  HEAPID heap_id;

  // ギミック管理ワーク作成
  heap_id      = FIELDMAP_GetHeapID( fieldmap );
  work         = (LF01WORK*)GMK_TMP_WK_AllocWork( fieldmap, GIMMICK_WORK_ASSIGN_ID,  heap_id, sizeof(LF01WORK) );
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
void LEAGUE_FRONT_01_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  LF01WORK* work;

  // セーブ
  work = GMK_TMP_WK_GetWork( fieldmap, GIMMICK_WORK_ASSIGN_ID );
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
void LEAGUE_FRONT_01_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  // 拡張オブジェクトのアニメ再生
  {
    FLD_EXP_OBJ_CNT_PTR exobj_cnt;
    exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
    FLD_EXP_OBJ_PlayAnime( exobj_cnt );
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
  u32*       save_work = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_C09P01 );

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
  u32*       save_work = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_C09P01 );
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
static void InitGimmick( LF01WORK* work, FIELDMAP_WORK* fieldmap )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;

  // 拡張オブジェクトのユニットを追加
  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  FLD_EXP_OBJ_AddUnitByHandle( exobj_cnt, &unit[LF01_EXUNIT_GIMMICK], LF01_EXUNIT_GIMMICK );

  // 各オブジェの配置
  {
    int obj_idx;
    GFL_G3D_OBJSTATUS* objstatus;
    for( obj_idx=0; obj_idx<LF01_EXOBJ_NUM; obj_idx++ )
    {
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, LF01_EXUNIT_GIMMICK, obj_idx );
      VEC_Set( &objstatus->trans, obj_pos[obj_idx].x, obj_pos[obj_idx].y, obj_pos[obj_idx].z );
    }
  }

  // リフトのアニメーション
  {
    GAMESYS_WORK* gsys;
    GAMEDATA* gdata;
    EVENTWORK* evwork;
    BOOL fight, evil, ghost, esper;
    // フラグチェック
    gsys   = FIELDMAP_GetGameSysWork( fieldmap );
    gdata  = GAMESYSTEM_GetGameData( gsys );
    evwork = GAMEDATA_GetEventWork( gdata );
    fight  = EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_BIGFOUR_FIGHTWIN );
    evil   = EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_BIGFOUR_EVILWIN );
    ghost  = EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_BIGFOUR_GHOSTWIN );
    esper  = EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_BIGFOUR_ESPWIN );
    // 各四天王クリアに応じたアニメを再生
    if( fight && evil && ghost && esper ) {
      FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIFT, LIFT_ANM_ON_TA, TRUE );
    }
  }

  // ライトのアニメーション
  {
    GAMESYS_WORK* gsys;
    GAMEDATA* gdata;
    EVENTWORK* evwork;
    BOOL fight, evil, ghost, esper;
    // フラグチェック
    gsys   = FIELDMAP_GetGameSysWork( fieldmap );
    gdata  = GAMESYSTEM_GetGameData( gsys );
    evwork = GAMEDATA_GetEventWork( gdata );
    fight  = EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_BIGFOUR_FIGHTWIN );
    evil   = EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_BIGFOUR_EVILWIN );
    ghost  = EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_BIGFOUR_GHOSTWIN );
    esper  = EVENTWORK_CheckEventFlag( evwork, SYS_FLAG_BIGFOUR_ESPWIN );
    // アニメ再生
    if( fight ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_FIGHT, LIGHT_FIGHT_ANM_ON,  TRUE ); }
    else       { FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_FIGHT, LIGHT_FIGHT_ANM_OFF, TRUE ); }
    if( evil  ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_EVIL,  LIGHT_EVIL_ANM_ON,   TRUE ); }
    else       { FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_EVIL,  LIGHT_EVIL_ANM_OFF,  TRUE ); }
    if( ghost ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_GHOST, LIGHT_GHOST_ANM_ON,  TRUE ); }
    else       { FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_GHOST, LIGHT_GHOST_ANM_OFF, TRUE ); }
    if( esper ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_ESPER, LIGHT_ESPER_ANM_ON,  TRUE ); }
    else       { FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, LF01_EXUNIT_GIMMICK, LF01_EXOBJ_LIGHT_ESPER, LIGHT_ESPER_ANM_OFF, TRUE ); }
  } 
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックの状態を復帰する
 *
 * @param work     復帰対象ギミックの管理ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void LoadGimmick( LF01WORK* work, FIELDMAP_WORK* fieldmap )
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
static void SaveGimmick( LF01WORK* work, FIELDMAP_WORK* fieldmap )
{
}
