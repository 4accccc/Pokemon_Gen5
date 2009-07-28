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

#include "btlv_scu.h"

//#define BMP_GAUGE   //有効にすることで、BMPWINで表示しているHPゲージに差し代わります

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


typedef struct {
  GFL_BMPWIN*       win;
  GFL_BMP_DATA*     bmp;
  const BTL_POKEPARAM*  bpp;
  BTLV_SCU*       parentWk;
  u16           hp;
  u8            pokePos;
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
  GFL_FONT*       smallFont;
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
  PRINTSTREAM_STATE  printState;
  u16           printWait;
  u16           printWaitOrg;
  u8            msgwinVisibleFlag;
  u16           msgwinVisibleSeq;
  MSGWIN_VISIBLE   msgwinVisibleWork;
};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static inline void* Scu_GetProcWork( BTLV_SCU* wk, u32 size );
static BOOL btlin_wild_single( int* seq, void* wk_adrs );
static BOOL btlin_wild_double( int* seq, void* wk_adrs );
static BOOL btlin_comm_double_multi( int* seq, void* wk_adrs );
static void tokwinRenewTask( GFL_TCBL* tcbl, void* wk_adrs );
static void taskDamageEffect( GFL_TCBL* tcbl, void* wk_adrs );
static void taskDeadEffect( GFL_TCBL* tcbl, void* wk_adrs );
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
static void statwin_hide( STATUS_WIN* stwin );
static BOOL statwin_erase( STATUS_WIN* stwin, u8 line );
static void statwin_update( STATUS_WIN* stwin, u16 hp, u8 col );
static void tokwin_setup( TOK_WIN* tokwin, BTLV_SCU* wk, BtlPokePos pos );
static void tokwin_update_cgx( TOK_WIN* tokwin );
static void tokwin_check_update_cgx( TOK_WIN* tokwin );
static void tokwin_cleanup( TOK_WIN* tokwin );
static void tokwin_disp_first( TOK_WIN* tokwin );
static void tokwin_hide( TOK_WIN* tokwin );
static void tokwin_disp( TOK_WIN* tokwin );



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
//  wk->smallFont = GFL_FONT_Create( ARCID_FONT, NARC_font_small_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

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
    GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };
  // 個別フレーム設定
  static const GFL_BG_BGCNT_HEADER bgcntStat = {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };
  // 個別フレーム設定
  static const GFL_BG_BGCNT_HEADER bgcntTok = {
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x08000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
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
//  GFL_FONT_Delete( wk->smallFont );

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
  if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_SINGLE )
  {
    BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_wild_single );
  }
  else
  {
    if( !BTL_MAIN_IsMultiMode(wk->mainModule) ){
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_wild_double );
    }else{
      BTL_UTIL_SetupProc( &wk->proc, wk, NULL, btlin_comm_double_multi );
    }
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
      (*seq)++;
    }
    break;
  case 2:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_Encount, 1, subwk->pokeID );
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

      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_Encount_Double, 2, subwk[0].pokeID, subwk[1].pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
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
 * 戦闘画面セットアップ完了までの演出（通信／ダブルマルチ）
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

      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_Encount_Double, 2, subwk[0].pokeID, subwk[1].pokeID );
      BTLV_SCU_StartMsg( wk, wk->strBuf, BTLV_MSGWAIT_STD );
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
      }
    }
    else
    {
      return TRUE;
    }
    break;

  case SEQ_WAIT_USERCTRL_NOT_COMM: // 待ち指定あり（通常時）
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

//  @@@ ターゲットの居ないワザを出すと止まることがあるのでコメントアウトしている  taya
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
void BTLV_SCU_StartWazaDamageAct( BTLV_SCU* wk, BtlPokePos defPos )
{
#ifdef BMP_GAUGE
  enum {
    DAMAGE_FRAME_MIN = 40,
  };

  GFL_TCBL* tcbl = GFL_TCBL_Create( wk->tcbl, taskDamageEffect, sizeof(DMG_EFF_TASK_WORK), BTLV_TASKPRI_DAMAGE_EFFECT );
  DMG_EFF_TASK_WORK* twk = GFL_TCBL_GetWork( tcbl );

  twk->taskCounter = &(wk->taskCounter[TASKTYPE_WAZA_DAMAGE]);
  twk->statWin = &wk->statusWin[defPos];
  twk->hpEnd = BPP_GetValue( twk->statWin->bpp, BPP_HP );
  twk->hpVal = FX32_CONST( twk->statWin->hp );
  {
    u16 damage = twk->statWin->hp - twk->hpEnd;
    twk->timer = (damage * 180) / 100;
    if( twk->timer < DAMAGE_FRAME_MIN )
    {
      twk->timer = DAMAGE_FRAME_MIN;
    }

    twk->hpMinusVal = FX32_CONST(twk->statWin->hp - twk->hpEnd) / twk->timer;
  }
  BTLV_EFFECT_Damage( BTL_MAIN_BtlPosToViewPos(wk->mainModule, defPos) );

  (*(twk->taskCounter))++;
#else
  {
    const BTL_POKEPARAM*  bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, defPos );
    int                   value = BPP_GetValue( bpp, BPP_HP );

    BTLV_EFFECT_CalcGauge( BTL_MAIN_BtlPosToViewPos( wk->mainModule, defPos ), value );
  }
  BTLV_EFFECT_Damage( BTL_MAIN_BtlPosToViewPos(wk->mainModule, defPos) );
