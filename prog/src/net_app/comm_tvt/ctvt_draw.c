//======================================================================
/**
 * @file	ctvt_talk.c
 * @brief	通信TVTシステム：落書き
 * @author	ariizumi
 * @data	09/12/19
 *
 * モジュール名：CTVT_DRAW
 */
//======================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/camera_system.h"
#include "system/bmp_winframe.h"
#include "net/network_define.h"
#include "app/app_menu_common.h"

#include "msg/msg_comm_tvt.h"

#include "comm_tvt.naix"

#include "ctvt_draw.h"
#include "ctvt_comm.h"
#include "ctvt_camera.h"
#include "ctvt_snd_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define CTVT_DRAW_BAR_ICON_Y (192-12)
#define CTVT_DRAW_PEN_X (16)
#define CTVT_DRAW_SPOITO_X (64)
#define CTVT_DRAW_KESHIGOMU_X (112)
#define CTVT_DRAW_STAMP_X (160)
#define CTVT_DRAW_PAUSE_X (192)
#define CTVT_DRAW_RETURN_X (224)

#define CTVT_DRAW_EXP_ICON_X      (56)
#define CTVT_DRAW_EXP_PEN_Y       ( 72)
#define CTVT_DRAW_EXP_SPOITO_Y    ( 92)
#define CTVT_DRAW_EXP_KESHIGOMU_Y (112)
#define CTVT_DRAW_EXP_STAMP_Y     (132)
#define CTVT_DRAW_EXP_RETURN_Y    (152-12)

#define CTVT_DRAW_PEN_SIZE_X (CTVT_DRAW_PEN_X)
#define CTVT_DRAW_PEN_SIZE_Y (192-32)
#define CTVT_DRAW_STAMP_TYPE_X (CTVT_DRAW_STAMP_X)
#define CTVT_DRAW_STAMP_TYPE_Y (192-32)

//文字位置
#define CTVT_DRAW_INFO_DRAW_X1 (8)
#define CTVT_DRAW_INFO_DRAW_X2 (80)

#define CTVT_DRAW_INFO_EDIT_X (64)
//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//編集ボタン種類
typedef enum
{
  CDED_PEN,
  CDED_SPOITO,
  CDED_KESHIGOMU,
  CDED_STAMP,
  CDED_PAUSE,
  CDED_RETURN,

  CDED_MAX,
}CTVT_DRAW_EDTI_BUTTON_TYPE;
//説明ボタン種類
typedef enum
{
  CDEX_PEN,
  CDEX_SPOITO,
  CDEX_KESHIGOMU,
  CDEX_STAMP,
  CDEX_RETURN,

  CDEX_MAX,
}CTVT_DRAW_EXPLAIN_BUTTON_TYPE;

//ステート
typedef enum
{
  CDS_FADEIN,
  CDS_FADEIN_WAIT,

  CDS_FADEOUT, //両方フェード
  CDS_FADEOUT_WAIT,

  CDS_DRAW,
  CDS_EDIT,
  
  //親のリクエストによる終了
  CDS_END_PARENT_REQ_INIT,
  CDS_END_PARENT_REQ,

  //WIFIのリクエストによる終了
  CDS_END_WIFI_REQ_SEND,
  CDS_END_WIFI_REQ_INIT_DISP,
  CDS_END_WIFI_REQ_DISP,
  CDS_END_WIFI_REQ_INIT,
  CDS_END_WIFI_REQ,
  
  
}CTVT_DRAW_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//ワーク
struct _CTVT_DRAW_WORK
{
  CTVT_DRAW_STATE state;
  
  u8 barScroll;     //目標座標
  u8 dispBarScroll; //実座標
  
  BOOL isTouch; //落書ききのTrgをはじくフラグ
  BOOL isHold;
  BOOL isDispPenSize;
  BOOL isDispStampType;
  BOOL canDraw;
  BOOL isFinish;
  CTVT_DRAW_EDTI_BUTTON_TYPE editMode;

  u32 befPenX;
  u32 befPenY;
  
  u16 penCol;
  u8  penSize;
  u8  stampType;

  u32 samplingRes;
  TP_ONE_DATA	tpData;

  
  BOOL isUpdateInfo;    //上画面用
  BOOL isUpdateInfoEdit;//上画面用の更新がEditモードか？
  BOOL isUpdateTitle;    //上画面用
  BOOL isUpdateMsgWin;  //下画面用
  GFL_BMPWIN *infoWin;  //上画面用
  GFL_BMPWIN *titleWin; //上画面用
  GFL_BMPWIN *msgWin;   //下画面用
  
  //セル関係
  GFL_CLWK    *clwkEditButton[CDED_MAX];
  GFL_CLWK    *clwkExplainButton[CDEX_MAX];
  GFL_CLWK    *clwkPenSize;
  GFL_CLWK    *clwkStampType;
  
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void CTVT_DRAW_UpdateDraw( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork );
static void CTVT_DRAW_UpdateEdit( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork );

static void CTVT_DRAW_UpdateBar( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork );
static void CTVT_DRAW_UpdateDrawing( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork );

static void CTVT_DRAW_UpdateEditButton( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork );

static void CTVT_DRAW_DrawInfoMsg( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork , const BOOL isEditMode );

static const BOOL CTVT_DRAW_CheckFinishReq( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork );
static void CTVT_DRAW_DispMessage( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork , const u16 msgId );

static void CTVT_DRAW_EreseSubWindow( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork );

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
CTVT_DRAW_WORK* CTVT_DRAW_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId )
{
  u8 i;
  CTVT_DRAW_WORK* drawWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_DRAW_WORK) );

  drawWork->penSize = DSPS_CIRCLE_1;
  drawWork->stampType = DSPS_STAMP_HEART;

  //バージョンで白黒を変える
