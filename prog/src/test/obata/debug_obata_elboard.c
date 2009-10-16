#include <gflib.h>
#include "debug_obata_elboard_setup.h"
#include "system/ica_anime.h"
#include "system/main.h"
#include "arc/arc_def.h"
#include "arc/debug_obata.naix"


//============================================================================================
// ■リソース
//============================================================================================

// リソースインデックス
enum
{
  RES_ELBOARD_NSBMD,
  RES_ELBOARD_NSBTX,
  RES_ELBOARD_NSBTA,
  RES_ELBOARD_NSBCA,
  RES_NUM
}; 
static const GFL_G3D_UTIL_RES res_table[] = 
{
  { ARCID_OBATA_DEBUG, NARC_debug_obata_elboard_test_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_elboard_test_nsbtx, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_elboard_test_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_elboard_test_nsbca, GFL_G3D_UTIL_RESARC },
};

// アニメインデックス
enum
{
  ANM_ELBOARD_NSBTA,
  ANM_ELBOARD_NSBCA,
  ANM_NUM
}; 
static const GFL_G3D_UTIL_ANM anm_table[] = 
{
  // アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ELBOARD_NSBTA, 0, },
  { RES_ELBOARD_NSBCA, 0, },
};

// オブジェクトインデックス
enum
{
  OBJ_ELBOARD,
  OBJ_NUM
};
static const GFL_G3D_UTIL_OBJ obj_table[] = 
{
  // モデルリソースID, モデルデータID(リソース内部INDEX), テクスチャリソースID, アニメテーブル, アニメリソース数
  { RES_ELBOARD_NSBMD, 0, RES_ELBOARD_NSBTX, anm_table, NELEMS(anm_table) },
}; 

// セットアップ番号
enum
{
  SETUP_INDEX_ELBOARD,
  SETUP_INDEX_MAX
}; 
static const GFL_G3D_UTIL_SETUP setup[] =
{
  { res_table, NELEMS(res_table), obj_table, NELEMS(obj_table) },
};


//============================================================================================
// ■ワーク
//============================================================================================
typedef struct
{
  // フレームカウンタ
  u32 frame;  

  // G3D_xxxx
  GFL_G3D_UTIL* g3dUtil;
  u16 unitIndex[ SETUP_INDEX_MAX ];

  // カメラ
  GFL_G3D_CAMERA* camera;
}
PROC_WORK;


//============================================================================================
// ■プロトタイプ宣言
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

  // 文字列書き込み
  {
    u16 obj_index_head = GFL_G3D_UTIL_GetUnitObjIdx( work->g3dUtil, SETUP_INDEX_ELBOARD );
    u16 obj_index_elboard = obj_index_head + OBJ_ELBOARD;
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, obj_index_elboard );
  }

  // カメラ作成
  {
    VecFx32    pos = { 0*FX32_ONE, 0*FX32_ONE, 100*FX32_ONE };
    VecFx32 target = { 0*FX32_ONE, 0*FX32_ONE, 0*FX32_ONE };
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
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[SETUP_INDEX_ELBOARD] );
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
