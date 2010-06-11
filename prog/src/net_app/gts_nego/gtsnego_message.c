//============================================================================================
/**
 * @file    gtsnego_message.c
 * @bfief   GTSネゴシエーションメッセージ表示関連
 * @author  k.ohno
 * @date    2009.11.14
 */
//============================================================================================

#include <gflib.h>
#include <dwc.h>

#include "arc_def.h"
#include "net_app/gsync.h"

#include "debug/debugwin_sys.h"
#include "test/debug_pause.h"

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

#include "gtsnego_local.h"
#include "gtsnego.naix"
#include "msg/msg_gtsnego.h"
#include "system/pms_draw.h"



static void _PMSDrawExit( GTSNEGO_MESSAGE_WORK* pWork );

//--------------------------------------------
// 画面構成定義
//--------------------------------------------
#define _WINDOW_MAXNUM (4)   //ウインドウのパターン数
#define _MESSAGE_BUF_NUM	( 240 )
#define _SUBMENU_LISTMAX (2)

#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (3)    // ウインドウ高さ


#define _NUM_FONT_PALETTE (4)

//-------------------------------------------------------------------------
///	文字表示色定義(default)	-> gflib/fntsys.hへ移動
//------------------------------------------------------------------

#define WINCLR_COL(col)	(((col)<<4)|(col))
//------------------------------------------------------------------
//	カラー関連ダミー定義
//------------------------------------------------------------------
// 通常のフォントカラー
#define	FBMP_COL_NULL		(0)
#define	FBMP_COL_BLACK		(1)
#define	FBMP_COL_BLK_SDW	(2)
#define	FBMP_COL_RED		(3)
#define	FBMP_COL_RED_SDW	(4)
#define	FBMP_COL_GREEN		(5)
#define	FBMP_COL_GRN_SDW	(6)
#define	FBMP_COL_BLUE		(7)
#define	FBMP_COL_BLU_SDW	(8)
#define	FBMP_COL_PINK		(9)
#define	FBMP_COL_PNK_SDW	(10)
#define	FBMP_COL_WHITE		(15)


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



struct _GTSNEGO_MESSAGE_WORK {
//  u32 bgchar;  //GFL_ARCUTIL_TRANSINFO
  u32 bgchar2S;  //SystemMsg
  u32 bgchar1M; //info

//  GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
  GFL_BUTTON_MAN* pButton;

  GFL_MSGDATA *pMsgData;  //

  WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
  GFL_FONT* pFontHandle;
  STRBUF* pStrBuf;
  STRBUF* pStrBufStream;
  STRBUF* pExStrBuf;
  int msgidx[_MESSAGE_INDEX_MAX];

  GFL_BMPWIN* infoDispWin;
  GFL_BMPWIN* systemDispWin;
  GFL_BMPWIN* mainDispWin[_BMP_WINDOW_NUM];

  GFL_BMPWIN* titleDispWin;
  
  GFL_BMPWIN* MyStatusDispWin[SCROLL_PANEL_NUM];

  GFL_BMPWIN* mainMsgWin;
  GFL_BMPWIN* pmsMsgWin;
  GFL_BMPWIN* numMsgWin2;
  GFL_BMPWIN* numMsgWin;
  GFL_BMPWIN* placeMsgWin;
  GFL_BMPWIN* nameMsgWin;

  PRINT_STREAM* pStream;
	GFL_TCBLSYS *pMsgTcblSys;
  PRINT_QUE*            SysMsgQue;

  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
	APP_TASKMENU_RES* pAppTaskRes;
  APP_TASKMENU_WIN_WORK* pAppWin;
  
//  int windowNum;
  HEAPID heapID;

  PMS_DRAW_WORK *pms_draw_work;
  
};


static void _ButtonSafeDelete(GTSNEGO_MESSAGE_WORK* pWork)
{
  if(pWork->pButton){
    GFL_BMN_Delete(pWork->pButton);
    pWork->pButton = NULL;
  }
}


GTSNEGO_MESSAGE_WORK* GTSNEGO_MESSAGE_Init(HEAPID id,int msg_dat)
{
  GTSNEGO_MESSAGE_WORK* pWork = GFL_HEAP_AllocClearMemory(id, sizeof(GTSNEGO_MESSAGE_WORK));
  pWork->heapID = id;

  GFL_BMPWIN_Init(pWork->heapID);
  GFL_FONTSYS_Init();
  pWork->pMsgTcblSys = GFL_TCBL_Init( pWork->heapID , pWork->heapID , 1 , 0 );
  pWork->SysMsgQue = PRINTSYS_QUE_Create( pWork->heapID );
  pWork->pStrBufStream = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pExStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msg_dat, pWork->heapID );
  pWork->pWordSet    = WORDSET_CreateEx( 6, 64,pWork->heapID );

  pWork->pAppTaskRes =
    APP_TASKMENU_RES_Create( GFL_BG_FRAME1_S, _SUBLIST_NORMAL_PAL,
                             pWork->pFontHandle, pWork->SysMsgQue, pWork->heapID  );

//  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME1_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  pWork->bgchar1M = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME1_M, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  pWork->bgchar2S = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME2_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
	
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
																0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
																0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);

