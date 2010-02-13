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
#include "ctvt_snd_def.h"
#include "enc_adpcm.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define CTVT_TALK_SLIDER_X (5)
#define CTVT_TALK_SLIDER_Y (116)
//#define CTVT_TALK_SLIDER_MOVE_Y (40)  //上にも下にも40  //外に出した
#define CTVT_TALK_SLIDER_WIDTH_H  (12)    //HALF
#define CTVT_TALK_SLIDER_HEIGHT_H (4) //HALF

#define CTVT_TALK_BAR_ICON_Y (192-12)
#define CTVT_TALK_YOBIDASHI_X (152)
#define CTVT_TALK_PAUSE_X (192)
#define CTVT_TALK_RETURN_X (232)

#define CTVT_TALK_REC_BUTTON_LEFT (7)
#define CTVT_TALK_REC_BUTTON_TOP  (12)
#define CTVT_TALK_REC_BUTTON_WIDTH (18)
#define CTVT_TALK_REC_BUTTON_HEIGHT (9)

#define CTVT_TALK_WAVE_DRAW_WIDTH   (16)
#define CTVT_TALK_WAVE_DRAW_HEIGHT  (8)

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

  //WIFIのリクエストによる終了
  CTS_END_WIFI_REQ_SEND,
  CTS_END_WIFI_REQ_INIT_DISP,
  CTS_END_WIFI_REQ_DISP,
  CTS_END_WIFI_REQ_INIT,
  CTS_END_WIFI_REQ,

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
  u8          sliderPos;
  
  //録音関係  
  CTVT_MIC_WORK *micWork;
  BOOL  reqSendWave;
  u8    sendCnt;
  
  void  *sendWaveBuf;  //下の2つはここからアドレスを持つ
  CTVT_COMM_WAVE_HEADER *sendWaveData;
  void *sendWaveBufTop;
  
  BOOL isUpdateMsgWin;
  GFL_BMPWIN *waveWin;  //波形を描く
  u8  wavePosX;
  u8  wavePosY;

  BOOL blinkButtonReq;
  u8   blinkButtonCnt;
  u16  blinkButtonCol[16];
  u16  blinkButtonBuf[16];

  GFL_BMPWIN *msgWin;
  APP_TASKMENU_WORK *yesNoWork;
  
  //セル関係
  GFL_CLWK    *clwkSlider;
  GFL_CLWK    *clwkPause;
  GFL_CLWK    *clwkReturn;
  GFL_CLWK    *clwkDraw;
  GFL_CLWK    *clwkTalk;

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

static const BOOL CTVT_TALK_CheckFinishReq( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork );

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