#if (PM_VERSION == VERSION_WHITE )
  drawWork->penCol = 0x7FFF+0x8000;  //最上位ビットがα
#else
  drawWork->penCol = 0x0000+0x8000;  //最上位ビットがα
#endif
  return drawWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void CTVT_DRAW_TermSystem( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  GFL_HEAP_FreeMemory( drawWork );
}
//--------------------------------------------------------------
//	モード切替時初期化
//--------------------------------------------------------------
void CTVT_DRAW_InitMode( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
  ARCHANDLE* arcHandle = COMM_TVT_GetArcHandle( work );

  GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_comm_tvt_tv_t_paint_bg_NSCR , 
                    CTVT_FRAME_SUB_BG ,  0 , 0, FALSE , heapId );
  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_BG );
  GFL_BG_SetScroll( CTVT_FRAME_MAIN_MSG , GFL_BG_SCROLL_Y_SET , -24 );
  GFL_BG_SetVisible( CTVT_FRAME_SUB_BAR , FALSE );
  GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

  {
    //MSGと共用なのでここでスクリーンを読む
    ARCHANDLE *commonArcHandle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId() , heapId );
    GFL_BG_ClearScreen( CTVT_FRAME_MAIN_MSG );
    GFL_ARCHDL_UTIL_TransVramScreen( commonArcHandle , APP_COMMON_GetBarScrnArcIdx() , 
                      CTVT_FRAME_MAIN_MSG ,  0 , 0, FALSE , heapId );
    GFL_BG_ChangeScreenPalette( CTVT_FRAME_MAIN_MSG , 0 , 21 , 32 , 3 , CTVT_PAL_BG_MAIN_BAR );
    GFL_BG_LoadScreenReq( CTVT_FRAME_MAIN_MSG );
    
    GFL_BG_SetScroll( CTVT_FRAME_MAIN_MSG , GFL_BG_SCROLL_Y_SET , -24 );

    GFL_ARC_CloseDataHandle( commonArcHandle );
  }
  {
    u8 i;
    //編集ボタン
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = CTVT_DRAW_PEN_X;
    cellInitData.pos_y = CTVT_DRAW_BAR_ICON_Y+24;
    cellInitData.anmseq = CTOAM_PEN;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    
    drawWork->clwkEditButton[CDED_PEN] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_ANM ),
              &cellInitData ,CLSYS_DRAW_MAIN , heapId );
    
    cellInitData.pos_x = CTVT_DRAW_SPOITO_X;
    cellInitData.anmseq = CTOAM_SUPOITO;
    drawWork->clwkEditButton[CDED_SPOITO] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_ANM ),
              &cellInitData ,CLSYS_DRAW_MAIN , heapId );
    
    cellInitData.pos_x = CTVT_DRAW_KESHIGOMU_X;
    cellInitData.anmseq = CTOAM_KESHIGOMU;
    drawWork->clwkEditButton[CDED_KESHIGOMU] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_ANM ),
              &cellInitData ,CLSYS_DRAW_MAIN , heapId );
    
    cellInitData.pos_x = CTVT_DRAW_STAMP_X;
    cellInitData.anmseq = CTOAM_STAMP;
    drawWork->clwkEditButton[CDED_STAMP] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_ANM ),
              &cellInitData ,CLSYS_DRAW_MAIN , heapId );
    
    cellInitData.pos_x = CTVT_DRAW_PAUSE_X;
    if( COMM_TVT_GetPause( work ) == TRUE )
    {
      cellInitData.anmseq = CTOAM_PAUSE;
    }
    else
    {
      cellInitData.anmseq = CTOAM_PLAY;
    }
    drawWork->clwkEditButton[CDED_PAUSE] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_ANM ),
              &cellInitData ,CLSYS_DRAW_MAIN , heapId );

    cellInitData.pos_x = CTVT_DRAW_RETURN_X;
    cellInitData.pos_y = CTVT_DRAW_BAR_ICON_Y-12+24;
    cellInitData.anmseq = APP_COMMON_BARICON_RETURN;
    drawWork->clwkEditButton[CDED_RETURN] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_M_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_M_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_ANM ),
              &cellInitData ,CLSYS_DRAW_MAIN , heapId );
    for( i=0;i<CDED_MAX;i++ )
    {
      GFL_CLACT_WK_SetDrawEnable( drawWork->clwkEditButton[i] , TRUE );
    }
    
    //説明ボタン
    cellInitData.pos_x = CTVT_DRAW_EXP_ICON_X;
    cellInitData.pos_y = CTVT_DRAW_EXP_PEN_Y;
    cellInitData.anmseq = CTOAS_PEN;
    drawWork->clwkExplainButton[CDEX_PEN] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    
    cellInitData.pos_y = CTVT_DRAW_EXP_SPOITO_Y;
    cellInitData.anmseq = CTOAS_SUPOITO;
    drawWork->clwkExplainButton[CDEX_SPOITO] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    
    cellInitData.pos_y = CTVT_DRAW_EXP_KESHIGOMU_Y;
    cellInitData.anmseq = CTOAS_KESHIGOMU;
    drawWork->clwkExplainButton[CDEX_KESHIGOMU] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    
    cellInitData.pos_y = CTVT_DRAW_EXP_STAMP_Y;
    cellInitData.anmseq = CTOAS_STAMP;
    drawWork->clwkExplainButton[CDED_STAMP] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    
    cellInitData.pos_x = CTVT_DRAW_EXP_ICON_X-12;
    cellInitData.pos_y = CTVT_DRAW_EXP_RETURN_Y;
    cellInitData.anmseq = APP_COMMON_BARICON_RETURN;
    drawWork->clwkExplainButton[CDEX_RETURN] = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    for( i=0;i<CDEX_MAX;i++ )
    {
      GFL_CLACT_WK_SetDrawEnable( drawWork->clwkExplainButton[i] , FALSE );
    }
    
    //ペンサイズ
    cellInitData.pos_x = CTVT_DRAW_PEN_SIZE_X;
    cellInitData.pos_y = CTVT_DRAW_PEN_SIZE_Y;
    cellInitData.anmseq = CTOAM_PEN_SELECT_ANIME;
    drawWork->clwkPenSize = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_ANM ),
              &cellInitData ,CLSYS_DRAW_MAIN , heapId );
    GFL_CLACT_WK_SetDrawEnable( drawWork->clwkPenSize , FALSE );

    //スタンプ種類
    cellInitData.pos_x = CTVT_DRAW_STAMP_TYPE_X;
    cellInitData.pos_y = CTVT_DRAW_STAMP_TYPE_Y;
    cellInitData.anmseq = CTOAM_STAMP_SELECT_ANIME;
    drawWork->clwkStampType = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_COMMON_M_ANM ),
              &cellInitData ,CLSYS_DRAW_MAIN , heapId );
    GFL_CLACT_WK_SetDrawEnable( drawWork->clwkStampType , FALSE );


  }
  CTVT_DRAW_UpdateEditButton( work , drawWork );
  
  drawWork->infoWin = NULL;

  //MakeScreenは文字表示の時
  drawWork->msgWin = GFL_BMPWIN_Create( CTVT_FRAME_MAIN_MSG , 
                                        1 , 1 , 30 , 4 ,
                                        CTVT_PAL_BG_SUB_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );
  
  {
    GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
    GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    STRBUF *str;
    drawWork->titleWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MSG , 
                                          3 , 1 , 28 , 2 ,
                                          CTVT_PAL_BG_SUB_FONT ,
                                          GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( drawWork->titleWin) , 0x0 );
    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_012 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->titleWin ) , 
            0 , 0 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    drawWork->isUpdateTitle = TRUE;
  }
  
  
  drawWork->state = CDS_FADEIN;
  drawWork->barScroll = 24;
  drawWork->dispBarScroll = 24;
  drawWork->isHold = FALSE;
  drawWork->isTouch = FALSE;
  drawWork->editMode = CDED_PEN;
  drawWork->isDispPenSize = FALSE;
  drawWork->isDispStampType = FALSE;
  drawWork->isUpdateInfo = FALSE;
  drawWork->isUpdateMsgWin = FALSE;
  drawWork->canDraw = FALSE;
  drawWork->isFinish = FALSE;

  CTVT_DRAW_DrawInfoMsg( work , drawWork , FALSE );
  
  GFL_NET_WirelessIconEasy_HoldLCD( TRUE , heapId );
  GFL_NET_ReloadIcon();

  GFL_UI_TP_AutoStartNoBuff();
  
}

