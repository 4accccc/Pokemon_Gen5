#include <gflib.h>
#include "debug_obata_3d_sound_setup.h"
#include "system/main.h"
#include "arc/arc_def.h"
#include "arc/debug_obata.naix"
#include "gamesystem/iss_3ds_sys.h"
#include "sound/pm_sndsys.h"


// BGMトラック番号マスク
#define TRACK_MASK_WIND (1<<(7-1))
#define TRACK_MASK_SHIP (1<<(8-1))
#define TRACK_MASK_CAR_1 (1<<(9-1))
#define TRACK_MASK_CAR_2 (1<<(10-1))


//============================================================================================
/**
 * @brief 3Dデータ
 */
//============================================================================================

// トレーラ
static const GFL_G3D_UTIL_RES res_table_tr[] = 
{
  { ARCID_OBATA_DEBUG, NARC_debug_obata_trailer_nsbmd, GFL_G3D_UTIL_RESARC },
};
static const GFL_G3D_UTIL_OBJ obj_table_tr[] = 
{
  {
    0,    // モデルリソースID
    0,    // モデルデータID(リソース内部INDEX)
    0,    // テクスチャリソースID
    NULL, // アニメテーブル(複数指定のため)
    0,    // アニメリソース数
  },
}; 

// 船
static const GFL_G3D_UTIL_RES res_table_ship[] = 
{
  { ARCID_OBATA_DEBUG, NARC_debug_obata_h01_ship_nsbmd, GFL_G3D_UTIL_RESARC },
};
static const GFL_G3D_UTIL_OBJ obj_table_ship[] = 
{
  {
    0,    // モデルリソースID
    0,    // モデルデータID(リソース内部INDEX)
    0,    // テクスチャリソースID
    NULL, // アニメテーブル(複数指定のため)
    0,    // アニメリソース数
  },
}; 

// 家
static const GFL_G3D_UTIL_RES res_table_house[] = 
{
  { ARCID_OBATA_DEBUG, NARC_debug_obata_t1_house_01_nsbmd, GFL_G3D_UTIL_RESARC },
};
static const GFL_G3D_UTIL_OBJ obj_table_house[] = 
{
  {
    0,    // モデルリソースID
    0,    // モデルデータID(リソース内部INDEX)
    0,    // テクスチャリソースID
    NULL, // アニメテーブル(複数指定のため)
    0,    // アニメリソース数
  },
}; 

// セットアップ番号
enum
{
  SETUP_INDEX_TRAILER,
  SETUP_INDEX_SHIP,
  SETUP_INDEX_HOUSE,
  SETUP_INDEX_MAX
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP setup[] =
{
  { res_table_tr, NELEMS(res_table_tr), obj_table_tr, NELEMS(obj_table_tr) },
  { res_table_ship, NELEMS(res_table_ship), obj_table_ship, NELEMS(obj_table_ship) },
  { res_table_house, NELEMS(res_table_house), obj_table_house, NELEMS(obj_table_house) },
};


//============================================================================================
/**
 * @brief 定数
 */
//============================================================================================
// テストモード
typedef enum
{
  TESTMODE_ALL,  // 全表示
  TESTMODE_TR,   // トレーラーのみ
  TESTMODE_TR1,  // トレーラ1のみ
  TESTMODE_TR2,  // トレーラ2のみ
  TESTMODE_SHIP, // 船のみ
  TESTMODE_NUM
} TESTMODE;

// 音源オブジェクト番号
typedef enum
{
  SOUNDOBJ_TR_1,
  SOUNDOBJ_TR_2,
  SOUNDOBJ_SHIP,
  SOUNDOBJ_NUM
} SOUNDOBJ_INDEX;

//============================================================================================
/**
 * @breif オブジェクト(トレーラ・船など)
 */
//============================================================================================
typedef struct
{
  BOOL              active; // 表示フラグ
  GFL_G3D_OBJ*         obj; // 描画オブジェクト
  GFL_G3D_OBJSTATUS status; // ステータス
  //ISS_3DS_UNIT* iss3dsUnit; // 3Dサウンドユニット
  u16                frame; // 現在フレーム
  u16             endFrame; // 最終フレーム
  VecFx32         startPos; // 開始位置
  VecFx32           endPos; // 最終位置 
}
SOUNDOBJ;


//============================================================================================
/**
 * @breif ワーク
 */
//============================================================================================
typedef struct
{
  GFL_G3D_UTIL* g3dUtil;
  u16 unitIndex[ SETUP_INDEX_MAX ];

  // カメラ
  GFL_G3D_CAMERA* camera;
  GFL_G3D_CAMERA* dammyCam;

  // 3Dサウンドシステム
  ISS_3DS_SYS* iss3dsSys;

  // トレーラー
  SOUNDOBJ soundObj[SOUNDOBJ_NUM];

  // プレイヤー位置
  VecFx32 playerPos;

  // カメラ位置オフセット
  VecFx32 camPosOffset;

  // テストモード
  TESTMODE mode;
}
PROC_WORK;


//============================================================================================
/**
 * @brief プロトタイプ宣言
 */
//============================================================================================
static void Initialize( PROC_WORK* work );
static void Finalize( PROC_WORK* work );
static BOOL Main( PROC_WORK* work );
static void Draw( PROC_WORK* work );

static void InitSoundObj( PROC_WORK* work );
static void DrawSoundObj( SOUNDOBJ* sobj );
static void MoveSoundObj( SOUNDOBJ* sobj );
static void SetActiveOn( SOUNDOBJ* sobj );
static void SetActiveOff( SOUNDOBJ* sobj );

static void SetMode( PROC_WORK* work, TESTMODE next_mode );


//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_3D_SOUND_MainProcFunc_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
#if 0
	PROC_WORK* work = NULL;

  // ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OBATA_DEBUG, 0x100000 );

	// ワークを作成
	work = GFL_PROC_AllocWork( proc, sizeof( PROC_WORK ), HEAPID_OBATA_DEBUG );

  // 初期化処理
  DEBUG_OBATA_3D_SOUND_Init( HEAPID_OBATA_DEBUG );
  Initialize( work );

#endif
	return GFL_PROC_RES_FINISH;
}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_3D_SOUND_MainProcFunc_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
#if 0
	PROC_WORK* work = mywk;
  BOOL end = FALSE;

  switch( *seq )
  {
  case 0:
    PMSND_PlayBGM( SEQ_BGM_H_01 );
    ++( *seq );
  case 1:
    end = Main( work );
    ISS_3DS_SYS_Main( work->iss3dsSys );
    PMSND_ChangeBGMVolume( TRACK_MASK_WIND, 0 );  // 風OFF
    Draw( work );
    if( end )
    {
      ++( *seq );
    }
    break;
  case 2:
    return GFL_PROC_RES_FINISH;
  }

#endif
  return GFL_PROC_RES_CONTINUE;
}


//============================================================================================
/**
 * @brief 終了関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_3D_SOUND_MainProcFunc_End( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
#if 0
	PROC_WORK* work = mywk;

  // 終了処理
  Finalize( work );

	// ワークを破棄
	GFL_PROC_FreeWork( proc );

  DEBUG_OBATA_3D_SOUND_Exit();

  // ヒープを破棄
  GFL_HEAP_DeleteHeap( HEAPID_OBATA_DEBUG );

#endif
	return GFL_PROC_RES_FINISH;
} 


//============================================================================================
/**
 * @brief プロセス定義データ
 */
//============================================================================================
const GFL_PROC_DATA DebugObata3DSoundMainProcData = 
{
	DEBUG_OBATA_3D_SOUND_MainProcFunc_Init,
	DEBUG_OBATA_3D_SOUND_MainProcFunc_Main,
	DEBUG_OBATA_3D_SOUND_MainProcFunc_End,
};


//============================================================================================
/**
 * @brief 非公開関数の定義
 */
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @breif 初期化
 */
