#include <gflib.h>
#include "debug_obata_common.h"
#include "system/ica_data.h"
#include "system/main.h"
#include "arc/arc_def.h"
#include "arc/debug_obata.naix"


//============================================================================================
/**
 * @brief 3Dデータ
 */
//============================================================================================

// リソース
static const GFL_G3D_UTIL_RES res_table[] = 
{
  { ARCID_OBATA_DEBUG, NARC_debug_obata_camera_test1_nsbmd, GFL_G3D_UTIL_RESARC },
};

// オブジェクト
static const GFL_G3D_UTIL_OBJ obj_table[] = 
{
  {
    0,    // モデルリソースID
    0,    // モデルデータID(リソース内部INDEX)
    0,    // テクスチャリソースID
    NULL, // アニメテーブル(複数指定のため)
    0,    // アニメリソース数
  },
}; 

// セットアップデータ
static const GFL_G3D_UTIL_SETUP setup =
{
  res_table,
  NELEMS(res_table),
  obj_table,
  NELEMS(obj_table),
};



//============================================================================================
/**
 * @breif ワーク
 */
//============================================================================================
typedef struct
{
  GFL_G3D_UTIL* g3dUtil;
  u16 unitIndex;

  ICA_DATA* icaData;
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


//============================================================================================
/**
 * @brief 初期化関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_MainProcFunc_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = NULL;

  // ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OBATA_DEBUG, 0x100000 );

	// ワークを作成
	work = GFL_PROC_AllocWork( proc, sizeof( PROC_WORK ), HEAPID_OBATA_DEBUG );

  // 初期化処理
  DEBUG_OBATA_COMMON_Init( HEAPID_OBATA_DEBUG );
  Initialize( work );

	return GFL_PROC_RES_FINISH;
}


//============================================================================================
/**
 * @brief メイン関数
 */
//============================================================================================
static GFL_PROC_RESULT DEBUG_OBATA_MainProcFunc_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
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
static GFL_PROC_RESULT DEBUG_OBATA_MainProcFunc_End( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
	PROC_WORK* work = mywk;

  // 終了処理
  Finalize( work );

	// ワークを破棄
  ICA_DATA_Delete( work->icaData );
	GFL_PROC_FreeWork( proc );

  DEBUG_OBATA_COMMON_Exit();

  // ヒープを破棄
  GFL_HEAP_DeleteHeap( HEAPID_OBATA_DEBUG );

	return GFL_PROC_RES_FINISH;
} 


//============================================================================================
/**
 * @brief プロセス定義データ
 */
//============================================================================================
const GFL_PROC_DATA DebugObataMainProcData = 
{
	DEBUG_OBATA_MainProcFunc_Init,
	DEBUG_OBATA_MainProcFunc_Main,
	DEBUG_OBATA_MainProcFunc_End,
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
  work->unitIndex = GFL_G3D_UTIL_AddUnit( work->g3dUtil, &setup );

  // icaデータをロード
  work->icaData = ICA_DATA_Create( 
      HEAPID_OBATA_DEBUG, ARCID_OBATA_DEBUG, NARC_debug_obata_ica_test_data_bin );

  // カメラ作成
  {
    VecFx32    pos = { 0, 0, 0 };
    VecFx32 target = { 0, 0, 0 };
    work->camera   = GFL_G3D_CAMERA_CreateDefault( &pos, &target, HEAPID_OBATA_DEBUG );
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
  GFL_G3D_UTIL_DelUnit( work->g3dUtil, work->unitIndex );

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
  static fx32 frame = 0;
  GFL_G3D_OBJSTATUS status;
  GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex );

  VEC_Set( &status.trans, 0, 0, 0 );
  VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &status.rotate );

  // カメラ更新
  ICA_DATA_SetCameraStatus( work->icaData, work->camera, frame );
  GFL_G3D_CAMERA_Switching( work->camera );

  // 描画
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  GFL_G3D_DRAW_DrawObject( obj, &status );
  GFL_G3D_DRAW_End();

  frame += FX32_ONE; 
}
