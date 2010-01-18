//======================================================================
/**
 * @file	ctvt_talk.c
 * @brief	通信TVTシステム：会話時下画面
 * @author	ariizumi
 * @data	09/12/18
 *
 * モジュール名：CTVT_TALK
 */
//======================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/bmp_winframe.h"
#include "system/camera_system.h"
#include "net/network_define.h"
#include "app/app_menu_common.h"

#include "msg/msg_comm_tvt.h"

#include "comm_tvt.naix"

#include "ctvt_talk.h"
#include "ctvt_comm.h"
#include "ctvt_camera.h"
#include "ctvt_mic.h"
#include "enc_adpcm.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define CTVT_TALK_SLIDER_X (8)
#define CTVT_TALK_SLIDER_Y (116)
//#define CTVT_TALK_SLIDER_MOVE_Y (40)  //上にも下にも40  //外に出した
#define CTVT_TALK_SLIDER_WIDTH_H  (12)    //HALF
#define CTVT_TALK_SLIDER_HEIGHT_H (4) //HALF

#define CTVT_TALK_BAR_ICON_Y (192-12)
#define CTVT_TALK_YOBIDASHI_X (152)
#define CTVT_TALK_PAUSE_X (192)
#define CTVT_TALK_RETURN_X (224)

#define CTVT_TALK_REC_BUTTON_LEFT (4)
#define CTVT_TALK_REC_BUTTON_TOP (1)
#define CTVT_TALK_REC_BUTTON_WIDTH (23)
#define CTVT_TALK_REC_BUTTON_HEIGHT (13)

#define CTVT_TALK_DRAW_BUTTON_LEFT   (4)
#define CTVT_TALK_DRAW_BUTTON_TOP    (15)
#define CTVT_TALK_DRAW_BUTTON_WIDTH  (23)
#define CTVT_TALK_DRAW_BUTTON_HEIGHT (6)

#define CTVT_TALK_WAVE_DRAW_WIDTH   (17)
#define CTVT_TALK_WAVE_DRAW_HEIGHT  (7)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//ステート
typedef enum
{
  CTS_FADEIN,
  CTS_FADEIN_WAIT,

  CTS_FADEOUT_DOWN, //下だけフェード
  CTS_FADEOUT_BOTH, //両方フェード
  CTS_FADEOUT_WAIT,

  CTS_WAIT,

  CTS_REQ_TALK,
  CTS_REQ_TALK_WAIT,
  CTS_TALKING,
  
  CTS_END_CONFIRM_INIT,
  CTS_END_CONFIRM,

  //親機による子機の終了待ち
  CTS_WAIT_FINISH_CHILD_INIT,
  CTS_WAIT_FINISH_CHILD,

  //親のリクエストによる終了
  CTS_END_PARENT_REQ_INIT,
  CTS_END_PARENT_REQ,

}CTVT_TALK_STATE;

typedef enum
{
  CTSS_INIT_REC,
  CTSS_RECORDING,
  CTSS_REC_TRANS, //転送
  CTSS_WAIT_PLAY, 
  
  CTSS_GO_DRAW,
  CTSS_GO_END,
}CTVT_TALK_SUB_STATE;

//Recボタン種類
typedef enum
{
  CRBT_NONE,
  CRBT_TALKING,
  CRBT_DISALE,
  
  CRBT_MAX,
}CTVT_REC_BUTTON_TYPE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//ワーク
struct _CTVT_TALK_WORK
{
  CTVT_TALK_STATE state;
  CTVT_TALK_SUB_STATE subState;
  
  //スライダー系
  s16         sliderPos;
  BOOL        isHoldSlider;
  
  //録音関係  
  CTVT_MIC_WORK *micWork;
  BOOL  reqSendWave;
  u8    sendCnt;
  
  void  *sendWaveBuf;  //下の2つはここからアドレスを持つ
  CTVT_COMM_WAVE_HEADER *sendWaveData;
  void *sendWaveBufTop;
  
  BOOL isUpdateMsgRec;
  BOOL isUpdateMsgDraw;
  BOOL isUpdateMsgWin;
  GFL_BMPWIN *recWin;
  GFL_BMPWIN *drawWin;
  GFL_BMPWIN *waveWin;  //波形を描く
  u8  wavePosX;
  u8  wavePosY;

  GFL_BMPWIN *msgWin;
  APP_TASKMENU_WORK *yesNoWork;
  
  //セル関係
  GFL_CLWK    *clwkSlider;
  GFL_CLWK    *clwkYobidasi;
  GFL_CLWK    *clwkPause;
  GFL_CLWK    *clwkReturn;

