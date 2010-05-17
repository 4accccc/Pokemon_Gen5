/////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ギミック登録関数 ( H01 )
 * @file   field_gimmick_h01.c
 * @author obata
 * @date   2009.10.21
 */
/////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h" 
#include "field_gimmick_h01.h"
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"
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
#define GIMMICK_WORK_ASSIGN_ID (0) // ギミックワークのアサインID
#define WIND_FADE_SPEED (16) // 風の音量のフェード速度
#define WIND_MIN_VOLUME (0)   // 風の最小ボリューム
#define WIND_MAX_VOLUME (127) // 風の最大ボリューム

#define PAUL_SOUND_PLAY_SPEED ( FX32_ONE*100 ) // 通過音を出すスピード
#define PAUL_SOUND_Y_TMP_NUM ( 32 ) // 通過音　Y座標保持数
#define PAUL_SOUND_NOTSE_STOP_POS_Z ( 1816<<FX32_SHIFT )        // 通過音をとめなくする座標
#define PAUL_SOUND_PLAY_POS_Z_RUN ( 1884<<FX32_SHIFT )        // 通過音を出すZ座標
#define PAUL_SOUND_PLAY_POS_Z_CYCLE ( 1980<<FX32_SHIFT )        // 通過音を出すZ座標
#define PAUL_SOUND_PLAY_SE_RUN ( SEQ_SE_FLD_173 )        // 通過音 はしり
#define PAUL_SOUND_PLAY_SE_CYCLE ( SEQ_SE_FLD_171 )      // 通過音 自転車
#define PAUL_SOUND_FADE_SPEED ( 16 )      // フェードアウト　スピード　減算値
#define PAUL_SOUND_FADE_START ( 127 )     // フェードアウト開始　ボリューム

//柱通過音SEPlayタイプ
enum{
  PAUL_SOUND_PLAY_NONE,
  PAUL_SOUND_PLAY_PLAY,
  PAUL_SOUND_PLAY_FADEOUT,
};


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
  RES_SHIP_NSBMD,          // 船のモデル
  RES_SHIP_NSBTA,          // 船のita
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[RES_NUM] = 
{
  { ARCID, NARC_h01_trailer_01a_nsbmd, GFL_G3D_UTIL_RESARC },  // トレーラー(前)のモデル
  { ARCID, NARC_h01_trailer_01b_nsbmd, GFL_G3D_UTIL_RESARC },  // トレーラー(後)のモデル
  { ARCID, NARC_h01_h01_ship_nsbmd,    GFL_G3D_UTIL_RESARC },  // 船のモデル
  { ARCID, NARC_h01_h01_ship_nsbta,    GFL_G3D_UTIL_RESARC },  // 船のita
};

// アニメーション
typedef enum {
  SHIP_ANIME_ITA,
  SHIP_ANIME_NUM
} SHIP_ANIME;
static const GFL_G3D_UTIL_ANM ship_anime[ SHIP_ANIME_NUM ] = 
{
  { RES_SHIP_NSBTA, 0 }, // SHIP_ANIME_ITA
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
  { RES_SHIP_NSBMD,         0, RES_SHIP_NSBMD,         ship_anime, SHIP_ANIME_NUM },  // 船
};

// セットアップ情報
static GFL_G3D_UTIL_SETUP setup = { res_table, RES_NUM, obj_table, OBJ_NUM };


//==========================================================================================
// ■風
//==========================================================================================
typedef struct {

  u16 trackBit;     // 操作対象トラックビット
  u16 padding;
  float minHeight;  // 音が鳴り出す高さ
  float maxHeight;  // 音が最大になる高さ

} WIND_DATA;


//==========================================================================================
// ■柱の間と通る音
//==========================================================================================
typedef struct {

  fx32 last_z;
  fx32 tmp_y[ PAUL_SOUND_Y_TMP_NUM ];
  s16  tmp_start_index;
  s16  tmp_count;

  // Volume fade out
  s16 se_fadeout;
  u16 se_play;
  u16 se_no;

} PAUL_SOUND;


