//=============================================================================================
/**
 * @file  btlv_scu.c
 * @brief ポケモンWB バトル 描画 上画面制御モジュール
 * @author  taya
 *
 * @date  2008.11.18  作成
 */
//=============================================================================================
#include <gflib.h>

#include "sound\pm_sndsys.h"
#include "poke_tool/monsno_def.h"
#include "print/printsys.h"
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "battle/battgra_wb.naix"

#include "battle/btl_common.h"
#include "battle/btl_util.h"
#include "battle/btl_string.h"
#include "btlv_common.h"
#include "btlv_core.h"
#include "btlv_effect.h"  //soga
#include "tr_tool/tr_tool.h"

#include "btlv_scu.h"

enum {
  PRINT_FLG = FALSE,
};


/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  TEST_STATWIN_BGCOL = 7,
  TEST_STATWIN_BGCOL_FLASH = 4,
  TEST_TOKWIN_BGCOL = 6,
  TEST_TOKWIN_CHAR_WIDTH = 10,
  TEST_TOKWIN_DOT_WIDTH  = TEST_TOKWIN_CHAR_WIDTH*8,

  MAIN_STRBUF_LEN = 512,
  SUB_STRBUF_LEN  = 32,
  SUBPROC_WORK_SIZE = 64,

  PALIDX_MSGWIN     = 0,
  PALIDX_TOKWIN1    = 1,

  COLIDX_MSGWIN_CLEAR  = 0x0c,
  COLIDX_MSGWIN_LETTER = 0x01,
  COLIDX_MSGWIN_SHADOW = 0x09,


  MSGWIN_EVA_MAX = 31,
  MSGWIN_EVA_MIN = 0,
  MSGWIN_EVB_MIN = 3,
  MSGWIN_EVB_MAX = 16,
  MSGWIN_VISIBLE_STEP = 6,

  BGFRAME_MAIN_MESSAGE  = GFL_BG_FRAME1_M,
  BGFRAME_TOKWIN_FRIEND = GFL_BG_FRAME2_M,
  BGFRAME_TOKWIN_ENEMY  = GFL_BG_FRAME3_M,

  TOKWIN_CGRDATA_CHAR_W = 18,   // とくせいウィンドウCGRデータ横キャラ数
  TOKWIN_DRAWAREA_CHAR_W = 17,  // とくせいウィンドウ書き込み可能領域の横キャラ数
  TOKWIN_DRAWAREA_CHAR_H = 4,   // とくせいウィンドウ書き込み可能領域の縦キャラ数
  TOKWIN_CGRDATA_TRANS_CHARS = TOKWIN_CGRDATA_CHAR_W * TOKWIN_DRAWAREA_CHAR_H,
  TOKWIN_CGRDATA_TRANS_SIZE = (TOKWIN_CGRDATA_TRANS_CHARS * 0x20),

  TOKWIN_CGRDATA_LINE_SIZE = (TOKWIN_CGRDATA_CHAR_W * 0x20),

  TOKWIN_DRAWAREA_WIDTH = TOKWIN_DRAWAREA_CHAR_W * 8,
  TOKWIN_LINE_CROSS_WIDTH = 12, // とくせいウィンドウに書く２行の文字が交差する最低ドット数
  TOKWIN_DRAWAREA_LINE1_OY = 7,
  TOKWIN_DRAWAREA_LINE2_OY = 18,

  TOKWIN_MARGIN_R = 4,
  TOKWIN_MARGIN_L = 8,
  TOKWIN_HIDEPOS_FRIEND = (TOKWIN_CGRDATA_CHAR_W * 8),
  TOKWIN_HIDEPOS_ENEMY = -(TOKWIN_CGRDATA_CHAR_W * 8),

  TOKWIN_MOVE_FRAMES = 8,
};


typedef enum {

  TASKTYPE_DEFAULT = 0,
  TASKTYPE_WAZA_DAMAGE,
  TASKTYPE_HP_GAUGE,
  TASKTYPE_MAX,

}TaskType;

typedef enum {

  MSGWIN_STATE_DISP = 0,
  MSGWIN_STATE_TO_HIDE,
  MSGWIN_STATE_HIDE,
  MSGWIN_STATE_TO_DISP,

}MsgWinState;

typedef enum {

  TOKWIN_SIDE_FRIEND,
  TOKWIN_SIDE_ENEMY,

}TokwinSide;

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BOOL (*BtlinEffectSeq)( BTLV_SCU*, int* );

/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/
typedef struct {
  const BTL_POKEPARAM*  bpp;
  BTLV_SCU*         parentWk;
  u8                pokePos;
  u8                vpos;
  u8                dispFlag;
  u8                enableFlag;
}STATUS_WIN;

typedef struct {
  BTLV_SCU*       parentWk;
  GFL_BMP_DATA*   bmp;
  u16             tokusei;
  u8              mySide;
  u8              bgFrame;
  u8              pokeID;
  u8              seq;
  const u8*       cgrSrc;

  fx32            posX;
  fx32            speedX;
  u8              moveTimer;
  u8              writeRaw;
  u8              dispFlag;
  u8              fFlash;

}TOK_WIN;

typedef struct {
  GFL_BMPWIN*  win;
  fx32  eva;
  fx32  evb;
  fx32  eva_step;
  fx32  evb_step;
  u8    state;
  u8    timer;
}MSGWIN_VISIBLE;

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
struct _BTLV_SCU {

  GFL_BMPWIN*     win;
  GFL_BMP_DATA*   bmp;

  PRINT_QUE*      printQue;
  PRINT_UTIL      printUtil;
  GFL_FONT*       defaultFont;
  GFL_FONT*       smallFont;
  GFL_TCBLSYS*    tcbl;
  PRINT_STREAM*      printStream;
  STRBUF*            strBufMain;
  STRBUF*            strBufSubA;
  STRBUF*            strBufSubB;

  u8              taskCounter[TASKTYPE_MAX];


  STATUS_WIN    statusWin[ BTL_POS_MAX ];
  TOK_WIN       tokWin[ BTL_SIDE_MAX ];
  void*         tokWinCgrHead;
  NNSG2dCharacterData*  tokWinCgr;


  BTL_PROC          proc;
  u8                procWork[ SUBPROC_WORK_SIZE ];
  const BTLV_CORE*  vcore;
  const BTL_MAIN_MODULE*  mainModule;
  const BTL_POKE_CONTAINER* pokeCon;
  HEAPID        heapID;
  u8            printSeq;
  u8            playerClientID;
  u8            printJustDoneFlag;
  PRINTSTREAM_STATE  printState;
  u16           printWait;
  u16           printWaitOrg;
  u8            msgwinVisibleFlag;
  u8            btlinSeq;
  u16           msgwinVisibleSeq;
  MSGWIN_VISIBLE   msgwinVisibleWork;
};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static inline void* Scu_GetProcWork( BTLV_SCU* wk, u32 size );
static BOOL btlin_wild_single( int* seq, void* wk_adrs );
static BOOL btlin_trainer_single( int* seq, void* wk_adrs );
static BOOL btlin_comm_single( int* seq, void* wk_adrs );
static BOOL btlin_wild_double( int* seq, void* wk_adrs );
static BOOL btlin_trainer_double( int* seq, void* wk_adrs );
static BOOL btlin_comm_double( int* seq, void* wk_adrs );
static BOOL btlin_comm_double_multi( int* seq, void* wk_adrs );
static BOOL btlin_trainer_triple( int* seq, void* wk_adrs );
static BOOL btlin_comm_triple( int* seq, void* wk_adrs );
static BOOL  btlinEff_OpponentTrainerIn( BTLV_SCU* wk, int* seq );
static BOOL btlinEff_OpponentPokeInSingle( BTLV_SCU* wk, int* seq );
static BOOL btlinEff_OpponentPokeInDouble( BTLV_SCU* wk, int* seq );
static BOOL btlinEff_OpponentPokeInTriple( BTLV_SCU* wk, int* seq );
static BOOL btlinEff_MyPokeInSingle( BTLV_SCU* wk, int* seq );
static BOOL btlinEff_MyPokeInDouble( BTLV_SCU* wk, int* seq );
static BOOL btlinEff_MyPokeInTriple( BTLV_SCU* wk, int* seq );
static u16 btlfinEffsub_getOpponentPokeInStrID( BTLV_SCU* wk, u8 pokeCount );
static void taskPokeOutAct( GFL_TCBL* tcbl, void* wk_adrs );
static void taskPokeInEffect( GFL_TCBL* tcbl, void* wk_adrs );
static void taskFakeDisable( GFL_TCBL* tcbl, void* wk_adrs );
static void msgWinVisible_Init( MSGWIN_VISIBLE* wk, GFL_BMPWIN* win );
static void msgWinVisible_Hide( MSGWIN_VISIBLE* wk );
static void msgWinVisible_Disp( MSGWIN_VISIBLE* wk );
static BOOL msgWinVisible_Update( MSGWIN_VISIBLE* wk );
static void statwin_setupAll( BTLV_SCU* wk );
static void statwin_cleanupAll( BTLV_SCU* wk );
static void statwin_setup( STATUS_WIN* stwin, BTLV_SCU* wk, BtlPokePos pokePos );
static void statwin_cleanup( STATUS_WIN* stwin );
static void statwin_hide( STATUS_WIN* stwin );
static void statwin_disp_start( STATUS_WIN* stwin );
static TokwinSide PokePosToTokwinSide( const BTL_MAIN_MODULE* mainModule, BtlPokePos pos );
static void tokwin_setupAll( BTLV_SCU* wk );
static void tokwin_cleanupAll( BTLV_SCU* wk );
static void tokwin_disp_first( TOK_WIN* tokwin, BtlPokePos pos, BOOL fFlash );
static BOOL tokwin_disp_progress( TOK_WIN* tokwin );
static void tokwin_update_cgr( TOK_WIN* tokwin );
static void tokwin_hide_first( TOK_WIN* tokwin );
static BOOL tokwin_hide_progress( TOK_WIN* tokwin );
static void tokwin_renew_start( TOK_WIN* tokwin, BtlPokePos pos );
static BOOL tokwin_renew_progress( TOK_WIN* tokwin );
static void bbgp_make( BTLV_SCU* wk, BTLV_BALL_GAUGE_PARAM* bbgp, u8 clientID, BTLV_BALL_GAUGE_TYPE type );



BTLV_SCU*  BTLV_SCU_Create( const BTLV_CORE* vcore,
  const BTL_MAIN_MODULE* mainModule, const BTL_POKE_CONTAINER* pokeCon,
  GFL_TCBLSYS* tcbl, GFL_FONT* defaultFont, GFL_FONT* smallFont, u8 playerClientID, HEAPID heapID )
{
  BTLV_SCU* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTLV_SCU) );

  wk->vcore = vcore;
  wk->mainModule = mainModule;
  wk->pokeCon = pokeCon;
  wk->heapID = heapID;
  wk->playerClientID = playerClientID;
  wk->defaultFont = defaultFont;
  wk->smallFont = smallFont;

  wk->printStream = NULL;
  wk->tcbl = tcbl;
  wk->strBufMain = GFL_STR_CreateBuffer( MAIN_STRBUF_LEN, heapID );
  wk->strBufSubA = GFL_STR_CreateBuffer( SUB_STRBUF_LEN, heapID );
  wk->strBufSubB = GFL_STR_CreateBuffer( SUB_STRBUF_LEN, heapID );
  wk->msgwinVisibleFlag = FALSE;

  wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
  PRINT_UTIL_Setup( &wk->printUtil, wk->win );

  return wk;
}