#endif
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
#ifdef BMP_GAUGE
  return wk->taskCounter[TASKTYPE_WAZA_DAMAGE] == 0;
#else
  return !( BTLV_EFFECT_CheckExecute() | BTLV_EFFECT_CheckExecuteGauge() );
#endif
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

//------------------------------------
typedef struct {

  STATUS_WIN*  statWin;
  u16     timer;
  u16     line;
  u8*     endFlag;

}DEAD_EFF_WORK;



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
  GFL_TCBL* tcbl = GFL_TCBL_Create( wk->tcbl, taskDeadEffect, sizeof(DEAD_EFF_WORK), BTLV_TASKPRI_DAMAGE_EFFECT );
  DEAD_EFF_WORK* twk = GFL_TCBL_GetWork( tcbl );

  twk->statWin = &wk->statusWin[pos];
  twk->endFlag = &wk->taskCounter[TASKTYPE_DEFAULT];
  twk->timer = 0;
  twk->line = 0;

  *(twk->endFlag) = FALSE;

  //soga
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
  return (wk->taskCounter[TASKTYPE_DEFAULT]);
}

static void taskDeadEffect( GFL_TCBL* tcbl, void* wk_adrs )
{
  DEAD_EFF_WORK* wk = wk_adrs;

  if( ++(wk->timer) > 4 )
  {
    wk->timer = 0;
    wk->line++;
    if( statwin_erase(wk->statWin, wk->line) )
    {
      *(wk->endFlag) = TRUE;
      GFL_TCBL_Delete( tcbl );
    }
  }
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
    statwin_hide( wk->statWin );
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
      BTLV_EFFECT_SetPokemon( BPP_GetSrcData( bpp ), vpos );
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
#ifdef BMP_GAUGE
  enum {
    DAMAGE_FRAME_MIN = 40,
  };

  GFL_TCBL* tcbl = GFL_TCBL_Create( wk->tcbl, taskHPGauge, sizeof(HP_GAUGE_EFFECT), BTLV_TASKPRI_DAMAGE_EFFECT );
  HP_GAUGE_EFFECT* twk = GFL_TCBL_GetWork( tcbl );

  twk->taskCounter = &(wk->taskCounter[TASKTYPE_HP_GAUGE]);
  twk->statWin = &wk->statusWin[pos];
  twk->hpEnd = BPP_GetValue( twk->statWin->bpp, BPP_HP );
  twk->hpVal = FX32_CONST( twk->statWin->hp );
  {
    u16 max, min, range;

    max = twk->statWin->hp;
    min = twk->hpEnd;
    if( max < min )
    {
      u16 tmp = max;
      max = min;
      min = max;
    }
    range = max - min;
    twk->timer = (range * 150) / 100;
    if( twk->timer < DAMAGE_FRAME_MIN )
    {
      twk->timer = DAMAGE_FRAME_MIN;
    }

    twk->hpAddVal = FX32_CONST((int)(twk->hpEnd - twk->statWin->hp)) / twk->timer;
    BTL_Printf("HP start=%d, end=%d, startVal=%08x, addVal=%08x, timer=%d\n", max, min, twk->hpVal, twk->hpAddVal, twk->timer);
  }

  (*(twk->taskCounter))++;
#else
  {
    const BTL_POKEPARAM*  bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, pos );
    int                   value = BPP_GetValue( bpp, BPP_HP );

    BTLV_EFFECT_CalcGauge( BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos ), value );
  }
