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

#include "poke_tool/monsno_def.h"
#include "print/printsys.h"
#include "gamesystem/msgspeed.h"
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


/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  TEST_STATWIN_BGCOL = 7,
  TEST_STATWIN_BGCOL_FLASH = 4,
  TEST_TOKWIN_BGCOL = 6,
  TEST_TOKWIN_CHAR_WIDTH = 10,
  TEST_TOKWIN_DOT_WIDTH  = TEST_TOKWIN_CHAR_WIDTH*8,

  STRBUF_LEN = 512,
  SUBPROC_WORK_SIZE = 64,

  PALIDX_MSGWIN     = 0,
  PALIDX_POKEWIN    = 1,

  COLIDX_MSGWIN_CLEAR  = 0x0c,
  COLIDX_MSGWIN_LETTER = 0x01,
  COLIDX_MSGWIN_SHADOW = 0x09,


  MSGWIN_EVA_MAX = 31,
  MSGWIN_EVA_MIN = 0,
  MSGWIN_EVB_MIN = 3,
  MSGWIN_EVB_MAX = 16,
  MSGWIN_VISIBLE_STEP = 6,
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

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BOOL (*BtlinEffectSeq)( BTLV_SCU*, int* );

/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/
typedef struct {
  GFL_BMPWIN*       win;
  GFL_BMP_DATA*     bmp;
  const BTL_POKEPARAM*  bpp;
  BTLV_SCU*         parentWk;
  u16               hp;
  u8                pokePos;
  u8                vpos;
  u8                dispFlag;
}STATUS_WIN;

typedef struct {
  GFL_BMPWIN*     win;
  GFL_BMP_DATA*   bmp;
  u16             tokusei;
  u8              pokeID;
  u8              pokePos;
  u8              renewingFlag;
  BTLV_SCU*     parentWk;
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
  GFL_TCBLSYS*    tcbl;
  PRINT_STREAM*      printStream;
  STRBUF*            strBuf;

  u8              taskCounter[TASKTYPE_MAX];


  STATUS_WIN    statusWin[ BTL_POS_MAX ];
  TOK_WIN       tokWin[ BTL_POS_MAX ];

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
static void tokwinRenewTask( GFL_TCBL* tcbl, void* wk_adrs );
static void taskDamageEffect( GFL_TCBL* tcbl, void* wk_adrs );
static void taskPokeOutAct( GFL_TCBL* tcbl, void* wk_adrs );
static void taskPokeInEffect( GFL_TCBL* tcbl, void* wk_adrs );
static void taskHPGauge( GFL_TCBL* tcbl, void* wk_adrs );
static void msgWinVisible_Init( MSGWIN_VISIBLE* wk, GFL_BMPWIN* win );
static void msgWinVisible_Hide( MSGWIN_VISIBLE* wk );
static void msgWinVisible_Disp( MSGWIN_VISIBLE* wk );
static BOOL msgWinVisible_Update( MSGWIN_VISIBLE* wk );
static void statwin_setupAll( BTLV_SCU* wk );
static void statwin_cleanupAll( BTLV_SCU* wk );
static void tokwin_setupAll( BTLV_SCU* wk );
static void tokwin_cleanupAll( BTLV_SCU* wk );
static void statwin_setup( STATUS_WIN* stwin, BTLV_SCU* wk, BtlPokePos pokePos );
static void statwin_cleanup( STATUS_WIN* stwin );
static void statwin_reset_data( STATUS_WIN* stwin );
static void statwin_disp_start( STATUS_WIN* stwin );
static void statwin_disp( STATUS_WIN* stwin );
static BOOL statwin_erase( STATUS_WIN* stwin, u8 line );
static void statwin_update( STATUS_WIN* stwin, u16 hp, u8 col );
static void tokwin_setup( TOK_WIN* tokwin, BTLV_SCU* wk, BtlPokePos pos );
static void tokwin_update_cgx( TOK_WIN* tokwin );
static void tokwin_check_update_cgx( TOK_WIN* tokwin );
static void tokwin_cleanup( TOK_WIN* tokwin );
static void tokwin_disp_first( TOK_WIN* tokwin );
static void tokwin_hide( TOK_WIN* tokwin );
static void tokwin_disp( TOK_WIN* tokwin );
static void bbgp_make( BTLV_SCU* wk, BTLV_BALL_GAUGE_PARAM* bbgp, u8 clientID, BTLV_BALL_GAUGE_TYPE type );



BTLV_SCU*  BTLV_SCU_Create( const BTLV_CORE* vcore,
  const BTL_MAIN_MODULE* mainModule, const BTL_POKE_CONTAINER* pokeCon,
  GFL_TCBLSYS* tcbl, GFL_FONT* defaultFont, u8 playerClientID, HEAPID heapID )
{
  BTLV_SCU* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTLV_SCU) );

  wk->vcore = vcore;
  wk->mainModule = mainModule;
  wk->pokeCon = pokeCon;
  wk->heapID = heapID;
  wk->playerClientID = playerClientID;
  wk->defaultFont = defaultFont;

  wk->printStream = NULL;
  wk->tcbl = tcbl;
  wk->strBuf = GFL_STR_CreateBuffer( STRBUF_LEN, heapID );
  wk->msgwinVisibleFlag = FALSE;

  wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
  PRINT_UTIL_Setup( &wk->printUtil, wk->win );

  return wk;
}