void BTLV_SCU_Setup( BTLV_SCU* wk )
{
  // 個別フレーム設定：メインメッセージウィンドウ用
  static const GFL_BG_BGCNT_HEADER bgcntText = {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x4800, GX_BG_CHARBASE_0x10000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };
  // 個別フレーム設定：味方側とくせいウィンドウ用
  static const GFL_BG_BGCNT_HEADER bgcntTok1 = {
    TOKWIN_HIDEPOS_FRIEND, 0, 0x1000, 0,
    GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x08000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };
  // 個別フレーム設定：敵側とくせいウィンドウ用
  static const GFL_BG_BGCNT_HEADER bgcntTok2 = {
    TOKWIN_HIDEPOS_ENEMY, 0, 0x2000, 0,
    GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };
  u32 winfrm_charpos;

  GFL_BG_SetBGControl( BGFRAME_MAIN_MESSAGE,  &bgcntText,   GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( BGFRAME_TOKWIN_FRIEND, &bgcntTok1,   GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( BGFRAME_TOKWIN_ENEMY,  &bgcntTok2,   GFL_BG_MODE_TEXT );

  PaletteWorkSetEx_Arc( BTLV_EFFECT_GetPfd(), ARCID_BATTGRA, NARC_battgra_wb_msgwin_frm_NCLR, wk->heapID,
      FADE_MAIN_BG, 0x20, PALIDX_MSGWIN*16, 0 );

  PaletteWorkSetEx_Arc( BTLV_EFFECT_GetPfd(), ARCID_BATTGRA, NARC_battgra_wb_tokusei_w_NCLR, wk->heapID,
        FADE_MAIN_BG, 0x40, PALIDX_TOKWIN1*16, PALIDX_TOKWIN1*16 );

  //BD面カラーを黒にする
  {
    u16 dat = 0;
    PaletteWorkSet( BTLV_EFFECT_GetPfd(), &dat, FADE_MAIN_BG, 0, 2 );
  }

  GFL_BG_FillScreen( BGFRAME_MAIN_MESSAGE, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
  GFL_BG_FillCharacter( BGFRAME_MAIN_MESSAGE, 0x00, 1, 0 );
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, GFL_HEAP_LOWID(wk->heapID) );
    GFL_ARCUTIL_TRANSINFO transInfo;
    transInfo = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( handle, NARC_battgra_wb_msgwin_frm_NCGR,
                BGFRAME_MAIN_MESSAGE, 0, FALSE, GFL_HEAP_LOWID(wk->heapID) );
    winfrm_charpos = GFL_ARCUTIL_TRANSINFO_GetPos( transInfo );
    GFL_ARC_CloseDataHandle( handle );
  }


//  GFL_BG_FillScreen( BGFRAME_TOKWIN_FRIEND, 0x0000, 0, 0, 64, 32, GFL_BG_SCRWRT_PALIN );
//  GFL_BG_FillScreen( BGFRAME_TOKWIN_ENEMY,  0x0000, 0, 0, 64, 32, GFL_BG_SCRWRT_PALIN );
//  GFL_ARC_UTIL_TransVramBgCharacterAreaMan( ARCID_BATTGRA, NARC_battgra_wb_tokusei_w_NCGR, BGFRAME_TOKWIN_FRIEND,
//                              0, FALSE, wk->heapID );
  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_BATTGRA, NARC_battgra_wb_tokusei_w_NCGR, BGFRAME_TOKWIN_FRIEND,
                              0, 0, FALSE, wk->heapID );
  GFL_ARC_UTIL_TransVramScreen( ARCID_BATTGRA, NARC_battgra_wb_tokusei_w01_NSCR, BGFRAME_TOKWIN_FRIEND,
          0, 0, FALSE, wk->heapID );
  GFL_ARC_UTIL_TransVramScreen( ARCID_BATTGRA, NARC_battgra_wb_tokusei_w02_NSCR, BGFRAME_TOKWIN_ENEMY,
          0, 0, FALSE, wk->heapID );

  wk->win = GFL_BMPWIN_Create( BGFRAME_MAIN_MESSAGE, 1, 19, 30, 4, PALIDX_MSGWIN, GFL_BMP_CHRAREA_GET_F );
  wk->bmp = GFL_BMPWIN_GetBmp( wk->win );
  GFL_BMPWIN_MakeScreen( wk->win );

  GFL_BMPWIN_MakeFrameScreen( wk->win, winfrm_charpos, PALIDX_MSGWIN );

  GFL_BMP_Clear( wk->bmp, COLIDX_MSGWIN_CLEAR );
  GFL_BMPWIN_TransVramCharacter( wk->win );

  msgWinVisible_Init( &wk->msgwinVisibleWork, wk->win );

  statwin_setupAll( wk );
  tokwin_setupAll( wk );

  GFL_BG_LoadScreenReq( BGFRAME_MAIN_MESSAGE  );
  GFL_BG_LoadScreenReq( BGFRAME_TOKWIN_FRIEND );
  GFL_BG_LoadScreenReq( BGFRAME_TOKWIN_ENEMY  );

//  GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
  GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
  GFL_BG_SetVisible( BGFRAME_MAIN_MESSAGE,   VISIBLE_ON );
  GFL_BG_SetVisible( BGFRAME_TOKWIN_FRIEND,  VISIBLE_ON );
  GFL_BG_SetVisible( BGFRAME_TOKWIN_ENEMY,   VISIBLE_ON );
  ///<obj
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

void BTLV_SCU_Delete( BTLV_SCU* wk )
{
  tokwin_cleanupAll( wk );
  statwin_cleanupAll( wk );

  GFL_BMPWIN_Delete( wk->win );
  GFL_BG_FreeBGControl( BGFRAME_MAIN_MESSAGE );
  GFL_BG_FreeBGControl( BGFRAME_TOKWIN_FRIEND );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );

  PRINTSYS_QUE_Delete( wk->printQue );
  GFL_STR_DeleteBuffer( wk->strBufSubB );
  GFL_STR_DeleteBuffer( wk->strBufSubA );
  GFL_STR_DeleteBuffer( wk->strBufMain );

  GFL_HEAP_FreeMemory( wk );
}


//--------------------------------------------------------------------------
/**
 *
 *
 * @param   wk
 * @param   size
 *
 * @retval  inline void*
 */
//--------------------------------------------------------------------------
static inline void* Scu_GetProcWork( BTLV_SCU* wk, u32 size )
{
  GF_ASSERT(size < sizeof(wk->procWork));
  return wk->procWork;
}


//=============================================================================================
/**
 * バトル画面初期セットアップ完了までの演出を開始する
 *
 * @param   wk
 *
 */
//=============================================================================================
void BTLV_SCU_StartBtlIn( BTLV_SCU* wk )
{
  BtlCompetitor  competitor = BTL_MAIN_GetCompetitor( wk->mainModule );

  wk->btlinSeq = 0;

  switch( BTL_MAIN_GetRule(wk->mainModule) ){
  case BTL_RULE_SINGLE:
    switch( competitor ){
    case BTL_COMPETITOR_WILD:     // 野生
    default:
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_wild_single );
      break;
    case BTL_COMPETITOR_TRAINER:  // ゲーム内トレーナー
    case BTL_COMPETITOR_SUBWAY:   // サブウェイトレーナー
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_trainer_single );
      break;
    case BTL_COMPETITOR_COMM:
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_comm_single );
      break;
    }
    break;

  case BTL_RULE_DOUBLE:
    switch( competitor ){
    case BTL_COMPETITOR_WILD:     // 野生
    default:
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_wild_double );
      break;

    case BTL_COMPETITOR_TRAINER:  // ゲーム内トレーナー
    case BTL_COMPETITOR_SUBWAY:   // サブウェイトレーナー
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_trainer_double );
      break;

    case BTL_COMPETITOR_COMM:
      if( !BTL_MAIN_IsMultiMode(wk->mainModule) ){
        BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_comm_double );
      }else{
        BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_comm_double_multi );
      }
    }
    break;

  case BTL_RULE_TRIPLE:
    // ゲーム内トレーナー
    if( competitor != BTL_COMPETITOR_COMM ){
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_trainer_triple );
    // 通信対戦
    }else{
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_comm_triple );
    }
    break;

  case BTL_RULE_ROTATION:
    // ゲーム内トレーナー
    if( competitor != BTL_COMPETITOR_COMM ){
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_trainer_double );
    // 通信対戦
    }else{
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_comm_double );
    }
    break;
  }
}

//=============================================================================================
/**
 * バトル画面初期セットアップ完了までの演出を終了するまで待つ
 *
 * @param   wk
 *
 * @retval  BOOL    終了したらTRUE
 */
//=============================================================================================
BOOL BTLV_SCU_WaitBtlIn( BTLV_SCU* wk )
{
  return BTL_UTIL_CallProc( &wk->proc );
}
//--------------------------------------------------------------------------
/**
 * 戦闘画面セットアップ完了までの演出（野生／シングル）
 * @retval  BOOL
 */
//--------------------------------------------------------------------------
static BOOL btlin_wild_single( int* seq, void* wk_adrs )
{
  typedef struct {
    const BTL_POKEPARAM* pp;
    BtlPokePos  pokePos;
    u8          viewPos;
    u8  pokeID;
  }ProcWork;

  BTLV_SCU* wk = wk_adrs;
  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:
    subwk->viewPos = BTLV_MCSS_POS_BB;
    subwk->pokePos = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, subwk->viewPos );
    subwk->pp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pokePos );
    subwk->pokeID = BPP_GetID( subwk->pp );
    msgWinVisible_Hide( &wk->msgwinVisibleWork );
    (*seq)++;
    break;
  case 1:
    if( msgWinVisible_Update(&wk->msgwinVisibleWork) )
    {
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->pp), subwk->viewPos );
      BTLV_EFFECT_AddByPos( subwk->viewPos, BTLEFF_SINGLE_ENCOUNT_1 );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );
      (*seq)++;
    }
    break;
  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_Encount_Wild1, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 3:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      statwin_disp_start( &wk->statusWin[ subwk->pokePos ] );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;
  case 4:
    if( msgWinVisible_Update(&wk->msgwinVisibleWork) )
    {
      const MYSTATUS* status = BTL_MAIN_GetPlayerStatus( wk->mainModule );
      if( MyStatus_GetMySex(status) == PM_MALE ){
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_MALE );
      }else{
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_FEMALE );
      }
      (*seq)++;
    }
    break;
  case 5:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      subwk->viewPos = BTLV_MCSS_POS_AA;
      subwk->pokePos = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, subwk->viewPos );
      subwk->pp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pokePos );
      subwk->pokeID = BPP_GetID( subwk->pp );

      BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_PutSingle, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 6:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->pp), subwk->viewPos );
      BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_3 );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;
  case 7:
    if( msgWinVisible_Update(&wk->msgwinVisibleWork) ){
      (*seq)++;
    }
    break;
  case 8:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      statwin_disp_start( &wk->statusWin[ subwk->pokePos ] );
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//--------------------------------------------------------------------------
/**
 * 戦闘画面セットアップ完了までの演出（ゲーム内トレーナー／シングル）
 * @retval  BOOL
 */
//--------------------------------------------------------------------------
static BOOL btlin_trainer_single( int* seq, void* wk_adrs )
{
  typedef struct {
    const BTL_POKEPARAM* pp;
    BtlPokePos  pokePos;
    u8          viewPos;
    u8  pokeID;
    u8  clientID;
  }ProcWork;

  BTLV_SCU* wk = wk_adrs;
  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:
    {
      enum {
        VPOS = BTLV_MCSS_POS_BB,
      };

      u8 btlPos = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, VPOS );
      u8 clientID = BTL_MAIN_BtlPosToClientID( wk->mainModule, btlPos );

      u16 trType = BTL_MAIN_GetClientTrainerType( wk->mainModule, clientID );
      BTLV_EFFECT_SetTrainer( trType, BTLV_MCSS_POS_TR_BB, 0, 0, 0 );

      subwk->viewPos = VPOS;
      subwk->clientID = clientID;
      subwk->pokePos = btlPos;
      subwk->pp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pokePos );
      subwk->pokeID = BPP_GetID( subwk->pp );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;
  case 1:
    if( msgWinVisible_Update(&wk->msgwinVisibleWork) )
    {
      BTLV_EFFECT_AddByPos( subwk->viewPos, BTLEFF_SINGLE_TRAINER_ENCOUNT_1 );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );
      (*seq)++;
    }
    break;
  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTLV_BALL_GAUGE_PARAM bbgp;

      bbgp_make( wk, &bbgp, subwk->clientID, BTLV_BALL_GAUGE_TYPE_ENEMY );
      BTLV_EFFECT_SetBallGauge( &bbgp );
      BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_Encount_NPC1, 1, subwk->clientID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 3:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_AddByPos( subwk->viewPos, BTLEFF_SINGLE_TRAINER_ENCOUNT_2 );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;
  case 4:
    if( ( msgWinVisible_Update(&wk->msgwinVisibleWork) ) &&
        ( !BTLV_EFFECT_CheckExecute() ) )
    {
      BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_PutSingle_NPC, 2, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 5:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->pp), subwk->viewPos );
      BTLV_EFFECT_AddByPos( subwk->viewPos, BTLEFF_SINGLE_TRAINER_ENCOUNT_3 );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;
  case 6:
    if( ( msgWinVisible_Update(&wk->msgwinVisibleWork) ) &&
        ( !BTLV_EFFECT_CheckExecute() ) )
    {
      BTLV_BALL_GAUGE_PARAM bbgp;

      bbgp_make( wk, &bbgp, BTL_MAIN_GetPlayerClientID(wk->mainModule), BTLV_BALL_GAUGE_TYPE_MINE );
      BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_ENEMY );
      BTLV_EFFECT_SetBallGauge( &bbgp );
      (*seq)++;
    }
    break;
  case 7:
