//============================================================================
/**
 *  @file   zukan_detail_form.c
 *  @brief  図鑑詳細画面の姿画面
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_FORM
 */
//============================================================================
#define DEBUG_KAWADA


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/mcss.h"
#include "system/mcss_tool.h"

#include "zukan_detail_def.h"
#include "zukan_detail_common.h"
#include "zukan_detail_graphic.h"
#include "zukan_detail_form.h"

// アーカイブ
#include "arc_def.h"

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// ProcMainのシーケンス
enum
{
  SEQ_START      = 0,
  SEQ_PREPARE,
  SEQ_FADE_IN,
  SEQ_MAIN,
  SEQ_FADE_OUT,
  SEQ_END,
};

// 終了命令
typedef enum
{
  END_CMD_NONE,
  END_CMD_INSIDE,
  END_CMD_OUTSIDE,
}
END_CMD;

// MCSSポケモン
enum
{
  POKE_CURR_F,    // 今のフォルム(前向き)
  POKE_CURR_B,    // 今のフォルム(後向き)
  POKE_COMP_F,    // 比較フォルム(前向き)
  POKE_COMP_B,    // 比較フォルム(後向き)
  POKE_MAX,
};

// フォルム数
#define FORM_MAX (64)  // これで足りるか？

// ポケモンの大きさ
#define POKE_SCALE    (16.0f)
#define POKE_SIZE_MAX (96.0f)

// ポケモンの位置
enum
{
  POKE_POS_CENTER,
  POKE_POS_CENTER_L,
  POKE_POS_CENTER_R,
  POKE_POS_RIGHT,
  POKE_POS_MAX,
};
static VecFx32 poke_pos[POKE_POS_MAX] =
{
  { FX_F32_TO_FX32(  0.0f), FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f) },
  { FX_F32_TO_FX32(-64.0f), FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f) },
  { FX_F32_TO_FX32( 64.0f), FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f) },
  { FX_F32_TO_FX32(128.0f), FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(0.0f) },
};


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // 他のところから借用するもの
  GFL_CLUNIT*                 clunit;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // ここで作成するもの
  MCSS_SYS_WORK*              mcss_sys_wk;
  MCSS_WORK*                  poke_wk[POKE_MAX];  // NULLのときなし

  u16                         form_comp_list[FORM_MAX];
  u16                         form_comp_num;  // 最大FORM_MAX
  u16                         form_comp_no;  // form_comp_list[form_comp_no]が比較フォルム現在

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // フェード
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_m;
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_s;

  // 終了命令
  END_CMD                     end_cmd;
}
ZUKAN_DETAIL_FORM_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Form_VBlankFunc( GFL_TCB* tcb, void* wk );

// MCSSポケモン
static void Zukan_Detail_Form_McssSysInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_McssSysExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static MCSS_WORK* PokeInit( MCSS_SYS_WORK* mcss_sys_wk,
                      int mons_no, int form_no, int sex, int rare, BOOL egg, int dir );
static void PokeExit( MCSS_SYS_WORK* mcss_sys_wk, MCSS_WORK* poke_wk );
static void PokeAdjustOfsPos( MCSS_WORK* poke_wk );

// ポケモン変更
static void Zukan_Detail_Form_ChangePoke( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// フォルム変更
static void Zukan_Detail_Form_ChangeForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd );
static void Zukan_Detail_Form_Draw3DFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );

const ZKNDTL_PROC_DATA    ZUKAN_DETAIL_FORM_ProcData =
{
  Zukan_Detail_Form_ProcInit,
  Zukan_Detail_Form_ProcMain,
  Zukan_Detail_Form_ProcExit,
  Zukan_Detail_Form_CommandFunc,
  Zukan_Detail_Form_Draw3DFunc,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             ZUKAN_DETAIL_FORM_InitParam(
                            ZUKAN_DETAIL_FORM_PARAM*  param,
                            HEAPID                    heap_id )
{
  param->heap_id = heap_id;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
  ZUKAN_DETAIL_FORM_WORK*     work;

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Form_ProcInit\n" );
  }
#endif

  // ヒープ
  {
    work = ZKNDTL_PROC_AllocWork(proc, sizeof(ZUKAN_DETAIL_FORM_WORK), param->heap_id);
    GFL_STD_MemClear( work, sizeof(ZUKAN_DETAIL_FORM_WORK) ); 
  }

  // 他のところから借用するもの
  {
    ZUKAN_DETAIL_GRAPHIC_WORK* graphic = ZKNDTL_COMMON_GetGraphic(cmn);
    work->clunit      = ZUKAN_DETAIL_GRAPHIC_GetClunit(graphic);
    work->font        = ZKNDTL_COMMON_GetFont(cmn);
    work->print_que   = ZKNDTL_COMMON_GetPrintQue(cmn);
  }

  // NULL初期化
  {
    u8 i;
    work->mcss_sys_wk = NULL;
    for( i=0; i<POKE_MAX; i++ )
    {
      work->poke_wk[i] = NULL;
    }
    work->form_comp_num = 0;
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Form_VBlankFunc, work, 1 );

  // フェード
  {
    work->fade_wk_m = ZKNDTL_COMMON_FadeInit( param->heap_id );
    work->fade_wk_s = ZKNDTL_COMMON_FadeInit( param->heap_id );
   
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_s );

    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
  }

  // 終了情報
  work->end_cmd = END_CMD_NONE;

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
  ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

  // フェード
  {
    ZKNDTL_COMMON_FadeExit( work->fade_wk_s );
    ZKNDTL_COMMON_FadeExit( work->fade_wk_m );
  }

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // ヒープ
  ZKNDTL_PROC_FreeWork( proc );

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Form_ProcExit\n" );
  }
