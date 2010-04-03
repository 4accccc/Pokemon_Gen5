//============================================================================
/**
 *  @file   d_test.c
 *  @brief  テスト
 *  @author Koji Kawada
 *  @data   2010.03.31
 *  @note   
 *  モジュール名：D_KAWADA_TEST, D_TEST
 */
//============================================================================
#define DEBUG_KAWADA  // これが定義されているとき、デバッグ出力があったりする


#define HEAPID_D_TEST (HEAPID_SHINKA_DEMO)


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "print/gf_font.h"
#include "print/printsys.h"


// バトルの下画面
#include "field/zonedata.h"
#include "tr_tool/trtype_def.h"
#include "battle/btlv/btlv_mcss.h"
#include "battle/btlv/btlv_input.h"
#include "battle/btlv/btlv_effect.h"


#include "d_test_graphic.h"
#include "d_test.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_yesnomenu.h"

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0xc0000)               ///< ヒープサイズ

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


  // バトルの下画面
  SEQ_BTLV,
 

  SEQ_FADE_OUT,
  SEQ_END,
};

// テキスト
enum
{
  TEXT_DUMMY,
  TEXT_BLACK,
  TEXT_NAME,
  TEXT_MAIN,
  TEXT_STAFF,
  TEXT_MAX,
};

// BG_PAL_POS_M_TEXTの割り当て
#define TEXT_PAL_POS      (BG_PAL_POS_M_TEXT)
#define TEXT_COLOR_L      (1)  // 文字主色
#define TEXT_COLOR_S      (2)  // 文字影色
#define TEXT_COLOR_B      (0)  // 文字背景色(透明)
#define TEXT_COLOR_BLACK  (3)  // スクロールする際の空き領域を黒にしておく

// 文字の色
#define TEXT_COLOR_VALUE_L   (0x294B)
#define TEXT_COLOR_VALUE_S   (0x4A74)

static const u8 bmpwin_setup[TEXT_MAX][9] =
{
  // frmnum           posx  posy  sizx  sizy  palnum                dir                    x  y (x,yは無視してセンタリングすることもある)
  {  BG_FRAME_M_TEXT,    0,    0,    1,    1, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_M_TEXT,    0,   24,    1,    1, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_M_TEXT,    1,    3,   30,    3, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_M_TEXT,    1,    7,   30,    9, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_M_TEXT,    1,   17,   30,    5, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },
};

// フェード
#define FADE_IN_WAIT  (0)
#define FADE_OUT_WAIT (0)

// 文字数
#define TEXT_NAME_LEN_MAX    (32)  // EOMを含めた文字数

// スクロール
#define SCROLL_START_POS_Y  (64)
#define SCROLL_WAIT         ( 0)  // 0以上の値。0で毎フレーム移動。1で待ち、移動、待ち、移動。2で待ち、待ち、移動。
#define SCROLL_VALUE        ( 2)  // 移動するときの移動量。


// OBJ
enum
{
  OBJ_BALL_RES_NCG,
  OBJ_BALL_RES_NCL,
  OBJ_BALL_RES_NCE,
  OBJ_GF_RES_NCG,
  OBJ_GF_RES_NCL,
  OBJ_GF_RES_NCE,
  OBJ_RES_MAX,
};
// CELL
enum
{
  OBJ_BALL_CELL,
  OBJ_GF_CELL,
  OBJ_CELL_MAX,
};
#define OBJ_BALL_CELL_ANMSEQ  (1)
#define OBJ_GF_CELL_ANMSEQ    (0)
static const GFL_CLWK_DATA obj_cell_data[OBJ_CELL_MAX] =
{
  // pos_x, pos_y, anmseq,               softpri, bgpri
  {   14,    11,   OBJ_BALL_CELL_ANMSEQ, 0,       BG_FRAME_PRI_M_FRONT },
  {  175,   136,   OBJ_GF_CELL_ANMSEQ,   0,       BG_FRAME_PRI_M_FRONT },
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
  // ヒープ、パラメータなど
  HEAPID                      heap_id;
  D_KAWADA_TEST_PARAM*        param;
  
  // グラフィック、フォントなど
  D_TEST_GRAPHIC_WORK*        graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // サブシーケンス
  int                         sub_seq;
  int                         sub_main_step;


  // バトルの下画面
  GFL_MSGDATA*                btlv_msgdata;
  u8                          btlv_cursor_flag;
  BTLV_INPUT_WORK*            btlv_input_work;


}
D_TEST_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void D_Test_VBlankFunc( GFL_TCB* tcb, void* wk );

