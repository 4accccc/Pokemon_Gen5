#include <gflib.h>
#include "fieldmap.h" 
#include "field_gimmick_h01.h"
#include "field_gimmick_def.h"
#include "gamesystem/iss_3ds_sys.h"
#include "savedata/gimmickwork.h"
#include "sound/pm_sndsys.h"
#include "system/gfl_use.h"
#include "sound_obj.h" 

#include "arc/arc_def.h"
#include "arc/h01.naix"


//==========================================================================================
// ■定数
//==========================================================================================

#define ARCID (ARCID_H01_GIMMICK) // ギミックデータのアーカイブID
#define ANIME_BUF_INTVL  (10) // アニメーションデータの読み込み間隔[frame]
#define EXPOBJ_UNIT_IDX  (0)  // フィールド拡張オブジェクトのユニット登録インデックス
#define ISS_3DS_UNIT_NUM (10) // 3Dユニット数
#define TAIL_INTERVAL    (1)  // 前部分が発射してから後部分が発射するまでの間隔

// 音源オブジェクトのインデックス
typedef enum {
  SOBJ_TRAILER_1_HEAD, // トレーラー1(前)
  SOBJ_TRAILER_1_TAIL, // トレーラー1(後)
  SOBJ_TRAILER_2_HEAD, // トレーラー2(前)
  SOBJ_TRAILER_2_TAIL, // トレーラー2(後)
  SOBJ_SHIP,           // 船
  SOBJ_NUM 
} SOBJ_INDEX;


