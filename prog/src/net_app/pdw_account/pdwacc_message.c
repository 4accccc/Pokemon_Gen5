//============================================================================================
/**
 * @file    pdwacc_message.c
 * @bfief   GAMESYNCメッセージ表示関連
 * @author  k.ohno
 * @date    2009.11.29
 */
//============================================================================================

#include <gflib.h>
#include <dwc.h>

#include "arc_def.h"
#include "net_app/pdwacc.h"

#include "infowin/infowin.h"
#include "system/main.h"
#include "system/wipe.h"
#include "gamesystem/msgspeed.h" // MSGSPEED_GetWait
#include "savedata/config.h"  // WIRELESSSAVE_ON

#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"

#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"
#include "../../field/event_ircbattle.h"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK

#include "pdwacc_local.h"
#include "gtsnego.naix"
#include "msg/msg_pdwacc.h"

#if PM_DEBUG
#include "debug/debugwin_sys.h"
#include "test/debug_pause.h"
#endif

//--------------------------------------------
// 画面構成定義
//--------------------------------------------
#define _WINDOW_MAXNUM (4)   //ウインドウのパターン数
#define _MESSAGE_BUF_NUM  ( 200*2 )
#define _SUBMENU_LISTMAX (2)

#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (3)    // ウインドウ高さ


//-------------------------------------------------------------------------
/// 文字表示色定義(default) -> gflib/fntsys.hへ移動
//------------------------------------------------------------------

#define WINCLR_COL(col) (((col)<<4)|(col))
//------------------------------------------------------------------
//  カラー関連ダミー定義
//------------------------------------------------------------------
// 通常のフォントカラー
#define FBMP_COL_NULL   (0)
#define FBMP_COL_BLACK    (1)
#define FBMP_COL_BLK_SDW  (2)
#define FBMP_COL_RED    (3)
#define FBMP_COL_RED_SDW  (4)
#define FBMP_COL_GREEN    (5)
#define FBMP_COL_GRN_SDW  (6)
#define FBMP_COL_BLUE   (7)
#define FBMP_COL_BLU_SDW  (8)
#define FBMP_COL_PINK   (9)
#define FBMP_COL_PNK_SDW  (10)
#define FBMP_COL_WHITE    (15)


#define _MESSAGE_NO_FRAME  (GFL_BG_FRAME1_M)
#define _MESSAGE_INFO_FRAME (GFL_BG_FRAME1_S)
#define _MESSAGE_SYSTEM_FRAME (GFL_BG_FRAME2_S)


typedef enum{
  _MESSAGE_LEVEL,
  _MESSAGE_MY,
  _MESSAGE_FRIEND,
  _MESSAGE_INDEX_MAX
} _MESSAGE_INDEX;

typedef struct {
  int leftx;
  int lefty;
  int width;
  int height;
} _WINDOWPOS;

#define _BMP_WINDOW_NUM  (12)



struct _PDWACC_MESSAGE_WORK {
  u32 bgcharNo;
  u32 bgchar2S;  //SystemMsg
  u32 bgcharInfoMessage; //info

  GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
  GFL_BUTTON_MAN* pButton;

  GFL_MSGDATA *pMsgData;  //

  WORDSET *pWordSet;                // メッセージ展開用ワークマネージャー
  GFL_FONT* pFontHandle;
  STRBUF* pStrBuf;
  int msgidx[_MESSAGE_INDEX_MAX];

  GFL_BMPWIN* noDispWin;
  GFL_BMPWIN* noTitleDispWin;
  GFL_BMPWIN* infoDispWin;
  GFL_BMPWIN* systemDispWin;
  GFL_BMPWIN* mainDispWin[_BMP_WINDOW_NUM];

  PRINT_STREAM* pStream;
  PRINT_STREAM* pStreamNo;
  GFL_TCBLSYS *pMsgTcblSys;
  PRINT_QUE*            SysMsgQue;

  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
  APP_TASKMENU_RES* pAppTaskRes;
  //  int windowNum;
  HEAPID heapID;

};


static void _ButtonSafeDelete(PDWACC_MESSAGE_WORK* pWork)
{
  if(pWork->pButton){
    GFL_BMN_Delete(pWork->pButton);
    pWork->pButton = NULL;
  }
}