  CTVT_REC_BUTTON_TYPE recButtonState;
  CTVT_REC_BUTTON_TYPE befRecButtonState;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void CTVT_TALK_UpdateWait( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork );
static void CTVT_TALK_UpdateTalk( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork );
static void CTVT_TALK_UpdateButton( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork );
static void CTVT_TALK_UpdateVoiceBar( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork );
static void CTVT_TALK_DrawLine( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork , u8 *charBuf , u8 x1 , u8 y1 , u8 x2 , u8 y2 );
static void CTVT_TALK_DrawDot( u8 *charBuf , u8 x , u8 y );

static void CTVT_TALK_DispMessage( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork , const u16 msgId );

static void CTVT_TALK_InitEndConfirm( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork );
static void CTVT_TALK_UpdateEndConfirm( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork );

static const GFL_UI_TP_HITTBL CTVT_TALK_HitRecButton[2] = 
{
  {
    CTVT_TALK_REC_BUTTON_TOP*8,
    (CTVT_TALK_REC_BUTTON_TOP+CTVT_TALK_REC_BUTTON_HEIGHT)*8,
    CTVT_TALK_REC_BUTTON_LEFT*8,
    (CTVT_TALK_REC_BUTTON_LEFT+CTVT_TALK_REC_BUTTON_WIDTH)*8,
  },
  {GFL_UI_TP_HIT_END,0,0,0}
};

static const GFL_UI_TP_HITTBL CTVT_TALK_HitDrawButton[2] = 
{
  {
    CTVT_TALK_DRAW_BUTTON_TOP*8,
    (CTVT_TALK_DRAW_BUTTON_TOP+CTVT_TALK_DRAW_BUTTON_HEIGHT)*8,
    CTVT_TALK_DRAW_BUTTON_LEFT*8,
    (CTVT_TALK_DRAW_BUTTON_LEFT+CTVT_TALK_DRAW_BUTTON_WIDTH)*8,
  },
  {GFL_UI_TP_HIT_END,0,0,0}
};
//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
CTVT_TALK_WORK* CTVT_TALK_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId )
{
  u8 i;
  CTVT_TALK_WORK* talkWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_TALK_WORK) );
  
  talkWork->micWork = CTVT_MIC_Init( heapId );
  talkWork->sendWaveBuf = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_COMM_WAVE_HEADER)+CTVT_SEND_WAVE_SIZE_ONE );
  talkWork->sendWaveData = talkWork->sendWaveBuf;
  talkWork->sendWaveBufTop = (void*)((u32)talkWork->sendWaveBuf+sizeof(CTVT_COMM_WAVE_HEADER));
  talkWork->sliderPos = 0;
  return talkWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void CTVT_TALK_TermSystem( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  
  GFL_HEAP_FreeMemory( talkWork->sendWaveData );
  CTVT_MIC_Term( talkWork->micWork );
  GFL_HEAP_FreeMemory( talkWork );
}
//--------------------------------------------------------------
//	モード切替時初期化
//--------------------------------------------------------------
void CTVT_TALK_InitMode( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
  ARCHANDLE* arcHandle = COMM_TVT_GetArcHandle( work );

  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NSCR , 
                    CTVT_FRAME_SUB_MISC ,  0 , 0, FALSE , heapId );
  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_MISC );
  
  {
    //スライダー
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = CTVT_TALK_SLIDER_X;
    cellInitData.pos_y = CTVT_TALK_SLIDER_Y + talkWork->sliderPos;
    cellInitData.anmseq = CTOAS_SLIDER;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    
    talkWork->clwkSlider = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    GFL_CLACT_WK_SetDrawEnable( talkWork->clwkSlider , TRUE );

    //呼び出しボタン
    cellInitData.pos_x = CTVT_TALK_YOBIDASHI_X;
    cellInitData.pos_y = CTVT_TALK_BAR_ICON_Y;
    cellInitData.anmseq = CTOAS_YOBIDASHI;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    
    talkWork->clwkYobidasi = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    GFL_CLACT_WK_SetDrawEnable( talkWork->clwkYobidasi , FALSE );

    //一時停止ボタン
    cellInitData.pos_x = CTVT_TALK_PAUSE_X;
    cellInitData.pos_y = CTVT_TALK_BAR_ICON_Y;
    //TODO 一時停止対応
    cellInitData.anmseq = CTOAS_PAUSE;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    
    talkWork->clwkPause = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    GFL_CLACT_WK_SetDrawEnable( talkWork->clwkPause , TRUE );

    //戻るボタン
    cellInitData.pos_x = CTVT_TALK_RETURN_X;
    cellInitData.pos_y = 192-24;
    cellInitData.anmseq = APP_COMMON_BARICON_RETURN;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    
    talkWork->clwkReturn = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    GFL_CLACT_WK_SetDrawEnable( talkWork->clwkReturn , TRUE );
  }

  talkWork->state = CTS_FADEIN;
  talkWork->recButtonState = CRBT_NONE;
  talkWork->befRecButtonState = CRBT_MAX;
  talkWork->isHoldSlider = FALSE;

  talkWork->recWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MSG , 
                                        7 , 10 , 17 , 2 ,
                                        CTVT_PAL_BG_SUB_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
  talkWork->drawWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MSG , 
                                        7 , 17 , 17 , 2 ,
                                        CTVT_PAL_BG_SUB_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
  talkWork->waveWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MSG , 
                                        7 , 2 , 
                                        CTVT_TALK_WAVE_DRAW_WIDTH , 
                                        CTVT_TALK_WAVE_DRAW_HEIGHT ,
                                        CTVT_PAL_BG_SUB_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
  //MakeScreenは文字表示の時
  talkWork->msgWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MSG , 
                                        1 , 1 , 30 , 4 ,
                                        CTVT_PAL_BG_SUB_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( talkWork->recWin ) , 0x0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( talkWork->drawWin) , 0x0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( talkWork->waveWin) , 0xF );
  GFL_BMPWIN_MakeScreen( talkWork->recWin );
  GFL_BMPWIN_MakeScreen( talkWork->drawWin );
  GFL_BMPWIN_MakeScreen( talkWork->waveWin );
  GFL_BMPWIN_TransVramCharacter( talkWork->waveWin );

  {
    GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
    GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    STRBUF *str;
    u16 len;
    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_TALK_01 );
    len = PRINTSYS_GetStrWidth( str , fontHandle , 0 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( talkWork->recWin ) , 
            (17*8-len)/2 , 0 , str , fontHandle ,CTVT_FONT_COLOR_WHITE );
    GFL_STR_DeleteBuffer( str );
    
    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_TALK_02 );
    len = PRINTSYS_GetStrWidth( str , fontHandle , 0 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( talkWork->drawWin ) , 
            (17*8-len)/2 , 0 , str , fontHandle ,CTVT_FONT_COLOR_WHITE );
    GFL_STR_DeleteBuffer( str );
    
  }
  talkWork->yesNoWork = NULL;
  talkWork->isUpdateMsgRec = TRUE;
  talkWork->isUpdateMsgDraw = TRUE;
  talkWork->isUpdateMsgWin = FALSE;
  GFL_BG_LoadScreenReq(CTVT_FRAME_SUB_MSG);

}