//--------------------------------------------------------------
//	モード切替時開放
//--------------------------------------------------------------
void CTVT_DRAW_TermMode( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  u8 i;
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
  
  //サンプリング終了
  GFL_UI_TP_AutoStop();

  GFL_NET_WirelessIconEasy_HoldLCD( FALSE , heapId );
  GFL_NET_ReloadIcon();

  BmpWinFrame_Clear( drawWork->infoWin , WINDOW_TRANS_ON );
  GFL_BMPWIN_ClearTransWindow( drawWork->infoWin );
  GFL_BMPWIN_Delete( drawWork->infoWin );
  GFL_BMPWIN_ClearTransWindow( drawWork->titleWin );
  GFL_BMPWIN_Delete( drawWork->titleWin );
  GFL_BMPWIN_ClearTransWindow( drawWork->msgWin );
  GFL_BMPWIN_Delete( drawWork->msgWin );
  
  for( i=0;i<CDEX_MAX;i++ )
  {
    GFL_CLACT_WK_Remove( drawWork->clwkExplainButton[i] );
  }
  for( i=0;i<CDED_MAX;i++ )
  {
    GFL_CLACT_WK_Remove( drawWork->clwkEditButton[i] );
  }
  GFL_CLACT_WK_SetDrawEnable( drawWork->clwkStampType , FALSE );
  GFL_CLACT_WK_Remove( drawWork->clwkStampType );
  GFL_CLACT_WK_SetDrawEnable( drawWork->clwkPenSize , FALSE );
  GFL_CLACT_WK_Remove( drawWork->clwkPenSize );

  GFL_BG_ClearScreen( CTVT_FRAME_MAIN_MSG );
  GFL_BG_LoadScreenReq( CTVT_FRAME_MAIN_MSG );
  GFL_BG_SetScroll( CTVT_FRAME_MAIN_MSG , GFL_BG_SCROLL_Y_SET , 0 );

  GFL_BG_ClearScreen( CTVT_FRAME_SUB_MISC );
  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_MISC );
  GFL_BG_SetVisible( CTVT_FRAME_SUB_BAR , TRUE );
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
  
  COMM_TVT_RedrawName( work );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