//==========================================================================================
// ■3Dリソース
//==========================================================================================
// リソース
typedef enum {
  RES_TRAILER_HEAD_NSBMD,  // トレーラー(前)のモデル
  RES_TRAILER_TAIL_NSBMD,  // トレーラー(後)のモデル
  RES_SHIP_NSBMD,     // 船のモデル
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[RES_NUM] = 
{
  { ARCID, NARC_h01_trailer_01a_nsbmd, GFL_G3D_UTIL_RESARC },  // トレーラー(前)のモデル
  { ARCID, NARC_h01_trailer_01b_nsbmd, GFL_G3D_UTIL_RESARC },  // トレーラー(後)のモデル
  { ARCID, NARC_h01_h01_ship_nsbmd,    GFL_G3D_UTIL_RESARC },  // 船のモデル
};

// オブジェクト
typedef enum {
  OBJ_TRAILER_1_HEAD,  // トレーラー1(前)
  OBJ_TRAILER_1_TAIL,  // トレーラー1(後)
  OBJ_TRAILER_2_HEAD,  // トレーラー2(前)
  OBJ_TRAILER_2_TAIL,  // トレーラー2(後)
  OBJ_SHIP,       // 船
  OBJ_NUM
} OBJ_INDEX;
static const GFL_G3D_UTIL_OBJ obj_table[OBJ_NUM] = 
{
  { RES_TRAILER_HEAD_NSBMD, 0, RES_TRAILER_HEAD_NSBMD, NULL, 0 },  // トレーラー1(前)
  { RES_TRAILER_TAIL_NSBMD, 0, RES_TRAILER_TAIL_NSBMD, NULL, 0 },  // トレーラー1(後)
  { RES_TRAILER_HEAD_NSBMD, 0, RES_TRAILER_HEAD_NSBMD, NULL, 0 },  // トレーラー2(前)
  { RES_TRAILER_TAIL_NSBMD, 0, RES_TRAILER_TAIL_NSBMD, NULL, 0 },  // トレーラー2(後)
  { RES_SHIP_NSBMD,         0, RES_SHIP_NSBMD,         NULL, 0 },  // 船
};

// セットアップ情報
static GFL_G3D_UTIL_SETUP setup = { res_table, RES_NUM, obj_table, OBJ_NUM };


//==========================================================================================
// ■風
//==========================================================================================
typedef struct
{
  u16 trackBit;  // 操作対象トラックビット
  u16 padding;
  float minHeight;  // 音が鳴り出す高さ
  float maxHeight;  // 音が最大になる高さ

} WIND_DATA;


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
  WIND_DATA      wind_data; // 風データ
} H01WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void InitWork( H01WORK* work, FIELDMAP_WORK* fieldmap );
static void LoadWaitTime( H01WORK* work );
static void LoadWindData( H01WORK* work );
static void SaveGimmick( H01WORK* work, FIELDMAP_WORK* fieldmap );
static void LoadGimmick( H01WORK* work, FIELDMAP_WORK* fieldmap );
static void SetStandBy( H01WORK* work, SOBJ_INDEX index );
static void MoveStart( H01WORK* work, SOBJ_INDEX index );
static void UpdateWindVolume( FIELDMAP_WORK* fieldmap, H01WORK* work );


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
void H01_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  u32* gmk_save;  // ギミックの実セーブデータ
  H01WORK* work;  // H01ギミック管理ワーク
  HEAPID                heapID = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  GAMESYS_WORK*            gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*               gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*          gmkwork = GAMEDATA_GetGimmickWork(gdata);

  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &setup, EXPOBJ_UNIT_IDX );

  // ギミック管理ワークを作成
  work = (H01WORK*)GFL_HEAP_AllocMemory( heapID, sizeof(H01WORK) );

  // ギミック管理ワークを初期化 
  InitWork( work, fieldmap );

  // ロード
  LoadGimmick( work, fieldmap );

  // ギミック管理ワークのアドレスを保存
  gmk_save    = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  gmk_save[0] = (int)work;

  // DEBUG:
  OBATA_Printf( "GIMMICK-H01: set up\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void H01_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  H01WORK*         work = (H01WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 風を止める
  PMSND_ChangeBGMVolume( work->wind_data.trackBit, 0 );

  // セーブ
  SaveGimmick( work, fieldmap );

  // 音源オブジェクトを破棄
  for( i=0; i<SOBJ_NUM; i++ )
  {
    SOUNDOBJ_Delete( work->sobj[i] );
  }

  // ギミック管理ワークを破棄
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OBATA_Printf( "GIMMICK-H01: end\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 動作関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void H01_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  H01WORK*         work = (H01WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 観測者の位置を設定
  {
    FIELD_CAMERA* fieldCamera;
    VecFx32 cameraPos, targetPos;

    fieldCamera = FIELDMAP_GetFieldCamera( fieldmap );
    FIELD_CAMERA_GetCameraPos( fieldCamera, &cameraPos );
    FIELD_CAMERA_GetTargetPos( fieldCamera, &targetPos );
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
        // 後部分を前部分に追従させる
        switch(i)
        {
        case SOBJ_TRAILER_1_HEAD:
          work->wait[SOBJ_TRAILER_1_TAIL] = TAIL_INTERVAL;
          break;
        case SOBJ_TRAILER_2_HEAD:
          work->wait[SOBJ_TRAILER_2_TAIL] = TAIL_INTERVAL;
          break;
        }
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

  // 風を更新
  UpdateWindVolume( fieldmap, work );
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
static void InitWork( H01WORK* work, FIELDMAP_WORK* fieldmap )
{
  int i;
  HEAPID                heapID = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  // ヒープIDを記憶
  work->heapID = heapID;

  // 3Dサウンドシステムを取得
  {
    GAMESYS_WORK* gameSystem = FIELDMAP_GetGameSysWork( fieldmap );
    ISS_SYS*      issSystem  = GAMESYSTEM_GetIssSystem( gameSystem );
    work->iss3dsSys = ISS_SYS_GetIss3DSSystem( issSystem );
  }

  // 音源オブジェクトを作成
  { // トレーラー1(前)
    u32* data;
    GFL_G3D_OBJSTATUS* status;
    SOUNDOBJ* sobj;

    // 設定データ取得
    data = GFL_ARC_LoadDataAlloc( ARCID, NARC_h01_trailer1_head_3dsu_data_bin, heapID );
    OBATA_Printf( "trailer1 data[0] = %d\n", data[0] );
    OBATA_Printf( "trailer1 data[1] = %d\n", data[1] );

    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, 0, OBJ_TRAILER_1_HEAD );
    sobj = SOUNDOBJ_Create( fieldmap, status, ISS3DS_UNIT_TRACK09, data[0] << FX32_SHIFT, data[1] );
    SOUNDOBJ_SetAnime( sobj, ARCID, NARC_h01_trailer1_head_ica_data_bin, ANIME_BUF_INTVL );
    work->sobj[SOBJ_TRAILER_1_HEAD] = sobj;

    GFL_HEAP_FreeMemory( data );
     
  }
  { // トレーラー1(後)
    GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, 0, OBJ_TRAILER_1_TAIL );
    SOUNDOBJ* sobj = SOUNDOBJ_CreateDummy( fieldmap, status );
    SOUNDOBJ_SetAnime( sobj, ARCID, NARC_h01_trailer1_tail_ica_data_bin, ANIME_BUF_INTVL );
    work->sobj[SOBJ_TRAILER_1_TAIL] = sobj;
  }
  { // トレーラー2(前)
    u32* data;
    GFL_G3D_OBJSTATUS* status;
    SOUNDOBJ* sobj;

    // 設定データ取得
    data = GFL_ARC_LoadDataAlloc( ARCID, NARC_h01_trailer2_head_3dsu_data_bin, heapID );
    OBATA_Printf( "trailer2 data[0] = %d\n", data[0] );
    OBATA_Printf( "trailer2 data[1] = %d\n", data[1] );

    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, 0, OBJ_TRAILER_2_HEAD );
    sobj = SOUNDOBJ_Create( fieldmap, status, ISS3DS_UNIT_TRACK10, data[0] << FX32_SHIFT, data[1] );
    SOUNDOBJ_SetAnime( sobj, ARCID, NARC_h01_trailer2_head_ica_data_bin, ANIME_BUF_INTVL );
    work->sobj[SOBJ_TRAILER_2_HEAD] = sobj;

    GFL_HEAP_FreeMemory( data );
  } 
  { // トレーラー2(後)
    GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, 0, OBJ_TRAILER_2_TAIL );
    SOUNDOBJ* sobj = SOUNDOBJ_CreateDummy( fieldmap, status );
    SOUNDOBJ_SetAnime( sobj, ARCID, NARC_h01_trailer2_tail_ica_data_bin, ANIME_BUF_INTVL );
    work->sobj[SOBJ_TRAILER_2_TAIL] = sobj;
  } 
  { // 船
    u32* data;
    GFL_G3D_OBJSTATUS* status;
    SOUNDOBJ* sobj;

    // 設定データ取得
    data = GFL_ARC_LoadDataAlloc( ARCID, NARC_h01_ship_3dsu_data_bin, heapID );
    OBATA_Printf( "ship data[0] = %d\n", data[0] );
    OBATA_Printf( "ship data[1] = %d\n", data[1] );

    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, 0, OBJ_SHIP );
    sobj = SOUNDOBJ_Create( fieldmap, status, ISS3DS_UNIT_TRACK08, data[0] << FX32_SHIFT, data[1] );
    SOUNDOBJ_SetAnime( sobj, ARCID, NARC_h01_ship_ica_data_bin, ANIME_BUF_INTVL );
    work->sobj[SOBJ_SHIP] = sobj;

    GFL_HEAP_FreeMemory( data );
  }

  // 動作待機カウンタ
  for( i=0; i<SOBJ_NUM; i++ ){ work->wait[i] = 0; }

  // 待機時間
  LoadWaitTime( work );
  // 風データ
  LoadWindData( work );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 各音源オブジェクトの待ち時間を読み込む
 *
 * @param work 設定対象ワーク
 */