#if DEBUG_ONLY_FOR_ohno
  DEBUGWIN_InitProc( GFL_BG_FRAME1_M , pWork->pFontHandle );
//  DEBUG_PAUSE_SetEnable( TRUE );
#endif
  
  return pWork;
}

void GTSNEGO_MESSAGE_Main(GTSNEGO_MESSAGE_WORK* pWork)
{
  GFL_FONTSYS_SetColor(1, 2, 15);
  GFL_TCBL_Main( pWork->pMsgTcblSys );
  PRINTSYS_QUE_Main(pWork->SysMsgQue);
  if(pWork->pAppWin){
    APP_TASKMENU_WIN_Update( pWork->pAppWin );
  }
  if( pWork->pms_draw_work ){
    PMS_DRAW_Main( pWork->pms_draw_work );
  }

}

void GTSNEGO_MESSAGE_End(GTSNEGO_MESSAGE_WORK* pWork)
{
  int i;
  
//  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
//                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_M,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar1M),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar1M));
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME2_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2S),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar2S));
  _ButtonSafeDelete(pWork);

//  GTSNEGO_MESSAGE_ButtonWindowDelete(pWork);

  GFL_FONTSYS_SetDefaultColor();
  GFL_MSG_Delete( pWork->pMsgData );
  GFL_FONT_Delete(pWork->pFontHandle);
  GFL_STR_DeleteBuffer(pWork->pStrBuf);
  GFL_STR_DeleteBuffer(pWork->pStrBufStream);
  GFL_STR_DeleteBuffer(pWork->pExStrBuf);
  
  _PMSDrawExit( pWork );

  if(pWork->pStream){
    PRINTSYS_PrintStreamDelete( pWork->pStream );
    pWork->pStream = NULL;
  }
  
  WORDSET_Delete(pWork->pWordSet );

  if(pWork->pAppWin){
    APP_TASKMENU_WIN_Delete(pWork->pAppWin);
    pWork->pAppWin = NULL;
  }
  
  APP_TASKMENU_RES_Delete( pWork->pAppTaskRes );
  GFL_TCBL_Exit(pWork->pMsgTcblSys);
  PRINTSYS_QUE_Clear(pWork->SysMsgQue);
  PRINTSYS_QUE_Delete(pWork->SysMsgQue);

  if(pWork->infoDispWin){
    GFL_BMPWIN_Delete(pWork->infoDispWin);
  }
  if(pWork->mainMsgWin){
    GFL_BMPWIN_Delete(pWork->mainMsgWin);
  }
  if(pWork->nameMsgWin){
    GFL_BMPWIN_Delete(pWork->nameMsgWin);
  }
  if(pWork->numMsgWin){
    GFL_BMPWIN_Delete(pWork->numMsgWin);
  }
  if(pWork->numMsgWin2){
    GFL_BMPWIN_Delete(pWork->numMsgWin2);
  }
  if(pWork->placeMsgWin){
    GFL_BMPWIN_Delete(pWork->placeMsgWin);
  }
  if(pWork->pmsMsgWin){
    GFL_BMPWIN_Delete(pWork->pmsMsgWin);
  }
  if(pWork->systemDispWin){
    GFL_BMPWIN_Delete(pWork->systemDispWin);
  }
  if(pWork->titleDispWin){
    GFL_BMPWIN_Delete(pWork->titleDispWin);
  }


  for(i=0;i< _BMP_WINDOW_NUM;i++){
    if(pWork->mainDispWin[i]){
      GFL_BMPWIN_Delete(pWork->mainDispWin[i]);
    }
  }
  for(i=0;i< SCROLL_PANEL_NUM;i++){
    if(pWork->MyStatusDispWin[i]){
      GFL_BMPWIN_Delete(pWork->MyStatusDispWin[i]);
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

void GTSNEGO_MESSAGE_InfoMessageDisp(GTSNEGO_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBufStream );
  
  if(pWork->infoDispWin==NULL){
    pWork->infoDispWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME1_M ,
      1 , 19, 30 ,4 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->infoDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) , 0, 0, pWork->pStrBufStream, pWork->pFontHandle );

//  pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pStrBufStream, pWork->pFontHandle,
 //                                       MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);

  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar1M), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);

}


//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_InfoMessageDispLine(GTSNEGO_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );
  
  if(pWork->infoDispWin==NULL){
    pWork->infoDispWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME1_M ,
      1 , 21, 30 ,2 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->infoDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) , 0, 0, pWork->pStrBuf, pWork->pFontHandle );

//  pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pStrBufStream, pWork->pFontHandle,
  //                                      MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);

  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar1M), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);


}





//  WORDSET_RegisterCountryName( pWork->pWordSet, 2, MyStatus_GetMyNation(pMyStatus));
//  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );




//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_MainMessageDispCore(GTSNEGO_MESSAGE_WORK* pWork)
{
  GFL_BMPWIN* pwin;

  if(pWork->mainMsgWin==NULL){
    pWork->mainMsgWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME1_M ,
      1 , 13, 30 ,6 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->mainMsgWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);
  GFL_FONTSYS_SetColor(15, 2, 0);
  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) , 3, 2, pWork->pStrBuf, pWork->pFontHandle );
  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
}