#endif

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
  ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

  ZUKAN_DETAIL_GRAPHIC_WORK*   graphic  = ZKNDTL_COMMON_GetGraphic(cmn);

  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  ZUKAN_DETAIL_HEADBAR_WORK*   headbar  = ZKNDTL_COMMON_GetHeadbar(cmn);
 
  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_PREPARE;

      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D );
      }

      // BG
      {
        // クリア
        u8 i;
        for( i=GFL_BG_FRAME0_M; i<=GFL_BG_FRAME3_S; i++ )
        {
          if(    i != ZKNDTL_BG_FRAME_M_TOUCHBAR
              && i != ZKNDTL_BG_FRAME_S_HEADBAR )
          {
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_X_SET, 0 );
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_Y_SET, 0 );
            GFL_BG_ClearFrame(i);
          }
        }

        // メインBG
        //GFL_BG_SetPriority( BG_FRAME_M_, BG_FRAME_PRI_M_ );
        
        // サブBG
        //GFL_BG_SetPriority( BG_FRAME_S_, BG_FRAME_PRI_S_ );
      }

      // 3D
      ZUKAN_DETAIL_GRAPHIC_Init3D( graphic, param->heap_id );

      // MCSSポケモン
      Zukan_Detail_Form_McssSysInit( param, work, cmn );
      work->poke_wk[0] = PokeInit( work->mcss_sys_wk, 201, 0, 0, 0, FALSE, MCSS_DIR_FRONT );
      work->poke_wk[1] = PokeInit( work->mcss_sys_wk, 201, 0, 0, 0, FALSE, MCSS_DIR_BACK );
      work->poke_wk[2] = PokeInit( work->mcss_sys_wk, 201, 1, 0, 0, FALSE, MCSS_DIR_FRONT );
      work->poke_wk[3] = PokeInit( work->mcss_sys_wk, 201, 1, 0, 0, FALSE, MCSS_DIR_BACK );
      {
        u8 i;
        for( i=0; i<4; i++ )
        {
          MCSS_SetPosition( work->poke_wk[i], &poke_pos[i] );

          MCSS_SetVanishFlag( work->poke_wk[i] );
          MCSS_SetPaletteFade( work->poke_wk[i], 16, 16, 0, 0x0000 );
        }
      }
    }
    break;
  case SEQ_PREPARE:
    {
      *seq = SEQ_FADE_IN;

      // フェード
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_m );
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_s );

      // タッチバー
      if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
            ZUKAN_DETAIL_TOUCHBAR_DISP_FORM );
        ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
      }
      // タイトルバー
      if( ZUKAN_DETAIL_HEADBAR_GetState( headbar ) != ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_HEADBAR_SetType(
            headbar,
            ZUKAN_DETAIL_HEADBAR_TYPE_FORM );
        ZUKAN_DETAIL_HEADBAR_Appear( headbar );
      }
    }
    break;
  case SEQ_FADE_IN:
    {
      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );

        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      if( work->end_cmd != END_CMD_NONE )
      {
        *seq = SEQ_FADE_OUT;

        // フェード
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_m );
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );
        
        // タイトルバー
        ZUKAN_DETAIL_HEADBAR_Disappear( headbar );
        
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_Disappear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }
      }
      else
      {
        // タッチ
        //Zukan_Detail_Form_Touch( param, work, cmn );
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR )
      {
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR )
          {
            *seq = SEQ_END;
          }
        }
        else
        {
          *seq = SEQ_END;
        }
      }
    }
    break;
  case SEQ_END:
    {
      // MCSSポケモン
      {
        u8 i;
        for( i=0; i<POKE_MAX; i++ )
        {
          PokeExit( work->mcss_sys_wk, work->poke_wk[i] );
          work->poke_wk[i] = NULL;
        }
      }
      Zukan_Detail_Form_McssSysExit( param, work, cmn );
      work->mcss_sys_wk = NULL;

      // 3D
      ZUKAN_DETAIL_GRAPHIC_Exit3D( graphic );

      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );
      }

      return ZKNDTL_PROC_RES_FINISH;
    }
    break;
  }

  // MCSSポケモン
  if( work->mcss_sys_wk )
  {
    MCSS_Main( work->mcss_sys_wk );
  } 

  // フェード
  ZKNDTL_COMMON_FadeMain( work->fade_wk_m, work->fade_wk_s );

  return ZKNDTL_PROC_RES_CONTINUE;
}

