//============================================================================================
/**
 * @file  pdc_msg.c
 * @brief ポケモンドリームキャッチ（パレスでの捕獲ゲーム）
 * @author  soga
 * @date  2010.03.16
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>

#include "print/wordset.h"

#include "../../battle/btlv/btlv_effect.h"

#include "app/pdc.h"
#include "pdc_main.h"
#include "pdc_msg.h"

#include "arc_def.h"

#include "message.naix"
#include "battle/battgra_wb.naix"
#include "msg/msg_pdc.h"

#include "gamesystem/msgspeed.h"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================
enum
{ 
  MSGWIN_X =  1,
  MSGWIN_Y = 19,
  MSGWIN_W = 30,
  MSGWIN_H =  4,

  MSG_FRAME = GFL_BG_FRAME1_M,

  MSGWIN_PAL =  0,
  MSGWIN_CLEAR  = 0x0c,
  MSGWIN_LETTER = 0x01,
  MSGWIN_SHADOW = 0x09,

  MSGWIN_STRBUF_SIZE = 256,

  MSGWIN_WAIT = 80,
  MSGWIN_NO_WAIT = 0,

  MSGWIN_EVA_MAX = 31,
  MSGWIN_EVA_MIN = 0,
  MSGWIN_EVB_MIN = 7,
  MSGWIN_EVB_MAX = 16,
  MSGWIN_VISIBLE_STEP = 6,
};

typedef enum {

  MSGWIN_STATE_DISP = 0,
  MSGWIN_STATE_TO_HIDE,
  MSGWIN_STATE_HIDE,
  MSGWIN_STATE_TO_DISP,
  MSGWIN_STATE_TO_DISP_READY,

}MsgWinState;

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

typedef struct {
  GFL_BMPWIN*  win;
  fx32  eva;
  fx32  evb;
  fx32  eva_step;
  fx32  evb_step;
  u8    state;
  u8    timer;
}MSGWIN_VISIBLE;

struct _PDC_MSG_WORK
{ 
  PRINT_STREAM*   printStream;
  MSGWIN_VISIBLE  msgwinVisibleWork;
  GFL_BMPWIN*     win;
  GFL_MSGDATA*    msg;
  GFL_FONT*       font;
  GFL_TCBLSYS*    tcbl;
  STRBUF*         str;
  WORDSET*        word;
  int             msg_speed;
  u8              printAtOnceFlag;
  u8              printSeq;
  u8              printJustDoneFlag;
  PRINTSTREAM_STATE  printState;
  u16             printWait;
  u16             printWaitOrg;
  HEAPID          heapID;
};

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  void  set_message( PDC_MSG_WORK* pmw, int wait );
static  void  msgWinVisible_Init( MSGWIN_VISIBLE* wk, GFL_BMPWIN* win );
static  void  msgWinVisible_Hide( MSGWIN_VISIBLE* wk );
static  void  msgWinVisible_Disp( MSGWIN_VISIBLE* wk, BOOL printAtOnceFlag );
static  BOOL  msgWinVisible_Update( MSGWIN_VISIBLE* wk );

//--------------------------------------------------------------------------
/**
 * @brief メッセージ描画初期化
 *
 * @param[in] font      使用するフォントハンドル
 * @param[in] config    コンフィグ（メッセージスピードを決定するために使用）
 * @param[in] tcbl      tcblsys
 * @param[in] heapID    ヒープID
 *
 * @retval  PDC_MSG_WORK
 */
//--------------------------------------------------------------------------
PDC_MSG_WORK* PDC_MSG_Create( GFL_FONT* font, CONFIG* config, GFL_TCBLSYS* tcbl, HEAPID heapID )
{ 
  PDC_MSG_WORK* pmw = GFL_HEAP_AllocClearMemory( heapID, sizeof( PDC_MSG_WORK ) );
  static  const GFL_BG_BGCNT_HEADER msgText = { 
    0, 0, 0x800, 0,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x4800, GX_BG_CHARBASE_0x10000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };

  GFL_BG_SetBGControl( MSG_FRAME, &msgText, GFL_BG_MODE_TEXT );

  pmw->heapID = heapID;
  pmw->tcbl   = tcbl;

  pmw->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pdc_dat, pmw->heapID );
  pmw->font = font;
  pmw->msg_speed = MSGSPEED_GetWaitByConfigParam( CONFIG_GetMsgSpeed( config ) );

  pmw->str = GFL_STR_CreateBuffer( MSGWIN_STRBUF_SIZE, heapID );
  pmw->word = WORDSET_Create( heapID );

  GFL_BG_FillScreen( MSG_FRAME, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
  GFL_BG_FillCharacter( MSG_FRAME, 0x00, 1, 0 );

  { 
    GFL_ARCUTIL_TRANSINFO info = GFL_ARC_UTIL_TransVramBgCharacterAreaMan( ARCID_BATTGRA, NARC_battgra_wb_msgwin_frm_NCGR,
                                                                           MSG_FRAME, 0, FALSE, pmw->heapID );
    PaletteWorkSetEx_Arc( BTLV_EFFECT_GetPfd(), ARCID_BATTGRA, NARC_battgra_wb_msgwin_frm_NCLR, pmw->heapID,
                          FADE_MAIN_BG, 0x20, MSGWIN_PAL*16, 0 );
    pmw->win = GFL_BMPWIN_Create( MSG_FRAME, MSGWIN_X, MSGWIN_Y, MSGWIN_W, MSGWIN_H, MSGWIN_PAL, GFL_BMP_CHRAREA_GET_F );
    GFL_BMPWIN_MakeScreen( pmw->win );
    GFL_BMPWIN_MakeFrameScreen( pmw->win, GFL_ARCUTIL_TRANSINFO_GetPos( info ), MSGWIN_PAL );
  }

  GFL_BG_LoadScreenReq( MSG_FRAME );

  GFL_BG_SetVisible( MSG_FRAME, VISIBLE_OFF );

  msgWinVisible_Init( &pmw->msgwinVisibleWork, pmw->win );

  return pmw;
}

