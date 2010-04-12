//============================================================================
/**
 *  @file   shinka_demo_view.c
 *  @brief  進化デモの演出
 *  @author Koji Kawada
 *  @data   2010.01.19
 *  @note   
 *
 *  モジュール名：SHINKADEMO_VIEW
 */
//============================================================================
//#define DEBUG_CODE


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/mcss.h"
#include "system/mcss_tool.h"

#include "sound/pm_wb_voice.h"  // wbではpm_voiceではなくこちらを使う
#include "sound/wb_sound_data.sadl"
#include "sound/pm_sndsys.h"

#include "shinka_demo_view.h"

// パーティクル
#include "arc_def.h"
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
  STEP_EVO_CRY_START,                // 鳴き開始待ち中→鳴く
  STEP_EVO_CRY,                      // 鳴き中
  STEP_EVO_CHANGE_SATRT,             // 進化開始待ち中→進化
  STEP_EVO_CHANGE_TO_WHITE_START,    // 進化中  // 白くする
  STEP_EVO_CHANGE_TO_WHITE,          // 進化中  // 白くなり中
  STEP_EVO_CHANGE_TRANSFORM,         // 進化中  // 変形
  STEP_EVO_CHANGE_CANCEL,            // 進化中  // キャンセル
  STEP_EVO_CHANGE_FROM_WHITE_START,  // 進化中  // 色付きに戻る
  STEP_EVO_CHANGE_FROM_WHITE,        // 進化中  // 色付きに戻り中
  STEP_EVO_CHANGE_CRY_START,         // 進化中  // 鳴き開始待ち中
  STEP_EVO_CHANGE_CRY,               // 進化中  // 鳴き中
  STEP_EVO_END,                      // 終了中

  // AFTER
  STEP_AFTER,                       // 進化後からスタート
}
STEP;

// MCSSポケモン
typedef enum
{
  POKE_BEFORE,
  POKE_AFTER,
  POKE_MAX,
}
POKE;

// 拡大縮小入れ替え演出
#define ANM_ADD_START        (0x400)
#define ANM_VAL_MAX          (0x10000)

  static const int anm_add_tbl[] =  // ANM_VAL_MAX/2 より小さな値で
  {                    // disp_poke
    ANM_ADD_START,     // BEFORE
    0x1000,
    0x1000,            // AFTER
    0x400,
    0x400,             // BEFORE
    0x1000,
    0x1000,            // AFTER
    0x400,
    0x400,             // BEFORE
    0x1000,
    0x1000,            // AFTER
    0x400,

    0x800,             // BEFORE
    0x1000,
    0x1000,            // AFTER
    0x800,
    0x800,             // BEFORE
    0x1000,
    0x1000,            // AFTER
    0x800,

    0x1000,            // BEFORE
    0x1000,
    0x1000,            // AFTER
    0x1000,
    0x1000,            // BEFORE
    0x1000,
    0x1000,            // AFTER
    0x1000,
    0x1000,            // BEFORE
    0x1000,
    0x1000,            // AFTER
    0x1000,
 
    0x1000,            // BEFORE
    0x800,
    0x800,             // AFTER
    0x1000,

    0x1000,            // BEFORE  // ANM_REPEAT_MAX-6
    0x400,
    0x400,             // AFTER   // ANM_REPEAT_MAX-4
    0x1000,
    0x1000,            // BEFORE  // ANM_REPEAT_MAX-2
    0x400,
  };
#define ANM_REPEAT_MAX       ( sizeof(anm_add_tbl) / sizeof(anm_add_tbl[0]) )

#define SCALE_MAX            (16.0f)
#define SCALE_MIN            ( 4.0f)
#define POKE_SIZE_MAX        (96.0f)

// パーティクル対応
#define POKE_TEX_ADRS_FOR_PARTICLE (0x30000)


// ポケモンのY座標
#define POKE_Y  (FX_F32_TO_FX32(-24.0f))


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// MCSSポケモン
//=====================================
typedef struct
{
  MCSS_WORK*           wk;
}
POKE_SET;

//-------------------------------------
/// ワーク
//=====================================
struct _SHINKADEMO_VIEW_WORK
{
  // 他のところのを覚えているだけで生成や破棄はしない。
  HEAPID                   heap_id;
  const POKEMON_PARAM*     pp;
  