const COMM_TVT_MODE CTVT_DRAW_Main( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
  
  int i;
  drawWork->samplingRes = GFL_UI_TP_AutoSamplingMain( &drawWork->tpData, TP_BUFFERING_JUST, 1 );
  drawWork->isTouch = FALSE;
  
  switch( drawWork->state )
  {
  case CDS_FADEIN:
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
    drawWork->state = CDS_FADEIN_WAIT;
    break;
  case CDS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      drawWork->canDraw = TRUE;
      drawWork->state = CDS_DRAW;
    }
    break;

  case CDS_FADEOUT:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    drawWork->state = CDS_FADEOUT_WAIT;
    drawWork->canDraw = FALSE;
    break;

  case CDS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      if( drawWork->isFinish == FALSE )
      {
        return CTM_TALK;
      }
      else
      {
        return CTM_END;
      }
    }
    break;
    
  case CDS_DRAW:
    if( CTVT_DRAW_CheckFinishReq( work , drawWork ) == FALSE )
    {
      CTVT_DRAW_UpdateDraw( work , drawWork );
    }
    break;

  case CDS_EDIT:
    if( CTVT_DRAW_CheckFinishReq( work , drawWork ) == FALSE )
    {
      CTVT_DRAW_UpdateEdit( work , drawWork );
    }
    break;
    
  //親のリクエストによる終了
  case CDS_END_PARENT_REQ_INIT:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      CTVT_COMM_ExitComm( work , commWork );
      
      CTVT_DRAW_DispMessage( work , drawWork , COMM_TVT_SYS_05 );
      drawWork->state = CDS_END_PARENT_REQ;
    }
    break;
  case CDS_END_PARENT_REQ:
    if( GFL_UI_TP_GetTrg() == TRUE ||
        GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) )
    {
      drawWork->isFinish = TRUE;
      drawWork->state = CDS_FADEOUT;
      COMM_TVT_SetSusspend( work , TRUE );
    }
    break;

  //WIFIのリクエストによる終了
  case CDS_END_WIFI_REQ_SEND:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      const BOOL ret = CTVT_COMM_SendFlg( work , commWork , CCFT_FINISH_PARENT , 0 );
      if( ret == TRUE )
      {
        drawWork->state = CDS_END_WIFI_REQ_INIT_DISP;
      }
    }
    break;
  case CDS_END_WIFI_REQ_INIT_DISP:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      CTVT_COMM_ExitComm( work , commWork );
      
      CTVT_DRAW_DispMessage( work , drawWork , COMM_TVT_SYS_06 );
      drawWork->state = CDS_END_WIFI_REQ_DISP;
    }
    break;
  case CDS_END_WIFI_REQ_DISP:
    if( GFL_UI_TP_GetTrg() == TRUE ||
        GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) )
    {
      drawWork->state = CDS_END_WIFI_REQ_INIT;
    }
    break;
  case CDS_END_WIFI_REQ_INIT:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      CTVT_COMM_SendTimingCommnad( work , commWork , CTVT_COMM_TIMING_END );
      drawWork->state = CDS_END_WIFI_REQ;
    }
    break;
  case CDS_END_WIFI_REQ:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      if( CTVT_COMM_CheckTimingCommnad( work , commWork , CTVT_COMM_TIMING_END ) == TRUE )
      {
        drawWork->isFinish = TRUE;
        drawWork->state = CDS_FADEOUT;
        COMM_TVT_SetSusspend( work , TRUE );
      }
    }
    break;
  }
  
  //バースクロール更新
  CTVT_DRAW_UpdateBar( work,drawWork );
  
  //落書き更新
  if( drawWork->canDraw == TRUE )
  {
    CTVT_DRAW_UpdateDrawing( work,drawWork );
  }
  
  if( drawWork->isUpdateInfo == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( drawWork->infoWin );
      GFL_BMPWIN_MakeScreen( drawWork->infoWin );
      GFL_BG_LoadScreenReq(CTVT_FRAME_SUB_MSG);
      drawWork->isUpdateInfo = FALSE;
      
      if( drawWork->isUpdateInfoEdit == TRUE )
      {
        u8 i;
        for( i=0;i<CDEX_MAX;i++ )
        {
          GFL_CLACT_WK_SetDrawEnable( drawWork->clwkExplainButton[i] , TRUE );
        }
      }
    }
  }
  
  if( drawWork->isUpdateTitle == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( drawWork->titleWin )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( drawWork->titleWin );
      GFL_BMPWIN_MakeScreen( drawWork->titleWin );
      GFL_BG_LoadScreenReq(CTVT_FRAME_SUB_MSG);
      drawWork->isUpdateTitle = FALSE;
    }
  }
  
  if( drawWork->isUpdateMsgWin == TRUE )
  {
    PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
    if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( drawWork->msgWin )) == FALSE )
    {
      GFL_BMPWIN_TransVramCharacter( drawWork->msgWin );
      GFL_BMPWIN_MakeScreen( drawWork->msgWin );
      GFL_BG_LoadScreenReq(CTVT_FRAME_MAIN_MSG);
      drawWork->isUpdateMsgWin = FALSE;
    }
  }
  return CTM_DRAW;
}

//--------------------------------------------------------------
//	お絵書き更新
//  キー操作・モード変更周り
//--------------------------------------------------------------
static void CTVT_DRAW_UpdateDraw( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  if( GFL_UI_KEY_GetTrg() & CTVT_BUTTON_DRAW_EDIT )
  {
    drawWork->state = CDS_EDIT;
    drawWork->barScroll = 0;
    CTVT_DRAW_DrawInfoMsg( work , drawWork , TRUE );
    PMSND_PlaySystemSE( CTVT_SND_TOUCH );
  }
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
  {
    drawWork->state = CDS_FADEOUT;
    PMSND_PlaySystemSE( CTVT_SND_CANCEL );
  }
  if( GFL_UI_KEY_GetTrg() & CTVT_BUTTON_PAUSE )
  {
    COMM_TVT_FlipPause( work );
    CTVT_DRAW_UpdateEditButton( work , drawWork );
    PMSND_PlaySystemSE( CTVT_SND_PAUSE );
  }
}