//    if( !BTLV_EFFECT_CheckExecuteBallGauge( BTLV_BALL_GAUGE_TYPE_MINE ) )
    {
      const MYSTATUS* status = BTL_MAIN_GetPlayerStatus( wk->mainModule );
      statwin_disp_start( &wk->statusWin[ subwk->pokePos ] );
      if( MyStatus_GetMySex(status) == PM_MALE ){
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_MALE );
      }else{
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_FEMALE );
      }
      (*seq)++;
    }
    break;
  case 8:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      subwk->viewPos = BTLV_MCSS_POS_AA;
      subwk->pokePos = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, subwk->viewPos );
      subwk->pp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pokePos );
      subwk->pokeID = BPP_GetID( subwk->pp );

      BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_PutSingle, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 9:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->pp), subwk->viewPos );
      BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_3 );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;
  case 10:
    if( msgWinVisible_Update(&wk->msgwinVisibleWork) ){
      (*seq)++;
    }
    break;
  case 11:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_MINE );
      statwin_disp_start( &wk->statusWin[ subwk->pokePos ] );
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//--------------------------------------------------------------------------
/**
 * 戦闘画面セットアップ完了までの演出（通信対戦トレーナー／シングル）
 * @retval  BOOL
 */
//--------------------------------------------------------------------------
static BOOL btlin_comm_single( int* seq, void* wk_adrs )
{
  static const BtlinEffectSeq funcs[][2] = {
    { btlinEff_OpponentTrainerIn,     btlinEff_OpponentTrainerIn    },
    { btlinEff_OpponentPokeInSingle,  btlinEff_MyPokeInSingle       },
    { btlinEff_MyPokeInSingle,        btlinEff_OpponentPokeInSingle },
  };

  BTLV_SCU* wk = wk_adrs;

  if( wk->btlinSeq < NELEMS(funcs) )
  {
    u8 myClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
    if( funcs[wk->btlinSeq][myClientID](wk, seq) ){
      wk->btlinSeq++;
      (*seq) = 0;
    }
    return FALSE;
  }
  else{
    return TRUE;
  }
}
//--------------------------------------------------------------------------
/**
 * 戦闘画面セットアップ完了までの演出（野生／ダブル）
 * @retval  BOOL    TRUEで終了
 */
//--------------------------------------------------------------------------
static BOOL btlin_wild_double( int* seq, void* wk_adrs )
{
  typedef struct {
    const BTL_POKEPARAM* pp[2];
    u8  pokeID[2];
    u8  pos[2];
    u8  frontMemberCount;
  }ProcWork;

  BTLV_SCU* wk = wk_adrs;
  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:
    {
      BtlPokePos myPos = BTL_MAIN_GetClientPokePos( wk->mainModule, wk->playerClientID, 0 );
      subwk->pos[0] = BTL_MAIN_GetOpponentPokePos( wk->mainModule, myPos, 0 );
      subwk->pp[0] = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos[0] );
      subwk->pokeID[0] = BPP_GetID( subwk->pp[0] );
      subwk->pos[1] = BTL_MAIN_GetOpponentPokePos( wk->mainModule, myPos, 1 );
      subwk->pp[1] = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos[1] );
      subwk->pokeID[1] = BPP_GetID( subwk->pp[1] );

      BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_Encount_Wild2, 2, subwk->pokeID[0], subwk->pokeID[1] );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk->pos[0] );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->pp[0]), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk->pos[0] ] );
      (*seq)++;
    }
    break;
  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk->pos[1] );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->pp[1]), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk->pos[1] ] );
      (*seq)++;
    }//subwk\f\[[0-1]\]\f\.\f[a-zA-Z]+
    break;
  case 3:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      subwk->pos[0] = BTL_MAIN_GetClientPokePos( wk->mainModule, wk->playerClientID, 0 );
      subwk->pp[0] = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos[0] );
      subwk->pokeID[0] = BPP_GetID( subwk->pp[0] );
      subwk->pos[1] = BTL_MAIN_GetClientPokePos( wk->mainModule, wk->playerClientID, 1 );
      subwk->pp[1] = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos[1] );
      if( (subwk->pp[1] != NULL)
      &&  (!BPP_IsDead(subwk->pp[1]))
      ){
        subwk->pokeID[1] = BPP_GetID( subwk->pp[1] );
        subwk->frontMemberCount = 2;
      }else{
        subwk->frontMemberCount = 1;
      }
      if( subwk->frontMemberCount == 2 ){
        BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_PutDouble, 2, subwk->pokeID[0], subwk->pokeID[1] );
      }else{
        BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_PutSingle, 1, subwk->pokeID[0] );
      }
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 4:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk->pos[0] );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->pp[0]), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk->pos[0] ] );
      (*seq)++;
    }
    break;
  case 5:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      if( subwk->frontMemberCount == 2 ){
        u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk->pos[1] );
        BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->pp[1]), viewPos );
        statwin_disp_start( &wk->statusWin[ subwk->pos[1] ] );
      }
      (*seq)++;
    }
    break;
  case 6:
    if( !BTLV_EFFECT_CheckExecute() )
     {
      return TRUE;
    }
  }
  return FALSE;
}
//--------------------------------------------------------------------------
/**
 * 戦闘画面セットアップ完了までの演出（ゲーム内トレーナー／ダブル）
 * @retval  BOOL    TRUEで終了
 */
//--------------------------------------------------------------------------
static BOOL btlin_trainer_double( int* seq, void* wk_adrs )
{
  static const BtlinEffectSeq funcs[] = {
    btlinEff_OpponentTrainerIn,
    btlinEff_OpponentPokeInDouble,
    btlinEff_MyPokeInDouble,
  };

  BTLV_SCU* wk = wk_adrs;

  if( wk->btlinSeq < NELEMS(funcs) )
  {
    if( funcs[wk->btlinSeq](wk, seq) ){
      wk->btlinSeq++;
      (*seq) = 0;
    }
    return FALSE;
  }
  else{
    return TRUE;
  }
}
//--------------------------------------------------------------------------
/**
 * 戦闘画面セットアップ完了までの演出（通信対戦／ダブル）
 * @retval  BOOL    TRUEで終了
 */
//--------------------------------------------------------------------------
static BOOL btlin_comm_double( int* seq, void* wk_adrs )
{
  static const BtlinEffectSeq funcs[][2] = {
    { btlinEff_OpponentTrainerIn,     btlinEff_OpponentTrainerIn    },
    { btlinEff_OpponentPokeInDouble,  btlinEff_MyPokeInDouble       },
    { btlinEff_MyPokeInDouble,        btlinEff_OpponentPokeInDouble },
  };

  BTLV_SCU* wk = wk_adrs;

  if( wk->btlinSeq < NELEMS(funcs) )
  {
    u8 clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
    if( funcs[wk->btlinSeq][clientID](wk, seq) ){
      wk->btlinSeq++;
      (*seq) = 0;
    }
    return FALSE;
  }
  else{
    return TRUE;
  }
}
//--------------------------------------------------------------------------
/**
 * 戦闘画面セットアップ完了までの演出（通信対戦／ダブルマルチ）
 * @retval  BOOL    TRUEで終了
 */
//--------------------------------------------------------------------------
static BOOL btlin_comm_double_multi( int* seq, void* wk_adrs )
{
  typedef struct {
    const BTL_POKEPARAM* pp;
    u8  pokeID;
    u8  pos;
  }ProcWork;

  BTLV_SCU* wk = wk_adrs;
  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork)*2 );

  switch( *seq ){
  case 0:
    {
      BtlPokePos myPos = BTL_MAIN_GetClientPokePos( wk->mainModule, wk->playerClientID, 0 );
      subwk[0].pos = BTL_MAIN_GetOpponentPokePos( wk->mainModule, myPos, 0 );
      subwk[0].pp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk[0].pos );
      subwk[0].pokeID = BPP_GetID( subwk[0].pp );
      subwk[1].pos = BTL_MAIN_GetOpponentPokePos( wk->mainModule, myPos, 1 );
      subwk[1].pp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk[1].pos );
      subwk[1].pokeID = BPP_GetID( subwk[1].pp );

      BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_Encount_Wild2, 2, subwk[0].pokeID, subwk[1].pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[0].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk[0].pp), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk[0].pos ] );
      (*seq)++;
    }
    break;
  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[1].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk[1].pp), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk[1].pos ] );
      (*seq)++;
    }
    break;
  case 3:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      subwk[0].pos = BTL_MAIN_GetClientPokePos( wk->mainModule, wk->playerClientID, 0 );
      subwk[0].pp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk[0].pos );
      subwk[0].pokeID = BPP_GetID( subwk[0].pp );
      subwk[1].pos = BTL_MAIN_GetNextPokePos( wk->mainModule, subwk[0].pos );
      subwk[1].pp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk[1].pos );
      subwk[1].pokeID = BPP_GetID( subwk[1].pp );

      BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_PutDouble, 2, subwk[0].pokeID, subwk[1].pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 4:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[0].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk[0].pp), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk[0].pos ] );
      (*seq)++;
    }
    break;
  case 5:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[1].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk[1].pp), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk[1].pos ] );
      (*seq)++;
    }
    break;
  case 6:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      return TRUE;
    }
  }
  return FALSE;
}
//--------------------------------------------------------------------------
/**
 * 戦闘画面セットアップ完了までの演出（ゲーム内トレーナー／トリプル）
 * @retval  BOOL    TRUEで終了
 */
//--------------------------------------------------------------------------
static BOOL btlin_trainer_triple( int* seq, void* wk_adrs )
{
  static const BtlinEffectSeq funcs[] = {
    btlinEff_OpponentTrainerIn,
    btlinEff_OpponentPokeInTriple,
    btlinEff_MyPokeInTriple,
  };

  BTLV_SCU* wk = wk_adrs;

  if( wk->btlinSeq < NELEMS(funcs) )
  {
    if( funcs[wk->btlinSeq](wk, seq) ){
      wk->btlinSeq++;
      (*seq) = 0;
    }
    return FALSE;
  }
  else{
    return TRUE;
  }
}
//--------------------------------------------------------------------------
/**
 * 戦闘画面セットアップ完了までの演出（通信対戦／トリプル）
 * @retval  BOOL    TRUEで終了
 */
//--------------------------------------------------------------------------
static BOOL btlin_comm_triple( int* seq, void* wk_adrs )
{
  static const BtlinEffectSeq funcs[][2] = {
    { btlinEff_OpponentTrainerIn,     btlinEff_OpponentTrainerIn    },
    { btlinEff_OpponentPokeInTriple,  btlinEff_MyPokeInTriple       },
    { btlinEff_MyPokeInTriple,        btlinEff_OpponentPokeInTriple },
  };

  BTLV_SCU* wk = wk_adrs;

  if( wk->btlinSeq < NELEMS(funcs) )
  {
    u8 clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
//    BTL_Printf("clientID=%d\n", clientID);

    if( funcs[wk->btlinSeq][clientID](wk, seq) ){
      wk->btlinSeq++;
      (*seq) = 0;
    }
    return FALSE;
  }
  else{
    return TRUE;
  }
}