PDWACC_MESSAGE_WORK* PDWACC_MESSAGE_Init(HEAPID id,int msg_dat)
{
  PDWACC_MESSAGE_WORK* pWork = GFL_HEAP_AllocClearMemory(id, sizeof(PDWACC_MESSAGE_WORK));
  pWork->heapID = id;

  GFL_BMPWIN_Init(pWork->heapID);
  GFL_FONTSYS_Init();
  pWork->pMsgTcblSys = GFL_TCBL_Init( pWork->heapID , pWork->heapID , 1 , 0 );
  pWork->SysMsgQue = PRINTSYS_QUE_Create( pWork->heapID );
  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msg_dat, pWork->heapID );

  pWork->pAppTaskRes =
    APP_TASKMENU_RES_Create( GFL_BG_FRAME1_S, _SUBLIST_NORMAL_PAL,
                             pWork->pFontHandle, pWork->SysMsgQue, pWork->heapID  );

  pWork->bgcharNo = BmpWinFrame_GraphicSetAreaMan(_MESSAGE_NO_FRAME, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  pWork->bgcharInfoMessage = BmpWinFrame_GraphicSetAreaMan(_MESSAGE_INFO_FRAME, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  pWork->bgchar2S = BmpWinFrame_GraphicSetAreaMan(_MESSAGE_SYSTEM_FRAME, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);

#if PM_DEBUG
  DEBUGWIN_InitProc( GFL_BG_FRAME3_M , pWork->pFontHandle );
  DEBUG_PAUSE_SetEnable( TRUE );
#endif
  return pWork;
}

void PDWACC_MESSAGE_Main(PDWACC_MESSAGE_WORK* pWork)
{
  GFL_TCBL_Main( pWork->pMsgTcblSys );
  PRINTSYS_QUE_Main(pWork->SysMsgQue);

}

void PDWACC_MESSAGE_End(PDWACC_MESSAGE_WORK* pWork)
{
  int i;

  GFL_BG_FreeCharacterArea(_MESSAGE_NO_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgcharNo),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgcharNo));
  GFL_BG_FreeCharacterArea(_MESSAGE_INFO_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgcharInfoMessage),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgcharInfoMessage));
  GFL_BG_FreeCharacterArea(_MESSAGE_SYSTEM_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2S),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar2S));
  _ButtonSafeDelete(pWork);

  //  PDWACC_MESSAGE_ButtonWindowDelete(pWork);

  GFL_FONTSYS_SetDefaultColor();
  GFL_MSG_Delete( pWork->pMsgData );
  GFL_FONT_Delete(pWork->pFontHandle);
  GFL_STR_DeleteBuffer(pWork->pStrBuf);

  APP_TASKMENU_RES_Delete( pWork->pAppTaskRes );
  GFL_TCBL_Exit(pWork->pMsgTcblSys);
  PRINTSYS_QUE_Clear(pWork->SysMsgQue);
  PRINTSYS_QUE_Delete(pWork->SysMsgQue);
  if(pWork->infoDispWin){
    GFL_BMPWIN_Delete(pWork->infoDispWin);
  }
  if(pWork->systemDispWin){
    GFL_BMPWIN_Delete(pWork->systemDispWin);
  }
  for(i=0;i< _BMP_WINDOW_NUM;i++){
    if(pWork->mainDispWin[i]){
      GFL_BMPWIN_Delete(pWork->mainDispWin[i]);
    }
  }

  GFL_BMPWIN_Exit();
  GFL_HEAP_FreeMemory(pWork);
}



//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void PDWACC_MESSAGE_InfoMessageDisp(PDWACC_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );

  if(pWork->infoDispWin==NULL){
    pWork->infoDispWin = GFL_BMPWIN_Create(
      _MESSAGE_INFO_FRAME ,
      1 , 3, 30 ,4 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->infoDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pStrBuf, pWork->pFontHandle,
                                        MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);

  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgcharInfoMessage), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(_MESSAGE_INFO_FRAME);
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了待ち
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL PDWACC_MESSAGE_InfoMessageEndCheck(PDWACC_MESSAGE_WORK* pWork)
{
  if(pWork->pStream){
    int state = PRINTSYS_PrintStreamGetState( pWork->pStream );
    switch(state){
    case PRINTSTREAM_STATE_DONE:
      PRINTSYS_PrintStreamDelete( pWork->pStream );
      pWork->pStream = NULL;
      break;
    case PRINTSTREAM_STATE_PAUSE:
      if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE){
        PRINTSYS_PrintStreamReleasePause( pWork->pStream );
      }
      break;
    default:
      break;
    }
    return FALSE;  //まだ終わってない
  }
  return TRUE;// 終わっている
}

//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ消去
 * @retval  none
 */
//------------------------------------------------------------------------------

