#include <gflib.h>
#include "debug_obata_ica_test_setup.h"
#include "system/ica_anime.h"
#include "system/main.h"
#include "arc/arc_def.h"
#include "arc/debug_obata.naix"


//============================================================================================
/**
 * @brief 3Dデータ
 */
//============================================================================================

// サイコロ
static const GFL_G3D_UTIL_RES res_table_dice[] = 
{
  { ARCID_OBATA_DEBUG, NARC_debug_obata_dice_nsbmd, GFL_G3D_UTIL_RESARC },
};
static const GFL_G3D_UTIL_OBJ obj_table_dice[] = 
{
  {
    0,     // モデルリソースID
    0,     // モデルデータID(リソース内部INDEX)
    0,     // テクスチャリソースID
    NULL,  // アニメテーブル(複数指定のため)
    0,     // アニメリソース数
  },
}; 

// セットアップ番号
enum
{
  SETUP_INDEX_DICE,
  SETUP_INDEX_MAX
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP setup[] =
{
  { res_table_dice, NELEMS(res_table_dice), obj_table_dice, NELEMS(obj_table_dice) },
};

// アニメーションデータ
typedef enum
{
  ANIME_SRT,
  ANIME_SR,
  ANIME_ST,
  ANIME_RT,
  ANIME_S,
  ANIME_R,
  ANIME_T,
  ANIME_MODE_NUM
} ANIME_MODE;

ARCDATID anime_dat_id[] = 
{
  NARC_debug_obata_dice_anime_srt_bin,
  NARC_debug_obata_dice_anime_sr_bin,
  NARC_debug_obata_dice_anime_st_bin,
  NARC_debug_obata_dice_anime_rt_bin,
  NARC_debug_obata_dice_anime_s_bin,
  NARC_debug_obata_dice_anime_r_bin,
  NARC_debug_obata_dice_anime_t_bin,
};


//============================================================================================
/**
 * @breif ワーク
 */
//============================================================================================
typedef struct
{
  GFL_G3D_UTIL* g3dUtil;
  u16 unitIndex[ SETUP_INDEX_MAX ];

  ANIME_MODE animeMode;
  ICA_ANIME* icaAnime;
  GFL_G3D_CAMERA* camera;
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
static void DrawDice( GFL_G3D_OBJ* obj, ICA_ANIME* anime );
static void SetAnimeMode( PROC_WORK* work, ANIME_MODE );


//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_ICA_TEST_MainProcFunc_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = NULL;

  // ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OBATA_DEBUG, 0x100000 );

	// ワークを作成
	work = GFL_PROC_AllocWork( proc, sizeof( PROC_WORK ), HEAPID_OBATA_DEBUG );

  // 初期化処理
  DEBUG_OBATA_ICA_TEST_Init( HEAPID_OBATA_DEBUG );
  Initialize( work );
  SetAnimeMode( work, ANIME_SRT );

	return GFL_PROC_RES_FINISH;
}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_ICA_TEST_MainProcFunc_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = mywk;
  BOOL end = FALSE;

  switch( *seq )
  {
  case 0:
    end = Main( work );
    Draw( work );
    if( end )
    {
      ++( *seq );
    }
    break;
  case 1:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}


//============================================================================================
/**
 * @brief 終了関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_ICA_TEST_MainProcFunc_End( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = mywk;

  // 終了処理
  Finalize( work );

	// ワークを破棄
  ICA_ANIME_Delete( work->icaAnime );
	GFL_PROC_FreeWork( proc );

  DEBUG_OBATA_ICA_TEST_Exit();

  // ヒープを破棄
  GFL_HEAP_DeleteHeap( HEAPID_OBATA_DEBUG );

	return GFL_PROC_RES_FINISH;
} 


//============================================================================================
/**
 * @brief プロセス定義データ
 */
//============================================================================================
const GFL_PROC_DATA DebugObataIcaTestMainProcData = 
{
	DEBUG_OBATA_ICA_TEST_MainProcFunc_Init,
	DEBUG_OBATA_ICA_TEST_MainProcFunc_Main,
	DEBUG_OBATA_ICA_TEST_MainProcFunc_End,
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

  // アニメーション有効化
  {
    int i;
    for( i=0; i<SETUP_INDEX_MAX; i++ )
    {
      int j;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[i] );
      int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      for( j=0; j<anime_count; j++ )
      {
        GFL_G3D_OBJECT_EnableAnime( obj, j );
      }
    }
  }