//---------------------------------------------------------------------------
/*
 *  開始エフェクト：相手トレーナー登場
 */
//---------------------------------------------------------------------------
static BOOL  btlinEff_OpponentTrainerIn( BTLV_SCU* wk, int* seq )
{
  typedef struct {
    const BTL_POKEPARAM* bpp[2];
    u8    myClientID;
    u8    enemyClientID;
    u8    pos;
    u8    vpos;
  }ProcWork;

  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:   // トレーナー絵セット
    {
      u16 trType;

      subwk->myClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
      subwk->enemyClientID = BTL_MAIN_GetOpponentClientID( wk->mainModule, subwk->myClientID, 0 );
      subwk->vpos = BTLV_MCSS_POS_TR_BB;

      trType = BTL_MAIN_GetClientTrainerType( wk->mainModule, subwk->enemyClientID );
      BTLV_EFFECT_SetTrainer( trType, subwk->vpos, 0, 0, 0 );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;

  case 1:   // カメラ、敵トレーナー等倍表示までズームアウト
    if( msgWinVisible_Update(&wk->msgwinVisibleWork) )
    {
      BTLV_EFFECT_AddByPos( subwk->vpos, BTLEFF_SINGLE_TRAINER_ENCOUNT_1 );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );
      (*seq)++;
    }
    break;

  case 2:   // 敵ボールゲージ出現＆「○○が勝負をしかけてきた！」表示
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTLV_BALL_GAUGE_PARAM bbgp;

      u16 strID = (BTL_MAIN_GetCompetitor(wk->mainModule) != BTL_COMPETITOR_COMM)?
                      BTL_STRID_STD_Encount_NPC1 : BTL_STRID_STD_Encount_Player1;


      bbgp_make( wk, &bbgp, subwk->enemyClientID, BTLV_BALL_GAUGE_TYPE_ENEMY );
      BTLV_EFFECT_SetBallGauge( &bbgp );

      BTL_STR_MakeStringStd( wk->strBufMain, strID, 1, subwk->enemyClientID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 3: // カメラ、デフォルト位置までズームアウト
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_AddByPos( subwk->vpos, BTLEFF_SINGLE_TRAINER_ENCOUNT_2 );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;

  case 4:
    if( !BTLV_EFFECT_CheckExecute() ){
      return TRUE;
    }
  }
  return FALSE;
}
//---------------------------------------------------------------------------
/*
 *  開始エフェクト：相手ポケモン登場（シングル）
 */
//---------------------------------------------------------------------------
static BOOL btlinEff_OpponentPokeInSingle( BTLV_SCU* wk, int* seq )
{
  typedef struct {
    const BTL_POKEPARAM* bpp;
    u8    pos;
    u8    vpos;
    u8    pokeID;
    u8    aliveCnt;
    u8    clientID;
  }ProcWork;

  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:
    {
      u32 i;
      u16 strID;

      subwk->vpos = BTLV_MCSS_POS_BB;
      {
        u8 myClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
        subwk->clientID = BTL_MAIN_GetOpponentClientID( wk->mainModule, myClientID, 0 );
      }
      subwk->pos = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, subwk->vpos );
      subwk->bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos );
      subwk->pokeID = BPP_GetID( subwk->bpp );

      strID = btlfinEffsub_getOpponentPokeInStrID( wk, 1 );
      BTL_STR_MakeStringStd( wk->strBufMain, strID, 2, subwk->clientID, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 1:
    if( BTLV_SCU_WaitMsg(wk) ){
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;

  case 2:
    BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp), subwk->vpos );
    BTLV_EFFECT_AddByPos( subwk->vpos, BTLEFF_SINGLE_TRAINER_ENCOUNT_3 );
    BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_ENEMY );
    (*seq)++;
    break;

  case 3:
    if( !BTLV_EFFECT_CheckExecute() ){
      statwin_disp_start( &wk->statusWin[ subwk->pos ] );
      (*seq)++;
    }
    break;

  case 4:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------
/*
 *  開始エフェクト：相手ポケモン登場（ダブル）
 */
//---------------------------------------------------------------------------
static BOOL btlinEff_OpponentPokeInDouble( BTLV_SCU* wk, int* seq )
{
  typedef struct {
    const BTL_POKEPARAM* bpp[2];
    u8    pos[2];
    u8    vpos[2];
    u8    pokeID[2];
    u8    aliveCnt;
    u8    clientID;
  }ProcWork;

  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:
    {
      u32 i;
      u16 strID;

      subwk->vpos[0] = BTLV_MCSS_POS_B;
      subwk->vpos[1] = BTLV_MCSS_POS_D;
      subwk->aliveCnt = 0;
      {
        u8 myClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
        subwk->clientID = BTL_MAIN_GetOpponentClientID( wk->mainModule, myClientID, 0 );
      }
      for(i=0; i<2; ++i){
        subwk->pos[i] = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, subwk->vpos[i] );
        subwk->bpp[i] = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos[i] );
        subwk->pokeID[i] = BPP_GetID( subwk->bpp[i] );
        BTL_Printf("N[%d] vpos=%d -> pos=%d, pokeID=%d\n", i, subwk->vpos[i], subwk->pos[i], subwk->pokeID[i]);
        if( !BPP_IsDead(subwk->bpp[i]) ){
          subwk->aliveCnt++;
        }
      }

      strID = btlfinEffsub_getOpponentPokeInStrID( wk, subwk->aliveCnt );
      BTL_STR_MakeStringStd( wk->strBufMain, strID, 3, subwk->clientID, subwk->pokeID[0], subwk->pokeID[1] );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 1:
    if( BTLV_SCU_WaitMsg(wk) ){
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;

  case 2:
    BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[0]), subwk->vpos[0] );
    if( subwk->aliveCnt == 2 ){
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[1]), subwk->vpos[1] );
    }
    BTLV_EFFECT_AddByPos( subwk->vpos[0], BTLEFF_SINGLE_TRAINER_ENCOUNT_3 );
    BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_ENEMY );
    (*seq)++;
    break;

  case 3:
    if( !BTLV_EFFECT_CheckExecute() ){
      statwin_disp_start( &wk->statusWin[ subwk->pos[0] ] );
      if( subwk->aliveCnt == 2 ){
        statwin_disp_start( &wk->statusWin[ subwk->pos[1] ] );
      }
      (*seq)++;
    }
    break;

  case 4:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------
/*
 *  開始エフェクト：相手ポケモン登場（トリプル）
 */
//---------------------------------------------------------------------------
static BOOL btlinEff_OpponentPokeInTriple( BTLV_SCU* wk, int* seq )
{
  typedef struct {
    const BTL_POKEPARAM* bpp[3];
    u8    pos[3];
    u8    vpos[3];
    u8    pokeID[3];
    u8    aliveCnt;
    u8    clientID;
  }ProcWork;

  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:
    {
      u32 i;
      u16 strID;

      subwk->vpos[0] = BTLV_MCSS_POS_B;
      subwk->vpos[1] = BTLV_MCSS_POS_D;
      subwk->vpos[2] = BTLV_MCSS_POS_F;
      subwk->aliveCnt = 0;
      {
        u8 myClientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
        subwk->clientID = BTL_MAIN_GetOpponentClientID( wk->mainModule, myClientID, 0 );
      }
      for(i=0; i<3; ++i)
      {
        subwk->pos[i] = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, subwk->vpos[i] );
        subwk->bpp[i] = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos[i] );
        subwk->pokeID[i] = BPP_GetID( subwk->bpp[i] );
        if( !BPP_IsDead(subwk->bpp[i]) ){
          subwk->aliveCnt++;
        }
      }

      strID = btlfinEffsub_getOpponentPokeInStrID( wk, subwk->aliveCnt );
      BTL_STR_MakeStringStd( wk->strBufMain, strID, 4, subwk->clientID, subwk->pokeID[0], subwk->pokeID[1], subwk->pokeID[2] );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 1:
    if( BTLV_SCU_WaitMsg(wk) ){
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      (*seq)++;
    }
    break;

  case 2:
    BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[0]), subwk->vpos[0] );
    if( subwk->aliveCnt > 1){
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[1]), subwk->vpos[1] );
    }
    if( subwk->aliveCnt > 2){
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[2]), subwk->vpos[2] );
    }
    BTLV_EFFECT_AddByPos( subwk->vpos[0], BTLEFF_SINGLE_TRAINER_ENCOUNT_3 );
    BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_ENEMY );
    (*seq)++;
    break;

  case 3:
    if( !BTLV_EFFECT_CheckExecute() ){
      statwin_disp_start( &wk->statusWin[ subwk->pos[0] ] );
      if( subwk->aliveCnt > 1 ){
        statwin_disp_start( &wk->statusWin[ subwk->pos[1] ] );
      }
      if( subwk->aliveCnt > 2 ){
        statwin_disp_start( &wk->statusWin[ subwk->pos[2] ] );
      }
      (*seq)++;
    }
    break;

  case 4:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------
/*
 *  開始エフェクト：自分ポケモン登場（シングル）
 */
//---------------------------------------------------------------------------
static BOOL btlinEff_MyPokeInSingle( BTLV_SCU* wk, int* seq )
{
  typedef struct {
    const BTL_POKEPARAM* bpp;
    u8    pos;
    u8    vpos;
    u8    pokeID;
    u8    clientID;
  }ProcWork;

  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:// 自分ボールゲージ出現
    {
      BTLV_BALL_GAUGE_PARAM bbgp;

      subwk->clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
      bbgp_make( wk, &bbgp, subwk->clientID, BTLV_BALL_GAUGE_TYPE_MINE );
      BTLV_EFFECT_SetBallGauge( &bbgp );
      (*seq)++;
    }
    break;

  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      const MYSTATUS* status = BTL_MAIN_GetPlayerStatus( wk->mainModule );

      if( MyStatus_GetMySex(status) == PM_MALE ){
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_MALE );
      }else{
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_FEMALE );
      }
      (*seq)++;
    }
    break;

  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      subwk->vpos = BTLV_MCSS_POS_AA;
      subwk->clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
      subwk->pos = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, subwk->vpos );
      subwk->bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos );
      subwk->pokeID = BPP_GetID( subwk->bpp );

      BTL_STR_MakeStringStd( wk->strBufMain, BTL_STRID_STD_PutSingle, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 3:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_MINE );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp), subwk->vpos );
      BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_3 );
      (*seq)++;
    }
    break;

  case 4:
    if( !BTLV_EFFECT_CheckExecute() ){
      statwin_disp_start( &wk->statusWin[ subwk->pos ] );
      (*seq)++;
    }
    break;

  case 5:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------
/*
 *  開始エフェクト：自分ポケモン登場（ダブル）
 */