//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_MainMessageDisp(GTSNEGO_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );

  GTSNEGO_MESSAGE_MainMessageDispCore(pWork);
}

//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了待ち
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL GTSNEGO_MESSAGE_InfoMessageEndCheck(GTSNEGO_MESSAGE_WORK* pWork)
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

void GTSNEGO_MESSAGE_InfoMessageEnd(GTSNEGO_MESSAGE_WORK* pWork)
{
  if(pWork->infoDispWin){
    BmpWinFrame_Clear(pWork->infoDispWin, WINDOW_TRANS_OFF);
    GFL_BMPWIN_ClearScreen(pWork->infoDispWin);
    GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
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



APP_TASKMENU_WORK* GTSNEGO_MESSAGE_YesNoStart(GTSNEGO_MESSAGE_WORK* pWork,int type)
{
  int i;
  APP_TASKMENU_INITWORK appinit;
  APP_TASKMENU_WORK* pAppTask;

  appinit.heapId = pWork->heapID;
  appinit.itemNum =  2;
  appinit.itemWork =  &pWork->appitem[0];

  switch(type){
  case GTSNEGO_YESNOTYPE_INFO:
    appinit.charPosX = 32;
    appinit.charPosY = 14;
    appinit.posType = ATPT_RIGHT_DOWN;
    break;
  case GTSNEGO_YESNOTYPE_SYS:
    appinit.charPosX = 32;
    appinit.charPosY = 24;
    appinit.posType = ATPT_RIGHT_DOWN;
    break;
  }
	appinit.w				 = APP_TASKMENU_PLATE_WIDTH;
	appinit.h				 = APP_TASKMENU_PLATE_HEIGHT;

  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, GTSNEGO_030, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pWork->appitem[1].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, GTSNEGO_031, pWork->appitem[1].str);
  pWork->appitem[1].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pAppTask = APP_TASKMENU_OpenMenu(&appinit,pWork->pAppTaskRes);
  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
  GFL_STR_DeleteBuffer(pWork->appitem[1].str);
 // G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_OBJ , -8 );
  return pAppTask;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ボタンイベント実行関数
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

void GTSNEGO_MESSAGE_ButtonWindowMain(GTSNEGO_MESSAGE_WORK* pWork)
{
  GFL_BMN_Main( pWork->pButton );
}

//------------------------------------------------------------------------------
/**
 * @brief   システムウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_SystemMessageDisp(GTSNEGO_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );
  
  if(pWork->systemDispWin==NULL){
    pWork->systemDispWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME2_S , 1 , 3, 30 , 16 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->systemDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,0,0, pWork->pStrBuf, pWork->pFontHandle );
  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2S), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
}


//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ消去
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_SystemMessageEnd(GTSNEGO_MESSAGE_WORK* pWork)
{
  BmpWinFrame_Clear(pWork->systemDispWin, WINDOW_TRANS_OFF);
  GFL_BMPWIN_ClearScreen(pWork->systemDispWin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
}


//------------------------------------------------------------------------------
/**
 * @brief   システムウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_ErrorMessageDisp(GTSNEGO_MESSAGE_WORK* pWork,int msgid,int no)
{
  WORDSET_RegisterNumber(pWork->pWordSet, 0, no,
                         5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  GTSNEGO_MESSAGE_SystemMessageDisp(pWork, msgid);
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void GTSNEGO_MESSAGE_DispInit(GTSNEGO_MESSAGE_WORK* pWork)
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


static int GTSNEGO_MESSAGE_DispFrame(GTSNEGO_MESSAGE_WORK* pWork,int msgid,int x,int y, int width, int frame)
{
  GFL_BMPWIN* pwin;
  int i;

  for(i=0;i<_BMP_WINDOW_NUM;i++){
    if(pWork->mainDispWin[i]==NULL){
      pWork->mainDispWin[i] = GFL_BMPWIN_Create(
        frame , x , y, width , 2 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );

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

static int GTSNEGO_MESSAGE_Disp(GTSNEGO_MESSAGE_WORK* pWork,int msgid,int x,int y)
{
  return GTSNEGO_MESSAGE_DispFrame(pWork,msgid,x,y,20,GFL_BG_FRAME2_S);
}



static void GTSNEGO_MESSAGE_DispMsgChange(GTSNEGO_MESSAGE_WORK* pWork,int msgid,int idx)
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


static void GTSNEGO_MESSAGE_DispTransReq(GTSNEGO_MESSAGE_WORK* pWork)
{

  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
}

//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ消去
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_DispClear(GTSNEGO_MESSAGE_WORK* pWork)
{
  int i;
  
  for(i=0;i<_BMP_WINDOW_NUM;i++){
    if(pWork->mainDispWin[i]!=NULL){
      GFL_BG_LoadScreenV_Req(GFL_BMPWIN_GetFrame(pWork->mainDispWin[i]));
      BmpWinFrame_Clear(pWork->mainDispWin[i], WINDOW_TRANS_OFF);
      GFL_BMPWIN_ClearScreen(pWork->mainDispWin[i]);
//      GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
    }
  }
}





#define _BUTTON_ANYONE_X (8)
#define _BUTTON_ANYONE_Y (7)

#define _BUTTON_FRIEND_X (8)
#define _BUTTON_FRIEND_Y (13)

#define _BUTTON_TYPE_WIDTH (16*8)
#define _BUTTON_TYPE_HEIGHT (3*8)

static const GFL_UI_TP_HITTBL _AnyoneOrFriendBtnTbl[] = {
  {	 _BUTTON_ANYONE_Y*8,  _BUTTON_ANYONE_Y*8 + _BUTTON_TYPE_HEIGHT ,
    _BUTTON_ANYONE_X*8 , _BUTTON_ANYONE_X*8 + _BUTTON_TYPE_WIDTH  },
  {	_BUTTON_FRIEND_Y*8,  _BUTTON_FRIEND_Y*8 + _BUTTON_TYPE_HEIGHT ,
    _BUTTON_FRIEND_X*8 , _BUTTON_FRIEND_X*8 + _BUTTON_TYPE_WIDTH  },
  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};


//------------------------------------------------------------------------------
/**
 * @brief   だれでもかしっているひとか選択
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_DispAnyoneOrFriend(GTSNEGO_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork)
{

  GTSNEGO_MESSAGE_DispInit(pWork);

  GTSNEGO_MESSAGE_DispFrame(pWork, GTSNEGO_005, _BUTTON_ANYONE_X, _BUTTON_ANYONE_Y,16,GFL_BG_FRAME2_S );
  GTSNEGO_MESSAGE_DispFrame(pWork, GTSNEGO_006, _BUTTON_FRIEND_X, _BUTTON_FRIEND_Y,16,GFL_BG_FRAME2_S);

  GTSNEGO_MESSAGE_DispTransReq(pWork);

  _ButtonSafeDelete(pWork);
	pWork->pButton = GFL_BMN_Create( _AnyoneOrFriendBtnTbl, callback, pParentWork,  pWork->heapID );
}



#define _TOUCH_MARGIN (13)


static const GFL_UI_TP_HITTBL _levelselectBtnTbl[] = {
  {	_ARROW_LEVEL_YU*8-_TOUCH_MARGIN,   _ARROW_LEVEL_YU*8 + _TOUCH_MARGIN ,
    _ARROW_LEVEL_XU*8-_TOUCH_MARGIN ,  _ARROW_LEVEL_XU*8 + _TOUCH_MARGIN  },
  {	_ARROW_LEVEL_YD*8-_TOUCH_MARGIN,   _ARROW_LEVEL_YD*8 + _TOUCH_MARGIN ,
    _ARROW_LEVEL_XD*8-_TOUCH_MARGIN ,  _ARROW_LEVEL_XD*8 + _TOUCH_MARGIN  },

  {	_ARROW_MY_YU*8-_TOUCH_MARGIN,   _ARROW_MY_YU*8 + _TOUCH_MARGIN ,
    _ARROW_MY_XU*8-_TOUCH_MARGIN ,  _ARROW_MY_XU*8 + _TOUCH_MARGIN  },
  {	_ARROW_MY_YD*8-_TOUCH_MARGIN,   _ARROW_MY_YD*8 + _TOUCH_MARGIN ,
    _ARROW_MY_XD*8-_TOUCH_MARGIN ,  _ARROW_MY_XD*8 + _TOUCH_MARGIN  },

  {	_ARROW_FRIEND_YU*8-_TOUCH_MARGIN,   _ARROW_FRIEND_YU*8 + _TOUCH_MARGIN ,
    _ARROW_FRIEND_XU*8-_TOUCH_MARGIN ,  _ARROW_FRIEND_XU*8 + _TOUCH_MARGIN  },
  {	_ARROW_FRIEND_YD*8-_TOUCH_MARGIN,   _ARROW_FRIEND_YD*8 + _TOUCH_MARGIN ,
    _ARROW_FRIEND_XD*8-_TOUCH_MARGIN ,  _ARROW_FRIEND_XD*8 + _TOUCH_MARGIN },

  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};



//------------------------------------------------------------------------------
/**
 * @brief   レベルの条件選択
 * @retval  none
 */
//------------------------------------------------------------------------------


static u16 levelmsg[]={GTSNEGO_012,GTSNEGO_009,GTSNEGO_010,GTSNEGO_011};
static u16 mymsg[]={GTSNEGO_017,GTSNEGO_013,GTSNEGO_014,GTSNEGO_015,GTSNEGO_016};


void GTSNEGO_MESSAGE_DispLevel(GTSNEGO_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork,int lvno,int myno, int friendno)
{

  GTSNEGO_MESSAGE_DispInit(pWork);

  GFL_FONTSYS_SetColor(FBMP_COL_WHITE, FBMP_COL_BLK_SDW, 0);
  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_007, 0, 0);

  GFL_FONTSYS_SetColor(FBMP_COL_WHITE, FBMP_COL_BLK_SDW, 0);
  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_008, 3, 6);
  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_028, 1, 11);
  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_029, 1, 16);

  GFL_FONTSYS_SetColor(FBMP_COL_BLACK, FBMP_COL_BLK_SDW, 0);
  pWork->msgidx[_MESSAGE_LEVEL] = GTSNEGO_MESSAGE_Disp(pWork, levelmsg[lvno],  13,  6);
  pWork->msgidx[_MESSAGE_MY] = GTSNEGO_MESSAGE_Disp(pWork, mymsg[myno], 16, 11);
  pWork->msgidx[_MESSAGE_FRIEND] = GTSNEGO_MESSAGE_Disp(pWork, mymsg[friendno], 16, 16);

