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
#include "app/manual.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
//#include "msg/msg_????.h"
//#include "manual_gra????.naix"

// サウンド

// オーバーレイ


#include "app/app_nogear_subscreen.h"


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x30000)               ///< ヒープサイズ

// メインBGフレーム
#define BG_FRAME_M_FRONT       (GFL_BG_FRAME1_M)
#define BG_FRAME_M_TEXT        (GFL_BG_FRAME2_M)

// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_FRONT   (1)
#define BG_FRAME_PRI_M_TEXT    (0)

// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_GRA_FRONT     = 15,
  BG_PAL_NUM_M_TEXT          = 1,
};
// 位置
enum
{
  BG_PAL_POS_M_GRA_FRONT    =  0,
  BG_PAL_POS_M_TEXT         = 15,
};

// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_BALL        = 2,
  OBJ_PAL_NUM_M_GF          = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_M_BALL        = 0,
  OBJ_PAL_POS_M_GF          = 2,
};

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
#define TEXT_PAL_POS      (BG_PAL_POS_M_TEXT)
#define TEXT_COLOR_L      (1)  // 文字主色
#define TEXT_COLOR_S      (2)  // 文字影色
#define TEXT_COLOR_B      (0)  // 文字背景色(透明)

static const u8 bmpwin_setup[TEXT_MAX][9] =
{
  // frmnum           posx  posy  sizx  sizy  palnum          dir                    x  y (x,yは無視してセンタリングすることもある)
  {  BG_FRAME_M_TEXT,    0,    0,    1,    1, TEXT_PAL_POS,   GFL_BMP_CHRAREA_GET_F, 0, 0 },
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
  
  // グラフィック、フォントなど
  MANUAL_GRAPHIC_WORK*        graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // テキスト
  GFL_MSGDATA*                msgdata;
  GFL_BMPWIN*                 text_bmpwin[TEXT_MAX];
  BOOL                        text_trans[TEXT_MAX];  // bmpwinの転送の必要がある場合TRUE
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
  }

  // グラフィック、フォントなど
  {
    work->graphic       = MANUAL_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // メインBG
  GFL_BG_SetPriority( BG_FRAME_M_FRONT,  BG_FRAME_PRI_M_FRONT );
  GFL_BG_SetPriority( BG_FRAME_M_TEXT,   BG_FRAME_PRI_M_TEXT  );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Manual_VBlankFunc, work, 1 );

  // サブBG
  {
    const MYSTATUS*  mystatus  = GAMEDATA_GetMyStatus( work->param->gamedata );
    APP_NOGEAR_SUBSCREEN_Init();
    APP_NOGEAR_SUBSCREEN_Trans( work->heap_id, mystatus->sex );  // PM_MALE or PM_FEMALE  // include/pm_version.h
  }

  // フェードイン(黒→見える)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );

  // 通信アイコン
  GFL_NET_WirelessIconEasy_HoldLCD( FALSE, work->heap_id );
  GFL_NET_ReloadIcon();

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Manual_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MANUAL_WORK* work = (MANUAL_WORK*)mywk;

  // 通信アイコン
  GFL_NET_WirelessIconEasy_DefaultLCD();

  // サブBG
  APP_NOGEAR_SUBSCREEN_Exit();

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Clear( work->print_que );
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    MANUAL_GRAPHIC_Exit( work->graphic );
  }

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
      }
      else if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
        b_end = TRUE;
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

  PRINTSYS_QUE_Main( work->print_que );

  // 2D描画
  MANUAL_GRAPHIC_2D_Draw( work->graphic );

  // 3D描画
  //MANUAL_GRAPHIC_3D_StartDraw( work->graphic );
  //MANUAL_GRAPHIC_3D_EndDraw( work->graphic );

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