//--------------------------------------------------------------
//	モード切替時開放
//--------------------------------------------------------------
void CTVT_TALK_TermMode( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{

  if( talkWork->yesNoWork != NULL )
  {
    APP_TASKMENU_CloseMenu( talkWork->yesNoWork );
  }

  GFL_BMPWIN_ClearTransWindow( talkWork->recWin );
  GFL_BMPWIN_ClearTransWindow( talkWork->drawWin );
  GFL_BMPWIN_ClearTransWindow( talkWork->waveWin );
  GFL_BMPWIN_ClearTransWindow( talkWork->msgWin );
  GFL_BMPWIN_Delete( talkWork->recWin );
  GFL_BMPWIN_Delete( talkWork->drawWin );
  GFL_BMPWIN_Delete( talkWork->waveWin );
  GFL_BMPWIN_Delete( talkWork->msgWin );
  
  GFL_CLACT_WK_Remove( talkWork->clwkReturn );
  GFL_CLACT_WK_Remove( talkWork->clwkPause );
  GFL_CLACT_WK_Remove( talkWork->clwkYobidasi );
  GFL_CLACT_WK_Remove( talkWork->clwkSlider );
  GFL_BG_ClearScreen( CTVT_FRAME_SUB_MISC );
  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_MISC );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
const COMM_TVT_MODE CTVT_TALK_Main( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
  switch( talkWork->state )
  {
  case CTS_FADEIN:
    if( COMM_TVT_GetUpperFade( work ) == TRUE )
    {
      WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
              WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    }
    else
    {
      WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
              WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    }
    talkWork->state = CTS_FADEIN_WAIT;
    break;
  case CTS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      talkWork->state = CTS_WAIT;
    }
    break;

  case CTS_FADEOUT_DOWN:
    WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    COMM_TVT_SetUpperFade( work , FALSE );
    talkWork->state = CTS_FADEOUT_WAIT;
    break;
  case CTS_FADEOUT_BOTH:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    COMM_TVT_SetUpperFade( work , TRUE );
    talkWork->state = CTS_FADEOUT_WAIT;
    break;

  case CTS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      if( talkWork->subState == CTSS_GO_DRAW )
      {
        return CTM_DRAW;
      }
      else
      {
        return CTM_END;
      }
    }
    break;
    
  case CTS_WAIT:
    CTVT_TALK_UpdateWait( work , talkWork );
    break;
    
  case CTS_REQ_TALK:
    if( COMM_TVT_GetFinishReq( work ) == TRUE )
    {
      //親のリクエストによる終了
      talkWork->state = CTS_END_PARENT_REQ_INIT;
    }
    else
    {
      const int isContTbl = GFL_UI_TP_HitCont( CTVT_TALK_HitRecButton );
      if( isContTbl == 0 || (GFL_UI_KEY_GetCont() & CTVT_BUTTON_TALK) )
      {
        CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
        const BOOL ret = CTVT_COMM_SendFlg( work , commWork , CCFT_REQ_TALK , 0 );
        if( ret == TRUE )
        {
          talkWork->state = CTS_REQ_TALK_WAIT;
        }
      }
      else
      {
        talkWork->state = CTS_WAIT;
      }
    }
    break;
  
  case CTS_REQ_TALK_WAIT:
    if( COMM_TVT_GetFinishReq( work ) == TRUE )
    {
      //親のリクエストによる終了
      talkWork->state = CTS_END_PARENT_REQ_INIT;
    }
    else
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      const u8 talkMember = CTVT_COMM_GetTalkMember( work , commWork );
      const u8 selfId = CTVT_COMM_GetSelfNetId( work , commWork );
      const int isContTbl = GFL_UI_TP_HitCont( CTVT_TALK_HitRecButton );
      if( isContTbl == 0 || (GFL_UI_KEY_GetCont() & CTVT_BUTTON_TALK)  )
      {
        if( talkMember != CTVT_COMM_INVALID_MEMBER )
        {
          if( talkMember == selfId )
          {
            //会話開始
            talkWork->state = CTS_TALKING;
            talkWork->subState = CTSS_INIT_REC;
          }
          else
          {
            //抽選もれ
            talkWork->state = CTS_WAIT;
          }
        }
      }
      else
      {
        //会話キャンセル
        talkWork->state = CTS_WAIT;
      }
    }
    break;
  case CTS_TALKING:
    CTVT_TALK_UpdateTalk( work , talkWork );
    break;
    
  case CTS_END_CONFIRM_INIT:
    CTVT_TALK_InitEndConfirm( work , talkWork );
    break;

  case CTS_END_CONFIRM:
    CTVT_TALK_UpdateEndConfirm( work , talkWork );
    break;

  //親機による子機の終了待ち
  case CTS_WAIT_FINISH_CHILD_INIT:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      const BOOL ret = CTVT_COMM_SendFlg( work , commWork , CCFT_FINISH_PARENT , 0 );
      if( ret == TRUE )
      {
        talkWork->state = CTS_WAIT_FINISH_CHILD;
      }
    }
    //送信リクエスト中に子が落ちてるかもしれないのでスルー
    //break;

  case CTS_WAIT_FINISH_CHILD:
    if( COMM_TVT_GetConnectNum( work ) <= 1 )
    {
      talkWork->subState = CTSS_GO_END;
      talkWork->state = CTS_FADEOUT_BOTH;
      COMM_TVT_SetSusspend( work , TRUE );
    }
    break;

  //親のリクエストによる終了
  case CTS_END_PARENT_REQ_INIT:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      CTVT_COMM_ExitComm( work , commWork );
      
      CTVT_TALK_DispMessage( work , talkWork , COMM_TVT_SYS_05 );
      talkWork->state = CTS_END_PARENT_REQ;
    }
    break;
  case CTS_END_PARENT_REQ:
    if( GFL_UI_TP_GetTrg() == TRUE )
    {
      talkWork->subState = CTSS_GO_END;
      talkWork->state = CTS_FADEOUT_BOTH;
      COMM_TVT_SetSusspend( work , TRUE );
    }
    break;
  }

  CTVT_TALK_UpdateButton( work , talkWork );
  CTVT_MIC_Main( talkWork->micWork );

  if( talkWork->isUpdateMsgRec == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( talkWork->recWin )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( talkWork->recWin );
      talkWork->isUpdateMsgRec = FALSE;
    }
  }
  if( talkWork->isUpdateMsgDraw == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( talkWork->drawWin )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( talkWork->drawWin );
      talkWork->isUpdateMsgDraw = FALSE;
    }
  }
  if( talkWork->isUpdateMsgWin == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( talkWork->msgWin )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( talkWork->msgWin );
      GFL_BMPWIN_MakeScreen( talkWork->msgWin );
      GFL_BG_LoadScreenReq(CTVT_FRAME_SUB_MSG);
      talkWork->isUpdateMsgWin = FALSE;
    }
  }
  
  return CTM_TALK;
}