  // SHINKADEMO_VIEW_Initの引数
  SHINKADEMO_VIEW_LAUNCH   launch;
  u16                      after_monsno;

  // ポケモン 
  POKEMON_PARAM*           after_pp;  // ここで一時的に生成する
  
  // ステップ
  STEP                     step;
  u32                      wait_count;

  // フラグ
  BOOL                     b_cry_start;      // 進化前の鳴きスタートしていいときはTRUE
  BOOL                     b_cry_end;        // 進化前の鳴きが完了していたらTRUE
  BOOL                     b_change_start;   // 進化スタートしていいときはTRUE
  BOOL                     b_change_end;     // 進化が完了していたらTRUE
  BOOL                     b_change_bgm_shinka_start;    // BGM SHINKA曲を開始してもよいか
  BOOL                     b_change_bgm_shinka_push;     // BGM SHINKA曲をpushしてもよいか
  BOOL                     b_change_cancel;  // 進化キャンセルしていたらTRUE

  // 拡大縮小入れ替え演出
  f32                      scale;
  u8                       disp_poke;
  int                      anm_val;
  int                      anm_add;
  u8                       count;

  // MCSS
  MCSS_SYS_WORK*           mcss_sys_wk;
  POKE_SET                 poke_set[POKE_MAX];
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
static void ShinkaDemo_View_McssInit( SHINKADEMO_VIEW_WORK* work );
static void ShinkaDemo_View_McssExit( SHINKADEMO_VIEW_WORK* work );

static void ShinkaDemo_View_PokeInit( SHINKADEMO_VIEW_WORK* work );
static void ShinkaDemo_View_PokeExit( SHINKADEMO_VIEW_WORK* work );

static u8 NotDispPoke( u8 disp_poke );

static BOOL ShinkaDemo_View_Evolve( SHINKADEMO_VIEW_WORK* work );
static void ShinkaDemo_View_AdjustPokePos( SHINKADEMO_VIEW_WORK* work );


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
 *  @retval        SHINKADEMO_VIEW_WORK
 */
//-----------------------------------------------------------------------------
SHINKADEMO_VIEW_WORK* SHINKADEMO_VIEW_Init(
                               HEAPID                   heap_id,
                               SHINKADEMO_VIEW_LAUNCH   launch,
                               const POKEMON_PARAM*     pp,
                               u16                      after_monsno
                             )
{
  SHINKADEMO_VIEW_WORK* work;

  // ワーク
  {
    work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(SHINKADEMO_VIEW_WORK) );
  }

  // 引数
  {
    work->heap_id      = heap_id;
    work->launch       = launch;
    work->pp           = pp;
    work->after_monsno = after_monsno;
  }
 
  // ポケモン
  {
    // ここで一時的に生成する
    work->after_pp = PP_Create( 1, 1, 0, work->heap_id );
    POKETOOL_CopyPPtoPP( (POKEMON_PARAM*)(work->pp), work->after_pp );
    PP_ChangeMonsNo( work->after_pp, work->after_monsno );  // 進化
  }
  
  // ステップ
  {
    if( work->launch == SHINKADEMO_VIEW_LAUNCH_EVO )
      work->step         = STEP_EVO_CRY_START;
    else
      work->step         = STEP_AFTER;
    
    work->wait_count = 0;
  }

  // フラグ
  {
    work->b_cry_start                = FALSE;
    work->b_cry_end                  = FALSE; 
    work->b_change_start             = FALSE;
    work->b_change_end               = FALSE;
    work->b_change_bgm_shinka_start  = FALSE;
    work->b_change_bgm_shinka_push   = FALSE;
    work->b_change_cancel            = FALSE;
  }
  
  // 拡大縮小入れ替え演出
  {
    work->scale        = SCALE_MAX;
    
    if( work->launch == SHINKADEMO_VIEW_LAUNCH_EVO )
      work->disp_poke    = POKE_BEFORE;
    else
      work->disp_poke    = POKE_AFTER;

    work->anm_val    = 0;
    work->anm_add    = ANM_ADD_START;
    work->count      = 0;
  }

  // MCSS
  {
    ShinkaDemo_View_McssInit( work );
    ShinkaDemo_View_PokeInit( work );
  }

