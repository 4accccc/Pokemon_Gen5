//=============================================================================
/**
 * @file	ircbattlemenu.c
 * @bfief	赤外線通信から無線に変わってバトルを行うメニュー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	09/02/19
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"

#include "ircbattlemenu.h"
#include "ircbattlematch.h"
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
#include "system/time_icon.h"
#include "system/palanm.h"
#include "sound/pm_sndsys.h"

#include "msg/msg_ircbattle.h"
#include "../../field/event_ircbattle.h"
#include "ircbattle.naix"
//#include "ircbattlecommon.h"
#include "cg_comm.naix"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK
#include "ir_ani_NANR_LBLDEFS.h"
#include "savedata/battle_box_save.h"
#include "app/app_keycursor.h"  //APP_TASKMENU_INITWORK
#include "net/wih_dwc.h"


#if DEBUG_ONLY_FOR_ohno
#define _NET_DEBUG (1)
#else
#define _NET_DEBUG (0)
#endif
#define _WORK_HEAPSIZE (0x1000)  // 調整が必要

// サウンドSE
#define _SE_DESIDE (SEQ_SE_SYS_69)
#define _SE_CANCEL (SEQ_SE_SYS_70)

FS_EXTERN_OVERLAY(ircbattlematch);

//--------------------------------------------
// 画面構成定義
//--------------------------------------------
#define _WINDOW_MAXNUM (4)   //ウインドウのパターン数

#define _MESSAGE_BUF_NUM	( 100*2 )

#define _BUTTON_MSG_PAL2   (8)  // メッセージフォント
#define _SUBLIST_NORMAL_PAL   (9)   //サブメニューの通常パレット
#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (3)    // ウインドウ高さ
#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (11)  // メッセージフォント

//BG面とパレット番号
#define _SUBSCREEN_BGPLANE	(GFL_BG_FRAME3_S)
#define _SUBSCREEN_PALLET	(0xE)



#include "ircbattlemenu.cdat"



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

#define _PALETTE_CHANGE_NUM (8)  //CGEARボタンに使っているパレット


typedef enum
{
  PLT_DS,
  PLT_OBJ,
  PLT_RESOURCE_MAX,
  CHAR_DS = PLT_RESOURCE_MAX,
  CHAR_OBJ,
  CHAR_RESOURCE_MAX,
  ANM_DS = CHAR_RESOURCE_MAX,
  ANM_OBJ,
  ANM_RESOURCE_MAX,
  CEL_RESOURCE_MAX,
} _CELL_RESOURCE_TYPE;

typedef enum
{
  CELL_IRWAVE1,
  CELL_IRWAVE2,
  _CELL_DISP_NUM,
} _CELL_WK_ENUM;



//--------------------------------------------
// 内部ワーク
//--------------------------------------------

enum _BATTLETYPE_SELECT {
  _SELECTBT_SINGLE = 0,
  _SELECTBT_DOUBLE,
  _SELECTBT_TRI,
  _SELECTBT_ROTATE,
  _SELECTBT_EXIT,
   _SELECTBT_MAX,
};

enum _TEMOTIORBOXTYPE_SELECT {
  _SELECTPOKE_BOX = 0,
  _SELECTPOKE_TEMOTI,
  _SELECTPOKE_EXIT,
  _SELECTPOKE_MAX,
};


enum _IBMODE_SELECT {
  _SELECTMODE_BATTLE = 0,
  _SELECTMODE_POKE_CHANGE,
	_SELECTMODE_COMPATIBLE,	//相性チェック
  _SELECTMODE_FRIENDCODE,
  _SELECTMODE_EXIT,
  _SELECTMODE_MAX,
  _SELECTMODE_BATTLE2 = _SELECTMODE_MAX,
  _SELECTMODE_POKE_CHANGE2,
  _SELECTMODE_FRIENDCODE2,
};

enum _IBMODE_ENTRY {
  _ENTRYNUM_DOUBLE = 0,
  _ENTRYNUM_FOUR,
  _ENTRYNUM_EXIT,
   _ENTRYNUM_MAX,
};

enum _IBMODE_CHANGE {
	_CHANGE_TRADE=0,
  _CHANGE_FRIENDCHANGE,
  _CHANGE_EXIT,
};


#define _SUBMENU_LISTMAX (2)


typedef void (StateFunc)(IRC_BATTLE_MENU* pState);
typedef BOOL (TouchFunc)(int no, IRC_BATTLE_MENU* pState);


struct _IRC_BATTLE_MENU {
//  IRC_BG_WORK aIrcBgWork;
  StateFunc* state;      ///< ハンドルのプログラム状態
  TouchFunc* touch;
  int selectType;   // 接続タイプ
  int bBattelBox;  //バトルボックスを選択
  HEAPID heapID;
  GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
  GFL_BUTTON_MAN* pButton;
  GFL_MSGDATA *pMsgData;  //
  GFL_FONT* pFontHandle;
  GAMEDATA* pGameData;
	STRBUF*  pExpStrBuf;
  STRBUF* pStrBuf;
  u32 bgchar;  //GFL_ARCUTIL_TRANSINFO
	u32 subchar;
	u32 mainchar;
  //    BMPWINFRAME_AREAMANAGER_POS aPos;

  u8 BackupPalette[16 * _PALETTE_CHANGE_NUM *2];
  u8 LightPalette[16 * _PALETTE_CHANGE_NUM *2];
  u16 TransPalette[16 ];

  GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル
  GFL_CLUNIT	*cellUnit;
  u32 cellRes[CEL_RESOURCE_MAX];
  GFL_CLWK* curIcon[_CELL_DISP_NUM];
  GFL_CLWK* buttonObj[_SELECTMODE_MAX];
  APP_KEYCURSOR_WORK* pKeyCursor;
  
  int yoffset;
  GFL_BMPWIN* infoDispWin;
  PRINT_STREAM* pStream;
	GFL_TCBLSYS *pMsgTcblSys;
  PRINT_QUE*            SysMsgQue;
  APP_TASKMENU_WORK* pAppTask;
  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
	APP_TASKMENU_RES* pAppTaskRes;
  APP_TASKMENU_WIN_WORK* pAppWin;
  EVENT_IRCBATTLE_WORK * dbw;
  TIMEICON_WORK* pTimeIcon;
  void* pVramOBJ;
  void* pVramBG;
  
  int windowNum;
  BOOL IsIrc;

//  GAMEDATA* gamedata;
//  GAMESYS_WORK *gsys;

  int anmCnt;  //決定時アニメカウント
  int bttnid;
  u16 anmCos;
  u8 endStart;
  u8 dummy;

};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(IRC_BATTLE_MENU* pWork,StateFunc* state);
static void _changeStateDebug(IRC_BATTLE_MENU* pWork,StateFunc* state, int line);
static void _buttonWindowCreate(int num,int* pMsgBuff,IRC_BATTLE_MENU* pWork, _WINDOWPOS* pos);
static void _modeSelectMenuInit(IRC_BATTLE_MENU* pWork);
static void _modeSelectMenuWait(IRC_BATTLE_MENU* pWork);
static void _modeSelectEntryNumInit(IRC_BATTLE_MENU* pWork);
static void _modeSelectEntryNumWait(IRC_BATTLE_MENU* pWork);
static void _modeReportInit(IRC_BATTLE_MENU* pWork);
static void _modeReportWait(IRC_BATTLE_MENU* pWork);
static BOOL _modeSelectMenuButtonCallback(int bttnid,IRC_BATTLE_MENU* pWork);
static BOOL _modeSelectEntryNumButtonCallback(int bttnid,IRC_BATTLE_MENU* pWork);
static BOOL _modeSelectBattleTypeButtonCallback(int bttnid,IRC_BATTLE_MENU* pWork);
static void _modeSelectBattleTypeInit(IRC_BATTLE_MENU* pWork);
static void _buttonWindowDelete(IRC_BATTLE_MENU* pWork);
static void _touchScreenChange(IRC_BATTLE_MENU* pWork,int no);
static void _ReturnButtonStart(IRC_BATTLE_MENU* pWork);
static void _CreateButtonObj4(IRC_BATTLE_MENU* pWork);
static void _CreateButtonObj2(IRC_BATTLE_MENU* pWork);
static void _CreateButtonObj3(IRC_BATTLE_MENU* pWork);
static void _CreateButtonObj(IRC_BATTLE_MENU* pWork);
static BOOL _infoMessageEndCheck(IRC_BATTLE_MENU* pWork);
static void _infoMessageDisp(IRC_BATTLE_MENU* pWork, const BOOL allput);
static void _hitAnyKeyWaitMode(IRC_BATTLE_MENU* pWork);
static void _modeTemotiOrBoxInit(IRC_BATTLE_MENU* pWork);



#ifdef _NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG




//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(IRC_BATTLE_MENU* pWork,StateFunc state)
{
  pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(IRC_BATTLE_MENU* pWork,StateFunc state, int line)
{
  NET_PRINT("ircbtl: %d\n",line);
  _changeState(pWork, state);
}
#endif



static void _createSubBg(IRC_BATTLE_MENU* pWork)
{

  // 背景面
  {
    int frame = GFL_BG_FRAME0_M;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000,
      0x8000,
      GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
//    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_ClearFrame( frame );
    GFL_BG_LoadScreenReq( frame );
		GFL_BG_SetVisible( frame, VISIBLE_ON );

  }
  {
    int frame = GFL_BG_FRAME1_M;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000,
      0x8000,
      GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( frame );
    GFL_BG_LoadScreenReq( frame );
		GFL_BG_SetVisible( frame, VISIBLE_ON );

  }

  {
    int frame = GFL_BG_FRAME0_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME1_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME2_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xc800, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME3_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
}


#if 1


static void _PaletteFadeSingle(IRC_BATTLE_MENU* pWork, int type, int palettenum)
{
  u32 addr;
  PALETTE_FADE_PTR pP = PaletteFadeInit(pWork->heapID);
  PaletteFadeWorkAllocSet(pP, type, 16 * 32, pWork->heapID);
  PaletteWorkSet_VramCopy( pP, type, 0, palettenum*32);
  SoftFadePfd(pP, type, 0, 16 * palettenum, 6, 0);
  addr = (u32)PaletteWorkTransWorkGet( pP, type );

  switch(type){
  case FADE_SUB_OBJ:
    GXS_LoadOBJPltt((void*)addr, 0, palettenum * 32);
    break;
  case FADE_SUB_BG:
    GXS_LoadBGPltt((void*)addr, 0, palettenum * 32);
    break;
  }
  PaletteFadeWorkAllocFree(pP,type);
  PaletteFadeFree(pP);
}



static void _PaletteFade(IRC_BATTLE_MENU* pWork,BOOL bFade)
{
  u32 addr;

  if(bFade){
    {
      GFL_STD_MemCopy((void*)HW_DB_OBJ_PLTT, pWork->pVramOBJ, 16*2*16);
      GFL_STD_MemCopy((void*)HW_DB_BG_PLTT, pWork->pVramBG, 16*2*16);
      _PaletteFadeSingle( pWork,  FADE_SUB_OBJ, 14);
      _PaletteFadeSingle( pWork,  FADE_SUB_BG, 9);
    }
  }
  else{
    GXS_LoadOBJPltt((void*)pWork->pVramOBJ, 0, 14 * 32);
    GXS_LoadBGPltt((void*)pWork->pVramBG, 0, 9 * 32);
  }

}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   タイムアイコンを出す
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _MESSAGE_WindowTimeIconStart(IRC_BATTLE_MENU* pWork)
{
  if(pWork->pTimeIcon){
    TIMEICON_Exit(pWork->pTimeIcon);
    pWork->pTimeIcon=NULL;
  }
  pWork->pTimeIcon =
    TIMEICON_CreateTcbl(pWork->pMsgTcblSys,pWork->infoDispWin, 15, TIMEICON_DEFAULT_WAIT, pWork->heapID);
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージの画面消去
 * @retval  none
 */
