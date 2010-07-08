//============================================================================
/**
 *  @file   egg_demo_view.c
 *  @brief  タマゴ孵化デモの演出
 *  @author Koji Kawada
 *  @data   2010.01.25
 *  @note   
 *
 *  モジュール名：EGG_DEMO_VIEW
 */
//============================================================================
//#define DEBUG_KAWADA
//#define SET_PARTICLE_Y_MODE  // これが定義されているとき、パーティクルのY値を編集できるモードになる
//#define DEBUG_SET_POKE_ARRANGE  // これが定義されているとき、ポケモンの配置を調整できるモードになる


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/mcss.h"
#include "system/mcss_tool.h"
#include "poke_tool/monsno_def.h"

#include "egg_demo_view.h"


// アーカイブ
#include "arc_def.h"
#include "../../../../resource/egg_demo/egg_demo_setup.h"
#include "egg_demo_particle.naix"
// サウンド
//#include "sound/pm_voice.h"
#include "sound/pm_wb_voice.h"  // wbではpm_voiceではなくこちらを使う
#include "sound/wb_sound_data.sadl"
#include "sound/pm_sndsys.h"


//=============================================================================
/**
 *  定数定義
 */
//=============================================================================
// ステップ
typedef enum
{
  STEP_EGG_START,       // タマゴ開始待ち中→デモ
  STEP_EGG_DEMO,        // タマゴデモ
  STEP_EGG_WHITE,       // タマゴ白へ
  STEP_EGG_ALPHA,       // タマゴ透明へ
  STEP_EGG_END,         // タマゴ終了中
  STEP_MON_READY,       // ポケモン準備中
  STEP_MON_WHITE,       // ポケモン白で待ち
  STEP_MON_START,       // ポケモン開始待ち中→鳴く
  STEP_MON_CRY,         // ポケモン鳴き終わるのを待ち中→終了する
  STEP_MON_END,         // ポケモン終了中
}
STEP;

// パーティクル対応
#define POKE_TEX_ADRS_FOR_PARTICLE  (0x30000)

// ポケモン
#define POKE_SCALE       (16.0f)
#define POKE_SIZE_MAX    (96.0f)

// マナフィ以外のポケモン
// パーティクルのフレームとポケモンアニメーションのフレームを連携させる
#define PARTICLE_BURST_FRAME   (380)  // ひびが入っている絵の最終フレーム  // 全部で381個フレームがあった
#define POKE_ANIME_INDEX (1)

#define EGG_CRACK_FRAME_01      ( 24 -4)  // ヒビ小開始
#define EGG_CRACK_FRAME_02      (112 -3)  // ヒビ中開始 
#define EGG_CRACK_FRAME_03      (227 -9)  // ヒビ大開始

#define EGG_LAND_FRAME_01   ( 10)  // 着地A-1
#define EGG_LAND_FRAME_02   ( 15)  // 着地A-2
#define EGG_LAND_FRAME_03   ( 85)  // 着地B-1
#define EGG_LAND_FRAME_04   ( 90)  // 着地B-2
#define EGG_SHAKE_FRAME_01   (150)  // 揺れ始め
#define EGG_SHAKE_FRAME_02   (309)  // 激しい揺れ開始

#define EGG_SHAKE_WAIT_COUNT_01   (20)  // EGG_SHAKE_FRAME_01<= <EGG_SHAKE_FRAME_02  // SEが鳴り終わったらこれだけ間隔を空ける
#define EGG_SHAKE_WAIT_COUNT_02   (0)   // EGG_SHAKE_FRAME_02<=

// マナフィ
// パーティクルのフレームとポケモンアニメーションのフレームを連携させる
#define MANAFI_PARTICLE_BURST_FRAME   (300)  // パーティクルエフェクトDEMO_MANA01, DEMO_MANA02は全部で300フレームあるらしい。
#define MANAFI_POKE_ANIME_INDEX (1)

#define MANAFI_EGG_SE_FRAME_01  ( 10)  // 1つ目
//#define MANAFI_EGG_SE_FRAME_02  (100)  // 2つ目  // 2つ目は1つ目が終わってからすぐ鳴らすことにする

typedef enum
{
  MANAFI_EGG_SE_STATE_BEFORE_PLAY,  // 再生前で何も再生していない
  MANAFI_EGG_SE_STATE_01,           // 1つ目再生中
  MANAFI_EGG_SE_STATE_02,           // 2つ目再生中
  MANAFI_EGG_SE_STATE_AFTER_PLAY,   // 再生後で何も再生していない
}
MANAFI_EGG_SE_STATE;


// 3D
// 3D個別
static const GFL_G3D_UTIL_RES rear_res_tbl[] =
{
  { ARCID_EGG_DEMO, NARC_egg_demo_particle_demo_sinka02_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_EGG_DEMO, NARC_egg_demo_particle_demo_sinka02_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_EGG_DEMO, NARC_egg_demo_particle_demo_sinka02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_EGG_DEMO, NARC_egg_demo_particle_demo_sinka02_nsbma, GFL_G3D_UTIL_RESARC },
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


// ポケモンとパーティクルのY座標
#define POKE_Y      (FX_F32_TO_FX32(-190.0f))//(FX_F32_TO_FX32(-100.0f))
#define PARTICLE_Y  (FX_F32_TO_FX32(-0.5f))


#ifdef SET_PARTICLE_Y_MODE
f32 particle_y = 0.0f;
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
  PARTICLE_SPA_FILE_0,      // ARCID_EGG_DEMO    // NARC_egg_demo_particle_egg_demo_spa
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
  { ARCID_EGG_DEMO, NARC_egg_demo_particle_egg_demo_spa  },
};

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

