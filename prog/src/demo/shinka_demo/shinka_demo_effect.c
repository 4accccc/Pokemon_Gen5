//============================================================================
/**
 *  @file   shinka_demo_effect.c
 *  @brief  進化デモのパーティクルと背景
 *  @author Koji Kawada
 *  @data   2010.04.09
 *  @note   
 *
 *  モジュール名：SHINKADEMO_EFFECT
 */
//============================================================================
//#define DEBUG_CODE
//#define SET_PARTICLE_Z_MODE  // これが定義されているとき、パーティクルのZ値を編集できるモードになる
//#define SET_REAR_FRAME_LENGTH_COLOR_MODE  // これが定義されているとき、白にするまでのフレーム数を編集できるモードになる


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "sound/wb_sound_data.sadl"
#include "sound/pm_sndsys.h"

#include "shinka_demo_effect.h"

// パーティクル
#include "arc_def.h"
#include "../../../../resource/shinka_demo/shinka_demo_setup.h"
#include "shinka_demo_particle.naix"


//=============================================================================
/**
 *  定数定義
 */
//=============================================================================
// ステップ
typedef enum
{
  // EVO
  STEP_EVO_START,             // 開始待ち中→デモ
  STEP_EVO_DEMO,              // デモ
  STEP_EVO_TO_WHITE_START,    // 白くする
  STEP_EVO_TO_WHITE,          // 白くなり中
  STEP_EVO_WHITE,             // 白
  STEP_EVO_FROM_WHITE,        // 白から戻る
  STEP_EVO_END,               // 終了中

  // AFTER
  STEP_AFTER,                 // 進化後からスタート
}
STEP;


// 3D
// 3D個別
static const GFL_G3D_UTIL_RES rear_res_tbl[] =
{
  { ARCID_SHINKA_DEMO, NARC_shinka_demo_particle_demo_sinka01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_SHINKA_DEMO, NARC_shinka_demo_particle_demo_sinka01_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_SHINKA_DEMO, NARC_shinka_demo_particle_demo_sinka01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_SHINKA_DEMO, NARC_shinka_demo_particle_demo_sinka01_nsbma, GFL_G3D_UTIL_RESARC },
};
enum
{
  REAR_ANM_T,
  REAR_ANM_C,
  REAR_ANM_M,
  REAR_ANM_MAX,
};
static const GFL_G3D_UTIL_ANM rear_anm_tbl[REAR_ANM_MAX] = 
{
  { 1, 0 },
  { 2, 0 },
  { 3, 0 },
};
static const GFL_G3D_UTIL_OBJ rear_obj_tbl[] = 
{
  {
    0,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    0,                         // テクスチャリソースID
    rear_anm_tbl,               // アニメテーブル(複数指定のため)
    NELEMS(rear_anm_tbl),       // アニメリソース数
  },
};
#define REAR_COLOR_FRAME  (0)    // REAR_COLOR_FRAMEで普通の色になる
#define REAR_WHITE_FRAME  (100)  // REAR_WHITE_FRAMEで真っ白になる
// 3D全体
#define THREE_RES_MAX              (4)   // 一度に読み込む総数
#define THREE_OBJ_MAX              (1)   // 一度に読み込む総数
// セットアップ番号
enum
{
  THREE_SETUP_IDX_REAR,
  THREE_SETUP_IDX_MAX
};
// セットアップデータ
static const GFL_G3D_UTIL_SETUP three_setup_tbl[THREE_SETUP_IDX_MAX] =
{
  { rear_res_tbl,   NELEMS(rear_res_tbl),   rear_obj_tbl,   NELEMS(rear_obj_tbl)   },
};
// ユーザ(このソースのプログラムを書いた人)が決めたオブジェクト番号
enum
{
  THREE_USER_OBJ_IDX_REAR,
  THREE_USER_OBJ_IDX_MAX,
};
// 3Dのアニメを1フレームでどれだけ進めるか
#define THREE_ADD (FX32_ONE)  // 60fps
// REARを白く飛ばすアニメの状態
typedef enum
{
  REAR_WHITE_ANIME_STATE_COLOR,
  REAR_WHITE_ANIME_STATE_COLOR_TO_WHITE,
  REAR_WHITE_ANIME_STATE_WHITE,
  REAR_WHITE_ANIME_STATE_WHITE_TO_COLOR,
}
REAR_WHITE_ANIME_STATE;

// REARのフレーム
#define REAR_FRAME_LENGTH_COLOR      ( 620 )  // STEP_EVO_DEMOになってからこのフレームだけ経過したら白にする
#define REAR_FRAME_LENGTH_WHITE      (   3 )//ポケモンを入れ替えなければならないので、最低でも3フレームは必要//(  60 )  // STEP_EVO_WHITEになってからこのフレームだけ経過したらカラーにする  // REAR_WHITE_ANIME_STATE_WHITEになってからこのフレームだけ経過したらREAR_WHITE_ANIME_STATE_WHITE_TO_COLORにする