//--------------------------------------------------------------------------
/**
 * @brief メッセージ描画破棄
 
 * @param[in] pmw       システム管理構造体
 */
//--------------------------------------------------------------------------
void  PDC_MSG_Delete( PDC_MSG_WORK* pmw )
{ 
  GFL_STR_DeleteBuffer( pmw->str );
  WORDSET_Delete( pmw->word );

  if( pmw->printStream )
  { 
    PRINTSYS_PrintStreamDelete( pmw->printStream );
  }

  GFL_BG_FreeBGControl( MSG_FRAME );
  GFL_MSG_Delete( pmw->msg );
  GFL_BMPWIN_Delete( pmw->win );

  GFL_HEAP_FreeMemory( pmw );
}

//--------------------------------------------------------------------------
/**
 * @brief エンカウントメッセージ
 
 * @param[in] pmw       システム管理構造体
 */
//--------------------------------------------------------------------------
void  PDC_MSG_SetEncountMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp )
{ 
  STRBUF* str = GFL_MSG_CreateString( pmw->msg, PDC_ENCOUNT );
  WORDSET_RegisterPokeNickName( pmw->word, 0, PDC_GetPP( psp ) );
  WORDSET_ExpandStr( pmw->word, pmw->str, str );
  set_message( pmw, MSGWIN_WAIT );
  GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------
/**
 * @brief どうする？メッセージ
 
 * @param[in] pmw       システム管理構造体
 */
//--------------------------------------------------------------------------
void  PDC_MSG_SetDousuruMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp )
{ 
  STRBUF* str = GFL_MSG_CreateString( pmw->msg, PDC_DOUSURU );
  WORDSET_RegisterPlayerName( pmw->word, 0, PDC_GetMyStatus( psp ) );
  WORDSET_ExpandStr( pmw->word, pmw->str, str );
  set_message( pmw, MSGWIN_NO_WAIT );
  GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------
/**
 * @brief 投げるメッセージ
 
 * @param[in] pmw       システム管理構造体
 */
//--------------------------------------------------------------------------
void  PDC_MSG_SetThrowMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp )
{ 
  STRBUF* str = GFL_MSG_CreateString( pmw->msg, PDC_BALL_THROW );
  WORDSET_RegisterPokeNickName( pmw->word, 0, PDC_GetPP( psp ) );
  WORDSET_ExpandStr( pmw->word, pmw->str, str );
  set_message( pmw, MSGWIN_WAIT );
  GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------
/**
 * @brief 捕獲メッセージ
 
 * @param[in] pmw       システム管理構造体
 */
//--------------------------------------------------------------------------
void  PDC_MSG_SetCaptureMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp )
{ 
  STRBUF* str = GFL_MSG_CreateString( pmw->msg, PDC_CAPTURE );
  WORDSET_RegisterPokeNickName( pmw->word, 0, PDC_GetPP( psp ) );
  WORDSET_ExpandStr( pmw->word, pmw->str, str );
  set_message( pmw, MSGWIN_WAIT );
  GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------
/**
 * @brief にげるメッセージ
 
 * @param[in] pmw       システム管理構造体
 */
//--------------------------------------------------------------------------
void  PDC_MSG_SetEscapeMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp )
{ 
  STRBUF* str = GFL_MSG_CreateString( pmw->msg, PDC_ESCAPE );
  WORDSET_RegisterPlayerName( pmw->word, 0, PDC_GetMyStatus( psp ) );
  WORDSET_ExpandStr( pmw->word, pmw->str, str );
  set_message( pmw, MSGWIN_WAIT );
  GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------
/**
 * @brief メッセージ終了待ち
 
 * @param[in] pmw       システム管理構造体
 */
//--------------------------------------------------------------------------
BOOL  PDC_MSG_Wait( PDC_MSG_WORK* wk )
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
    msgWinVisible_Disp( &wk->msgwinVisibleWork, wk->printAtOnceFlag );
    wk->printSeq++;
    /* fallthru */
  case SEQ_WAIT_MSGWIN_VISIBLE:
    if( !msgWinVisible_Update(&wk->msgwinVisibleWork) ){
      break;
    }
    else
    {
      if( wk->printAtOnceFlag )
      {
        wk->printSeq = SEQ_DONE;
        break;
      }
      else
      {
        if( wk->printStream ){
          PRINTSYS_PrintStreamRun( wk->printStream );
        }
        wk->printSeq++;
      }
    }
    /* fallthru */
  case SEQ_WAIT_STREAM_RUNNING:
    if( wk->printStream )
    {
      wk->printState = PRINTSYS_PrintStreamGetState( wk->printStream );
      if( wk->printState != PRINTSTREAM_STATE_RUNNING )
      {
        wk->printSeq = SEQ_WAIT_USERCTRL_NOT_COMM;
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

//--------------------------------------------------------------------------
/**
 * @brief メッセージウィンドウ消し
 
 * @param[in] pmw       システム管理構造体
 */
//--------------------------------------------------------------------------
void  PDC_MSG_HideWindow( PDC_MSG_WORK* wk )
{ 
  msgWinVisible_Hide( &wk->msgwinVisibleWork );
}

//--------------------------------------------------------------------------
/**
 * @brief メッセージウィンドウ消し待ち
 
 * @param[in] pmw       システム管理構造体
 *
 * @retval  TRUE:終了 FALSE:消し中
 */
//--------------------------------------------------------------------------
BOOL  PDC_MSG_WaitHideWindow( PDC_MSG_WORK* wk )
{ 
  return msgWinVisible_Update( &wk->msgwinVisibleWork );
}

//--------------------------------------------------------------------------
/**
 * @brief * メッセージ終端までの表示を終えたタイミングかどうか判定
 
 * @param[in] pmw       システム管理構造体
 *
 * @retval  TRUE:終了 FALSE:表示中
 */
//--------------------------------------------------------------------------
BOOL  PDC_MSG_IsJustDone( PDC_MSG_WORK* wk )
{ 
  if( wk->printJustDoneFlag ){
    wk->printJustDoneFlag = FALSE;
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------
/**
 * @brief エンカウントメッセージ
 
 * @param[in] pmw       システム管理構造体
 */
//--------------------------------------------------------------------------
static  void  set_message( PDC_MSG_WORK* pmw, int wait )
{ 
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( pmw->win ), MSGWIN_CLEAR );
  GFL_FONTSYS_SetColor( MSGWIN_LETTER, MSGWIN_SHADOW, MSGWIN_CLEAR );

  GFL_BG_SetVisible( MSG_FRAME, VISIBLE_ON );

  if( wait )
  { 
    pmw->printStream = PRINTSYS_PrintStream( pmw->win, 0, 0, pmw->str,
                                             pmw->font, pmw->msg_speed, pmw->tcbl, 0, pmw->heapID, MSGWIN_CLEAR );
    PRINTSYS_PrintStreamStop( pmw->printStream );
    pmw->printAtOnceFlag = FALSE;
  }
  else
  { 
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( pmw->win ), 0, 0, pmw->str, pmw->font );
    GFL_BMPWIN_MakeScreen( pmw->win );
    GFL_BG_LoadScreenReq( MSG_FRAME );
    GFL_BMPWIN_TransVramCharacter( pmw->win );
    pmw->printAtOnceFlag = TRUE;
  }
  pmw->printSeq = 0;
  pmw->printWait = wait;
  pmw->printWaitOrg = wait;
  pmw->printJustDoneFlag = FALSE;
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
static void msgWinVisible_Disp( MSGWIN_VISIBLE* wk, BOOL printAtOnceFlag )
{
  if( wk->state == MSGWIN_STATE_HIDE
  ||  wk->state == MSGWIN_STATE_TO_HIDE
  ){
    if( !printAtOnceFlag )
    {
      GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( wk->win );
      GFL_BMP_Clear( bmp, MSGWIN_CLEAR );
      GFL_BMPWIN_TransVramCharacter( wk->win );
    }

    wk->eva = FX32_CONST( MSGWIN_EVA_MIN );
    wk->evb = FX32_CONST( MSGWIN_EVB_MAX );
    wk->eva_step = FX32_CONST( MSGWIN_EVA_MAX - MSGWIN_EVA_MIN ) / MSGWIN_VISIBLE_STEP;
    wk->evb_step = FX32_CONST( MSGWIN_EVB_MIN - MSGWIN_EVB_MAX ) / MSGWIN_VISIBLE_STEP;
    wk->timer = MSGWIN_VISIBLE_STEP;
    wk->state = MSGWIN_STATE_TO_DISP_READY;
  }
}
static BOOL msgWinVisible_Update( MSGWIN_VISIBLE* wk )
{
  switch( wk->state ){
  case MSGWIN_STATE_HIDE:
    return TRUE;
  case MSGWIN_STATE_DISP:
    return TRUE;
  case MSGWIN_STATE_TO_DISP_READY:
    wk->state = MSGWIN_STATE_TO_DISP;
    return FALSE;
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