// バトルの下画面
static void D_Test_BtlvInit( D_TEST_WORK* work );
static void D_Test_BtlvExit( D_TEST_WORK* work );
static void D_Test_BtlvMain( D_TEST_WORK* work );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT D_Test_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT D_Test_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT D_Test_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    D_KAWADA_TEST_ProcData =
{
  D_Test_ProcInit,
  D_Test_ProcMain,
  D_Test_ProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           パラメータ生成
 *
 *  @param[in]       heap_id       ヒープID
 *  @param[in]       dummy
 *
 *  @retval          D_KAWADA_TEST_PARAM
 */
//------------------------------------------------------------------
D_KAWADA_TEST_PARAM*  D_KAWADA_TEST_AllocParam(
                                HEAPID               heap_id,
                                u32                  dummy
                           )
{
  D_KAWADA_TEST_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof( D_KAWADA_TEST_PARAM ) );
  D_KAWADA_TEST_InitParam(
      param,
      dummy );
  return param;
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータ解放
 *
 *  @param[in,out]   param      パラメータ生成で生成したもの
 *
 *  @retval          
 */
//------------------------------------------------------------------
void            D_KAWADA_TEST_FreeParam(
                            D_KAWADA_TEST_PARAM*  param )
{
  GFL_HEAP_FreeMemory( param );
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータを設定する
 *
 *  @param[in,out]   param         D_KAWADA_TEST_PARAM
 *  @param[in]       dummy
 *
 *  @retval          
 */
//------------------------------------------------------------------
void  D_KAWADA_TEST_InitParam(
                  D_KAWADA_TEST_PARAM*      param,
                  u32                       dummy
                         )
{
  param->dummy             = dummy;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT D_Test_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_WORK*     work;

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "D_Test_ProcInit\n" );
  }
#endif

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_D_TEST, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(D_TEST_WORK), HEAPID_D_TEST );
    GFL_STD_MemClear( work, sizeof(D_TEST_WORK) );
    
    work->heap_id       = HEAPID_D_TEST;
    work->param         = (D_KAWADA_TEST_PARAM*)pwk;
  }

  // グラフィック、フォントなど
  {
    work->graphic       = D_TEST_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( D_Test_VBlankFunc, work, 1 );

  // サブシーケンス
  work->sub_seq = 0;

  // フェードイン(黒→見える)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT D_Test_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_WORK*     work     = (D_TEST_WORK*)mywk;

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    D_TEST_GRAPHIC_Exit( work->graphic );
  }

  // ヒープ、パラメータなど
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_D_TEST );
  }

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "D_Test_ProcExit\n" );
  }
#endif

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT D_Test_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_WORK*     work     = (D_TEST_WORK*)mywk;

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
      if( work->sub_seq == 0 )
      {
        *seq = SEQ_BTLV;
        work->sub_seq = 0;
      }
      else
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


  // バトルの下画面
  case SEQ_BTLV:
    {
      switch(work->sub_seq)
      {
      case 0:
        {
          D_Test_BtlvInit( work );
          work->sub_seq = 1;
        }
        break;
      case 1:
        {
          D_Test_BtlvMain( work );
          if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
          {
            GFL_UI_SetTouchOrKey( GFL_APP_KTST_KEY );
            work->sub_seq = 2;
          } 
        }
        break;
      case 2:
        {
          D_Test_BtlvExit( work );
          *seq = SEQ_MAIN;
          work->sub_seq = 1;
        }
        break;
      }
    }
    break;


  // 
//  case SEQ_:
//    {
//    }
//    break;

  }

  PRINTSYS_QUE_Main( work->print_que );

  // 2D描画
  D_TEST_GRAPHIC_2D_Draw( work->graphic );

  // 3D描画
  //D_TEST_GRAPHIC_3D_StartDraw( work->graphic );
  //D_TEST_GRAPHIC_3D_EndDraw( work->graphic );

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
static void D_Test_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  D_TEST_WORK* work = (D_TEST_WORK*)wk;
}


