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
#define DEBUG_KAWADA


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/mcss.h"
#include "system/mcss_tool.h"

#include "egg_demo_view.h"


// アーカイブ
#include "arc_def.h"
#include "../../../../resource/egg_demo/egg_demo_setup.h"
#include "egg_demo_particle.naix"
// サウンド
#include "sound/pm_voice.h"


//=============================================================================
/**
 *  定数定義
 */
//=============================================================================
// BGフレーム
#define BG_FRAME_M_POKEMON        (GFL_BG_FRAME0_M)            // プライオリティ1

// ステップ
typedef enum
{
  STEP_EGG_START,       // タマゴ開始待ち中→デモ
  STEP_EGG_DEMO,        // タマゴデモ
  STEP_EGG_END,         // タマゴ終了中
  STEP_MON_READY,       // ポケモン準備中
  STEP_MON_START,       // ポケモン開始待ち中→鳴く
  STEP_MON_CRY,         // ポケモン鳴き終わるのを待ち中→終了する
  STEP_MON_END,         // ポケモン終了中
}
STEP;

// パーティクル対応
#define POKE_TEX_ADRS_FOR_PARTICLE (0x30000)

// ポケモン
#define POKE_SCALE       (16.0f)
#define POKE_SIZE_MAX    (96.0f)

// パーティクルとポケモンを連携させる
#define PARTICLE_BURST_FRAME   (300)
#define PARTICLE_LAST_FRAME    (310)


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
  PARTICLE_SPA_FILE_0,      // ARCID_EGG_DEMO    // NARC___particle_egg_demo_particle_egg_demo_normal_spa
  PARTICLE_SPA_FILE_1,      // ARCID_EGG_DEMO    // NARC___particle_egg_demo_particle_egg_demo_special_spa
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
  { ARCID_EGG_DEMO, NARC___particle_egg_demo_particle_egg_demo_normal_spa  },
  { ARCID_EGG_DEMO, NARC___particle_egg_demo_particle_egg_demo_special_spa },
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
#if 0
static const PARTICLE_PLAY_DATA particle_play_data_tbl[] =
{
  {    0,     PARTICLE_SPA_FILE_0,        0 },  // タマゴの小さな殻が少し飛ぶ
  {   80,     PARTICLE_SPA_FILE_0,        1 },  // タマゴの小さな殻が大きく飛ぶ
  {  160,     PARTICLE_SPA_FILE_0,        2 },  // タマゴの大きな殻が放射状に激しく飛ぶ
  {  240,     PARTICLE_SPA_FILE_0,        3 },  // 黄色の星キラキラ
  {  320,     PARTICLE_SPA_FILE_1,        0 },  // 白紫の大きな円
  {  400,     PARTICLE_SPA_FILE_1,        1 },  // 小さな白玉がたくさん放射状に出る
  {  480,     PARTICLE_SPA_FILE_1,        2 },  // 白輪が広がる
  {  560,     PARTICLE_SPA_FILE_1,        3 },  // 奇妙な色のタマゴ
};
#else
static const PARTICLE_PLAY_DATA particle_play_data_tbl[] =
{
  {                     120,     PARTICLE_SPA_FILE_0,        0 },  // タマゴの小さな殻が少し飛ぶ
  {                     210,     PARTICLE_SPA_FILE_0,        1 },  // タマゴの小さな殻が大きく飛ぶ
  {    PARTICLE_BURST_FRAME,     PARTICLE_SPA_FILE_0,        2 },  // タマゴの大きな殻が放射状に激しく飛ぶ
  {  PARTICLE_LAST_FRAME -5,     PARTICLE_SPA_FILE_0,        3 },  // 黄色の星キラキラ
  {     PARTICLE_LAST_FRAME,     PARTICLE_SPA_FILE_0,        3 },  // 黄色の星キラキラ
};
#endif


//-------------------------------------
/// ワーク
//=====================================
struct _EGG_DEMO_VIEW_WORK
{
  // 他のところのを覚えているだけで生成や破棄はしない。
  HEAPID                   heap_id;
  const POKEMON_PARAM*     pp;
  
  // ステップ
  STEP                     step;

  // フラグなど
  BOOL                     b_start;
  BOOL                     b_white;
  u32                      voicePlayerIdx;
  u32                      wait_count;

  // MCSS
  MCSS_SYS_WORK*           mcss_sys_wk;
  MCSS_WORK*               mcss_wk;

