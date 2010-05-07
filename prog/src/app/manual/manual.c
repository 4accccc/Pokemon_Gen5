//============================================================================
/**
 *  @file   manual.c
 *  @brief  ゲーム内マニュアル
 *  @author Koji Kawada
 *  @data   2010.04.26
 *  @note   
 *  モジュール名：MANUAL
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_data.h"
#include "print/gf_font.h"
#include "print/printsys.h"

#include "manual_graphic.h"
#include "manual_def.h"
#include "manual_common.h"
#include "manual_touchbar.h"
#include "manual_top.h"
#include "app/manual.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_manual.h"
#include "manual.naix"

// ダミー
#include "msg/msg_zkn.h"
#include "zukan_gra.naix"


// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x30000)               ///< ヒープサイズ

// ProcMainのシーケンス
enum
{
  SEQ_START          = 0,
  SEQ_FADE_IN,
  SEQ_MAIN,
  SEQ_FADE_OUT,
  SEQ_END,
};

// テキスト
enum
{
  TEXT_DUMMY,
  TEXT_MAX,
};

// BG_PAL_POS_M_TEXTの割り当て
#define TEXT_PAL_POS      (BG_PAL_POS_S_TEXT)
#define TEXT_COLOR_L      (1)  // 文字主色
#define TEXT_COLOR_S      (2)  // 文字影色
#define TEXT_COLOR_B      (0)  // 文字背景色(透明)

static const u8 bmpwin_setup[TEXT_MAX][9] =
{
  // frmnum              posx  posy  sizx  sizy  palnum          dir                    x  y (x,yは無視してセンタリングすることもある)
  {  BG_FRAME_S_TB_TEXT,    0,    0,    1,    1, TEXT_PAL_POS,   GFL_BMP_CHRAREA_GET_F, 0, 0 },
};

// フェード
#define FADE_IN_WAIT  (0)
#define FADE_OUT_WAIT (0)


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
  // ヒープ、パラメータなど
  HEAPID                      heap_id;
  MANUAL_PARAM*               param;
 
  // 共通
  MANUAL_COMMON_WORK*         cmn_wk;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // タッチバー
  MANUAL_TOUCHBAR_WORK*       mtb_wk;
  // トップ画面
  MANUAL_TOP_WORK*            top_wk;
}
MANUAL_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Manual_VBlankFunc( GFL_TCB* tcb, void* wk );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Manual_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Manual_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Manual_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA  MANUAL_ProcData =
{
  Manual_ProcInit,
  Manual_ProcMain,
  Manual_ProcExit,
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
 *  @param[in]       
 *  @param[in,out]       
 *
 *  @retval          
 */
//------------------------------------------------------------------


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT Manual_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MANUAL_WORK*     work;

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MANUAL, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(MANUAL_WORK), HEAPID_MANUAL );
    GFL_STD_MemClear( work, sizeof(MANUAL_WORK) );
    
    work->heap_id       = HEAPID_MANUAL;
    work->param         = (MANUAL_PARAM*)pwk;

    work->param->result = MANUAL_RESULT_RETURN;
  }

  // 共通
  work->cmn_wk = MANUAL_COMMON_Init( work->param->gamedata, work->heap_id );

  // メインBG
  GFL_BG_SetPriority( BG_FRAME_M_PIC,     BG_FRAME_PRI_M_PIC );
  // サブBG
  GFL_BG_SetPriority( BG_FRAME_S_REAR,    BG_FRAME_PRI_S_REAR );
  GFL_BG_SetPriority( BG_FRAME_S_MAIN,    BG_FRAME_PRI_S_MAIN );
  GFL_BG_SetPriority( BG_FRAME_S_TB_BAR,  BG_FRAME_PRI_S_TB_BAR );
  GFL_BG_SetPriority( BG_FRAME_S_TB_TEXT, BG_FRAME_PRI_S_TB_TEXT );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_VBlankFunc, work, 1 );
  
  // フェードイン(黒→見える)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );

  // 通信アイコン
  GFL_NET_WirelessIconEasy_HoldLCD( FALSE, work->heap_id );
  GFL_NET_ReloadIcon();

  // マニュアルタッチバー
  work->mtb_wk = MANUAL_TOUCHBAR_Init( work->cmn_wk );
  // トップ画面
  work->top_wk = MANUAL_TOP_Init( work->cmn_wk );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Manual_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MANUAL_WORK* work = (MANUAL_WORK*)mywk;

  // トップ画面
  MANUAL_TOP_Exit( work->top_wk );
  // マニュアルタッチバー
  MANUAL_TOUCHBAR_Exit( work->mtb_wk );

  // 通信アイコン
  GFL_NET_WirelessIconEasy_DefaultLCD();

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // 共通
  MANUAL_COMMON_Exit( work->cmn_wk );

  // ヒープ、パラメータなど
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_MANUAL );
  }

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT Manual_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MANUAL_WORK* work = (MANUAL_WORK*)mywk;

  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_FADE_IN;
    }
    break;
  case SEQ_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      BOOL b_end = FALSE;
      u32 x, y;
      if( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) )
      {
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        b_end = TRUE;
        work->param->result = MANUAL_RESULT_RETURN;
      }
      else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
      {
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        b_end = TRUE;
        work->param->result = MANUAL_RESULT_CLOSE;
      }
      else if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
        b_end = TRUE;
        work->param->result = MANUAL_RESULT_RETURN;
      }

      if( b_end )
      {
        *seq = SEQ_FADE_OUT;
        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        *seq = SEQ_END;
      }
    }
    break;
  case SEQ_END:
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  // 共通
  MANUAL_COMMON_Main( work->cmn_wk );

  // マニュアルタッチバー
  MANUAL_TOUCHBAR_Main( work->mtb_wk );
  // トップ画面
  MANUAL_TOP_Main( work->top_wk );

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Manual_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  MANUAL_WORK* work = (MANUAL_WORK*)wk;
}