//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
static void CTVT_TALK_UpdateWait( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );

  const u8 connectNum = COMM_TVT_GetConnectNum( work );
  //u32 trgX,trgY,contX,contY;
  //const BOOL isTrg = GFL_UI_TP_GetPointTrg( &trgX,&trgY );
  //const BOOL isCont = GFL_UI_TP_GetPointTrg( &contX,&contY );

  if( COMM_TVT_GetFinishReq( work ) == TRUE )
  {
    //親のリクエストによる終了
    talkWork->state = CTS_END_PARENT_REQ_INIT;
    return;
  }

  //会話ボタン
  if( connectNum == 1 )
  {
    talkWork->recButtonState = CRBT_DISALE;
  }
  else
  {
    const u8 selfNexId = CTVT_COMM_GetSelfNetId( work , commWork );
    const u8 talkMember = CTVT_COMM_GetTalkMember( work , commWork );
    BOOL isActiveRecButton;
    
    if( talkMember != CTVT_COMM_INVALID_MEMBER )
    {
      if( talkMember == selfNexId )
      {
        CTVT_COMM_SendFlg( work , commWork , CCFT_CANCEL_TALK , 0 );
      }
      talkWork->recButtonState = CRBT_DISALE;
    }
    else
    {
      if( GFL_UI_KEY_GetTrg() & CTVT_BUTTON_TALK )
      {
        talkWork->state = CTS_REQ_TALK;
      }
      else
      if( GFL_UI_TP_HitTrg( CTVT_TALK_HitRecButton ) == 0 )
      {
        talkWork->state = CTS_REQ_TALK;
      }
      
      talkWork->recButtonState = CRBT_NONE;
    }
  }
  
  //お絵描きボタン
  if( talkWork->state == CTS_WAIT )
  {
    if( GFL_UI_KEY_GetTrg() & CTVT_BUTTON_DRAW ||
        (GFL_UI_TP_HitTrg( CTVT_TALK_HitDrawButton ) == 0) )
    {
      talkWork->state = CTS_FADEOUT_BOTH;
      talkWork->subState = CTSS_GO_DRAW;
    }
  }
  
  {
    static const GFL_UI_TP_HITTBL hitTbl[2] = 
    {
      {
        CTVT_TALK_BAR_ICON_Y-12 , CTVT_TALK_BAR_ICON_Y+12 ,
        CTVT_TALK_RETURN_X , CTVT_TALK_RETURN_X+24 ,
      },
      {GFL_UI_TP_HIT_END,0,0,0}
    };
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    if( ret == 0 )
    {
      talkWork->state = CTS_END_CONFIRM_INIT;
      /*
      talkWork->subState = CTSS_GO_END;
      talkWork->state = CTS_FADEOUT_BOTH;
      COMM_TVT_SetSusspend( work , TRUE );
      */
    }
  }

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    talkWork->subState = CTSS_GO_END;
    talkWork->state = CTS_FADEOUT_BOTH;
    COMM_TVT_SetSusspend( work , TRUE );
  }
  
  CTVT_TALK_UpdateVoiceBar( work , talkWork );
}