//------------------------------------------------------------------------------------------
static void LoadWaitTime( H01WORK* work )
{
  int i;
  ARCDATID dat_id[SOBJ_NUM] = 
  {
    NARC_h01_trailer1_head_wait_data_bin,
    NARC_h01_trailer1_tail_wait_data_bin,
    NARC_h01_trailer2_head_wait_data_bin,
    NARC_h01_trailer2_tail_wait_data_bin,
    NARC_h01_ship_wait_data_bin,
  };

  for( i=0; i<SOBJ_NUM; i++ )
  {
    u32* data = GFL_ARC_LoadDataAlloc( ARCID, dat_id[i], work->heapID );
    work->minWait[i] = data[0];
    work->maxWait[i] = data[1];
    GFL_HEAP_FreeMemory( data );
  }

  // DEBUG:
  OBATA_Printf( "GIMMICK-H01: load wait data\n" );
  for( i=0; i<SOBJ_NUM; i++ )
  {
    OBATA_Printf( "- minWait[%d] = %d\n", i, work->minWait[i] );
    OBATA_Printf( "- maxWait[%d] = %d\n", i, work->maxWait[i] );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief 風データを読み込む
 *
 * @param work 設定対象ワーク
 */
//------------------------------------------------------------------------------------------
static void LoadWindData( H01WORK* work )
{
  GFL_ARC_LoadDataOfs( &work->wind_data,  
                       ARCID, NARC_h01_wind_data_bin, 0, sizeof(WIND_DATA) );
  // DEBUG:
  {
    int i;
    OBATA_Printf( "GIMMICK-H01: load wind data\n" );
    OBATA_Printf( "- trackBit = " );
    for( i=0; i<16; i++ )
    {
      if( work->wind_data.trackBit & (1<<(16-i-1)) ) OBATA_Printf( "■" );
      else                                           OBATA_Printf( "□" );
    }
    OBATA_Printf( "\n" );
    OBATA_Printf( "- minHeight = %d\n", (int)work->wind_data.minHeight );
    OBATA_Printf( "- maxHeight = %d\n", (int)work->wind_data.maxHeight );
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
static void SaveGimmick( H01WORK* work, FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );

  // セーブ(各音源オブジェクトのアニメーションフレーム数)
  for( i=0; i<SOBJ_NUM; i++ )
  {
    fx32 frame  = SOUNDOBJ_GetAnimeFrame( work->sobj[i] );
    gmk_save[i] = (frame >> FX32_SHIFT);
    OBATA_Printf( "GIMMICK-H01: gimmick save[%d] = %d\n", i, gmk_save[i] );
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
static void LoadGimmick( H01WORK* work, FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );

  // セーブ処理
  {
    // 各音源オブジェクトのアニメーションフレーム数を復帰
    for( i=0; i<SOBJ_NUM; i++ )
    {
      fx32 frame = gmk_save[i] << FX32_SHIFT;
      SOUNDOBJ_SetAnimeFrame( work->sobj[i], frame );
      OBATA_Printf( "GIMMICK-H01: gmk load[%d] = %d\n", i, gmk_save[i] );
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
static void SetStandBy( H01WORK* work, SOBJ_INDEX index )
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
static void MoveStart( H01WORK* work, SOBJ_INDEX index )
{ 
  SOUNDOBJ_SetAnimeFrame( work->sobj[index], 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 風の音量を調整する
 *
 * @param fieldmap フィールドマップ
 * @param work     ギミック管理ワーク
 */
//--------------------------------------------------------------------------------------------
static void UpdateWindVolume( FIELDMAP_WORK* fieldmap, H01WORK* work )
{
  int volume;
  VecFx32 pos;
  float cam_y;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( fieldmap );

  // カメラ座標を取得
  FIELD_CAMERA_GetCameraPos( camera, &pos );
  cam_y = FX_FX32_TO_F32( pos.y );

  // 風の音量を算出
  if( cam_y <= work->wind_data.minHeight )
  {
    volume = 0;
  }
  else if( work->wind_data.maxHeight < cam_y )
  {
    volume = 127;
  }
  else
  {
    float    max = work->wind_data.maxHeight - work->wind_data.minHeight;
    float height = cam_y - work->wind_data.minHeight;
    volume = 127 * height / max;
    // DEBUG:
    OBATA_Printf( "GIMMICK-H01: update wind volume => %d\n", volume );
  }

  // 音量を調整
  PMSND_ChangeBGMVolume( work->wind_data.trackBit, volume );
}
