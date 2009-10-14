#include <gflib.h>
#include "debug_obata_elboard_setup.h"
#include "system/ica_anime.h"
#include "system/main.h"
#include "arc/arc_def.h"
#include "arc/debug_obata.naix"


//============================================================================================
// ■リソース
//============================================================================================

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
static ARCDATID anime_dat_id[] = 
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
  u32 frame;
  GFL_G3D_UTIL* g3dUtil;
  u16 unitIndex[ SETUP_INDEX_MAX ];

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
static void UpdateCamera( PROC_WORK* work );


//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_ELBOARD_MainProcFunc_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = NULL;

  // ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OBATA_DEBUG, 0x100000 );

	// ワークを作成
	work = GFL_PROC_AllocWork( proc, sizeof( PROC_WORK ), HEAPID_OBATA_DEBUG );

  // 初期化処理
  DEBUG_OBATA_ELBOARD_Init( HEAPID_OBATA_DEBUG );
  Initialize( work );

	return GFL_PROC_RES_FINISH;
}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_ELBOARD_MainProcFunc_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
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
static GFL_PROC_RESULT DEBUG_OBATA_ELBOARD_MainProcFunc_End( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = mywk;

  // 終了処理
  Finalize( work );

	// ワークを破棄
	GFL_PROC_FreeWork( proc );

  DEBUG_OBATA_ELBOARD_Exit();

  // ヒープを破棄
  GFL_HEAP_DeleteHeap( HEAPID_OBATA_DEBUG );

	return GFL_PROC_RES_FINISH;
} 


//============================================================================================
/**
 * @brief プロセス定義データ
 */
//============================================================================================
const GFL_PROC_DATA DebugObataElboardMainProcData = 
{
	DEBUG_OBATA_ELBOARD_MainProcFunc_Init,
	DEBUG_OBATA_ELBOARD_MainProcFunc_Main,
	DEBUG_OBATA_ELBOARD_MainProcFunc_End,
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

  // カメラ作成
  {
    VecFx32    pos = { 0, 0, 5*FX32_ONE };
    VecFx32 target = { 0, 0, 0 };
    VecFx32     up = { 0, FX32_ONE, 0 };
    fx32       far = FX32_ONE * 4096;
    work->camera   = GFL_G3D_CAMERA_CreateDefault( &pos, &target, HEAPID_OBATA_DEBUG );
    GFL_G3D_CAMERA_GetCamUp( work->camera, &up );
    GFL_G3D_CAMERA_SetFar( work->camera, &far );
  }

  // その他初期化
  work->frame = 0;
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
  if( trg & PAD_BUTTON_SELECT ) return TRUE; // セレクトで終了

  // フレームカウンタ更新
  work->frame++;
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @breif 描画
 */
//-------------------------------------------------------------------------------------------- 
static void Draw( PROC_WORK* work )
{
  GFL_G3D_OBJSTATUS status; 
  VEC_Set( &status.trans, 0, 0, 0 );
  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &status.rotate );

  // カメラ更新
  UpdateCamera( work );
  GFL_G3D_CAMERA_Switching( work->camera );

  // 描画
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[SETUP_INDEX_DICE] );
    GFL_G3D_DRAW_DrawObject( obj, &status );
  }
  GFL_G3D_DRAW_End();
} 

//--------------------------------------------------------------------------------------------
/**
 * @breif カメラを更新する
 */
//-------------------------------------------------------------------------------------------- 
static void UpdateCamera( PROC_WORK* work )
{
  GFL_G3D_CAMERA_Switching( work->camera );
}