void PDWACC_MESSAGE_InfoMessageEnd(PDWACC_MESSAGE_WORK* pWork)
{
  if(pWork->infoDispWin){
    BmpWinFrame_Clear(pWork->infoDispWin, WINDOW_TRANS_OFF);
    GFL_BMPWIN_ClearScreen(pWork->infoDispWin);
    GFL_BG_LoadScreenV_Req(_MESSAGE_INFO_FRAME);
    GFL_BMPWIN_Delete(pWork->infoDispWin);
    pWork->infoDispWin=NULL;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   はいいいえウインドウ
 * @retval  none
 */
//------------------------------------------------------------------------------



APP_TASKMENU_WORK* PDWACC_MESSAGE_YesNoStart(PDWACC_MESSAGE_WORK* pWork,int type)
{
  int i;
  APP_TASKMENU_INITWORK appinit;
  APP_TASKMENU_WORK* pAppTask;

  appinit.heapId = pWork->heapID;
  appinit.itemNum =  2;
  appinit.itemWork =  &pWork->appitem[0];

  switch(type){
  case PDWACC_YESNOTYPE_INFO:
    appinit.charPosX = 32;
    appinit.charPosY = 14;
    appinit.posType = ATPT_RIGHT_DOWN;
    break;
  case PDWACC_YESNOTYPE_SYS:
    appinit.charPosX = 32;
    appinit.charPosY = 24;
    appinit.posType = ATPT_RIGHT_DOWN;
    break;
  }
  appinit.w        = APP_TASKMENU_PLATE_WIDTH;
  appinit.h        = APP_TASKMENU_PLATE_HEIGHT;

  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, PDWACC_005, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pWork->appitem[1].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, PDWACC_006, pWork->appitem[1].str);
  pWork->appitem[1].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pAppTask = APP_TASKMENU_OpenMenu(&appinit,pWork->pAppTaskRes);
  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
  GFL_STR_DeleteBuffer(pWork->appitem[1].str);
  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , -8 );
  return pAppTask;
}


//----------------------------------------------------------------------------
/**
 *  @brief  ボタンイベント実行関数
 *
 *  @param  bttnid    ボタンID
 *  @param  event   イベント種類
 *  @param  p_work    ワーク
 */
//-----------------------------------------------------------------------------

void PDWACC_MESSAGE_ButtonWindowMain(PDWACC_MESSAGE_WORK* pWork)
{
  GFL_BMN_Main( pWork->pButton );
}

//------------------------------------------------------------------------------
/**
 * @brief   システムウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void PDWACC_MESSAGE_SystemMessageDisp(PDWACC_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );

  if(pWork->systemDispWin==NULL){
    pWork->systemDispWin = GFL_BMPWIN_Create(
      _MESSAGE_SYSTEM_FRAME , 1 , 3, 30 , 16 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->systemDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,0,0, pWork->pStrBuf, pWork->pFontHandle );
  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2S), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(_MESSAGE_SYSTEM_FRAME);
}


//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ消去
 * @retval  none
 */
//------------------------------------------------------------------------------

void PDWACC_MESSAGE_SystemMessageEnd(PDWACC_MESSAGE_WORK* pWork)
{
  BmpWinFrame_Clear(pWork->systemDispWin, WINDOW_TRANS_OFF);
  GFL_BMPWIN_ClearScreen(pWork->systemDispWin);
  GFL_BG_LoadScreenV_Req(_MESSAGE_SYSTEM_FRAME);
}


//------------------------------------------------------------------------------
/**
 * @brief   システムウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void PDWACC_MESSAGE_ErrorMessageDisp(PDWACC_MESSAGE_WORK* pWork,int msgid,int no)
{
  WORDSET_RegisterNumber(pWork->pWordSet, 0, no,
                         5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  PDWACC_MESSAGE_SystemMessageDisp(pWork, msgid);
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void PDWACC_MESSAGE_DispInit(PDWACC_MESSAGE_WORK* pWork)
{
  GFL_BMPWIN* pwin;
  int i;

  for(i=0;i<_BMP_WINDOW_NUM;i++){
    if(pWork->mainDispWin[i]!=NULL){
      GFL_BMPWIN_Delete(pWork->mainDispWin[i]);
      pWork->mainDispWin[i]=NULL;
    }
  }
  GFL_FONTSYS_SetColor(1, 2, 0);
}

static int PDWACC_MESSAGE_Disp(PDWACC_MESSAGE_WORK* pWork,int msgid,int x,int y)
{
  GFL_BMPWIN* pwin;
  int i;

  for(i=0;i<_BMP_WINDOW_NUM;i++){
    if(pWork->mainDispWin[i]==NULL){
      pWork->mainDispWin[i] = GFL_BMPWIN_Create(
        _MESSAGE_SYSTEM_FRAME , x , y, 20 , 2 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );

      pwin = pWork->mainDispWin[i];

      GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);

      GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );
      PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,1, 2, pWork->pStrBuf, pWork->pFontHandle );
      GFL_BMPWIN_TransVramCharacter(pwin);
      GFL_BMPWIN_MakeScreen(pwin);
      return i;
    }
  }
  return 0;
}


static void PDWACC_MESSAGE_DispMsgChange(PDWACC_MESSAGE_WORK* pWork,int msgid,int idx)
{
  GFL_BMPWIN* pwin;
  int i=idx;

  if(pWork->mainDispWin[i]!=NULL){
    pwin = pWork->mainDispWin[i];
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);
    GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );
    PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,1, 2, pWork->pStrBuf, pWork->pFontHandle );
    GFL_BMPWIN_TransVramCharacter(pwin);
  }
}


static void PDWACC_MESSAGE_DispTransReq(PDWACC_MESSAGE_WORK* pWork)
{

  GFL_BG_LoadScreenV_Req(_MESSAGE_SYSTEM_FRAME);
}

//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ消去
 * @retval  none
 */