//--------------------------------------------------------------
//	録音部分の更新
//--------------------------------------------------------------
static void CTVT_TALK_UpdateTalk( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  const int isContTbl = GFL_UI_TP_HitCont( CTVT_TALK_HitRecButton );
  if( COMM_TVT_GetFinishReq( work ) == TRUE )
  {
    //親のリクエストによる終了
    talkWork->state = CTS_END_PARENT_REQ_INIT;
    return;
  }
  switch( talkWork->subState )
  {
  case CTSS_INIT_REC:
    {
      const BOOL ret = CTVT_MIC_StartRecord( talkWork->micWork );
      if( ret == TRUE )
      {
        talkWork->recButtonState = CRBT_TALKING;
        talkWork->subState = CTSS_RECORDING;
        talkWork->sendCnt = 0;
        talkWork->reqSendWave = FALSE;
        ENC_ADPCM_ResetParam();

        talkWork->wavePosX = 0;
        talkWork->wavePosY = CTVT_TALK_WAVE_DRAW_HEIGHT*8/2;
        GFL_BMP_Clear( GFL_BMPWIN_GetBmp( talkWork->waveWin) , 0xF );
      }
    }
    break;
  case CTSS_RECORDING:
    if( !(isContTbl == 0) && !(GFL_UI_KEY_GetCont() & CTVT_BUTTON_TALK) )
    {
      CTVT_MIC_StopRecord( talkWork->micWork );
    }

    if( CTVT_MIC_IsRecording( talkWork->micWork ) == FALSE )
    {
      talkWork->subState = CTSS_REC_TRANS;
      talkWork->recButtonState = CRBT_DISALE;
    }
    //波形を描く処理
    {
      const u32 recSize = CTVT_MIC_GetRecSize( talkWork->micWork );
      void* recBuffer = CTVT_MIC_GetRecBuffer( talkWork->micWork );
      const u32 waveLen = recSize * (CTVT_TALK_WAVE_DRAW_WIDTH*8) / CTVT_SEND_WAVE_SIZE;
      GFL_BMP_DATA*	bmpData = GFL_BMPWIN_GetBmp( talkWork->waveWin );
      u8 *charaBuf = GFL_BMP_GetCharacterAdrs( bmpData );
      BOOL isUpdate = FALSE;
      while( talkWork->wavePosX < waveLen )
      {
        const u32 bufOfs = (talkWork->wavePosX+1) * CTVT_SEND_WAVE_SIZE / (CTVT_TALK_WAVE_DRAW_WIDTH*8) /2; //u16の配列で取るから/2
        const s16 *buf = recBuffer;
        const s16 vol = buf[bufOfs];
        const u32 posY = (vol+0x8000)*((CTVT_TALK_WAVE_DRAW_HEIGHT-1)*8)/0x10000 +4;//上下に若干隙間を出すためサイズを-1して4ピクセル補填
        CTVT_TALK_DrawLine( work , talkWork , charaBuf , 
                            talkWork->wavePosX , talkWork->wavePosY ,
                            talkWork->wavePosX +1 , posY );
        talkWork->wavePosX++;
        talkWork->wavePosY = posY;
        
        isUpdate = TRUE;
        //OS_TFPrintf(3,"size[%5d][%3d][%3d]\n",recSize,waveLen,posY);
      }
      if( isUpdate == TRUE )
      {
        GFL_BMPWIN_TransVramCharacter( talkWork->waveWin );
      }

    }
    //転送は録音中もやるのでスキップ
    //break through;
  case CTSS_REC_TRANS:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      const BOOL isSendWave = CTVT_COMM_GetCommWaveData( work , commWork );
      const u32 recSize = CTVT_MIC_GetRecSize( talkWork->micWork );
      if( talkWork->reqSendWave == FALSE && isSendWave == FALSE )
      {
        if( recSize >= CTVT_SEND_WAVE_SIZE_ONE*(talkWork->sendCnt+1) ||
            talkWork->subState == CTSS_REC_TRANS)
        {
          //エンコード
          void* recBuffer = CTVT_MIC_GetRecBuffer( talkWork->micWork );
          void* sendTopBuf = (void*)((u32)recBuffer+talkWork->sendCnt*CTVT_SEND_WAVE_SIZE_ONE );
          const u32 endSize = CTVT_MIC_EncodeData( talkWork->micWork , sendTopBuf , talkWork->sendWaveBufTop , CTVT_SEND_WAVE_SIZE_ONE );
          //送信
          talkWork->sendWaveData->dataNo = talkWork->sendCnt;
          talkWork->sendWaveData->encSize = endSize;
          talkWork->sendWaveData->recSize = recSize;
          talkWork->sendWaveData->pitch = talkWork->sliderPos;
          if( talkWork->subState == CTSS_REC_TRANS && 
            recSize <= CTVT_SEND_WAVE_SIZE_ONE*(talkWork->sendCnt+1) )
          {
            talkWork->sendWaveData->isLast = TRUE;
          }
          else
          {
            talkWork->sendWaveData->isLast = FALSE;
          }
              
          talkWork->reqSendWave = TRUE;
        }
      }
      
      if( talkWork->reqSendWave == TRUE )
      {
        const BOOL ret = CTVT_COMM_SendWave( work , commWork , talkWork->sendWaveData );
        if( ret == TRUE )
        {
          talkWork->sendCnt++;
          talkWork->reqSendWave = FALSE;
          if( talkWork->sendWaveData->isLast == TRUE )
          {
            talkWork->subState = CTSS_WAIT_PLAY;
          }
        }
      }
    }

    break;
  case CTSS_WAIT_PLAY:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      const BOOL isSendWave = CTVT_COMM_GetCommWaveData( work , commWork );
      if( isSendWave == FALSE )
      {
        const BOOL ret = CTVT_COMM_SendFlg( work , commWork , CCFT_FINISH_TALK , 0 );
        if( ret == TRUE )
        {
          talkWork->state = CTS_WAIT;
        }
      }
    }
    break;
  }
}