//---------------------------------------------------------------------------
static BOOL btlinEff_MyPokeInDouble( BTLV_SCU* wk, int* seq )
{
  typedef struct {
    const BTL_POKEPARAM* bpp[2];
    u8    pos[2];
    u8    vpos[2];
    u8    pokeID[2];
    u8    aliveCnt;
    u8    clientID;
  }ProcWork;

  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:// 自分ボールゲージ出現
    {
      BTLV_BALL_GAUGE_PARAM bbgp;

      subwk->clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
      bbgp_make( wk, &bbgp, subwk->clientID, BTLV_BALL_GAUGE_TYPE_MINE );

      BTLV_EFFECT_SetBallGauge( &bbgp );
      (*seq)++;
    }
    break;

  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      const MYSTATUS* status = BTL_MAIN_GetPlayerStatus( wk->mainModule );

      if( MyStatus_GetMySex(status) == PM_MALE ){
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_MALE );
      }else{
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_FEMALE );
      }
      (*seq)++;
    }
    break;

  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u32 i;
      u16 strID;

      subwk->vpos[0] = BTLV_MCSS_POS_A;
      subwk->vpos[1] = BTLV_MCSS_POS_C;
      subwk->aliveCnt = 0;
      subwk->clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
      for(i=0; i<2; ++i){
        subwk->pos[i] = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, subwk->vpos[i] );
        subwk->bpp[i] = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos[i] );
        if( subwk->bpp[i] != NULL ){
          subwk->pokeID[i] = BPP_GetID( subwk->bpp[i] );
          if( !BPP_IsDead(subwk->bpp[i]) ){
            subwk->aliveCnt++;
          }
        }
      }

      strID = ( subwk->aliveCnt == 2 )? BTL_STRID_STD_PutDouble : BTL_STRID_STD_PutSingle;
      BTL_STR_MakeStringStd( wk->strBufMain, strID, 2, subwk->pokeID[0], subwk->pokeID[1] );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 3:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_MINE );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[0]), subwk->vpos[0] );
      if( subwk->aliveCnt > 1){
        BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[1]), subwk->vpos[1] );
      }
      BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_3 );
      (*seq)++;
    }
    break;

  case 4:
    if( !BTLV_EFFECT_CheckExecute() ){
      statwin_disp_start( &wk->statusWin[ subwk->pos[0] ] );
      if( subwk->aliveCnt > 1){
        statwin_disp_start( &wk->statusWin[ subwk->pos[1] ] );
      }
      (*seq)++;
    }
    break;

  case 5:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------
/*
 *  開始エフェクト：自分ポケモン登場（トリプル）
 */
//---------------------------------------------------------------------------
static BOOL btlinEff_MyPokeInTriple( BTLV_SCU* wk, int* seq )
{
  typedef struct {
    const BTL_POKEPARAM* bpp[3];
    u8    pos[3];
    u8    vpos[3];
    u8    pokeID[3];
    u8    aliveCnt;
    u8    clientID;
  }ProcWork;

  ProcWork* subwk = Scu_GetProcWork( wk, sizeof(ProcWork) );

  switch( *seq ){
  case 0:// 自分ボールゲージ出現
    {
      BTLV_BALL_GAUGE_PARAM bbgp;
      subwk->clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
      BTL_Printf("自ポケ登場トリプル：ClientID=%d\n", subwk->clientID);
      bbgp_make( wk, &bbgp, subwk->clientID, BTLV_BALL_GAUGE_TYPE_MINE );
      BTLV_EFFECT_SetBallGauge( &bbgp );
      (*seq)++;
    }
    break;

  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      const MYSTATUS* status = BTL_MAIN_GetPlayerStatus( wk->mainModule );
      if( MyStatus_GetMySex(status) == PM_MALE ){
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_MALE );
      }else{
        BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_2_FEMALE );
      }
      (*seq)++;
    }
    break;

  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u32 i;
      u16 strID;

      subwk->vpos[0] = BTLV_MCSS_POS_A;
      subwk->vpos[1] = BTLV_MCSS_POS_C;
      subwk->vpos[2] = BTLV_MCSS_POS_E;
      subwk->aliveCnt = 0;
      subwk->clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
      for(i=0; i<3; ++i)
      {
        subwk->pos[i] = BTL_MAIN_ViewPosToBtlPos( wk->mainModule, subwk->vpos[i] );
        subwk->bpp[i] = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk->pos[i] );
        subwk->pokeID[i] = BPP_GetID( subwk->bpp[i] );
        if( !BPP_IsDead(subwk->bpp[i]) ){
          subwk->aliveCnt++;
        }
      }

      if( subwk->aliveCnt == 3 ){
        strID = BTL_STRID_STD_PutTriple;
      }else{
        strID = ( subwk->aliveCnt == 2 )? BTL_STRID_STD_PutDouble : BTL_STRID_STD_PutSingle;
      }
      BTL_STR_MakeStringStd( wk->strBufMain, strID, 3, subwk->pokeID[0], subwk->pokeID[1], subwk->pokeID[2] );
      BTLV_SCU_StartMsg( wk, wk->strBufMain, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 3:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_MINE );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[0]), subwk->vpos[0] );
      if( subwk->aliveCnt > 1){
        BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[1]), subwk->vpos[1] );
      }
      if( subwk->aliveCnt > 2){
        BTLV_EFFECT_SetPokemon( BPP_GetViewSrcData(subwk->bpp[2]), subwk->vpos[2] );
      }
      BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_3 );
      statwin_disp_start( &wk->statusWin[ subwk->pos[0] ] );
      if( subwk->aliveCnt > 1){
        statwin_disp_start( &wk->statusWin[ subwk->pos[1] ] );
      }
      if( subwk->aliveCnt > 1){
        statwin_disp_start( &wk->statusWin[ subwk->pos[2] ] );
      }
      (*seq)++;
    }
    break;

  case 4:
    if( !BTLV_EFFECT_CheckExecute() ){

      (*seq)++;
    }
    break;

  case 5:
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 相手ポケモン登場メッセージの文字列IDを取得
 *
 * @param   wk
 * @param   pokeCount
 *
 * @retval  u16
 */
//----------------------------------------------------------------------------------
static u16 btlfinEffsub_getOpponentPokeInStrID( BTLV_SCU* wk, u8 pokeCount )
{
  BtlCompetitor competitor = BTL_MAIN_GetCompetitor( wk->mainModule );
  switch( competitor ){
  case BTL_COMPETITOR_WILD:
  default:
    return ( pokeCount == 1 )? BTL_STRID_STD_Encount_Wild1 : BTL_STRID_STD_Encount_Wild2;

  case BTL_COMPETITOR_TRAINER:
  case BTL_COMPETITOR_SUBWAY:
    if( pokeCount == 1 ){
      return BTL_STRID_STD_PutSingle_NPC;
    }else if( pokeCount == 2 ){
      return BTL_STRID_STD_PutDouble_NPC;
    }
    return BTL_STRID_STD_PutTriple_NPC;

  case BTL_COMPETITOR_COMM:
    if( pokeCount == 1 ){
      return BTL_STRID_STD_PutSingle_Player;
    }else if( pokeCount == 2 ){
      return BTL_STRID_STD_PutDouble_Player;
    }
    return BTL_STRID_STD_PutTriple_Player;
  }
}

//=============================================================================================
/**
 * メッセージ表示開始
 *
 * @param   wk
 * @param   str     表示文字列
 * @param   wait    表示終了後の待ち方( 0:即終了 / 1～:通常時はキー待ち，通信時は指定フレーム待ち）
 *
 */
//=============================================================================================
void BTLV_SCU_StartMsg( BTLV_SCU* wk, const STRBUF* str, u16 wait )
{
  GF_ASSERT( wk->printStream == NULL );

  GFL_BMP_Clear( wk->bmp, COLIDX_MSGWIN_CLEAR );

  GFL_FONTSYS_SetColor( COLIDX_MSGWIN_LETTER, COLIDX_MSGWIN_SHADOW, COLIDX_MSGWIN_CLEAR );

  wk->printStream = PRINTSYS_PrintStream(
        wk->win, 0, 0, str, wk->defaultFont, BTL_MAIN_GetPrintWait(wk->mainModule), wk->tcbl, BTLV_TASKPRI_MAIN_WINDOW,
        wk->heapID, COLIDX_MSGWIN_CLEAR
  );
  PRINTSYS_PrintStreamStop( wk->printStream );

  wk->printSeq = 0;
  wk->printWait = wait;
  wk->printWaitOrg = wait;
  wk->printJustDoneFlag = FALSE;
}
//=============================================================================================
/**
 * メッセージ終端までの表示を終えたタイミングかどうか判定
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_SCU_IsJustDoneMsg( BTLV_SCU* wk )
{
  if( wk->printJustDoneFlag ){
    wk->printJustDoneFlag = FALSE;
    return TRUE;
  }
  return FALSE;
}
//=============================================================================================
/**
 * メッセージ表示終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_SCU_WaitMsg( BTLV_SCU* wk )
{
  enum {
    SEQ_START_MSGWIN_VISIBLE,
    SEQ_WAIT_MSGWIN_VISIBLE,
    SEQ_WAIT_STREAM_RUNNING,
    SEQ_WAIT_USERCTRL_NOT_COMM,
    SEQ_WAIT_USERCTRL_COMM,
    SEQ_AFTER_USERCTRL,
    SEQ_DONE,
  };

  switch( wk->printSeq ){
  case SEQ_START_MSGWIN_VISIBLE:
    msgWinVisible_Disp( &wk->msgwinVisibleWork );
    wk->printSeq++;
    /* fallthru */
  case SEQ_WAIT_MSGWIN_VISIBLE:
    if( !msgWinVisible_Update(&wk->msgwinVisibleWork) ){
      break;
    }else{
      if( wk->printStream )
      {
        PRINTSYS_PrintStreamRun( wk->printStream );
      }
      wk->printSeq++;
    }
    /* fallthru */
  case SEQ_WAIT_STREAM_RUNNING:
    if( wk->printStream )
    {
      wk->printState = PRINTSYS_PrintStreamGetState( wk->printStream );
      if( wk->printState != PRINTSTREAM_STATE_RUNNING )
      {
        if( BTL_MAIN_GetCommMode(wk->mainModule) == BTL_COMM_NONE ){
          wk->printSeq = SEQ_WAIT_USERCTRL_NOT_COMM;
        } else {
          wk->printSeq = SEQ_WAIT_USERCTRL_COMM;
        }
        if( wk->printState == PRINTSTREAM_STATE_DONE ){
          wk->printJustDoneFlag = TRUE;
        }
      }
    }
    else
    {
      return TRUE;
    }
    break;

  case SEQ_WAIT_USERCTRL_NOT_COMM: // 待ち指定あり（非通信時）
    if( (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B))
    ||  (GFL_UI_TP_GetTrg())
    ){
      wk->printWait = 0;
    }
    if( wk->printWait ){
      wk->printWait--;
    }else{
      wk->printSeq = SEQ_AFTER_USERCTRL;
    }
    break;

  case SEQ_WAIT_USERCTRL_COMM: // 待ち指定あり（通信時）
    if( wk->printWait )
    {
      wk->printWait--;
    }
    else
    {
      wk->printSeq = SEQ_AFTER_USERCTRL;
    }
    break;

  case SEQ_AFTER_USERCTRL:
    if( wk->printState == PRINTSTREAM_STATE_DONE )
    {
      GFL_FONTSYS_SetDefaultColor();
      PRINTSYS_PrintStreamDelete( wk->printStream );
      wk->printStream = NULL;
      wk->printSeq = SEQ_DONE;
      return TRUE;
    }
    else
    {
      PRINTSYS_PrintStreamReleasePause( wk->printStream );
      wk->printSeq = SEQ_WAIT_STREAM_RUNNING;
      wk->printWait = wk->printWaitOrg;
    }
    break;

  case SEQ_DONE:
    return TRUE;
  }
  return FALSE;
}




//==============================================================================

//=============================================================================================
/**
 * ワザエフェクト発動開始
 *
 * @param   wk
 * @param   atPos
 * @param   defPos
 * @param   waza
 * @param   turnType
 * @param   continueCount
 */
//=============================================================================================
void BTLV_SCU_StartWazaEffect( BTLV_SCU* wk, BtlvMcssPos atPos, BtlvMcssPos defPos,
  WazaID waza, BtlvWazaEffect_TurnType turnType, u8 continueCount )
{
  BTLV_WAZAEFFECT_PARAM param;

  param.waza = waza;
  param.from = atPos;
  param.to = defPos;
  param.turn_count = turnType;
  param.continue_count = continueCount;

  BTLV_EFFECT_AddWazaEffect( &param );
}

//=============================================================================================
/**
 * ワザエフェクト終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL    TRUEで終了
 */
//=============================================================================================
BOOL BTLV_SCU_WaitWazaEffect( BTLV_SCU* wk )
{
  return BTLV_EFFECT_CheckExecute() == FALSE;
}