//  GTSNEGO_MESSAGE_DispLevelChange(pWork->pMessageWork,pWork->chageLevel);
//  GTSNEGO_MESSAGE_DispMyChange(pWork->pMessageWork,pWork->myChageType);
//  GTSNEGO_MESSAGE_DispFriendChange(pWork->pMessageWork,pWork->friendChageType);

  GTSNEGO_MESSAGE_DispTransReq(pWork);

  _ButtonSafeDelete(pWork);
	pWork->pButton = GFL_BMN_Create( _levelselectBtnTbl, callback, pParentWork,  pWork->heapID );
}


void GTSNEGO_MESSAGE_DispLevelChange(GTSNEGO_MESSAGE_WORK* pWork,int no)
{
  GTSNEGO_MESSAGE_DispMsgChange(pWork, levelmsg[no],  pWork->msgidx[_MESSAGE_LEVEL]);
}

void GTSNEGO_MESSAGE_DispMyChange(GTSNEGO_MESSAGE_WORK* pWork,int no)
{
  GTSNEGO_MESSAGE_DispMsgChange(pWork, mymsg[no],  pWork->msgidx[_MESSAGE_MY]);
}

void GTSNEGO_MESSAGE_DispFriendChange(GTSNEGO_MESSAGE_WORK* pWork,int no)
{
  GTSNEGO_MESSAGE_DispMsgChange(pWork, mymsg[no],  pWork->msgidx[_MESSAGE_FRIEND]);
}