//--------------------------------------------------------------
//	編集更新
//--------------------------------------------------------------
static void CTVT_DRAW_UpdateEdit( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  static const GFL_UI_TP_HITTBL hitTbl[CDED_MAX+1] = 
  {
    {
      CTVT_DRAW_BAR_ICON_Y-12, CTVT_DRAW_BAR_ICON_Y+12,
      CTVT_DRAW_PEN_X-12,      CTVT_DRAW_PEN_X+12,
    },
    {
      CTVT_DRAW_BAR_ICON_Y-12, CTVT_DRAW_BAR_ICON_Y+12,
      CTVT_DRAW_SPOITO_X-12,   CTVT_DRAW_SPOITO_X+12,
    },
    {
      CTVT_DRAW_BAR_ICON_Y-12, CTVT_DRAW_BAR_ICON_Y+12,
      CTVT_DRAW_KESHIGOMU_X-12,CTVT_DRAW_KESHIGOMU_X+12,
    },
    {
      CTVT_DRAW_BAR_ICON_Y-12, CTVT_DRAW_BAR_ICON_Y+12,
      CTVT_DRAW_STAMP_X-12,CTVT_DRAW_STAMP_X+12,
    },
    {
      CTVT_DRAW_BAR_ICON_Y-12, CTVT_DRAW_BAR_ICON_Y+12,
      CTVT_DRAW_PAUSE_X-12,    CTVT_DRAW_PAUSE_X+12,
    },
    {
      CTVT_DRAW_BAR_ICON_Y-12, CTVT_DRAW_BAR_ICON_Y+12,
      CTVT_DRAW_RETURN_X-12,   CTVT_DRAW_RETURN_X+12,
    },
    {GFL_UI_TP_HIT_END,0,0,0}
  };
  
  const int ret = GFL_UI_TP_HitTrg( hitTbl );
  if( ret == CDED_PEN )
  {
    drawWork->editMode = CDED_PEN;
    CTVT_DRAW_UpdateEditButton( work , drawWork );
    drawWork->isTouch = TRUE;
    drawWork->isDispPenSize = TRUE;
    GFL_CLACT_WK_SetDrawEnable( drawWork->clwkPenSize , TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( drawWork->clwkPenSize , TRUE );
    PMSND_PlaySystemSE( CTVT_SND_TOUCH );
    CTVT_TPrintf("PenMode\n");
  }
  if( ret == CDED_SPOITO )
  {
    drawWork->editMode = CDED_SPOITO;
    CTVT_DRAW_UpdateEditButton( work , drawWork );
    drawWork->isTouch = TRUE;
    PMSND_PlaySystemSE( CTVT_SND_TOUCH );
    CTVT_TPrintf("SpoitoMode\n");
  }
  if( ret == CDED_KESHIGOMU )
  {
    drawWork->editMode = CDED_KESHIGOMU;
    CTVT_DRAW_UpdateEditButton( work , drawWork );
    drawWork->isTouch = TRUE;
    PMSND_PlaySystemSE( CTVT_SND_TOUCH );
    CTVT_TPrintf("KeshigomuMode\n");
  }
  if( ret == CDED_STAMP )
  {
    drawWork->editMode = CDED_STAMP;
    CTVT_DRAW_UpdateEditButton( work , drawWork );
    drawWork->isTouch = TRUE;
    drawWork->isDispStampType = TRUE;
    GFL_CLACT_WK_SetDrawEnable( drawWork->clwkStampType , TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( drawWork->clwkStampType , TRUE );
    PMSND_PlaySystemSE( CTVT_SND_TOUCH );
    CTVT_TPrintf("StampMode\n");
  }
  if( ret == CDED_PAUSE )
  {
    COMM_TVT_FlipPause( work );
    CTVT_DRAW_UpdateEditButton( work , drawWork );
    drawWork->isTouch = TRUE;
    PMSND_PlaySystemSE( CTVT_SND_PAUSE );
    CTVT_TPrintf("Pause\n");
  }
  
  //ペンサイズ
  if( drawWork->isDispPenSize == TRUE )
  {
    static const GFL_UI_TP_HITTBL penHitTbl[CDED_MAX+1] = 
    {
      {
        CTVT_DRAW_PEN_SIZE_Y-8, CTVT_DRAW_PEN_SIZE_Y+8,
        CTVT_DRAW_PEN_SIZE_X-8,     CTVT_DRAW_PEN_SIZE_X+8,
      },
      {
        CTVT_DRAW_PEN_SIZE_Y-8, CTVT_DRAW_PEN_SIZE_Y+8,
        CTVT_DRAW_PEN_SIZE_X-8+16,     CTVT_DRAW_PEN_SIZE_X+8+16,
      },
      {
        CTVT_DRAW_PEN_SIZE_Y-8, CTVT_DRAW_PEN_SIZE_Y+8,
        CTVT_DRAW_PEN_SIZE_X-8+32,     CTVT_DRAW_PEN_SIZE_X+8+32,
      },
      {GFL_UI_TP_HIT_END,0,0,0}
    };
    static const penSizeArr[3] = {DSPS_CIRCLE_8,DSPS_CIRCLE_4,DSPS_CIRCLE_1};
    const int retPen = GFL_UI_TP_HitTrg( penHitTbl );
    if( retPen != GFL_UI_TP_HIT_NONE )
    {
      drawWork->penSize = penSizeArr[retPen];
      drawWork->isTouch = TRUE;
  
      CTVT_DRAW_EreseSubWindow( work , drawWork );
      PMSND_PlaySystemSE( CTVT_SND_TOUCH );
    }
  }
  
  //スタンプタイプ
  if( drawWork->isDispStampType == TRUE )
  {
    static const GFL_UI_TP_HITTBL penHitTbl[CDED_MAX+1] = 
    {
      {
        CTVT_DRAW_STAMP_TYPE_Y-8, CTVT_DRAW_STAMP_TYPE_Y+8,
        CTVT_DRAW_STAMP_TYPE_X-8-32,     CTVT_DRAW_STAMP_TYPE_X+8-32,
      },
      {
        CTVT_DRAW_STAMP_TYPE_Y-8, CTVT_DRAW_STAMP_TYPE_Y+8,
        CTVT_DRAW_STAMP_TYPE_X-8-16,     CTVT_DRAW_STAMP_TYPE_X+8-16,
      },
      {
        CTVT_DRAW_STAMP_TYPE_Y-8, CTVT_DRAW_STAMP_TYPE_Y+8,
        CTVT_DRAW_STAMP_TYPE_X-8,     CTVT_DRAW_STAMP_TYPE_X+8,
      },
      {
        CTVT_DRAW_STAMP_TYPE_Y-8, CTVT_DRAW_STAMP_TYPE_Y+8,
        CTVT_DRAW_STAMP_TYPE_X-8+16,     CTVT_DRAW_STAMP_TYPE_X+8+16,
      },
      {
        CTVT_DRAW_STAMP_TYPE_Y-8, CTVT_DRAW_STAMP_TYPE_Y+8,
        CTVT_DRAW_STAMP_TYPE_X-8+32,     CTVT_DRAW_STAMP_TYPE_X+8+32,
      },
      {GFL_UI_TP_HIT_END,0,0,0}
    };
    static const stampTypeArr[5] = {DSPS_STAMP_HEART,
                                    DSPS_STAMP_EXUS,
                                    DSPS_STAMP_QUES,
                                    DSPS_STAMP_STAR,
                                    DSPS_STAMP_DROP};
    const int retPen = GFL_UI_TP_HitTrg( penHitTbl );
    if( retPen != GFL_UI_TP_HIT_NONE )
    {
      drawWork->stampType = stampTypeArr[retPen];
      drawWork->isTouch = TRUE;

      drawWork->isDispStampType = FALSE;
      GFL_CLACT_WK_SetDrawEnable( drawWork->clwkStampType , FALSE );
      PMSND_PlaySystemSE( CTVT_SND_TOUCH );
    }
  }
  
  if( GFL_UI_KEY_GetTrg() & CTVT_BUTTON_DRAW_EDIT ||
      GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ||
      ret == CDED_RETURN )
  {
    drawWork->state = CDS_DRAW;
    drawWork->barScroll = 24;
    drawWork->isTouch = TRUE;
    CTVT_DRAW_DrawInfoMsg( work , drawWork , FALSE );
    CTVT_DRAW_EreseSubWindow( work , drawWork );
    PMSND_PlaySystemSE( CTVT_SND_CANCEL );
  }
  if( GFL_UI_KEY_GetTrg() & CTVT_BUTTON_PAUSE )
  {
    COMM_TVT_FlipPause( work );
    CTVT_DRAW_UpdateEditButton( work , drawWork );
    PMSND_PlaySystemSE( CTVT_SND_PAUSE );
  }
}

//--------------------------------------------------------------
//	落書き更新
//--------------------------------------------------------------
static void CTVT_DRAW_UpdateDrawing( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  u32 tpx,tpy;
  GFL_UI_TP_GetPointTrg( &tpx,&tpy );
  if( COMM_TVT_GetSusspendDraw( work ) == TRUE )
  {
    return;
  }
  
  if( GFL_UI_TP_GetTrg() == TRUE &&
      drawWork->isTouch == FALSE &&
      drawWork->state == CDS_EDIT &&
      tpy >= 192-24 )
  {
    if( drawWork->editMode == CDED_SPOITO )
    {
      drawWork->editMode = CDED_PEN;
      CTVT_DRAW_UpdateEditButton( work , drawWork );
    }
    CTVT_DRAW_EreseSubWindow( work , drawWork );
    return;
  }
  if( drawWork->state == CDS_EDIT &&
      GFL_UI_TP_GetTrg() == TRUE &&
      drawWork->isTouch == FALSE &&
      drawWork->editMode == CDED_STAMP )
  {
    BOOL isFull;
    const u8 connectNum = COMM_TVT_GetConnectNum( work );
    CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
    DRAW_SYS_WORK *drawSys = COMM_TVT_GetDrawSys( work );
    DRAW_SYS_PEN_INFO *info = CTVT_COMM_GetDrawBuf(work,commWork,&isFull);

    info->startX = tpx;
    info->startY = tpy;
    info->endX = tpx;
    info->endY = tpy;
    info->penType = drawWork->stampType;
    info->col  = drawWork->penCol;
    
    if( connectNum == 1 )
    {
      DRAW_SYS_SetPenInfo( drawSys , info );
    }
    else
    {
      CTVT_COMM_AddDrawBuf( work , commWork );
    }
  }
  
  if( GFL_UI_TP_GetTrg() == TRUE &&
      drawWork->isTouch == FALSE )
  {
    if( drawWork->editMode == CDED_SPOITO )
    {
      u16* spoitColAdr = (u16*)(HW_OBJ_PLTT + 0*32 + 2*0xd);
      //BG2と3のアドレス
      u16* adr2 = (u16*)((u32)G2_GetBG2ScrPtr() + tpx*2 + tpy*256*2);
      u16* adr3 = (u16*)((u32)G2_GetBG3ScrPtr() + tpx*2 + tpy*256*2);

      if( COMM_TVT_IsDoubleMode(work) == TRUE )
      {
        //拡大時はアドレスがずれる
        adr3 = (u16*)((u32)G2_GetBG3ScrPtr() + (tpx/2)*2 + (tpy/2)*256*2);
      }
      else
      {
        adr3 = (u16*)((u32)G2_GetBG3ScrPtr() + tpx*2 + tpy*256*2);
      }
      if( *adr2 & 0x8000 )
      {
        drawWork->penCol = *adr2;
      }
      else
      if( *adr3 & 0x8000 )
      {
        drawWork->penCol = *adr3;
      }
      else
      {
        //強制黒
        drawWork->penCol = 0x8000;
        
      }
      *spoitColAdr = (drawWork->penCol&0x7FFF);
      
      drawWork->editMode = CDED_PEN;
      CTVT_DRAW_UpdateEditButton( work , drawWork );
      CTVT_TPrintf("Spoit!![%x]\n",drawWork->penCol-0x8000);
      PMSND_PlaySystemSE( CTVT_SND_SUPOITO );
    }
    else
    {
      //drawWork->befPenX = tpx;
      //drawWork->befPenY = tpy;
      drawWork->isHold = TRUE;
      drawWork->befPenX = 0xFFFF;
      drawWork->befPenY = 0xFFFF;
      CTVT_DRAW_EreseSubWindow( work , drawWork );
    }
  }
  
  //落書き部分
  if( drawWork->editMode == CDED_PEN ||
      drawWork->editMode == CDED_KESHIGOMU )
  {
    if( drawWork->samplingRes == TP_OK )
    {
      u8 i;
      if( drawWork->tpData.Size > 0 )
      {
        for(i=0 ; i<drawWork->tpData.Size ; i++ )
        {
          if( drawWork->tpData.TPDataTbl[i].validity ==  TP_VALIDITY_VALID &&
              drawWork->tpData.TPDataTbl[i].touch == TRUE )
          {
            if( drawWork->isHold == TRUE &&
                drawWork->befPenX < 0xFFFF &&
                drawWork->befPenY < 0xFFFF )
            {
              BOOL isFull;
              const u8 connectNum = COMM_TVT_GetConnectNum( work );
              CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
              DRAW_SYS_WORK *drawSys = COMM_TVT_GetDrawSys( work );
              DRAW_SYS_PEN_INFO *info = CTVT_COMM_GetDrawBuf(work,commWork,&isFull);
              if( isFull == FALSE ||
                  connectNum == 1 )
              {
                info->startX = drawWork->befPenX;
                info->startY = drawWork->befPenY;
              }
              if( drawWork->befPenX != drawWork->tpData.TPDataTbl[i].x ||
                  drawWork->befPenY != drawWork->tpData.TPDataTbl[i].y )
              {
                info->endX = drawWork->tpData.TPDataTbl[i].x;
                info->endY = drawWork->tpData.TPDataTbl[i].y;
                info->penType = drawWork->penSize;
                if( drawWork->editMode == CDED_KESHIGOMU )
                {
                  info->col = 0;
                }
                else
                {
                  info->col  = drawWork->penCol;
                }
                
                if( connectNum == 1 )
                {
                  DRAW_SYS_SetPenInfo( drawSys , info );
                }
                else
                {
                  CTVT_COMM_AddDrawBuf( work , commWork );
                }
              }
            }
            drawWork->befPenX = drawWork->tpData.TPDataTbl[i].x;
            drawWork->befPenY = drawWork->tpData.TPDataTbl[i].y;
          }
          else
          {
          }
        }
      }
    }
  }
  if( GFL_UI_TP_GetCont() == FALSE )
  {
    drawWork->isHold = FALSE;
  }

}

//--------------------------------------------------------------
//	バー位置更新
//--------------------------------------------------------------
static void CTVT_DRAW_UpdateBar( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  BOOL isUpdate = FALSE;
  if( drawWork->barScroll < drawWork->dispBarScroll )
  {
    drawWork->dispBarScroll -= 2;
    isUpdate = TRUE;
  }
  else
  if( drawWork->barScroll > drawWork->dispBarScroll )
  {
    drawWork->dispBarScroll += 2;
    isUpdate = TRUE;
  }
  
  if( isUpdate == TRUE )
  {
    u8 i;
    GFL_BG_SetScrollReq( CTVT_FRAME_MAIN_MSG , GFL_BG_SCROLL_Y_SET , -drawWork->dispBarScroll );
    for( i=0;i<CDED_MAX;i++ )
    {
      GFL_CLACTPOS pos;
      GFL_CLACT_WK_GetPos( drawWork->clwkEditButton[i] , &pos , CLSYS_DRAW_MAIN );
      pos.y = CTVT_DRAW_BAR_ICON_Y+drawWork->dispBarScroll;
      if( i == CDED_RETURN )
      {
        pos.y -= 12;
      }
      GFL_CLACT_WK_SetPos( drawWork->clwkEditButton[i] , &pos , CLSYS_DRAW_MAIN );
    }
  }
}


//--------------------------------------------------------------
//	ボタングラフィック更新
//--------------------------------------------------------------
static void CTVT_DRAW_UpdateEditButton( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  {
    static const u8 anmIdx[CDED_MAX][2] = 
    {
      {CTOAM_PEN,CTOAM_PEN_ACTIVE},
      {CTOAM_SUPOITO,CTOAM_SUPOITO_ACTIVE},
      {CTOAM_KESHIGOMU,CTOAM_KESHIGOMU_ACTIVE},
    };
    
    GFL_CLACT_WK_SetAnmSeq( drawWork->clwkEditButton[CDED_PEN] , 
                            anmIdx[CDED_PEN][(drawWork->editMode==CDED_PEN?1:0)] );
    GFL_CLACT_WK_SetAnmSeq( drawWork->clwkEditButton[CDED_SPOITO] , 
                            anmIdx[CDED_SPOITO][(drawWork->editMode==CDED_SPOITO?1:0)] );
    GFL_CLACT_WK_SetAnmSeq( drawWork->clwkEditButton[CDED_KESHIGOMU] , 
                            anmIdx[CDED_KESHIGOMU][(drawWork->editMode==CDED_KESHIGOMU?1:0)] );
  }
  
  if( COMM_TVT_GetPause( work ) == TRUE )
  {
    GFL_CLACT_WK_SetAnmSeq( drawWork->clwkEditButton[CDED_PAUSE] , CTOAM_PAUSE );
  }
  else
  {
    GFL_CLACT_WK_SetAnmSeq( drawWork->clwkEditButton[CDED_PAUSE] , CTOAM_PLAY );
  }
  
  if( drawWork->editMode != CDED_PEN &&
      drawWork->isDispPenSize == TRUE ) 
  {
    drawWork->isDispPenSize = FALSE;
    GFL_CLACT_WK_SetDrawEnable( drawWork->clwkPenSize , FALSE );
    GFL_CLACT_WK_ResetAnm( drawWork->clwkPenSize );
    GFL_CLACT_WK_SetAnmFrame( drawWork->clwkPenSize , 0 );
    GFL_CLACT_WK_SetAutoAnmFlag( drawWork->clwkPenSize , FALSE );
  }
  
  if( drawWork->editMode != CDED_STAMP &&
      drawWork->isDispStampType == TRUE ) 
  {
    drawWork->isDispStampType = FALSE;
    GFL_CLACT_WK_SetDrawEnable( drawWork->clwkStampType , FALSE );
    GFL_CLACT_WK_ResetAnm( drawWork->clwkStampType );
    GFL_CLACT_WK_SetAnmFrame( drawWork->clwkStampType , 0 );
    GFL_CLACT_WK_SetAutoAnmFlag( drawWork->clwkStampType , FALSE );
  }

}

//--------------------------------------------------------------
//	説明メッセージ更新
//--------------------------------------------------------------
static void CTVT_DRAW_DrawInfoMsg( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork , const BOOL isEditMode )
{
  GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
  GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
  PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );

  if( drawWork->infoWin != NULL )
  {
    PRINTSYS_QUE_Clear( printQue );
    BmpWinFrame_Clear( drawWork->infoWin , WINDOW_TRANS_ON );
    GFL_BMPWIN_ClearTransWindow( drawWork->infoWin );
    GFL_BMPWIN_Delete( drawWork->infoWin );
  }

  drawWork->isUpdateInfoEdit = isEditMode;
  if( isEditMode == TRUE )
  {
    u8 i;
    STRBUF *str;

    drawWork->infoWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MSG , 
                                      2 ,  8 , 28 , 13 ,
                                      CTVT_PAL_BG_SUB_FONT ,
                                      GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 0xF );
    
    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_007 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_EDIT_X , 4 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    
    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_008 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_EDIT_X , 24 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    
    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_009 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_EDIT_X , 44 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    
    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_011 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_EDIT_X , 64 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    
    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_010 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_EDIT_X , 84 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    
  }
  else
  {
    u8 i;

    STRBUF *str;
    
    drawWork->infoWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MSG , 
                                      2 , 16 , 28 ,  6 ,
                                      CTVT_PAL_BG_SUB_FONT ,
                                      GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 0xF );

    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_001 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_DRAW_X1 , 0 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );
    
    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_002 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_DRAW_X1 , 16 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );

    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_003 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_DRAW_X1 , 32 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );

    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_004 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_DRAW_X2 , 0 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );

    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_005 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_DRAW_X2 , 16 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );

    str = GFL_MSG_CreateString( msgHandle , COMM_TVT_DRAW_006 );
    PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->infoWin ) , 
            CTVT_DRAW_INFO_DRAW_X2 , 32 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
    GFL_STR_DeleteBuffer( str );

    for( i=0;i<CDEX_MAX;i++ )
    {
      GFL_CLACT_WK_SetDrawEnable( drawWork->clwkExplainButton[i] , FALSE );
    }
  }
  BmpWinFrame_Write( drawWork->infoWin , WINDOW_TRANS_OFF , 
                      CTVT_BMPWIN_CGX , CTVT_PAL_BG_SUB_WINFRAME );
  drawWork->isUpdateInfo = TRUE;
}