//-------------------------------------------------------------------------------------------- 
static void Initialize( PROC_WORK* work )
{
#if 0
  // 3D管理ユーティリティーのセットアップ
  work->g3dUtil = GFL_G3D_UTIL_Create( 10, 10, HEAPID_OBATA_DEBUG );

  // ユニット追加
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      work->unitIndex[i] = GFL_G3D_UTIL_AddUnit( work->g3dUtil, &setup[i] );
    }
  }

  // カメラ作成
  {
    VecFx32    pos = { 0, 1000*FX32_ONE, 400*FX32_ONE };
    VecFx32 target = { 0, 0, 0 };
    work->camera   = GFL_G3D_CAMERA_CreateDefault( &pos, &target, HEAPID_OBATA_DEBUG );
  }
  // カメラ設定
  {
    fx32 far = FX32_ONE * 4096;
    GFL_G3D_CAMERA_SetFar( work->camera, &far );
  }
  { // ダミーカメラ
    VecFx32    pos = { 0, 0, 0 };
    VecFx32 target = { 0, 0, -FX32_ONE };
    VecFx32     up = { 0, FX32_ONE, 0 };
    work->dammyCam = GFL_G3D_CAMERA_CreateDefault( &pos, &target, HEAPID_OBATA_DEBUG );
    GFL_G3D_CAMERA_SetCamUp( work->dammyCam, &up );
  }

  // 3Dサウンドシステム作成
  work->iss3dsSys = ISS_3DS_SYS_Create( HEAPID_OBATA_DEBUG, 10 );
  ISS_3DS_SYS_SetObserver( work->iss3dsSys, work->camera );

  // トレーラ作成
  InitSoundObj( work );

  // その他の初期化
  SetMode( work, TESTMODE_ALL );
  VEC_Set( &work->playerPos, 0, 0, 0 );
  VEC_Set( &work->camPosOffset, FX32_ONE, 100*FX32_ONE, 300*FX32_ONE );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 終了
 */
//-------------------------------------------------------------------------------------------- 
static void Finalize( PROC_WORK* work )
{ 
#if 0
  // 3Dサウンドシステム破棄
  ISS_3DS_SYS_ClearObserver( work->iss3dsSys );
  ISS_3DS_SYS_Delete( work->iss3dsSys );

  // カメラ破棄
  GFL_G3D_CAMERA_Delete( work->camera );
  GFL_G3D_CAMERA_Delete( work->dammyCam );

  // ユニット破棄
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      GFL_G3D_UTIL_DelUnit( work->g3dUtil, work->unitIndex[i] );
    }
  }

  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( work->g3dUtil );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @breif メイン処理 
 */
