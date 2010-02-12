//============================================================================
/**
 *  @file   zukan_detail_voice.c
 *  @brief  図鑑詳細画面の鳴き声画面
 *  @author Koji Kawada
 *  @data   2010.02.09
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_VOICE
 */
//============================================================================
#define DEBUG_KAWADA


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/bmp_winframe.h"
#include "system/poke2dgra.h"
#include "print/printsys.h"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/global_msg.h"
#include "sound/pm_voice.h"

#include "zukan_detail_def.h"
#include "zukan_detail_common.h"
#include "zukan_detail_graphic.h"
#include "zukan_detail_voice.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_zkn.h"

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// メインBGフレーム
#define BG_FRAME_M_GRAPH           (GFL_BG_FRAME2_M)
#define BG_FRAME_M_TIME            (GFL_BG_FRAME3_M)
// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_GRAPH       (2)
#define BG_FRAME_PRI_M_TIME        (1)

// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_GRAPH             = 1,
  BG_PAL_NUM_M_TIME              = 1,
};
// 位置
enum
{
  BG_PAL_POS_M_GRAPH             = 0,
  BG_PAL_POS_M_TIME              = 1,
};

// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_             = 0,
};
// 位置
enum
{
  OBJ_PAL_POS_M_             = 0,
};


// サブBGフレーム
#define BG_FRAME_S_NAME           (GFL_BG_FRAME2_S)
// サブBGフレームのプライオリティ
#define BG_FRAME_PRI_S_NAME       (1)

// サブBGパレット
enum
{
  BG_PAL_NUM_S_NAME              = 1,
};
// 位置
enum
{
  BG_PAL_POS_S_NAME              = 0,
};

// サブOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_S_POKE             = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_S_POKE             = 0,
};


// 文字数
#define STRBUF_TIME_LENGTH       (  8)  // ?'??"


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
  GFL_BMPWIN*                 name_bmpwin;
  
  GFL_BMPWIN*                 time_bmpwin;
  GFL_MSGDATA*                time_msgdata;
  
  u32                         poke_ncg;
  u32                         poke_ncl;
  u32                         poke_nce;
  GFL_CLWK*                   poke_clwk;
  
  u32                         voice_idx;
  BOOL                        voice_play;
  OSTick                      voice_start;
  u16                         voice_disp_millisec;
  u16                         voice_prev_disp_millisec;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // フェード
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_m;
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_s;

  // 終了命令
  END_CMD                     end_cmd;
}
ZUKAN_DETAIL_VOICE_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Voice_VBlankFunc( GFL_TCB* tcb, void* wk );