// パーティクルのY座標
#define PARTICLE_Y  (FX_F32_TO_FX32(0.0f))


#ifdef SET_PARTICLE_Z_MODE
f32 particle_z = 0.0f;
#endif

#ifdef SET_REAR_FRAME_LENGTH_COLOR_MODE
u32 rear_frame_length_color = REAR_FRAME_LENGTH_COLOR;
#endif


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// パーティクルリソースデータ
//=====================================
typedef enum
{
  PARTICLE_SPA_FILE_0,      // ARCID_SHINKA_DEMO    // NARC_shinka_demo_particle_shinka_demo_spa
  PARTICLE_SPA_FILE_MAX
}
PARTICLE_SPA_FILE;

typedef struct
{
  int  arc;
  int  idx;
}
PARTICLE_ARC;

static const PARTICLE_ARC particle_arc[PARTICLE_SPA_FILE_MAX] =
{
  { ARCID_SHINKA_DEMO, NARC_shinka_demo_particle_shinka_demo_spa },
};

// パーティクルのポリゴンID
#define PARTICLE_FIX_POLYGON_ID ( 5)
#define PARTICLE_MIN_POLYGON_ID ( 6)
#define PARTICLE_MAX_POLYGON_ID (59)


//-------------------------------------
/// パーティクル
//=====================================
typedef struct
{
  u16  frame;
  u8   spa_idx;
  u8   res_no;
}
PARTICLE_PLAY_DATA;

typedef struct
{
  u8             wk[PARTICLE_LIB_HEAP_SIZE];
  GFL_PTC_PTR    ptc;
  u8             res_num;
}
PARTICLE_SPA_SET;

typedef struct
{
  u16                          frame;
  u16                          data_no;
  u16                          data_num;
  const PARTICLE_PLAY_DATA*    data_tbl;
  PARTICLE_SPA_SET             spa_set[PARTICLE_SPA_FILE_MAX];
  BOOL                         play;  // TRUEのとき実行中
  s32                          stop_count;  // stop_count>=0のときカウントダウンし、stop_count==0になったら実行中のものを全て消去する
}
PARTICLE_MANAGER;

static const PARTICLE_PLAY_DATA particle_play_data_tbl[] =
{
  {    0,     PARTICLE_SPA_FILE_0,         DEMO_SINKA01A },
  {    0,     PARTICLE_SPA_FILE_0,         DEMO_SINKA01 },
  {    0,     PARTICLE_SPA_FILE_0,         DEMO_SINKA02 },
  {    0,     PARTICLE_SPA_FILE_0,         DEMO_SINKA03 },
  {    0,     PARTICLE_SPA_FILE_0,         DEMO_SINKA04 },  // これをなくせば処理落ちしなくなる
  {  500,     PARTICLE_SPA_FILE_0,         DEMO_SINKA05 },
  {  500,     PARTICLE_SPA_FILE_0,         DEMO_SINKA06 },
  {  755/*563*//*620*/,     PARTICLE_SPA_FILE_0,         DEMO_SINKA07 },
  {  783/*840*/,     PARTICLE_SPA_FILE_0,         DEMO_SINKA08 },
};

//-------------------------------------
/// 3Dオブジェクトのプロパティ
//=====================================
typedef struct
{
  u16                         idx;        // GFL_G3D_UTILが割り振る番号
  GFL_G3D_OBJSTATUS           objstatus;
  BOOL                        draw;       // TRUEのとき描画する
}
THREE_OBJ_PROPERTY;
static THREE_OBJ_PROPERTY* ThreeObjPropertyCreateArray( HEAPID heap_id, u16 num );
static void ThreeObjPropertyDeleteArray( THREE_OBJ_PROPERTY* prop );
static THREE_OBJ_PROPERTY* ThreeObjPropertyCreateArray( HEAPID heap_id, u16 num )
{
  u16 i;
  THREE_OBJ_PROPERTY* prop_array = GFL_HEAP_AllocClearMemory( heap_id, sizeof(THREE_OBJ_PROPERTY) * num );
  // 0以外の値で初期化するものについて初期化を行う 
  for( i=0; i<num; i++ )
  {
    THREE_OBJ_PROPERTY* prop = &(prop_array[i]);
    VEC_Set( &(prop->objstatus.trans), 0, 0, 0 );
    VEC_Set( &(prop->objstatus.scale), FX32_ONE, FX32_ONE, FX32_ONE );
    MTX_Identity33( &(prop->objstatus.rotate) );
  }
  return prop_array;
}
static void ThreeObjPropertyDeleteArray( THREE_OBJ_PROPERTY* prop_array )
{
  GFL_HEAP_FreeMemory( prop_array );
}


//-------------------------------------
/// ワーク
//=====================================
struct _SHINKADEMO_EFFECT_WORK
{
  // 他のところのを覚えているだけで生成や破棄はしない。
  HEAPID                   heap_id;
  