//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct { 

  HEAPID       heapID;            // 使用するヒープID
  ISS_3DS_SYS* iss3dsSys;         // 3Dサウンドシステム
  SOUNDOBJ*    sobj[SOBJ_NUM];    // 音源オブジェクト
  int          wait[SOBJ_NUM];    // 音源オブジェクトの動作待機カウンタ
  int          minWait[SOBJ_NUM]; // 最短待ち時間
  int          maxWait[SOBJ_NUM]; // 最長待ち時間
  WIND_DATA    wind_data;         // 風データ
	int          wind_volume;       // 風の音量
  PAUL_SOUND   paul_sound;        // 柱の間を通る音

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
// 風
static void ApplyWindVolumeForBGM( const H01WORK* work ); // 風の音量をBGMに反映させる
static void UpdateWindVolume( H01WORK* work, FIELDMAP_WORK* fieldmap ); // 風の音量を更新する
static int CalcWindVolume( const H01WORK* work, FIELDMAP_WORK* fieldmap ); // 風の音量 ( 目標値 ) を計算する
static void WindVolumeUp( H01WORK* work, int max_volume ); // 風の音量を上げる
static void WindVolumeDown( H01WORK* work, int min_volume ); // 風の音量を下げる
// 柱の間を通る
static void InitPaulSound( PAUL_SOUND* work );
static void UpdatePaulSound( PAUL_SOUND* work, const FIELD_CAMERA* camera, const FIELD_PLAYER* cp_player );


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
  H01WORK* work;  // H01ギミック管理ワーク
  HEAPID              heapID    = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnitByHandle( exobj_cnt, &setup, EXPOBJ_UNIT_IDX );

  // 船のアニメーション開始
  {
    EXP_OBJ_ANM_CNT_PTR exobj_anm; 
    FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_SHIP, SHIP_ANIME_ITA, TRUE ); 
    exobj_anm = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_SHIP, SHIP_ANIME_ITA );
    FLD_EXP_OBJ_ChgAnmStopFlg( exobj_anm, FALSE );
    FLD_EXP_OBJ_ChgAnmLoopFlg( exobj_anm, TRUE );
  }

  // ギミック管理ワークを作成
  work = (H01WORK*)GMK_TMP_WK_AllocWork( fieldmap, GIMMICK_WORK_ASSIGN_ID, heapID, sizeof(H01WORK) );

  // ギミック管理ワークを初期化 
  InitWork( work, fieldmap );

  // ロード
  LoadGimmick( work, fieldmap );

  // 橋ISSのマスターボリュームを復帰
  ISS_3DS_SYS_SetMasterVolume( work->iss3dsSys, 127 );
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
  H01WORK* work = (H01WORK*)GMK_TMP_WK_GetWork( fieldmap, GIMMICK_WORK_ASSIGN_ID );

  // 橋ISSのマスターボリュームを絞る
  ISS_3DS_SYS_SetMasterVolume( work->iss3dsSys, 0 );

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
  GMK_TMP_WK_FreeWork( fieldmap, GIMMICK_WORK_ASSIGN_ID );
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
  H01WORK* work = (H01WORK*)GMK_TMP_WK_GetWork( fieldmap, GIMMICK_WORK_ASSIGN_ID );
  FIELD_CAMERA* fieldCamera;
  fieldCamera = FIELDMAP_GetFieldCamera( fieldmap );

  // 観測者の位置を設定
  {
    VecFx32 cameraPos, targetPos;

    FIELD_CAMERA_GetCameraPos( fieldCamera, &cameraPos );
    FIELD_CAMERA_GetTargetPos( fieldCamera, &targetPos );
    ISS_3DS_SYS_SetObserverPos( work->iss3dsSys, &cameraPos, &targetPos );
  }

  // すべての音源オブジェクトを動かす
  for( i=0; i<SOBJ_NUM; i++ )
  { 
    // 待機状態 ==> 発車カウントダウン
    if( 0 < work->wait[i] ) {
      // カウントダウン終了 ==> 発車
      if( --work->wait[i] <= 0 ) {
        MoveStart( work, i );
        // 後部分を前部分に追従させる
        switch(i) {
        case SOBJ_TRAILER_1_HEAD: work->wait[SOBJ_TRAILER_1_TAIL] = TAIL_INTERVAL; break;
        case SOBJ_TRAILER_2_HEAD: work->wait[SOBJ_TRAILER_2_TAIL] = TAIL_INTERVAL; break;
        }
      }
    }
    // 動作中 ==> アニメーションを更新
    else {
      // アニメーション再生が終了 ==> 待機状態へ
      if( SOUNDOBJ_IncAnimeFrame( work->sobj[i], FX32_ONE ) ) {
        SetStandBy( work, i );
      }
    }
  }

  // 風を更新
  UpdateWindVolume( work, fieldmap );
	ApplyWindVolumeForBGM( work );

  // 拡張オブジェのアニメーション再生
  {
    FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
    FLD_EXP_OBJ_PlayAnime( exobj_cnt );
  }

  // 柱の間音データ
  UpdatePaulSound(&work->paul_sound, fieldCamera, FIELDMAP_GetFieldPlayer( fieldmap ));

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

  work->heapID = heapID;
	work->wind_volume = 0;

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
  // 柱の間音データ
  InitPaulSound(&work->paul_sound);
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
 * @brief 風の音量をBGMに反映させる
 *
 * @param work
 */
