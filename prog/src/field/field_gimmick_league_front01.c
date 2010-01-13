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
#include "gamesystem/game_event.h"
#include "gamesystem/gamesystem.h"
#include "savedata/gimmickwork.h"  // for GIMMICKWORK
#include "system/ica_anime.h"  // for ICA_ANIME
#include "field_gimmick_def.h"  // for FLD_GIMMICK_C09P01
#include "field/field_const.h"  // for FIELD_CONST_GRID_SIZE
#include "fld_exp_obj.h"  // for FLD_EXP_OBJ
#include "arc/league_front.naix"  // for NARC_xxxx
#include "../../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_C09P02
#include "event_mapchange.h"  // for EVENT_ChangeMapPosNoFade
#include "event_fieldmap_control.h"  // for EVENT_FieldBrightOut
#include "field_gimmick_league_front02.h"  // for EVENT_LEAGUE_FRONT_02_GIMMICK_GetLiftDownEvent
#include "eventwork.h"  // for EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/flag_define.h"  // for SYS_FLAG_BIGFOUR_xxxx


//==========================================================================================
// ■定数
//==========================================================================================
// ギミックデータのアーカイブID
#define ARCID (ARCID_LEAGUE_FRONT_GIMMICK)  

// 四天王ライトがリフトと一緒に降下するかどうか
#define LIGHT_DOWN

// ギミックワークのデータインデックス
typedef enum{
  GIMMICKWORK_DATA_WORK_ADRS,  // LF01ギミック管理ワークのアドレス
  GIMMICKWORK_DATA_NUM,
  GIMMICKWORK_DATA_MAX = GIMMICKWORK_DATA_NUM - 1
} GIMMICKWORK_DATA_INDEX;

// リフトアップ時のパラメータ
#define LIFT_UP_FRAME (30)
#define LIFT_UP_CAMERA_X (0x000001f8 << FX32_SHIFT) 
#define LIFT_UP_CAMERA_Y (0x00000075 << FX32_SHIFT) 
#define LIFT_UP_CAMERA_Z (0x000002e8 << FX32_SHIFT)
#define LIFT_UP_TARGET_X (0x000001f8 << FX32_SHIFT) 
#define LIFT_UP_TARGET_Y (0xffffffb4 << FX32_SHIFT) 
#define LIFT_UP_TARGET_Z (0x0000027a << FX32_SHIFT)