void GTSNEGO_MESSAGE_DeleteDispLevel(GTSNEGO_MESSAGE_WORK* pWork)
{
  _ButtonSafeDelete(pWork);
}




APP_TASKMENU_WIN_WORK* GTSNEGO_MESSAGE_SearchButtonStart(GTSNEGO_MESSAGE_WORK* pWork,int msgno)
{
  APP_TASKMENU_WIN_WORK* pAppWin;
  int length;

  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, msgno, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  pWork->appitem[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;

  length = PRINTSYS_GetStrWidth( pWork->appitem[0].str , pWork->pFontHandle , 0 );
  length+=32;
  pAppWin = APP_TASKMENU_WIN_CreateEx( pWork->pAppTaskRes,
                                           pWork->appitem, 16-(length/16), 24-3, ((length/16)*2),3, FALSE,TRUE, pWork->heapID);

  
  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
  
  return pAppWin;
}




void GTSNEGO_MESSAGE_DispCountryInfo(GTSNEGO_MESSAGE_WORK* pWork, int msg)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msg, pWork->pStrBuf );
  if(pWork->placeMsgWin==NULL){
    pWork->placeMsgWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_M , 1 , 13, 14 , 3 ,  _NUM_FONT_PALETTE , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->placeMsgWin;
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);
  GFL_FONTSYS_SetColor(15, 14, 0);
  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) , 2, 3, pWork->pStrBuf, pWork->pFontHandle );
  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  
}




static void _SetCountry(GTSNEGO_MESSAGE_WORK* pWork,MYSTATUS* pMyStatus)
{
  GFL_BMPWIN* pwin;

  WORDSET_RegisterCountryName( pWork->pWordSet, 0, MyStatus_GetMyNation(pMyStatus));
  WORDSET_RegisterLocalPlaceName( pWork->pWordSet, 1, MyStatus_GetMyNation(pMyStatus),MyStatus_GetMyArea(pMyStatus));

  GFL_MSG_GetString( pWork->pMsgData, GTSNEGO_039, pWork->pExStrBuf );
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );

  if(pWork->mainMsgWin==NULL){
    pWork->mainMsgWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME1_M ,
      1 , 15, 30 ,5 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->mainMsgWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);
  GFL_FONTSYS_SetColor(15, 2, 0);
  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) , 5, 5, pWork->pStrBuf, pWork->pFontHandle );
  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);

  
}


void GTSNEGO_MESSAGE_DeleteCountryMsg(GTSNEGO_MESSAGE_WORK* pWork)
{
  if(pWork->mainMsgWin){
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->mainMsgWin), 0);
    GFL_BMPWIN_TransVramCharacter(pWork->mainMsgWin);
    GFL_BMPWIN_ClearScreen(pWork->mainMsgWin);
    GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
    GFL_BMPWIN_Delete(pWork->mainMsgWin);
    pWork->mainMsgWin=NULL;
  }
}




static int _PrintMyStatusDisp(GTSNEGO_MESSAGE_WORK* pWork,MYSTATUS* pMyStatus, int i)
{
  GFL_BMPWIN* pwin;

  if(pWork->MyStatusDispWin[ i ]==NULL){
    pWork->MyStatusDispWin[i] = GFL_BMPWIN_Create(
      GFL_BG_FRAME2_S , 7 , i * 6 + 1, 22 , 4 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->MyStatusDispWin[i];
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);

  GFL_STR_ClearBuffer(pWork->pExStrBuf);
  GFL_STR_ClearBuffer(pWork->pStrBuf);

  WORDSET_RegisterPlayerName( pWork->pWordSet, 0, pMyStatus );
  WORDSET_RegisterNumber(pWork->pWordSet, 1, MyStatus_GetID_Low(pMyStatus),
                         5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterCountryName( pWork->pWordSet, 2, MyStatus_GetMyNation(pMyStatus));

  GFL_MSG_GetString( pWork->pMsgData, GTSNEGO_033, pWork->pExStrBuf );
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );

  GFL_FONTSYS_SetColor(FBMP_COL_WHITE, FBMP_COL_BLK_SDW, 0);
  
  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,0, 0, pWork->pStrBuf, pWork->pFontHandle );
  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  return 0;
}