//-------------------------------------
/// バトルの下画面
//=====================================
FS_EXTERN_OVERLAY(battle_view);
#include "../../battle/btlv/data/yesno_sel.cdat"
static void D_Test_BtlvInit( D_TEST_WORK* work )
{
#ifdef DEBUG_KAWADA
  {
    OS_Printf( "D_Test_BtlvInit\n" );
  }
#endif

  ZONEDATA_Open( work->heap_id );
  work->btlv_msgdata = GFL_MSG_Create(
                           GFL_MSG_LOAD_NORMAL,
                           ARCID_MESSAGE,
                           NARC_message_yesnomenu_dat,
                           work->heap_id );
  work->btlv_cursor_flag = ( GFL_UI_CheckTouchOrKey() == GFL_APP_KTST_KEY ) ? ( 1 ) : ( 0 );

  GFL_OVERLAY_Load(FS_OVERLAY_ID(battle_view));
  {
    BTL_FIELD_SITUATION bfs = { 
      0, 0, 0, 0, 0
    };
    u16 tr_type[] = { 
      TRTYPE_HERO, TRTYPE_HERO, 0xffff, 0xffff,
    };
    BTLV_EFFECT_SETUP_PARAM* besp = BTLV_EFFECT_MakeSetUpParam( BTL_RULE_SINGLE, &bfs, FALSE, tr_type, NULL, work->heap_id );
    BTLV_EFFECT_Init( besp, work->font, work->heap_id );
    GFL_HEAP_FreeMemory( besp );
  }
  work->btlv_input_work = BTLV_INPUT_InitEx( BTLV_INPUT_TYPE_SINGLE, BTLV_EFFECT_GetPfd(), work->font, &work->btlv_cursor_flag, work->heap_id );

  work->sub_main_step = 0;
}
static void D_Test_BtlvExit( D_TEST_WORK* work )
{
  BTLV_INPUT_Exit( work->btlv_input_work );
  BTLV_EFFECT_Exit();
  GFL_OVERLAY_Unload(FS_OVERLAY_ID(battle_view));
 
  GFL_UI_SetTouchOrKey( ( work->btlv_cursor_flag ) ? ( GFL_APP_KTST_KEY ) : ( GFL_APP_KTST_TOUCH ) );
  GFL_MSG_Delete( work->btlv_msgdata );
  ZONEDATA_Close();

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "D_Test_BtlvExit\n" );
  }
#endif
}
static void D_Test_BtlvMain( D_TEST_WORK* work )
{
  if( work->sub_main_step == 0 )
  {
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L )
    {
      BTLV_INPUT_YESNO_PARAM param;
      param.yes_msg = GFL_MSG_CreateString( work->btlv_msgdata, msgid_yesno_yes );
      param.no_msg = GFL_MSG_CreateString( work->btlv_msgdata, msgid_yesno_no );
      BTLV_INPUT_CreateScreen( work->btlv_input_work, BTLV_INPUT_SCRTYPE_YES_NO, &param );
      GFL_STR_DeleteBuffer( param.yes_msg );
      GFL_STR_DeleteBuffer( param.no_msg );
      work->sub_main_step++;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
    {
      BTLV_INPUT_CreateScreen( work->btlv_input_work, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
    }
  }
  else if( work->sub_main_step == 1 )
  {
    int input = BTLV_INPUT_CheckInput( work->btlv_input_work, &YesNoTouchData, YesNoKeyData );
    if( input != GFL_UI_TP_HIT_NONE )
    {
      BTLV_EFFECT_Stop();  // BTLV_INPUT_CheckInputの結果がGFL_UI_TP_HIT_NONEのとき、BTLV_EFFECT_Addしているので。
                           // BTLV_EFFECT_Mainを回していれば解放されるが、回していないときは解放されずに残ってしまう。
      BTLV_INPUT_CreateScreen( work->btlv_input_work, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
      work->sub_main_step = 0;
    }
  }

  //BTLV_EFFECT_Main();  // Initは必要だが、Mainはなくても大丈夫みたい。Mainを回すと上画面におぼんが表示されてしまう。
  BTLV_INPUT_Main( work->btlv_input_work );
}


//-------------------------------------
/// 
//=====================================


