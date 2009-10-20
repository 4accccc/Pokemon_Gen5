#include <gflib.h>
#include "fieldmap.h"
#include "field_gimmick_h03.h"

#include "gamesystem/iss_3ds_unit.h"
#include "gamesystem/iss_3ds_sys.h"
#include "arc/h03.naix"
#include "sound_obj.h" 
#include "savedata/gimmickwork.h"
#include "field_gimmick_def.h"


//==========================================================================================
// ■定数
//==========================================================================================

#define ARCID (ARCID_H03_GIMMICK) // ギミックデータのアーカイブID
#define ANIME_BUF_INTVL  (10) // アニメーションデータの読み込み間隔[frame]
#define EXPOBJ_UNIT_IDX  (0)  // フィールド拡張オブジェクトのユニット登録インデックス
#define ISS_3DS_UNIT_NUM (10) // 3Dユニット数

// 音源オブジェクトのインデックス
typedef enum
{
  SOBJ_TRAIN_1, // 電車1
  SOBJ_TRAIN_2, // 電車2
  SOBJ_NUM 
} SOBJ_INDEX;


//==========================================================================================
// ■3Dリソース
//==========================================================================================
// リソース
typedef enum
{
  RES_TRAIN_NSBMD,  // 電車のモデル
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[RES_NUM] = 
{
  { ARCID, NARC_h03_h03_train1_nsbmd, GFL_G3D_UTIL_RESARC },  // 電車のモデル
};

// オブジェクト
typedef enum
{
  OBJ_TRAIN_1,  // 電車1
  OBJ_TRAIN_2,  // 電車2
  OBJ_NUM
} OBJ_INDEX;
static const GFL_G3D_UTIL_OBJ obj_table[OBJ_NUM] = 
{
  { RES_TRAIN_NSBMD, 0, 0, NULL, 0 }, // 電車1
  { RES_TRAIN_NSBMD, 0, 0, NULL, 0 }, // 電車2
};

// セットアップ情報
static GFL_G3D_UTIL_SETUP setup = { res_table, RES_NUM, obj_table, OBJ_NUM };


//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct
{ 
  HEAPID          heapID; // 使用するヒープID
  ISS_3DS_SYS* iss3dsSys; // 3Dサウンドシステム
  SOUNDOBJ*         sobj[SOBJ_NUM]; // 音源オブジェクト
} H03WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
void InitWork( H03WORK* work, FIELDMAP_WORK* fieldmap );


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
void H03_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  u32* gmk_save;  // ギミックの実セーブデータ
  H03WORK* work;  // H03ギミック管理ワーク
  HEAPID                heap_id = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  GAMESYS_WORK*            gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*               gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK*         sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*          gmkwork = SaveData_GetGimmickWork( sv );

  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &setup, EXPOBJ_UNIT_IDX );

  // ギミック管理ワークを作成
  work = (H03WORK*)GFL_HEAP_AllocMemory( heap_id, sizeof(H03WORK) );

  // ギミック管理ワークを初期化 
  InitWork( work, fieldmap );

  // セーブデータをチェック
  if( GIMMICKWORK_GetAssignID( gmkwork ) == FLD_GIMMICK_H03 )
  { // 有効なデータが存在する場合 ==> 取得
    gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H03 );
  }
  else  
  { // 初回セットアップ時 ==> 初期化
    gmk_save = (u32*)GIMMICKWORK_Assign( gmkwork, FLD_GIMMICK_H03 );
  }

  // ギミック管理ワークのアドレスを保存
  gmk_save[0] = (int)work;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void H03_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*  gmkwork = SaveData_GetGimmickWork( sv );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H03 );
  H03WORK*         work = (H03WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // セーブ

  // 音源オブジェクトを破棄
  for( i=0; i<SOBJ_NUM; i++ )
  {
    SOUNDOBJ_Delete( work->sobj[i] );
  }

  // 3Dサウンドシステムを破棄
  ISS_3DS_SYS_Delete( work->iss3dsSys );

  // ギミック管理ワークを破棄
  GFL_HEAP_FreeMemory( work );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 動作関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void H03_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*  gmkwork = SaveData_GetGimmickWork( sv );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H03 );
  H03WORK*         work = (H03WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 音源オブジェクトを動かす
  for( i=0; i<SOBJ_NUM; i++ )
  { 
    SOUNDOBJ_IncAnimeFrame( work->sobj[i], FX32_ONE );
  }

  // 3Dサウンドシステム動作
  ISS_3DS_SYS_Main( work->iss3dsSys );
}


//==========================================================================================
// ■ 非公開関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief ギミック管理ワークを初期化する
 *
 * @param work     初期化対象ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
void InitWork( H03WORK* work, FIELDMAP_WORK* fieldmap )
{
  HEAPID                heap_id = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  // ヒープIDを記憶
  work->heapID = heap_id;

  // 3Dサウンドシステムを作成
  {
    FIELD_CAMERA*   field_camera = FIELDMAP_GetFieldCamera( fieldmap );
    const GFL_G3D_CAMERA* camera = FIELD_CAMERA_GetCameraPtr( field_camera );
    work->iss3dsSys = ISS_3DS_SYS_Create( heap_id, ISS_3DS_UNIT_NUM, camera );
  }

  // 音源オブジェクトを作成
  { // 電車1
    GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, 0, OBJ_TRAIN_1 );
    SOUNDOBJ* sobj = SOUNDOBJ_Create( fieldmap, work->iss3dsSys, status );
    SOUNDOBJ_SetAnime( sobj, ARCID, NARC_h03_train1_ica_data_bin, ANIME_BUF_INTVL );
    SOUNDOBJ_Set3DSUnitStatus( sobj, ARCID, NARC_h03_train1_3ds_unit_data_bin );
    work->sobj[SOBJ_TRAIN_1] = sobj;
  }
  {
    // 電車2
    GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, 0, OBJ_TRAIN_2 );
    SOUNDOBJ* sobj = SOUNDOBJ_Create( fieldmap, work->iss3dsSys, status );
    SOUNDOBJ_SetAnime( sobj, ARCID, NARC_h03_train2_ica_data_bin, ANIME_BUF_INTVL );
    SOUNDOBJ_Set3DSUnitStatus( sobj, ARCID, NARC_h03_train2_3ds_unit_data_bin );
    work->sobj[SOBJ_TRAIN_2] = sobj;
  } 
}