//----------------------------------------------------------------------------
/**
 *	@brief	フレンド選択初期化
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_MESSAGE_FriendListPlateDisp(GTSNEGO_MESSAGE_WORK* pWork,GAMEDATA* pGameData)
{
  int i, j, count;
  MYSTATUS* pMyStatus;

  GFL_FONTSYS_SetColor(FBMP_COL_WHITE, 2, 0);
  GTSNEGO_MESSAGE_DispFrame(pWork, GTSNEGO_035, 0, 0, 20,GFL_BG_FRAME1_S);

  for(i = 0 ; i < 3; i++){
    pMyStatus = GTSNEGO_GetMyStatus(pGameData, i);
    if(pMyStatus){
      _PrintMyStatusDisp(pWork, pMyStatus, i+2);
    }
  }
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
}


//----------------------------------------------------------------------------
/**
 *	@brief	フレンドリストのダウンスタート
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_MESSAGE_FriendListDownStart(GTSNEGO_MESSAGE_WORK* pWork,GAMEDATA* pGameData, int no)
{
  MYSTATUS* pMyStatus;
  pMyStatus = GTSNEGO_GetMyStatus(pGameData,no);
  _PrintMyStatusDisp(pWork, pMyStatus, SCROLL_PANEL_NUM-1);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
}



void GTSNEGO_MESSAGE_FriendListDownEnd(GTSNEGO_MESSAGE_WORK* pWork)
{
  int i;

  //一番上を消す
  if(pWork->MyStatusDispWin[0]){
    GFL_BMPWIN_Delete(pWork->MyStatusDispWin[0]);
    pWork->MyStatusDispWin[0]=NULL;
  }
  for(i = 1 ; i < SCROLL_PANEL_NUM ; i++){  //場所スライド
    if(pWork->MyStatusDispWin[ i ]){
      int y = GFL_BMPWIN_GetPosY( pWork->MyStatusDispWin[ i ] );
      GFL_BMPWIN_SetPosY( pWork->MyStatusDispWin[ i ], y - (SCROLL_HEIGHT_SINGLE/8) );
      GFL_BMPWIN_MakeScreen(pWork->MyStatusDispWin[ i ]);
    }
  }
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
  //バッファ移動
  {
    int endmark = SCROLL_PANEL_NUM-1;
    for(i = 0 ; i < endmark ; i++){  //場所スライド
      pWork->MyStatusDispWin[ i ] = pWork->MyStatusDispWin[ i+1 ];
    }
    pWork->MyStatusDispWin[ endmark ] = NULL;
  }

}



//----------------------------------------------------------------------------
/**
 *	@brief	フレンドリストの全書き換え
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_MESSAGE_FriendListRenew(GTSNEGO_MESSAGE_WORK* pWork,GAMEDATA* pGameData, int no)
{
  MYSTATUS* pMyStatus;
  int i;

  for( i =0;i < SCROLL_PANEL_NUM;i++){
    int x = i+no;
    if(x >= 0){
      pMyStatus = GTSNEGO_GetMyStatus(pGameData, x);
      if(pMyStatus){
        _PrintMyStatusDisp(pWork, pMyStatus, i);
      }
    }
  }
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
}




//----------------------------------------------------------------------------
/**
 *	@brief	フレンドリストのアップスタート
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_MESSAGE_FriendListUpStart(GTSNEGO_MESSAGE_WORK* pWork,GAMEDATA* pGameData, int no)
{
  MYSTATUS* pMyStatus;
  pMyStatus = GTSNEGO_GetMyStatus(pGameData,no);
  _PrintMyStatusDisp(pWork, pMyStatus, 0);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
}



void GTSNEGO_MESSAGE_FriendListUpEnd(GTSNEGO_MESSAGE_WORK* pWork)
{
  int i;

  //一番下を消す
  if(pWork->MyStatusDispWin[5]){
    GFL_BMPWIN_Delete(pWork->MyStatusDispWin[5]);
    pWork->MyStatusDispWin[5]=NULL;
  }
  for(i = 0 ; i < SCROLL_PANEL_NUM ; i++){  //場所スライド
    if(pWork->MyStatusDispWin[ i ]){
      int y = GFL_BMPWIN_GetPosY( pWork->MyStatusDispWin[ i ] );
      GFL_BMPWIN_SetPosY( pWork->MyStatusDispWin[ i ], y + (SCROLL_HEIGHT_SINGLE/8) );
      GFL_BMPWIN_MakeScreen(pWork->MyStatusDispWin[ i ]);
    }
  }
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
  //バッファ移動

  {
    int endmark = SCROLL_PANEL_NUM-1;
    for(i = endmark ; i > 0; i--){  //場所スライド
      pWork->MyStatusDispWin[i] =pWork->MyStatusDispWin[i-1];
    }
    pWork->MyStatusDispWin[0] = NULL;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   まちあわせ もどるウインドウ
 * @retval  none
 */