//==============================================================================
typedef struct {

  STATUS_WIN*  statWin;
  fx32    hpVal;
  fx32    hpMinusVal;
  u16     hpEnd;
  u16     timer;
  u8*     taskCounter;

}DMG_EFF_TASK_WORK;



//=============================================================================================
/**
 *
 *
 * @param   wk
 * @param   defPos
 *
 */
//=============================================================================================
void BTLV_SCU_StartWazaDamageAct( BTLV_SCU* wk, BtlPokePos defPos, WazaID wazaID )
{
  const BTL_POKEPARAM*  bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, defPos );
  int                   value = BPP_GetValue( bpp, BPP_HP );

  BTLV_EFFECT_CalcGaugeHP( BTL_MAIN_BtlPosToViewPos( wk->mainModule, defPos ), value );
  BTLV_EFFECT_Damage( BTL_MAIN_BtlPosToViewPos(wk->mainModule, defPos), wazaID );
}

//=============================================================================================
/**
 * わざエフェクト終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_SCU_WaitWazaDamageAct( BTLV_SCU* wk )
{
  return !( BTLV_EFFECT_CheckExecute() | BTLV_EFFECT_CheckExecuteGauge() );
}

//=============================================================================================
/**
 * ポケモンひんしアクション開始
 *
 * @param   wk
 * @param   pos   ひんしになったポケモンの位置ID
 *
 */
//=============================================================================================
void BTLV_SCU_StartDeadAct( BTLV_SCU* wk, BtlPokePos pos )
{
  BTLV_EFFECT_DelGauge( BTL_MAIN_BtlPosToViewPos(wk->mainModule, pos) );
  BTLV_EFFECT_Hinshi( BTL_MAIN_BtlPosToViewPos(wk->mainModule, pos) );
}

//=============================================================================================
/**
 * ポケモンひんしアクション終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_SCU_WaitDeadAct( BTLV_SCU* wk )
{
  //瀕死エフェクトが入ったらそれ待ちをいれる
  return !BTLV_EFFECT_CheckExecute();
}

//--------------------------------------------------------
// ポケモン退場アクション
//--------------------------------------------------------
typedef struct {

  STATUS_WIN*  statWin;
  u16     seq;
  u16     viewpos;
  u8*     endFlag;

}POKEOUT_ACT_WORK;


void BTLV_SCU_StartMemberOutAct( BTLV_SCU* wk, BtlvMcssPos vpos )
{
  GFL_TCBL* tcbl = GFL_TCBL_Create( wk->tcbl, taskPokeOutAct, sizeof(POKEOUT_ACT_WORK), BTLV_TASKPRI_DAMAGE_EFFECT );
  POKEOUT_ACT_WORK* twk = GFL_TCBL_GetWork( tcbl );

  twk->viewpos = vpos;
  twk->statWin = &wk->statusWin[ vpos ];
  twk->endFlag = &wk->taskCounter[TASKTYPE_DEFAULT];
  twk->seq = 0;


  *(twk->endFlag) = FALSE;
}
BOOL BTLV_SCU_WaitMemberOutAct( BTLV_SCU* wk )
{
  return wk->taskCounter[TASKTYPE_DEFAULT];
}

static void taskPokeOutAct( GFL_TCBL* tcbl, void* wk_adrs )
{
  POKEOUT_ACT_WORK* wk = wk_adrs;

  switch( wk->seq ){
  case 0:
    BTLV_EFFECT_DelGauge( wk->viewpos );
    BTLV_EFFECT_DelPokemon( wk->viewpos );
    wk->seq++;
    break;
  case 1:
    *(wk->endFlag) = TRUE;
    GFL_TCBL_Delete( tcbl );
  }
}

//--------------------------------------------------------
// ポケモン入場アクション
//--------------------------------------------------------
typedef struct {

  STATUS_WIN*  statWin;
  u16     seq;
  u16     line;
  u8*     endFlag;

}POKEIN_ACT_WORK;


//=============================================================================================
/**
 * ポケモン入場アクション開始
 *
 * @param   wk
 * @param   clientID
 *
 */
//=============================================================================================
void BTLV_SCU_StartPokeIn( BTLV_SCU* wk, BtlPokePos pos, u8 clientID, u8 memberIdx )
{
  GFL_TCBL* tcbl = GFL_TCBL_Create( wk->tcbl, taskPokeInEffect, sizeof(POKEIN_ACT_WORK), BTLV_TASKPRI_DAMAGE_EFFECT );
  POKEIN_ACT_WORK* twk = GFL_TCBL_GetWork( tcbl );

  twk->statWin = &wk->statusWin[ pos ];
  twk->endFlag = &wk->taskCounter[TASKTYPE_DEFAULT];
  twk->seq = 0;

  *(twk->endFlag) = FALSE;

  //soga
  {
    const BTL_POKEPARAM* bpp = BTL_POKECON_GetClientPokeDataConst( wk->pokeCon, clientID, memberIdx );
    {
      u8 vpos =  BTL_MAIN_BtlPosToViewPos(wk->mainModule, pos);
      const POKEMON_PARAM* pp = BPP_GetViewSrcData( bpp );
      BTL_Printf("入るポケモン  memIdx=%d, pokeID=%d, monsno=%d, monsno_src=%d\n",
            memberIdx, BPP_GetID(bpp), BPP_GetMonsNo(bpp), PP_Get(pp, ID_PARA_monsno, NULL) );
      BTLV_EFFECT_SetPokemon( pp, vpos );
    }
  }
}


BOOL BTLV_SCU_WaitPokeIn( BTLV_SCU* wk )
{
  return wk->taskCounter[TASKTYPE_DEFAULT];
}

static void taskPokeInEffect( GFL_TCBL* tcbl, void* wk_adrs )
{
  POKEIN_ACT_WORK* wk = wk_adrs;

  switch( wk->seq ){
  case 0:
    statwin_disp_start( wk->statWin );
    wk->seq++;
    break;
  case 1:
    *(wk->endFlag) = TRUE;
    GFL_TCBL_Delete( tcbl );
  }
}

//--------------------------------------------------------
// HPゲージ増減エフェクト
//--------------------------------------------------------
typedef struct {

  STATUS_WIN*  statWin;
  fx32    hpVal;
  fx32    hpAddVal;
  u16     hpEnd;
  u16     timer;
  u8*     taskCounter;

}HP_GAUGE_EFFECT;


//=============================================================================================
/**
 * HPゲージ増減エフェクト開始
 *
 * @param   wk
 * @param   pos
 *
 */
//=============================================================================================
void BTLV_SCU_StartHPGauge( BTLV_SCU* wk, BtlPokePos pos )
{
  const BTL_POKEPARAM*  bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, pos );
  int                   value = BPP_GetValue( bpp, BPP_HP );

  BTLV_EFFECT_CalcGaugeHP( BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos ), value );
}
//=============================================================================================
/**
 * HPゲージ増減エフェクト終了待ち
 *
 * @param   wk
 * @param   pos
 *
 */
//=============================================================================================
BOOL BTLV_SCU_WaitHPGauge( BTLV_SCU* wk )
{
  return !(BTLV_EFFECT_CheckExecuteGauge());
}

//=============================================================================================
/**
 * HPゲージのムーブ動作を開始
 *
 * @param   wk
 * @param   pos1
 * @param   pos2
 */
//=============================================================================================
void BTLV_SCU_MoveGauge_Start( BTLV_SCU* wk, BtlPokePos pos1, BtlPokePos pos2 )
{
  statwin_hide( &wk->statusWin[ pos1 ] );
  statwin_hide( &wk->statusWin[ pos2 ] );
  statwin_disp_start( &wk->statusWin[ pos1 ] );
  statwin_disp_start( &wk->statusWin[ pos2 ] );
}

//=============================================================================================
/**
 * HPゲージのムーブ動作 終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_SCU_MoveGauge_Wait( BTLV_SCU* wk )
{
  return TRUE;
}

//=============================================================================================
/**
 * HPゲージ更新（対象ポケモンの変更などによる）
 *
 * @param   wk
 * @param   pos
 */
//=============================================================================================
void BTLV_SCU_UpdateGauge_Start( BTLV_SCU* wk, BtlPokePos pos )
{
  statwin_hide( &wk->statusWin[ pos ] );
  statwin_disp_start( &wk->statusWin[ pos ] );
}
//=============================================================================================
/**
 * HPゲージ更新（対象ポケモンの変更などによる）終了待ち
 *
 * @param   wk
 */
//=============================================================================================
BOOL BTLV_SCU_UpdateGauge_Wait( BTLV_SCU* wk, BtlPokePos pos )
{
  return TRUE;
}

//=============================================================================================
/**
 * きのみ食べるアクション開始
 *
 * @param   wk
 * @param   pos
 */
//=============================================================================================
void BTLV_SCU_KinomiAct_Start( BTLV_SCU* wk, BtlvMcssPos pos )
{
  BTLV_EFFECT_AddByPos( pos, BTLEFF_SOUBI_ITEM );
}
//=============================================================================================
/**
 * きのみ食べるアクションの終了待ち
 *
 * @param   wk
 * @param   pos
 *
 * @retval  BOOL    終了時TRUE
 */
//=============================================================================================
BOOL BTLV_SCU_KinomiAct_Wait( BTLV_SCU* wk, BtlvMcssPos pos )
{
  return BTLV_EFFECT_CheckExecute() == FALSE;
}

//==============================================================================================
// イリュージョン解除動作
//==============================================================================================

//--------------------------------------------------------
// ポケモン入場アクション
//--------------------------------------------------------
typedef struct {

  BTLV_SCU*    parentWork;
  BtlPokePos   pos;
  BtlvMcssPos  vpos;
  u32          seq;
  u8*          pTaskCounter;

}FAKEDISABLE_ACT_WORK;


//=============================================================================================


/**
 *  イリュージョン解除動作：開始
 */
void BTLV_SCU_FakeDisable_Start( BTLV_SCU* wk, BtlPokePos pos )
{
  GFL_TCBL* tcbl = GFL_TCBL_Create( wk->tcbl, taskFakeDisable, sizeof(FAKEDISABLE_ACT_WORK), BTLV_TASKPRI_DAMAGE_EFFECT );
  FAKEDISABLE_ACT_WORK* twk = GFL_TCBL_GetWork( tcbl );

  twk->parentWork = wk;
  twk->pos = pos;
  twk->vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos );
  twk->pTaskCounter = &wk->taskCounter[TASKTYPE_DEFAULT];
  twk->seq = 0;

  (*(twk->pTaskCounter))++;
}

/**
 *  イリュージョン解除動作：終了待ち
 */
BOOL BTLV_SCU_FakeDisable_Wait( BTLV_SCU* wk )
{
  return ( wk->taskCounter[ TASKTYPE_DEFAULT ] == 0 );
}

/**
 *  イリュージョン解除動作：実行タスク
 */
static void taskFakeDisable( GFL_TCBL* tcbl, void* wk_adrs )
{
  FAKEDISABLE_ACT_WORK* wk = wk_adrs;

  // @todo 今はただ消して, 出してるだけです
  switch( wk->seq ){
  case 0:
    BTLV_EFFECT_DelPokemon( wk->vpos );
    BTLV_EFFECT_DelGauge( wk->vpos );
    wk->seq++;
    break;
  case 1:
    {
      const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( wk->parentWork->pokeCon, wk->pos );
      const POKEMON_PARAM* pp = BPP_GetSrcData( bpp );
      BTLV_EFFECT_SetPokemon( pp, wk->vpos );
      BTLV_EFFECT_SetGauge( wk->parentWork->mainModule, bpp, wk->vpos );
      wk->seq++;
    }
  case 2:
    if( !BTLV_EFFECT_CheckExecute() ){
      (*(wk->pTaskCounter))--;
      GFL_TCBL_Delete( tcbl );
    }
    break;
  }
}

