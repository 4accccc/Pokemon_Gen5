//============================================================================
/**
 *  @file   egg_demo.c
 *  @brief  タマゴ孵化デモ
 *  @author Koji Kawada
 *  @data   2010.01.25
 *  @note   
 *  モジュール名：EGG_DEMO
 */
//============================================================================
#define HEAPID_EGG_DEMO (HEAPID_SHINKA_DEMO)


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "gamesystem/msgspeed.h"
#include "system/palanm.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menu.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "poke_tool/poke_tool.h"
#include "sound/pm_sndsys.h"
#include "app/name_input.h"

#include "egg_demo_graphic.h"
#include "egg_demo_view.h"
#include "demo/egg_demo.h"


// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "script_message.naix"
#include "msg/script/msg_egg_event.h"
// サウンド
#include "sound/wb_sound_data.sadl"


// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x50000)               ///< ヒープサイズ

// BGフレーム
#define BG_FRAME_M_BACK              (GFL_BG_FRAME2_M)        // プライオリティ2
#define BG_FRAME_M_POKEMON           (GFL_BG_FRAME0_M)        // プライオリティ1
#define BG_FRAME_M_TEXT              (GFL_BG_FRAME1_M)        // プライオリティ0

#define BG_FRAME_S_BACK              (GFL_BG_FRAME0_S)        // プライオリティ0

// BGパレット
// 本数
enum
{
  BG_PAL_NUM_M_BACKDROP      = 1,
  BG_PAL_NUM_M_TEXT_FONT     = 1,
  BG_PAL_NUM_M_TEXT_FRAME    = 1,
  BG_PAL_NUM_M_TM            = 1,  // 使用せず
  BG_PAL_NUM_M_BACK          = 1,
};
// 位置
enum
{
  BG_PAL_POS_M_BACKDROP      = 0,                                                         // 0
  BG_PAL_POS_M_TEXT_FONT     = BG_PAL_POS_M_BACKDROP    + BG_PAL_NUM_M_BACKDROP        ,  // 1
  BG_PAL_POS_M_TEXT_FRAME    = BG_PAL_POS_M_TEXT_FONT   + BG_PAL_NUM_M_TEXT_FONT       ,  // 2
  BG_PAL_POS_M_TM            = BG_PAL_POS_M_TEXT_FRAME  + BG_PAL_NUM_M_TEXT_FRAME      ,  // 3  // 使用せず
  BG_PAL_POS_M_BACK          = BG_PAL_POS_M_TM          + BG_PAL_NUM_M_TM              ,  // 4
  BG_PAL_POS_M_MAX           = BG_PAL_POS_M_BACK        + BG_PAL_NUM_M_BACK            ,  // 5  // ここから空き
};
// 本数
enum
{
  BG_PAL_NUM_S_BACK          = 1,
};
// 位置
enum
{
  BG_PAL_POS_S_BACK          = 0                                                      ,  // 0
  BG_PAL_POS_S_MAX           = BG_PAL_POS_S_BACK        + BG_PAL_NUM_S_BACK           ,  // 1  // ここから空き
};

// OBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_             = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_M_MAX          = 0                                                      ,       // ここから空き
};
// 本数
enum
{
  OBJ_PAL_NUM_S_             = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_S_MAX          = 0                                                      ,      // ここから空き
};

// TEXT
#define TEXT_WININ_BACK_COLOR        (15)

// フェード
#define FADE_IN_WAIT           (2)         ///< フェードインのスピード
#define FADE_OUT_WAIT          (2)         ///< フェードアウトのスピード

#define NAMEIN_FADE_OUT_WAIT   (0)         ///< 名前入力へ移行するためのフェードアウトのスピード

// 白く飛ばす演出のためのパレットフェード
#define PFADE_TCBSYS_TASK_MAX  (8)
#define PFADE_WAIT_TO_WHITE    (2)
#define PFADE_WAIT_FROM_WHITE  (4)

// 上下に黒帯を表示するためのwnd
#define WND_UP_Y_APPEAR   ( 20)
#define WND_DOWN_Y_APPEAR (130)

// はい・いいえウィンドウ
typedef enum
{
  TM_RESULT_NO,
  TM_RESULT_YES,
  TM_RESULT_SEL,
}
TM_RESULT;

// 文字数
#define STRBUF_LENGTH                (256)  // この文字数で足りるかbuflen.hで要確認
#define NAMEIN_STRBUF_LENGTH         ( 32)  // この文字数で足りるかbuflen.hで要確認

// 根幹ステップ
typedef enum
{
  TRUNK_STEP_FADE_IN,
  TRUNK_STEP_SOUND_INTRO,
  TRUNK_STEP_DEMO_EGG,
  TRUNK_STEP_DEMO_TO_WHITE,
  TRUNK_STEP_DEMO_BRIGHT_TO_WHITE,
  TRUNK_STEP_DEMO_WHITE,
  TRUNK_STEP_DEMO_BRIGHT_WHITE,
  TRUNK_STEP_DEMO_MON,
  TRUNK_STEP_TEXT_0,
  TRUNK_STEP_TEXT_1,
  TRUNK_STEP_TM,
  TRUNK_STEP_NAMEIN_FADE_OUT,
  TRUNK_STEP_NAMEIN,
  TRUNK_STEP_NAMEIN_BLACK,
  TRUNK_STEP_NAMEIN_END,
  TRUNK_STEP_FADE_OUT,
  TRUNK_STEP_END,
}
TRUNK_STEP;