  // SHINKADEMO_EFFECT_Initの引数
  SHINKADEMO_EFFECT_LAUNCH   launch;

  // ステップ
  STEP                     step;
  u32                      wait_count;

  // パーティクル
  PARTICLE_MANAGER*        particle_mgr;

  // 3D
  GFL_G3D_UTIL*            three_util;
  u16                      three_unit_idx[THREE_SETUP_IDX_MAX];             // GFL_G3D_UTILが割り振る番号        // 添え字はTHREE_SETUP_IDX_(THREE_SETUP_IDX_MAX)
  u16                      three_obj_prop_tbl_idx[THREE_USER_OBJ_IDX_MAX];  // three_obj_prop_tblのインデックス  // 添え字はTHREE_USER_OBJ_IDX_(THREE_USER_OBJ_IDX_MAX)
  u16                      three_obj_prop_num;
  THREE_OBJ_PROPERTY*      three_obj_prop_tbl;
  // 3Dフレーム
  u32                      three_frame;
  REAR_WHITE_ANIME_STATE   rear_white_anime_state;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// パーティクル
//=====================================
static PARTICLE_MANAGER* Particle_Init( HEAPID heap_id, u16 data_num, const PARTICLE_PLAY_DATA* data_tbl );
static void Particle_Exit( PARTICLE_MANAGER* mgr );
static void Particle_Main( PARTICLE_MANAGER* mgr );
static void Particle_Draw( PARTICLE_MANAGER* mgr );
static void Particle_Start( PARTICLE_MANAGER* mgr );
static void Particle_Stop( PARTICLE_MANAGER* mgr, s32 stop_count );
static void Particle_Vanish( PARTICLE_MANAGER* mgr, s32 stop_count );  // 今表示しているパーティクルを全て消す(パーティクルシステム自体は動き続けるので、これ移行の新しいパーティクルは再生される)


//-------------------------------------
/// 3D
//=====================================
// 3D全体
static void ShinkaDemo_Effect_ThreeInit( SHINKADEMO_EFFECT_WORK* work );
static void ShinkaDemo_Effect_ThreeExit( SHINKADEMO_EFFECT_WORK* work );
static void ShinkaDemo_Effect_ThreeDraw( SHINKADEMO_EFFECT_WORK* work );  // 3D描画(GRAPHIC_3D_StartDrawとPSEL_GRAPHIC_3D_EndDrawの間で呼ぶ)
// 3D個別
static void ShinkaDemo_Effect_ThreeRearInit( SHINKADEMO_EFFECT_WORK* work );
static void ShinkaDemo_Effect_ThreeRearExit( SHINKADEMO_EFFECT_WORK* work );
static void ShinkaDemo_Effect_ThreeRearMain( SHINKADEMO_EFFECT_WORK* work );
static void ShinkaDemo_Effect_ThreeRearStartColorToWhite( SHINKADEMO_EFFECT_WORK* work );
static BOOL ShinkaDemo_Effect_ThreeRearIsWhite( SHINKADEMO_EFFECT_WORK* work );
static void ShinkaDemo_Effect_ThreeRearStartWhiteToColor( SHINKADEMO_EFFECT_WORK* work );
static BOOL ShinkaDemo_Effect_ThreeRearIsColor( SHINKADEMO_EFFECT_WORK* work );


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief         初期化処理 
 *
 *  @param[in,out] 
 *
 *  @retval        SHINKADEMO_EFFECT_WORK
 */
//-----------------------------------------------------------------------------
SHINKADEMO_EFFECT_WORK* SHINKADEMO_EFFECT_Init(
                               HEAPID                     heap_id,
                               SHINKADEMO_EFFECT_LAUNCH   launch
                             )
{
  SHINKADEMO_EFFECT_WORK* work;

  // ワーク
  {
    work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(SHINKADEMO_EFFECT_WORK) );
  }

  // 引数
  {
    work->heap_id      = heap_id;
    work->launch       = launch;
  }

  // ステップ
  {
    if( work->launch == SHINKADEMO_EFFECT_LAUNCH_EVO )
      work->step         = STEP_EVO_START;
    else
      work->step         = STEP_AFTER;
  }
  work->wait_count = 0;
 
  // パーティクル
  work->particle_mgr = Particle_Init( work->heap_id, NELEMS(particle_play_data_tbl), particle_play_data_tbl );

  // 3D
  ShinkaDemo_Effect_ThreeInit( work );
  ShinkaDemo_Effect_ThreeRearInit( work );