//==============================================================================================
// メッセージウィンドウ表示・消去処理
//==============================================================================================
static void msgWinVisible_Init( MSGWIN_VISIBLE* wk, GFL_BMPWIN* win )
{
  wk->win = win;
  wk->state = MSGWIN_STATE_DISP;
  wk->eva = FX32_CONST( MSGWIN_EVA_MAX );
  wk->evb = FX32_CONST( MSGWIN_EVB_MIN );

  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,
                    GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 |
                    GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
                    MSGWIN_EVA_MAX, MSGWIN_EVB_MIN );
}
static void msgWinVisible_Hide( MSGWIN_VISIBLE* wk )
{
  if( wk->state == MSGWIN_STATE_DISP
  ||  wk->state == MSGWIN_STATE_TO_DISP
  ){
    wk->eva = FX32_CONST( MSGWIN_EVA_MAX );
    wk->evb = FX32_CONST( MSGWIN_EVB_MIN );
    wk->eva_step = FX32_CONST( MSGWIN_EVA_MIN - MSGWIN_EVA_MAX ) / MSGWIN_VISIBLE_STEP;
    wk->evb_step = FX32_CONST( MSGWIN_EVB_MAX - MSGWIN_EVB_MIN ) / MSGWIN_VISIBLE_STEP;
    wk->timer = MSGWIN_VISIBLE_STEP;
    wk->state = MSGWIN_STATE_TO_HIDE;
  }
}
static void msgWinVisible_Disp( MSGWIN_VISIBLE* wk )
{
  if( wk->state == MSGWIN_STATE_HIDE
  ||  wk->state == MSGWIN_STATE_TO_HIDE
  ){
    GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( wk->win );
    GFL_BMP_Clear( bmp, COLIDX_MSGWIN_CLEAR );
    GFL_BMPWIN_TransVramCharacter( wk->win );

    wk->eva = FX32_CONST( MSGWIN_EVA_MIN );
    wk->evb = FX32_CONST( MSGWIN_EVB_MAX );
    wk->eva_step = FX32_CONST( MSGWIN_EVA_MAX - MSGWIN_EVA_MIN ) / MSGWIN_VISIBLE_STEP;
    wk->evb_step = FX32_CONST( MSGWIN_EVB_MIN - MSGWIN_EVB_MAX ) / MSGWIN_VISIBLE_STEP;
    wk->timer = MSGWIN_VISIBLE_STEP;
    wk->state = MSGWIN_STATE_TO_DISP;
  }
}
static BOOL msgWinVisible_Update( MSGWIN_VISIBLE* wk )
{
  switch( wk->state ){
  case MSGWIN_STATE_HIDE:
    return TRUE;
  case MSGWIN_STATE_DISP:
    return TRUE;
  case MSGWIN_STATE_TO_DISP:
    if( wk->timer ){
      wk->eva += wk->eva_step;
      wk->evb += wk->evb_step;
      wk->timer--;
    }else{
      wk->eva = FX32_CONST( MSGWIN_EVA_MAX );
      wk->evb = FX32_CONST( MSGWIN_EVB_MIN );
      wk->state = MSGWIN_STATE_DISP;
    }
    break;
  case MSGWIN_STATE_TO_HIDE:
    if( wk->timer ){
      wk->eva += wk->eva_step;
      wk->evb += wk->evb_step;
      wk->timer--;
    }else{
      wk->eva = FX32_CONST( MSGWIN_EVA_MIN );
      wk->evb = FX32_CONST( MSGWIN_EVB_MAX );
      wk->state = MSGWIN_STATE_HIDE;
    }
    break;
  }

  {
    u8 eva = wk->eva >> FX32_SHIFT;
    u8 evb = wk->evb >> FX32_SHIFT;

    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,
                    GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 |
                    GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
                    eva, evb );
  }
  return FALSE;
}
//----------------------------

static void statwin_setupAll( BTLV_SCU* wk )
{
  u32 pos_end;
  u32 i;

  for(i=0; i<=NELEMS(wk->statusWin); ++i)
  {
    wk->statusWin[i].enableFlag = FALSE;
    wk->statusWin[i].bpp = NULL;
  }

  pos_end = BTL_MAIN_GetEnablePosEnd( wk->mainModule );
  for(i=0; i<=pos_end; ++i){
    statwin_setup( &wk->statusWin[ i ], wk, i );
  }
}

static void statwin_cleanupAll( BTLV_SCU* wk )
{
  int i;
  for(i=0; i<NELEMS(wk->statusWin); i++){
    statwin_cleanup( &wk->statusWin[i] );
  }
}

static void statwin_setup( STATUS_WIN* stwin, BTLV_SCU* wk, BtlPokePos pokePos )
{
  stwin->pokePos = pokePos;
  stwin->vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pokePos );
  stwin->parentWk = wk;
  stwin->dispFlag = FALSE;
  stwin->enableFlag = TRUE;
  stwin->bpp = NULL;
}

static void statwin_cleanup( STATUS_WIN* stwin )
{
  if( stwin->enableFlag )
  {
    if( stwin->dispFlag ){
      BTLV_EFFECT_DelGauge( stwin->vpos );
      stwin->dispFlag = FALSE;
    }
    stwin->bpp = NULL;
    stwin->enableFlag = FALSE;
  }
}

static void statwin_hide( STATUS_WIN* stwin )
{
  if( stwin->dispFlag ){
    BTLV_EFFECT_DelGauge( stwin->vpos );
    stwin->dispFlag = FALSE;
  }
}

static void statwin_disp_start( STATUS_WIN* stwin )
{
  if( stwin->enableFlag )
  {
    stwin->bpp = BTL_POKECON_GetFrontPokeDataConst( stwin->parentWk->pokeCon, stwin->pokePos );
    if( !BPP_IsDead(stwin->bpp) )
    {
      BTL_Printf("ここでポケID=%d, HP=%d pos=%d\n", BPP_GetID(stwin->bpp), BPP_GetValue(stwin->bpp,BPP_HP), stwin->pokePos);
      BTLV_EFFECT_SetGauge( stwin->parentWk->mainModule, stwin->bpp, stwin->vpos );
      stwin->dispFlag = TRUE;
    }
  }
  else
  {
    GF_ASSERT(0);
  }
}


//----------------------------------------------------------------------------------------------
// とくせいウィンドウ
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
/**
 * 戦闘位置 -> とくせいウィンドウ位置へ変換
 *
 * @param   mainModule
 * @param   pos
 *
 * @retval  TokwinSide
 */
//----------------------------------------------------------------------------------
static TokwinSide PokePosToTokwinSide( const BTL_MAIN_MODULE* mainModule, BtlPokePos pos )
{
  BtlSide btlSide = BTL_MAINUTIL_PosToSide( pos );
  return BTL_MAIN_IsPlayerSide( mainModule, btlSide )? TOKWIN_SIDE_FRIEND : TOKWIN_SIDE_ENEMY;
}

//=============================================================================================
/**
 * とくせいウィンドウ表示開始
 *
 * @param   wk
 * @param   pos
 * @param   fFlash    効果発動を示すフラッシュ動作を入れるか?
 */
//=============================================================================================
void BTLV_SCU_TokWin_DispStart( BTLV_SCU* wk, BtlPokePos pos, BOOL fFlash )
{
  TokwinSide side = PokePosToTokwinSide( wk->mainModule, pos );
  BU_Printf( PRINT_FLG, "[SCU] TOKWIN DISP ... pos=%d, side=%d\n", pos, side);
  tokwin_disp_first( &wk->tokWin[side], pos, fFlash );
}
BOOL BTLV_SCU_TokWin_DispWait( BTLV_SCU* wk, BtlPokePos pos )
{
  TokwinSide side = PokePosToTokwinSide( wk->mainModule, pos );
  return tokwin_disp_progress( &wk->tokWin[side] );
}
//=============================================================================================
/**
 * とくせいウィンドウ表示オフ
 *
 * @param   wk
 * @param   clientID
 *
 */
//=============================================================================================
void BTLV_SCU_TokWin_HideStart( BTLV_SCU* wk, BtlPokePos pos )
{
  TokwinSide side = PokePosToTokwinSide( wk->mainModule, pos );
  BU_Printf( PRINT_FLG, "[SCU] TOKWIN HIDE ... pos=%d, side=%d\n", pos, side);
  tokwin_hide_first( &wk->tokWin[side] );
}
BOOL BTLV_SCU_TokWin_HideWait( BTLV_SCU* wk, BtlPokePos pos )
{
  TokwinSide side = PokePosToTokwinSide( wk->mainModule, pos );
  return tokwin_hide_progress( &wk->tokWin[side] );
}
//=============================================================================================
/**
 * とくせいウィンドウの内容更新（開始）
 *
 * @param   wk
 * @param   pos
 *
 */
//=============================================================================================
void BTLV_SCU_TokWin_Renew_Start( BTLV_SCU* wk, BtlPokePos pos )
{
  TokwinSide side = PokePosToTokwinSide( wk->mainModule, pos );
  tokwin_renew_start( &wk->tokWin[side], pos );
}
//=============================================================================================
/**
 * とくせいウィンドウの内容更新（終了待ち）
 *
 * @param   wk
 * @param   pos
 *
 * @retval  BOOL    終了したらTRUE
 */
//=============================================================================================
BOOL BTLV_SCU_TokWin_Renew_Wait( BTLV_SCU* wk, BtlPokePos pos )
{
  TokwinSide side = PokePosToTokwinSide( wk->mainModule, pos );
  return tokwin_renew_progress( &wk->tokWin[side] );
}