// サウンドステップ
typedef enum
{
  SOUND_STEP_WAIT,
  SOUND_STEP_INTRO,
  SOUND_STEP_HATCH,
  SOUND_STEP_CONGRATULATE,
}
SOUND_STEP;

// TEXTのステップ
typedef enum
{
  TEXT_STEP_WAIT,
}
TEXT_STEP;

// はい・いいえウィンドウのステップ
typedef enum
{
  TM_STEP_WAIT,
  TM_STEP_SELECT,
  TM_STEP_END,
}
TM_STEP;

// 白く飛ばす演出のためのパレットフェードのステップ
typedef enum
{
  PFADE_STEP_WAIT,
}
PFADE_STEP;

// 上下に黒帯を表示するためのwndのステップ
typedef enum
{
  WND_STEP_WAIT,
}
WND_STEP;


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
  // グラフィック、フォントなど
  HEAPID                      heap_id;
  EGG_DEMO_GRAPHIC_WORK*      graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // ステップ
  TRUNK_STEP                  trunk_step;
  SOUND_STEP                  sound_step;
  TEXT_STEP                   text_step;
  TM_STEP                     tm_step;
  PFADE_STEP                  pfade_step;
  WND_STEP                    wnd_step;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // 単一色、単一キャラのBG
  GFL_ARCUTIL_TRANSINFO       sim_transinfo;  // ARCUTILは使用していないが、位置とサイズをひとまとめにしたかったので。

  // TEXT
  PRINT_STREAM*               text_stream;
  GFL_TCBLSYS*                text_tcblsys;
  GFL_BMPWIN*                 text_winin_bmpwin;
  GFL_BMPWIN*                 text_dummy_bmpwin;      ///< 0番のキャラクターを1x1でつくっておく
  GFL_ARCUTIL_TRANSINFO       text_winfrm_tinfo;
  GFL_MSGDATA*                text_msgdata_egg;
  STRBUF*                     text_strbuf;

  // はい・いいえウィンドウ
  BMPWIN_YESNO_DAT            tm_dat;
  BMPMENU_WORK*               tm_wk;
  TM_RESULT                   tm_result;

  // 白く飛ばす演出のためのパレットフェード
  GFL_TCBSYS*                 pfade_tcbsys;
  void*                       pfade_tcbsys_wk;
  PALETTE_FADE_PTR            pfade_ptr;

  // 上下に黒帯を表示するためのwnd
  u8                          wnd_up_y;    // wnd_up_y <= 見えるピクセル < wnd_down_y
  u8                          wnd_down_y;  //        0 <= 見えるピクセル < 192
  s16                         wnd_appear_speed;    // 1回の移動で何ピクセル移動するか(0=移動しない、+=黒帯が見えてくる、-=黒帯がはけていく)
  u8                          wnd_appear_wait;     // 1回の移動をするのに何フレーム待つか(0=毎フレーム移動、1=1フレームおきに移動、2=3フレームに1回移動)
  u8                          wnd_count;

  // タマゴ孵化デモの演出
  EGG_DEMO_VIEW_WORK*         view;

  // 名前入力
  NAMEIN_PARAM*               namein_param;
}
EGG_DEMO_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Egg_Demo_VBlankFunc( GFL_TCB* tcb, void* wk );

