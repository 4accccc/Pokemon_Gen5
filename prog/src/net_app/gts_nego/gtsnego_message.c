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
  u32 bgchar;  //GFL_ARCUTIL_TRANSINFO
  u32 bgchar2S;  //SystemMsg
  u32 bgchar1M; //info

  GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
  GFL_BUTTON_MAN* pButton;

  GFL_MSGDATA *pMsgData;  //

  WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
  GFL_FONT* pFontHandle;
  STRBUF* pStrBuf;
  int msgidx[_MESSAGE_INDEX_MAX];

  GFL_BMPWIN* infoDispWin;
  GFL_BMPWIN* systemDispWin;
  GFL_BMPWIN* mainDispWin[_BMP_WINDOW_NUM];

  PRINT_STREAM* pStream;
	GFL_TCBLSYS *pMsgTcblSys;
  PRINT_QUE*            SysMsgQue;

  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
	APP_TASKMENU_RES* pAppTaskRes;
  
//  int windowNum;
  HEAPID heapID;
  
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
  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msg_dat, pWork->heapID );

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
  
  return pWork;
}

void GTSNEGO_MESSAGE_Main(GTSNEGO_MESSAGE_WORK* pWork)
{
  GFL_TCBL_Main( pWork->pMsgTcblSys );
  PRINTSYS_QUE_Main(pWork->SysMsgQue);

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

void GTSNEGO_MESSAGE_InfoMessageDisp(GTSNEGO_MESSAGE_WORK* pWork,int msgid)
{
  GFL_BMPWIN* pwin;

  GFL_MSG_GetString( pWork->pMsgData, msgid, pWork->pStrBuf );
  
  if(pWork->infoDispWin==NULL){
    pWork->infoDispWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME1_M ,
      1 , 19, 30 ,4 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->infoDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pStrBuf, pWork->pFontHandle,
                                        MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);

  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar1M), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
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
  BmpWinFrame_Clear(pWork->infoDispWin, WINDOW_TRANS_OFF);
  GFL_BMPWIN_ClearScreen(pWork->infoDispWin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_M);
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
  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , -8 );
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

static int GTSNEGO_MESSAGE_Disp(GTSNEGO_MESSAGE_WORK* pWork,int msgid,int x,int y)
{
  GFL_BMPWIN* pwin;
  int i;

  for(i=0;i<_BMP_WINDOW_NUM;i++){
    if(pWork->mainDispWin[i]==NULL){
      pWork->mainDispWin[i] = GFL_BMPWIN_Create(
        GFL_BG_FRAME2_S , x , y, 20 , 2 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );

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
      BmpWinFrame_Clear(pWork->mainDispWin[i], WINDOW_TRANS_OFF);
      GFL_BMPWIN_ClearScreen(pWork->mainDispWin[i]);
      GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_S);
    }
  }
}





#define _BUTTON_ANYONE_X (9)
#define _BUTTON_ANYONE_Y (7)

#define _BUTTON_FRIEND_X (9)
#define _BUTTON_FRIEND_Y (13)

#define _BUTTON_TYPE_WIDTH (13*8)
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

  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_005, _BUTTON_ANYONE_X, _BUTTON_ANYONE_Y);
  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_006, _BUTTON_FRIEND_X, _BUTTON_FRIEND_Y);

  GTSNEGO_MESSAGE_DispTransReq(pWork);

  _ButtonSafeDelete(pWork);
	pWork->pButton = GFL_BMN_Create( _AnyoneOrFriendBtnTbl, callback, pParentWork,  pWork->heapID );
}






static const GFL_UI_TP_HITTBL _levelselectBtnTbl[] = {
  {	_ARROW_LEVEL_YU*8-8,   _ARROW_LEVEL_YU*8 + 8 ,
    _ARROW_LEVEL_XU*8-8 ,  _ARROW_LEVEL_XU*8 + 8  },
  {	_ARROW_LEVEL_YD*8-8,   _ARROW_LEVEL_YD*8 + 8 ,
    _ARROW_LEVEL_XD*8-8 ,  _ARROW_LEVEL_XD*8 + 8  },

  {	_ARROW_MY_YU*8-8,   _ARROW_MY_YU*8 + 8 ,
    _ARROW_MY_XU*8-8 ,  _ARROW_MY_XU*8 + 8  },
  {	_ARROW_MY_YD*8-8,   _ARROW_MY_YD*8 + 8 ,
    _ARROW_MY_XD*8-8 ,  _ARROW_MY_XD*8 + 8  },

  {	_ARROW_FRIEND_YU*8-8,   _ARROW_FRIEND_YU*8 + 8 ,
    _ARROW_FRIEND_XU*8-8 ,  _ARROW_FRIEND_XU*8 + 8  },
  {	_ARROW_FRIEND_YD*8-8,   _ARROW_FRIEND_YD*8 + 8 ,
    _ARROW_FRIEND_XD*8-8 ,  _ARROW_FRIEND_XD*8 + 8  },

  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};