//--------------------------------------------------------------
//	ボタンの色変更
//--------------------------------------------------------------
static void CTVT_TALK_UpdateButton( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  if( talkWork->recButtonState != talkWork->befRecButtonState )
  {
    u8 palNo;
    if( talkWork->recButtonState == CRBT_TALKING )
    {
      palNo = CTVT_PAL_BG_BUTTON_ACTIVE;
    }
    else
    if( talkWork->recButtonState == CRBT_DISALE )
    {
      palNo = CTVT_PAL_BG_BUTTON_DISABLE;
    }
    else
    {
      palNo = CTVT_PAL_BG_BUTTON_NONE;
    }
    
    GFL_BG_ChangeScreenPalette( CTVT_FRAME_SUB_MISC ,
                                CTVT_TALK_REC_BUTTON_LEFT , 
                                CTVT_TALK_REC_BUTTON_TOP , 
                                CTVT_TALK_REC_BUTTON_WIDTH , 
                                CTVT_TALK_REC_BUTTON_HEIGHT , 
                                palNo );
    GFL_BG_LoadScreenV_Req( CTVT_FRAME_SUB_MISC );
    talkWork->befRecButtonState = talkWork->recButtonState;
  }
  
}

static void CTVT_TALK_UpdateVoiceBar( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  if( talkWork->isHoldSlider == FALSE )
  {
    GFL_UI_TP_HITTBL hitTbl[2] = 
    {
      {
        0,  //計算で出すので外で
        0,
        CTVT_TALK_SLIDER_X-CTVT_TALK_SLIDER_WIDTH_H,
        CTVT_TALK_SLIDER_X+CTVT_TALK_SLIDER_WIDTH_H,
      },
      {GFL_UI_TP_HIT_END,0,0,0}
    };
    
    hitTbl[0].rect.top    = CTVT_TALK_SLIDER_Y + talkWork->sliderPos - CTVT_TALK_SLIDER_HEIGHT_H;
    hitTbl[0].rect.bottom = CTVT_TALK_SLIDER_Y + talkWork->sliderPos + CTVT_TALK_SLIDER_HEIGHT_H;
    if( GFL_UI_TP_HitTrg( hitTbl ) == 0 )
    {
      talkWork->isHoldSlider = TRUE;
    }
  }
  else
  {
    GFL_UI_TP_HITTBL hitTbl[2] = 
    {
      {
        CTVT_TALK_SLIDER_Y - CTVT_TALK_SLIDER_MOVE_Y - CTVT_TALK_SLIDER_HEIGHT_H,
        CTVT_TALK_SLIDER_Y + CTVT_TALK_SLIDER_MOVE_Y + CTVT_TALK_SLIDER_HEIGHT_H,
        CTVT_TALK_SLIDER_X-CTVT_TALK_SLIDER_WIDTH_H,
        CTVT_TALK_SLIDER_X+CTVT_TALK_SLIDER_WIDTH_H,
      },
      {GFL_UI_TP_HIT_END,0,0,0}
    };
    if( GFL_UI_TP_HitCont( hitTbl ) == 0 )
    {
      u32 tpx,tpy;
      GFL_CLACTPOS cellPos;
      GFL_UI_TP_GetPointCont(&tpx,&tpy);
      
      talkWork->sliderPos = tpy-CTVT_TALK_SLIDER_Y;
      if( talkWork->sliderPos > CTVT_TALK_SLIDER_MOVE_Y )
      {
        talkWork->sliderPos = CTVT_TALK_SLIDER_MOVE_Y;
      }
      if( talkWork->sliderPos < -CTVT_TALK_SLIDER_MOVE_Y )
      {
        talkWork->sliderPos = -CTVT_TALK_SLIDER_MOVE_Y;
      }
      cellPos.x = CTVT_TALK_SLIDER_X;
      cellPos.y = CTVT_TALK_SLIDER_Y+talkWork->sliderPos;
      GFL_CLACT_WK_SetPos( talkWork->clwkSlider , &cellPos , CLSYS_DRAW_SUB );
    }
    else
    {
      talkWork->isHoldSlider = FALSE;
    }
  }

}