//--------------------------------------------------------------------------------------------
static void ApplyWindVolumeForBGM( const H01WORK* work )
{
  PMSND_ChangeBGMVolume( work->wind_data.trackBit, work->wind_volume );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 風の音量を調整する
 *
 * @param fieldmap フィールドマップ
 * @param work     ギミック管理ワーク
 */
//--------------------------------------------------------------------------------------------
static void UpdateWindVolume( H01WORK* work, FIELDMAP_WORK* fieldmap )
{
  int volume;

  // 風の音量を算出
	volume = CalcWindVolume( work, fieldmap );

  // 音量を調整 ( フェード )
	if( work->wind_volume < volume ) {
		WindVolumeUp( work, volume );
	}
	else if( volume < work->wind_volume ) {
		WindVolumeDown( work, volume );
	} 
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 風の音量 ( 目標値 ) を計算する
 *
 * @param work     ギミック管理ワーク
 * @param fieldmap 
 *
 * @return 現在の状況における風の音量
 */
//--------------------------------------------------------------------------------------------
static int CalcWindVolume( const H01WORK* work, FIELDMAP_WORK* fieldmap )
{
  int volume;
  float cam_y;

  // カメラ座標を取得
	{
		FIELD_CAMERA* camera;
		VecFx32 pos;

		camera = FIELDMAP_GetFieldCamera( fieldmap );
		FIELD_CAMERA_GetCameraPos( camera, &pos );
		cam_y = FX_FX32_TO_F32( pos.y );
	}

  // 音量を算出
  if( cam_y <= work->wind_data.minHeight ) {
		// 最小
    volume = WIND_MIN_VOLUME;
  }
  else if( work->wind_data.maxHeight < cam_y ) {
		// 最大
    volume = WIND_MAX_VOLUME;
  }
  else {
		// 補間
    float max = work->wind_data.maxHeight - work->wind_data.minHeight;
    float now = cam_y - work->wind_data.minHeight;
    volume = WIND_MIN_VOLUME + (WIND_MAX_VOLUME - WIND_MIN_VOLUME) * now / max;
  }

	return volume;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 風の音量を上げる
 *
 * @param work       ギミック管理ワーク
 * @param max_volume 最大音量
 */
//--------------------------------------------------------------------------------------------
static void WindVolumeUp( H01WORK* work, int max_volume )
{
	GF_ASSERT( work->wind_volume < max_volume );

	work->wind_volume += WIND_FADE_SPEED;

	// 最大値補正
	if( max_volume < work->wind_volume ) {
		work->wind_volume = max_volume;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 風の音量を下げる
 *
 * @param work       ギミック管理ワーク
 * @param min_volume 最小音量
 */
//--------------------------------------------------------------------------------------------
static void WindVolumeDown( H01WORK* work, int min_volume )
{
	GF_ASSERT( min_volume < work->wind_volume );

	work->wind_volume -= WIND_FADE_SPEED;

	// 最小値補正
	if( work->wind_volume < min_volume ) {
		work->wind_volume = min_volume;
	}
}



//----------------------------------------------------------------------------
/**
 *	@brief  柱の間を通る音　初期化
 *
 *	@param	work  ワーク
 */
//-----------------------------------------------------------------------------
static void InitPaulSound( PAUL_SOUND* work )
{
  GFL_STD_MemClear32( work, sizeof(PAUL_SOUND) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  柱の間を通る音　更新処理
 *
 *	@param	work      ワーク
 *	@param	camera    カメラ
 */
//-----------------------------------------------------------------------------
static void UpdatePaulSound( PAUL_SOUND* work, const FIELD_CAMERA* camera, const FIELD_PLAYER* cp_player )
{
  VecFx32 camera_pos;
  fx32 dist_z;
  fx32 check_z;
  int se;
  u32 draw_form;


  // SEの停止チェック
  if( (work->se_play == PAUL_SOUND_PLAY_PLAY) || (work->se_play == PAUL_SOUND_PLAY_FADEOUT) ){
    if( PMSND_CheckPlaySE_byPlayerID( PMSND_GetSE_DefaultPlayerID( SEQ_SE_FLD_171 ) ) == FALSE ){
      work->se_play = PAUL_SOUND_PLAY_NONE;
    }
    
    // フェードアウト処理
    if( work->se_play == PAUL_SOUND_PLAY_FADEOUT ){
      work->se_fadeout -= PAUL_SOUND_FADE_SPEED;
      if( work->se_fadeout < 0 ){
        PMSND_StopSE_byPlayerID( PMSND_GetSE_DefaultPlayerID( work->se_no ) );// 停止
        work->se_play = PAUL_SOUND_PLAY_NONE; //フェードアウト完了
      }else{
        PMSND_PlayerSetVolume( PMSND_GetSE_DefaultPlayerID( work->se_no ), work->se_fadeout );
      }

    }
  }
  

  // 移動スピード計測
  FIELD_CAMERA_GetCameraPos( camera, &camera_pos );
  dist_z = MATH_ABS( camera_pos.z - work->last_z );
  if( dist_z > 0 ){
    // 移動フレーム。
    s32 set_index;

    if( work->tmp_count < PAUL_SOUND_Y_TMP_NUM ){
      set_index = (work->tmp_start_index + work->tmp_count) % PAUL_SOUND_Y_TMP_NUM;
      work->tmp_count ++;
    }else{
      set_index = work->tmp_start_index;
      work->tmp_start_index = (work->tmp_start_index + 1) % PAUL_SOUND_Y_TMP_NUM;
    }

    work->tmp_y[ set_index ] = camera_pos.y;
    //TOMOYA_Printf( "set index %d   count %d\n", set_index, work->tmp_count );
  }else{
    // 移動なしフレーム
    // 全クリア
    work->tmp_count = 0;
    work->tmp_start_index = 0;

    // SEフェードアウト開始
    if( (work->se_play == PAUL_SOUND_PLAY_PLAY) && (camera_pos.z > PAUL_SOUND_NOTSE_STOP_POS_Z) ){
      work->se_play = PAUL_SOUND_PLAY_FADEOUT;
      work->se_fadeout = PAUL_SOUND_FADE_START;
      TOMOYA_Printf( "fadeout On z %d\n", camera_pos.z>>FX32_SHIFT );
    }
    //TOMOYA_Printf( "tmp y clear\n" );
  }


  
  // カメラ位置が鳴らす場所を通過したかチェック。
  if( work->tmp_count > 0 ){

    // 移動スピードから、走り用と自転車用を切り替える。
    draw_form = FIELD_PLAYER_GetDrawForm( cp_player );
    if( draw_form == PLAYER_DRAW_FORM_NORMAL  ){
      check_z = PAUL_SOUND_PLAY_POS_Z_RUN;
      se     = PAUL_SOUND_PLAY_SE_RUN;
    }else{
      check_z = PAUL_SOUND_PLAY_POS_Z_CYCLE;
      se     = PAUL_SOUND_PLAY_SE_CYCLE;
    }

    // 通過したときにだけ鳴らす。
    if( ((camera_pos.z < check_z) && (work->last_z > check_z)) || 
        ((camera_pos.z > check_z) && (work->last_z < check_z)) ){
      fx32 dist;

      // 一番昔の座標からどのくらいかわったのか？
      dist = work->tmp_y[ work->tmp_start_index ] - camera_pos.y;
    
      TOMOYA_Printf( "camera_speed 0x%x\n", dist );
      if( dist >= PAUL_SOUND_PLAY_SPEED ){
        PMSND_PlaySE( se );
        work->se_play = PAUL_SOUND_PLAY_PLAY;
        work->se_no = se;
        TOMOYA_Printf( "sound on\n" );
      }
    }
  }

  work->last_z = camera_pos.z;
}