//-----------------------------------------------------------------------------

static void _infoMessageDispClear(IRC_BATTLE_MENU* pWork)
{
  if(pWork->pTimeIcon){
    TIMEICON_Exit(pWork->pTimeIcon);
    pWork->pTimeIcon=NULL;
  }
  BmpWinFrame_Clear(pWork->infoDispWin, WINDOW_TRANS_OFF);
  GFL_BMPWIN_ClearScreen(pWork->infoDispWin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _infoMessageEnd(IRC_BATTLE_MENU* pWork)
{
  _infoMessageDispClear(pWork);
  GFL_BMPWIN_Delete(pWork->infoDispWin);
  pWork->infoDispWin=NULL;
}

//------------------------------------------------------------------------------
/**
 * @brief   ボタンを点滅させる＆点滅し終わったらTRUEを返す
 * @retval  点滅し終わったらTRUE
 */
//------------------------------------------------------------------------------

static BOOL _flashDispAndCheck(IRC_BATTLE_MENU* pWork,u8* btnplt)
{
    //決定時アニメ
  int pltNo = btnplt[pWork->bttnid];
  const u8 isBlink = (pWork->anmCnt/APP_TASKMENU_ANM_INTERVAL)%2;
  if( isBlink == 0 )
  {
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                        pltNo * 32 ,
                                        &pWork->BackupPalette[32*pltNo] , 32 );
  }
  else
  {
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                        pltNo * 32 ,
                                        &pWork->LightPalette[32*pltNo] , 32 );
  }
  pWork->anmCnt++;

  if( pWork->anmCnt >= APP_TASKMENU_ANM_CNT )
  {
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                        pltNo * 32 ,
                                        &pWork->BackupPalette[32*pltNo] , 32 );
    pWork->anmCnt=0;
//    APP_TASKMENU_WIN_Delete( pWork->pAppWin );
//    pWork->pAppWin = NULL;
    return TRUE;
  }
  return FALSE;
}