static void tokwin_setupAll( BTLV_SCU* wk )
{
  u32 i;

  wk->tokWinCgrHead = GFL_ARC_UTIL_LoadBGCharacter( ARCID_BATTGRA, NARC_battgra_wb_tokusei_w_NCGR,
                        FALSE, &wk->tokWinCgr, wk->heapID );

  for(i=0; i<NELEMS(wk->tokWin); ++i)
  {
    wk->tokWin[i].parentWk = wk;
    wk->tokWin[i].mySide = i;
    wk->tokWin[i].bgFrame = (i == TOKWIN_SIDE_FRIEND)? BGFRAME_TOKWIN_FRIEND : BGFRAME_TOKWIN_ENEMY;
    wk->tokWin[i].cgrSrc = ((const u8*)wk->tokWinCgr->pRawData) + TOKWIN_CGRDATA_TRANS_SIZE * i;
    wk->tokWin[i].bmp = GFL_BMP_Create( TOKWIN_CGRDATA_CHAR_W, TOKWIN_DRAWAREA_CHAR_H, GFL_BMP_16_COLOR, wk->heapID );
    wk->tokWin[i].tokusei = POKETOKUSEI_NULL;
    wk->tokWin[i].pokeID = BTL_POKEID_NULL;
    wk->tokWin[i].dispFlag = FALSE;
  }
}
static void tokwin_cleanupAll( BTLV_SCU* wk )
{
  int i;
  for(i=0; i<NELEMS(wk->tokWin); i++)
  {
    if( wk->tokWin[i].bmp != NULL ){
      GFL_BMP_Delete( wk->tokWin[i].bmp );
      wk->tokWin[i].bmp = NULL;
    }
  }
  GFL_HEAP_FreeMemory( wk->tokWinCgrHead );
}
// 表示開始
static void tokwin_disp_first( TOK_WIN* tokwin, BtlPokePos pos, BOOL fFlash )
{
  if( tokwin->dispFlag == FALSE )
  {
    const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( tokwin->parentWk->pokeCon, pos );
    u16 tokusei = BPP_GetValue( bpp, BPP_TOKUSEI );
    u8  pokeID = BPP_GetID( bpp );

    if( (tokusei != tokwin->tokusei) || (pokeID != tokwin->pokeID) )
    {
      tokwin->tokusei = tokusei;
      tokwin->pokeID = pokeID;
      tokwin_update_cgr( tokwin );
    }
    tokwin->fFlash = fFlash;
    tokwin->seq = 0;
  }
}
// 表示更新
static BOOL tokwin_disp_progress( TOK_WIN* tokwin )
{
  switch( tokwin->seq ){
  case 0:
    GFL_BG_LoadCharacter( tokwin->bgFrame, tokwin->cgrSrc,
        TOKWIN_CGRDATA_TRANS_SIZE, TOKWIN_CGRDATA_TRANS_SIZE * tokwin->mySide );
    tokwin->seq++;
    break;
  case 1:
    tokwin->moveTimer = TOKWIN_MOVE_FRAMES;
    if( tokwin->mySide == TOKWIN_SIDE_FRIEND )
    {
      tokwin->posX = FX32_CONST( TOKWIN_HIDEPOS_FRIEND );
      tokwin->speedX = FX32_CONST( -TOKWIN_HIDEPOS_FRIEND ) / TOKWIN_MOVE_FRAMES;
    }
    else
    {
      tokwin->posX = FX32_CONST( TOKWIN_HIDEPOS_ENEMY );
      tokwin->speedX = FX32_CONST( -TOKWIN_HIDEPOS_ENEMY ) / TOKWIN_MOVE_FRAMES;
    }
    PMSND_PlaySE( WAVE_SE_WB_SHOOTER );  // @TODO 仮
    tokwin->seq++;
    break;
  case 2:
    if( tokwin->moveTimer )
    {
      int x;
      tokwin->posX += tokwin->speedX;
      x = tokwin->posX >> FX32_SHIFT;
      GFL_BG_SetScroll( tokwin->bgFrame, GFL_BG_SCROLL_X_SET, x );
      tokwin->moveTimer--;
    }
    else
    {
      GFL_BG_SetScroll( tokwin->bgFrame, GFL_BG_SCROLL_X_SET, 0 );
      tokwin->writeRaw = 0;
      tokwin->seq++;
    }
    break;
  case 3:
    if( tokwin->writeRaw < TOKWIN_DRAWAREA_CHAR_W )
    {
      const u8* cp = GFL_BMP_GetCharacterAdrs( tokwin->bmp );
      u32 offs = tokwin->writeRaw * 0x20;
      u32 writeCharOfs = (tokwin->mySide * TOKWIN_CGRDATA_TRANS_CHARS) + tokwin->writeRaw;
      u32 i;
      cp += offs;
      for(i=0; i<TOKWIN_DRAWAREA_CHAR_H; ++i)
      {
        GFL_BG_LoadCharacter( tokwin->bgFrame, cp+TOKWIN_CGRDATA_LINE_SIZE*i, 0x20,
              writeCharOfs+TOKWIN_CGRDATA_CHAR_W*i );
      }
      tokwin->writeRaw++;
    }
    else
    {
      tokwin->dispFlag = TRUE;
      if( tokwin->fFlash ){
        tokwin->seq++;
      }else{
        return TRUE;
      }
    }
    break;
  case 4:
    {
      PALETTE_FADE_PTR pfd = BTLV_EFFECT_GetPfd();
      GFL_TCBSYS* tcbSys = BTLV_EFFECT_GetTCBSYS();
      u16 palBit = 1 << (PALIDX_TOKWIN1 + tokwin->mySide );

      PMSND_PlaySE( BANK_SE_WB_DECIDE2 );  // @TODO 仮
      PaletteFadeReq( pfd, PF_BIT_MAIN_BG, palBit, 0, 16, 0, 0x7fff, tcbSys );
      tokwin->seq++;
    }
    break;
  case 5:
    {
      PALETTE_FADE_PTR pfd = BTLV_EFFECT_GetPfd();

      if( PaletteFadeCheck(pfd) == 0){
        tokwin->seq++;
      }
    }
    break;
  case 6:
    if(  !PMSND_CheckPlaySE() ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}

// CGR更新
static void tokwin_update_cgr( TOK_WIN* tokwin )
{
  BTLV_SCU* wk = tokwin->parentWk;

  // 自前CGR領域を元データで初期化する
  {
    u32 bmp_size = GFL_BMP_GetBmpDataSize( tokwin->bmp );
    u8* dst_p = GFL_BMP_GetCharacterAdrs( tokwin->bmp );
    u8* src_p = ((u8*)(tokwin->parentWk->tokWinCgr->pRawData)) + (tokwin->mySide * TOKWIN_CGRDATA_TRANS_SIZE);
    GFL_STD_MemCopy32( src_p, dst_p, bmp_size );
  }

  {
    u32 width[ 2 ];
    u8  lines;

    // ２行ある場合、文字列をイイ感じに配置する
    BTL_STR_MakeStringStd( wk->strBufSubA, BTL_STRID_STD_TokWin, 2, tokwin->pokeID, tokwin->tokusei );
    lines = PRINTSYS_GetStrLineWidth( wk->strBufSubA, wk->smallFont, 0, width, NELEMS(width) );
    if( lines > 1 )
    {
      int line1_end  = TOKWIN_MARGIN_L + width[0];
      int line2_head = TOKWIN_DRAWAREA_WIDTH - TOKWIN_MARGIN_R - width[1];
      int diff = line1_end - line2_head;
      if( diff < TOKWIN_LINE_CROSS_WIDTH )
      {
        diff = TOKWIN_LINE_CROSS_WIDTH - diff;
        line1_end += (diff / 2);
        line2_head -= (diff - (diff / 2));
      }

      {
        int line1_head = line1_end - width[0];
        if( line1_head < 0 ){ line1_head = 0; }
        if( line2_head < 0 ){ line2_head = 0; }

        PRINTSYS_CopyLineStr( wk->strBufSubA, wk->strBufSubB, 0 );
        PRINTSYS_Print( tokwin->bmp, line1_head, TOKWIN_DRAWAREA_LINE1_OY, wk->strBufSubB, wk->smallFont );

        PRINTSYS_CopyLineStr( wk->strBufSubA, wk->strBufSubB, 1 );
        PRINTSYS_Print( tokwin->bmp, line2_head, TOKWIN_DRAWAREA_LINE2_OY, wk->strBufSubB, wk->smallFont );
      }
    }
    // １行なら単純なセンタリング表示
    else
    {
      int xpos = (TOKWIN_DRAWAREA_WIDTH - PRINTSYS_GetStrWidth(wk->strBufSubA, wk->smallFont, 0)) / 2;
      if( xpos < 0 ){
        xpos = 0;
      }
      PRINTSYS_Print( tokwin->bmp, xpos, TOKWIN_DRAWAREA_LINE2_OY, wk->strBufSubA, wk->smallFont );
    }
  }
}

static void tokwin_hide_first( TOK_WIN* tokwin )
{
  if( tokwin->mySide == TOKWIN_SIDE_FRIEND ){
    GFL_BG_SetScroll( tokwin->bgFrame, GFL_BG_SCROLL_X_SET, TOKWIN_HIDEPOS_FRIEND );
  }else{
    GFL_BG_SetScroll( tokwin->bgFrame, GFL_BG_SCROLL_X_SET, TOKWIN_HIDEPOS_ENEMY );
  }
  tokwin->dispFlag = FALSE;
}
static BOOL tokwin_hide_progress( TOK_WIN* tokwin )
{
  return TRUE;
}

static void tokwin_renew_start( TOK_WIN* tokwin, BtlPokePos pos )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( tokwin->parentWk->pokeCon, pos );
  u16 tokusei = BPP_GetValue( bpp, BPP_TOKUSEI );
  u8  pokeID = BPP_GetID( bpp );

  if( (tokusei != tokwin->tokusei) || (pokeID != tokwin->pokeID) )
  {
    tokwin->tokusei = tokusei;
    tokwin->pokeID = pokeID;
    tokwin_update_cgr( tokwin );
  }
  tokwin->seq = 0;
}
static BOOL tokwin_renew_progress( TOK_WIN* tokwin )
{
  enum {
    MOSAIC_WAIT_FRAMES = 1,
  };
  switch( tokwin->seq ){
  case 0:
    G2_SetBGMosaicSize( 0, 0 );
    if( tokwin->mySide == TOKWIN_SIDE_FRIEND ){
      G2_BG2Mosaic( TRUE );
    }else{
      G2_BG3Mosaic( TRUE );
    }
    tokwin->moveTimer = 0;
    tokwin->writeRaw = 0;
    PMSND_PlaySE( BANK_SE_WB_DECIDE3 );  // @TODO 仮
    tokwin->seq++;
    break;
  case 1:
    tokwin->moveTimer++;
    if( tokwin->moveTimer > MOSAIC_WAIT_FRAMES )
    {
      tokwin->moveTimer = 0;
      tokwin->writeRaw++;
      G2_SetBGMosaicSize( tokwin->writeRaw, tokwin->writeRaw );
      if( tokwin->writeRaw >= 15 ){
        tokwin->seq++;
      }
    }
    break;
  case 2:
    {
      const u8* cp = GFL_BMP_GetCharacterAdrs( tokwin->bmp );
      GFL_BG_LoadCharacter( tokwin->bgFrame, cp, TOKWIN_CGRDATA_TRANS_SIZE, tokwin->mySide*TOKWIN_CGRDATA_TRANS_CHARS );
      tokwin->seq++;
    }
    break;
  case 3:
    tokwin->moveTimer++;
    if( tokwin->moveTimer > MOSAIC_WAIT_FRAMES )
    {
      tokwin->moveTimer = 0;
      tokwin->writeRaw--;
      G2_SetBGMosaicSize( tokwin->writeRaw, tokwin->writeRaw );
      if( tokwin->writeRaw == 0 ){
        tokwin->seq++;
      }
    }
    break;
  case 4:
    if( tokwin->mySide == TOKWIN_SIDE_FRIEND ){
      G2_BG2Mosaic( FALSE );
    }else{
      G2_BG3Mosaic( FALSE );
    }
    (tokwin->seq++);
    break;
  case 5:
    if(  !PMSND_CheckPlaySE() ){
      tokwin->seq++;
    }
    break;
  default:
    return TRUE;
  }
  return FALSE;
}


//=============================================================================================
/**
 * @brief ボールゲージセットアップパラメータ生成
 *
 * @param[in]   wk
 * @param[out]  bbgp  セットアップパラメータ構造体のポインタ
 * @param[in]   pos   セットアップするポケモンの立ち位置
 * @param[in]   type  ボールゲージタイプ
 *
 */
//=============================================================================================
static void bbgp_make( BTLV_SCU* wk, BTLV_BALL_GAUGE_PARAM* bbgp, u8 clientID, BTLV_BALL_GAUGE_TYPE type )
{
  int i;
  const BTL_PARTY*      bparty = BTL_POKECON_GetPartyDataConst( wk->pokeCon, clientID );
  const BTL_POKEPARAM*  bpp;
  int count = BTL_PARTY_GetMemberCount( bparty );

  bbgp->type = type;

  for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
  {
    if( i < count )
    {
      bpp = BTL_PARTY_GetMemberDataConst( bparty, i );
      if( BPP_IsDead( bpp ) )
      {
        bbgp->status[ i ] = BTLV_BALL_GAUGE_STATUS_DEAD;
      }
      else if( BPP_GetPokeSick( bpp ) != POKESICK_NULL )
      {
        bbgp->status[ i ] = BTLV_BALL_GAUGE_STATUS_NG;
      }
      else
      {
        bbgp->status[ i ] = BTLV_BALL_GAUGE_STATUS_ALIVE;
      }
    }
    else
    {
      bbgp->status[ i ] = BTLV_BALL_GAUGE_STATUS_NONE;
    }
  }
}


//=============================================================================================
//  「通信待機中」表示オン・オフ
//=============================================================================================
void BTLV_SCU_StartCommWaitInfo( BTLV_SCU* wk )
{
  BTL_STR_GetUIString( wk->strBufMain, BTLSTR_UI_COMM_WAIT );
  BTLV_SCU_StartMsg( wk, wk->strBufMain, 0 );
}

BOOL BTLV_SCU_WaitCommWaitInfo( BTLV_SCU* wk )
{
  return BTLV_SCU_WaitMsg( wk );
}
void BTLV_SCU_ClearCommWaitInfo( BTLV_SCU* wk )
{
//  GFL_BMP_Clear( wk->bmp, 0x0f );
//  GFL_BMPWIN_TransVramCharacter( wk->win );
}