// ポケモン2D
static void Zukan_Detail_Voice_CreatePoke( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Voice_DeletePoke( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// ポケモン名
static void Zukan_Detail_Voice_CreateNameBase( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Voice_DeleteNameBase( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Voice_CreateName( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Voice_DeleteName( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// ボイス再生時間
static void Zukan_Detail_Voice_CreateTimeBase( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Voice_DeleteTimeBase( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Voice_CreateTime( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                           u16 disp_sec, u16 disp_milli );
static void Zukan_Detail_Voice_DeleteTime( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Voice_UpdateTime( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Voice_ResetTime( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// ポケモン変更
static void Zukan_Detail_Voice_ChangePoke( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 再生ボタンのタッチ
static void Zukan_Detail_Voice_TouchPlayButton( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Voice_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Voice_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Voice_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Voice_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd );

const ZKNDTL_PROC_DATA    ZUKAN_DETAIL_VOICE_ProcData =
{
  Zukan_Detail_Voice_ProcInit,
  Zukan_Detail_Voice_ProcMain,
  Zukan_Detail_Voice_ProcExit,
  Zukan_Detail_Voice_CommandFunc,
  NULL,
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
void             ZUKAN_DETAIL_VOICE_InitParam(
                            ZUKAN_DETAIL_VOICE_PARAM*  param,
                            HEAPID                     heap_id )
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
static ZKNDTL_PROC_RESULT Zukan_Detail_Voice_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_VOICE_PARAM*    param    = (ZUKAN_DETAIL_VOICE_PARAM*)pwk;
  ZUKAN_DETAIL_VOICE_WORK*     work;

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Voice_ProcInit\n" );
  }
#endif

  // ヒープ
  {
    work = ZKNDTL_PROC_AllocWork(proc, sizeof(ZUKAN_DETAIL_VOICE_WORK), param->heap_id);
    GFL_STD_MemClear( work, sizeof(ZUKAN_DETAIL_VOICE_WORK) ); 
  }

  // 他のところから借用するもの
  {
    ZUKAN_DETAIL_GRAPHIC_WORK* graphic = ZKNDTL_COMMON_GetGraphic(cmn);
    work->clunit      = ZUKAN_DETAIL_GRAPHIC_GetClunit(graphic);
    work->font        = ZKNDTL_COMMON_GetFont(cmn);
    work->print_que   = ZKNDTL_COMMON_GetPrintQue(cmn);
  }

  // ボイス
  work->voice_play = FALSE;
  work->voice_disp_millisec = 0;
  work->voice_prev_disp_millisec = 1;  // 最初の描画がされるように、voice_disp_millisecと異なる値で初期化しておく

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Voice_VBlankFunc, work, 1 );

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
static ZKNDTL_PROC_RESULT Zukan_Detail_Voice_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_VOICE_PARAM*    param    = (ZUKAN_DETAIL_VOICE_PARAM*)pwk;
  ZUKAN_DETAIL_VOICE_WORK*     work     = (ZUKAN_DETAIL_VOICE_WORK*)mywk;

  // ポケモン2D
  Zukan_Detail_Voice_DeletePoke( param, work, cmn );
  // ポケモン名
  Zukan_Detail_Voice_DeleteName( param, work, cmn );
  Zukan_Detail_Voice_DeleteNameBase( param, work, cmn );
  // ボイス再生時間
  Zukan_Detail_Voice_DeleteTime( param, work, cmn );
  Zukan_Detail_Voice_DeleteTimeBase( param, work, cmn );

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
    OS_Printf( "Zukan_Detail_Voice_ProcExit\n" );
  }
#endif

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Voice_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_VOICE_PARAM*    param    = (ZUKAN_DETAIL_VOICE_PARAM*)pwk;
  ZUKAN_DETAIL_VOICE_WORK*     work     = (ZUKAN_DETAIL_VOICE_WORK*)mywk;

  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  ZUKAN_DETAIL_HEADBAR_WORK*   headbar  = ZKNDTL_COMMON_GetHeadbar(cmn);
 
  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_PREPARE;

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
        GFL_BG_SetPriority( BG_FRAME_M_GRAPH, BG_FRAME_PRI_M_GRAPH );
        GFL_BG_SetPriority( BG_FRAME_M_TIME, BG_FRAME_PRI_M_TIME );
        
        // サブBG
        GFL_BG_SetPriority( BG_FRAME_S_NAME, BG_FRAME_PRI_S_NAME );
      }

      // ポケモン2D
      Zukan_Detail_Voice_CreatePoke( param, work, cmn );
      // ポケモン名
      Zukan_Detail_Voice_CreateNameBase( param, work, cmn );
      Zukan_Detail_Voice_CreateName( param, work, cmn );
      // ボイス再生時間
      Zukan_Detail_Voice_CreateTimeBase( param, work, cmn );
      Zukan_Detail_Voice_CreateTime( param, work, cmn, 0, 0 );
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
            ZUKAN_DETAIL_TOUCHBAR_DISP_VOICE );
        ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
      }
      // タイトルバー
      if( ZUKAN_DETAIL_HEADBAR_GetState( headbar ) != ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_HEADBAR_SetType(
            headbar,
            ZUKAN_DETAIL_HEADBAR_TYPE_VOICE );
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
        // 再生ボタンのタッチ
        Zukan_Detail_Voice_TouchPlayButton( param, work, cmn );
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
      return ZKNDTL_PROC_RES_FINISH;
    }
    break;
  }

  // ボイスの再生時間
  if( *seq >= SEQ_FADE_IN )
  {
    Zukan_Detail_Voice_UpdateTime( param, work, cmn );
  }

  // フェード
  ZKNDTL_COMMON_FadeMain( work->fade_wk_m, work->fade_wk_s );

  return ZKNDTL_PROC_RES_CONTINUE;
}

//-------------------------------------
/// PROC 命令処理
//=====================================
static void Zukan_Detail_Voice_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd )
{
  if( mywk )
  {
    ZUKAN_DETAIL_VOICE_PARAM*    param    = (ZUKAN_DETAIL_VOICE_PARAM*)pwk;
    ZUKAN_DETAIL_VOICE_WORK*     work     = (ZUKAN_DETAIL_VOICE_WORK*)mywk;

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
    case ZKNDTL_CMD_FORM:
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
          Zukan_Detail_Voice_ChangePoke(param, work, cmn);
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
          Zukan_Detail_Voice_ChangePoke(param, work, cmn);
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


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Zukan_Detail_Voice_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_VOICE_WORK*     work     = (ZUKAN_DETAIL_VOICE_WORK*)wk;
}

//-------------------------------------
/// ポケモン2D
//=====================================
static void Zukan_Detail_Voice_CreatePoke( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

  // リソースを読み込む
  {
    POKEMON_PASO_PARAM*   ppp;
    ARCHANDLE*            handle;

    CLSYS_DRAW_TYPE draw_type = CLSYS_DRAW_SUB;

    // PPP作成
    ppp = (POKEMON_PASO_PARAM*)PP_Create( monsno, 0, 0, param->heap_id );

    // ハンドル
    handle = POKE2DGRA_OpenHandle( param->heap_id );
    // リソース読みこみ
    work->poke_ncg = POKE2DGRA_OBJ_CGR_RegisterPPP( handle, ppp, POKEGRA_DIR_FRONT, draw_type, param->heap_id );
    work->poke_ncl = POKE2DGRA_OBJ_PLTT_RegisterPPP( handle, ppp, POKEGRA_DIR_FRONT, draw_type,
                                                     OBJ_PAL_POS_S_POKE * 0x20, param->heap_id );
    work->poke_nce = POKE2DGRA_OBJ_CELLANM_RegisterPPP( ppp, POKEGRA_DIR_FRONT, ZKNDTL_OBJ_MAPPING_S,
                                                        draw_type, param->heap_id );
    GFL_ARC_CloseDataHandle( handle );

    // PPP破棄
    GFL_HEAP_FreeMemory( ppp );
  }

  // OBJを生成する
  {
    GFL_CLWK_DATA cldata;

    CLSYS_DEFREND_TYPE defrend_type = CLSYS_DEFREND_SUB;

    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    cldata.pos_x = 128;
    cldata.pos_y = 128;
    work->poke_clwk = GFL_CLACT_WK_Create( work->clunit, 
                                           work->poke_ncg, work->poke_ncl, work->poke_nce,
                                           &cldata, defrend_type, param->heap_id );
  
    GFL_CLACT_WK_SetObjMode( work->poke_clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
  }
}
static void Zukan_Detail_Voice_DeletePoke( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // OBJを破棄する
  {
    GFL_CLACT_WK_Remove( work->poke_clwk );
  }

  // リソースを破棄する
  {
    GFL_CLGRP_PLTT_Release( work->poke_ncl );
    GFL_CLGRP_CGR_Release( work->poke_ncg );
    GFL_CLGRP_CELLANIM_Release( work->poke_nce );
  }
}

//-------------------------------------
/// ポケモン名
//=====================================
static void Zukan_Detail_Voice_CreateNameBase( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // パレット
  PALTYPE paltype = PALTYPE_SUB_BG;

  GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FONT, NARC_font_default_nclr,
       paltype,
       0,
       BG_PAL_POS_S_NAME * 0x20,
       BG_PAL_NUM_S_NAME * 0x20,
       param->heap_id );

  // ビットマップウィンドウ
  work->name_bmpwin = GFL_BMPWIN_Create( BG_FRAME_S_NAME,
                                         10, 5, 12, 4,
                                         BG_PAL_POS_S_NAME,
                                         GFL_BMP_CHRAREA_GET_B );
 
  // クリア
  Zukan_Detail_Voice_DeleteName( param, work, cmn );
}
static void Zukan_Detail_Voice_DeleteNameBase( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  GFL_BMPWIN_Delete( work->name_bmpwin );
}
static void Zukan_Detail_Voice_CreateName( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

  STRBUF* strbuf = GFL_MSG_CreateString( GlobalMsg_PokeName, monsno );

  u16 str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
  u16 bmp_width = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->name_bmpwin) );
  u16 x = ( bmp_width - str_width ) / 2;

/*
  u16 str_height = (u16)( PRINTSYS_GetStrHeight( strbuf, work->font ) );
  u16 bmp_height = GFL_BMP_GetSizeY( GFL_BMPWIN_GetBmp(work->name_bmpwin) );
  u16 y = ( bmp_height - str_height ) / 2;
*/
  u16 y = 10;  // 計算で出した位置は中心より上に見えるので

  PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->name_bmpwin ),
                          x, y, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );

  GFL_STR_DeleteBuffer( strbuf );

  // スクリーン転送
  GFL_BMPWIN_MakeTransWindow_VBlank( work->name_bmpwin );
}
static void Zukan_Detail_Voice_DeleteName( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->name_bmpwin ), 0 );

  GFL_BMPWIN_TransVramCharacter( work->name_bmpwin );
}