//-------------------------------------
/// パーティクル再生データ
//=====================================
static const PARTICLE_PLAY_DATA particle_play_data_tbl[] =
{
  {    PARTICLE_BURST_FRAME,     PARTICLE_SPA_FILE_0,        DEMO_EGG01 },
  {    PARTICLE_BURST_FRAME,     PARTICLE_SPA_FILE_0,        DEMO_EGG02 },
  {    PARTICLE_BURST_FRAME,     PARTICLE_SPA_FILE_0,        DEMO_EGG03 },
};

//   0 ヒビなし
//  24 ヒビ小開始
//  28 ヒビ小拡大
//  78 ヒビ中開始
//  82 ヒビ中拡大
// 132 ヒビ大開始
// 202 = 0

static const PARTICLE_PLAY_DATA manafi_particle_play_data_tbl[] =
{
  {    0,                               PARTICLE_SPA_FILE_0,        DEMO_MANA01 },  // 再生開始フレームが早い順に並べておくこと
  {    0,                               PARTICLE_SPA_FILE_0,        DEMO_MANA02 },
  {    MANAFI_PARTICLE_BURST_FRAME,     PARTICLE_SPA_FILE_0,        DEMO_EGG01  },
  {    MANAFI_PARTICLE_BURST_FRAME,     PARTICLE_SPA_FILE_0,        DEMO_EGG02  },
  {    MANAFI_PARTICLE_BURST_FRAME,     PARTICLE_SPA_FILE_0,        DEMO_EGG03  },
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
struct _EGG_DEMO_VIEW_WORK
{
  // 他のところのを覚えているだけで生成や破棄はしない。
  HEAPID                   heap_id;
  const POKEMON_PARAM*     pp;
  u16                      monsno;
  
  // ステップ
  STEP                     step;

  // フラグなど
  BOOL                     b_start;
  BOOL                     b_white;
  //u32                      voicePlayerIdx;
  u32                      wait_count;

  // マナフィ以外の普通のタマゴの揺れ
  BOOL                     egg_shake_se_play;      // 再生中ならTRUE
  SEPLAYER_ID              egg_shake_seplayer_id;  // egg_shake_se_playがTRUEのとき有効
  u32                      egg_shake_wait_count;
  // マナフィのSEの状態
  MANAFI_EGG_SE_STATE      manafi_egg_se_state;

  // VBlank中TCB
  GFL_TCB*                 vblank_tcb;

  // MCSS
  MCSS_SYS_WORK*           mcss_sys_wk;
  MCSS_WORK*               mcss_wk;
  BOOL                     mcss_anime_end;
  BOOL                     mcss_alpha_anime;
  u8                       mcss_alpha_end;

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
// VBlank関数
static void Egg_Demo_View_VBlankFunc( GFL_TCB* tcb, void* wk );

//-------------------------------------
/// MCSS
//=====================================
static void Egg_Demo_View_McssSysInit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_McssSysExit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_McssInit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_McssExit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_McssVanish( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_McssSetAnimeIndex( EGG_DEMO_VIEW_WORK* work, int index, BOOL b_last_stop );
static void Egg_Demo_View_McssCallBackFunctor( u32 data, fx32 currentFrame );
static void Egg_Demo_View_McssAlphaAnimeStart( EGG_DEMO_VIEW_WORK* work, u8 start_alpha, u8 end_alpha );
static void Egg_Demo_View_McssAlphaAnimeMain( EGG_DEMO_VIEW_WORK* work );
static BOOL Egg_Demo_View_McssAlphaAnimeIsEnd( EGG_DEMO_VIEW_WORK* work );

//-------------------------------------
/// タマゴ
//=====================================
static void Egg_Demo_View_EggStopWhite( EGG_DEMO_VIEW_WORK* work );
static BOOL Egg_Demo_View_EggIsWhite( EGG_DEMO_VIEW_WORK* work );

//-------------------------------------
/// ポケモン
//=====================================
static void Egg_Demo_View_MonColor( EGG_DEMO_VIEW_WORK* work );

//-------------------------------------
/// パーティクル
//=====================================
static PARTICLE_MANAGER* Particle_Init( HEAPID heap_id, u16 data_num, const PARTICLE_PLAY_DATA* data_tbl );
static void Particle_Exit( PARTICLE_MANAGER* mgr );
static void Particle_Main( PARTICLE_MANAGER* mgr );
static void Particle_Draw( PARTICLE_MANAGER* mgr );
static void Particle_Start( PARTICLE_MANAGER* mgr );
static void Particle_Stop( PARTICLE_MANAGER* mgr, s32 stop_count );

//-------------------------------------
/// 3D
//=====================================
// 3D全体
static void Egg_Demo_View_ThreeInit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_ThreeExit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_ThreeDraw( EGG_DEMO_VIEW_WORK* work );  // 3D描画(GRAPHIC_3D_StartDrawとPSEL_GRAPHIC_3D_EndDrawの間で呼ぶ)
// 3D個別
static void Egg_Demo_View_ThreeRearInit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_ThreeRearExit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_ThreeRearMain( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_ThreeRearStartColorToWhite( EGG_DEMO_VIEW_WORK* work );
static BOOL Egg_Demo_View_ThreeRearIsWhite( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_ThreeRearStartWhiteToColor( EGG_DEMO_VIEW_WORK* work );
static BOOL Egg_Demo_View_ThreeRearIsColor( EGG_DEMO_VIEW_WORK* work );

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
 *  @retval        EGG_DEMO_VIEW_WORK
 */
//-----------------------------------------------------------------------------
EGG_DEMO_VIEW_WORK* EGG_DEMO_VIEW_Init(
                               HEAPID                   heap_id,
                               const POKEMON_PARAM*     pp
                    )
{
  EGG_DEMO_VIEW_WORK* work;

  // ワーク
  {
    work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(EGG_DEMO_VIEW_WORK) );
  }

  // 引数
  {
    work->heap_id      = heap_id;
    work->pp           = pp;
    work->monsno       = (u16)PP_Get( work->pp, ID_PARA_monsno, NULL );
  }

  // ステップ
  {
    work->step         = STEP_EGG_START;
  }

  // フラグなど
  {
    work->b_start          = FALSE;
    work->b_white          = FALSE;
    //work->voicePlayerIdx   = 0;
    work->wait_count       = 0;
  }

  // マナフィ以外の普通のタマゴの揺れ
  {
    work->egg_shake_se_play = FALSE;
    work->egg_shake_wait_count = 0;
  }
  // マナフィのSEの状態
  {
    work->manafi_egg_se_state = MANAFI_EGG_SE_STATE_BEFORE_PLAY;
  }

  // VBlank中TCB
  {
    work->vblank_tcb = GFUser_VIntr_CreateTCB( Egg_Demo_View_VBlankFunc, work, 1 );
  }
  
  // MCSS
  {
    Egg_Demo_View_McssSysInit( work );
    Egg_Demo_View_McssInit( work );
  }

  // パーティクル
  if( work->monsno == MONSNO_MANAFI )
  {
    work->particle_mgr = Particle_Init( work->heap_id, NELEMS(manafi_particle_play_data_tbl), manafi_particle_play_data_tbl );
  }
  else
  {
    work->particle_mgr = Particle_Init( work->heap_id, NELEMS(particle_play_data_tbl), particle_play_data_tbl );
  }

  // 3D
  Egg_Demo_View_ThreeInit( work );
  Egg_Demo_View_ThreeRearInit( work );

  return work;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         終了処理 
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void EGG_DEMO_VIEW_Exit( EGG_DEMO_VIEW_WORK* work )
{
  // 3D
  Egg_Demo_View_ThreeRearExit( work );
  Egg_Demo_View_ThreeExit( work );

  // パーティクル
  Particle_Exit( work->particle_mgr );

  // MCSS
  {
    Egg_Demo_View_McssExit( work );
    Egg_Demo_View_McssSysExit( work );
  }

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // ワーク
  {
    GFL_HEAP_FreeMemory( work );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief         主処理 
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void EGG_DEMO_VIEW_Main( EGG_DEMO_VIEW_WORK* work )
{
  switch(work->step)
  {
  case STEP_EGG_START:
    {
      if( work->b_start )
      {
        // 次へ
        work->step = STEP_EGG_DEMO;
        work->wait_count = 0;

        // アニメーション
        Egg_Demo_View_McssSetAnimeIndex( work, (work->monsno==MONSNO_MANAFI)?(MANAFI_POKE_ANIME_INDEX):(POKE_ANIME_INDEX), TRUE );
        
        // パーティクル
        Particle_Start( work->particle_mgr );
      }
    }
    break;
  case STEP_EGG_DEMO:
    {
      u32 particle_burst_frame = (work->monsno==MONSNO_MANAFI)?(MANAFI_PARTICLE_BURST_FRAME):(PARTICLE_BURST_FRAME);

      work->wait_count++;
      if( work->monsno == MONSNO_MANAFI )
      {
        switch( work->manafi_egg_se_state )
        {
        case MANAFI_EGG_SE_STATE_BEFORE_PLAY:
          {
            if( work->wait_count == MANAFI_EGG_SE_FRAME_01 )
            {
              PMSND_PlaySE( SEQ_SE_EDEMO_02 );
              work->manafi_egg_se_state = MANAFI_EGG_SE_STATE_01;
            }
          }
          break;
        case MANAFI_EGG_SE_STATE_01:
          {
            if( !PMSND_CheckPlaySE() )
            {
              PMSND_PlaySE( SEQ_SE_EDEMO_03 );
              work->manafi_egg_se_state = MANAFI_EGG_SE_STATE_02;
            }
          }
          break;
        case MANAFI_EGG_SE_STATE_02:
          {
            if( !PMSND_CheckPlaySE() )
            {
              work->manafi_egg_se_state = MANAFI_EGG_SE_STATE_AFTER_PLAY;
            }
          }
          break;
        case MANAFI_EGG_SE_STATE_AFTER_PLAY:
          {
          }
          break;
        }
      }
      else
      {
        // プレイヤー Player1
        if( work->wait_count == EGG_CRACK_FRAME_01 )
        {
          PMSND_PlaySE( SEQ_SE_EDEMO_01 );
        }
        else if( work->wait_count == EGG_CRACK_FRAME_02 )
        {
          PMSND_PlaySE( SEQ_SE_EDEMO_01 );
        }
        else if( work->wait_count == EGG_CRACK_FRAME_03 )
        {
          PMSND_PlaySE( SEQ_SE_W181_01 );
        }

        // プレイヤー player sys
        if( work->wait_count == EGG_LAND_FRAME_01 )
        {
          PMSND_PlaySE( SEQ_SE_EDEMO_05 );
        }
        else if( work->wait_count == EGG_LAND_FRAME_02 )
        {
          PMSND_PlaySE( SEQ_SE_EDEMO_05 );
        }
        else if( work->wait_count == EGG_LAND_FRAME_03 )
        {
          PMSND_PlaySE( SEQ_SE_EDEMO_05 );
        }
        else if( work->wait_count == EGG_LAND_FRAME_04 )
        {
          PMSND_PlaySE( SEQ_SE_EDEMO_05 );
        }
        /*
        else if( work->wait_count == EGG_SHAKE_FRAME_01 )
        {
          PMSND_PlaySE( SEQ_SE_EDEMO_04 );
        }
        else if( work->wait_count == EGG_SHAKE_FRAME_02 )
        {
          PMSND_PlaySE( SEQ_SE_EDEMO_04 );
        }
        */

        // マナフィ以外の普通のタマゴの揺れ
        if( work->egg_shake_se_play )
        {
          if( !PMSND_CheckPlaySE_byPlayerID( work->egg_shake_seplayer_id ) )
          {
            work->egg_shake_se_play = FALSE;
            if( EGG_SHAKE_FRAME_01<=work->wait_count && work->wait_count<EGG_SHAKE_FRAME_02 )
            {
              work->egg_shake_wait_count = EGG_SHAKE_WAIT_COUNT_01;
            }
            else if( EGG_SHAKE_FRAME_02<work->wait_count )
            {
              work->egg_shake_wait_count = EGG_SHAKE_WAIT_COUNT_02;
            }
          }
        }
        if( work->wait_count >= EGG_SHAKE_FRAME_01 && work->wait_count < particle_burst_frame )
        {
          if( !(work->egg_shake_se_play) )
          {
            if( work->egg_shake_wait_count == 0 )
            {
              work->egg_shake_seplayer_id = PMSND_GetSE_DefaultPlayerID( SEQ_SE_EDEMO_04 );
              PMSND_PlaySE_byPlayerID( SEQ_SE_EDEMO_04, work->egg_shake_seplayer_id );
              work->egg_shake_se_play = TRUE;
            }
            else
            {
              work->egg_shake_wait_count--;
            }
          }
        }
        else if( work->wait_count == particle_burst_frame )
        {
          if( work->egg_shake_se_play )
          {
            if( PMSND_CheckPlaySE_byPlayerID( work->egg_shake_seplayer_id ) )
            {
              PMSND_StopSE_byPlayerID( work->egg_shake_seplayer_id );
              work->egg_shake_se_play = FALSE;
            }
          }
        }
      }
      if( work->wait_count == particle_burst_frame )
      {
        Egg_Demo_View_EggStopWhite( work );
        Egg_Demo_View_ThreeRearStartColorToWhite( work );
        PMSND_PlaySE( SEQ_SE_TDEMO_011 );
        
        // 次へ
        work->step = STEP_EGG_WHITE;
      }
    }
    break;
  case STEP_EGG_WHITE:
    {
      if( Egg_Demo_View_EggIsWhite( work ) )
      {
        //aplhaは身体の部品が見えてしまうのでやめたEgg_Demo_View_McssAlphaAnimeStart( work, 31, 0 );

        // 次へ
        work->step = STEP_EGG_ALPHA;
      }
    }
    break;
  case STEP_EGG_ALPHA:
    {
      if(    1//aplhaは身体の部品が見えてしまうのでやめたEgg_Demo_View_McssAlphaAnimeIsEnd( work ) 
          && Egg_Demo_View_ThreeRearIsWhite( work ) )
      {
        Egg_Demo_View_McssVanish( work );
        work->b_white = TRUE;
        
        // 次へ
        work->step = STEP_EGG_END;
      }
    }
    break;
  case STEP_EGG_END:
    {
      work->b_white = FALSE;
    }
    break;
  case STEP_MON_READY:
    {
    }
    break;
  case STEP_MON_WHITE:
    {
      work->wait_count++;
      if( work->wait_count >= 120 )
      {
        Egg_Demo_View_MonColor( work );
        Egg_Demo_View_ThreeRearStartWhiteToColor( work );

        // 次へ
        work->step = STEP_MON_START;
        work->wait_count = 0;
      }
    }
    break;
  case STEP_MON_START:
    {
      work->wait_count++;
      if( work->wait_count >= 65 )
      {
        // 次へ
        work->step = STEP_MON_CRY;

        {
          u32 monsno  = PP_Get( work->pp, ID_PARA_monsno, NULL );
          u32 form_no = PP_Get( work->pp, ID_PARA_form_no, NULL );
          //work->voicePlayerIdx = PMVOICE_Play( monsno, form_no, 64, FALSE, 0, 0, FALSE, 0 );
          PMV_PlayVoice( monsno, form_no );
        }
      }
    }
    break;
  case STEP_MON_CRY:
    {
      //if( !PMVOICE_CheckPlay( work->voicePlayerIdx ) )
      if( !PMV_CheckPlay() )
      {
        // 次へ
        work->step = STEP_MON_END;
      }
    }
    break;
  case STEP_MON_END:
    {
    }
    break;
  }


#ifdef SET_PARTICLE_Y_MODE
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
  {
    OS_Printf( "particle_y = %f\n", particle_y );
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
  {
    particle_y += 0.1f;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  {
    particle_y -= 0.1f;
  }
#endif


  // MCSS
  Egg_Demo_View_McssAlphaAnimeMain( work );
  MCSS_Main( work->mcss_sys_wk );
  
  // パーティクル
  Particle_Main( work->particle_mgr );

  // 3D
  Egg_Demo_View_ThreeRearMain( work );


#ifdef DEBUG_SET_POKE_ARRANGE
  {
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
    {
      BOOL b_move = FALSE;

      VecFx32 pos;
      f32 pos_x, pos_y;
      const f32 add = 0.1f;

      int trg = GFL_UI_KEY_GetTrg();

      MCSS_GetPosition( work->mcss_wk, &pos );
      pos_x = FX_FX32_TO_F32(pos.x);
      pos_y = FX_FX32_TO_F32(pos.y);

      if( trg & PAD_KEY_LEFT )
      {
        b_move = TRUE;
        pos_x -= add;
      }
      else if( trg & PAD_KEY_RIGHT )
      {
        b_move = TRUE;
        pos_x += add;
      }
      else if( trg & PAD_KEY_UP )
      {
        b_move = TRUE;
        pos_y += add;
      }
      else if( trg & PAD_KEY_DOWN )
      {
        b_move = TRUE;
        pos_y -= add;
      }

      if( b_move )
      {
        pos.x = FX_F32_TO_FX32(pos_x);
        pos.y = FX_F32_TO_FX32(pos_y);
        MCSS_SetPosition( work->mcss_wk, &pos );
        OS_Printf( "(%f, %f)\n", pos_x, pos_y );
      }
    }
  }
#endif
}

//-----------------------------------------------------------------------------
/**
 *  @brief         描画処理 
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし 
 *
 *  @note          GRAPHIC_3D_StartDrawとGRAPHIC_3D_EndDrawの間で呼ぶこと
 */
//-----------------------------------------------------------------------------
void EGG_DEMO_VIEW_Draw( EGG_DEMO_VIEW_WORK* work )
{
  // MCSS
  MCSS_Draw( work->mcss_sys_wk );

  // 3D
  Egg_Demo_View_ThreeDraw( work );

  // パーティクル
  Particle_Draw( work->particle_mgr );

  //メモ
  //Egg_Demo_View_ThreeDraw( work );
  //MCSS_Draw( work->mcss_sys_wk );
  //Particle_Draw( work->particle_mgr );
  //の順に描画したら、
  //ポケモンMCSSが、REAR3Dのスクロールアニメにつられてスクロールしてしまっていた。
  //
  //メモ
  //MCSS_Draw( work->mcss_sys_wk );
  //Particle_Draw( work->particle_mgr );
  //Egg_Demo_View_ThreeDraw( work );
  //の順に描画したら、
  //パーティクルが、REAR3Dの描画より後ろになってしまっていた。
}

//-----------------------------------------------------------------------------
/**
 *  @brief         スタート
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void EGG_DEMO_VIEW_Start( EGG_DEMO_VIEW_WORK* work )
{
  work->b_start = TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         白く飛んでいるか(1フレームしかTRUEにならない)
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        白く飛んだ1フレームにおいてだけTRUEを返す
 */
//-----------------------------------------------------------------------------
BOOL EGG_DEMO_VIEW_White( EGG_DEMO_VIEW_WORK* work )
{
  return work->b_white;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         タマゴをポケモンに差し替える
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし
 */
//-----------------------------------------------------------------------------
void EGG_DEMO_VIEW_Hatch( EGG_DEMO_VIEW_WORK* work, const POKEMON_PARAM* pp )
{
  // タマゴ終了
  Egg_Demo_View_McssExit( work );

  // ポケモン開始
  work->pp = pp;
  Egg_Demo_View_McssInit( work );
  Egg_Demo_View_McssSetAnimeIndex( work, 0, FALSE );

  // 白にする
  MCSS_SetVanishFlag( work->mcss_wk );
  MCSS_SetPaletteFade( work->mcss_wk, 16, 16, 0, 0x7fff );
  
  // 白→普通
  //MCSS_SetPaletteFade( work->mcss_wk, 16, 0, 1, 0x7fff );

  work->step = STEP_MON_READY;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         ポケモンの準備ができたか
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        準備ができているときTRUEを返す
 */
//-----------------------------------------------------------------------------
BOOL EGG_DEMO_VIEW_IsReady( EGG_DEMO_VIEW_WORK* work )
{
  // リソースのロードが完了するまでフェードは始まらないので、それを利用してリソース読み込み完了を待つ
  return !MCSS_CheckExecutePaletteFade( work->mcss_wk );
}

//-----------------------------------------------------------------------------
/**
 *  @brief         ポケモンスタート
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void EGG_DEMO_VIEW_MonStart( EGG_DEMO_VIEW_WORK* work )
{
  work->step = STEP_MON_WHITE;
  work->wait_count = 0;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         終了しているか
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        終了しているときTRUEを返す
 */
//-----------------------------------------------------------------------------
BOOL EGG_DEMO_VIEW_IsEnd( EGG_DEMO_VIEW_WORK* work )
{
  if( work->step >= STEP_MON_END )
    return TRUE;
  return FALSE;
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Egg_Demo_View_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  EGG_DEMO_VIEW_WORK* work = (EGG_DEMO_VIEW_WORK*)wk;
}

//-------------------------------------
/// MCSSシステム初期化処理
//=====================================
static void Egg_Demo_View_McssSysInit( EGG_DEMO_VIEW_WORK* work )
{
  work->mcss_sys_wk = MCSS_Init( 1, work->heap_id );
  MCSS_SetTextureTransAdrs( work->mcss_sys_wk, POKE_TEX_ADRS_FOR_PARTICLE );  // パーティクルと一緒に使うため
  MCSS_SetOrthoMode( work->mcss_sys_wk );

  work->mcss_alpha_anime = FALSE;
  work->mcss_alpha_end   = 0;
}
//-------------------------------------
/// MCSSシステム終了処理
//=====================================
static void Egg_Demo_View_McssSysExit( EGG_DEMO_VIEW_WORK* work )
{
  MCSS_Exit( work->mcss_sys_wk );
}
//-------------------------------------
/// MCSSポケモン初期化処理
//=====================================

// ポケモンの配置
typedef struct
{
  u32 monsno;
  f32 pos_x;
  f32 pos_y;
}
POKE_ARRANGE_INFO;
#define POKE_ARRANGE_INFO_TBL_NUM (4)
static const POKE_ARRANGE_INFO poke_arrange_info_tbl[POKE_ARRANGE_INFO_TBL_NUM] =
{
//  { MONSNO_KENTAROSU,  0.0f, -190.0f },
  { MONSNO_HINOARASI,  -0.5f, -190.0f },
//  { MONSNO_MARIRU,     0.0f, -190.0f },
  { MONSNO_MAGUMAGGU,  -0.5f, -189.9f },
//  { MONSNO_KINOKOKO,   0.0f, -190.0f },
  { MONSNO_SOONANO,    0.0f, -189.8f },
//  { MONSNO_HUWANTE,    0.0f, -190.0f },
//  { MONSNO_521,        0.0f, -190.0f },  // モンメン
  { MONSNO_630,        0.0f, -190.2f },  // コジョフー
};

static void Egg_Demo_View_McssInit( EGG_DEMO_VIEW_WORK* work )
{
  {
    work->mcss_wk = MCSS_TOOL_AddPokeMcss( work->mcss_sys_wk, work->pp, MCSS_DIR_FRONT,
                        0, POKE_Y, FX_F32_TO_FX32(-800.0f) );//-800.0f) );
    MCSS_SetShadowVanishFlag( work->mcss_wk, TRUE );  // 影を消しておかないと、小さな点として影が表示されてしまう。 

#ifdef DEBUG_KAWADA
    {
      OS_Printf( "EGG_DEMO_VIEW : MCellAnmNum=%d, StopCellAnms=%d\n",
          MCSS_GetMCellAnmNum(work->mcss_wk),
          MCSS_GetStopCellAnms(work->mcss_wk) );
      if( MCSS_GetStopCellAnms(work->mcss_wk) > 0 )
          OS_Printf( "EGG_DEMO_VIEW : StopNode=%d\n", MCSS_GetStopNode(work->mcss_wk, 0) );
    }
#endif
  }

  // ポケモンの配置
  {
    u8  i;
    u32 monsno = PP_Get( work->pp, ID_PARA_monsno_egg, NULL );
    if( monsno != MONSNO_TAMAGO )
    {
      for( i=0; i<POKE_ARRANGE_INFO_TBL_NUM; i++ )
      {
        if( monsno == poke_arrange_info_tbl[i].monsno )
        {
          VecFx32 pos;
          pos.x = FX_F32_TO_FX32(poke_arrange_info_tbl[i].pos_x);
          pos.y = FX_F32_TO_FX32(poke_arrange_info_tbl[i].pos_y);
          pos.z = FX_F32_TO_FX32(-800.0f);
          MCSS_SetPosition( work->mcss_wk, &pos );
          break;
        }
      }
    }
  }

  {
    VecFx32 scale;
    scale.x = FX_F32_TO_FX32( POKE_SCALE );
    scale.y = FX_F32_TO_FX32( POKE_SCALE );
    scale.z = FX32_ONE;
    MCSS_SetScale( work->mcss_wk, &scale );
  }

  {
#if 0
    f32 size_y = (f32)MCSS_GetSizeY( work->mcss_wk );
    f32 ofs_y;
    VecFx32 ofs;
    if( size_y > POKE_SIZE_MAX ) size_y = POKE_SIZE_MAX;
    ofs_y = ( POKE_SIZE_MAX - size_y ) / 2.0f;
    ofs.x = 0;  ofs.y = FX_F32_TO_FX32(ofs_y);  ofs.z = 0;
    MCSS_SetOfsPosition( work->mcss_wk, &ofs );
#else

    f32      size_y   = (f32)MCSS_GetSizeY( work->mcss_wk );
    f32      offset_y = (f32)MCSS_GetOffsetY( work->mcss_wk );  // 浮いているとき-, 沈んでいるとき+
    f32      offset_x = (f32)MCSS_GetOffsetX( work->mcss_wk );  // 右にずれているとき+, 左にずれているとき-
    f32      ofs_position_y;
    f32      ofs_position_x;
    VecFx32  ofs_position;

    if( size_y > POKE_SIZE_MAX ) size_y = POKE_SIZE_MAX;

    ofs_position_y = ( POKE_SIZE_MAX - size_y ) / 2.0f + offset_y;
    ofs_position_x = - offset_x;
      
    ofs_position.x = FX_F32_TO_FX32(ofs_position_x);  ofs_position.y = FX_F32_TO_FX32(ofs_position_y);  ofs_position.z = 0;
    MCSS_SetOfsPosition( work->mcss_wk, &ofs_position );

#endif
  }

  MCSS_SetAnmStopFlag( work->mcss_wk );  // 生成時にこれを呼んでおかないと、何故か0番のアニメが再生されてしまう。
}
//-------------------------------------
/// MCSSポケモン終了処理
//=====================================
static void Egg_Demo_View_McssExit( EGG_DEMO_VIEW_WORK* work )
{
  MCSS_SetVanishFlag( work->mcss_wk );
  MCSS_Del( work->mcss_sys_wk, work->mcss_wk );
}
//-------------------------------------
/// MCSSポケモン消去
//=====================================
static void Egg_Demo_View_McssVanish( EGG_DEMO_VIEW_WORK* work )
{
  MCSS_SetVanishFlag( work->mcss_wk );
}
//-------------------------------------
/// MCSSポケモンアニメーション設定
//=====================================
static void Egg_Demo_View_McssSetAnimeIndex( EGG_DEMO_VIEW_WORK* work, int index, BOOL b_last_stop )
{
  NNSG2dMultiCellAnimation* mcss_anim_ctrl;

  MCSS_SetAnimeIndex( work->mcss_wk, index );
  mcss_anim_ctrl = MCSS_GetAnimCtrl( work->mcss_wk );
  NNS_G2dRestartMCAnimation( mcss_anim_ctrl );
  MCSS_ResetAnmStopFlag( work->mcss_wk );

  if( b_last_stop )  // 最終フレームでコールバックを呼ぶ場合
  {
    NNS_G2dSetAnimCtrlCallBackFunctor(
        NNS_G2dGetMCAnimAnimCtrl(mcss_anim_ctrl),
        NNS_G2D_ANMCALLBACKTYPE_LAST_FRM,
        (u32)work,
        Egg_Demo_View_McssCallBackFunctor );
    work->mcss_anime_end = FALSE;
  }
  else
  {
    MCSS_TOOL_SetAnmRestartCallback( work->mcss_wk );  // 1ループしたらアニメーションリセットを呼ぶためのコールバックセット
  }
}
//-------------------------------------
/// MCSSポケモン最終フレームで呼ばれるコールバック
//=====================================
static void Egg_Demo_View_McssCallBackFunctor( u32 data, fx32 currentFrame )
{
  EGG_DEMO_VIEW_WORK* work = (EGG_DEMO_VIEW_WORK*)data;

  MCSS_SetAnmStopFlag( work->mcss_wk );  // このタイミングだと0フレームに戻ったところで止まるので、1フレーム遅くて使い物にならない
  work->mcss_anime_end = TRUE;
}
//-------------------------------------
/// MCSSポケモンアルファアニメーション
//=====================================
static void Egg_Demo_View_McssAlphaAnimeStart( EGG_DEMO_VIEW_WORK* work, u8 start_alpha, u8 end_alpha )  // 0-31
{
  u8 curr_alpha = MCSS_GetAlpha( work->mcss_wk );
  work->mcss_alpha_end = end_alpha;
  if( start_alpha == end_alpha )
  {
    work->mcss_alpha_anime = FALSE;
    MCSS_SetAlpha( work->mcss_wk, end_alpha );
  }
  else
  {
    work->mcss_alpha_anime = TRUE;
    MCSS_SetAlpha( work->mcss_wk, start_alpha );
  }
}
static void Egg_Demo_View_McssAlphaAnimeMain( EGG_DEMO_VIEW_WORK* work )
{
  if( work->mcss_alpha_anime )
  {
    u8 curr_alpha = MCSS_GetAlpha( work->mcss_wk );
    if( curr_alpha > work->mcss_alpha_end )
    {
      curr_alpha--;
    }
    else  // if( curr_alpha < work->mcss_alpha_end )  // ( curr_alpha == end_alpha )になることはない
    {
      curr_alpha++;
    }
    MCSS_SetAlpha( work->mcss_wk, curr_alpha );
    if( curr_alpha == work->mcss_alpha_end )
    {
      work->mcss_alpha_anime = FALSE;
    }
  }
}
static BOOL Egg_Demo_View_McssAlphaAnimeIsEnd( EGG_DEMO_VIEW_WORK* work )
{
  return !(work->mcss_alpha_anime);
}

//-------------------------------------
/// タマゴのアニメを止め、白くする
//=====================================
static void Egg_Demo_View_EggStopWhite( EGG_DEMO_VIEW_WORK* work )
{
  // アニメを止める
  MCSS_SetAnmStopFlag( work->mcss_wk );
  // 普通→白
  MCSS_SetPaletteFade( work->mcss_wk, 0, 16, 0, 0x7fff );
}
//-------------------------------------
/// タマゴが白くなっていたらtrue
//=====================================
static BOOL Egg_Demo_View_EggIsWhite( EGG_DEMO_VIEW_WORK* work )
{
  return !MCSS_CheckExecutePaletteFade( work->mcss_wk );
}

//-------------------------------------
/// ポケモンを表示し、透明、白から普通の色にする、
//=====================================
static void Egg_Demo_View_MonColor( EGG_DEMO_VIEW_WORK* work )
{
  // 透明→普通
  //aplhaは身体の部品が見えてしまうのでやめたEgg_Demo_View_McssAlphaAnimeStart( work, 0, 31 );
  // 表示
  MCSS_ResetVanishFlag( work->mcss_wk );
  // 白→普通
  MCSS_SetPaletteFade( work->mcss_wk, 16, 0, 2, 0x7fff );
}

//-------------------------------------
/// パーティクル
//=====================================
static PARTICLE_MANAGER* Particle_Init( HEAPID heap_id, u16 data_num, const PARTICLE_PLAY_DATA* data_tbl )
{
  PARTICLE_MANAGER* mgr;
  u8 i;

  // パーティクル専用カメラ	
  VecFx32 cam_eye = {            0,         0,   FX32_CONST(70) };
  VecFx32 cam_up  = {            0,  FX32_ONE,                0 };
  VecFx32 cam_at  = {            0,         0,                0 };
  
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

    mgr->spa_set[i].ptc = GFL_PTC_Create( mgr->spa_set[i].wk, sizeof(mgr->spa_set[i].wk), TRUE, heap_id );
		//GFL_PTC_PersonalCameraDelete( mgr->spa_set[i].ptc );
		//GFL_PTC_PersonalCameraCreate( mgr->spa_set[i].ptc, &proj, DEFAULT_PERSP_WAY, &cam_eye, &cam_up, &cam_at, heap_id );
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

static void Particle_Main( PARTICLE_MANAGER* mgr )
{
  if( mgr->play )
  {
    VecFx32 pos = { 0, PARTICLE_Y, 0 };//FX_F32_TO_FX32(0.5f), 0 };
    GFL_EMIT_PTR emit;


#ifdef SET_PARTICLE_Y_MODE
    pos.y = FX_F32_TO_FX32(particle_y);
#endif


    while( mgr->data_no < mgr->data_num )
    {
      if( mgr->frame == mgr->data_tbl[mgr->data_no].frame )
      {
        emit = GFL_PTC_CreateEmitter( mgr->spa_set[ mgr->data_tbl[mgr->data_no].spa_idx ].ptc, mgr->data_tbl[mgr->data_no].res_no, &pos );

#ifdef DEBUG_KAWADA
        {
          OS_Printf( "EGG_DEMO_VIEW : frame=%d, jspa_idx=%d, res_no=%d, emit=%p\n",
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

//-------------------------------------
/// 3D
//=====================================
// 3D全体
static void Egg_Demo_View_ThreeInit( EGG_DEMO_VIEW_WORK* work )
{
  // レンダリングスワップバッファ
  GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );  // マニュアルソートにした(EGG_DEMO_GRAPHICではオートソートに設定していたが)

  // 3D管理ユーティリティーのセットアップ
  work->three_util = GFL_G3D_UTIL_Create( THREE_RES_MAX, THREE_OBJ_MAX, work->heap_id );

  // NULL、ゼロ初期化
  work->three_obj_prop_num = 0;
}
static void Egg_Demo_View_ThreeExit( EGG_DEMO_VIEW_WORK* work )
{
  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( work->three_util );
}
static void Egg_Demo_View_ThreeDraw( EGG_DEMO_VIEW_WORK* work )  // 3D描画(GRAPHIC_3D_StartDrawとPSEL_GRAPHIC_3D_EndDrawの間で呼ぶ)
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
static void Egg_Demo_View_ThreeRearInit( EGG_DEMO_VIEW_WORK* work )
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
        VEC_Set( &(prop->objstatus.trans), 0, FX_F32_TO_FX32(0.0f), 0 );
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
static void Egg_Demo_View_ThreeRearExit( EGG_DEMO_VIEW_WORK* work )
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
static void Egg_Demo_View_ThreeRearMain( EGG_DEMO_VIEW_WORK* work )
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
	  //GFL_G3D_OBJECT_SetAnimeFrame( obj, REAR_ANM_C, &work->rear_white_anime_frame );  // GFL_G3D_OBJECT_SetAnimeFrameだと絵が更新されなかったので、
	  //GFL_G3D_OBJECT_SetAnimeFrame( obj, REAR_ANM_M, &work->rear_white_anime_frame );  // GFL_G3D_OBJECT_IncAnimeFrameにした。
  }
}
static void Egg_Demo_View_ThreeRearStartColorToWhite( EGG_DEMO_VIEW_WORK* work )
{
  work->rear_white_anime_state = REAR_WHITE_ANIME_STATE_COLOR_TO_WHITE;
}
static BOOL Egg_Demo_View_ThreeRearIsWhite( EGG_DEMO_VIEW_WORK* work )
{
  return ( work->rear_white_anime_state == REAR_WHITE_ANIME_STATE_WHITE );
}
static void Egg_Demo_View_ThreeRearStartWhiteToColor( EGG_DEMO_VIEW_WORK* work )
{
  work->rear_white_anime_state = REAR_WHITE_ANIME_STATE_WHITE_TO_COLOR;
}
static BOOL Egg_Demo_View_ThreeRearIsColor( EGG_DEMO_VIEW_WORK* work )
{
  return ( work->rear_white_anime_state == REAR_WHITE_ANIME_STATE_COLOR );
}