void BTLV_SCU_Setup( BTLV_SCU* wk )
{
  // 個別フレーム設定
  static const GFL_BG_BGCNT_HEADER bgcntText = {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x10000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };
  // 個別フレーム設定
  static const GFL_BG_BGCNT_HEADER bgcntTok = {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x08000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };
  // 個別フレーム設定
  static const GFL_BG_BGCNT_HEADER bgcntStat = {
    0, 0, 0x2000, 0,
    GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000, 0x8000,
    GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
  };
  u32 winfrm_charpos;

  GFL_BG_SetBGControl( GFL_BG_FRAME1_M,   &bgcntText,   GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( GFL_BG_FRAME2_M,   &bgcntTok,   GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( GFL_BG_FRAME3_M,   &bgcntStat,   GFL_BG_MODE_TEXT );

//  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
  PaletteWorkSetEx_Arc( BTLV_EFFECT_GetPfd(), ARCID_FONT, NARC_font_default_nclr, wk->heapID, FADE_MAIN_BG, 0x20,
        PALIDX_POKEWIN*16, 0 );

  PaletteWorkSetEx_Arc( BTLV_EFFECT_GetPfd(), ARCID_BATTGRA, NARC_battgra_wb_msgwin_frm_NCLR, wk->heapID, FADE_MAIN_BG, 0x20,
        PALIDX_MSGWIN*16, 0 );

  GFL_BG_FillScreen( GFL_BG_FRAME1_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
  GFL_BG_FillCharacter( GFL_BG_FRAME1_M, 0x00, 1, 0 );
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, GFL_HEAP_LOWID(wk->heapID) );
    GFL_ARCUTIL_TRANSINFO transInfo;
    transInfo = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( handle, NARC_battgra_wb_msgwin_frm_NCGR,
                GFL_BG_FRAME1_M, 0, FALSE, GFL_HEAP_LOWID(wk->heapID) );
    winfrm_charpos = GFL_ARCUTIL_TRANSINFO_GetPos( transInfo );
    GFL_ARC_CloseDataHandle( handle );
  }

  GFL_BG_FillCharacter( GFL_BG_FRAME3_M, 0x00, 1, 0 );
  GFL_BG_FillCharacter( GFL_BG_FRAME3_M, 0xff, 1, 1 );
  GFL_BG_FillScreen( GFL_BG_FRAME2_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
//  GFL_BG_FillScreen( GFL_BG_FRAME3_M, 0x0001, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
  GFL_BG_FillScreen( GFL_BG_FRAME3_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );


  wk->win = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 1, 19, 30, 4, PALIDX_MSGWIN, GFL_BMP_CHRAREA_GET_F );
  wk->bmp = GFL_BMPWIN_GetBmp( wk->win );
  GFL_BMPWIN_MakeScreen( wk->win );

//    GFL_BG_FillCharacter( GFL_BG_FRAME1_M, 0xaa, 9, 1 );
  GFL_BMPWIN_MakeFrameScreen( wk->win, winfrm_charpos, PALIDX_MSGWIN );

  GFL_BMP_Clear( wk->bmp, COLIDX_MSGWIN_CLEAR );
  GFL_BMPWIN_TransVramCharacter( wk->win );

  msgWinVisible_Init( &wk->msgwinVisibleWork, wk->win );

  statwin_setupAll( wk );
  tokwin_setupAll( wk );

  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME3_M );

//  GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
  GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
  GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_ON  );
  GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_ON  );
  GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_ON );
  ///<obj
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