//------------------------------------------------------------------------------
/**
 * @brief   受け取った数のウインドウを等間隔に作る 幅は3char
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _buttonWindowCreate(int num,int* pMsgBuff,IRC_BATTLE_MENU* pWork, _WINDOWPOS* pos)
{
  int i;
  u32 cgx;
  int frame = GFL_BG_FRAME3_S;

 // GFL_BG_SetVisible(frame,VISIBLE_OFF);
  GFL_BG_ClearScreen(frame);
  
  pWork->windowNum = num;
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL2, 0x20, pWork->heapID);

  for(i = 0;i < _WINDOW_MAXNUM;i++){
		if(pWork->buttonWin[i]){
			GFL_BMPWIN_ClearScreen(pWork->buttonWin[i]);
			BmpWinFrame_Clear(pWork->buttonWin[i], WINDOW_TRANS_OFF);
			GFL_BMPWIN_Delete(pWork->buttonWin[i]);
		}
    pWork->buttonWin[i] = NULL;
  }

  for(i=0;i < num;i++){
    GFL_FONTSYS_SetColor(0xf,0x2,0);

    pWork->buttonWin[i] = GFL_BMPWIN_Create(
      frame,
      pos[i].leftx, pos[i].lefty,
      pos[i].width, pos[i].height,
      _BUTTON_MSG_PAL2,GFL_BMP_CHRAREA_GET_F);

    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0 );
    // システムウインドウ枠描画
    GFL_MSG_GetString(  pWork->pMsgData, pMsgBuff[i], pWork->pStrBuf );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0, 0, pWork->pStrBuf, pWork->pFontHandle);

    GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
    GFL_BMPWIN_MakeScreen(pWork->buttonWin[i]);
  }

	if(pWork->pButton){
		GFL_BMN_Delete(pWork->pButton);
	}
  pWork->pButton = NULL;
//  GFL_BG_LoadScreenV_Req(frame);
  GFL_BG_LoadScreenV_Req(frame);
  GFL_BG_SetVisible(frame,VISIBLE_ON);
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンイベントコールバック
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _buttonWindowDelete(IRC_BATTLE_MENU* pWork)
{
  int i;

	if(pWork->pButton){
		GFL_BMN_Delete(pWork->pButton);
	}
  pWork->pButton = NULL;
  for(i=0;i < _WINDOW_MAXNUM;i++){
		if(pWork->buttonWin[i]){
			GFL_BMPWIN_ClearScreen(pWork->buttonWin[i]);
			GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
			BmpWinFrame_Clear(pWork->buttonWin[i], WINDOW_TRANS_OFF);
			
//			GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0 );
//			GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
			GFL_BMPWIN_Delete(pWork->buttonWin[i]);
		}
    pWork->buttonWin[i] = NULL;
  }
  pWork->windowNum = 0;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ボタンイベントコールバック
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void _BttnCallBack( u32 bttnid, u32 event, void* p_work )
{
  IRC_BATTLE_MENU *pWork = p_work;
  u32 friendNo;

  switch( event ){
  case GFL_BMN_EVENT_TOUCH:		///< 触れた瞬間
    if(pWork->touch!=NULL){
      if(pWork->touch(bttnid, pWork)){
        return;
      }
    }
    break;

  default:
    break;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   渡されたテーブルを元に明るいパレットを作る
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _lightPaletteMake(u16* pal, u16* PaletteTable, int num)
{
  int i,j;
  u16 palr,palg,palb;
  
  for(i =0 ;i< num;i++){
    for(j=0;j<16;j++){
      palr = pal[i*16+j] & 0x001f;
      palg = (pal[i*16+j] & 0x03e0) >> 5;
      palb = (pal[i*16+j] & 0x7c00) >> 10;
      palr += PaletteTable[j];
      palg += PaletteTable[j];
      palb += PaletteTable[j];
      if(palr>0x1f){
        palr=0x1f;
      }
      if(palg>0x1f){
        palg=0x1f;
      }
      if(palb>0x1f){
        palb=0x1f;
      }
      pal[i*16+j] = palr + (palg<<5) + (palb<<10);
    }
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeInit(IRC_BATTLE_MENU* pWork)
{
  pWork->IsIrc=FALSE;

  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_ircbattle_dat, pWork->heapID );

	{
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCBATTLE, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_ircbattle_ir_demo_NCLR,
                                      PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);
    // サブ画面BG0キャラ転送
    pWork->mainchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_ircbattle_ir_demo_NCGR,
                                                                  GFL_BG_FRAME0_M, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_ircbattle_ir_demo1_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);
    GFL_ARC_CloseDataHandle(p_handle);
  }

  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_CG_COMM, pWork->heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_cg_comm_comm_bg_ir_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  pWork->heapID);

    {
      NNSG2dPaletteData* pPal;
      void* pData;

      pData = GFL_ARCHDL_UTIL_LoadPalette( p_handle, NARC_cg_comm_comm_bg_ir_NCLR, &pPal,  pWork->heapID);
      GFL_STD_MemCopy(pPal->pRawData, pWork->BackupPalette, _PALETTE_CHANGE_NUM*32);
      GFL_STD_MemCopy(pPal->pRawData, pWork->LightPalette, _PALETTE_CHANGE_NUM*32);
      _lightPaletteMake((u16*)pWork->LightPalette, Btn_PaletteTable, _PALETTE_CHANGE_NUM);

      GFL_HEAP_FreeMemory(pData);
    }

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_cg_comm_background_NCLR,
                                      PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);

    // サブ画面BG0キャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_cg_comm_comm_bg_NCGR,
                                                                  GFL_BG_FRAME0_S, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_cg_comm_comm_base_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_cg_comm_comm_ir_btn_NSCR,
                                              GFL_BG_FRAME2_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);
		GFL_ARC_CloseDataHandle(p_handle);
	}

  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME1_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
	
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
																0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);


}

//------------------------------------------------------------------------------
/**
 * @brief   文字表示パネル変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _touchScreenChange(IRC_BATTLE_MENU* pWork,int no)
{
  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_CG_COMM, pWork->heapID );
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, no,
                                            GFL_BG_FRAME2_S, 0,
                                            GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                            pWork->heapID);
		GFL_ARC_CloseDataHandle(p_handle);
  }
  GFL_BG_SetVisible(GFL_BG_FRAME2_S,VISIBLE_ON);
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuInit(IRC_BATTLE_MENU* pWork)
{
	int aMsgBuff[]={IRCBTL_STR_01,IRCBTL_STR_02,IRCBTL_STR_15,IRCBTL_STR_14};

  _touchScreenChange( pWork, NARC_cg_comm_comm_ir_btn_NSCR);
  _CreateButtonObj(pWork);
	_buttonWindowCreate(NELEMS(aMsgBuff), aMsgBuff, pWork, wind_irmain);

	pWork->pButton = GFL_BMN_Create( btn_irmain, _BttnCallBack, pWork,  pWork->heapID );
	pWork->touch = &_modeSelectMenuButtonCallback;

  _ReturnButtonStart(pWork);
  

	_CHANGE_STATE(pWork,_modeSelectMenuWait);

}

static void _workEnd(IRC_BATTLE_MENU* pWork)
{
  GFL_FONTSYS_SetDefaultColor();

	_buttonWindowDelete(pWork);
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_S, 1, 0);
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
  GFL_MSG_Delete( pWork->pMsgData );
  GFL_FONT_Delete(pWork->pFontHandle);
  GFL_STR_DeleteBuffer(pWork->pStrBuf);
  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );

}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeout(IRC_BATTLE_MENU* pWork)
{
	if(WIPE_SYS_EndCheck()){
    if(pWork->selectType !=  EVENTIRCBTL_ENTRYMODE_EXIT){
      if(GAME_COMM_NO_NULL == GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(IrcBattle_GetGAMESYS_WORK(pWork->dbw)))){
        _CHANGE_STATE(pWork, NULL);        // 終わり
      }
    }
    else{
        _CHANGE_STATE(pWork, NULL);        // 終わり
    }
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeoutStart(IRC_BATTLE_MENU* pWork)
{
  pWork->endStart=TRUE;
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

  if(pWork->selectType !=  EVENTIRCBTL_ENTRYMODE_EXIT){
    if(GAME_COMM_NO_NULL!= GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(IrcBattle_GetGAMESYS_WORK(pWork->dbw)))){
      GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(IrcBattle_GetGAMESYS_WORK(pWork->dbw)));
    }
  }
  _CHANGE_STATE(pWork, _modeFadeout);        // 終わり
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _modeAppWinFlashCallback(u32 param, fx32 currentFrame )
{
  IRC_BATTLE_MENU* pWork = (IRC_BATTLE_MENU*)param;
  {
    if(pWork->selectType ==  EVENTIRCBTL_ENTRYMODE_EXIT){
      _CHANGE_STATE(pWork, _modeFadeoutStart);
    }
    else if(WIRELESSSAVE_ON == CONFIG_GetWirelessSaveMode(SaveData_GetConfig(IrcBattle_GetSAVE_CONTROL_WORK(pWork->dbw)))){
      _CHANGE_STATE(pWork, _modeReportInit);
    }
    else if(pWork->selectType == EVENTIRCBTL_ENTRYMODE_TRADE ||
            pWork->selectType == EVENTIRCBTL_ENTRYMODE_COMPATIBLE ||
            pWork->selectType == EVENTIRCBTL_ENTRYMODE_FRIEND ){
      _CHANGE_STATE(pWork, _modeFadeoutStart);
    }
    else{
      _CHANGE_STATE(pWork, _modeSelectEntryNumInit);        // 人数確認
    }
  }
}

static void _modeButtonFlash2(IRC_BATTLE_MENU* pWork)
{
}



//------------------------------------------------------------------------------
/**
 * @brief   タッチした際に画面が点滅
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeButtonFlash(IRC_BATTLE_MENU* pWork)
{
  GFL_CLWK_ANM_CALLBACK cbwk;
  int objno= pWork->bttnid;

  cbwk.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM ;  // CLWK_ANM_CALLBACK_TYPE
  cbwk.param = (u32)pWork;          // コールバックワーク
  cbwk.p_func = _modeAppWinFlashCallback; // コールバック関数

  if(_SELECTMODE_BATTLE2 == pWork->bttnid){
    objno = _SELECTMODE_BATTLE;
  }
  else if(_SELECTMODE_POKE_CHANGE2 == pWork->bttnid){
    objno = _SELECTMODE_POKE_CHANGE;
  }
  else if(_SELECTMODE_FRIENDCODE2 == pWork->bttnid){
    objno = _SELECTMODE_FRIENDCODE;
  }
  GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[objno],TRUE);
  GFL_CLACT_WK_StartAnmCallBack( pWork->buttonObj[objno], &cbwk );
  GFL_CLACT_WK_ResetAnm( pWork->buttonObj[objno] );
  
  _CHANGE_STATE(pWork,_modeButtonFlash2);

}


static void _modeKeyWait(IRC_BATTLE_MENU* pWork)
{
  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  if(GFL_UI_TP_GetTrg()){
    _infoMessageEnd(pWork);
    _CHANGE_STATE(pWork,_modeSelectMenuWait);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectMenuButtonCallback(int bttnid,IRC_BATTLE_MENU* pWork)
{
  BOOL ret = FALSE;

  pWork->bttnid=bttnid;
  switch( bttnid ){
  case _SELECTMODE_BATTLE:
  case _SELECTMODE_BATTLE2:
		PMSND_PlaySystemSE(_SE_DESIDE);
    _CHANGE_STATE(pWork,_modeButtonFlash);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_SINGLE;
    ret = TRUE;
    break;
  case _SELECTMODE_POKE_CHANGE:
  case _SELECTMODE_POKE_CHANGE2:
    {
      POKEPARTY* party = GAMEDATA_GetMyPokemon(pWork->pGameData);
      if(PokeParty_GetPokeCountNotEgg(party) < 2 ){
//      if(PokeParty_GetPokeCount(party) < 2 && 0 == BOXDAT_GetPokeExistCountTotal(GAMEDATA_GetBoxManager(pWork->pGameData))){
        GFL_MSG_GetString( pWork->pMsgData, IRCBTL_STR_45, pWork->pStrBuf );
        _infoMessageDisp(pWork, FALSE);
        _CHANGE_STATE(pWork,_modeKeyWait);
        break;
      }
    }
    PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_TRADE;
    _CHANGE_STATE(pWork,_modeButtonFlash);
    ret = TRUE;
    break;
	case _SELECTMODE_COMPATIBLE:
		PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_COMPATIBLE;
    _CHANGE_STATE(pWork,_modeButtonFlash);
    ret = TRUE;
    break;
  case _SELECTMODE_FRIENDCODE:
  case _SELECTMODE_FRIENDCODE2:  
		PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_FRIEND;
    _CHANGE_STATE(pWork,_modeButtonFlash);
    ret = TRUE;
    break;
  case _SELECTMODE_EXIT:
		PMSND_PlaySystemSE(_SE_CANCEL);
//    APP_TASKMENU_WIN_SetDecide(pWork->pAppWin, TRUE);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_EXIT;
    _CHANGE_STATE(pWork,_modeButtonFlash);        // 終わり
    break;
  default:
    break;
  }
  return ret;
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(IRC_BATTLE_MENU* pWork)
{
	if(WIPE_SYS_EndCheck()){
		GFL_BMN_Main( pWork->pButton );
	}
}






//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectEntryNumInit(IRC_BATTLE_MENU* pWork)
{
  int aMsgBuff[]={IRCBTL_STR_04,IRCBTL_STR_05};

  _CreateButtonObj2(pWork);

  _touchScreenChange( pWork, NARC_cg_comm_comm_vs1_btn_NSCR);

  _buttonWindowCreate(2, aMsgBuff, pWork, wind_irvs1);

  pWork->pButton = GFL_BMN_Create( btn_irvs1, _BttnCallBack, pWork,  pWork->heapID );
  pWork->touch = &_modeSelectEntryNumButtonCallback;

  _ReturnButtonStart(pWork);
  
  _CHANGE_STATE(pWork,_modeSelectEntryNumWait);

}





static void _modeAppWinFlashCallback2(u32 param, fx32 currentFrame )
{
  IRC_BATTLE_MENU* pWork = (IRC_BATTLE_MENU*)param;

  if(pWork->selectType == EVENTIRCBTL_ENTRYMODE_MULTH){
#if 1
    BATTLE_BOX_SAVE* bxsv;
    bxsv = BATTLE_BOX_SAVE_GetBattleBoxSave(pWork->dbw->ctrl);
    if(BATTLE_BOX_SAVE_IsIn(bxsv)){
      _CHANGE_STATE(pWork, _modeTemotiOrBoxInit);
    }
    else{
      _CHANGE_STATE(pWork, _modeFadeoutStart); //BBOXに何もなければ開始
    }
#else
    _CHANGE_STATE(pWork, _modeFadeoutStart);
#endif
  }
  else{
    _CHANGE_STATE(pWork, _modeSelectBattleTypeInit);        // バトルモード
  }
}


static void _modeSelectEntryButtonFlash2(IRC_BATTLE_MENU* pWork)
{
}




//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理 タッチした際に画面が点滅
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeSelectEntryButtonFlash(IRC_BATTLE_MENU* pWork)
{
  int i=0;
  GFL_CLWK_ANM_CALLBACK cbwk;

  if(pWork->selectType == EVENTIRCBTL_ENTRYMODE_MULTH){
    i=1;
  }

  
  cbwk.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM;  // CLWK_ANM_CALLBACK_TYPE
  cbwk.param = (u32)pWork;          // コールバックワーク
  cbwk.p_func = _modeAppWinFlashCallback2; // コールバック関数
  GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[i],TRUE);
  GFL_CLACT_WK_StartAnmCallBack( pWork->buttonObj[i], &cbwk );
  GFL_CLACT_WK_ResetAnm( pWork->buttonObj[i] );

  _CHANGE_STATE(pWork, _modeSelectEntryButtonFlash2);
}



static void _modeAppWinFlash4(u32 param, fx32 currentFrame )
{
  IRC_BATTLE_MENU* pWork = (IRC_BATTLE_MENU*)param;

  _CHANGE_STATE(pWork, _modeSelectMenuInit);
}

static void _modeAppWinFlash5(IRC_BATTLE_MENU* pWork)
{
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeAppWinFlash2(IRC_BATTLE_MENU* pWork)
{
  GFL_CLWK_ANM_CALLBACK cbwk;

  cbwk.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM ;  // CLWK_ANM_CALLBACK_TYPE
  cbwk.param = (u32)pWork;          // コールバックワーク
  cbwk.p_func = _modeAppWinFlash4; // コールバック関数
  GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[pWork->bttnid], TRUE);
  GFL_CLACT_WK_StartAnmCallBack( pWork->buttonObj[pWork->bttnid], &cbwk );
  GFL_CLACT_WK_ResetAnm( pWork->buttonObj[pWork->bttnid] );
  _CHANGE_STATE(pWork,_modeAppWinFlash5);
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectEntryNumButtonCallback(int bttnid,IRC_BATTLE_MENU* pWork)
{
  BOOL ret = TRUE;

  pWork->bttnid = bttnid;
  switch(bttnid){
  case _ENTRYNUM_DOUBLE:
		PMSND_PlaySystemSE(_SE_DESIDE);
    _CHANGE_STATE(pWork,_modeSelectEntryButtonFlash);
    ret = TRUE;
    break;
  case _ENTRYNUM_FOUR:
		PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_MULTH;
    _CHANGE_STATE(pWork,_modeSelectEntryButtonFlash);
    ret = TRUE;
    break;
  case _ENTRYNUM_EXIT:
		PMSND_PlaySystemSE(_SE_CANCEL);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_EXIT;
    _CHANGE_STATE(pWork,_modeAppWinFlash2);        // 終わり

    break;
  default:
    break;
  }
  return ret;
}


//------------------------------------------------------------------------------
/**
 * @brief   バトルの種類画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectBattleTypeInit(IRC_BATTLE_MENU* pWork)
{
  int aMsgBuff[]={IRCBTL_STR_06,IRCBTL_STR_07,IRCBTL_STR_08,IRCBTL_STR_32};

  _CreateButtonObj3(pWork);
  _touchScreenChange( pWork, NARC_cg_comm_comm_vs2_btn_NSCR);

  _buttonWindowCreate(4,aMsgBuff,pWork,wind_irvs2);

  pWork->pButton = GFL_BMN_Create( btn_irvs2, _BttnCallBack, pWork,  pWork->heapID );
  pWork->touch = &_modeSelectBattleTypeButtonCallback;

  _ReturnButtonStart(pWork);
  
  _CHANGE_STATE(pWork,_modeSelectEntryNumWait);

}







static void _modeTemotiOrBoxButtonCallback2(u32 param, fx32 currentFrame )
{
  IRC_BATTLE_MENU* pWork = (IRC_BATTLE_MENU*)param;

  if(pWork->bBattelBox == _SELECTPOKE_EXIT){
    if(pWork->selectType == EVENTIRCBTL_ENTRYMODE_MULTH){
      pWork->selectType = EVENTIRCBTL_ENTRYMODE_SINGLE;  //BTS3492
      _CHANGE_STATE(pWork, _modeSelectEntryNumInit);  //バトルモード
    }
    else
    {
      _CHANGE_STATE(pWork, _modeSelectBattleTypeInit);  //人数選択にもどる
    }
  }
  else{
    _CHANGE_STATE(pWork, _modeFadeoutStart);
  }
}


static void _modeTemotiOrBoxButtonFlash2(IRC_BATTLE_MENU* pWork)
{
}


//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理 タッチした際に画面が点滅
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeTemotiOrBoxButtonFlash(IRC_BATTLE_MENU* pWork)
{
int i;
  GFL_CLWK_ANM_CALLBACK cbwk;
  u8 buttonno[]={1,0,2};
  u8 bno = buttonno[pWork->bBattelBox];
  

  cbwk.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM ;  // CLWK_ANM_CALLBACK_TYPE
  cbwk.param = (u32)pWork;          // コールバックワーク
  cbwk.p_func = _modeTemotiOrBoxButtonCallback2; // コールバック関数
  GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[bno],TRUE);
  GFL_CLACT_WK_StartAnmCallBack( pWork->buttonObj[bno], &cbwk );
  GFL_CLACT_WK_ResetAnm( pWork->buttonObj[bno] );
  _CHANGE_STATE(pWork,_modeTemotiOrBoxButtonFlash2);
}


//------------------------------------------------------------------------------
/**
 * @brief   手持ちかボックスかをせんたく
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeTemotiOrBoxWait(IRC_BATTLE_MENU* pWork)
{

  GFL_BMN_Main( pWork->pButton );
}

//------------------------------------------------------------------------------
/**
 * @brief   手持ちかボックスかをせんたく
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeTemotiOrBoxButtonCallback(int bttnid,IRC_BATTLE_MENU* pWork)
{
  BOOL ret=FALSE;

  pWork->bttnid = bttnid;
    
  switch(bttnid){
  case _SELECTPOKE_TEMOTI:
		PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->bBattelBox = 0;
    _CHANGE_STATE(pWork,_modeTemotiOrBoxButtonFlash);
    ret=TRUE;
    break;
  case _SELECTPOKE_BOX:
		PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->bBattelBox = 1;
    _CHANGE_STATE(pWork,_modeTemotiOrBoxButtonFlash);
    ret=TRUE;
    break;
  case _SELECTPOKE_EXIT:
    PMSND_PlaySystemSE(_SE_CANCEL);
    pWork->bBattelBox = _SELECTPOKE_EXIT;
    _CHANGE_STATE(pWork,_modeTemotiOrBoxButtonFlash); 
  default:
    break;
  }
  return ret;
}




//------------------------------------------------------------------------------
/**
 * @brief   手持ちかボックスかをせんたく
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeTemotiOrBoxInit(IRC_BATTLE_MENU* pWork)
{
  int aMsgBuff[]={IRCBTL_STR_41,IRCBTL_STR_42};

  _CreateButtonObj4(pWork);
  _touchScreenChange( pWork, NARC_cg_comm_comm_vs2_btn_NSCR);

  _buttonWindowCreate(2,aMsgBuff,pWork,wind_irvs1);

  pWork->pButton = GFL_BMN_Create( btn_irvs1, _BttnCallBack, pWork,  pWork->heapID );
  pWork->touch = &_modeTemotiOrBoxButtonCallback;

  _ReturnButtonStart(pWork);
  
  _CHANGE_STATE(pWork,_modeTemotiOrBoxWait);

}


static void _modeSelectBattleTypeButtonCallback2(u32 param, fx32 currentFrame )
{
  IRC_BATTLE_MENU* pWork = (IRC_BATTLE_MENU*)param;
  BATTLE_BOX_SAVE* bxsv;

  if(pWork->selectType==EVENTIRCBTL_ENTRYMODE_EXIT){
    
    _CHANGE_STATE(pWork, _modeSelectEntryNumInit);  //人数選択にもどる
  }
  else{
    bxsv = BATTLE_BOX_SAVE_GetBattleBoxSave(pWork->dbw->ctrl);
    if(BATTLE_BOX_SAVE_IsIn(bxsv)){
      _CHANGE_STATE(pWork, _modeTemotiOrBoxInit);
    }
    else{
      _CHANGE_STATE(pWork, _modeFadeoutStart); //BBOXに何もなければ開始
    }
  }
}

static void _modeSelectBattleTypeButtonFlash2(IRC_BATTLE_MENU* pWork)
{
}


//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理 タッチした際に画面が点滅
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeSelectBattleTypeButtonFlash(IRC_BATTLE_MENU* pWork)
{
int i;
  GFL_CLWK_ANM_CALLBACK cbwk;

  switch( pWork->selectType)
  {
  case EVENTIRCBTL_ENTRYMODE_SINGLE:
    i = 0;
    break;
  case EVENTIRCBTL_ENTRYMODE_DOUBLE:
    i = 1;
    break;
  case EVENTIRCBTL_ENTRYMODE_TRI:
    i = 2;
    break;
  case EVENTIRCBTL_ENTRYMODE_ROTATE:
    i = 3;
    break;
  default:
    i = 4;
    break;
  }

  cbwk.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM ;  // CLWK_ANM_CALLBACK_TYPE
  cbwk.param = (u32)pWork;          // コールバックワーク
  cbwk.p_func = _modeSelectBattleTypeButtonCallback2; // コールバック関数
  GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[i],TRUE);
  GFL_CLACT_WK_StartAnmCallBack( pWork->buttonObj[i], &cbwk );
  GFL_CLACT_WK_ResetAnm( pWork->buttonObj[i] );
  _CHANGE_STATE(pWork,_modeSelectBattleTypeButtonFlash2);
}


//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeAppWinFlash3(IRC_BATTLE_MENU* pWork)
{
  if( _flashDispAndCheck( pWork , Btn_PalettePos) ){
    _CHANGE_STATE(pWork,_modeSelectEntryNumInit);        // ２人４人選択へ
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectBattleTypeButtonCallback(int bttnid,IRC_BATTLE_MENU* pWork)
{
  BOOL ret=FALSE;

  pWork->bttnid = bttnid;
    
  switch(bttnid){
  case _SELECTBT_SINGLE:
		PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_SINGLE;
    _CHANGE_STATE(pWork,_modeSelectBattleTypeButtonFlash);
    ret=TRUE;
    break;
  case _SELECTBT_DOUBLE:
		PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_DOUBLE;
    _CHANGE_STATE(pWork,_modeSelectBattleTypeButtonFlash);
    ret=TRUE;
    break;
  case _SELECTBT_TRI:
		PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_TRI;
    _CHANGE_STATE(pWork,_modeSelectBattleTypeButtonFlash);
    ret=TRUE;
    break;
  case _SELECTBT_ROTATE:
		PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_ROTATE;
    _CHANGE_STATE(pWork,_modeSelectBattleTypeButtonFlash);
    ret=TRUE;
    break;
  case _SELECTBT_EXIT:
    PMSND_PlaySystemSE(_SE_CANCEL);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_EXIT;
    _CHANGE_STATE(pWork,_modeSelectBattleTypeButtonFlash);        // 終わり

  default:
    break;
  }
  return ret;
}

//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectEntryNumWait(IRC_BATTLE_MENU* pWork)
{

  GFL_BMN_Main( pWork->pButton );
}



//----------------------------------------------------------------------------
/**
 *	@brief	波紋OBJ表示
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


static void _CLACT_SetResource(IRC_BATTLE_MENU* pWork)
{
  int i=0;
  {
/*   ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCBATTLE, pWork->heapID );
 * 
 *   pWork->cellRes[CHAR_DS] =
 *     GFL_CLGRP_CGR_Register( p_handle , NARC_ircbattle_ir_demo_ani_NCGR ,
 *                             FALSE , CLSYS_DRAW_MAIN , pWork->heapID );
 *   pWork->cellRes[PLT_DS] =
 *     GFL_CLGRP_PLTT_RegisterEx(
 *       p_handle ,NARC_ircbattle_ir_demo_obj_NCLR , CLSYS_DRAW_MAIN, 0, 0, 3, pWork->heapID  );
 *   pWork->cellRes[ANM_DS] =
 *     GFL_CLGRP_CELLANIM_Register(
 *       p_handle , NARC_ircbattle_ir_ani_NCER, NARC_ircbattle_ir_ani_NANR , pWork->heapID  );
 *   GFL_ARC_CloseDataHandle(p_handle);
 */
  }

  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_CG_COMM, pWork->heapID );
    pWork->cellRes[CHAR_OBJ] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_cg_comm_comm_btn_NCGR ,
                              FALSE , CLSYS_DRAW_SUB , pWork->heapID );
    pWork->cellRes[PLT_OBJ] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_cg_comm_comm_btn_NCLR , CLSYS_DRAW_SUB ,    0, 0, 10, pWork->heapID  );
    pWork->cellRes[ANM_OBJ] =
      GFL_CLGRP_CELLANIM_Register(
        p_handle , NARC_cg_comm_comm_btn_NCER, NARC_cg_comm_comm_btn_NANR , pWork->heapID  );
    
		GFL_ARC_CloseDataHandle(p_handle);
	}
}


