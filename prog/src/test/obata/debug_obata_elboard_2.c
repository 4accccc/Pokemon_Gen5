#include <wchar.h>    // wcslen
#include <gflib.h>
#include "debug_obata_elboard_setup.h"
#include "system/ica_anime.h"
#include "system/main.h"
#include "arc/arc_def.h"
#include "arc/debug_obata.naix"
#include "system/el_scoreboard.h"
#include "field/gimmick_obj_elboard.h"
#include "arc/arc_def.h"
#include "arc/message.naix"
#include "msg/msg_place_name.h"


//============================================================================================
// ■リソース
//============================================================================================

// リソースインデックス
typedef enum
{
  RES_ELBOARD_NSBMD,    // 掲示板のモデル
  RES_ELBOARD_NSBTX,    // 掲示板のテクスチャ
  RES_ELBOARD_NSBTA_1,  // スクロールアニメーション1
  RES_ELBOARD_NSBTA_2,  // スクロールアニメーション2
  RES_ELBOARD_NSBTA_3,  // スクロールアニメーション3
  RES_ELBOARD_NSBTA_4,  // スクロールアニメーション4
  RES_ELBOARD_NSBTA_5,  // スクロールアニメーション5
  RES_ELBOARD_NSBTA_6,  // スクロールアニメーション6
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[] = 
{
  { ARCID_OBATA_DEBUG, NARC_debug_obata_gelboard01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_gelboard01_nsbtx, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_gelboard01_1_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_gelboard01_2_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_gelboard01_3_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_gelboard01_4_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_gelboard01_5_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_OBATA_DEBUG, NARC_debug_obata_gelboard01_6_nsbta, GFL_G3D_UTIL_RESARC },
};

// アニメインデックス
typedef enum
{
  ANM_ELBOARD_NEWS_1, // ニュース1
  ANM_ELBOARD_NEWS_2, // ニュース2
  ANM_ELBOARD_NEWS_3, // ニュース3
  ANM_ELBOARD_NEWS_4, // ニュース4
  ANM_ELBOARD_NEWS_5, // ニュース5
  ANM_ELBOARD_NEWS_6, // ニュース6
  ANM_NUM
} ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table[] = 
{
  // アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ELBOARD_NSBTA_1, 0 },
  { RES_ELBOARD_NSBTA_2, 0 },
  { RES_ELBOARD_NSBTA_3, 0 },
  { RES_ELBOARD_NSBTA_4, 0 },
  { RES_ELBOARD_NSBTA_5, 0 },
  { RES_ELBOARD_NSBTA_6, 0 },
};

// オブジェクトインデックス
typedef enum
{
  OBJ_ELBOARD,  // 電光掲示板
  OBJ_NUM
} OBJ_INDEX;
static const GFL_G3D_UTIL_OBJ obj_table[] = 
{
  // モデルリソースID, 
  // モデルデータID(リソース内部INDEX), 
  // テクスチャリソースID,
  // アニメテーブル, 
  // アニメリソース数
  { RES_ELBOARD_NSBMD, 0, RES_ELBOARD_NSBTX, anm_table, NELEMS(anm_table) },
}; 

// ユニットインデックス
typedef enum
{
  UNIT_ELBOARD, // 電光掲示板
  UNIT_NUM
} UNIT_INDEX;
static const GFL_G3D_UTIL_SETUP unit[] =
{
  { res_table, RES_NUM, obj_table, OBJ_NUM },
};


//============================================================================================
// ■掲示板パラメータ
//============================================================================================

// ニュース番号
typedef enum
{
  NEWS_1,
  NEWS_2,
  NEWS_3,
  NEWS_4,
  NEWS_5,
  NEWS_6,
  NEWS_NUM,
  NEWS_MAX = NEWS_NUM-1
} NEWS_INDEX; 

// テクスチャ名
static char* tex_name[NEWS_NUM] = 
{
  "gelboard_1",
  "gelboard_2",
  "gelboard_3",
  "gelboard_4",
  "gelboard_5",
  "gelboard_6",
};
// パレット名
static char* plt_name[NEWS_NUM] = 
{
  "gelboard_1_pl",
  "gelboard_2_pl",
  "gelboard_3_pl",
  "gelboard_4_pl",
  "gelboard_5_pl",
  "gelboard_6_pl",
};
// ニュースアニメ・インデックス
static const int news_anm_index[NEWS_NUM] = 
{
  ANM_ELBOARD_NEWS_1, 
  ANM_ELBOARD_NEWS_2, 
  ANM_ELBOARD_NEWS_3, 
  ANM_ELBOARD_NEWS_4, 
  ANM_ELBOARD_NEWS_5, 
  ANM_ELBOARD_NEWS_6, 
};
// 各ニュースの引用元アーカイブID
static const ARCID news_arc_id[NEWS_NUM] = 
{
  ARCID_MESSAGE,
  ARCID_MESSAGE,
  ARCID_MESSAGE,
  ARCID_MESSAGE,
  ARCID_MESSAGE,
  ARCID_MESSAGE,
};
// 各ニュースの引用元アーカイブ内インデックス
static const ARCDATID news_dat_id[NEWS_NUM] = 
{
  NARC_message_place_name_dat, 
  NARC_message_place_name_dat, 
  NARC_message_place_name_dat, 
  NARC_message_place_name_dat, 
  NARC_message_place_name_dat, 
  NARC_message_place_name_dat, 
};
// 各ニュースのメッセージID
static const u32 news_msg_id[NEWS_NUM] = 
{
  MAPNAME_T1KANOKO,
  MAPNAME_T2KARAKUSA,
  MAPNAME_C1SANYOU,
  MAPNAME_C2SIPPOU,
  MAPNAME_C3YAGURUMA,
  MAPNAME_C4RAIMON,
};


static NEWS_PARAM news_param[NEWS_NUM] = 
{
  { ANM_ELBOARD_NEWS_1, "gelboard_1", "gelboard_1_pl", 
    ARCID_MESSAGE, NARC_message_place_name_dat, MAPNAME_T1KANOKO },
  { ANM_ELBOARD_NEWS_2, "gelboard_2", "gelboard_2_pl", 
    ARCID_MESSAGE, NARC_message_place_name_dat, MAPNAME_T2KARAKUSA },
  { ANM_ELBOARD_NEWS_3, "gelboard_3", "gelboard_3_pl", 
    ARCID_MESSAGE, NARC_message_place_name_dat, MAPNAME_C1SANYOU },
  { ANM_ELBOARD_NEWS_4, "gelboard_4", "gelboard_4_pl", 
    ARCID_MESSAGE, NARC_message_place_name_dat, MAPNAME_C2SIPPOU },
  { ANM_ELBOARD_NEWS_5, "gelboard_5", "gelboard_5_pl", 
    ARCID_MESSAGE, NARC_message_place_name_dat, MAPNAME_C3YAGURUMA },
  { ANM_ELBOARD_NEWS_6, "gelboard_6", "gelboard_6_pl", 
    ARCID_MESSAGE, NARC_message_place_name_dat, MAPNAME_C4RAIMON },
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
  u16 unitIndex[ UNIT_NUM ];

  // 電光掲示板
  GOBJ_ELBOARD* elboard;

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


//--------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_OBATA_ELBOARD2_MainProcFunc_Init( 
    GFL_PROC* proc, int* seq, void* pwk, void* mywk )
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


//--------------------------------------------------------------------------------------------
/**
 * @brief メイン関数
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_OBATA_ELBOARD2_MainProcFunc_Main( 
    GFL_PROC* proc, int* seq, void* pwk, void* mywk )
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


//--------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_OBATA_ELBOARD2_MainProcFunc_End( 
    GFL_PROC* proc, int* seq, void* pwk, void* mywk )
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
// ■プロセス定義データ
//============================================================================================
const GFL_PROC_DATA DebugObataElboard2MainProcData = 
{
	DEBUG_OBATA_ELBOARD2_MainProcFunc_Init,
	DEBUG_OBATA_ELBOARD2_MainProcFunc_Main,
	DEBUG_OBATA_ELBOARD2_MainProcFunc_End,
};


//============================================================================================
// ■非公開関数の定義
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
    for( i=0; i<UNIT_NUM; i++ )
    {
      OBATA_Printf( "%d\n", i );
      work->unitIndex[i] = GFL_G3D_UTIL_AddUnit( work->g3dUtil, &unit[i] );
    }
  }

  // 電光掲示板を作成
  {
    int i;
    ELBOARD_PARAM param;
    param.heapID       = HEAPID_OBATA_DEBUG;
    param.maxNewsNum   = NEWS_NUM;
    param.dispSize     = 8;
    param.newsInterval = 5;
    param.g3dObj       = GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[UNIT_ELBOARD] );
    work->elboard      = GOBJ_ELBOARD_Create( &param ); 
    for( i=0; i<NEWS_NUM; i++ )
    {
      GOBJ_ELBOARD_AddNews( work->elboard, &news_param[i] );
    }
  }

  // カメラ作成
  {
    VecFx32    pos = { 0*FX32_ONE, 50*FX32_ONE, 200*FX32_ONE };
    VecFx32 target = { 0*FX32_ONE, 50*FX32_ONE, 0*FX32_ONE };
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
  int i;

  // カメラ破棄
  GFL_G3D_CAMERA_Delete( work->camera );

  // 電光掲示板を破棄
  GOBJ_ELBOARD_Delete( work->elboard );

  // ユニット破棄
  {
    int i;
    for( i=0; i<UNIT_NUM; i++ )
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
  int i;
  int trg = GFL_UI_KEY_GetTrg();
  int key = GFL_UI_KEY_GetCont(); 
  if( trg & PAD_BUTTON_SELECT ) return TRUE; // セレクトで終了

  // フレームカウンタ更新
  work->frame++; 

  // 電光掲示板のアニメーション更新
  GOBJ_ELBOARD_Main( work->elboard, FX32_ONE );

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
    GFL_G3D_OBJ* obj = 
      GFL_G3D_UTIL_GetObjHandle( work->g3dUtil, work->unitIndex[UNIT_ELBOARD] );
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