  // パーティクル
  PARTICLE_MANAGER*        particle_mgr;
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// MCSS
//=====================================
static void Egg_Demo_View_McssSysInit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_McssSysExit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_McssInit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_McssExit( EGG_DEMO_VIEW_WORK* work );
static void Egg_Demo_View_McssVanish( EGG_DEMO_VIEW_WORK* work );
 
//-------------------------------------
/// パーティクル
//=====================================
static PARTICLE_MANAGER* Particle_Init( HEAPID heap_id );
static void Particle_Exit( PARTICLE_MANAGER* mgr );
static void Particle_Main( PARTICLE_MANAGER* mgr );
static void Particle_Draw( PARTICLE_MANAGER* mgr );
static void Particle_Start( PARTICLE_MANAGER* mgr );
static void Particle_Stop( PARTICLE_MANAGER* mgr, s32 stop_count );


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
  }

  // ステップ
  {
    work->step         = STEP_EGG_START;
  }

  // フラグなど
  {
    work->b_start          = FALSE;
    work->b_white          = FALSE;
    work->voicePlayerIdx   = 0;
    work->wait_count       = 0;
  }
  
  // MCSS
  {
    Egg_Demo_View_McssSysInit( work );
    Egg_Demo_View_McssInit( work );
  }

  // パーティクル
  work->particle_mgr = Particle_Init( work->heap_id );

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
  // パーティクル
  Particle_Exit( work->particle_mgr );

  // MCSS
  {
    Egg_Demo_View_McssExit( work );
    Egg_Demo_View_McssSysExit( work );
  }

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

        // パーティクル
        Particle_Start( work->particle_mgr );
      }
    }
    break;
  case STEP_EGG_DEMO:
    {
      work->wait_count++;
      if( work->wait_count == PARTICLE_BURST_FRAME +1 )
        Egg_Demo_View_McssVanish( work );
      if( work->wait_count == PARTICLE_LAST_FRAME +20 )
      {
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
  case STEP_MON_START:
    {
      work->wait_count++;
      if( work->wait_count >= 60 )
      {
        // 次へ
        work->step = STEP_MON_CRY;

        {
          u32 monsno  = PP_Get( work->pp, ID_PARA_monsno, NULL );
          u32 form_no = PP_Get( work->pp, ID_PARA_form_no, NULL );
          work->voicePlayerIdx = PMVOICE_Play( monsno, form_no, 64, FALSE, 0, 0, FALSE, 0 );
        }
      }
    }
    break;
  case STEP_MON_CRY:
    {
      if( !PMVOICE_CheckPlay( work->voicePlayerIdx ) )
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

  // MCSS
  MCSS_Main( work->mcss_sys_wk );

  // パーティクル
  Particle_Main( work->particle_mgr );
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

  // パーティクル
  Particle_Draw( work->particle_mgr );
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
 *  @brief         白く飛ばす演出のためのパレットフェードをして欲しいか(1フレームしかTRUEにならない)
 *
 *  @param[in,out] work  EGG_DEMO_VIEW_Initで生成したワーク
 *
 *  @retval        パレットフェードをして欲しい1フレームにおいてだけTRUEを返す
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

  // 白にする
  MCSS_SetVanishFlag( work->mcss_wk );
  MCSS_SetPaletteFade( work->mcss_wk, 16, 16, 0, 0x7fff );
  
  // 白→普通
  //MCSS_SetPaletteFade( work->mcss_wk, 16, 0, 1, 0x7fff );

  work->step = STEP_MON_READY;
  
  work->wait_count = 0;
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
  return MCSS_CheckExecutePaletteFade( work->mcss_wk );
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
  // 白→普通
  MCSS_ResetVanishFlag( work->mcss_wk );
  MCSS_SetPaletteFade( work->mcss_wk, 16, 0, 1, 0x7fff );

  work->step = STEP_MON_START;
  
  work->wait_count = 0;

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
/// MCSSシステム初期化処理
//=====================================
static void Egg_Demo_View_McssSysInit( EGG_DEMO_VIEW_WORK* work )
{
  work->mcss_sys_wk = MCSS_Init( 1, work->heap_id );
  MCSS_SetTextureTransAdrs( work->mcss_sys_wk, POKE_TEX_ADRS_FOR_PARTICLE );  // パーティクルと一緒に使うため
  MCSS_SetOrthoMode( work->mcss_sys_wk );
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
static void Egg_Demo_View_McssInit( EGG_DEMO_VIEW_WORK* work )
{
  {
    MCSS_ADD_WORK add_wk;
    MCSS_TOOL_MakeMAWPP( work->pp, &add_wk, MCSS_DIR_FRONT );
    work->mcss_wk = MCSS_Add( work->mcss_sys_wk, 0, 0, 0, &add_wk );

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

  {
    VecFx32 scale;
    scale.x = FX_F32_TO_FX32( POKE_SCALE );
    scale.y = FX_F32_TO_FX32( POKE_SCALE );
    scale.z = FX32_ONE;
    MCSS_SetScale( work->mcss_wk, &scale );
  }

  {
    f32 size_y = (f32)MCSS_GetSizeY( work->mcss_wk );
    f32 ofs_y;
    VecFx32 ofs;
    if( size_y > POKE_SIZE_MAX ) size_y = POKE_SIZE_MAX;
    ofs_y = ( POKE_SIZE_MAX - size_y ) / 2.0f;
    ofs.x = 0;  ofs.y = FX_F32_TO_FX32(ofs_y);  ofs.z = 0;
    MCSS_SetOfsPosition( work->mcss_wk, &ofs );
  }
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
/// パーティクル
//=====================================
static PARTICLE_MANAGER* Particle_Init( HEAPID heap_id )
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
    mgr->data_num        = NELEMS(particle_play_data_tbl);
    mgr->data_tbl        = particle_play_data_tbl;
    mgr->play            = FALSE;
    mgr->stop_count      = -1;
  }

  GFL_PTC_Init( heap_id );

  for(i=0; i<PARTICLE_SPA_FILE_MAX; i++)
  {
    void* arc_res;

    mgr->spa_set[i].ptc = GFL_PTC_Create( mgr->spa_set[i].wk, sizeof(mgr->spa_set[i].wk), TRUE, heap_id );
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

static void Particle_Main( PARTICLE_MANAGER* mgr )
{
  if( mgr->play )
  {
    VecFx32 pos = { 0, FX_F32_TO_FX32(0.5f), 0 };
    GFL_EMIT_PTR emit;

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