//-------------------------------------
/// ボイス再生時間
//=====================================
static void Zukan_Detail_Voice_CreateTimeBase( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // パレット
  PALTYPE paltype = PALTYPE_MAIN_BG;

  GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FONT, NARC_font_default_nclr,
       paltype,
       0,
       BG_PAL_POS_M_TIME * 0x20,
       BG_PAL_NUM_M_TIME * 0x20,
       param->heap_id );

  // ビットマップウィンドウ
  work->time_bmpwin = GFL_BMPWIN_Create( BG_FRAME_M_TIME,
                                         26, 18, 5, 2,
                                         BG_PAL_POS_M_TIME,
                                         GFL_BMP_CHRAREA_GET_B );
 
  // クリア
  Zukan_Detail_Voice_DeleteTime( param, work, cmn );

  // メッセージ
  work->time_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
                           ARCID_MESSAGE, NARC_message_zkn_dat,
                           param->heap_id );
}
static void Zukan_Detail_Voice_DeleteTimeBase( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  GFL_MSG_Delete( work->time_msgdata );
  GFL_BMPWIN_Delete( work->time_bmpwin );
}
static void Zukan_Detail_Voice_CreateTime( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                           u16 disp_sec, u16 disp_milli )
{
  WORDSET* wordset = WORDSET_Create( param->heap_id );
  STRBUF* src_strbuf = GFL_MSG_CreateString( work->time_msgdata, ZKN_VOICE_TEXT_01 );
  STRBUF* strbuf = GFL_STR_CreateBuffer( STRBUF_TIME_LENGTH, param->heap_id );
    
  WORDSET_RegisterNumber( wordset, 0, disp_sec, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( wordset, 1, disp_milli, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
   
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(work->time_bmpwin), 0, 1, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );  // PRINTSYS_PrintQueColor

  GFL_STR_DeleteBuffer( strbuf );
  GFL_STR_DeleteBuffer( src_strbuf );
  WORDSET_Delete( wordset );

  GFL_BMPWIN_MakeTransWindow_VBlank( work->time_bmpwin );
}
static void Zukan_Detail_Voice_DeleteTime( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->time_bmpwin ), 0 );

  GFL_BMPWIN_TransVramCharacter( work->time_bmpwin );
}
static void Zukan_Detail_Voice_UpdateTime( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  if( work->voice_play )
  {
    if( !PMVOICE_CheckPlay( work->voice_idx ) )
    {
      work->voice_play = FALSE;
    }
    {
      OSTick now = OS_GetTick();  // MachineSystem_Init関数でOS_InitTickは呼ばれている
      OSTick diff;
      u64 diff_millisec;
      if( now >= work->voice_start )
      {
        diff = now - work->voice_start;
      }
      else
      {
        diff = now + ( 0xFFFFFFFFFFFFFFFF - work->voice_start );
      }
      diff_millisec =OS_TicksToMilliSeconds( diff );
      if( diff_millisec > 9990 ) diff_millisec = 9990;
      work->voice_disp_millisec = (u16)diff_millisec;
    }
  }

  // 表示
  if( work->voice_disp_millisec != work->voice_prev_disp_millisec )
  {
    u16 disp_sec = work->voice_disp_millisec / 1000;
    u16 disp_milli = work->voice_disp_millisec % 1000;
    disp_milli = disp_milli / 10 + ( (disp_milli%10 < 5) ? (0) : (1) );  // 四捨五入

    Zukan_Detail_Voice_DeleteTime( param, work, cmn );
    Zukan_Detail_Voice_CreateTime( param, work, cmn, disp_sec, disp_milli );

    work->voice_prev_disp_millisec = work->voice_disp_millisec;
  }
}
static void Zukan_Detail_Voice_ResetTime( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  if( work->voice_play )
  {
    if( PMVOICE_CheckPlay( work->voice_idx ) )
    {
      PMVOICE_Stop( work->voice_idx );
    }
    work->voice_play = FALSE;
  }

  work->voice_disp_millisec = 0;
}