static void CTVT_TALK_DrawLine( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork , u8 *charBuf , u8 x1 , u8 y1 , u8 x2 , u8 y2 )
{
  //精度確保でfx32
  const int xLen = MATH_ABS( x1-x2 );
  const int yLen = MATH_ABS( y1-y2 );
  const int loopNum = ( xLen > yLen ? xLen : yLen )+1;
  fx32 addX = FX32_CONST(x2-x1) / loopNum;
  fx32 addY = FX32_CONST(y2-y1) / loopNum;
  fx32 subX = 0;
  fx32 subY = 0;
  u8 i;
  OS_TFPrintf(3,"[%d:%d][%d:%d]\n",x1,y1,x2,y2 );
  for( i=0;i<loopNum;i++ )
  {
    const int posX = x1 + (subX>>FX32_SHIFT);
    const int posY = y1 + (subY>>FX32_SHIFT);
    
    CTVT_TALK_DrawDot( charBuf , posX , posY );
    
    subX += addX;
    subY += addY;
  }
  
}
static void CTVT_TALK_DrawDot( u8 *charBuf , u8 x , u8 y )
{
  const u8 drawCol = 0xA;

  const int charX = x/8;
  const int charY = y/8;
  const u16 charNo = charX+charY*CTVT_TALK_WAVE_DRAW_WIDTH;
  const u8 ofsX = x%8;
  const u8 ofsY = y%8;
  const u8 ofsXmod = ofsX%4;
  const u32 ofsAdr = (ofsX/4) + ofsY*2;
  u16 *trgAdr = (u16*)((u32)charBuf + charNo*0x20 + ofsAdr*2);
  static const u16 shiftArr[4]={0x0001*drawCol,
                                0x0010*drawCol,
                                0x0100*drawCol,
                                0x1000*drawCol};
  static const u16 maskArr[4]= {0xFFF0,
                                0xFF0F,
                                0xF0FF,
                                0x0FFF};
  if( (u32)trgAdr <  (u32)charBuf+(0x20*CTVT_TALK_WAVE_DRAW_WIDTH*CTVT_TALK_WAVE_DRAW_HEIGHT) &&
      (u32)trgAdr >= (u32)charBuf )
  {
    *trgAdr = (*trgAdr & maskArr[ofsXmod]) + shiftArr[ofsXmod];
  }
}