  // icaデータをロード
  work->icaAnime = ICA_ANIME_CreateStreamingAlloc(
      HEAPID_OBATA_DEBUG, ARCID_OBATA_DEBUG, NARC_debug_obata_dice_anime_srt_bin, 10 );

  // カメラ作成
  {
    VecFx32    pos = { 0, 0, 50 * FX32_ONE };
    VecFx32 target = { 0, 0, 0 };
    VecFx32     up = { 0, FX32_ONE, 0 };
    fx32       far = FX32_ONE * 4096;
    work->camera   = GFL_G3D_CAMERA_CreateDefault( &pos, &target, HEAPID_OBATA_DEBUG );
    GFL_G3D_CAMERA_GetCamUp( work->camera, &up );
    GFL_G3D_CAMERA_SetFar( work->camera, &far );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 終了
 */
//-------------------------------------------------------------------------------------------- 
static void Finalize( PROC_WORK* work )
{ 
  // カメラ破棄
  GFL_G3D_CAMERA_Delete( work->camera );

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
}

//--------------------------------------------------------------------------------------------
/**
 * @breif メイン処理 
 */
//-------------------------------------------------------------------------------------------- 
static BOOL Main( PROC_WORK* work )
{
  int trg = GFL_UI_KEY_GetTrg();
  int key = GFL_UI_KEY_GetCont();

  // モード切替
  if( trg & PAD_BUTTON_A )
  {
    ANIME_MODE next = (work->animeMode + 1) % ANIME_MODE_NUM;
    SetAnimeMode( work, next );
  }
  // セレクトで終了
  if( trg & PAD_BUTTON_SELECT ) return TRUE;
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 描画
 */
//-------------------------------------------------------------------------------------------- 
static void Draw( PROC_WORK* work )
{
  const fx32 anime_speed = FX32_ONE;
  GFL_G3D_OBJSTATUS status;

  VEC_Set( &status.trans, 0, 0, 0 );
  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &status.rotate );

  // カメラ更新
  GFL_G3D_CAMERA_Switching( work->camera );

  // 描画
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[SETUP_INDEX_DICE] );
    DrawDice( obj, work->icaAnime );
  }
  GFL_G3D_DRAW_End();

  // アニメーションを進める
  ICA_ANIME_IncAnimeFrame( work->icaAnime, anime_speed );
}


//--------------------------------------------------------------------------------------------
/**
 * @breif サイコロ描画
 */
//-------------------------------------------------------------------------------------------- 
static void DrawDice( GFL_G3D_OBJ* obj, ICA_ANIME* anime )
{
  VecFx32 scaleval, rotval, transval;
  BOOL havescale, haverot, havetrans;
  GFL_G3D_OBJSTATUS status;

  VEC_Set( &status.trans, 0, 0, 0 );
  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &status.rotate );

  havetrans = ICA_ANIME_GetTranslate( anime, &transval );
  haverot   = ICA_ANIME_GetRotate( anime, &rotval );
  havescale = ICA_ANIME_GetScale( anime, &scaleval );

  if( havetrans )
  {
    VEC_Set( &status.trans, transval.x, transval.y, transval.z );
  }
  if( haverot )
  {
    float x, y, z;
    u16 rx, ry, rz;
    x = FX_FX32_TO_F32( rotval.x );
    y = FX_FX32_TO_F32( rotval.y );
    z = FX_FX32_TO_F32( rotval.z );
    while( x < 0 ) x += 360.0f;
    while( y < 0 ) y += 360.0f;
    while( z < 0 ) z += 360.0f;
    rx = x / 360.0f * 0xffff;
    ry = y / 360.0f * 0xffff;
    rz = z / 360.0f * 0xffff; 
    GFL_CALC3D_MTX_CreateRot( rx, ry, rz, &status.rotate );
  }
  if( havescale )
  {
    VEC_Set( &status.scale, scaleval.x, scaleval.y, scaleval.z );
  }

  // 描画
  GFL_G3D_DRAW_DrawObject( obj, &status );
}

//--------------------------------------------------------------------------------------------
/**
 * @breif サイコロ描画
 */
//-------------------------------------------------------------------------------------------- 
static void SetAnimeMode( PROC_WORK* work, ANIME_MODE mode )
{ 
  work->animeMode = mode;
  ICA_ANIME_Delete( work->icaAnime );
  work->icaAnime = ICA_ANIME_CreateStreamingAlloc(
      HEAPID_OBATA_DEBUG, ARCID_OBATA_DEBUG, anime_dat_id[ mode ], 10 );
}