void BTLV_SCU_Delete( BTLV_SCU* wk )
{
  tokwin_cleanupAll( wk );
  statwin_cleanupAll( wk );

  GFL_BMPWIN_Delete( wk->win );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );

  PRINTSYS_QUE_Delete( wk->printQue );
  GFL_STR_DeleteBuffer( wk->strBuf );

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
  // @@@ いずれはトレーナー戦かどうかなどでも判定を別ける必要あり
  BtlCompetitor  comp = BTL_MAIN_GetCompetitor( wk->mainModule );

  wk->btlinSeq = 0;

  switch( BTL_MAIN_GetRule(wk->mainModule) ){
  case BTL_RULE_SINGLE:
    switch( comp ){
    case BTL_COMPETITOR_WILD:     // 野生
    default:
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_wild_single );
      break;
    case BTL_COMPETITOR_TRAINER:  // ゲーム内トレーナー
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_trainer_single );
      break;
    case BTL_COMPETITOR_COMM:
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_comm_single );
      break;
    }
    break;

  case BTL_RULE_DOUBLE:
    switch( comp ){
    case BTL_COMPETITOR_WILD:     // 野生
    default:
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_wild_double );
      break;

    case BTL_COMPETITOR_TRAINER:  // ゲーム内トレーナー
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
    if( BTL_MAIN_GetCompetitor( wk->mainModule ) == BTL_COMPETITOR_TRAINER){   ///< ゲーム内トレーナー
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_trainer_triple );
    }else{
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_comm_triple ); // @@@ 通信用つくるよ
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
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->pp), subwk->viewPos );
      BTLV_EFFECT_AddByPos( subwk->viewPos, BTLEFF_SINGLE_ENCOUNT_1 );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );
      (*seq)++;
    }
    break;
  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_Encount_Wild1, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
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

      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_PutSingle, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 6:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->pp), subwk->viewPos );
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
      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_Encount_NPC1, 1, subwk->clientID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
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
      TrainerID  trID = BTL_MAIN_GetTrainerID( wk->mainModule );
      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_PutSingle_NPC, 2, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 5:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->pp), subwk->viewPos );
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

      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_PutSingle, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 9:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->pp), subwk->viewPos );
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

      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_Encount_Wild2, 2, subwk[0].pokeID, subwk[1].pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[0].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk[0].pp), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk[0].pos ] );
      (*seq)++;
    }
    break;
  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[1].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk[1].pp), viewPos );
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
      subwk[1].pos = BTL_MAIN_GetClientPokePos( wk->mainModule, wk->playerClientID, 1 );
      subwk[1].pp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, subwk[1].pos );
      subwk[1].pokeID = BPP_GetID( subwk[1].pp );
      if( !BPP_IsDead(subwk[1].pp) ){
        BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_PutDouble, 2, subwk[0].pokeID, subwk[1].pokeID );
      }else{
        BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_PutSingle, 1, subwk[0].pokeID );
      }
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 4:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[0].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk[0].pp), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk[0].pos ] );
      (*seq)++;
    }
    break;
  case 5:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      if( !BPP_IsDead(subwk[1].pp) ){
        u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[1].pos );
        BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk[1].pp), viewPos );
        statwin_disp_start( &wk->statusWin[ subwk[1].pos ] );
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

      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_Encount_Wild2, 2, subwk[0].pokeID, subwk[1].pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 2 );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[0].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk[0].pp), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk[0].pos ] );
      (*seq)++;
    }
    break;
  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[1].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk[1].pp), viewPos );
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

      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_PutDouble, 2, subwk[0].pokeID, subwk[1].pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;
  case 4:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[0].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk[0].pp), viewPos );
      statwin_disp_start( &wk->statusWin[ subwk[0].pos ] );
      (*seq)++;
    }
    break;
  case 5:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u8 viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk[1].pos );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk[1].pp), viewPos );
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

      u16 strID = (BTL_MAIN_GetCompetitor(wk->mainModule) == BTL_COMPETITOR_TRAINER)?
                      BTL_STRID_STD_Encount_NPC1 : BTL_STRID_STD_Encount_Player1;


      bbgp_make( wk, &bbgp, subwk->enemyClientID, BTLV_BALL_GAUGE_TYPE_ENEMY );
      BTLV_EFFECT_SetBallGauge( &bbgp );

      BTL_STR_MakeStringStd( wk->strBuf, strID, 1, subwk->enemyClientID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
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
      BTL_STR_MakeStringStd( wk->strBuf, strID, 2, subwk->clientID, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
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
    BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp), subwk->vpos );
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
      BTL_STR_MakeStringStd( wk->strBuf, strID, 3, subwk->clientID, subwk->pokeID[0], subwk->pokeID[1] );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
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
    BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[0]), subwk->vpos[0] );
    if( subwk->aliveCnt == 2 ){
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[1]), subwk->vpos[1] );
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
      BTL_STR_MakeStringStd( wk->strBuf, strID, 4, subwk->clientID, subwk->pokeID[0], subwk->pokeID[1], subwk->pokeID[2] );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
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
    BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[0]), subwk->vpos[0] );
    if( subwk->aliveCnt > 1){
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[1]), subwk->vpos[1] );
    }
    if( subwk->aliveCnt > 2){
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[1]), subwk->vpos[2] );
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
//      BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_ENEMY );
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

      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_PutSingle, 1, subwk->pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 3:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_MINE );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp), subwk->vpos );
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
        subwk->pokeID[i] = BPP_GetID( subwk->bpp[i] );
        if( !BPP_IsDead(subwk->bpp[i]) ){
          subwk->aliveCnt++;
        }
      }

      strID = ( subwk->aliveCnt == 2 )? BTL_STRID_STD_PutDouble : BTL_STRID_STD_PutSingle;
      BTL_STR_MakeStringStd( wk->strBuf, strID, 2, subwk->pokeID[0], subwk->pokeID[1] );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 3:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_MINE );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[0]), subwk->vpos[0] );
      if( subwk->aliveCnt > 1){
        BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[1]), subwk->vpos[1] );
      }
      BTLV_EFFECT_Add( BTLEFF_SINGLE_ENCOUNT_3 );
      statwin_disp_start( &wk->statusWin[ subwk->pos[0] ] );
      if( subwk->aliveCnt > 1){
        statwin_disp_start( &wk->statusWin[ subwk->pos[1] ] );
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
      BTL_STR_MakeStringStd( wk->strBuf, strID, 3, subwk->pokeID[0], subwk->pokeID[1], subwk->pokeID[2] );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
      (*seq)++;
    }
    break;

  case 3:
    if( BTLV_SCU_WaitMsg(wk) )
    {
      BTLV_EFFECT_DelBallGauge( BTLV_BALL_GAUGE_TYPE_MINE );
      msgWinVisible_Hide( &wk->msgwinVisibleWork );
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[0]), subwk->vpos[0] );
      if( subwk->aliveCnt > 1){
        BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[1]), subwk->vpos[1] );
      }
      if( subwk->aliveCnt > 2){
        BTLV_EFFECT_SetPokemon( BPP_GetSrcData(subwk->bpp[2]), subwk->vpos[2] );
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
 * とくせいウィンドウ表示オン
 *
 * @param   wk
 * @param   pos
 *
 */
//=============================================================================================
void BTLV_SCU_DispTokWin( BTLV_SCU* wk, BtlPokePos pos )
{
  tokwin_disp_first( &wk->tokWin[pos] );
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
void BTLV_SCU_HideTokWin( BTLV_SCU* wk, BtlPokePos pos )
{
  tokwin_hide( &wk->tokWin[pos] );
}

//----------------------------------------------

typedef struct {

  TOK_WIN*  tokwin;
  u16       seq;
  u16       timer;
  u16       count;

}TOKWIN_RENEW_WORK;

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
  GFL_TCBL* tcbl = GFL_TCBL_Create( wk->tcbl, tokwinRenewTask, sizeof(TOKWIN_RENEW_WORK), BTLV_TASKPRI_DAMAGE_EFFECT );
  TOKWIN_RENEW_WORK* twk = GFL_TCBL_GetWork( tcbl );

  twk->tokwin = &wk->tokWin[pos];
  twk->tokwin->renewingFlag = TRUE;
  twk->seq = 0;
  twk->timer = 0;
  twk->count = 0;
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
  return (wk->tokWin[pos].renewingFlag == FALSE);
}

// とくせいウィンドウの内容更新タスク
static void tokwinRenewTask( GFL_TCBL* tcbl, void* wk_adrs )
{
  TOKWIN_RENEW_WORK* wk = wk_adrs;

  if( ++(wk->timer) > 4 )
  {
    wk->timer = 0;
    wk->count++;
    if( wk->count & 1 ){
      tokwin_hide( wk->tokwin );
      if( wk->count == 11 ){
        tokwin_check_update_cgx( wk->tokwin );
      }
    }else{
      tokwin_disp( wk->tokwin );
      if( wk->count == 20 ){
        wk->tokwin->renewingFlag = FALSE;
        GFL_TCBL_Delete( tcbl );
      }
    }
  }
}

//----------------------------------------------

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
        wk->win, 0, 0, str, wk->defaultFont, MSGSPEED_GetWait(), wk->tcbl, BTLV_TASKPRI_MAIN_WINDOW,
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
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
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

#ifdef DEBUG_ONLY_FOR_taya
//  @@@ ロジックを追うのにエフェクトが出る時間が無駄なのでオフる
//  BTLV_EFFECT_AddWazaEffect( &param );
#else
  BTLV_EFFECT_AddWazaEffect( &param );
#endif
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

static void taskDamageEffect( GFL_TCBL* tcbl, void* wk_adrs )
{
  DMG_EFF_TASK_WORK* wk = wk_adrs;

  if( wk->timer )
  {
    u16 hp;
    u8 col;

    if( --(wk->timer) )
    {
      wk->hpVal -= wk->hpMinusVal;
      hp = wk->hpVal >> FX32_SHIFT;
      col = ((wk->timer % 16) <= 7)? TEST_STATWIN_BGCOL : TEST_STATWIN_BGCOL_FLASH;
      statwin_update( wk->statWin, hp, col );
    }
    else
    {
      statwin_update( wk->statWin, wk->hpEnd, TEST_STATWIN_BGCOL );
    }
  }
  else
  {
    if( !BTLV_EFFECT_CheckExecute() )
    {
      (*(wk->taskCounter))--;
      GFL_TCBL_Delete( tcbl );
    }
  }
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
  BTLV_EFFECT_DelPokemon( BTL_MAIN_BtlPosToViewPos(wk->mainModule, pos) );

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
  return TRUE;
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
      const POKEMON_PARAM* pp = BPP_GetSrcData( bpp );
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
    statwin_reset_data( wk->statWin );
    wk->seq++;
    break;
  case 1:
    statwin_disp_start( wk->statWin );
    wk->seq++;
    break;
  case 2:
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

static void taskHPGauge( GFL_TCBL* tcbl, void* wk_adrs )
{
  HP_GAUGE_EFFECT* wk = wk_adrs;
  if( wk->timer )
  {
    u16 hp;
    u8 col;

    if( --(wk->timer) )
    {
      wk->hpVal += wk->hpAddVal;
      hp = wk->hpVal >> FX32_SHIFT;
      statwin_update( wk->statWin, hp, TEST_STATWIN_BGCOL );
    }
    else
    {
      statwin_update( wk->statWin, wk->hpEnd, TEST_STATWIN_BGCOL );
    }
  }
  else
  {
    if( !BTLV_EFFECT_CheckExecute() )
    {
      (*(wk->taskCounter))--;
      GFL_TCBL_Delete( tcbl );
    }
  }
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
  statwin_cleanup( &wk->statusWin[ pos1 ] );
  statwin_cleanup( &wk->statusWin[ pos2 ] );

  statwin_setup( &wk->statusWin[ pos1 ], wk, pos1 );
  statwin_setup( &wk->statusWin[ pos2 ], wk, pos2 );

  statwin_reset_data( &wk->statusWin[ pos1 ] );
  statwin_reset_data( &wk->statusWin[ pos2 ] );

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
 * きのみ食べるアクション開始
 *
 * @param   wk
 * @param   pos
 */
//=============================================================================================
void BTLV_SCU_KinomiAct_Start( BTLV_SCU* wk, BtlvMcssPos pos )
{
  // @@@ 今は適当にワザエフェクトを出しておく
  BTLV_WAZAEFFECT_PARAM param;

  param.waza = WAZANO_HATAKU;
  param.from = pos;
  param.to = pos;
  param.turn_count = 0;
  param.continue_count = 0;
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

  for(i=0; i<NELEMS(wk->statusWin); i++){
    wk->statusWin[i].win = NULL;
  }

  pos_end = BTL_MAIN_GetEnablePosEnd( wk->mainModule );
  for(i=0; i<=pos_end; ++i){
    statwin_setup( &wk->statusWin[ i ], wk, i );
  }
}

static void statwin_cleanupAll( BTLV_SCU* wk )
{
  int i;
  for(i=0; i<NELEMS(wk->statusWin); i++)
  {
    if( wk->statusWin[i].win != NULL)
    {
      statwin_cleanup( &wk->statusWin[i] );
    }
  }
}

static void tokwin_setupAll( BTLV_SCU* wk )
{
  u32 pos_end;
  u32 i;

  for(i=0; i<NELEMS(wk->tokWin); i++){
    wk->tokWin[i].win = NULL;
  }

  pos_end = BTL_MAIN_GetEnablePosEnd( wk->mainModule );
  for(i=0; i<=pos_end; ++i){
    tokwin_setup( &wk->tokWin[ i ], wk, i );
  }
}

static void tokwin_cleanupAll( BTLV_SCU* wk )
{
  int i;
  for(i=0; i<NELEMS(wk->tokWin); i++)
  {
    if( wk->tokWin[i].win != NULL)
    {
      tokwin_cleanup( &wk->tokWin[i] );
    }
  }
}




static void statwin_setup( STATUS_WIN* stwin, BTLV_SCU* wk, BtlPokePos pokePos )
{
  enum {
    STATWIN_WIDTH = 7,
    STATWIN_HEIGHT = 4,
  };
  static const struct {
    u8 x;
    u8 y;
  } winpos[] = {
    { 18, 13 },
    {  4,  2 },

    { 17, 13 },
    {  8,  2 },
    { 25, 14 },
    {  0,  1 },
  };

  u8 viewPos, px, py;

  stwin->pokePos = pokePos;
  stwin->parentWk = wk;

  viewPos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pokePos );

  px = winpos[viewPos].x;
  py = winpos[viewPos].y;


  stwin->win = GFL_BMPWIN_Create( GFL_BG_FRAME3_M, px, py, STATWIN_WIDTH, STATWIN_HEIGHT,
      PALIDX_POKEWIN, GFL_BMP_CHRAREA_GET_F );
  stwin->bmp = GFL_BMPWIN_GetBmp( stwin->win );

  stwin->dispFlag = FALSE;
  stwin->vpos = viewPos;

  statwin_reset_data( stwin );
}

static void statwin_cleanup( STATUS_WIN* stwin )
{
  GFL_BMPWIN_Delete( stwin->win );
  if( stwin->dispFlag ){
    BTLV_EFFECT_DelGauge( stwin->vpos );
    stwin->dispFlag = FALSE;
  }
}

static void statwin_reset_data( STATUS_WIN* stwin )
{
  BTLV_SCU* wk = stwin->parentWk;

  stwin->bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, stwin->pokePos );
  stwin->hp = BPP_GetValue( stwin->bpp, BPP_HP );

  GFL_BMP_Clear( stwin->bmp, TEST_STATWIN_BGCOL );
  BTL_STR_MakeStatusWinStr( wk->strBuf, stwin->bpp, stwin->hp );
  PRINTSYS_Print( stwin->bmp, 0, 0, wk->strBuf, wk->defaultFont );
}


static void statwin_disp_start( STATUS_WIN* stwin )
{
  u8 viewPos = BTL_MAIN_BtlPosToViewPos( stwin->parentWk->mainModule, stwin->pokePos );
  BTLV_EFFECT_SetGauge( stwin->bpp, viewPos );
  stwin->dispFlag = TRUE;
}

static void statwin_disp( STATUS_WIN* stwin )
{
  GFL_BMPWIN_MakeScreen( stwin->win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(stwin->win) );
}

// １行ずつ消す。全部消えたらTRUE
static BOOL statwin_erase( STATUS_WIN* stwin, u8 line )
{
  // @todo 未対応？
  return TRUE;
}

static void statwin_update( STATUS_WIN* stwin, u16 hp, u8 col )
{
  BTLV_SCU* wk = stwin->parentWk;

  stwin->hp = hp;

  GFL_BMP_Clear( stwin->bmp, col );
  BTL_STR_MakeStatusWinStr( wk->strBuf, stwin->bpp, stwin->hp );
  PRINTSYS_Print( stwin->bmp, 0, 0, wk->strBuf, wk->defaultFont );
  GFL_BMPWIN_TransVramCharacter( stwin->win );
}

//----------------------------

static void tokwin_setup( TOK_WIN* tokwin, BTLV_SCU* wk, BtlPokePos pos )
{
  static const struct {
    u8 x;
    u8 y;
  } winpos[] = {
    {  4, 14 },   // VPOS_AA
    { 18,  3 },   // VPOS_BB
    {  2, 14 },   // VPOS_A
    { 20,  4 },   // VPOS_B
    {  6, 15 },   // VPOS_C
    { 15,  3 },   // VPOS_D
    {  4, 16 },   // VPOS_E
    { 18,  1 },   // VPOS_F
  };

  u8 isPlayer, playerClientID, px, py;
  u8 vpos;

  tokwin->parentWk = wk;
  tokwin->pokePos = pos;
  tokwin->renewingFlag = FALSE;

  playerClientID = BTLV_CORE_GetPlayerClientID( wk->vcore );
  vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos );
  px = winpos[vpos].x;
  py = winpos[vpos].y;

  tokwin->win = GFL_BMPWIN_Create( GFL_BG_FRAME2_M, px, py, TEST_TOKWIN_CHAR_WIDTH, 2,
      PALIDX_POKEWIN, GFL_BMP_CHRAREA_GET_F );
  tokwin->bmp = GFL_BMPWIN_GetBmp( tokwin->win );

  {
    const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, pos );
    tokwin->tokusei = BPP_GetValue( bpp, BPP_TOKUSEI );
    tokwin->pokeID = BPP_GetID( bpp );
  }

  tokwin_update_cgx( tokwin );
}
// CGX更新
static void tokwin_update_cgx( TOK_WIN* tokwin )
{
  BTLV_SCU* wk = tokwin->parentWk;
  GFL_MSGDATA* msg;
  u16 xpos;

  msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_tokusei_dat, GFL_HEAP_LOWID(wk->heapID) );

  GFL_BMP_Clear( tokwin->bmp, TEST_TOKWIN_BGCOL );
  GFL_MSG_GetString( msg, tokwin->tokusei, wk->strBuf );
  xpos = (TEST_TOKWIN_DOT_WIDTH - PRINTSYS_GetStrWidth(wk->strBuf, wk->defaultFont, 0)) / 2;
  PRINTSYS_Print( tokwin->bmp, xpos, 2, wk->strBuf, wk->defaultFont );
  GFL_MSG_Delete( msg );

  GFL_BMPWIN_TransVramCharacter( tokwin->win );
}
// 更新の必要があればCGX更新
static void tokwin_check_update_cgx( TOK_WIN* tokwin )
{
  BTLV_SCU* wk = tokwin->parentWk;
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, tokwin->pokePos );
  u8 pokeID = BPP_GetID( bpp );
  u16 tokusei = BPP_GetValue( bpp, BPP_TOKUSEI );
  if( (pokeID != tokwin->pokeID)
  ||  (tokusei != tokwin->tokusei)
  ){
    BTL_Printf("とくせいウィンドウ書き換わり pos=%d, tok=%d -> %d\n", tokwin->pokePos, tokwin->tokusei, tokusei );
    tokwin->pokeID = pokeID;
    tokwin->tokusei = tokusei;

    tokwin_update_cgx( tokwin );
  }
}

static void tokwin_cleanup( TOK_WIN* tokwin )
{
  GFL_BMPWIN_Delete( tokwin->win );
}
static void tokwin_disp_first( TOK_WIN* tokwin )
{
  tokwin_check_update_cgx( tokwin );
  tokwin_disp( tokwin );
}
static void tokwin_hide( TOK_WIN* tokwin )
{
  GFL_BMPWIN_ClearScreen( tokwin->win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(tokwin->win) );
}

static void tokwin_disp( TOK_WIN* tokwin )
{
  GFL_BMPWIN_MakeScreen( tokwin->win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(tokwin->win) );
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