//形が矩形ではないので2個判定
static const GFL_UI_TP_HITTBL CTVT_TALK_HitDrawButton[3] = 
{
  {   8, 88 ,184, 240 },
  {  40,104 ,208, 255 },
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
  talkWork->sliderPos = 4;
  
  //アニメ用に、パレットを退避
  GFL_STD_MemCopy16( (void*)(HW_DB_BG_PLTT+CTVT_PAL_BG_BUTTON_NONE*32) , talkWork->blinkButtonBuf , 16*2 );
  
  for( i=0;i<16;i++ )
  {
    talkWork->blinkButtonCol[i] = 0x7FFF;
  }
  
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

  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NCGR ,
                    CTVT_FRAME_SUB_MISC , 0 , 0, FALSE , heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NSCR , 
                    CTVT_FRAME_SUB_MISC ,  0 , 0, FALSE , heapId );

  GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_comm_tvt_tv_t_tuuwa_bg_NCGR ,
                    CTVT_FRAME_SUB_BAR , 0 , 0x5000, FALSE , heapId );
  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_comm_tvt_tv_t_cur_bg_NSCR , 
                    CTVT_FRAME_SUB_BAR ,  0 , 0, FALSE , heapId );

  G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1 , GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_OBJ , 5 , 15 );

  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_MISC );
  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_BAR );
  
  {
    //スライダー
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 128;
    cellInitData.pos_y = 96;
    cellInitData.anmseq = CTOAS_PITCH_1 + talkWork->sliderPos;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 2;
    
    talkWork->clwkSlider = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    GFL_CLACT_WK_SetDrawEnable( talkWork->clwkSlider , TRUE );

    //一時停止ボタン
    if( COMM_TVT_GetPause( work ) == TRUE )
    {
      cellInitData.anmseq = CTOAS_TALK_PAUSE_ON;
    }
    else
    {
      cellInitData.anmseq = CTOAS_TALK_PAUSE_OFF;
    }
    
    talkWork->clwkPause = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    GFL_CLACT_WK_SetDrawEnable( talkWork->clwkPause , TRUE );

    //お絵描きボタン
    cellInitData.anmseq = CTOAS_GO_DRAW;
    talkWork->clwkDraw = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    GFL_CLACT_WK_SetDrawEnable( talkWork->clwkDraw , TRUE );

    //通話ボタン
    cellInitData.anmseq = CTOAS_TALK_OFF;
    talkWork->clwkTalk = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    GFL_CLACT_WK_SetDrawEnable( talkWork->clwkTalk , TRUE );

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

  talkWork->waveWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MSG , 
                                        8 , 3 , 
                                        CTVT_TALK_WAVE_DRAW_WIDTH , 
                                        CTVT_TALK_WAVE_DRAW_HEIGHT ,
                                        CTVT_PAL_BG_SUB_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
  //MakeScreenは文字表示の時
  talkWork->msgWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MSG , 
                                        1 , 1 , 30 , 4 ,
                                        CTVT_PAL_BG_SUB_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( talkWork->waveWin) , 0x0 );
  GFL_BMPWIN_MakeScreen( talkWork->waveWin );
  GFL_BMPWIN_TransVramCharacter( talkWork->waveWin );

  talkWork->yesNoWork = NULL;
  talkWork->isUpdateMsgWin = FALSE;
  GFL_BG_LoadScreenReq(CTVT_FRAME_SUB_MSG);

  talkWork->blinkButtonReq = FALSE;
  talkWork->blinkButtonCnt = 0;

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

  GFL_BMPWIN_ClearTransWindow( talkWork->waveWin );
  GFL_BMPWIN_ClearTransWindow( talkWork->msgWin );
  GFL_BMPWIN_Delete( talkWork->waveWin );
  GFL_BMPWIN_Delete( talkWork->msgWin );
  
  GFL_CLACT_WK_Remove( talkWork->clwkReturn );
  GFL_CLACT_WK_Remove( talkWork->clwkTalk );
  GFL_CLACT_WK_Remove( talkWork->clwkDraw );
  GFL_CLACT_WK_Remove( talkWork->clwkPause );
  GFL_CLACT_WK_Remove( talkWork->clwkSlider );
  GFL_BG_ClearScreen( CTVT_FRAME_SUB_MISC );
  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_MISC );

  G2S_BlendNone();
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
    if( talkWork->subState == CTSS_GO_DRAW &&
        GFL_CLACT_WK_CheckAnmActive( talkWork->clwkDraw ) == TRUE )
    {
      break;
    }
        
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
    if( CTVT_TALK_CheckFinishReq( work , talkWork ) == FALSE )
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
    if( CTVT_TALK_CheckFinishReq( work , talkWork ) == FALSE )
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
    if( GFL_UI_TP_GetTrg() == TRUE ||
        GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) )
    {
      talkWork->subState = CTSS_GO_END;
      talkWork->state = CTS_FADEOUT_BOTH;
      COMM_TVT_SetSusspend( work , TRUE );
    }
    break;
    
    //WIFIのリクエストによる終了
    
  case CTS_END_WIFI_REQ_SEND:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      const BOOL ret = CTVT_COMM_SendFlg( work , commWork , CCFT_FINISH_PARENT , 0 );
      if( ret == TRUE )
      {
        talkWork->state = CTS_END_WIFI_REQ_INIT;
      }
    }
    break;

  case CTS_END_WIFI_REQ_INIT_DISP:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      CTVT_COMM_ExitComm( work , commWork );
      
      CTVT_TALK_DispMessage( work , talkWork , COMM_TVT_SYS_06 );
      talkWork->state = CTS_END_WIFI_REQ_DISP;
    }
    break;
    
  case CTS_END_WIFI_REQ_DISP:
    if( GFL_UI_TP_GetTrg() == TRUE ||
        GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) )
    {
      talkWork->state = CTS_END_WIFI_REQ_INIT;
    }
    break;
    
  case CTS_END_WIFI_REQ_INIT:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      CTVT_COMM_SendTimingCommnad( work , commWork , CTVT_COMM_TIMING_END );
      talkWork->state = CTS_END_WIFI_REQ;
    }
    break;
    
  case CTS_END_WIFI_REQ:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      if( CTVT_COMM_CheckTimingCommnad( work , commWork , CTVT_COMM_TIMING_END ) == TRUE )
      {
        talkWork->subState = CTSS_GO_END;
        talkWork->state = CTS_FADEOUT_BOTH;
        COMM_TVT_SetSusspend( work , TRUE );
      }
    }
    break;
  }

  CTVT_TALK_UpdateButton( work , talkWork );
  CTVT_MIC_Main( talkWork->micWork );

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

  //ボタンの点灯
  if( talkWork->blinkButtonReq == TRUE )
  {
    static const u8 blinkTime = 6;
    static const u8 blinkNum = 1;

    talkWork->blinkButtonCnt++;
    if( talkWork->blinkButtonCnt > blinkTime*blinkNum )
    {
      talkWork->blinkButtonReq = FALSE;
      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                          CTVT_PAL_BG_BUTTON_NONE * 32,
                                          talkWork->blinkButtonBuf , 32 );
    }
    else
    {
      if( ( talkWork->blinkButtonCnt%(blinkTime*2) ) < blinkTime )
      {
        NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                            CTVT_PAL_BG_BUTTON_NONE * 32,
                                            talkWork->blinkButtonCol , 32 );
      }
      else
      {
        NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                            CTVT_PAL_BG_BUTTON_NONE * 32,
                                            talkWork->blinkButtonBuf , 32 );
      }
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

  if( CTVT_TALK_CheckFinishReq( work , talkWork ) == TRUE )
  {
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
      if( GFL_UI_KEY_GetTrg() & CTVT_BUTTON_TALK ||
          GFL_UI_TP_HitTrg( CTVT_TALK_HitRecButton ) == 0 )
      {
        talkWork->state = CTS_REQ_TALK;
        PMSND_PlaySystemSE( CTVT_SND_TOUCH );

        talkWork->blinkButtonReq = TRUE;
        talkWork->blinkButtonCnt = 0;
      }
      
      talkWork->recButtonState = CRBT_NONE;
    }
  }
  
  //お絵描きボタン
  if( talkWork->state == CTS_WAIT )
  {
    if( GFL_UI_KEY_GetTrg() & CTVT_BUTTON_DRAW ||
        GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ||
        (GFL_UI_TP_HitTrg( CTVT_TALK_HitDrawButton ) != GFL_UI_TP_HIT_NONE ) )
    {
      talkWork->state = CTS_FADEOUT_BOTH;
      talkWork->subState = CTSS_GO_DRAW;
      GFL_CLACT_WK_SetAnmSeq( talkWork->clwkDraw , CTOAS_GO_DRAW );
      GFL_CLACT_WK_SetAutoAnmFlag( talkWork->clwkDraw , TRUE );
      PMSND_PlaySystemSE( CTVT_SND_TOUCH );
    }
  }
  
  {
    static const GFL_UI_TP_HITTBL hitTbl[3] = 
    {
      //戻る
      {
        CTVT_TALK_BAR_ICON_Y-12 , CTVT_TALK_BAR_ICON_Y+12 ,
        CTVT_TALK_RETURN_X , CTVT_TALK_RETURN_X+24 ,
      },
      //一時停止
      { 104, 160, 216, 255 },
      {GFL_UI_TP_HIT_END,0,0,0}
    };
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    if( ret == 0 ||
        GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      talkWork->state = CTS_END_CONFIRM_INIT;
      PMSND_PlaySystemSE( CTVT_SND_CANCEL );
    }
    else
    if( ret == 1 ||
        GFL_UI_KEY_GetTrg() & CTVT_BUTTON_PAUSE )
    {
      COMM_TVT_FlipPause( work );
      PMSND_PlaySystemSE( CTVT_SND_PAUSE );
      if( COMM_TVT_GetPause( work ) == TRUE )
      {
        GFL_CLACT_WK_SetAnmSeq( talkWork->clwkPause , CTOAS_TALK_PAUSE_ON );
      }
      else
      {
        GFL_CLACT_WK_SetAnmSeq( talkWork->clwkPause , CTOAS_TALK_PAUSE_OFF );
      }
    }
  }
  if( COMM_TVT_GetSelfIdx(work) == 0 &&
      GFL_UI_KEY_GetTrg() & PAD_BUTTON_L )
  {
    COMM_TVT_FlipDoubleMode( work );
    PMSND_PlaySystemSE( CTVT_SND_TOUCH );
  }

  CTVT_TALK_UpdateVoiceBar( work , talkWork );
}

