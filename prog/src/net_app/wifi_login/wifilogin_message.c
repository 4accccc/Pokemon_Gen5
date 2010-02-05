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

#include "wifilogin_local.h"
#include "wifi_login.naix"
#include "msg/msg_wifi_system.h"

//--------------------------------------------
// 画面構成定義
//--------------------------------------------
#define _WINDOW_MAXNUM (4)   //ウインドウのパターン数
#define _MESSAGE_BUF_NUM	( 200*2 )
#define _SUBMENU_LISTMAX (2)

#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (3)    // ウインドウ高さ


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

#define _SUBLIST_NORMAL_PAL   (9)   //サブメニューの通常パレット


struct _WIFILOGIN_YESNO_WORK
{ 
  WIFILOGIN_DISPLAY display;
  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
	APP_TASKMENU_RES  *pAppTaskRes;
  APP_TASKMENU_WORK *pAppTask;
  BMPMENU_WORK      *pYesNoWork;
  u32 yesno_ret;
};

struct _WIFILOGIN_MESSAGE_WORK {
  u32 bgchar;  //GFL_ARCUTIL_TRANSINFO
  u32 bgchar2S;  //SystemMsg

  GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
  GFL_BUTTON_MAN* pButton;

  GFL_MSGDATA *pMsgData;  //

  WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
  GFL_FONT* pFontHandle;
  STRBUF* pStrBuf;

  GFL_BMPWIN* infoDispWin;
  GFL_BMPWIN* systemDispWin;


  PRINT_STREAM* pStream;
	GFL_TCBLSYS *pMsgTcblSys;
  PRINT_QUE*            SysMsgQue;

//  int windowNum;
  HEAPID heapID;

  WIFILOGIN_DISPLAY display;

  WIFILOGIN_YESNO_WORK  yesno_wk;
};


//------------------------------------------------------------------
//	プロトタイプ
//------------------------------------------------------------------
static u8 WifiLogin_Message_GetTextFrame( WIFILOGIN_DISPLAY display );
static u8 WifiLogin_Message_GetSysFrame( WIFILOGIN_DISPLAY display );
static BMPMENU_WORK * WIFILOGIN_MESSAGE_YesNoWinCreate(WIFILOGIN_MESSAGE_WORK* pWork);