//-------------------------------------
/// ポケモン変更
//=====================================
static void Zukan_Detail_Voice_ChangePoke( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  Zukan_Detail_Voice_DeleteName( param, work, cmn );
  Zukan_Detail_Voice_DeletePoke( param, work, cmn );
  
  Zukan_Detail_Voice_CreatePoke( param, work, cmn );
  Zukan_Detail_Voice_CreateName( param, work, cmn );

  Zukan_Detail_Voice_ResetTime( param, work, cmn );
}

//-------------------------------------
/// 再生ボタンのタッチ
//=====================================
static void Zukan_Detail_Voice_TouchPlayButton( ZUKAN_DETAIL_VOICE_PARAM* param, ZUKAN_DETAIL_VOICE_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u32 x, y;

  // タッチ判定
  if( GFL_UI_TP_GetPointTrg(&x, &y) )
  {
    if(    0<=x && x<256
        && 0<=y && y<168 )
    {
      u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);
      u16 formno = 0;
   
      work->voice_idx = PMVOICE_Play( monsno, formno, 64, FALSE, 0, 0, FALSE, 0 );
      work->voice_disp_millisec = 0;
      work->voice_start = OS_GetTick();  // MachineSystem_Init関数でOS_InitTickは呼ばれている
      work->voice_play = TRUE;
    }
  }
}