//--------------------------------------------------------------
//	録音部分の更新
//--------------------------------------------------------------
static void CTVT_TALK_UpdateTalk( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  const int isContTbl = GFL_UI_TP_HitCont( CTVT_TALK_HitRecButton );
  if( CTVT_TALK_CheckFinishReq( work , talkWork ) == TRUE )
  {
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

        talkWork->wavePosX = 3;
        talkWork->wavePosY = CTVT_TALK_WAVE_DRAW_HEIGHT*8/2;
        GFL_BMP_Clear( GFL_BMPWIN_GetBmp( talkWork->waveWin) , 0x0 );
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
      const u32 waveLen = recSize * ((CTVT_TALK_WAVE_DRAW_WIDTH*8)-5) / CTVT_SEND_WAVE_SIZE;
      GFL_BMP_DATA*	bmpData = GFL_BMPWIN_GetBmp( talkWork->waveWin );
      u8 *charaBuf = GFL_BMP_GetCharacterAdrs( bmpData );
      BOOL isUpdate = FALSE;
      while( talkWork->wavePosX < waveLen )
      {
        //高さ補正用
        static const u8 maxHeight[9] = {3,7,11,14,17,21,24,32,32};
        const u32 bufOfs = (talkWork->wavePosX+1) * CTVT_SEND_WAVE_SIZE / (CTVT_TALK_WAVE_DRAW_WIDTH*8) /2; //u16の配列で取るから/2
        const s16 *buf = recBuffer;
        const s16 vol = buf[bufOfs];
        u32 posY = (vol+0x8000)*(CTVT_TALK_WAVE_DRAW_HEIGHT*8)/0x10000;
        if( talkWork->wavePosX < 3+8 )
        {
          if( posY < (CTVT_TALK_WAVE_DRAW_HEIGHT*4) - maxHeight[talkWork->wavePosX-3] )
          {
            posY = (CTVT_TALK_WAVE_DRAW_HEIGHT*4) - maxHeight[talkWork->wavePosX-3];
          }
        }
        if( talkWork->wavePosX > (CTVT_TALK_WAVE_DRAW_WIDTH*8)-6-8 )
        {
          const u8 idx = (CTVT_TALK_WAVE_DRAW_WIDTH*8)-6 - talkWork->wavePosX;
          if( posY < (CTVT_TALK_WAVE_DRAW_HEIGHT*4) - maxHeight[idx] )
          {
            posY = (CTVT_TALK_WAVE_DRAW_HEIGHT*4) - maxHeight[idx];
          }
        }
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
          GFL_BMP_Clear( GFL_BMPWIN_GetBmp( talkWork->waveWin) , 0x0 );
          GFL_BMPWIN_TransVramCharacter( talkWork->waveWin );
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
    if( talkWork->recButtonState == CRBT_TALKING )
    {
      GFL_CLACT_WK_SetAnmSeq( talkWork->clwkTalk , CTOAS_TALK_ON );
    }
    else
    if( talkWork->recButtonState == CRBT_DISALE )
    {
      GFL_CLACT_WK_SetAnmSeq( talkWork->clwkTalk , CTOAS_TALK_OFF );
    }
    else
    {
      GFL_CLACT_WK_SetAnmSeq( talkWork->clwkTalk , CTOAS_TALK_OFF );
    }
  }

  /*
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
    GFL_BG_ChangeScreenPalette( CTVT_FRAME_SUB_MISC ,
                                CTVT_TALK_REC_BUTTON_LEFT2 , 
                                CTVT_TALK_REC_BUTTON_TOP2 , 
                                CTVT_TALK_REC_BUTTON_WIDTH2 , 
                                CTVT_TALK_REC_BUTTON_HEIGHT2 , 
                                palNo );
    GFL_BG_LoadScreenV_Req( CTVT_FRAME_SUB_MISC );
    talkWork->befRecButtonState = talkWork->recButtonState;
  }
  */
}

static void CTVT_TALK_UpdateVoiceBar( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  static const GFL_UI_TP_HITTBL hitTbl[4] = 
  {
    {  8,  40 ,16 , 72 }, //上１
    { 32,  77 , 0 , 64 }, //上２
    { 77, 160 , 0 , 48 }, //下
    {GFL_UI_TP_HIT_END,0,0,0}
  };
  const int ret = GFL_UI_TP_HitTrg( hitTbl );
  BOOL isTouhc = FALSE;
  
  if( ret == 0 || ret == 1 )
  {
    if( talkWork->sliderPos < CTVT_PITCH_MAX )
    {
      isTouhc = TRUE;
      talkWork->sliderPos++;
      GFL_CLACT_WK_SetAnmSeq( talkWork->clwkSlider , CTOAS_PITCH_1+talkWork->sliderPos );
    }
  }
  else
  if( ret  == 2 )
  {
    if( talkWork->sliderPos > CTVT_PITCH_MIN )
    {
      isTouhc = TRUE;
      talkWork->sliderPos--;
      GFL_CLACT_WK_SetAnmSeq( talkWork->clwkSlider , CTOAS_PITCH_1+talkWork->sliderPos );
    }
  }
  
  if( isTouhc == TRUE )
  {
    const int pitch = (talkWork->sliderPos - CTVT_PITCH_DEFAULT)*64;
    PMSND_PlaySE_byPlayerID( CTVT_SND_TOUCH , SEPLAYER_SE1 );
    PMSND_SetStatusSE_byPlayerID( SEPLAYER_SE1 , PMSND_NOEFFECT , pitch , PMSND_NOEFFECT );
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
  //OS_TFPrintf(3,"[%d:%d][%d:%d]\n",x1,y1,x2,y2 );
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
  //Fontパレット
  const u8 drawCol = 0x9;

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

  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
  if( CTVT_COMM_GetMode( work , commWork ) == CCIM_CONNECTED )
  {
    //Wifi終了確認
    CTVT_TALK_DispMessage( work , talkWork , COMM_TVT_SYS_01 );
  }
  else
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
  if( CTVT_TALK_CheckFinishReq( work , talkWork ) == TRUE )
  {
    //リクエストによる終了
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
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      if( CTVT_COMM_GetMode( work , commWork ) == CCIM_CONNECTED )
      {
        //Wifi終了確認
        talkWork->state = CTS_END_WIFI_REQ_SEND;
      }
      else
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
      GFL_BMPWIN_MakeScreen( talkWork->waveWin );
      GFL_BG_LoadScreenReq(CTVT_FRAME_SUB_MSG);

    }
    APP_TASKMENU_CloseMenu( talkWork->yesNoWork );
    talkWork->yesNoWork = NULL;
  }
}

static const BOOL CTVT_TALK_CheckFinishReq( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  if( COMM_TVT_GetFinishReq( work ) == TRUE )
  {
    CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
    if( CTVT_COMM_GetMode( work , commWork ) == CCIM_CONNECTED )
    {
      //Wifiのリクエストによる終了
      talkWork->state = CTS_END_WIFI_REQ_INIT_DISP;
    }
    else
    {
      //親のリクエストによる終了
      talkWork->state = CTS_END_PARENT_REQ_INIT;
    }
    return TRUE;
  }
  return FALSE;
}


#pragma mark[>outer func
CTVT_MIC_WORK* CTVT_TALK_GetMicWork( COMM_TVT_WORK *work , CTVT_TALK_WORK *talkWork )
{
  return talkWork->micWork;
}