  return work;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         終了処理 
 *
 *  @param[in,out] work  SHINKADEMO_EFFECT_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_EFFECT_Exit( SHINKADEMO_EFFECT_WORK* work )
{
  // 3D
  ShinkaDemo_Effect_ThreeRearExit( work );
  ShinkaDemo_Effect_ThreeExit( work );

  // パーティクル
  Particle_Exit( work->particle_mgr );

  // ワーク
  {
    GFL_HEAP_FreeMemory( work );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief         主処理 
 *
 *  @param[in,out] work  SHINKADEMO_EFFECT_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_EFFECT_Main( SHINKADEMO_EFFECT_WORK* work )
{
  switch(work->step)
  {
  // EVO
  case STEP_EVO_START:
    {
    }
    break;
  case STEP_EVO_DEMO:
    {
/*
      外からSHINKADEMO_EFFECT_StartWhite関数で設定することにした
      if( work->wait_count == 0 )
      {
        work->step = STEP_EVO_TO_WHITE_START;
      }
      else
      {
        work->wait_count--;
      }
*/
    }
    break;
  case STEP_EVO_TO_WHITE_START:
    {
      ShinkaDemo_Effect_ThreeRearStartColorToWhite( work );
      work->step = STEP_EVO_TO_WHITE;
    }
    break;
  case STEP_EVO_TO_WHITE:
    {
      if( ShinkaDemo_Effect_ThreeRearIsWhite( work ) )
      {
        work->step = STEP_EVO_WHITE;
        work->wait_count = REAR_FRAME_LENGTH_WHITE;

        // 今表示しているパーティクルを全て消す(パーティクルシステム自体は動き続けるので、これ移行の新しいパーティクルは再生される)
        Particle_Vanish( work->particle_mgr, 0 );
      }
    }
    break;
  case STEP_EVO_WHITE:
    {
      if( work->wait_count == 0 )
      {
        ShinkaDemo_Effect_ThreeRearStartWhiteToColor( work );
        work->step = STEP_EVO_FROM_WHITE;
      }
      else
      {
        work->wait_count--;
      }
    }
    break;
  case STEP_EVO_FROM_WHITE:
    {
      if( ShinkaDemo_Effect_ThreeRearIsColor( work ) )
      {
        work->step = STEP_EVO_END;
      }
    }
    break;
  case STEP_EVO_END:
    {
    }
    break;

  // AFTER
  case STEP_AFTER:
    {
    }
    break;
  }


#ifdef SET_PARTICLE_Z_MODE
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
  {
    OS_Printf( "particle_z = %f\n", particle_z );
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
  {
    particle_z += 0.1f;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  {
    particle_z -= 0.1f;
  }
#endif

#ifdef SET_REAR_FRAME_LENGTH_COLOR_MODE
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
  {
    OS_Printf( "rear_frame_length_color = %d\n", rear_frame_length_color );
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
  {
    rear_frame_length_color += 10;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  {
    rear_frame_length_color -= 10;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
  {
    rear_frame_length_color += 1;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
  {
    rear_frame_length_color -= 1;
  }
#endif

  // 3D
  ShinkaDemo_Effect_ThreeRearMain( work );

  // パーティクル
  Particle_Main( work->particle_mgr );
}

//-----------------------------------------------------------------------------
/**
 *  @brief         描画処理 
 *
 *  @param[in,out] work  SHINKADEMO_EFFECT_Initで生成したワーク
 *
 *  @retval        なし 
 *
 *  @note          GRAPHIC_3D_StartDrawとGRAPHIC_3D_EndDrawの間で呼ぶこと
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_EFFECT_Draw( SHINKADEMO_EFFECT_WORK* work )
{
  // 3D
  ShinkaDemo_Effect_ThreeDraw( work );

  // パーティクル
  Particle_Draw( work->particle_mgr );
}

//-----------------------------------------------------------------------------
/**
 *  @brief         進化キャンセル
 *
 *  @param[in,out] work  SHINKADEMO_EFFECT_Initで生成したワーク
 *
 *  @retval        なし
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_EFFECT_Cancel( SHINKADEMO_EFFECT_WORK* work )
{
  // パーティクル
  Particle_Stop( work->particle_mgr, 30 );

  // 3D
  if( work->step == STEP_EVO_DEMO )
  {
    work->step = STEP_EVO_TO_WHITE_START;
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief         スタート
 *
 *  @param[in,out] work  SHINKADEMO_EFFECT_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_EFFECT_Start( SHINKADEMO_EFFECT_WORK* work )
{
  // パーティクル
  Particle_Start( work->particle_mgr );

  work->step = STEP_EVO_DEMO;
  work->wait_count = REAR_FRAME_LENGTH_COLOR;

#ifdef SET_REAR_FRAME_LENGTH_COLOR_MODE
  work->wait_count = rear_frame_length_color;
#endif
}

//-----------------------------------------------------------------------------
/**
 *  @brief         REARを白く飛ばすアニメ開始
 *
 *  @param[in,out] work  SHINKADEMO_EFFECT_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_EFFECT_StartWhite( SHINKADEMO_EFFECT_WORK* work )
{
  // 3D
  if( work->step == STEP_EVO_DEMO )
  {
    work->step = STEP_EVO_TO_WHITE_START;
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief         REARが完全に白く飛んでいるか
 *
 *  @param[in,out] work  SHINKADEMO_EFFECT_Initで生成したワーク
 *
 *  @retval        REARが完全に白く飛んでいるときTRUE 
 */
//-----------------------------------------------------------------------------
BOOL SHINKADEMO_EFFECT_IsWhite( SHINKADEMO_EFFECT_WORK* work )
{
  // 3D
  if( work->step == STEP_EVO_WHITE )
  {
    return TRUE;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         REARが白から戻り始めているか
 *
 *  @param[in,out] work  SHINKADEMO_EFFECT_Initで生成したワーク
 *
 *  @retval        REARが白から戻り始めているときTRUE 
 */
//-----------------------------------------------------------------------------
BOOL SHINKADEMO_EFFECT_IsFromWhite( SHINKADEMO_EFFECT_WORK* work )
{
  // 3D
  if(    work->step == STEP_EVO_WHITE
      && work->wait_count <= 1 )
  {
    return TRUE;
  }
  else if( work->step == STEP_EVO_FROM_WHITE )
  {
    return TRUE;
  }
  return FALSE;
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// パーティクル
//=====================================
static PARTICLE_MANAGER* Particle_Init( HEAPID heap_id, u16 data_num, const PARTICLE_PLAY_DATA* data_tbl )
{
  PARTICLE_MANAGER* mgr;
  u8 i;

  // パーティクル専用カメラ	
/*
  VecFx32 cam_eye = {            0,         0,   FX32_CONST(10) };
  VecFx32 cam_up  = {            0,  FX32_ONE,                0 };
  VecFx32 cam_at  = {            0,         0,                0 };
*/
/*
  VecFx32 cam_eye = {            0,         0,   FX32_CONST(2) };
  VecFx32 cam_up  = {            0,  FX32_ONE,                0 };
  VecFx32 cam_at  = {            0,         0,                0 };
*/

// 全く見えない
//  VecFx32 cam_eye = {            0,  FX32_CONST(10),  FX32_CONST(100) };
//  VecFx32 cam_up  = {            0,        FX32_ONE,                0 };
//  VecFx32 cam_at  = {            0,               0,                0 };

// そこそこいい感じ
//  VecFx32 cam_eye = {            0,  FX32_CONST(1),  FX32_CONST(10) };
//  VecFx32 cam_up  = {            0,        FX32_ONE,                0 };
//  VecFx32 cam_at  = {            0,               0,                0 };

// もうちょっと近くてもいいかも
//  VecFx32 cam_eye = {            0,  FX32_CONST(0),  FX32_CONST(15) };
//  VecFx32 cam_up  = {            0,        FX32_ONE,                0 };
//  VecFx32 cam_at  = {            0,               0,                0 };

  VecFx32 cam_eye = {            0,  FX32_CONST(0),  FX32_CONST(13) };
  VecFx32 cam_up  = {            0,        FX32_ONE,                0 };
  VecFx32 cam_at  = {            0,               0,                0 };


/*
  GFL_G3D_PROJECTION proj; 
	{
		proj.type      = GFL_G3D_PRJORTH;
		proj.param1    =  FX32_CONST(3);
		proj.param2    = -FX32_CONST(3);
		proj.param3    = -FX32_CONST(4);
		proj.param4    =  FX32_CONST(4);
		proj.near      = defaultCameraNear;
		proj.far       = defaultCameraFar;
		proj.scaleW    = 0;
	}
*/
/*
  GFL_G3D_PROJECTION proj; 
	{
		proj.type      = GFL_G3D_PRJPERS;
		proj.param1    = FX_SinIdx( defaultCameraFovy/2 *PERSPWAY_COEFFICIENT );
		proj.param2    = FX_CosIdx( defaultCameraFovy/2 *PERSPWAY_COEFFICIENT );
		proj.param3    = defaultCameraAspect;
		proj.param4    = 0;
		proj.near      = defaultCameraNear;
		proj.far       = defaultCameraFar;
		proj.scaleW    = 0;
	}
*/
/*
  GFL_G3D_PROJECTION proj; 
	{
		proj.type      = GFL_G3D_PRJPERS;
		proj.param1    = FX_SinIdx( 80/2 *PERSPWAY_COEFFICIENT );
		proj.param2    = FX_CosIdx( 80/2 *PERSPWAY_COEFFICIENT );
		proj.param3    = defaultCameraAspect;
		proj.param4    = 0;
		proj.near      = defaultCameraNear;
		proj.far       = defaultCameraFar;
		proj.scaleW    = 0;
	}
*/
  GFL_G3D_PROJECTION proj; 
	{
		proj.type      = GFL_G3D_PRJPERS;
		proj.param1    = FX_SinIdx( defaultCameraFovy/2 *PERSPWAY_COEFFICIENT );
		proj.param2    = FX_CosIdx( defaultCameraFovy/2 *PERSPWAY_COEFFICIENT );
		proj.param3    = defaultCameraAspect;
		proj.param4    = 0;
		proj.near      = defaultCameraNear;
		proj.far       = defaultCameraFar;
		proj.scaleW    = 0;
	}

  // PARTICLE_MANAGER
  {
    mgr = GFL_HEAP_AllocClearMemory( heap_id, sizeof(PARTICLE_MANAGER) );
    mgr->frame           = 0;
    mgr->data_no         = 0;
    mgr->data_num        = data_num;
    mgr->data_tbl        = data_tbl;
    mgr->play            = FALSE;
    mgr->stop_count      = -1;
  }

  GFL_PTC_Init( heap_id );

  for(i=0; i<PARTICLE_SPA_FILE_MAX; i++)
  {
    void* arc_res;

    //mgr->spa_set[i].ptc = GFL_PTC_Create( mgr->spa_set[i].wk, sizeof(mgr->spa_set[i].wk), TRUE, heap_id );
    
    //mgr->spa_set[i].ptc = GFL_PTC_Create2( mgr->spa_set[i].wk, sizeof(mgr->spa_set[i].wk), TRUE, heap_id, 5, 6, 63 );  // 3DのポリゴンIDは5にしているが、これだとパーティクルが3Dによって欠けてしまうことがある。単純にZがパーティクルのほうが後ろなのだろうか？
    //mgr->spa_set[i].ptc = GFL_PTC_Create2( mgr->spa_set[i].wk, sizeof(mgr->spa_set[i].wk), TRUE, heap_id, 5, 6, 59 );  // これだとパーティクルが欠けない。6から59よりかなり小さい範囲だと、パーティクル同士で欠けてしまう。
    mgr->spa_set[i].ptc = GFL_PTC_CreateEx( mgr->spa_set[i].wk, sizeof(mgr->spa_set[i].wk), TRUE,
        PARTICLE_FIX_POLYGON_ID, PARTICLE_MIN_POLYGON_ID, PARTICLE_MAX_POLYGON_ID, heap_id );

    //GFL_PTC_PersonalCameraDelete( mgr->spa_set[i].ptc );
		//GFL_PTC_PersonalCameraCreate( mgr->spa_set[i].ptc, &proj, DEFAULT_PERSP_WAY, &cam_eye, &cam_up, &cam_at, heap_id );
    GFL_PTC_PersonalCameraDelete( mgr->spa_set[i].ptc );
		GFL_PTC_PersonalCameraCreate( mgr->spa_set[i].ptc, &proj, DEFAULT_PERSP_WAY, &cam_eye, &cam_up, &cam_at, heap_id );
    
    arc_res = GFL_PTC_LoadArcResource( particle_arc[i].arc, particle_arc[i].idx, heap_id );
    mgr->spa_set[i].res_num = GFL_PTC_GetResNum( arc_res );
    GFL_PTC_SetResource( mgr->spa_set[i].ptc, arc_res, TRUE, NULL );
  }

  return mgr;
}

static void Particle_Exit( PARTICLE_MANAGER* mgr )
{
  GFL_PTC_Exit();

  GFL_HEAP_FreeMemory( mgr );
}


static void Particle_EmitFunc( GFL_EMIT_PTR emit_ptr );
static void Particle_EmitFunc( GFL_EMIT_PTR emit_ptr )
{
  void* temp_ptr;


  VecFx32 pos = { 0, PARTICLE_Y, 0 };//FX_F32_TO_FX32(0.5f), 0 };

#ifdef SET_PARTICLE_Z_MODE
    pos.z = FX_F32_TO_FX32(particle_z);
#endif
 

  temp_ptr = GFL_PTC_GetTempPtr();


  GFL_PTC_SetEmitterPosition( emit_ptr, &pos );


  if( emit_ptr )
  {
    SPLEmitter* spl_emitter = emit_ptr;
    SPLParticle* spl_particle;
    spl_particle = spl_emitter->act_ptcl_list.p_begin;
    while( spl_particle )
    {
#ifdef DEBUG_CODE
      OS_Printf("%d\n", spl_particle->alp.current_polygonID );
#endif
      spl_particle = spl_particle->p_next;
    }    
  }
}


static void Particle_Main( PARTICLE_MANAGER* mgr )
{
  if( mgr->play )
  {
    VecFx32 pos = { 0, PARTICLE_Y, 0 };//FX_F32_TO_FX32(0.5f), 0 };
    GFL_EMIT_PTR emit;


#ifdef SET_PARTICLE_Z_MODE
    pos.z = FX_F32_TO_FX32(particle_z);
#endif
/*
    pos.z = FX_F32_TO_FX32(2.0f);
*/

    while( mgr->data_no < mgr->data_num )
    {
      if( mgr->frame == mgr->data_tbl[mgr->data_no].frame )
      {
        emit = GFL_PTC_CreateEmitter( mgr->spa_set[ mgr->data_tbl[mgr->data_no].spa_idx ].ptc, mgr->data_tbl[mgr->data_no].res_no, &pos );
        //emit = GFL_PTC_CreateEmitterCallback( mgr->spa_set[ mgr->data_tbl[mgr->data_no].spa_idx ].ptc, mgr->data_tbl[mgr->data_no].res_no, Particle_EmitFunc, NULL );

#ifdef DEBUG_CODE
        {
          OS_Printf( "SHINKADEMO_EFFECT : frame=%d, jspa_idx=%d, res_no=%d, emit=%p\n",
              mgr->frame, mgr->data_tbl[mgr->data_no].spa_idx, mgr->data_tbl[mgr->data_no].res_no, emit );
        }
#endif

        mgr->data_no++;
      }
      else
      {
        break;
      }
    }
    mgr->frame++;
  }

  if( mgr->stop_count >= 0 )
  {
    if( mgr->stop_count == 0 )
    {
      u8 i;
      for(i=0; i<PARTICLE_SPA_FILE_MAX; i++)
      {
        GFL_PTC_DeleteEmitterAll( mgr->spa_set[i].ptc );
      }
    }
    mgr->stop_count--;
  }
}

static void Particle_Draw( PARTICLE_MANAGER* mgr )
{
  GFL_PTC_Main();
}

static void Particle_Start( PARTICLE_MANAGER* mgr )
{
  mgr->play = TRUE;
}

static void Particle_Stop( PARTICLE_MANAGER* mgr, s32 stop_count )
{
  mgr->play = FALSE;
  mgr->stop_count = stop_count;
}

static void Particle_Vanish( PARTICLE_MANAGER* mgr, s32 stop_count )  // 今表示しているパーティクルを全て消す(パーティクルシステム自体は動き続けるので、これ移行の新しいパーティクルは再生される)
{
  mgr->stop_count = stop_count;
}


//-------------------------------------
/// 3D
//=====================================
// 3D全体
static void ShinkaDemo_Effect_ThreeInit( SHINKADEMO_EFFECT_WORK* work )
{
  // レンダリングスワップバッファ
  //GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_W );

  // 3D管理ユーティリティーのセットアップ
  work->three_util = GFL_G3D_UTIL_Create( THREE_RES_MAX, THREE_OBJ_MAX, work->heap_id );

  // NULL、ゼロ初期化
  work->three_obj_prop_num = 0;
}
static void ShinkaDemo_Effect_ThreeExit( SHINKADEMO_EFFECT_WORK* work )
{
  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( work->three_util );
}
static void ShinkaDemo_Effect_ThreeDraw( SHINKADEMO_EFFECT_WORK* work )  // 3D描画(GRAPHIC_3D_StartDrawとGRAPHIC_3D_EndDrawの間で呼ぶ)
{
  u16 i;
  for( i=0; i<work->three_obj_prop_num; i++ )
  {
    THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[i]);
    if( prop->draw )
    {
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
      GFL_G3D_DRAW_DrawObject( obj, &(prop->objstatus) );
    }
  }
}
// 3D個別
static void ShinkaDemo_Effect_ThreeRearInit( SHINKADEMO_EFFECT_WORK* work )
{
  // ユニット追加
  {
    u16 i = THREE_SETUP_IDX_REAR;
    {
      work->three_unit_idx[i] = GFL_G3D_UTIL_AddUnit( work->three_util, &three_setup_tbl[i] );
    }
  }

  // オブジェクト全体
  {
    work->three_obj_prop_num = 1;
    work->three_obj_prop_tbl = ThreeObjPropertyCreateArray( work->heap_id, work->three_obj_prop_num );
  }

  // オブジェクト
  {
    u16 h = 0;

    u16 i = THREE_SETUP_IDX_REAR;
    {
      u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( work->three_util, work->three_unit_idx[i] );
      u16 head_user_obj_idx = THREE_USER_OBJ_IDX_REAR;
      u16 j = 0;
      {
        THREE_OBJ_PROPERTY* prop;
        work->three_obj_prop_tbl_idx[head_user_obj_idx +j] = h;
        prop = &(work->three_obj_prop_tbl[h]);
        prop->idx  = head_obj_idx +j;
        VEC_Set( &(prop->objstatus.trans), 0, FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f) );
/*
        VEC_Set( &(prop->objstatus.trans), 0, FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(-600.0f) );
*/
        prop->draw = TRUE;
        h++;
      }
    }
  }

  // アニメーション有効化
  {
    u16 i;
    for( i=0; i<work->three_obj_prop_num; i++ )
    {
      THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[i]);
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
      //u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      //u16 j;
      //for( j=0; j<anime_count; j++ )
      //{
      //  GFL_G3D_OBJECT_EnableAnime( obj, j );
      //}
      GFL_G3D_OBJECT_EnableAnime( obj, REAR_ANM_T );
    }
  }

  // フレーム
  // プログラムでは開始フレームを0として処理する。デザイナーさんの3Dオーサリングツール上でも0フレームからスタートしているようだ。
  work->three_frame = 0;

  // REARを白く飛ばすアニメの状態
  {
    THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[ work->three_obj_prop_tbl_idx[THREE_USER_OBJ_IDX_REAR] ]);
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
    int anime_frame = REAR_COLOR_FRAME;
    work->rear_white_anime_state = REAR_WHITE_ANIME_STATE_COLOR;
    GFL_G3D_OBJECT_EnableAnime( obj, REAR_ANM_C );
    GFL_G3D_OBJECT_EnableAnime( obj, REAR_ANM_M );
	  GFL_G3D_OBJECT_SetAnimeFrame( obj, REAR_ANM_C, &anime_frame );
	  GFL_G3D_OBJECT_SetAnimeFrame( obj, REAR_ANM_M, &anime_frame );
  }
}
static void ShinkaDemo_Effect_ThreeRearExit( SHINKADEMO_EFFECT_WORK* work )
{
  // ユニット破棄
  {
    u16 i = THREE_SETUP_IDX_REAR;
    {
      GFL_G3D_UTIL_DelUnit( work->three_util, work->three_unit_idx[i] );
    }
  }

  // オブジェクト全体
  {
    ThreeObjPropertyDeleteArray( work->three_obj_prop_tbl );
    work->three_obj_prop_num = 0;
  }
}
static void ShinkaDemo_Effect_ThreeRearMain( SHINKADEMO_EFFECT_WORK* work )
{
  const fx32 anime_speed = THREE_ADD;  // 増加分（FX32_ONEで１フレーム進める）
  
  // アニメーション更新
  {
    u16 i;
    for( i=0; i<work->three_obj_prop_num; i++ )
    {
      THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[i]);
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
      //u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      //u16 j;
      //for( j=0; j<anime_count; j++ )
      //{
      //  GFL_G3D_OBJECT_LoopAnimeFrame( obj, j, anime_speed );
      //}
      GFL_G3D_OBJECT_LoopAnimeFrame( obj, REAR_ANM_T, anime_speed );
    }
  }
  
  // フレーム
  work->three_frame++;

  // REARを白く飛ばすアニメの状態
  {
    THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[ work->three_obj_prop_tbl_idx[THREE_USER_OBJ_IDX_REAR] ]);
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
    BOOL b_continue_c = TRUE;
    BOOL b_continue_m = TRUE;
    fx32 anime_add;  // 増加分（FX32_ONEで１フレーム進める）
    if( work->rear_white_anime_state == REAR_WHITE_ANIME_STATE_COLOR_TO_WHITE )
    {
      anime_add = FX32_ONE * 5;
	    b_continue_c = GFL_G3D_OBJECT_IncAnimeFrame( obj, REAR_ANM_C, anime_add );
	    b_continue_m = GFL_G3D_OBJECT_IncAnimeFrame( obj, REAR_ANM_M, anime_add );
      if( (!b_continue_c) && (!b_continue_m) )
      {
        work->rear_white_anime_state = REAR_WHITE_ANIME_STATE_WHITE;
      }
    }
    else if( work->rear_white_anime_state == REAR_WHITE_ANIME_STATE_WHITE_TO_COLOR )
    {
      anime_add = FX32_ONE * (-2);
	    b_continue_c = GFL_G3D_OBJECT_IncAnimeFrame( obj, REAR_ANM_C, anime_add );
	    b_continue_m = GFL_G3D_OBJECT_IncAnimeFrame( obj, REAR_ANM_M, anime_add );
      if( (!b_continue_c) && (!b_continue_m) )
      {
        work->rear_white_anime_state = REAR_WHITE_ANIME_STATE_COLOR;
      }
    }
  }
}
static void ShinkaDemo_Effect_ThreeRearStartColorToWhite( SHINKADEMO_EFFECT_WORK* work )
{
  work->rear_white_anime_state = REAR_WHITE_ANIME_STATE_COLOR_TO_WHITE;
}
static BOOL ShinkaDemo_Effect_ThreeRearIsWhite( SHINKADEMO_EFFECT_WORK* work )
{
  return ( work->rear_white_anime_state == REAR_WHITE_ANIME_STATE_WHITE );
}
static void ShinkaDemo_Effect_ThreeRearStartWhiteToColor( SHINKADEMO_EFFECT_WORK* work )
{
  work->rear_white_anime_state = REAR_WHITE_ANIME_STATE_WHITE_TO_COLOR;
}
static BOOL ShinkaDemo_Effect_ThreeRearIsColor( SHINKADEMO_EFFECT_WORK* work )
{
  return ( work->rear_white_anime_state == REAR_WHITE_ANIME_STATE_COLOR );
}