//--------------------------------------------------------------
//	強制終了チェック
//--------------------------------------------------------------
static const BOOL CTVT_DRAW_CheckFinishReq( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  if( COMM_TVT_GetFinishReq( work ) == TRUE )
  {
    CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
    if( CTVT_COMM_GetMode( work , commWork ) == CCIM_CONNECTED )
    {
      //Wifiのリクエストによる終了
      drawWork->state = CDS_END_WIFI_REQ_INIT_DISP;
    }
    else
    {
      //親のリクエストによる終了
      drawWork->state = CDS_END_PARENT_REQ_INIT;
    }
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//	Msg表示
//--------------------------------------------------------------
static void CTVT_DRAW_DispMessage( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork , const u16 msgId )
{
  GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
  GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
  PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
  STRBUF *str;
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( drawWork->msgWin) , 0xF );
  str = GFL_MSG_CreateString( msgHandle , msgId );
  PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( drawWork->msgWin ) , 
          0 , 0 , str , fontHandle ,CTVT_FONT_COLOR_BLACK );
  GFL_STR_DeleteBuffer( str );

  BmpWinFrame_Write( drawWork->msgWin , WINDOW_TRANS_OFF , 
                      CTVT_BMPWIN_CGX , CTVT_PAL_BG_MAIN_WINFRAME );
  drawWork->isUpdateMsgWin = TRUE;

}

//--------------------------------------------------------------
//  ペンサイズ、スタンプタイプ非表示
//--------------------------------------------------------------
static void CTVT_DRAW_EreseSubWindow( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork )
{
  //UpdateEditButtonにも単独で消しているところがあるので注意
  drawWork->isDispPenSize = FALSE;
  drawWork->isDispStampType = FALSE;
  GFL_CLACT_WK_ResetAnm( drawWork->clwkPenSize );
  GFL_CLACT_WK_ResetAnm( drawWork->clwkStampType );
  GFL_CLACT_WK_SetAnmFrame( drawWork->clwkPenSize , 0 );
  GFL_CLACT_WK_SetAnmFrame( drawWork->clwkStampType , 0 );
  GFL_CLACT_WK_SetAutoAnmFlag( drawWork->clwkPenSize , FALSE );
  GFL_CLACT_WK_SetAutoAnmFlag( drawWork->clwkStampType , FALSE );
  GFL_CLACT_WK_SetDrawEnable( drawWork->clwkPenSize , FALSE );
  GFL_CLACT_WK_SetDrawEnable( drawWork->clwkStampType , FALSE );
}