//------------------------------------------------------------------------------
/**
 * @brief   レベルの条件選択
 * @retval  none
 */
//------------------------------------------------------------------------------

void GTSNEGO_MESSAGE_DispLevel(GTSNEGO_MESSAGE_WORK* pWork,pBmnCallBackFunc callback,void* pParentWork)
{

  GTSNEGO_MESSAGE_DispInit(pWork);

  GFL_FONTSYS_SetColor(15, 14, 0);
  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_007, 0, 0);

  GFL_FONTSYS_SetColor(15, 14, 0);
  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_008, 3, 6);
  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_028, 1, 11);
  GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_029, 1, 16);

  GFL_FONTSYS_SetColor(1, 2, 0);
  pWork->msgidx[_MESSAGE_LEVEL] = GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_012,  13,  6);
  pWork->msgidx[_MESSAGE_MY] = GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_017,     16, 11);
  pWork->msgidx[_MESSAGE_FRIEND] = GTSNEGO_MESSAGE_Disp(pWork, GTSNEGO_017, 16, 16);

  GTSNEGO_MESSAGE_DispTransReq(pWork);

  _ButtonSafeDelete(pWork);
	pWork->pButton = GFL_BMN_Create( _levelselectBtnTbl, callback, pParentWork,  pWork->heapID );
}


void GTSNEGO_MESSAGE_DispLevelChange(GTSNEGO_MESSAGE_WORK* pWork,int no)
{
  u16 msg[]={GTSNEGO_012,GTSNEGO_009,GTSNEGO_010,GTSNEGO_011};
  
  GTSNEGO_MESSAGE_DispMsgChange(pWork, msg[no],  pWork->msgidx[_MESSAGE_LEVEL]);
}

void GTSNEGO_MESSAGE_DispMyChange(GTSNEGO_MESSAGE_WORK* pWork,int no)
{
  u16 msg[]={GTSNEGO_017,GTSNEGO_013,GTSNEGO_014,GTSNEGO_015,GTSNEGO_016};
  
  GTSNEGO_MESSAGE_DispMsgChange(pWork, msg[no],  pWork->msgidx[_MESSAGE_MY]);
}

void GTSNEGO_MESSAGE_DispFriendChange(GTSNEGO_MESSAGE_WORK* pWork,int no)
{
  u16 msg[]={GTSNEGO_017,GTSNEGO_013,GTSNEGO_014,GTSNEGO_015,GTSNEGO_016};
  
  GTSNEGO_MESSAGE_DispMsgChange(pWork, msg[no],  pWork->msgidx[_MESSAGE_FRIEND]);
}


void GTSNEGO_MESSAGE_DeleteDispLevel(GTSNEGO_MESSAGE_WORK* pWork)
{
  _ButtonSafeDelete(pWork);
}




APP_TASKMENU_WIN_WORK* GTSNEGO_MESSAGE_SearchButtonStart(GTSNEGO_MESSAGE_WORK* pWork,int msgno)
{
#if 0
  int i;
  APP_TASKMENU_INITWORK appinit;
  APP_TASKMENU_WORK* pAppTask;

  appinit.heapId = pWork->heapID;
  appinit.itemNum =  1;
  appinit.itemWork =  &pWork->appitem[0];
  appinit.charPosX = 18;
  appinit.charPosY = 24;
  appinit.posType = ATPT_RIGHT_DOWN;
	appinit.w				 = APP_TASKMENU_PLATE_WIDTH;
	appinit.h				 = APP_TASKMENU_PLATE_HEIGHT;

  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, msgno, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pAppTask = APP_TASKMENU_OpenMenu(&appinit,pWork->pAppTaskRes);
  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
#endif
  APP_TASKMENU_WIN_WORK* pAppWin;


  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, msgno, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  pWork->appitem[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  pAppWin =APP_TASKMENU_WIN_Create( pWork->pAppTaskRes,
                                           pWork->appitem, 16-4, 24-3, 10, pWork->heapID);

  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
  
  return pAppWin;
}



//----------------------------------------------------------------------------
/**
 *	@brief	フレンド選択初期化
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_MESSAGE_FriendListPlateDisp(GTSNEGO_MESSAGE_WORK* pMessageWork)
{
  int i, j, count;
  MYSTATUS* pMyStatus;
#if 0  

  count = WIFI_NEGOTIATION_SV_GetCount();

  for(i = 0 ; i < 4; i++){
    j = count - 1 - i;
    pMyStatus = WIFI_NEGOTIATION_SV_GetMyStatus(GAMEDATA_GetWifiNegotiation(pWork->pGameData),
                                                j % WIFI_NEGOTIATION_DATAMAX);
    if(pMyStatus){
        GTSNEGO_MESSAGE_PlateDisp(pMessageWork, pMyStatus, i);
    }
  }
#endif

}