  return work;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         終了処理 
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_VIEW_Exit( SHINKADEMO_VIEW_WORK* work )
{
  // MCSS
  {
    ShinkaDemo_View_PokeExit( work );
    ShinkaDemo_View_McssExit( work );
  }

  // ここで一時的に生成したものを破棄する
  {
    GFL_HEAP_FreeMemory( work->after_pp );
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
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_VIEW_Main( SHINKADEMO_VIEW_WORK* work )
{
  switch(work->step)
  {
  // EVO
  case STEP_EVO_CRY_START:
    {
      if( work->b_cry_start )
      {
        // 次へ
        work->step = STEP_EVO_CRY;

        {
          u32 monsno  = PP_Get( work->pp, ID_PARA_monsno, NULL );
          u32 form_no = PP_Get( work->pp, ID_PARA_form_no, NULL );
          PMV_PlayVoice( monsno, form_no );
        }
      }
    }
    break;
  case STEP_EVO_CRY:
    {
      if( !PMV_CheckPlay() )
      {
        // 次へ
        work->step = STEP_EVO_CHANGE_SATRT;
        
        work->b_cry_end = TRUE;
      }
    }
    break;
  case STEP_EVO_CHANGE_SATRT:
    {
      if( work->b_change_start )
      {
        // 次へ
        work->step = STEP_EVO_CHANGE_TO_WHITE_START;

        MCSS_SetPaletteFade( work->poke_set[work->disp_poke].wk, 0, 16, 3, 0x7fff );
        MCSS_SetPaletteFade( work->poke_set[NotDispPoke(work->disp_poke)].wk, 16, 16, 0, 0x7fff );
      }
    }
    break;
  case STEP_EVO_CHANGE_TO_WHITE_START:
    {
      // 次へ
      work->step = STEP_EVO_CHANGE_TO_WHITE;

      MCSS_SetPaletteFade( work->poke_set[work->disp_poke].wk, 0, 16, 3, 0x7fff );
      MCSS_SetPaletteFade( work->poke_set[NotDispPoke(work->disp_poke)].wk, 16, 16, 0, 0x7fff );
    }
    break;
  case STEP_EVO_CHANGE_TO_WHITE:
    {
      if( !MCSS_CheckExecutePaletteFade( work->poke_set[work->disp_poke].wk ) )
      {
        // 次へ
        work->step = STEP_EVO_CHANGE_TRANSFORM;

        work->b_change_bgm_shinka_start = TRUE;
      }
    }
    break;
  case STEP_EVO_CHANGE_TRANSFORM:
    {
      if( ShinkaDemo_View_Evolve( work ) )
      {
        // 次へ
        work->step = STEP_EVO_CHANGE_FROM_WHITE_START;

        work->b_change_bgm_shinka_push = TRUE;
      } 
    }
    break;
  case STEP_EVO_CHANGE_CANCEL:
    {
      if( ShinkaDemo_View_Evolve( work ) )
      {
        // 次へ
        work->step = STEP_EVO_CHANGE_FROM_WHITE_START;

        work->b_change_bgm_shinka_push = TRUE;
      }
    }
    break;
  case STEP_EVO_CHANGE_FROM_WHITE_START:
    {
      // 次へ
      work->step = STEP_EVO_CHANGE_FROM_WHITE;

      MCSS_SetPaletteFade( work->poke_set[work->disp_poke].wk, 16, 0, 1, 0x7fff );
    }
    break;
  case STEP_EVO_CHANGE_FROM_WHITE:
    {
      if( !MCSS_CheckExecutePaletteFade( work->poke_set[work->disp_poke].wk ) )
      {
        // 次へ
        work->step = STEP_EVO_CHANGE_CRY_START;
      }
    }
    break;
  case STEP_EVO_CHANGE_CRY_START:
    {
      {
        // 次へ
        work->step = STEP_EVO_CHANGE_CRY;

        {
          const POKEMON_PARAM* curr_pp = (work->b_change_cancel)?(work->pp):(work->after_pp);
          u32 monsno  = PP_Get( curr_pp, ID_PARA_monsno, NULL );
          u32 form_no = PP_Get( curr_pp, ID_PARA_form_no, NULL );
          PMV_PlayVoice( monsno, form_no );
        }
      }
    }
    break;
  case STEP_EVO_CHANGE_CRY:
    {
      if( !PMV_CheckPlay() )
      {
        // 次へ
        work->step = STEP_EVO_END;

        work->b_change_end = TRUE;
      }
    }
    break;
  case STEP_EVO_END:
    {
      // 何もしない
    }
    break;

  // AFTER
  case STEP_AFTER:
    {
      // 何もしない
    }
    break;
  }

  MCSS_Main( work->mcss_sys_wk );
}

//-----------------------------------------------------------------------------
/**
 *  @brief         描画処理 
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし 
 *
 *  @note          GRAPHIC_3D_StartDrawとGRAPHIC_3D_EndDrawの間で呼ぶこと
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_VIEW_Draw( SHINKADEMO_VIEW_WORK* work )
{
  MCSS_Draw( work->mcss_sys_wk );
}

//-----------------------------------------------------------------------------
/**
 *  @brief         進化前の鳴きスタート
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_VIEW_CryStart( SHINKADEMO_VIEW_WORK* work )
{
  work->b_cry_start = TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         進化前の鳴きが完了しているか
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        完了しているときTRUEを返す
 */
//-----------------------------------------------------------------------------
BOOL SHINKADEMO_VIEW_CryIsEnd( SHINKADEMO_VIEW_WORK* work )
{
  return work->b_cry_end;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         進化スタート
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_VIEW_ChangeStart( SHINKADEMO_VIEW_WORK* work )
{
  work->b_change_start = TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         進化が完了しているか
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        完了しているときTRUEを返す
 */
//-----------------------------------------------------------------------------
BOOL SHINKADEMO_VIEW_ChangeIsEnd( SHINKADEMO_VIEW_WORK* work )
{
  return work->b_change_end;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         BGM SHINKA曲を開始してもよいか
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        開始してもよいときTRUEを返す
 */
//-----------------------------------------------------------------------------
BOOL SHINKADEMO_VIEW_ChangeIsBgmShinkaStart( SHINKADEMO_VIEW_WORK* work )
{
  return work->b_change_bgm_shinka_start;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         BGM SHINKA曲をpushしてもよいか
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        pushしてもよいときTRUEを返す
 */
//-----------------------------------------------------------------------------
BOOL SHINKADEMO_VIEW_ChangeIsBgmShinkaPush( SHINKADEMO_VIEW_WORK* work )
{
  return work->b_change_bgm_shinka_push;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         進化キャンセル
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        成功したらTRUEを返し、直ちに進化キャンセルのフローに切り替わる
 */
//-----------------------------------------------------------------------------
BOOL SHINKADEMO_VIEW_ChangeCancel( SHINKADEMO_VIEW_WORK* work )
{
  if( work->step == STEP_EVO_CHANGE_TRANSFORM )
  {
    if( work->count <= ANM_REPEAT_MAX-3 )
    {
      work->b_change_cancel = TRUE;
      work->step = STEP_EVO_CHANGE_CANCEL;
      return TRUE;
    }
  }
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
static void ShinkaDemo_View_McssInit( SHINKADEMO_VIEW_WORK* work )
{
  work->mcss_sys_wk = MCSS_Init( POKE_MAX, work->heap_id );
  MCSS_SetTextureTransAdrs( work->mcss_sys_wk, POKE_TEX_ADRS_FOR_PARTICLE );  // パーティクルと一緒に使うため
  MCSS_SetOrthoMode( work->mcss_sys_wk );
}
//-------------------------------------
/// MCSSシステム終了処理
//=====================================
static void ShinkaDemo_View_McssExit( SHINKADEMO_VIEW_WORK* work )
{
  MCSS_Exit( work->mcss_sys_wk );
}

//-------------------------------------
/// MCSSポケモン初期化処理
//=====================================
static void ShinkaDemo_View_PokeInit( SHINKADEMO_VIEW_WORK* work )
{
  VecFx32 scale;

  {
    u8 i;
    for(i=0; i<POKE_MAX; i++)
    {
      work->poke_set[i].wk        = NULL;        // NULLで初期化
    }

    scale.x = FX_F32_TO_FX32(work->scale);
    scale.y = FX_F32_TO_FX32(work->scale);
    scale.z = FX32_ONE;
  }

  if( work->launch == SHINKADEMO_VIEW_LAUNCH_EVO )
  {
    u8 i;
    MCSS_ADD_WORK add_wk;

    for(i=0; i<POKE_MAX; i++)
    {
      if(i == POKE_BEFORE)
      {
        MCSS_TOOL_MakeMAWPP( work->pp, &add_wk, MCSS_DIR_FRONT );
      }
      else
      {
        MCSS_TOOL_MakeMAWPP( work->after_pp, &add_wk, MCSS_DIR_FRONT );
      }
      work->poke_set[i].wk = MCSS_Add( work->mcss_sys_wk, 0, POKE_Y, 0, &add_wk );
      MCSS_SetScale( work->poke_set[i].wk, &scale );

      if(i == POKE_BEFORE)
      {
        // 何もしない
      }
      else
      {
/*
  alphaはきれいに見えないのでやめておく(半透明に見えないし、スプライトごとの切れ目が見えるし(Z位置の前後関係のせい？、カメラのせい？))
        MCSS_SetAlpha( work->poke_set[i].wk, 0 );
*/
        MCSS_SetVanishFlag( work->poke_set[i].wk );
      }
    } 
  }
  else
  {
    u8 i;
    MCSS_ADD_WORK add_wk;

    i = POKE_AFTER;

    MCSS_TOOL_MakeMAWPP( work->pp, &add_wk, MCSS_DIR_FRONT );
    work->poke_set[i].wk = MCSS_Add( work->mcss_sys_wk, 0, POKE_Y, 0, &add_wk );
    MCSS_SetScale( work->poke_set[i].wk, &scale );
  }

  ShinkaDemo_View_AdjustPokePos(work);
}
//-------------------------------------
/// MCSSポケモン終了処理
//=====================================
static void ShinkaDemo_View_PokeExit( SHINKADEMO_VIEW_WORK* work )
{
  u8 i;
  for(i=0; i<POKE_MAX; i++)
  {
    if(work->poke_set[i].wk)
    {
      MCSS_SetVanishFlag( work->poke_set[i].wk );
      MCSS_Del( work->mcss_sys_wk, work->poke_set[i].wk );
    }
  }
}

//-------------------------------------
/// disp_pokeではないほうのポケモンを得る
//=====================================
static u8 NotDispPoke( u8 disp_poke )
{
  if( disp_poke == POKE_BEFORE ) return POKE_AFTER;
  else                           return POKE_BEFORE;
}

//-------------------------------------
/// ポケモンの進化演出
//=====================================
static BOOL ShinkaDemo_View_Evolve( SHINKADEMO_VIEW_WORK* work )
{
  BOOL b_finish = FALSE;
  int prev_anm_val = work->anm_val;

  // アニメーションを進める
  work->anm_val += work->anm_add;
  
  // ちょうど一周終了したとき
  if( work->anm_val >= ANM_VAL_MAX )
  {
    work->count++;
    if(
           ( work->count >= ANM_REPEAT_MAX )
        || ( work->b_change_cancel && work->disp_poke == POKE_AFTER )
    )
    {
      work->anm_add = ANM_ADD_START;
      work->anm_val = 0;
      
      b_finish = TRUE;
    }
    else
    {
      work->anm_add = anm_add_tbl[work->count];
      work->anm_val -= ANM_VAL_MAX;
    }

    // disp_pokeとNotDispPokeを入れ替え、新NotDispPokeを非表示にする
    work->disp_poke = NotDispPoke( work->disp_poke );
    MCSS_SetVanishFlag( work->poke_set[NotDispPoke(work->disp_poke)].wk );

    // キャンセル成功しているのに進化してしまわないように、念のため
    if( b_finish && work->b_change_cancel )
    {
      work->disp_poke = POKE_BEFORE;
      MCSS_ResetVanishFlag( work->poke_set[work->disp_poke].wk );
      MCSS_SetVanishFlag( work->poke_set[NotDispPoke(work->disp_poke)].wk );
    }
  }
  // ちょうど半周終了したとき
  else if( prev_anm_val < ANM_VAL_MAX/2 && work->anm_val >= ANM_VAL_MAX/2 )
  {
    work->count++;
    work->anm_add = anm_add_tbl[work->count];

    // disp_pokeを非表示にする
    MCSS_SetVanishFlag( work->poke_set[work->disp_poke].wk );  // alphaはきれいに見えないのでやめておく(半透明に見えないし、スプライトごとの切れ目が見えるし(Z位置の前後関係のせい？、カメラのせい？))
    // NotDispPokeを表示する
    MCSS_ResetVanishFlag( work->poke_set[NotDispPoke(work->disp_poke)].wk );
  }

  // alphaはきれいに見えないのでやめておく(半透明に見えないし、スプライトごとの切れ目が見えるし(Z位置の前後関係のせい？、カメラのせい？))
  //        anm_val        cos01  scale         ポケモン0           ポケモン1
  //        0              1      SCALE_MAX     disp_poke   表示    NotDispPoke 非表示
  //        |              |                    disp_poke   表示    NotDispPoke 非表示
  //  半周  ANM_VAL_MAX/2  0      SCALE_MIN     disp_poke   表示    NotDispPoke 非表示
  //  ちょうど半周終了したとき                  disp_poke   非表示  NotDispPoke 表示
  //        |              |                    disp_poke   非表示  NotDispPoke 表示
  //  一周  ANM_VAL_MAX    1      SCALE_MAX     disp_poke   非表示  NotDispPoke 表示
  //  ちょうど一周終了したとき                  NotDispPoke 非表示  disp_poke   表示
  
  // alphaありのとき
  //        anm_val        cos01  scale         ポケモン0          a   ポケモン1          a
  //        0              1      SCALE_MAX     disp_poke   表示   31  NotDispPoke 非表示 *
  //        |              |                    disp_poke   表示   31  NotDispPoke 非表示 *
  //  半周  ANM_VAL_MAX/2  0      SCALE_MIN     disp_poke   表示   31  NotDispPoke 非表示 *
  //  ちょうど半周終了したとき                  disp_poke   表示   31  NotDispPoke 表示   0
  //        |              |                    disp_poke   表示   |   NotDispPoke 表示   |
  //        |              |                    disp_poke   表示   |   NotDispPoke 表示   31
  //        |              |                    disp_poke   表示   0   NotDispPoke 表示   |
  //        |              |                    disp_poke   表示   |   NotDispPoke 表示   |
  //  一周  ANM_VAL_MAX    1      SCALE_MAX     disp_poke   表示   0   NotDispPoke 表示   31
  //  ちょうど一周終了したとき                  NotDispPoke 非表示 *   disp_poke   表示   31

  // スケール
  {
    // 0～1の範囲におさめたcos値
    fx16 cos01_fx16 = ( FX_CosIdx( work->anm_val ) + FX16_ONE ) /2;
    f32 cos01 = FX_FX16_TO_F32( cos01_fx16 );
    VecFx32 scale;
    u8 i;

    work->scale = ( SCALE_MAX - SCALE_MIN ) * cos01 + SCALE_MIN;

    scale.x = FX_F32_TO_FX32(work->scale);
    scale.y = FX_F32_TO_FX32(work->scale);
    scale.z = FX32_ONE;

    for(i=0; i<POKE_MAX; i++)
    {
      MCSS_SetScale( work->poke_set[i].wk, &scale );
    }
  }

  ShinkaDemo_View_AdjustPokePos(work);

  return b_finish;
}

//-------------------------------------
/// ポケモンの位置を調整する
//=====================================
static void ShinkaDemo_View_AdjustPokePos( SHINKADEMO_VIEW_WORK* work )
{
/*
  u8 i;
  for(i=0; i<POKE_MAX; i++)
  {
    if( work->poke_set[i].wk != NULL )
    {
      f32 size_y = (f32)MCSS_GetSizeY( work->poke_set[i].wk );
      f32 ofs_y;
      VecFx32 ofs;
      size_y *= work->scale / SCALE_MAX;
      if( size_y > POKE_SIZE_MAX ) size_y = POKE_SIZE_MAX;
      ofs_y = ( POKE_SIZE_MAX - size_y ) / 2.0f;
      ofs.x = 0;  ofs.y = FX_F32_TO_FX32(ofs_y);  ofs.z = 0;
      MCSS_SetOfsPosition( work->poke_set[i].wk, &ofs );
    }
  }
*/
}