//------------------------------------------------------------------------------



APP_TASKMENU_WORK* GTSNEGO_MESSAGE_MatchOrReturnStart(GTSNEGO_MESSAGE_WORK* pWork,int type)
{
  int i;
  APP_TASKMENU_INITWORK appinit;
  APP_TASKMENU_WORK* pAppTask;

  appinit.heapId = pWork->heapID;
  appinit.itemNum =  2;
  appinit.itemWork =  &pWork->appitem[0];

  switch(type){
  case GTSNEGO_YESNOTYPE_INFO:
    appinit.charPosX = 32;
    appinit.charPosY = 14;
    appinit.posType = ATPT_RIGHT_DOWN;
    break;
  case GTSNEGO_YESNOTYPE_SYS:
    appinit.charPosX = 32;
    appinit.charPosY = 24;
    appinit.posType = ATPT_RIGHT_DOWN;
    break;
  }
	appinit.w				 = APP_TASKMENU_PLATE_WIDTH;
	appinit.h				 = APP_TASKMENU_PLATE_HEIGHT;

  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, GTSNEGO_032, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pWork->appitem[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  pWork->appitem[1].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, GTSNEGO_038, pWork->appitem[1].str);
  pWork->appitem[1].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pWork->appitem[1].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  pAppTask = APP_TASKMENU_OpenMenu(&appinit,pWork->pAppTaskRes);
  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
  GFL_STR_DeleteBuffer(pWork->appitem[1].str);
  return pAppTask;
}


void GTSNEGO_MESSAGE_AppMenuClose(APP_TASKMENU_WORK* pAppTask)
{
  if(pAppTask){
    G2_BlendNone();
    APP_TASKMENU_CloseMenu(pAppTask);
  }
}



