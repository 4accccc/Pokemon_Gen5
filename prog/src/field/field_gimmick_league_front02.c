////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief ギミック処理(ポケモンリーグフロント下)
 * @file field_gimmick_league_front02.c
 * @author obata
 * @date 2009.12.01
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include "fieldmap.h"
#include "field_gimmick_league_front02.h"
#include "field_gimmick_def.h"  // for FLD_GIMMICK_C09P02
#include "arc/league_front.naix"  // for NARC_xxxx
#include "savedata/gimmickwork.h"  // for GIMMICKWORK


//==========================================================================================
// ■定数
//==========================================================================================
#define ARCID (ARCID_LEAGUE_FRONT_GIMMICK)  // ギミックデータのアーカイブID

// ギミックワークのデータインデックス
typedef enum{
  GIMMICKWORK_DATA_WORK_ADRS,  // LF02ギミック管理ワークのアドレス
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
  RES_LIFT_NSBMD,  // リフトのモデル
  RES_LIFT_NSBTA,  // リフトのテクスチャアニメーション
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[RES_NUM] = 
{
  {ARCID, NARC_league_front_pl_ele_00_nsbmd,    GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_ele_00_00_nsbta, GFL_G3D_UTIL_RESARC},
}; 
//------------------------
// アニメーション(リフト)
//------------------------
typedef enum{
  LIFT_ANM_TA,  // テクスチャアニメーション
  LIFT_ANM_NUM
} LIFT_ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table_lift[LIFT_ANM_NUM] = 
{
  {RES_LIFT_NSBTA, 0},
}; 
//-------------
// オブジェクト
//-------------
typedef enum{
  OBJ_LIFT,  // リフト
  OBJ_NUM
} OBJ_INDEX;
static const GFL_G3D_UTIL_OBJ obj_table[OBJ_NUM] = 
{
  {RES_LIFT_NSBMD, 0, RES_LIFT_NSBMD, anm_table_lift, LIFT_ANM_NUM},  // リフト
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
// ■ギミックワーク
//==========================================================================================
typedef struct
{ 
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

  // DEBUG:
  OBATA_Printf( "GIMMICK-LF02: setup\n" ); 

  // ギミック管理ワーク作成
  heap_id      = FIELDMAP_GetHeapID( fieldmap );
  work         = (LF02WORK*)GFL_HEAP_AllocMemory( heap_id, sizeof(LF02WORK) );
  work->heapID = heap_id; 

  // ギミック初期化
  InitGimmick( work, fieldmap );
  LoadGimmick( work, fieldmap ); 

  // LF02ギミック管理ワークのアドレスを保存
  Save( fieldmap, GIMMICKWORK_DATA_WORK_ADRS, (u32)work );

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
  work = (LF02WORK*)Load( fieldmap, GIMMICKWORK_DATA_WORK_ADRS );
  SaveGimmick( work, fieldmap );

  // ギミック管理ワーク破棄
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OBATA_Printf( "GIMMICK-LF02: end\n" );
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
  LF02WORK* work = (LF02WORK*)Load( fieldmap, GIMMICKWORK_DATA_WORK_ADRS );
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

  // DEBUG:
  OBATA_Printf( "GIMMICK-FL02: save[%d] = %d\n", idx, data );
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

  // DEBUG:
  OBATA_Printf( "GIMMICK-FL02: load[%d] = %d\n", idx, val );

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
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;

  // 拡張オブジェクトのユニットを追加
  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &unit[UNIT_GIMMICK], UNIT_GIMMICK );
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