static void _RemoveButtonObj(IRC_BATTLE_MENU* pWork)
{
  int i;
  
  for(i=0;i<_SELECTMODE_MAX;i++){
    if(pWork->buttonObj[i]){
      GFL_CLACT_WK_Remove(pWork->buttonObj[i]);
      pWork->buttonObj[i]=NULL;
    }
  }
}


//最初のボタン
static void _CreateButtonObj(IRC_BATTLE_MENU* pWork)
{
  int i;
  u8 buffx[]={ 128,    128,128,    128,     224};
  u8 buffy[]={ 192/2 , 192/2,192/2 , 192/2, 177};
  u8 buttonno[]={2,1,3,4, 0};

  _RemoveButtonObj(pWork);

  for(i=0;i<_SELECTMODE_MAX;i++){
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = buffx[i];
    cellInitData.pos_y = buffy[i];
    cellInitData.anmseq = buttonno[i];
    cellInitData.softpri = 0;
    cellInitData.bgpri = 1;
    pWork->buttonObj[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->cellRes[CHAR_OBJ],
                                               pWork->cellRes[PLT_OBJ],
                                               pWork->cellRes[ANM_OBJ],
                                               &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->buttonObj[i] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( pWork->buttonObj[i], TRUE );
    GFL_CLACT_WK_SetObjMode(pWork->buttonObj[i],GX_OAM_MODE_XLU);
  }
}



///人数モード選択
static void _CreateButtonObj2(IRC_BATTLE_MENU* pWork)
{
  int i;
  u8 buffx[]={ 128,    128,     224};
  u8 buffy[]={ 64+8 , 64+8+40+8 ,  177};
  u8 buttonno[]={9, 10, 0};

  _RemoveButtonObj(pWork);

  
  for(i=0;i<_ENTRYNUM_MAX;i++){
    GFL_CLWK_DATA cellInitData;

    cellInitData.pos_x = buffx[i];
    cellInitData.pos_y = buffy[i];
    cellInitData.anmseq = buttonno[i];
    cellInitData.softpri = 0;
    cellInitData.bgpri = 1;
    pWork->buttonObj[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->cellRes[CHAR_OBJ],
                                               pWork->cellRes[PLT_OBJ],
                                               pWork->cellRes[ANM_OBJ],
                                               &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->buttonObj[i] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( pWork->buttonObj[i], TRUE );
    GFL_CLACT_WK_SetObjMode(pWork->buttonObj[i],GX_OAM_MODE_XLU);
  }
}



///バトルのモード選択
static void _CreateButtonObj3(IRC_BATTLE_MENU* pWork)
{
  int i;
  u8 buffx[]={ 128,    128,  128,    128,     224};
  u8 buffy[]={ 64-40 , 64 , 64+40 , 64+40*2, 177};
  u8 buttonno[]={5, 6, 7, 8, 0};

  _RemoveButtonObj(pWork);
  
  for(i=0;i<_SELECTBT_MAX;i++){
    GFL_CLWK_DATA cellInitData;

    cellInitData.pos_x = buffx[i];
    cellInitData.pos_y = buffy[i];
    cellInitData.anmseq = buttonno[i];
    cellInitData.softpri = 0;
    cellInitData.bgpri = 1;
    pWork->buttonObj[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->cellRes[CHAR_OBJ],
                                               pWork->cellRes[PLT_OBJ],
                                               pWork->cellRes[ANM_OBJ],
                                               &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->buttonObj[i] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( pWork->buttonObj[i], TRUE );
    GFL_CLACT_WK_SetObjMode(pWork->buttonObj[i],GX_OAM_MODE_XLU);
  }
}


///手持ちボックスモード選択
static void _CreateButtonObj4(IRC_BATTLE_MENU* pWork)
{
  int i;
  u8 buffx[]={ 128,    128,     224};
  u8 buffy[]={ 64+8 , 64+8+40+8 ,  177};
  u8 buttonno[]={17, 17, 0};

  _RemoveButtonObj(pWork);

  
  for(i=0;i<_ENTRYNUM_MAX;i++){
    GFL_CLWK_DATA cellInitData;

    cellInitData.pos_x = buffx[i];
    cellInitData.pos_y = buffy[i];
    cellInitData.anmseq = buttonno[i];
    cellInitData.softpri = 0;
    cellInitData.bgpri = 1;
    pWork->buttonObj[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->cellRes[CHAR_OBJ],
                                               pWork->cellRes[PLT_OBJ],
                                               pWork->cellRes[ANM_OBJ],
                                               &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->buttonObj[i] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( pWork->buttonObj[i], TRUE );
    GFL_CLACT_WK_SetObjMode(pWork->buttonObj[i],GX_OAM_MODE_XLU);
  }
}



static void _CLACT_SetAnim(IRC_BATTLE_MENU* pWork,int x,int y,int no,int anm)
{  
/*   if(pWork->curIcon[no]==NULL){
 *     GFL_CLWK_DATA cellInitData;
 * 
 *     cellInitData.pos_x = x;
 *     cellInitData.pos_y = y;
 *     cellInitData.anmseq = anm;
 *     cellInitData.softpri = 0;
 *     cellInitData.bgpri = 1;
 *     pWork->curIcon[no] = GFL_CLACT_WK_Create( pWork->cellUnit ,
 *                                                          pWork->cellRes[CHAR_DS],
 *                                                          pWork->cellRes[PLT_DS],
 *                                                          pWork->cellRes[ANM_DS],
 *                                                          &cellInitData ,CLSYS_DRAW_MAIN , pWork->heapID );
 *     GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[no] , TRUE );
 *     GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[no], TRUE );
 *     GFL_CLACT_WK_SetAffineParam(pWork->curIcon[no], CLSYS_AFFINETYPE_DOUBLE);
 * 
 *   }
 */
}

//----------------------------------------------------------------------------
/**
 *	@brief	CLACT開放
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void _CLACT_Release(IRC_BATTLE_MENU* pWork)
{
  int i=0;

  for(i = 0 ; i < _SELECTMODE_MAX ;i++){
    if(pWork->buttonObj[i]!=NULL){
      GFL_CLACT_WK_Remove(pWork->buttonObj[i]);
      pWork->buttonObj[i]=NULL;
    }
  }
  for(i = 0 ; i < _CELL_DISP_NUM ;i++){
    if(pWork->curIcon[i]!=NULL){
      GFL_CLACT_WK_Remove(pWork->curIcon[i]);
      pWork->curIcon[i]=NULL;
    }
  }
  for(i=0;i<PLT_RESOURCE_MAX;i++){
    if(pWork->cellRes[i] ){
      GFL_CLGRP_PLTT_Release(pWork->cellRes[i] );
    }
  }
  for(;i<CHAR_RESOURCE_MAX;i++){
    if(pWork->cellRes[i] ){
      GFL_CLGRP_CGR_Release(pWork->cellRes[i] );
    }
  }
  for(;i<ANM_RESOURCE_MAX;i++){
    if(pWork->cellRes[i] ){
      GFL_CLGRP_CELLANIM_Release(pWork->cellRes[i] );
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   はいいいえウインドウ
 * @retval  none
 */
//------------------------------------------------------------------------------



static void _YesNoStart(IRC_BATTLE_MENU* pWork)
{
  int i;
  APP_TASKMENU_INITWORK appinit;

  appinit.heapId = pWork->heapID;
  appinit.itemNum =  2;
  appinit.itemWork =  &pWork->appitem[0];

  appinit.posType = ATPT_RIGHT_DOWN;
  appinit.charPosX = 32;
  appinit.charPosY = 14;
	appinit.w				 = APP_TASKMENU_PLATE_WIDTH;
	appinit.h				 = APP_TASKMENU_PLATE_HEIGHT;

  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, IRCBTL_STR_27, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  pWork->appitem[1].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, IRCBTL_STR_28, pWork->appitem[1].str);
  pWork->appitem[1].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  pWork->appitem[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  pWork->appitem[1].type = APP_TASKMENU_WIN_TYPE_NORMAL;

  pWork->pAppTask			= APP_TASKMENU_OpenMenu(&appinit,pWork->pAppTaskRes);
  APP_TASKMENU_SetDisableKey(pWork->pAppTask, TRUE);  //キー抑制
  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
  GFL_STR_DeleteBuffer(pWork->appitem[1].str);
  _PaletteFade(pWork, TRUE);
//  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , -8 );
}


static void _ReturnButtonStart(IRC_BATTLE_MENU* pWork)
{
  int i;
#if 0
  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, IRCBTL_STR_03, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  pWork->appitem[0].type = APP_TASKMENU_WIN_TYPE_RETURN;
  pWork->pAppWin =APP_TASKMENU_WIN_Create( pWork->pAppTaskRes,
                                           pWork->appitem, 32-10, 24-4, 10, pWork->heapID);

  
  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
#endif
}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了待ち
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _infoMessageEndCheck(IRC_BATTLE_MENU* pWork)
{
  int state;
  if(pWork->pStream){
    if(pWork->infoDispWin){
      APP_KEYCURSOR_Main( pWork->pKeyCursor, pWork->pStream, pWork->infoDispWin );
    }
    state = PRINTSYS_PrintStreamGetState( pWork->pStream );
    switch(state){
    case PRINTSTREAM_STATE_DONE:
      PRINTSYS_PrintStreamDelete( pWork->pStream );
      pWork->pStream = NULL;
      break;
    case PRINTSTREAM_STATE_PAUSE:
      if(GFL_UI_TP_GetTrg()){
        PMSND_PlaySystemSE(SEQ_SE_MESSAGE);
        PRINTSYS_PrintStreamReleasePause( pWork->pStream );
      }
      break;
    case PRINTSTREAM_STATE_RUNNING:     //20100604 add Saito
      if(GFL_UI_TP_GetCont()){
        PRINTSYS_PrintStreamShortWait( pWork->pStream, 0 );
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
 * @brief   説明ウインドウ表示
 * @param   pWork
 * @param   allput    TRUE:一括表示　FALSE:非一括表示   
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _infoMessageDisp(IRC_BATTLE_MENU* pWork, const BOOL allput)
{
  GFL_BMPWIN* pwin;

  if(pWork->infoDispWin==NULL){
    pWork->infoDispWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME1_S ,
      1 , 3, 30 ,4 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->infoDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  if (!allput)
  {
    pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pStrBuf, pWork->pFontHandle,
                                        MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);
  }
  else
  {
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pwin), 0, 0, pWork->pStrBuf, pWork->pFontHandle );
    pWork->pStream = NULL;
  }

  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
}



//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportInit(IRC_BATTLE_MENU* pWork)
{

  //    GAMEDATA_Save(GAMESYSTEM_GetGameData(GMEVENT_GetGameSysWork(event)));

  GFL_BG_ClearScreenCodeVReq(GFL_BG_FRAME1_S,0);

//  GFL_BG_SetVisible(GFL_BG_FRAME2_S,VISIBLE_OFF);
  
  GFL_MSG_GetString( pWork->pMsgData, IRCBTL_STR_26, pWork->pStrBuf );
  
  _infoMessageDisp(pWork, FALSE);

  _CHANGE_STATE(pWork,_modeReportWait);
}


//------------------------------------------------------------------------------
/**
 * @brief   セーブ中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReporting2(IRC_BATTLE_MENU* pWork)
{
  SAVE_RESULT svr = GAMEDATA_SaveAsyncMain(pWork->pGameData);
    
  if(svr == SAVE_RESULT_OK){
    
    BmpWinFrame_Clear(pWork->infoDispWin, WINDOW_TRANS_OFF);
    GFL_BMPWIN_ClearScreen(pWork->infoDispWin);
    GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
    if( pWork->selectType == EVENTIRCBTL_ENTRYMODE_SINGLE){
      _CHANGE_STATE(pWork, _modeSelectEntryNumInit);
    }
    else{
      _CHANGE_STATE(pWork, _modeFadeoutStart);
    }
    pWork->IsIrc = TRUE;  //赤外線接続開始
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   セーブ中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReporting(IRC_BATTLE_MENU* pWork)
{

  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  GAMEDATA_SaveAsyncStart(pWork->pGameData);
  _CHANGE_STATE(pWork,_modeReporting2);
}

//------------------------------------------------------------------------------
/**
 * @brief   キーを押したらもどる
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _hitAnyKeyWaitMode(IRC_BATTLE_MENU* pWork)
{
  if(!_infoMessageEndCheck(pWork)){
    return;
  }
	if(GFL_UI_TP_GetTrg()){
    GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
    _infoMessageEnd(pWork);
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_EXIT;
    _CHANGE_STATE(pWork,_modeSelectMenuWait);
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait2(IRC_BATTLE_MENU* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){
      if(SaveControl_IsOverwritingOtherData( GAMEDATA_GetSaveControlWork(pWork->pGameData))){
        GFL_MSG_GetString( pWork->pMsgData, IRCBTL_STR_46, pWork->pStrBuf );
        _infoMessageDisp(pWork, FALSE);
        _CHANGE_STATE(pWork,  _hitAnyKeyWaitMode);
      }
      else{
        GFL_MSG_GetString( pWork->pMsgData, IRCBTL_STR_29, pWork->pStrBuf );
        _infoMessageDisp(pWork, TRUE);
        _MESSAGE_WindowTimeIconStart(pWork);
        _CHANGE_STATE(pWork,_modeReporting);
      }
    }
    else{
      GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
      _infoMessageEnd(pWork);
      pWork->selectType = EVENTIRCBTL_ENTRYMODE_EXIT;

      _CHANGE_STATE(pWork,_modeSelectMenuWait);

      //_CHANGE_STATE(pWork,  _modeSelectMenuInit);
    }
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
    _PaletteFade(pWork, FALSE);
//    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 );
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait(IRC_BATTLE_MENU* pWork)
{
  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  _YesNoStart(pWork);
  _CHANGE_STATE(pWork,_modeReportWait2);
}

static GFL_DISP_VRAM _defVBTbl = {
  GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_128K,

};

//--------------------------------------------------------------
/**
 * G3D VBlank処理
 * @param TCB GFL_TCB
 * @param work tcb work
 * @retval nothing
 */
//--------------------------------------------------------------
static void	_VBlank( GFL_TCB *tcb, void *work )
{
  IRC_BATTLE_MENU *pWork=work;

  GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank

}

//------------------------------------------------------------------------------
/**
 * @brief   PROCスタート
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleMenuProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, 0x18000 );

  {
    IRC_BATTLE_MENU *pWork = GFL_PROC_AllocWork( proc, sizeof( IRC_BATTLE_MENU ), HEAPID_IRCBATTLE );
    GFL_STD_MemClear(pWork, sizeof(IRC_BATTLE_MENU));
    pWork->heapID = HEAPID_IRCBATTLE;


		GFL_BG_Init(pWork->heapID);
		GFL_BMPWIN_Init(pWork->heapID);
		GFL_FONTSYS_Init();
		GFL_DISP_SetBank( &_defVBTbl );
		{
			GFL_BG_SYS_HEADER BGsys_data = {
				GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
			};
			GFL_BG_SetBGMode( &BGsys_data );
		}
		GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
		GFL_DISP_GXS_SetVisibleControlDirect(0);

    GFL_CLACT_SYS_Create(	&GFL_CLSYSINIT_DEF_DIVSCREEN, &_defVBTbl, pWork->heapID );
    pWork->cellUnit = GFL_CLACT_UNIT_Create( 40 , 0 , pWork->heapID );

    pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

    _createSubBg(pWork);
		_modeInit(pWork);
    pWork->pVramOBJ = GFL_HEAP_AllocMemory(pWork->heapID, 16*2*16);  //バックアップ
    pWork->pVramBG = GFL_HEAP_AllocMemory(pWork->heapID, 16*2*16);  //バックアップ
    
    pWork->pMsgTcblSys = GFL_TCBL_Init( pWork->heapID , pWork->heapID , 2 , 0 );
    pWork->pKeyCursor = APP_KEYCURSOR_Create( 15, FALSE, TRUE, pWork->heapID );
    pWork->SysMsgQue = PRINTSYS_QUE_Create( pWork->heapID );
    pWork->pAppTaskRes =
      APP_TASKMENU_RES_Create( GFL_BG_FRAME1_S, _SUBLIST_NORMAL_PAL,
                               pWork->pFontHandle, pWork->SysMsgQue, pWork->heapID  );

    _CLACT_SetResource(pWork);

    _CLACT_SetAnim(pWork,57,154,CELL_IRWAVE1,NANR_ir_ani_CellAnime10);
    _CLACT_SetAnim(pWork,200,38,CELL_IRWAVE2,NANR_ir_ani_CellAnime11);

      
		WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
									WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
		_CHANGE_STATE(pWork,_modeSelectMenuInit);
    pWork->dbw = pwk;
    pWork->pGameData = GAMESYSTEM_GetGameData(IrcBattle_GetGAMESYS_WORK(pWork->dbw));

    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG0 , 15, 4 );

    
//    pWork->aIrcBgWork.heapID = pWork->heapID;

//    ircBGAnimInit(&pWork->aIrcBgWork);
    
    
    GFL_NET_ReloadIconTopOrBottom(FALSE, pWork->heapID);
  }
  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_OBJ );
  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_OBJ );
  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_OBJ,  GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG0, 9, 14);

  
  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleMenuProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  IRC_BATTLE_MENU* pWork = mywk;
  GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;
  StateFunc* state = pWork->state;

  //通信アイコンLEVEL更新のため
  WIH_DWC_GetAllBeaconTypeBit(GAMEDATA_GetWiFiList(pWork->pGameData));

  if(state != NULL){
    state(pWork);
    retCode = GFL_PROC_RES_CONTINUE;
  }

  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }
  if(pWork->pAppWin){
    APP_TASKMENU_WIN_Update( pWork->pAppWin );
  }

  GFL_BG_SetScroll( GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, pWork->yoffset );
  pWork->yoffset--;

  GFL_TCBL_Main( pWork->pMsgTcblSys );
  PRINTSYS_QUE_Main(pWork->SysMsgQue);

  GFL_CLACT_SYS_Main();

  if(pWork->endStart==FALSE){
    GAMESYSTEM_CommBootAlways( IrcBattle_GetGAMESYS_WORK(pWork->dbw) );
  }

  if(GAMESYSTEM_IsBatt10Sleep(IrcBattle_GetGAMESYS_WORK(pWork->dbw))){
    retCode= GFL_PROC_RES_FINISH;
    pWork->selectType = EVENTIRCBTL_ENTRYMODE_EXIT;
    WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
    WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
  }
  
  return retCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleMenuProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  IRC_BATTLE_MENU* pWork = mywk;
  EVENT_IRCBATTLE_WORK* pParentWork =pwk;
  
  if(!WIPE_SYS_EndCheck()){
    return GFL_PROC_RES_CONTINUE;
  }

  _workEnd(pWork);
  EVENT_IrcBattleSetType(pParentWork, pWork->selectType);
  pParentWork->bBattelBox = pWork->bBattelBox;

  if(pWork->pTimeIcon){
    TIMEICON_Exit(pWork->pTimeIcon);
    pWork->pTimeIcon=NULL;
  }
  
  GFL_HEAP_FreeMemory(pWork->pVramOBJ);
  GFL_HEAP_FreeMemory(pWork->pVramBG);
  _CLACT_Release(pWork);

  GFL_PROC_FreeWork(proc);

  APP_KEYCURSOR_Delete( pWork->pKeyCursor );
  GFL_TCBL_Exit(pWork->pMsgTcblSys);
	GFL_BG_FreeBGControl(_SUBSCREEN_BGPLANE);
  PRINTSYS_QUE_Clear(pWork->SysMsgQue);
  PRINTSYS_QUE_Delete(pWork->SysMsgQue);
  if(pWork->infoDispWin){
    GFL_BMPWIN_Delete(pWork->infoDispWin);
  }
  if(pWork->pAppWin){
    APP_TASKMENU_WIN_Delete(pWork->pAppWin);
  }
  APP_TASKMENU_RES_Delete( pWork->pAppTaskRes );

  GFL_TCB_DeleteTask( pWork->g3dVintr );
  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  GFL_CLACT_SYS_Delete();
  
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();


	GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);


  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA IrcBattleMenuProcData = {
  IrcBattleMenuProcInit,
  IrcBattleMenuProcMain,
  IrcBattleMenuProcEnd,
};


