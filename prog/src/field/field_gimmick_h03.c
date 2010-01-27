#include <gflib.h>
#include "fieldmap.h"
#include "field_gimmick_h03.h" 
#include "gamesystem/iss_3ds_sys.h"
#include "sound_obj.h" 
#include "savedata/gimmickwork.h"
#include "field_gimmick_def.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "arc/h03.naix"


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
  HEAPID            heapID; // 使用するヒープID
  ISS_3DS_SYS*   iss3dsSys; // 3Dサウンドシステム
  SOUNDOBJ* sobj[SOBJ_NUM]; // 音源オブジェクト
  int       wait[SOBJ_NUM]; // 音源オブジェクトの動作待機カウンタ
  int    minWait[SOBJ_NUM]; // 最短待ち時間
  int    maxWait[SOBJ_NUM]; // 最長待ち時間
} H03WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void InitWork( H03WORK* work, FIELDMAP_WORK* fieldmap );
static void LoadWaitTime( H03WORK* work );
static void SaveGimmick( H03WORK* work, FIELDMAP_WORK* fieldmap );
static void LoadGimmick( H03WORK* work, FIELDMAP_WORK* fieldmap );
static void SetStandBy( H03WORK* work, SOBJ_INDEX index );
static void MoveStart( H03WORK* work, SOBJ_INDEX index );


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
  GIMMICKWORK*          gmkwork = GAMEDATA_GetGimmickWork(gdata);

  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &setup, EXPOBJ_UNIT_IDX );

  // ギミック管理ワークを作成
  work = (H03WORK*)GFL_HEAP_AllocMemory( heap_id, sizeof(H03WORK) );

  // ギミック管理ワークを初期化 
  InitWork( work, fieldmap );

  // ロード
  LoadGimmick( work, fieldmap );

  // ギミック管理ワークのアドレスを保存(セーブデータをクリア)
  gmk_save    = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H03 );
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
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H03 );
  H03WORK*         work = (H03WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // セーブ
  SaveGimmick( work, fieldmap );

  // 音源オブジェクトを破棄
  for( i=0; i<SOBJ_NUM; i++ )
  {
    SOUNDOBJ_Delete( work->sobj[i] );
  }

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
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H03 );
  H03WORK*         work = (H03WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 観測者の位置を設定
  {
    FIELD_CAMERA* fieldCamera;
    const GFL_G3D_CAMERA* g3dCamera;
    VecFx32 cameraPos, targetPos;

    fieldCamera = FIELDMAP_GetFieldCamera( fieldmap );
    g3dCamera   = FIELD_CAMERA_GetCameraPtr( fieldCamera );
    GFL_G3D_CAMERA_GetPos( g3dCamera, &cameraPos );
    GFL_G3D_CAMERA_GetTarget( g3dCamera, &targetPos );
    ISS_3DS_SYS_SetObserverPos( work->iss3dsSys, &cameraPos, &targetPos );
  }

  // すべての音源オブジェクトを動かす
  for( i=0; i<SOBJ_NUM; i++ )
  { 
    // 待機状態 ==> 発車カウントダウン
    if( 0 < work->wait[i] )
    {
      // カウントダウン終了 ==> 発車
      if( --work->wait[i] <= 0 )
      {
        MoveStart( work, i );
      }
    }
    // 動作中 ==> アニメーションを更新
    else
    {
      // アニメーション再生が終了 ==> 待機状態へ
      if( SOUNDOBJ_IncAnimeFrame( work->sobj[i], FX32_ONE ) )
      {
        SetStandBy( work, i );
      }
    }
  }
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
static void InitWork( H03WORK* work, FIELDMAP_WORK* fieldmap )
{
  int i;
  HEAPID                heapID = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exObjCnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  // ヒープIDを記憶
  work->heapID = heapID;

  // 3Dサウンドシステムを取得
  {
    GAMESYS_WORK* gameSystem = FIELDMAP_GetGameSysWork( fieldmap );
    ISS_SYS*      issSystem  = GAMESYSTEM_GetIssSystem( gameSystem );
    work->iss3dsSys = ISS_SYS_GetIss3DSSystem( issSystem );
  }

  // 音源オブジェクトを作成
  {
    // 電車1
    { 
      u32* data;
      GFL_G3D_OBJSTATUS* status;
      SOUNDOBJ* sobj;

      // 設定データ取得
      data = GFL_ARC_LoadDataAlloc( ARCID, NARC_h03_train1_3ds_unit_data_bin, heapID );
      OBATA_Printf( "train1 data[0] = %d\n", data[0] );
      OBATA_Printf( "train1 data[1] = %d\n", data[1] );

      // 作成
      status = FLD_EXP_OBJ_GetUnitObjStatus( exObjCnt, 0, OBJ_TRAIN_1 );
      sobj = SOUNDOBJ_Create( fieldmap, status, ISS3DS_UNIT_TRACK09, data[0] << FX32_SHIFT, data[1] );
      SOUNDOBJ_SetAnime( sobj, ARCID, NARC_h03_train1_ica_data_bin, ANIME_BUF_INTVL );
      work->sobj[ SOBJ_TRAIN_1 ] = sobj;

      GFL_HEAP_FreeMemory( data );
    }
    // 電車2
    {
      u32* data;
      GFL_G3D_OBJSTATUS* status;
      SOUNDOBJ* sobj;

      // 設定データ取得
      data = GFL_ARC_LoadDataAlloc( ARCID, NARC_h03_train2_3ds_unit_data_bin, heapID );
      OBATA_Printf( "train2 data[0] = %d\n", data[0] );
      OBATA_Printf( "train2 data[1] = %d\n", data[1] );

      // 作成
      status = FLD_EXP_OBJ_GetUnitObjStatus( exObjCnt, 0, OBJ_TRAIN_2 );
      sobj = SOUNDOBJ_Create( fieldmap, status, ISS3DS_UNIT_TRACK10, data[0] << FX32_SHIFT, data[1] );
      SOUNDOBJ_SetAnime( sobj, ARCID, NARC_h03_train2_ica_data_bin, ANIME_BUF_INTVL );
      work->sobj[ SOBJ_TRAIN_2 ] = sobj;

      GFL_HEAP_FreeMemory( data );
    } 
  }

  // 動作待機カウンタ
  for( i=0; i<SOBJ_NUM; i++ ){ work->wait[i] = 0; }

  // 待機時間
  LoadWaitTime( work );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 各音源オブジェクトの待ち時間を読み込む
 *
 * @param work 設定対象ワーク
 */
//------------------------------------------------------------------------------------------
static void LoadWaitTime( H03WORK* work )
{
  int i;
  ARCDATID dat_id[SOBJ_NUM] = 
  {
    NARC_h03_train1_wait_data_bin,
    NARC_h03_train2_wait_data_bin,
  };

  for( i=0; i<SOBJ_NUM; i++ )
  {
    u32* data = GFL_ARC_LoadDataAlloc( ARCID, dat_id[i], work->heapID );
    work->minWait[i] = data[0];
    work->maxWait[i] = data[1];
    GFL_HEAP_FreeMemory( data );
    // DEBUG:
    //OBATA_Printf( "minWait[%d] = %d\n", i, work->minWait[i] );
    //OBATA_Printf( "maxWait[%d] = %d\n", i, work->maxWait[i] );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックの状態を保存する
 *
 * @param work     保存対象ギミックの管理ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void SaveGimmick( H03WORK* work, FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H03 );

  // セーブ(各音源オブジェクトのアニメーションフレーム数)
  for( i=0; i<SOBJ_NUM; i++ )
  {
    fx32 frame  = SOUNDOBJ_GetAnimeFrame( work->sobj[i] );
    gmk_save[i] = (frame >> FX32_SHIFT);
    OBATA_Printf( "SaveGimmick: gmk_save[%d] = %d\n", i, gmk_save[i] );
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
static void LoadGimmick( H03WORK* work, FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H03 );

  // セーブ処理
  {
    // 各音源オブジェクトのアニメーションフレーム数を復帰
    for( i=0; i<SOBJ_NUM; i++ )
    {
      fx32 frame = gmk_save[i] << FX32_SHIFT;
      SOUNDOBJ_SetAnimeFrame( work->sobj[i], frame );
      OBATA_Printf( "LoadGimmick: gmk_save[%d] = %d\n", i, gmk_save[i] );
      OBATA_Printf( "LoadGimmick: frame = %x\n", i, frame );
    }
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェクトを待機状態にする
 *
 * @param work  操作対象ワーク
 * @param index 待機状態にするオブジェクトを指定
 */
//------------------------------------------------------------------------------------------
static void SetStandBy( H03WORK* work, SOBJ_INDEX index )
{
  u32 range = work->maxWait[index] - work->minWait[index] + 1;      // 幅 = 最長 - 最短
  u32 time  = work->minWait[index] + GFUser_GetPublicRand0(range);  // 待ち時間 = 最短 + 幅
  work->wait[index] = time;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェクトを動作状態にする
 *
 * @param work  操作対象ワーク
 * @param index 待機状態にするオブジェクトを指定
 */
//------------------------------------------------------------------------------------------
static void MoveStart( H03WORK* work, SOBJ_INDEX index )
{ 
  SOUNDOBJ_SetAnimeFrame( work->sobj[index], 0 );
}