//-------------------------------------------------------------------------------------------- 
static BOOL Main( PROC_WORK* work )
{
#if 0
  int i;
  VecFx32 vel, pos;
  int trg = GFL_UI_KEY_GetTrg();
  int key = GFL_UI_KEY_GetCont();

  // カメラオフセット移動
  VEC_Set( &vel, 0, 0, 0 );
  if( key & PAD_BUTTON_X )  work->camPosOffset.y += FX32_ONE * 10;
  if( key & PAD_BUTTON_B )  work->camPosOffset.y -= FX32_ONE * 10;

  // 主人公移動
  VEC_Set( &vel, 0, 0, 0 );
  if( key & PAD_KEY_UP )    vel.z -= FX32_ONE * 10;
  if( key & PAD_KEY_DOWN )  vel.z += FX32_ONE * 10;
  if( key & PAD_KEY_LEFT )  vel.x -= FX32_ONE * 10;
  if( key & PAD_KEY_RIGHT ) vel.x += FX32_ONE * 10;
  VEC_Add( &work->playerPos, &vel, &work->playerPos );
  VEC_Add( &work->playerPos, &work->camPosOffset, &pos );
  GFL_G3D_CAMERA_SetPos( work->camera, &pos );
  GFL_G3D_CAMERA_SetTarget( work->camera, &work->playerPos );


  // オブジェクトを動かす
  for( i=0; i<SOUNDOBJ_NUM; i++ )
  { 
    MoveSoundObj( &work->soundObj[i] );
  }

  // スタートでモード切替
  if( trg & PAD_BUTTON_START ) SetMode( work, work->mode + 1 );

  // セレクトで終了
  if( trg & PAD_BUTTON_SELECT ) return TRUE;

#endif
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 描画
 */
//-------------------------------------------------------------------------------------------- 
static void Draw( PROC_WORK* work )
{
#if 0
  int i;

  // カメラ更新
  GFL_G3D_CAMERA_Switching( work->camera );

  // 描画
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();

  // 音源オブジェ描画
  for( i=0; i<SOUNDOBJ_NUM; i++ ) DrawSoundObj( &work->soundObj[i] );

  // プレイヤー描画
  {
    GFL_G3D_OBJ* obj;
    GFL_G3D_OBJSTATUS status;
    VEC_Set( &status.trans, work->playerPos.x, work->playerPos.y, work->playerPos.z );
    VEC_Set( &status.scale, FX32_ONE>>1, FX32_ONE>>1, FX32_ONE>>1 );
    MTX_Identity33( &status.rotate );
    obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[SETUP_INDEX_HOUSE] );
    GFL_G3D_DRAW_DrawObject( obj, &status );
  }
  GFL_G3D_DRAW_End(); 
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェを初期化する
 */
//--------------------------------------------------------------------------------------------
static void InitSoundObj( PROC_WORK* work )
{
#if 0
  int index;
  SOUNDOBJ* sobj;

  // トレーラー1
  sobj = &work->soundObj[SOUNDOBJ_TR_1];
  sobj->obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[SETUP_INDEX_TRAILER] );
  VEC_Set( &sobj->status.trans, 0, 0, 0 );
  VEC_Set( &sobj->status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  GFL_CALC3D_MTX_CreateRot( 0, 180.0f/360.0f*0xffff, 0, &sobj->status.rotate );
  index = ISS_3DS_SYS_AddUnit( work->iss3dsSys );
  sobj->iss3dsUnit = ISS_3DS_SYS_GetUnit( work->iss3dsSys, index );
  ISS_3DS_UNIT_SetPos( sobj->iss3dsUnit, &sobj->status.trans );
  ISS_3DS_UNIT_SetRange( sobj->iss3dsUnit, 500*FX32_ONE );
  ISS_3DS_UNIT_SetMaxVolume( sobj->iss3dsUnit, 127 );
  ISS_3DS_UNIT_SetTrackBit( sobj->iss3dsUnit, TRACK_MASK_CAR_1 );
  sobj->frame = 0;
  sobj->endFrame = 500;
  VEC_Set( &sobj->startPos, 50*FX32_ONE, 0*FX32_ONE, -1000*FX32_ONE );
  VEC_Set( &sobj->endPos,   50*FX32_ONE, 0*FX32_ONE,  1000*FX32_ONE );

  // トレーラー2
  sobj = &work->soundObj[SOUNDOBJ_TR_2];
  sobj->obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[SETUP_INDEX_TRAILER] );
  VEC_Set( &sobj->status.trans, 0, 0, 0 );
  VEC_Set( &sobj->status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &sobj->status.rotate );
  index = ISS_3DS_SYS_AddUnit( work->iss3dsSys );
  sobj->iss3dsUnit = ISS_3DS_SYS_GetUnit( work->iss3dsSys, index );
  ISS_3DS_UNIT_SetPos( sobj->iss3dsUnit, &sobj->status.trans );
  ISS_3DS_UNIT_SetRange( sobj->iss3dsUnit, 500*FX32_ONE );
  ISS_3DS_UNIT_SetMaxVolume( sobj->iss3dsUnit, 127 );
  ISS_3DS_UNIT_SetTrackBit( sobj->iss3dsUnit, TRACK_MASK_CAR_2 );
  sobj->frame = 0;
  sobj->endFrame = 500;
  VEC_Set( &sobj->startPos, -50*FX32_ONE, 0*FX32_ONE,  1000*FX32_ONE );
  VEC_Set( &sobj->endPos,   -50*FX32_ONE, 0*FX32_ONE, -1000*FX32_ONE );

  // 船
  sobj = &work->soundObj[SOUNDOBJ_SHIP];
  sobj->obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[SETUP_INDEX_SHIP] );
  VEC_Set( &sobj->status.trans, 0, 0, 0 );
  VEC_Set( &sobj->status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &sobj->status.rotate );
  index = ISS_3DS_SYS_AddUnit( work->iss3dsSys );
  sobj->iss3dsUnit = ISS_3DS_SYS_GetUnit( work->iss3dsSys, index );
  ISS_3DS_UNIT_SetPos( sobj->iss3dsUnit, &sobj->status.trans );
  ISS_3DS_UNIT_SetRange( sobj->iss3dsUnit, 3000*FX32_ONE );
  ISS_3DS_UNIT_SetMaxVolume( sobj->iss3dsUnit, 127 );
  ISS_3DS_UNIT_SetTrackBit( sobj->iss3dsUnit, TRACK_MASK_SHIP );
  sobj->frame = 0;
  sobj->endFrame = 1000;
  VEC_Set( &sobj->startPos, -800*FX32_ONE, 0*FX32_ONE, 0*FX32_ONE );
  VEC_Set( &sobj->endPos,    800*FX32_ONE, 0*FX32_ONE, 0*FX32_ONE );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェを描画する
 */