//-------------------------------------
/// PROC 命令処理
//=====================================
static void Zukan_Detail_Form_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd )
{
  if( mywk )
  {
    ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
    ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

    ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  
    switch( cmd )
    {
    case ZKNDTL_CMD_NONE:
      {
      }
      break;
    case ZKNDTL_CMD_CLOSE:
    case ZKNDTL_CMD_RETURN:
      {
        work->end_cmd = END_CMD_OUTSIDE;
      }
      break;
    case ZKNDTL_CMD_INFO:
    case ZKNDTL_CMD_MAP:
    case ZKNDTL_CMD_VOICE:
      {
        work->end_cmd = END_CMD_INSIDE;
      }
      break;
    case ZKNDTL_CMD_CUR_D:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToNextPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          //Zukan_Detail_Form_ChangePoke(param, work, cmn);
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CUR_U:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToPrevPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          //Zukan_Detail_Form_ChangePoke(param, work, cmn);
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CHECK:
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    default:
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    }
  }
}

//-------------------------------------
/// PROC 3D描画
//=====================================
static void Zukan_Detail_Form_Draw3DFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  if( mywk )
  {
    ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
    ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

    if( work->mcss_sys_wk )
      MCSS_Draw( work->mcss_sys_wk );
  }
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Zukan_Detail_Form_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)wk;
}

//-------------------------------------
/// MCSSポケモン
//=====================================
static void Zukan_Detail_Form_McssSysInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  work->mcss_sys_wk = MCSS_Init( POKE_MAX, param->heap_id );
  MCSS_SetTextureTransAdrs( work->mcss_sys_wk, 0 );  // 他に一緒に出す3Dはないので
  MCSS_SetOrthoMode( work->mcss_sys_wk );
}
static void Zukan_Detail_Form_McssSysExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  MCSS_Exit( work->mcss_sys_wk );
}

static MCSS_WORK* PokeInit( MCSS_SYS_WORK* mcss_sys_wk,
                      int mons_no, int form_no, int sex, int rare, BOOL egg, int dir )
{
  VecFx32 scale =
  {
    FX_F32_TO_FX32(POKE_SCALE),
    FX_F32_TO_FX32(POKE_SCALE),
    FX32_ONE, 
  };

  MCSS_ADD_WORK add_wk;
  MCSS_WORK*    poke_wk;

  MCSS_TOOL_MakeMAWParam( mons_no, form_no, sex, rare, egg, &add_wk, dir );
  poke_wk = MCSS_Add( mcss_sys_wk, 0, 0, 0, &add_wk );
  
  MCSS_SetScale( poke_wk, &scale );

  PokeAdjustOfsPos( poke_wk );

  return poke_wk;
}
static void PokeExit( MCSS_SYS_WORK* mcss_sys_wk, MCSS_WORK* poke_wk )
{
  MCSS_SetVanishFlag( poke_wk );
  MCSS_Del( mcss_sys_wk, poke_wk );
}
static void PokeAdjustOfsPos( MCSS_WORK* poke_wk )
{
  f32 size_y = (f32)MCSS_GetSizeY( poke_wk );
  f32 ofs_y;
  VecFx32 ofs;
  if( size_y > POKE_SIZE_MAX ) size_y = POKE_SIZE_MAX;
  ofs_y = ( POKE_SIZE_MAX - size_y ) / 2.0f;
  ofs.x = 0;  ofs.y = FX_F32_TO_FX32(ofs_y);  ofs.z = 0;
  MCSS_SetOfsPosition( poke_wk, &ofs );
}

//-------------------------------------
/// ポケモン変更
//=====================================
static void Zukan_Detail_Form_ChangePoke( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
}

//-------------------------------------
/// フォルム変更
//=====================================
static void Zukan_Detail_Form_ChangeForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
}