#endif
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
#ifdef BMP_GAUGE
  return wk->taskCounter[TASKTYPE_HP_GAUGE] == 0;
#else
  return !(BTLV_EFFECT_CheckExecuteGauge());
#endif
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
  int i;
//  u8 plClientID, enClientID, i;

  for(i=0; i<NELEMS(wk->statusWin); i++)
  {
    wk->statusWin[i].win = NULL;
  }

  statwin_setup( &wk->statusWin[ BTL_POS_1ST_0 ], wk, BTL_POS_1ST_0 );
  statwin_setup( &wk->statusWin[ BTL_POS_2ND_0 ], wk, BTL_POS_2ND_0 );

  if( BTL_MAIN_GetRule(wk->mainModule) != BTL_RULE_SINGLE )
  {
    statwin_setup( &wk->statusWin[ BTL_POS_1ST_1 ], wk, BTL_POS_1ST_1 );
    statwin_setup( &wk->statusWin[ BTL_POS_2ND_1 ], wk, BTL_POS_2ND_1 );
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
  u8 i;

  for(i=0; i<NELEMS(wk->tokWin); i++)
  {
    wk->tokWin[i].win = NULL;
  }

  tokwin_setup( &wk->tokWin[ BTL_POS_1ST_0 ], wk, BTL_POS_1ST_0 );
  tokwin_setup( &wk->tokWin[ BTL_POS_2ND_0 ], wk, BTL_POS_2ND_0 );

  if( BTL_MAIN_GetRule(wk->mainModule) != BTL_RULE_SINGLE )
  {
    tokwin_setup( &wk->tokWin[ BTL_POS_1ST_1 ], wk, BTL_POS_1ST_1 );
    tokwin_setup( &wk->tokWin[ BTL_POS_2ND_1 ], wk, BTL_POS_2ND_1 );
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

  BTL_PrintfSimple("[STATWIN Setup] pokePos=%d, viewPos=%d (%d,%d)\n", pokePos, viewPos, px, py);

  stwin->win = GFL_BMPWIN_Create( GFL_BG_FRAME3_M, px, py, STATWIN_WIDTH, STATWIN_HEIGHT,
      PALIDX_POKEWIN, GFL_BMP_CHRAREA_GET_F );
  stwin->bmp = GFL_BMPWIN_GetBmp( stwin->win );

  statwin_reset_data( stwin );
}

static void statwin_cleanup( STATUS_WIN* stwin )
{
  GFL_BMPWIN_Delete( stwin->win );
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
#ifdef BMP_GAUGE
  GFL_BMPWIN_TransVramCharacter( stwin->win );
  GFL_BMPWIN_MakeScreen( stwin->win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(stwin->win) );
  BTL_Printf("StatusWin (pos=%d) disp start!!\n", stwin->pokePos);
#else
  u8 viewPos = BTL_MAIN_BtlPosToViewPos( stwin->parentWk->mainModule, stwin->pokePos );
  BTLV_EFFECT_SetGauge( BPP_GetSrcData( stwin->bpp ), viewPos );
#endif
}

static void statwin_disp( STATUS_WIN* stwin )
{
  GFL_BMPWIN_MakeScreen( stwin->win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(stwin->win) );
}

static void statwin_hide( STATUS_WIN* stwin )
{
#ifdef BMP_GAUGE
  GFL_BMPWIN_ClearScreen( stwin->win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(stwin->win) );
#else
  u8 viewPos = BTL_MAIN_BtlPosToViewPos( stwin->parentWk->mainModule, stwin->pokePos );
  BTLV_EFFECT_DelGauge( viewPos );
#endif
}

// １行ずつ消す。全部消えたらTRUE
static BOOL statwin_erase( STATUS_WIN* stwin, u8 line )
{
#ifdef BMP_GAUGE
  u8 px, py, width, height, frame;

  px     = GFL_BMPWIN_GetPosX( stwin->win );
  py     = GFL_BMPWIN_GetPosY( stwin->win );
  width  = GFL_BMPWIN_GetSizeX( stwin->win );
  height = GFL_BMPWIN_GetSizeY( stwin->win );
  frame  = GFL_BMPWIN_GetFrame( stwin->win );

  if( line > height )
  {
    line = height;
  }

  GFL_BG_FillScreen( frame, 0, px, py, width, line, GFL_BG_SCRWRT_PALIN );
  GFL_BG_LoadScreenReq( frame );

  return line == height;
#else
  BTLV_EFFECT_DelGauge( stwin->pokePos );

  return TRUE;
#endif
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