void GTSNEGO_MESSAGE_PMSDrawInit(GTSNEGO_MESSAGE_WORK* pWork, GTSNEGO_DISP_WORK* pDispWork)
{
  if(pWork->pms_draw_work==NULL){
    GFL_CLUNIT* cellUnit = GTSNEGO_DISP_GetCellUtil(pDispWork);
    pWork->pms_draw_work = PMS_DRAW_Init( cellUnit, CLSYS_DRAW_MAIN, 
                                          pWork->SysMsgQue, pWork->pFontHandle, _OBJPAL_PMS_POS_MAIN, 4, pWork->heapID );
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief 簡易会話描画システム解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void _PMSDrawExit( GTSNEGO_MESSAGE_WORK* pWork )
{
  if(pWork->pms_draw_work){
    PMS_DRAW_Exit( pWork->pms_draw_work );
    pWork->pms_draw_work =NULL;
  }
}






//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ表示 GTSNEGO_040
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_PMSDisp(GTSNEGO_MESSAGE_WORK* pWork,PMS_DATA* pms)
{
  GFL_BMPWIN* pwin;
  int i;
  GFL_POINT offset={5,5};

  if(pWork->pmsMsgWin==NULL){
    pWork->pmsMsgWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME1_M ,
      1 , 15, 30 , 5 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  
  {
    pwin = pWork->pmsMsgWin;
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);
    GFL_FONTSYS_SetColor(15, 2, 0);

    PMS_DRAW_SetNullColorPallet( pWork->pms_draw_work, 0 );
    PMS_DRAW_SetPrintColor( pWork->pms_draw_work, PRINTSYS_LSB_Make( 15, 2, 0) );
    

    PMS_DRAW_PrintOffset(pWork->pms_draw_work, pwin,  pms, 0, &offset );

//    PMS_DRAW_Print( pWork->pms_draw_work, pwin,  pms, 0 );
    
//    PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,0,0, pWork->pStrBuf, pWork->pFontHandle );
    GFL_BMPWIN_TransVramCharacter(pwin);
    GFL_BMPWIN_MakeScreen(pwin);
  }
  
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
}



//------------------------------------------------------------------------------
/**
 * @brief   システムウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_TitleMessage(GTSNEGO_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );
  
  if(pWork->titleDispWin==NULL){
    pWork->titleDispWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_M , 0 , 0, 14 , 2 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->titleDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);
  GFL_FONTSYS_SetColor(15, 2, 0);

  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,1, 2, pWork->pStrBuf, pWork->pFontHandle );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
}






static const GFL_UI_TP_HITTBL _CancelBtnTbl[] = {
  { 24*8-32 , 24*8, 128-48, 128+48},
  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};





//------------------------------------------------------------------------------
/**
 * @brief   キャンセルボタンをつくる
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_CancelButtonCreate(GTSNEGO_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork )
{

  
  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, GTSNEGO_020, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  pWork->appitem[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;

  GF_ASSERT(pWork->pAppWin==NULL);
  pWork->pAppWin =APP_TASKMENU_WIN_CreateEx( pWork->pAppTaskRes,
                                           pWork->appitem, 16 - 6, 24-3, 12, 3,FALSE,TRUE, pWork->heapID);

  if(GFL_UI_CheckTouchOrKey()==GFL_APP_KTST_KEY){
    APP_TASKMENU_WIN_SetActive( pWork->pAppWin , TRUE);
  }

  GFL_STR_DeleteBuffer(pWork->appitem[0].str);

  _ButtonSafeDelete(pWork);
	pWork->pButton = GFL_BMN_Create( _CancelBtnTbl, callback, pParentWork,  pWork->heapID );

}

//------------------------------------------------------------------------------
/**
 * @brief   キャンセルボタンを決定
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_CancelButtonDecide(GTSNEGO_MESSAGE_WORK* pWork)
{
  APP_TASKMENU_WIN_SetDecide(pWork->pAppWin,TRUE);
}



//------------------------------------------------------------------------------
/**
 * @brief   キャンセルボタンを消す
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_CancelButtonDeleteForce(GTSNEGO_MESSAGE_WORK* pWork)
{
  APP_TASKMENU_WIN_Delete(pWork->pAppWin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
  pWork->pAppWin = NULL;
}


//------------------------------------------------------------------------------
/**
 * @brief   キャンセルボタンを消す
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL GTSNEGO_MESSAGE_CancelButtonDelete(GTSNEGO_MESSAGE_WORK* pWork)
{
  if(APP_TASKMENU_WIN_IsFinish(pWork->pAppWin)){
    GTSNEGO_MESSAGE_CancelButtonDeleteForce(pWork);
    return TRUE;
  }
  return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   システムウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _NumMessage(GTSNEGO_MESSAGE_WORK* pWork,int msgid, int no)
{
  GFL_BMPWIN* pwin;



  GFL_MSG_GetString( pWork->pMsgData, GTSNEGO_045, pWork->pStrBuf );

  if(pWork->numMsgWin2==NULL){
    pWork->numMsgWin2 = GFL_BMPWIN_Create( GFL_BG_FRAME1_M , 0x15 , 4, 10 , 2 ,  _NUM_FONT_PALETTE , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->numMsgWin2;
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);
  GFL_FONTSYS_SetColor(15, 14, 0);
  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) , 0, 3, pWork->pStrBuf, pWork->pFontHandle );
  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);


  WORDSET_RegisterNumber(pWork->pWordSet, 0, no,
                         5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
  GFL_MSG_GetString( pWork->pMsgData, GTSNEGO_041, pWork->pExStrBuf );
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
  
  
  if(pWork->numMsgWin==NULL){
    pWork->numMsgWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_M , 0x15 , 6, 10 , 3 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->numMsgWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);
  GFL_FONTSYS_SetColor(15, 2, 0);

  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) , 0, 5, pWork->pStrBuf, pWork->pFontHandle );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
}

//------------------------------------------------------------------------------
/**
 * @brief   システムウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _NameMessage(GTSNEGO_MESSAGE_WORK* pWork,MYSTATUS* pMy)
{
  GFL_BMPWIN* pwin;

  WORDSET_RegisterPlayerName( pWork->pWordSet, 0, pMy );
  
  GFL_MSG_GetString( pWork->pMsgData, GTSNEGO_042, pWork->pExStrBuf );
  WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
  
  if(pWork->nameMsgWin==NULL){
    pWork->nameMsgWin = GFL_BMPWIN_Create( GFL_BG_FRAME1_M , 11 , 10, 10 , 3 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->nameMsgWin;
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 0);
  GFL_FONTSYS_SetColor(1, 2, 0);
  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) , 0, 2, pWork->pStrBuf, pWork->pFontHandle );
  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
}



void GTSNEGO_MESSAGE_FindPlayer(GTSNEGO_MESSAGE_WORK* pWork,MYSTATUS* pMy, int num)
{

  _NameMessage(pWork, pMy);
  _NumMessage(pWork, GTSNEGO_041, num);
  GTSNEGO_MESSAGE_DispCountryInfo(pWork, GTSNEGO_046);
  _SetCountry(pWork, pMy);




  //  GTSNEGO_MESSAGE_MainMessageDispCore(pWork);
  GTSNEGO_MESSAGE_InfoMessageDispLine(pWork,GTSNEGO_022);
  
}



//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ消去
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_ResetDispSet(GTSNEGO_MESSAGE_WORK* pWork)
{
  int i;
  if(pWork->titleDispWin){
    BmpWinFrame_Clear(pWork->titleDispWin, WINDOW_TRANS_OFF);
    GFL_BMPWIN_ClearScreen(pWork->titleDispWin);
    GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
    GFL_BMPWIN_Delete(pWork->titleDispWin);
    pWork->titleDispWin=NULL;
  }
  if(pWork->pAppWin){
    APP_TASKMENU_WIN_Delete(pWork->pAppWin);
    pWork->pAppWin = NULL;
  }

  for(i=0;i< SCROLL_PANEL_NUM;i++){
    if(pWork->MyStatusDispWin[i]){
      GFL_BMPWIN_ClearScreen(pWork->MyStatusDispWin[i]);
      GFL_BMPWIN_Delete(pWork->MyStatusDispWin[i]);
      pWork->MyStatusDispWin[i]=NULL;
    }
  }
  
  GTSNEGO_MESSAGE_DispClear(pWork);
  GTSNEGO_MESSAGE_DispInit(pWork);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
																0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);

}