//==========================================================================================
// ■3Dリソース
//==========================================================================================
//----------
// リソース
//----------
typedef enum{
  RES_LIFT_NSBMD,             // リフトのモデル
  RES_LIFT_ALL_NSBTA,         // 四天王(全員)　　を倒した時のita
  RES_LIFT_FIGHT_NSBTA,       // 四天王(格闘)　　を倒した時のita
  RES_LIFT_EVIL_NSBTA,        // 四天王(悪)　　　を倒した時のita
  RES_LIFT_GHOST_NSBTA,       // 四天王(ゴースト)を倒した時のita
  RES_LIFT_ESPER_NSBTA,       // 四天王(エスパー)を倒した時のita
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
  {ARCID, NARC_league_front_pl_ele_00_nsbmd,    GFL_G3D_UTIL_RESARC}, 
  {ARCID, NARC_league_front_pl_ele_00_00_nsbta, GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_ele_00_01_nsbta, GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_ele_00_02_nsbta, GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_ele_00_03_nsbta, GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_ele_00_04_nsbta, GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite1_nsbmd,     GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite1_off_nsbta, GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite1_on_nsbta,  GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite2_nsbmd,     GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite2_off_nsbta, GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite2_on_nsbta,  GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite3_nsbmd,     GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite3_off_nsbta, GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite3_on_nsbta,  GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_lite4_nsbmd,     GFL_G3D_UTIL_RESARC}, 
  {ARCID, NARC_league_front_pl_lite4_off_nsbta, GFL_G3D_UTIL_RESARC}, 
  {ARCID, NARC_league_front_pl_lite4_on_nsbta,  GFL_G3D_UTIL_RESARC}, 
}; 
//------------------------
// アニメーション(リフト)
//------------------------
typedef enum{
  LIFT_ANM_ALL,     // 四天王(全員)　　を倒した時のita
  LIFT_ANM_FIGHT,   // 四天王(格闘)　　を倒した時のita 
  LIFT_ANM_EVIL,    // 四天王(悪)　　　を倒した時のita
  LIFT_ANM_GHOST,   // 四天王(ゴースト)を倒した時のita
  LIFT_ANM_ESPER,   // 四天王(エスパー)を倒した時のita
  LIFT_ANM_NUM
} LIFT_ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table_lift[LIFT_ANM_NUM] = 
{
  {RES_LIFT_ALL_NSBTA,   0},
  {RES_LIFT_FIGHT_NSBTA, 0},
  {RES_LIFT_EVIL_NSBTA,  0},
  {RES_LIFT_GHOST_NSBTA, 0},
  {RES_LIFT_ESPER_NSBTA, 0},
}; 
//------------------------
// アニメーション(ライト)
//------------------------
// 格闘
typedef enum{
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
typedef enum{
  OBJ_LIFT,         // リフト
  OBJ_LIGHT_FIGHT,  // ライト(格闘)
  OBJ_LIGHT_EVIL,   // ライト(悪)
  OBJ_LIGHT_GHOST,  // ライト(ゴースト)
  OBJ_LIGHT_ESPER,  // ライト(エスパー)
  OBJ_NUM
} OBJ_INDEX;
static const GFL_G3D_UTIL_OBJ obj_table[OBJ_NUM] = 
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
typedef enum{
  UNIT_GIMMICK,
  UNIT_NUM
} UNIT_INDEX;
static const GFL_G3D_UTIL_SETUP unit[UNIT_NUM] = 
{ 
  {res_table, RES_NUM, obj_table, OBJ_NUM},
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
#define LIGHT_FIGHT_POS_X_GRID (28)
#define LIGHT_FIGHT_POS_Y_GRID (0)
#define LIGHT_FIGHT_POS_Z_GRID (39)
#define LIGHT_FIGHT_POS_X ((LIGHT_FIGHT_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_FIGHT_POS_Y ((LIGHT_FIGHT_POS_Y_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_FIGHT_POS_Z ((LIGHT_FIGHT_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
// ライト座標(悪)
#define LIGHT_EVIL_POS_X_GRID (34)
#define LIGHT_EVIL_POS_Y_GRID (0)
#define LIGHT_EVIL_POS_Z_GRID (39)
#define LIGHT_EVIL_POS_X ((LIGHT_EVIL_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_EVIL_POS_Y ((LIGHT_EVIL_POS_Y_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_EVIL_POS_Z ((LIGHT_EVIL_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
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
#define LIGHT_ESPER_POS_Z_GRID (44)
#define LIGHT_ESPER_POS_X ((LIGHT_ESPER_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_ESPER_POS_Y ((LIGHT_ESPER_POS_Y_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIGHT_ESPER_POS_Z ((LIGHT_ESPER_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)

// 配置座標
static const VecFx32 obj_pos[OBJ_NUM] = 
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

  // DEBUG:
  OBATA_Printf( "GIMMICK-LF01: setup\n" ); 

  // ギミック管理ワーク作成
  heap_id      = FIELDMAP_GetHeapID( fieldmap );
  work         = (LF01WORK*)GFL_HEAP_AllocMemory( heap_id, sizeof(LF01WORK) );
  work->heapID = heap_id; 

  // ギミック初期化
  InitGimmick( work, fieldmap );
  LoadGimmick( work, fieldmap ); 

  // LF01ギミック管理ワークのアドレスを保存
  Save( fieldmap, GIMMICKWORK_DATA_WORK_ADRS, (u32)work );

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
  work = (LF01WORK*)Load( fieldmap, GIMMICKWORK_DATA_WORK_ADRS );
  SaveGimmick( work, fieldmap );

  // ギミック管理ワーク破棄
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OBATA_Printf( "GIMMICK-LF01: end\n" );
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

  // DEBUG:
  OBATA_Printf( "GIMMICK-FL01: save[%d] = %d\n", idx, data );
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

  // DEBUG:
  OBATA_Printf( "GIMMICK-FL01: load[%d] = %d\n", idx, val );

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
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &unit[UNIT_GIMMICK], UNIT_GIMMICK );

  // 各オブジェの配置
  {
    int obj_idx;
    GFL_G3D_OBJSTATUS* objstatus;
    for( obj_idx=0; obj_idx<OBJ_NUM; obj_idx++ )
    {
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, UNIT_GIMMICK, obj_idx );
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
    // アニメ再生
    if( fight && evil && ghost && esper )
    { // 四天王制覇
      FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT, LIFT_ANM_ALL, TRUE );
    }
    else
    { // 各四天王クリアに応じたアニメを再生
      if( fight ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT, LIFT_ANM_FIGHT, TRUE ); }
      if( evil  ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT, LIFT_ANM_EVIL,  TRUE ); }
      if( ghost ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT, LIFT_ANM_GHOST, TRUE ); }
      if( esper ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT, LIFT_ANM_ESPER, TRUE ); }
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
    if( fight ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_FIGHT, LIGHT_FIGHT_ANM_ON,  TRUE ); }
    else       { FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_FIGHT, LIGHT_FIGHT_ANM_OFF, TRUE ); }
    if( evil  ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_EVIL,  LIGHT_EVIL_ANM_ON,   TRUE ); }
    else       { FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_EVIL,  LIGHT_EVIL_ANM_OFF,  TRUE ); }
    if( ghost ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_GHOST, LIGHT_GHOST_ANM_ON,  TRUE ); }
    else       { FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_GHOST, LIGHT_GHOST_ANM_OFF, TRUE ); }
    if( esper ){ FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_ESPER, LIGHT_ESPER_ANM_ON,  TRUE ); }
    else       { FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_ESPER, LIGHT_ESPER_ANM_OFF, TRUE ); }
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


