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
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/mcss.h"
#include "system/mcss_tool.h"

#include "shinka_demo_view.h"


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
  // EVO
  STEP_EVO_START,             // 開始待ち中→白くする
  STEP_EVO_WHITE,             // 白くなるのを待ち中→拡大縮小入れ替え演出を開始する
  STEP_EVO_SCALE,             // 拡大縮小入れ替え演出中→色を付ける
  STEP_EVO_COLOR,             // 色が付くのを待ち中→終了する
  STEP_EVO_END,               // 終了中

  // AFTER
  STEP_AFTER,                 // 進化後からスタート
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
#define ANM_REPEAT_MAX       (15)
#define SCALE_MAX            (16.0f)
#define SCALE_MIN            ( 4.0f)
#define POKE_SIZE_MAX        (96.0f)


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
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

  // ステップ
  STEP                     step;

  // フラグ
  BOOL                     b_start;
  BOOL                     b_cancel;  // 進化キャンセル

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

  // ステップ
  {
    if( work->launch == SHINKADEMO_VIEW_LAUNCH_EVO )
      work->step         = STEP_EVO_START;
    else
      work->step         = STEP_AFTER;
  }

  // フラグ
  {
    work->b_start      = FALSE;
    work->b_cancel     = FALSE;
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
  case STEP_EVO_START:
    {
      if( work->b_start )
      {
        // 次へ
        work->step = STEP_EVO_WHITE;

        MCSS_SetPaletteFade( work->poke_set[work->disp_poke].wk, 0, 16, 1, 0x7fff );
        MCSS_SetPaletteFade( work->poke_set[NotDispPoke(work->disp_poke)].wk, 16, 16, 0, 0x7fff );
      }
    }
    break;
  case STEP_EVO_WHITE:
    {
      if( !MCSS_CheckExecutePaletteFade( work->poke_set[work->disp_poke].wk ) )
      {
        // 次へ
        work->step = STEP_EVO_SCALE;
      }
    }
    break;
  case STEP_EVO_SCALE:
    {
      if( ShinkaDemo_View_Evolve( work ) )
      {
        // 次へ
        work->step = STEP_EVO_COLOR;
        
        MCSS_SetPaletteFade( work->poke_set[work->disp_poke].wk, 16, 0, 1, 0x7fff );
      }
    }
    break;
  case STEP_EVO_COLOR:
    {
      if( !MCSS_CheckExecutePaletteFade( work->poke_set[work->disp_poke].wk ) )
      {
        // 次へ
        work->step = STEP_EVO_END;
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
 *  @brief         進化キャンセル
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval       キャンセルできたときTRUE
 */
//-----------------------------------------------------------------------------
BOOL SHINKADEMO_VIEW_CancelShinka( SHINKADEMO_VIEW_WORK* work )
{
  if( work->count <= ANM_REPEAT_MAX-2 )
  {
    work->b_cancel = TRUE;
    return TRUE;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         スタート
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        なし 
 */
//-----------------------------------------------------------------------------
void SHINKADEMO_VIEW_StartShinka( SHINKADEMO_VIEW_WORK* work )
{
  work->b_start = TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief         終了しているか
 *
 *  @param[in,out] work  SHINKADEMO_VIEW_Initで生成したワーク
 *
 *  @retval        終了しているときTRUEを返す
 */
//-----------------------------------------------------------------------------
BOOL SHINKADEMO_VIEW_IsEndShinka( SHINKADEMO_VIEW_WORK* work )
{
  if(
         work->step == STEP_EVO_COLOR
      || work->step == STEP_EVO_END
      || work->step == STEP_AFTER
  ) 
    return TRUE;
  return FALSE;
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// 
//=====================================
static void ShinkaDemo_View_McssInit( SHINKADEMO_VIEW_WORK* work )
{
  work->mcss_sys_wk = MCSS_Init( POKE_MAX, work->heap_id );
  MCSS_SetTextureTransAdrs( work->mcss_sys_wk, 0 );
  MCSS_SetOrthoMode( work->mcss_sys_wk );
}
//-------------------------------------
/// 
//=====================================
static void ShinkaDemo_View_McssExit( SHINKADEMO_VIEW_WORK* work )
{
  MCSS_Exit( work->mcss_sys_wk );
}

//-------------------------------------
/// 
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
        int sex = PP_Get( work->pp, ID_PARA_sex, NULL );
        MCSS_TOOL_MakeMAWParam( work->after_monsno, 0, sex, 0, 0, &add_wk, MCSS_DIR_FRONT );
      }
      work->poke_set[i].wk = MCSS_Add( work->mcss_sys_wk, 0, 0, 0, &add_wk );
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
    work->poke_set[i].wk = MCSS_Add( work->mcss_sys_wk, 0, 0, 0, &add_wk );
    MCSS_SetScale( work->poke_set[i].wk, &scale );
  }

  ShinkaDemo_View_AdjustPokePos(work);
}
//-------------------------------------
/// 
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
/// 
//=====================================
static u8 NotDispPoke( u8 disp_poke )
{
  if( disp_poke == POKE_BEFORE ) return POKE_AFTER;
  else                           return POKE_BEFORE;
}

//-------------------------------------
/// 
//=====================================
static BOOL ShinkaDemo_View_Evolve( SHINKADEMO_VIEW_WORK* work )
{
  const int anm_add_tbl[ANM_REPEAT_MAX] =  // ANM_VAL_MAX/2 より小さな値で
  {                    // disp_poke
    ANM_ADD_START,     // BEFORE
    0x400,
    0x400,             // BEFORE
    0x800,
    0x800,             // BEFORE
    0x800,
    0x800,             // BEFORE
    0x1000,
    0x1000,            // BEFORE
    0x1000,
    0x1000,            // BEFORE
    0x1000,
    0x1000,            // BEFORE        // ANM_REPEAT_MAX-3
    0x1000,                             // ANM_REPEAT_MAX-2
    0x1000,            // BEFORE        // ANM_REPEAT_MAX-1
  };

  BOOL b_finish = FALSE;
  int prev_anm_val = work->anm_val;

  // アニメーションを進める
  work->anm_val += work->anm_add;
  
  // ちょうど一周終了したとき
  if( work->anm_val >= ANM_VAL_MAX )
  {
    work->count++;
    if(
           ( work->count >= sizeof(anm_add_tbl)/sizeof(int) )
        || ( work->b_cancel && work->disp_poke == POKE_AFTER )
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
  }

  // ちょうど半周終了したとき
  if( prev_anm_val < ANM_VAL_MAX/2 && work->anm_val >= ANM_VAL_MAX/2 )
  {
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

/*
  alphaはきれいに見えないのでやめておく(半透明に見えないし、スプライトごとの切れ目が見えるし(Z位置の前後関係のせい？、カメラのせい？))
  // アルファ
  if( work->anm_val < ANM_VAL_MAX/2 )  // だんだん小さく
  {
    MCSS_SetAlpha( work->poke_set[work->disp_poke].wk, 31 );
    MCSS_SetAlpha( work->poke_set[NotDispPoke(work->disp_poke)].wk, 0 );
  }
  else  // だんだん大きく
  {
    int alpha_anm_val;
    fx16 alpha_cos01_fx16;
    f32 alpha_cos01;
    f32 alpha_f32;
    u8 alpha;
    {
      alpha_anm_val = ( work->anm_val - ANM_VAL_MAX/2 ) *2;
      if( alpha_anm_val > ANM_VAL_MAX/2 ) alpha_anm_val = ANM_VAL_MAX/2;
      alpha_cos01_fx16 = ( FX_CosIdx( alpha_anm_val ) + FX16_ONE ) /2;
      alpha_cos01 = FX_FX16_TO_F32( alpha_cos01_fx16 );
      alpha_f32 = 32.0f * alpha_cos01;
      if( alpha_f32 < 0.0f ) alpha_f32 = 0.0f;
      alpha = (u8)alpha_f32;
      if( alpha > 31 ) alpha = 31;
      MCSS_SetAlpha( work->poke_set[work->disp_poke].wk, alpha );
    }
    {
      alpha_anm_val = ( work->anm_val - ANM_VAL_MAX/2 ) *3;
      if( alpha_anm_val > ANM_VAL_MAX/2 ) alpha_anm_val = ANM_VAL_MAX/2;
      alpha_cos01_fx16 = ( FX_CosIdx( alpha_anm_val ) + FX16_ONE ) /2;
      alpha_cos01 = FX_FX16_TO_F32( alpha_cos01_fx16 );
      alpha_cos01 = 1.0f - alpha_cos01;
      alpha_f32 = 32.0f * alpha_cos01;
      if( alpha_f32 < 0.0f ) alpha_f32 = 0.0f;
      alpha = (u8)alpha_f32;
      if( alpha > 31 ) alpha = 31;
      MCSS_SetAlpha( work->poke_set[NotDispPoke(work->disp_poke)].wk, alpha );
    }
  }
*/

  ShinkaDemo_View_AdjustPokePos(work);

  return b_finish;
}

//-------------------------------------
/// 
//=====================================
static void ShinkaDemo_View_AdjustPokePos( SHINKADEMO_VIEW_WORK* work )
{
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
}