// 初期化処理、終了処理(PROC変更前後に行うことができるもの)
static void Egg_Demo_Init( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_Exit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// サウンド
static void Egg_Demo_SoundInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPlayIntro( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_SoundCheckPlayIntro( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPlayHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPushHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPopHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPlayCongratulate( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_SoundCheckPlayCongratulate( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundFadeOutHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_SoundCheckFadeOutHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// 単一色、単一キャラのBG
static void Egg_Demo_CreateSimpleBG( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_DeleteSimpleBG( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// TEXT
static void Egg_Demo_TextInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TextExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TextMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TextShowWindow( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TextMakeStream( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work,
                                     u32 str_id );
static BOOL Egg_Demo_TextWaitStream( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// はい・いいえウィンドウ
static void Egg_Demo_TmInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TmExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TmMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TmStartSelect(  EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static TM_RESULT Egg_Demo_TmGetResult( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// 白く飛ばす演出のためのパレットフェード
static void Egg_Demo_PFadeZero( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_PFadeInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );  // パレットを書き換えるので、全パレットの用意が済んでから呼ぶこと
static void Egg_Demo_PFadeExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_PFadeMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_PFadeToWhite( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_PFadeFromWhite( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_PFadeCheck( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );  // TRUE処理中、FALSE終了

// 上下に黒帯を表示するためのwnd
static void Egg_Demo_WndInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_WndExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_WndMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_WndAppear( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_WndDisappear( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Egg_Demo_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Egg_Demo_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Egg_Demo_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    EGG_DEMO_ProcData =
{
  Egg_Demo_ProcInit,
  Egg_Demo_ProcMain,
  Egg_Demo_ProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief       タマゴ孵化デモ用パラメータ生成
 *
 *  @param[in]   heap_id       ヒープID
 *  @param[in]   gamedata      GAMEDATA
 *  @param[in]   pp            POKEMON_PARAM
 *
 *  @retval      EGG_DEMO_PARAM
 */
//------------------------------------------------------------------
EGG_DEMO_PARAM*  EGG_DEMO_AllocParam(
                            HEAPID           heap_id,
                            GAMEDATA*        gamedata,
                            POKEMON_PARAM*   pp )
{
  EGG_DEMO_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof( EGG_DEMO_PARAM ) );
  EGG_DEMO_InitParam( param, gamedata, pp );
  return param;
}

//------------------------------------------------------------------
/**
 *  @brief           タマゴ孵化デモ用パラメータ解放
 *
 *  @param[in,out]   param      EGG_DEMO_AllocParamで生成したもの
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             EGG_DEMO_FreeParam(
                            EGG_DEMO_PARAM*  param )
{
  GFL_HEAP_FreeMemory( param );
}

//------------------------------------------------------------------
/**
 *  @brief           タマゴ孵化デモ用パラメータを設定する
 *
 *  @param[in,out]   param      EGG_DEMO_PARAM
 *  @param[in]       gamedata   GAMEDATA
 *  @param[in]       pp         POKEMON_PARAM
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             EGG_DEMO_InitParam(
                            EGG_DEMO_PARAM*  param,
                            GAMEDATA*        gamedata,
                            POKEMON_PARAM*   pp )
{
  param->gamedata    = gamedata;
  param->pp          = pp;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT Egg_Demo_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  EGG_DEMO_PARAM*    param    = (EGG_DEMO_PARAM*)pwk;
  EGG_DEMO_WORK*     work;

  // ヒープ
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_EGG_DEMO, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(EGG_DEMO_WORK), HEAPID_EGG_DEMO );
    GFL_STD_MemClear( work, sizeof(EGG_DEMO_WORK) );
    
    work->heap_id       = HEAPID_EGG_DEMO;
  }

  // ステップ
  {
    work->trunk_step    = TRUNK_STEP_FADE_IN;
    work->sound_step    = SOUND_STEP_WAIT;
    work->text_step     = TEXT_STEP_WAIT;
    work->tm_step       = TM_STEP_WAIT;
    work->pfade_step    = PFADE_STEP_WAIT;
    work->wnd_step      = WND_STEP_WAIT;
  }

  Egg_Demo_Init( param, work );

  // サウンド
  Egg_Demo_SoundInit( param, work );
  
  // フェードイン(黒→見える)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Egg_Demo_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  EGG_DEMO_PARAM*    param    = (EGG_DEMO_PARAM*)pwk;
  EGG_DEMO_WORK*     work     = (EGG_DEMO_WORK*)mywk;

  // サウンド
  Egg_Demo_SoundExit( param, work );

  if( work->trunk_step != TRUNK_STEP_NAMEIN_END )
    Egg_Demo_Exit( param, work );

  // ヒープ
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_EGG_DEMO );
  }

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT Egg_Demo_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  EGG_DEMO_PARAM*    param    = (EGG_DEMO_PARAM*)pwk;
  EGG_DEMO_WORK*     work     = (EGG_DEMO_WORK*)mywk;

  int key_trg = GFL_UI_KEY_GetTrg();

  switch( work->trunk_step )
  {
  case TRUNK_STEP_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_SOUND_INTRO;

        Egg_Demo_SoundPlayIntro( param, work );
      }
    }
    break;
  case TRUNK_STEP_SOUND_INTRO:
    {
      if( !Egg_Demo_SoundCheckPlayIntro( param, work ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_EGG;

        // タマゴ孵化デモの演出
        EGG_DEMO_VIEW_Start( work->view );
        
        Egg_Demo_SoundPlayHatch( param, work );
      }
    }
    break;
  case TRUNK_STEP_DEMO_EGG:
    {
      // タマゴ孵化デモの演出
      if( EGG_DEMO_VIEW_White( work->view ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_TO_WHITE;

        Egg_Demo_WndDisappear( param, work );
        Egg_Demo_PFadeToWhite( param, work );

        Egg_Demo_SoundPushHatch( param, work );
      }
    }
    break;
  case TRUNK_STEP_DEMO_TO_WHITE:
    {
      if( !Egg_Demo_PFadeCheck( param, work ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_BRIGHT_TO_WHITE;

        // フェードアウト(見える→白)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 16, 0 );
      }
    }
    break;
  case TRUNK_STEP_DEMO_BRIGHT_TO_WHITE:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_WHITE;

        Egg_Demo_TextShowWindow( param, work );
        
        // タマゴ孵化
        PP_Put( param->pp, ID_PARA_tamago_flag, 0 );

        // タマゴ孵化デモの演出
        EGG_DEMO_VIEW_Hatch( work->view, param->pp );
      }
    }
    break;
  case TRUNK_STEP_DEMO_WHITE:
    {
      // タマゴ孵化デモの演出
      if( EGG_DEMO_VIEW_IsReady( work->view ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_BRIGHT_WHITE;

        Egg_Demo_PFadeFromWhite( param, work );
      }
    }
    break;
  case TRUNK_STEP_DEMO_BRIGHT_WHITE:
    {
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_MON;

        // フェードイン(白→見える)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, PFADE_WAIT_FROM_WHITE );
        
        // タマゴ孵化デモの演出
        EGG_DEMO_VIEW_MonStart( work->view );
      }
    }
    break;
  case TRUNK_STEP_DEMO_MON:
    {
      // タマゴ孵化デモの演出
      if( EGG_DEMO_VIEW_IsEnd( work->view ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_TEXT_0;

        Egg_Demo_TextMakeStream( param, work, egg_evemt_02 );
        
        Egg_Demo_SoundPlayCongratulate( param, work );
      }
    }
    break;
  case TRUNK_STEP_TEXT_0:
    {
      if(    Egg_Demo_TextWaitStream( param, work )
          && (!Egg_Demo_SoundCheckPlayCongratulate( param, work )) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_TEXT_1;
        
        Egg_Demo_TextMakeStream( param, work, egg_evemt_03 );
      }
    }
  break;
  case TRUNK_STEP_TEXT_1:
    {
      if( Egg_Demo_TextWaitStream( param, work ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_TM;

        Egg_Demo_TmStartSelect( param, work );
      }
    }
    break;
  case TRUNK_STEP_TM:
    {
      TM_RESULT tm_result = Egg_Demo_TmGetResult( param, work );
      if( tm_result == TM_RESULT_NO )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_FADE_OUT;

        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );

        Egg_Demo_SoundFadeOutHatch( param, work );
      }
      else if( tm_result == TM_RESULT_YES )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_NAMEIN_FADE_OUT;

        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, NAMEIN_FADE_OUT_WAIT );
      }
    }
    break;
  case TRUNK_STEP_NAMEIN_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        Egg_Demo_Exit( param, work );

        // 次へ
        work->trunk_step = TRUNK_STEP_NAMEIN;

        // 名前入力へ行く
        GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
        work->namein_param = NAMEIN_AllocParamPokemonByPP( work->heap_id, param->pp, NAMEIN_POKEMON_LENGTH, NULL );
        GFL_PROC_SysCallProc( NO_OVERLAY_ID, &NameInputProcData, work->namein_param );
      }
    }
    break;
  case TRUNK_STEP_NAMEIN:
    {
      // 名前入力から戻る
      if( !NAMEIN_IsCancel( work->namein_param ) )
      {
        STRBUF* strbuf = GFL_STR_CreateBuffer( NAMEIN_STRBUF_LENGTH, work->heap_id );
        NAMEIN_CopyStr( work->namein_param, strbuf );
        PP_Put( param->pp, ID_PARA_nickname, (u32)strbuf );
        GFL_STR_DeleteBuffer( strbuf );
      }
      NAMEIN_FreeParam( work->namein_param );
      GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );

      // 次へ
      work->trunk_step = TRUNK_STEP_NAMEIN_BLACK;
     
      Egg_Demo_SoundFadeOutHatch( param, work );
    }
    break;
  case TRUNK_STEP_NAMEIN_BLACK:
    {
      if( Egg_Demo_SoundCheckFadeOutHatch( param, work ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_NAMEIN_END;
 
        return GFL_PROC_RES_FINISH;
      }
    }
    break;
  case TRUNK_STEP_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_END;

        return GFL_PROC_RES_FINISH;
      }
    }
    break;
  default:
    break;
  }

  if(    work->trunk_step != TRUNK_STEP_NAMEIN
      && work->trunk_step != TRUNK_STEP_NAMEIN_BLACK
      && work->trunk_step != TRUNK_STEP_NAMEIN_END
      && work->trunk_step != TRUNK_STEP_END )
  {
    // TEXT
    Egg_Demo_TextMain( param, work );
    // はい・いいえウィンドウ
    Egg_Demo_TmMain( param, work );
    // 上下に黒帯を表示するためのwnd
    Egg_Demo_WndMain( param, work );

    // 白く飛ばす演出のためのパレットフェード
    Egg_Demo_PFadeMain( param, work );

    // タマゴ孵化デモの演出
    EGG_DEMO_VIEW_Main( work->view );

    // サウンド
    Egg_Demo_SoundMain( param, work );

    PRINTSYS_QUE_Main( work->print_que );

    // 2D描画
    EGG_DEMO_GRAPHIC_2D_Draw( work->graphic );
    // 3D描画
    EGG_DEMO_GRAPHIC_3D_StartDraw( work->graphic );
    EGG_DEMO_VIEW_Draw( work->view );  // タマゴ孵化デモの演出
    EGG_DEMO_GRAPHIC_3D_EndDraw( work->graphic );
  }

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
static void Egg_Demo_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  EGG_DEMO_WORK* work = (EGG_DEMO_WORK*)wk;

  // 白く飛ばす演出のためのパレットフェード
  if( work->pfade_ptr ) PaletteFadeTrans( work->pfade_ptr );
}

//-------------------------------------
/// 初期化処理、終了処理(PROC変更前後に行うことができるもの)
//=====================================
static void Egg_Demo_Init( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // 何よりも先に行う初期化
  {
    // 非表示
    u8 i;
    for(i=GFL_BG_FRAME0_M; i<=GFL_BG_FRAME3_S; i++)
    {
      GFL_BG_SetVisible( i, VISIBLE_OFF );
    }
  } 

  // グラフィック、フォントなど
  {
    work->graphic       = EGG_DEMO_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // VBlank中TCB
  {
    work->vblank_tcb = GFUser_VIntr_CreateTCB( Egg_Demo_VBlankFunc, work, 1 );
  }

  // 単一色、単一キャラのBG
  Egg_Demo_CreateSimpleBG( param, work );

  // TEXT
  Egg_Demo_TextInit( param, work );
  // はい・いいえウィンドウ
  Egg_Demo_TmInit( param, work );
  // 上下に黒帯を表示するためのwnd
  Egg_Demo_WndInit( param, work );

  // プライオリティ、表示、背景色など
  {
    GFL_BG_SetPriority( BG_FRAME_M_BACK              , 2 );
    GFL_BG_SetPriority( BG_FRAME_M_POKEMON           , 1 );
    GFL_BG_SetPriority( BG_FRAME_M_TEXT              , 0 );  // 最前面

    GFL_BG_SetPriority( BG_FRAME_S_BACK              , 0 );  // 最前面

    GFL_BG_SetVisible( BG_FRAME_M_BACK               , VISIBLE_ON );
    GFL_BG_SetVisible( BG_FRAME_M_POKEMON            , VISIBLE_ON );
    GFL_BG_SetVisible( BG_FRAME_M_TEXT               , VISIBLE_ON );
  
    GFL_BG_SetVisible( BG_FRAME_S_BACK               , VISIBLE_ON );

    // パーティクル対応
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0,
                      GX_BLEND_PLANEMASK_BD | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1,
                      0, 0 );

    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );
    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x0000 );
  }

  // 白く飛ばす演出のためのパレットフェード
  Egg_Demo_PFadeInit( param, work );  // パレットを書き換えるので、全パレットの用意が済んでから呼ぶこと

  // タマゴ孵化デモの演出
  work->view = EGG_DEMO_VIEW_Init( work->heap_id, param->pp );
}
static void Egg_Demo_Exit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // タマゴ孵化デモの演出
  EGG_DEMO_VIEW_Exit( work->view );

  // 白く飛ばす演出のためのパレットフェード
  Egg_Demo_PFadeExit( param, work );

  // 上下に黒帯を表示するためのwnd
  Egg_Demo_WndExit( param, work );
  // はい・いいえウィンドウ
  Egg_Demo_TmExit( param, work );
  // TEXT
  Egg_Demo_TextExit( param, work );

  // 単一色、単一キャラのBG
  Egg_Demo_DeleteSimpleBG( param, work );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    EGG_DEMO_GRAPHIC_Exit( work->graphic );
  }
}

//-------------------------------------
/// サウンド
//=====================================
static void Egg_Demo_SoundInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_PushBGM();
}
static void Egg_Demo_SoundExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_StopBGM();
  PMSND_PopBGM();
}
static void Egg_Demo_SoundMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  switch( work->sound_step )
  {
  case SOUND_STEP_WAIT:
    {
    }
    break;
  case SOUND_STEP_INTRO:
    {
      if( !PMSND_CheckPlayBGM() )
      {
        work->sound_step = SOUND_STEP_WAIT;
      }
    }
    break;
  case SOUND_STEP_HATCH:
    {
    }
    break;
  case SOUND_STEP_CONGRATULATE:
    {
      if( !PMSND_CheckPlayBGM() )
      {
        Egg_Demo_SoundPopHatch( param, work );
        work->sound_step = SOUND_STEP_HATCH;
      }
    }
    break;
  }
}
static void Egg_Demo_SoundPlayIntro( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_PlayBGM(SEQ_BGM_SHINKA);
  work->sound_step = SOUND_STEP_INTRO;
}
static BOOL Egg_Demo_SoundCheckPlayIntro( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_INTRO ) return TRUE;
  else                                       return FALSE;
}
static void Egg_Demo_SoundPlayHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_PlayBGM(SEQ_BGM_KOUKAN);
  work->sound_step = SOUND_STEP_HATCH;
}
static void Egg_Demo_SoundPushHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_PushBGM();
}
static void Egg_Demo_SoundPopHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_PopBGM();
}
static void Egg_Demo_SoundPlayCongratulate( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_PlayBGM(SEQ_ME_SHINKAOME);
  work->sound_step = SOUND_STEP_CONGRATULATE;
}
static BOOL Egg_Demo_SoundCheckPlayCongratulate( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  return ( work->sound_step == SOUND_STEP_CONGRATULATE );
}
static void Egg_Demo_SoundFadeOutHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_FadeOutBGM( 16 * FADE_OUT_WAIT / 2 );
}
static BOOL Egg_Demo_SoundCheckFadeOutHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  return PMSND_CheckFadeOnBGM();
}

//-------------------------------------
/// 単一色、単一キャラのBG
//=====================================
static void Egg_Demo_CreateSimpleBG( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // パレットの作成＆転送
  {
    u16* pal = GFL_HEAP_AllocClearMemory( work->heap_id, sizeof(u16) * 0x10 );
    pal[0x00] = 0x0000;  // 透明
    pal[0x01] = 0x0000;  // 黒
    pal[0x02] = 0x7fff;  // 白
    pal[0x03] = 0x2108;  // 灰

    GFL_BG_LoadPalette( BG_FRAME_M_BACK, pal, 0x20, BG_PAL_POS_M_BACK*0x20 );
    GFL_HEAP_FreeMemory( pal );
  }

  // キャラの作成＆転送
  {
    u32 sim_bmp_pos;
    u32 sim_bmp_size;

    GFL_BMP_DATA* sim_bmp = GFL_BMP_Create( 1, 1, GFL_BMP_16_COLOR, work->heap_id );
    GFL_BMP_Fill( sim_bmp, 0, 0, 8, 8, 0x03 );

    sim_bmp_size = GFL_BMP_GetBmpDataSize(sim_bmp);
    sim_bmp_pos = GFL_BG_LoadCharacterAreaMan(
                    BG_FRAME_M_BACK,
                    GFL_BMP_GetCharacterAdrs(sim_bmp),
                    sim_bmp_size );

    GF_ASSERT_MSG( sim_bmp_pos != AREAMAN_POS_NOTFOUND, "EGG_DEMO : BGキャラ領域が足りませんでした。\n" );  // gflibのarc_util.cの
    GF_ASSERT_MSG( sim_bmp_pos < 0xffff, "EGG_DEMO : BGキャラの位置がよくありません。\n" );                 // _TransVramBgCharacterAreaMan
    GF_ASSERT_MSG( sim_bmp_size < 0xffff, "EGG_DEMO : BGキャラのサイズがよくありません。\n" );              // を参考にした。

    work->sim_transinfo = GFL_ARCUTIL_TRANSINFO_Make( sim_bmp_pos, sim_bmp_size );
    
    GFL_BMP_Delete( sim_bmp );
  }

  // スクリーンの作成＆転送
  {
    u16* sim_scr = GFL_HEAP_AllocClearMemory( work->heap_id, sizeof(u16) * 32*24 );
    u8 i, j;
    u16 h = 0;
    for(i=0; i<32; i++)
    {
      for(j=0; j<24; j++)
      {
        u16 chara_name = GFL_ARCUTIL_TRANSINFO_GetPos(work->sim_transinfo);
        u16 flip_h     = 0;
        u16 flip_v     = 0;
        u16 pal        = BG_PAL_POS_M_BACK;
        sim_scr[h] = ( pal << 12 ) | ( flip_v << 11 ) | ( flip_h << 10 ) | ( chara_name << 0 );
        
        h++;
      }
    }

    GFL_BG_WriteScreen( BG_FRAME_M_BACK, sim_scr, 0, 0, 32, 24 );
    GFL_BG_LoadScreenReq( BG_FRAME_M_BACK );

    GFL_HEAP_FreeMemory( sim_scr );
  }
}
static void Egg_Demo_DeleteSimpleBG( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  GFL_BG_FreeCharacterArea( BG_FRAME_M_BACK,
                            GFL_ARCUTIL_TRANSINFO_GetPos(work->sim_transinfo),
                            GFL_ARCUTIL_TRANSINFO_GetSize(work->sim_transinfo) );
}

//-------------------------------------
/// TEXT
//=====================================
static void Egg_Demo_TextInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // パレット
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
      BG_PAL_POS_M_TEXT_FONT * 0x20, BG_PAL_NUM_M_TEXT_FONT * 0x20, work->heap_id );

  // BGフレームのスクリーンの空いている箇所に何も表示がされないようにしておく
  work->text_dummy_bmpwin = GFL_BMPWIN_Create( BG_FRAME_M_TEXT, 0, 0, 1, 1,
                                BG_PAL_POS_M_TEXT_FONT, GFL_BMP_CHRAREA_GET_F );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_dummy_bmpwin), 0 );
  GFL_BMPWIN_TransVramCharacter(work->text_dummy_bmpwin);

  // ウィンドウ内
  work->text_winin_bmpwin = GFL_BMPWIN_Create( BG_FRAME_M_TEXT, 1, 19, 30, 4,
                                 BG_PAL_POS_M_TEXT_FONT, GFL_BMP_CHRAREA_GET_F );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_winin_bmpwin), TEXT_WININ_BACK_COLOR );
  GFL_BMPWIN_TransVramCharacter(work->text_winin_bmpwin);
    
  // ウィンドウ枠
  work->text_winfrm_tinfo = BmpWinFrame_GraphicSetAreaMan( BG_FRAME_M_TEXT,
                                BG_PAL_POS_M_TEXT_FRAME,
                                MENU_TYPE_SYSTEM, work->heap_id );
  
  // ウィンドウ内、ウィンドウ枠どちらの表示もまだ行わないので、初期化ではここまで。

  // メッセージ
  work->text_msgdata_egg     = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_egg_event_dat, work->heap_id );

  // TCBL、フォントカラー、転送など
  work->text_tcblsys = GFL_TCBL_Init( work->heap_id, work->heap_id, 1, 0 );
  GFL_FONTSYS_SetColor( 1, 2, TEXT_WININ_BACK_COLOR );
  GFL_BG_LoadScreenV_Req( BG_FRAME_M_TEXT );

  // NULL初期化
  work->text_stream       = NULL;
  work->text_strbuf       = NULL;
}
static void Egg_Demo_TextExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  if( work->text_stream ) PRINTSYS_PrintStreamDelete( work->text_stream );
  if( work->text_strbuf ) GFL_STR_DeleteBuffer( work->text_strbuf );
  GFL_TCBL_Exit( work->text_tcblsys );
  GFL_MSG_Delete( work->text_msgdata_egg );
  GFL_BG_FreeCharacterArea( BG_FRAME_M_TEXT,
      GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
      GFL_ARCUTIL_TRANSINFO_GetSize(work->text_winfrm_tinfo) );
  GFL_BMPWIN_Delete( work->text_winin_bmpwin );
  GFL_BMPWIN_Delete( work->text_dummy_bmpwin );
}
static void Egg_Demo_TextMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  switch( work->text_step )
  {
  case TEXT_STEP_WAIT:
    {
    }
    break;
  }

  GFL_TCBL_Main( work->text_tcblsys );
}
static void Egg_Demo_TextShowWindow( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  BmpWinFrame_Write( work->text_winin_bmpwin, WINDOW_TRANS_ON_V,
      GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
      BG_PAL_POS_M_TEXT_FRAME );
  GFL_BMPWIN_MakeTransWindow_VBlank( work->text_winin_bmpwin );
}
static void Egg_Demo_TextMakeStream( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work,
                                     u32 str_id )
{
  // 一旦消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_winin_bmpwin), TEXT_WININ_BACK_COLOR );
  
  // 前のを消す
  if( work->text_stream ) PRINTSYS_PrintStreamDelete( work->text_stream );  // この関数でタスクも削除してくれるので、同時に動くタスクは1つで済む
  if( work->text_strbuf ) GFL_STR_DeleteBuffer( work->text_strbuf );

  // 文字列作成
  {
    STRBUF*   src_strbuf  = GFL_MSG_CreateString( work->text_msgdata_egg, str_id );
    WORDSET*  wordset     = WORDSET_Create( work->heap_id );
    WORDSET_RegisterPokeMonsName( wordset, 0, param->pp );
    work->text_strbuf = GFL_STR_CreateBuffer( STRBUF_LENGTH, work->heap_id );
    WORDSET_ExpandStr( wordset, work->text_strbuf, src_strbuf );
    WORDSET_Delete( wordset );
    GFL_STR_DeleteBuffer( src_strbuf );
  }
  
  // ストリーム開始
  work->text_stream = PRINTSYS_PrintStream(
                          work->text_winin_bmpwin,
                          0, 0,
                          work->text_strbuf,
                          work->font, MSGSPEED_GetWait(),
                          work->text_tcblsys, 2,
                          work->heap_id,
                          TEXT_WININ_BACK_COLOR );
}
static BOOL Egg_Demo_TextWaitStream( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  BOOL ret = FALSE;

  switch( PRINTSYS_PrintStreamGetState( work->text_stream ) )
  { 
  case PRINTSTREAM_STATE_PAUSE:
    if( ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ) || GFL_UI_TP_GetTrg() )
    { 
      PRINTSYS_PrintStreamReleasePause( work->text_stream );
    }
    break;
  case PRINTSTREAM_STATE_DONE:
    {
      ret = TRUE;
    }
    break;
  }
  
  return ret;
}

//-------------------------------------
/// はい・いいえウィンドウ
//=====================================
static void Egg_Demo_TmInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // この関数はEgg_Demo_TextInitが済んで
  // BG_PAL_POS_M_TEXT_FONTが設定されている状態で呼ぶこと
  // GFL_FONTSYS_SetColorの設定も必要かもしれない

  u32 size = GFL_BG_1CHRDATASIZ * ( 7*4 +12 );  // system/bmp_menu.cのBmpMenu_YesNoSelectInitを参考にしたサイズ + 少し大きめに
  u32 pos = GFL_BG_AllocCharacterArea( BG_FRAME_M_TEXT, size, GFL_BG_CHRAREA_GET_B );
  GF_ASSERT_MSG( pos != AREAMAN_POS_NOTFOUND, "EGG_DEMO : BGキャラ領域が足りませんでした。\n" );
  GFL_BG_FreeCharacterArea( BG_FRAME_M_TEXT, pos, size );

  work->tm_dat.frmnum    = BG_FRAME_M_TEXT;
  work->tm_dat.pos_x     = 24;
  work->tm_dat.pos_y     = 13;
  work->tm_dat.palnum    = BG_PAL_POS_M_TEXT_FONT;//BG_PAL_POS_M_TM;
  work->tm_dat.chrnum    = pos;  // 使われていないようだ

  work->tm_result = TM_RESULT_SEL;
}
static void Egg_Demo_TmExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // BmpMenu_YesNoSelectMainが解放してくれるので、何もしなくてよい
}
static void Egg_Demo_TmMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  switch( work->tm_step )
  {
  case TM_STEP_WAIT:
    {
    }
    break;
  case TM_STEP_SELECT:
    {
      u32 ret = BmpMenu_YesNoSelectMain( work->tm_wk );
      if( ret != BMPMENU_NULL )
      {
        if( ret == 0 )
          work->tm_result = TM_RESULT_YES;
        else //if( ret == BMPMENU_CANCEL )
          work->tm_result = TM_RESULT_NO;

        work->tm_step = TM_STEP_END;
      }
    }
    break;
  }
}
static void Egg_Demo_TmStartSelect(  EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // この関数はEgg_Demo_TextInitが済んで
  // work->text_winfrm_tinfoが設定されている状態で呼ぶこと

  work->tm_wk = BmpMenu_YesNoSelectInit(
                    &work->tm_dat,
                    GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
                    BG_PAL_POS_M_TEXT_FRAME,
                    0,
                    work->heap_id );

  work->tm_result = TM_RESULT_SEL;
  work->tm_step = TM_STEP_SELECT;
}
static TM_RESULT Egg_Demo_TmGetResult( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  return work->tm_result;
}

//-------------------------------------
/// 白く飛ばす演出のためのパレットフェード
//=====================================
static void Egg_Demo_PFadeZero( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  work->pfade_tcbsys       = NULL;
  work->pfade_tcbsys_wk    = NULL;
  work->pfade_ptr          = NULL;
}
static void Egg_Demo_PFadeInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )  // パレットを書き換えるので、全パレットの用意が済んでから呼ぶこと
{
  Egg_Demo_PFadeZero( param, work );

  // タスク
  work->pfade_tcbsys_wk = GFL_HEAP_AllocClearMemory( work->heap_id, GFL_TCB_CalcSystemWorkSize(PFADE_TCBSYS_TASK_MAX) );
  work->pfade_tcbsys = GFL_TCB_Init( PFADE_TCBSYS_TASK_MAX, work->pfade_tcbsys_wk );

  // パレットフェード
  work->pfade_ptr = PaletteFadeInit( work->heap_id );
  PaletteTrans_AutoSet( work->pfade_ptr, TRUE );
  PaletteFadeWorkAllocSet( work->pfade_ptr, FADE_MAIN_BG, 0x1e0, work->heap_id );
  PaletteFadeWorkAllocSet( work->pfade_ptr, FADE_MAIN_OBJ, 0x1e0, work->heap_id );

  // 現在VRAMにあるパレットを壊さないように、VRAMからパレット内容をコピーする
  PaletteWorkSet_VramCopy( work->pfade_ptr, FADE_MAIN_BG, 0, 0x1e0 );
  PaletteWorkSet_VramCopy( work->pfade_ptr, FADE_MAIN_OBJ, 0, 0x1e0 );
}
static void Egg_Demo_PFadeExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // パレットフェード
  PaletteFadeWorkAllocFree( work->pfade_ptr, FADE_MAIN_BG );
  PaletteFadeWorkAllocFree( work->pfade_ptr, FADE_MAIN_OBJ );
  PaletteFadeFree( work->pfade_ptr );

  // タスク
  GFL_TCB_Exit( work->pfade_tcbsys );
  GFL_HEAP_FreeMemory( work->pfade_tcbsys_wk );

  Egg_Demo_PFadeZero( param, work );
}
static void Egg_Demo_PFadeMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // タスク
  if( work->pfade_tcbsys ) GFL_TCB_Main( work->pfade_tcbsys );

  // ステップ
  switch( work->pfade_step )
  {
  case PFADE_STEP_WAIT:
    {
    }
    break;
  }
}
static void Egg_Demo_PFadeToWhite( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // 白く飛ばす
  u16 req_bit = (1<<BG_PAL_POS_M_BACKDROP) | (1<<BG_PAL_POS_M_BACK);
  PaletteFadeReq(
    work->pfade_ptr, PF_BIT_MAIN_BG, req_bit, PFADE_WAIT_TO_WHITE, 0, 16, 0x7fff, work->pfade_tcbsys
  );
}
static void Egg_Demo_PFadeFromWhite( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // 白から戻る
  u16 req_bit = (1<<BG_PAL_POS_M_BACKDROP) | (1<<BG_PAL_POS_M_BACK);
//  PaletteFadeReq(
//    work->pfade_ptr, PF_BIT_MAIN_BG, req_bit, PFADE_WAIT_FROM_WHITE, 16, 0, 0x7fff, work->pfade_tcbsys
//  );
  PaletteFadeReq(
    work->pfade_ptr, PF_BIT_MAIN_BG, req_bit, 0, 0, 0, 0x7fff, work->pfade_tcbsys
  );
}
static BOOL Egg_Demo_PFadeCheck( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )  // TRUE処理中、FALSE終了
{
  if( PaletteFadeCheck(work->pfade_ptr) == 0 )
    return FALSE;
  else
    return TRUE;
}
 

//-------------------------------------
/// 上下に黒帯を表示するためのwnd
//=====================================
static void Egg_Demo_WndInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  work->wnd_up_y     = WND_UP_Y_APPEAR;
  work->wnd_down_y   = WND_DOWN_Y_APPEAR;
  //work->wnd_up_y     = 0;
  //work->wnd_down_y   = 192;

  GX_SetVisibleWnd( GX_WNDMASK_W0 | GX_WNDMASK_W1 );

  G2_SetWnd0Position(   0, work->wnd_up_y,      128, work->wnd_down_y );
  G2_SetWnd1Position( 128, work->wnd_up_y, 0/*256*/, work->wnd_down_y );

  G2_SetWnd0InsidePlane(
    GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,
    TRUE );
  G2_SetWnd1InsidePlane(
    GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,
    TRUE );

  G2_SetWndOutsidePlane(
    GX_WND_PLANEMASK_BG1,
    TRUE );

  work->wnd_appear_speed  = 0;
  work->wnd_appear_wait   = 0;
  work->wnd_count         = 0;
}
static void Egg_Demo_WndExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  GX_SetVisibleWnd( GX_WNDMASK_NONE );
}
static void Egg_Demo_WndMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  if( work->wnd_appear_speed != 0 )
  {
    work->wnd_count++;
    if( work->wnd_count >= work->wnd_appear_wait )
    {
      BOOL b_up_y_move  = FALSE;
      s16  down_y       = work->wnd_down_y;
      s16  up_y         = work->wnd_up_y;

      down_y -= work->wnd_appear_speed;
      if( work->wnd_appear_speed > 0 )
      {
        if( down_y < WND_DOWN_Y_APPEAR + WND_UP_Y_APPEAR ) b_up_y_move = TRUE;
      }
      else
      {
        b_up_y_move = TRUE;
      }
      if( b_up_y_move ) up_y += work->wnd_appear_speed;

      if( down_y < WND_DOWN_Y_APPEAR ) work->wnd_down_y = WND_DOWN_Y_APPEAR;
      else if( down_y > 192 )          work->wnd_down_y = 192;
      else                             work->wnd_down_y = (u8)down_y;

      if( up_y < 0 )                    work->wnd_up_y = 0;
      else if( up_y > WND_UP_Y_APPEAR ) work->wnd_up_y = WND_UP_Y_APPEAR;
      else                              work->wnd_up_y = (u8)up_y;

      G2_SetWnd0Position(   0, work->wnd_up_y,      128, work->wnd_down_y );
      G2_SetWnd1Position( 128, work->wnd_up_y, 0/*256*/, work->wnd_down_y );

      {
        if( work->wnd_appear_speed > 0 )
        {
          if( work->wnd_up_y == WND_UP_Y_APPEAR && work->wnd_down_y == WND_DOWN_Y_APPEAR )
            work->wnd_appear_speed = 0;
        }
        else
        {
          if( work->wnd_up_y == 0 && work->wnd_down_y == 192 )
            work->wnd_appear_speed = 0;
        }
      }

      work->wnd_count = 0;
    }
  }
}
static void Egg_Demo_WndAppear( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  work->wnd_appear_speed = 1;
}
static void Egg_Demo_WndDisappear( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  work->wnd_appear_speed = -2;
}