//==========================================================================================
// ■ イベント
//==========================================================================================
typedef struct
{
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;
  ICA_ANIME* liftAnime;  // リフトの移動アニメーション

} LIFTDOWN_EVENTWORK;

//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベント処理
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT LiftDownEvent( GMEVENT* event, int* seq, void* wk )
{
  LIFTDOWN_EVENTWORK* work = (LIFTDOWN_EVENTWORK*)wk;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( work->fieldmap );

  switch( *seq )
  {
  // カメラ移動開始
  case 0:
    { // 線形カメラ設定
      FLD_CAM_MV_PARAM moveParam;
      moveParam.Chk.Shift = FALSE;
      moveParam.Chk.Pitch = FALSE;
      moveParam.Chk.Yaw = FALSE;
      moveParam.Chk.Dist = FALSE;
      moveParam.Chk.Fovy = FALSE;
      moveParam.Chk.Pos = TRUE;
      VEC_Set( &moveParam.Core.CamPos, LIFT_UP_CAMERA_X, LIFT_UP_CAMERA_Y, LIFT_UP_CAMERA_Z ); 
      VEC_Set( &moveParam.Core.TrgtPos, LIFT_UP_TARGET_X, LIFT_UP_TARGET_Y, LIFT_UP_TARGET_Z ); 
      FIELD_CAMERA_SetLinerParam( camera, &moveParam, LIFT_UP_FRAME );
    }
    (*seq)++;
    OBATA_Printf( "GIMMICK-LF01 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    break;
  // カメラ移動終了待ち
  case 1:
    if( FIELD_CAMERA_CheckMvFunc(camera) == FALSE )
    { 
      (*seq)++; 
      OBATA_Printf( "GIMMICK-LF01 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    }
    break;
  // リフト移動開始
  case 2:
    {
      HEAPID heap_id;
      heap_id = FIELDMAP_GetHeapID( work->fieldmap );
      work->liftAnime = ICA_ANIME_CreateAlloc( heap_id, ARCID, 
                                               NARC_league_front_pl_ele_00_ica_bin );
    }
    ++(*seq);
    OBATA_Printf( "GIMMICK-LF01 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    break;
  // リフト移動終了待ち
  case 3:
    // 自機, リフトの座標を更新
    {
      VecFx32 trans, pos;
      FIELD_PLAYER* player;
      GFL_G3D_OBJSTATUS* objstatus;
      FLD_EXP_OBJ_CNT_PTR exobj_cnt;
      // リフト
      ICA_ANIME_GetTranslate( work->liftAnime, &trans );
      exobj_cnt = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT );
      objstatus->trans.y = trans.y;
#ifdef LIGHT_DOWN
      // ライト
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_FIGHT );
      objstatus->trans.y = trans.y;
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_EVIL );
      objstatus->trans.y = trans.y;
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_GHOST );
      objstatus->trans.y = trans.y;
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, UNIT_GIMMICK, OBJ_LIGHT_ESPER );
      objstatus->trans.y = trans.y;
#endif
      // 自機
      player = FIELDMAP_GetFieldPlayer( work->fieldmap );
      FIELD_PLAYER_GetPos( player, &pos );
      pos.y = trans.y;
      FIELD_PLAYER_SetPos( player, &pos );
    }
    // アニメーション更新
    if( ICA_ANIME_IncAnimeFrame( work->liftAnime, FX32_ONE ) )  // if(アニメ終了)
    { 
      ICA_ANIME_Delete( work->liftAnime );
      ++(*seq); 
      OBATA_Printf( "GIMMICK-LF01 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    }
    break;
  // フェードアウト
  case 4:
    {
      GMEVENT* new_event;
      new_event = EVENT_FieldFadeOut_Black( work->gsys, work->fieldmap, FIELD_FADE_WAIT );
      GMEVENT_CallEvent( event, new_event );
    }
    ++(*seq);
    OBATA_Printf( "GIMMICK-LF01 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    break;
  // マップチェンジ
  case 5:
    {
      GMEVENT* new_event;
      VecFx32 pos;
      VEC_Set( &pos, 0, 0, 0 );
      new_event = EVENT_ChangeMapPosNoFade( work->gsys, work->fieldmap, 
                                            ZONE_ID_C09P02, &pos, DIR_DOWN );
      GMEVENT_CallEvent( event, new_event );
    }
    ++(*seq);
    OBATA_Printf( "GIMMICK-LF01 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    break;
  // 次のイベントへ切り替え
  case 6:
    {
      GMEVENT* next_event;
      next_event = LEAGUE_FRONT_02_GIMMICK_GetLiftDownEvent( work->gsys, work->fieldmap );
      GMEVENT_ChangeEvent( event, next_event );
    }
    ++(*seq);
    OBATA_Printf( "GIMMICK-LF01 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    break;
  // 終了
  case 7:
    OBATA_Printf( "GIMMICK-LF01 LIFT DOWN EVENT: seq ==> finish" );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベントを作成する
 * 
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return リフト降下イベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* LEAGUE_FRONT_01_GIMMICK_GetLiftDownEvent( GAMESYS_WORK* gsys, 
                                                   FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  LIFTDOWN_EVENTWORK* evwork;
  LF01WORK* gmkwork = (LF01WORK*)Load( fieldmap, GIMMICKWORK_DATA_WORK_ADRS );

  // 生成
  event = GMEVENT_Create( gsys, NULL, LiftDownEvent, sizeof(LIFTDOWN_EVENTWORK) );
  // 初期化
  evwork = GMEVENT_GetEventWork( event );
  evwork->gsys      = gsys;
  evwork->fieldmap  = fieldmap;
  evwork->liftAnime = NULL;
  return event;
}