static void CTVT_TALK_DispMessage( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork , const u16 msgId )
{
  GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
  GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
  PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
  STRBUF *str;
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( talkWork->msgWin) , 0xF );
  str = GFL_MSG_CreateString( msgHandle , msgId );
  PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( talkWork->msgWin ) , 
          0 , 0 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
  GFL_STR_DeleteBuffer( str );

  BmpWinFrame_Write( talkWork->msgWin , WINDOW_TRANS_OFF , 
                      CTVT_BMPWIN_CGX , CTVT_PAL_BG_SUB_WINFRAME );
  talkWork->isUpdateMsgWin = TRUE;

}

static void CTVT_TALK_InitEndConfirm( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{

  if( COMM_TVT_GetSelfIdx( work ) == 0 &&
      COMM_TVT_GetConnectNum( work ) > 1 )
  {
    //親機終了確認
    CTVT_TALK_DispMessage( work , talkWork , COMM_TVT_SYS_02 );
  }
  else
  {
    //子機終了確認
    CTVT_TALK_DispMessage( work , talkWork , COMM_TVT_SYS_01 );
  }
  
  talkWork->yesNoWork = COMM_TVT_OpenYesNoMenu( work );
  
  talkWork->state = CTS_END_CONFIRM;
  
}

static void CTVT_TALK_UpdateEndConfirm( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  if( COMM_TVT_GetFinishReq( work ) == TRUE )
  {
    //親のリクエストによる終了
    talkWork->state = CTS_END_PARENT_REQ_INIT;
    APP_TASKMENU_CloseMenu( talkWork->yesNoWork );
    talkWork->yesNoWork = NULL;
    return;
  }
  APP_TASKMENU_UpdateMenu( talkWork->yesNoWork );
  if( APP_TASKMENU_IsFinish( talkWork->yesNoWork ) == TRUE )
  {
    const u8 retVal = APP_TASKMENU_GetCursorPos( talkWork->yesNoWork );
    if( retVal == 0 )
    {
      if( COMM_TVT_GetSelfIdx( work ) == 0 &&
          COMM_TVT_GetConnectNum( work ) > 1 )
      {
        talkWork->state = CTS_WAIT_FINISH_CHILD_INIT;
      }
      else
      {
        talkWork->subState = CTSS_GO_END;
        talkWork->state = CTS_FADEOUT_BOTH;
        COMM_TVT_SetSusspend( work , TRUE );
      }
    }
    else
    {
      talkWork->state = CTS_WAIT;
      BmpWinFrame_Clear( talkWork->msgWin , WINDOW_TRANS_ON_V );

      GFL_BMPWIN_ClearScreen( talkWork->msgWin );
      GFL_BMPWIN_MakeScreen( talkWork->recWin );
      GFL_BMPWIN_MakeScreen( talkWork->drawWin );
      GFL_BMPWIN_MakeScreen( talkWork->waveWin );
      GFL_BG_LoadScreenReq(CTVT_FRAME_SUB_MSG);

    }
    APP_TASKMENU_CloseMenu( talkWork->yesNoWork );
    talkWork->yesNoWork = NULL;
  }
}


#pragma mark[>outer func
CTVT_MIC_WORK* CTVT_TALK_GetMicWork( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  return talkWork->micWork;
}