//--------------------------------------------------------------------------------------------
static void DrawSoundObj( SOUNDOBJ* sobj )
{
#if 0
  // 有効なオブジェのみを描画
  if( sobj->active )
  {
    GFL_G3D_DRAW_DrawObject( sobj->obj, &sobj->status );
  }
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェを動かす
 */
//--------------------------------------------------------------------------------------------
static void MoveSoundObj( SOUNDOBJ* sobj )
{
#if 0
  // 有効なオブジェのみを動かす
  if( sobj->active )
  { 
    fx32 w0, w1;
    VecFx32 v0, v1;

    // フレーム進める
    sobj->frame = (sobj->frame + 1) % sobj->endFrame;

    // 位置ベクトルを線形補間
    w1 = FX_Div( sobj->frame, sobj->endFrame );
    w0 = FX32_ONE - w1;
    GFL_CALC3D_VEC_MulScalar( &sobj->startPos, w0, &v0 );
    GFL_CALC3D_VEC_MulScalar( &sobj->endPos,   w1, &v1 );
    VEC_Add( &v0, &v1, &sobj->status.trans );

    // 音源位置を合わせる
    ISS_3DS_UNIT_SetPos( sobj->iss3dsUnit, &sobj->status.trans );
  }
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェを有効化する
 */
//--------------------------------------------------------------------------------------------
static void SetActiveOn( SOUNDOBJ* sobj )
{ 
#if 0
  sobj->active = TRUE;
  ISS_3DS_UNIT_SetMaxVolume( sobj->iss3dsUnit, 127 );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェを無効化する
 */
//--------------------------------------------------------------------------------------------
static void SetActiveOff( SOUNDOBJ* sobj )
{
#if 0
  sobj->active = FALSE;
  ISS_3DS_UNIT_SetMaxVolume( sobj->iss3dsUnit, 0 );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief モードを変更する
 */
//--------------------------------------------------------------------------------------------
static void SetMode( PROC_WORK* work, TESTMODE next_mode )
{ 
#if 0
  // モード変更
  work->mode = next_mode % TESTMODE_NUM;

  // モード設定
  switch( work->mode )
  {
  case TESTMODE_ALL:
    SetActiveOn( &work->soundObj[SOUNDOBJ_TR_1] );
    SetActiveOn( &work->soundObj[SOUNDOBJ_TR_2] );
    SetActiveOn( &work->soundObj[SOUNDOBJ_SHIP] );
    break;
  case TESTMODE_TR:
    SetActiveOn( &work->soundObj[SOUNDOBJ_TR_1] );
    SetActiveOn( &work->soundObj[SOUNDOBJ_TR_2] );
    SetActiveOff( &work->soundObj[SOUNDOBJ_SHIP] );
    break;
  case TESTMODE_TR1:
    SetActiveOn( &work->soundObj[SOUNDOBJ_TR_1] );
    SetActiveOff( &work->soundObj[SOUNDOBJ_TR_2] );
    SetActiveOff( &work->soundObj[SOUNDOBJ_SHIP] );
    break;
  case TESTMODE_TR2:
    SetActiveOff( &work->soundObj[SOUNDOBJ_TR_1] );
    SetActiveOn( &work->soundObj[SOUNDOBJ_TR_2] );
    SetActiveOff( &work->soundObj[SOUNDOBJ_SHIP] );
    break;
  case TESTMODE_SHIP:
    SetActiveOff( &work->soundObj[SOUNDOBJ_TR_1] );
    SetActiveOff( &work->soundObj[SOUNDOBJ_TR_2] );
    SetActiveOn( &work->soundObj[SOUNDOBJ_SHIP] );
    break;
  }
#endif
}