WIFILOGIN_MESSAGE_WORK* WIFILOGIN_MESSAGE_Init(HEAPID id,int msg_dat, WIFILOGIN_DISPLAY display)
{
  WIFILOGIN_MESSAGE_WORK* pWork = GFL_HEAP_AllocClearMemory(id, sizeof(WIFILOGIN_MESSAGE_WORK));
  pWork->heapID   = id;
  pWork->display  = display;

  GFL_BMPWIN_Init(pWork->heapID);
  GFL_FONTSYS_Init();
  pWork->pMsgTcblSys = GFL_TCBL_Init( pWork->heapID , pWork->heapID , 1 , 0 );
  pWork->SysMsgQue = PRINTSYS_QUE_Create( pWork->heapID );
  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msg_dat, pWork->heapID );

  //下画面でしかAPP_TASKは使わない
  pWork->yesno_wk.pAppTaskRes =
    APP_TASKMENU_RES_Create( GFL_BG_FRAME1_S, _SUBLIST_NORMAL_PAL,
                             pWork->pFontHandle, pWork->SysMsgQue, pWork->heapID  );

  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan( WifiLogin_Message_GetTextFrame( pWork->display ), _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  pWork->bgchar2S = BmpWinFrame_GraphicSetAreaMan(WifiLogin_Message_GetSysFrame( pWork->display ), _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
	
  {
    PALTYPE paltype;
    if( pWork->display == WIFILOGIN_DISPLAY_DOWN )
    { 
      paltype = PALTYPE_SUB_BG;
    }
    else
    { 
      paltype = PALTYPE_MAIN_BG;
    }
    GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, paltype,
																0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  }
  
  return pWork;
}

void WIFILOGIN_MESSAGE_Main(WIFILOGIN_MESSAGE_WORK* pWork)
{
  GFL_TCBL_Main( pWork->pMsgTcblSys );
  PRINTSYS_QUE_Main(pWork->SysMsgQue);

}

void WIFILOGIN_MESSAGE_End(WIFILOGIN_MESSAGE_WORK* pWork)
{
  GFL_BG_FreeCharacterArea(WifiLogin_Message_GetTextFrame( pWork->display ),
      GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));
  GFL_BG_FreeCharacterArea(WifiLogin_Message_GetSysFrame( pWork->display ),
                          GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2S),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar2S));


  GFL_FONTSYS_SetDefaultColor();
  GFL_MSG_Delete( pWork->pMsgData );
  GFL_FONT_Delete(pWork->pFontHandle);
  GFL_STR_DeleteBuffer(pWork->pStrBuf);

  APP_TASKMENU_RES_Delete( pWork->yesno_wk.pAppTaskRes );
  GFL_TCBL_Exit(pWork->pMsgTcblSys);
  PRINTSYS_QUE_Clear(pWork->SysMsgQue);
  PRINTSYS_QUE_Delete(pWork->SysMsgQue);
  if(pWork->infoDispWin){
    GFL_BMPWIN_Delete(pWork->infoDispWin);
  }
  if(pWork->systemDispWin){
    GFL_BMPWIN_Delete(pWork->systemDispWin);
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

void WIFILOGIN_MESSAGE_InfoMessageDisp(WIFILOGIN_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;
  u8 y;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );

  if( pWork->display == WIFILOGIN_DISPLAY_DOWN )
  { 
    y = 1;
  }
  else
  { 
    y = 24-5;
  }
  
  if(pWork->infoDispWin==NULL){
    pWork->infoDispWin = GFL_BMPWIN_Create(
      WifiLogin_Message_GetTextFrame( pWork->display ) ,
      1 , y, 30 ,4 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->infoDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pStrBuf, pWork->pFontHandle,
                                        MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);

  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(WifiLogin_Message_GetTextFrame( pWork->display ));
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了待ち
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL WIFILOGIN_MESSAGE_InfoMessageEndCheck(WIFILOGIN_MESSAGE_WORK* pWork)
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

void WIFILOGIN_MESSAGE_InfoMessageEnd(WIFILOGIN_MESSAGE_WORK* pWork)
{
  BmpWinFrame_Clear(pWork->infoDispWin, WINDOW_TRANS_OFF);
  GFL_BMPWIN_ClearScreen(pWork->infoDispWin);
  GFL_BG_LoadScreenV_Req(WifiLogin_Message_GetTextFrame( pWork->display ));
}

//------------------------------------------------------------------------------
/**
 * @brief   はいいいえウインドウ
 * @retval  none
 */
//------------------------------------------------------------------------------



WIFILOGIN_YESNO_WORK* WIFILOGIN_MESSAGE_YesNoStart(WIFILOGIN_MESSAGE_WORK* pWork,int type)
{
  WIFILOGIN_YESNO_WORK* yesno_wk  = &pWork->yesno_wk;
  yesno_wk->display = pWork->display;

  switch( yesno_wk->display )
  { 
  case WIFILOGIN_DISPLAY_DOWN:
    { 
      int i;
      APP_TASKMENU_INITWORK appinit;
      APP_TASKMENU_WORK* pAppTask;

      appinit.heapId = pWork->heapID;
      appinit.itemNum =  2;
      appinit.itemWork =  &yesno_wk->appitem[0];

      switch(type){
      case WIFILOGIN_YESNOTYPE_INFO:
        appinit.charPosX = 32;
        appinit.charPosY = 13;
        appinit.posType = ATPT_RIGHT_DOWN;
        break;
      case WIFILOGIN_YESNOTYPE_SYS:
        appinit.charPosX = 32;
        appinit.charPosY = 24;
        appinit.posType = ATPT_RIGHT_DOWN;
        break;
      }
      appinit.w				 = APP_TASKMENU_PLATE_WIDTH;
      appinit.h				 = APP_TASKMENU_PLATE_HEIGHT;

      yesno_wk->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
      GFL_MSG_GetString(pWork->pMsgData, dwc_message_0013, yesno_wk->appitem[0].str);
      yesno_wk->appitem[0].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
      yesno_wk->appitem[1].str = GFL_STR_CreateBuffer(100, pWork->heapID);
      GFL_MSG_GetString(pWork->pMsgData, dwc_message_0014, yesno_wk->appitem[1].str);
      yesno_wk->appitem[1].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
      yesno_wk->pAppTask = APP_TASKMENU_OpenMenu(&appinit,yesno_wk->pAppTaskRes);
      GFL_STR_DeleteBuffer(yesno_wk->appitem[0].str);
      GFL_STR_DeleteBuffer(yesno_wk->appitem[1].str);
      G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , -8 );
    }
    break;

  case WIFILOGIN_DISPLAY_UP:
    yesno_wk->pYesNoWork = WIFILOGIN_MESSAGE_YesNoWinCreate(pWork);
    break;
  }
  return yesno_wk;
}
//------------------------------------------------------------------------------
/**
 * @brief   はいいいえウインドウ  終了
 * @param   pWork はいいいえワーク
 */
//------------------------------------------------------------------------------
void WIFILOGIN_MESSAGE_YesNoEnd( WIFILOGIN_YESNO_WORK* pWork )
{ 
  switch( pWork->display )
  { 
  case WIFILOGIN_DISPLAY_DOWN:
    APP_TASKMENU_CloseMenu( pWork->pAppTask );
    break;
  case WIFILOGIN_DISPLAY_UP:
    //下がコメントなのは、BmpMenu_YesNoSelectMainの内部で、決定した瞬間に
    //EXITされているので
    //BmpMenu_YesNoMenuExit( pWork->pYesNoWork );
    break;
  }
}
//------------------------------------------------------------------------------
/**
 * @brief   はいいいえウインドウ  実行
 * @param   pWork はいいいえワーク
 */
//------------------------------------------------------------------------------
void WIFILOGIN_MESSAGE_YesNoUpdate( WIFILOGIN_YESNO_WORK* pWork )
{ 
  switch( pWork->display )
  { 
  case WIFILOGIN_DISPLAY_DOWN:
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
    break;
  case WIFILOGIN_DISPLAY_UP:
    pWork->yesno_ret  = BmpMenu_YesNoSelectMain( pWork->pYesNoWork );
    break;
  }
}
//------------------------------------------------------------------------------
/**
 * @brief   はいいいえウインドウ  選んだかどうか
 * @param   pWork はいいいえワーク
 * @retval  TRUEで選択終了  FALSEで選択中
 */
//------------------------------------------------------------------------------
BOOL WIFILOGIN_MESSAGE_YesNoIsFinish( const WIFILOGIN_YESNO_WORK* pWork )
{ 
  switch( pWork->display )
  { 
  case WIFILOGIN_DISPLAY_DOWN:
    return APP_TASKMENU_IsFinish(pWork->pAppTask);
  case WIFILOGIN_DISPLAY_UP:
    return pWork->yesno_ret != BMPMENU_NULL;
  default:
    GF_ASSERT(0);
    return 0;
  }
}
//------------------------------------------------------------------------------
/**
 * @brief   はいいいえウインドウ  カーソルの位置を取得
 * @param   pWork はいいいえワーク
 * @retval  カーソルの位置を取得
 */
//------------------------------------------------------------------------------
u8 WIFILOGIN_MESSAGE_YesNoGetCursorPos( const WIFILOGIN_YESNO_WORK* pWork )
{ 
  switch( pWork->display )
  { 
  case WIFILOGIN_DISPLAY_DOWN:
    return APP_TASKMENU_GetCursorPos(pWork->pAppTask);
  case WIFILOGIN_DISPLAY_UP:
    return pWork->yesno_ret;
  default:
    GF_ASSERT(0);
    return 0;
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   システムウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void WIFILOGIN_MESSAGE_SystemMessageDisp(WIFILOGIN_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );
  
  if(pWork->systemDispWin==NULL){
    pWork->systemDispWin = GFL_BMPWIN_Create(
      WifiLogin_Message_GetSysFrame( pWork->display )
      , 1 , 2, 30 , 16 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->systemDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin) ,0,0, pWork->pStrBuf, pWork->pFontHandle );
  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2S), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(WifiLogin_Message_GetSysFrame( pWork->display ));
}


//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ消去
 * @retval  none
 */
//------------------------------------------------------------------------------

void WIFILOGIN_MESSAGE_SystemMessageEnd(WIFILOGIN_MESSAGE_WORK* pWork)
{
  BmpWinFrame_Clear(pWork->systemDispWin, WINDOW_TRANS_OFF);
  GFL_BMPWIN_ClearScreen(pWork->systemDispWin);
  GFL_BG_LoadScreenV_Req(WifiLogin_Message_GetSysFrame( pWork->display ));
}


//------------------------------------------------------------------------------
/**
 * @brief   システムウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

void WIFILOGIN_MESSAGE_ErrorMessageDisp(WIFILOGIN_MESSAGE_WORK* pWork,int msgid,int no)
{
  WORDSET_RegisterNumber(pWork->pWordSet, 0, no,
                         5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  WIFILOGIN_MESSAGE_SystemMessageDisp(pWork, msgid);
}

//----------------------------------------------------------------------------
/**
 *	@brief  テキスト表示面を取得
 *
 *	@param	WIFILOGIN_DISPLAY display   描画先
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static u8 WifiLogin_Message_GetTextFrame( WIFILOGIN_DISPLAY display )
{ 
  switch( display )
  { 
  case WIFILOGIN_DISPLAY_UP:
    return GFL_BG_FRAME1_M;

  case WIFILOGIN_DISPLAY_DOWN:
    return GFL_BG_FRAME1_S;

  default:
    GF_ASSERT( 0);
    return 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  システムメッセージ表示面を取得
 *
 *	@param	WIFILOGIN_DISPLAY display   描画先
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static u8 WifiLogin_Message_GetSysFrame( WIFILOGIN_DISPLAY display )
{ 
  switch( display )
  { 
  case WIFILOGIN_DISPLAY_UP:
    return GFL_BG_FRAME2_M;

  case WIFILOGIN_DISPLAY_DOWN:
    return GFL_BG_FRAME2_S;

  default:
    GF_ASSERT( 0);
    return 0;
  }
}

//------------------------------------------------------------------
/**
 * $brief   yesnoウインドウを出す
 * @param   msg_index
 * @retval  int       int friend = GFL_NET_DWC_GetFriendIndex();

 */
//------------------------------------------------------------------
#define	FLD_YESNO_WIN_PX	( 24 )
#define	FLD_YESNO_WIN_PY	( 13 )
static const BMPWIN_YESNO_DAT _yesNoBmpDat = {
  GFL_BG_FRAME2_M, FLD_YESNO_WIN_PX, FLD_YESNO_WIN_PY,
  _BUTTON_MSG_PAL, 0
};


static BMPMENU_WORK * WIFILOGIN_MESSAGE_YesNoWinCreate(WIFILOGIN_MESSAGE_WORK* pWork)
{
  BMPWIN_YESNO_DAT  dat = _yesNoBmpDat;
  dat.chrnum  = GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2S);
  return BmpMenu_YesNoSelectInit( &_yesNoBmpDat, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2S),
                             _BUTTON_WIN_PAL, 0, pWork->heapID );
}