//------------------------------------------------------------------------------

void PDWACC_MESSAGE_DispClear(PDWACC_MESSAGE_WORK* pWork)
{
  int i;

  for(i=0;i<_BMP_WINDOW_NUM;i++){
    if(pWork->mainDispWin[i]!=NULL){
      BmpWinFrame_Clear(pWork->mainDispWin[i], WINDOW_TRANS_OFF);
      GFL_BMPWIN_ClearScreen(pWork->mainDispWin[i]);
      GFL_BG_LoadScreenV_Req(_MESSAGE_SYSTEM_FRAME);
    }
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   アクセスコード表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void PDWACC_MESSAGE_NoMessageDisp(PDWACC_MESSAGE_WORK* pWork,u64 code)
{
  GFL_BMPWIN* pwin;
  int i;
  //  STRCODE buff[]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46};
  // STRCODE buff2[]="ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
  STRCODE buff2[]={
    65,	//A
    66,	//B
    67,	//C
    68,	//D
    69,	//E
    70,	//F
    71,	//G
    72,	//H
    //73,	//I
    74,	//J
    75,	//K
    76,	//L
    77,	//M
    78,	//N
    //79,	//O
    80,	//P
    81,	//Q
    82,	//R
    83,	//S
    84,	//T
    85,	//U
    86,	//V
    87,	//W
    88,	//X
    89,	//Y
    90,	//Z
    //48,	//0
    //49,	//1
    50,	//2
    51,	//3
    52,	//4
    53,	//5
    54,	//6
    55,	//7
    56,	//8
    57,	//9
  };

  u64 moji=code;
  u16 word;

  STRCODE disp[13*sizeof(STRCODE)];

  GFL_STD_MemFill(disp,0xff,sizeof(disp));
  for(i = 0; i < 10 ; i++){
    word = moji & 0x1f;
    moji = moji >> 5;
    disp[i]=buff2[word];
  }
  GFL_STR_SetStringCode(pWork->pStrBuf,disp);

  //GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );

  if(pWork->noDispWin==NULL){
    pWork->noDispWin = GFL_BMPWIN_Create(
      _MESSAGE_NO_FRAME ,
      1 , 8, 30 ,4 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->noDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,0,0, pWork->pStrBuf, pWork->pFontHandle);

  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgcharNo), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);


  if(pWork->noTitleDispWin==NULL){
    pWork->noTitleDispWin = GFL_BMPWIN_Create(
      _MESSAGE_NO_FRAME ,
      3 , 5, 26 , 2,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->noTitleDispWin;
  GFL_FONTSYS_SetColor(15, 14, 0);
  GFL_MSG_GetString( pWork->pMsgData, PDWACC_009, pWork->pStrBuf );
  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,0,0, pWork->pStrBuf, pWork->pFontHandle);
  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);


  GFL_BG_LoadScreenV_Req(_MESSAGE_INFO_FRAME);
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了待ち
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL PDWACC_MESSAGE_NoMessageEndCheck(PDWACC_MESSAGE_WORK* pWork)
{
  return TRUE;// 終わっている
}

//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ消去
 * @retval  none
 */
//------------------------------------------------------------------------------

void PDWACC_MESSAGE_NoMessageEnd(PDWACC_MESSAGE_WORK* pWork)
{
  if(pWork->noDispWin){
    GFL_BMPWIN_Delete(pWork->noDispWin);
    GFL_BMPWIN_Delete(pWork->noTitleDispWin);
  }
}

