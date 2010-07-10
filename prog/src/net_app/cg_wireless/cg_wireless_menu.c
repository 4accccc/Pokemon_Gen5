//=============================================================================
/**
 * @file	  cg_wireless_menu.c
 * @bfief	  CGEARのワイヤレスボタンを押した時
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/12/14
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
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

#include "net/network_define.h"
#include "net/wih_dwc.h"
#include "net/net_whpipe.h"

#include "net_app/cg_wireless_menu.h"
#include "../../field/event_cg_wireless.h"

#include "cg_comm.naix"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK

#include "cg_wireless_menu.cdat"

#include "msg/msg_cg_wireless.h"

#include "../../field/field_comm/intrude_work.h"
#include "../../field/field_comm/intrude_main.h"

#include "net/net_whpipe.h"
#include "net/ctvt_beacon.h"



typedef enum
{
  PLT_OBJ,
  PLT_RESOURCE_MAX,
  CHAR_OBJ = PLT_RESOURCE_MAX,
  CHAR_RESOURCE_MAX,
  ANM_OBJ = CHAR_RESOURCE_MAX,
  ANM_RESOURCE_MAX,
  CEL_RESOURCE_MAX,
} _CELL_RESOURCE_TYPE;


#define _PALETTE_R(pal)  (pal & 0x001f)
#define _PALETTE_G(pal)  ((pal & 0x03e0) >> 5)
#define _PALETTE_B(pal)  ((pal & 0x7c00) >> 10)


#if DEBUG_ONLY_FOR_ohno
#define _NET_DEBUG (1)
#else
#define _NET_DEBUG (0)
#endif
#define _WORK_HEAPSIZE (0x1000)  // 調整が必要

#define _SE_DESIDE (SEQ_SE_SYS_69)
#define _SE_CANCEL (SEQ_SE_SYS_70)

#define _TEL_TIMER (66)  //電話タイマー  CGEARのパレットタイマーと同じ値

//--------------------------------------------
// 画面構成定義
//--------------------------------------------

#define _UNIONROOM_MSG (4)
#define _UNIONROOM_NUM (5)
#define _FUSHIGI_MSG (6)
#define _FUSHIGI_ONOFF (7)
#define _SURE_MSG   (8)
#define _SURE_ONOFF (9)
#define _HI_NAMAE (10)
#define _WINDOW_MAXNUM (11)   //ウインドウのパターン数

#define _MESSAGE_BUF_NUM	( 100*2 )

#define _SUBLIST_NORMAL_PAL   (13)   //サブメニューの通常パレット
#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (3)    // ウインドウ高さ
#define _BUTTON_WIN_PAL   (11)  // ウインドウ
#define _BUTTON_MSG_PAL   (12)  // メッセージフォント

//BG面とパレット番号
#define _SUBSCREEN_BGPLANE	(GFL_BG_FRAME1_S)
#define _SUBSCREEN_PALLET	(0xE)


#define _CONNECT_COUNT_MAX (16)  //ユニオン関連のビーコンを集める

// ２－６とDEFのパレットは＋１５
// １，７、８のパレットは＋５


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



#define _PALETTE_CHANGE_NUM (7)  //CGEARボタンに使っているパレット

//--------------------------------------------
// 内部ワーク
//--------------------------------------------


enum _IBMODE_SELECT {
  _SELECTMODE_PALACE = 0,
  _SELECTMODE_TV,
  _SELECTMODE_EXIT,
  _SELECTMODE_MAX,
  _SELECTMODE_PALACE2 = _SELECTMODE_MAX,
  _SELECTMODE_TV2,
};


#define _SUBMENU_LISTMAX (2)


typedef void (StateFunc)(CG_WIRELESS_MENU* pState);
typedef BOOL (TouchFunc)(int no, CG_WIRELESS_MENU* pState);

/**
 * @brief 接続人数収集構造体
 */

typedef struct{
  u8 bssid[WM_SIZE_BSSID+2];
  u16 downSec;
  u16 connectNum;
} _CONNECTNUM_INFO;




struct _CG_WIRELESS_MENU {
  StateFunc* state;      ///< ハンドルのプログラム状態
  TouchFunc* touch;
  int selectType;   // 接続タイプ
  HEAPID heapID;
  GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理

  GFL_BMPWIN* nameWin; /// ウインドウ管理

  GFL_BUTTON_MAN* pButton;
  GFL_MSGDATA *pMsgData;  //
  WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
  GFL_FONT* pFontHandle;
  STRBUF*  pExpStrBuf;
  STRBUF* pStrBuf;
  STRBUF* pStrBufTVTName;
  u32 bgchar;  //GFL_ARCUTIL_TRANSINFO
  u32 subchar;
  _CONNECTNUM_INFO aConnectNum[_CONNECT_COUNT_MAX];
  
  int palace_state;
  int palace_num;
  u8 BackupPalette[16 * _PALETTE_CHANGE_NUM *2];
  u8 LightPalette[16 * _PALETTE_CHANGE_NUM *2];
  u16 TransPalette[16 ];
  GFL_CLUNIT	*cellUnit;
  GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル

  GFL_BMPWIN* infoDispWin;
  PRINT_STREAM* pStream;
  GFL_TCBLSYS *pMsgTcblSys;
  PRINT_QUE*            SysMsgQue;
  APP_TASKMENU_WORK* pAppTask;
  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
  APP_TASKMENU_RES* pAppTaskRes;
  //  APP_TASKMENU_WIN_WORK* pAppWin;
  EVENT_CG_WIRELESS_WORK * dbw;
  int windowNum;
  int tvtIndex;
  GAMEDATA* gamedata;
  GAMESYS_WORK *gsys;
  int yoffset;
  //  int anmCnt;  //決定時アニメカウント
  int bttnid;
  u16 anmCos;
  GAME_COMM_STATUS_BIT bit;
  GAME_COMM_STATUS_BIT bitold;
  int unionnum;
  int unionnumOld;
  int cross_change;
  u32 cellRes[CEL_RESOURCE_MAX];
  int TelTimer;
  BOOL bFadeStart;
  GFL_CLWK* buttonObj[_SELECTMODE_MAX];
  GFL_CLWK* TVTCall;
  GFL_CLWK* TVTCallName;
  GFL_CLWK* HiName;

};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(CG_WIRELESS_MENU* pWork,StateFunc* state);
static void _changeStateDebug(CG_WIRELESS_MENU* pWork,StateFunc* state, int line);
static void _buttonWindowCreate(int num,int* pMsgBuff,CG_WIRELESS_MENU* pWork, _WINDOWPOS* pos);
static void _modeSelectMenuInit(CG_WIRELESS_MENU* pWork);
static void _modeSelectMenuWait(CG_WIRELESS_MENU* pWork);

static void _modeReportInit(CG_WIRELESS_MENU* pWork);
static void _modeReportWait(CG_WIRELESS_MENU* pWork);
static BOOL _modeSelectMenuButtonCallback(int bttnid,CG_WIRELESS_MENU* pWork);
static void _modeSelectBattleTypeInit(CG_WIRELESS_MENU* pWork);

static void _buttonWindowDelete(CG_WIRELESS_MENU* pWork);
static void _ReturnButtonStart(CG_WIRELESS_MENU* pWork);
static void _UpdatePalletAnime(CG_WIRELESS_MENU* pWork );
static void _buttonWindowDeleteF3S(int no, CG_WIRELESS_MENU* pWork);  //20100604 add Saito


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

static void _changeState(CG_WIRELESS_MENU* pWork,StateFunc state)
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
static void _changeStateDebug(CG_WIRELESS_MENU* pWork,StateFunc state, int line)
{
  NET_PRINT("cgw: %d\n",line);
  _changeState(pWork, state);
}
#endif





static void _createSubBg(CG_WIRELESS_MENU* pWork)
{

  // 背景面
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000,
      0x8000,
      GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME0_M );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
    GFL_BG_SetPriority( GFL_BG_FRAME0_M, 0 );
    GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );

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

    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
  }
  {
    int frame = GFL_BG_FRAME2_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
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
      1, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   受け取った数のウインドウを等間隔に作る 幅は3char
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _buttonWindowCreate(int num,int* pMsgBuff,CG_WIRELESS_MENU* pWork, _WINDOWPOS* pos)
{
  int i;
  u32 cgx;
  int frame = GFL_BG_FRAME3_S;


  pWork->windowNum = num;
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);

  for(i = 0;i < _WINDOW_MAXNUM;i++){
    if(pWork->buttonWin[i]){
      GFL_BMPWIN_ClearScreen(pWork->buttonWin[i]);
      BmpWinFrame_Clear(pWork->buttonWin[i], WINDOW_TRANS_OFF);
      GFL_BMPWIN_Delete(pWork->buttonWin[i]);
    }
    pWork->buttonWin[i] = NULL;
  }

  for(i=0;i < num;i++){
    pWork->buttonWin[i] = GFL_BMPWIN_Create(
      frame,
      pos[i].leftx, pos[i].lefty,
      pos[i].width, pos[i].height,
      _SUBLIST_NORMAL_PAL,GFL_BMP_CHRAREA_GET_F);
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0 );
    // システムウインドウ枠描画
    GFL_MSG_GetString(  pWork->pMsgData, pMsgBuff[i], pWork->pStrBuf );
    PRINTSYS_PrintColor(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0, 0,
                        pWork->pStrBuf, pWork->pFontHandle, APP_TASKMENU_ITEM_MSGCOLOR);
    GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
    GFL_BMPWIN_MakeScreen(pWork->buttonWin[i]);
  }


  if(pWork->pButton){
    GFL_BMN_Delete(pWork->pButton);
  }
  pWork->pButton = NULL;
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME3_S);
}


//------------------------------------------------------------------------------
/**
 * @brief   受け取った数のウインドウを等間隔に作る 幅は3char
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _buttonWindowCreate2(int no, CG_WIRELESS_MENU* pWork, _WINDOWPOS* pos)
{
  int i = no;
  u32 cgx;
  int frame = GFL_BG_FRAME3_S;


  if(pWork->buttonWin[i]){
    GFL_BMPWIN_ClearScreen(pWork->buttonWin[i]);
    BmpWinFrame_Clear(pWork->buttonWin[i], WINDOW_TRANS_OFF);
    GFL_BMPWIN_Delete(pWork->buttonWin[i]);
    pWork->buttonWin[i] = NULL;
  }

  pWork->buttonWin[i] = GFL_BMPWIN_Create(
    frame,
    pos[0].leftx, pos[0].lefty,
    pos[0].width, pos[0].height,  _SUBLIST_NORMAL_PAL,GFL_BMP_CHRAREA_GET_F);
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0 );
  // システムウインドウ枠描画
  PRINTSYS_PrintColor(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0, 0,
                      pWork->pStrBuf, pWork->pFontHandle, APP_TASKMENU_ITEM_MSGCOLOR);
  GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
  GFL_BMPWIN_MakeScreen(pWork->buttonWin[i]);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME3_S);
}

static void _buttonWindowDelete2(int no, CG_WIRELESS_MENU* pWork)
{
  int i=no;
  if(pWork->buttonWin[i]){
    GFL_BMPWIN_ClearScreen(pWork->buttonWin[i]);
    BmpWinFrame_Clear(pWork->buttonWin[i], WINDOW_TRANS_OFF);
    GFL_BMPWIN_Delete(pWork->buttonWin[i]);
  }
  pWork->buttonWin[i] = NULL;
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
}

//----------------------------------------------------------------------------
/**
 *	@brief	BGフレーム3（サブ画面）のウィンドウを消去
 *
 *	@param	no		ボタンID
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void _buttonWindowDeleteF3S(int no, CG_WIRELESS_MENU* pWork)
{
  int i=no;
  if(pWork->buttonWin[i]){
    GFL_BMPWIN_ClearScreen(pWork->buttonWin[i]);
    BmpWinFrame_Clear(pWork->buttonWin[i], WINDOW_TRANS_OFF);
    GFL_BMPWIN_Delete(pWork->buttonWin[i]);
  }
  pWork->buttonWin[i] = NULL;
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME3_S);
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

static void _buttonWindowDelete(CG_WIRELESS_MENU* pWork)
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
  CG_WIRELESS_MENU *pWork = p_work;
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

static void _CreateButtonObj(CG_WIRELESS_MENU* pWork)
{
  int i;
  u8 buffx[]={128,128,224};
  u8 buffy[]={ 192/2 , 192/2, 177};
  u8 buttonno[]={12,11,0};
  GFL_CLWK_DATA cellInitData;


  for(i=0;i<_SELECTMODE_MAX;i++){
    cellInitData.pos_x = buffx[i];
    cellInitData.pos_y = buffy[i];
    cellInitData.anmseq = buttonno[i];
    cellInitData.softpri = 1;
    cellInitData.bgpri = 2;
    pWork->buttonObj[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->cellRes[CHAR_OBJ],
                                               pWork->cellRes[PLT_OBJ],
                                               pWork->cellRes[ANM_OBJ],
                                               &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->buttonObj[i] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( pWork->buttonObj[i], TRUE );
    GFL_CLACT_WK_SetObjMode(pWork->buttonObj[i],GX_OAM_MODE_XLU);
  }



  cellInitData.pos_x = 128;
  cellInitData.pos_y = 192/2;
  cellInitData.anmseq = 15;
  cellInitData.softpri = 0;
  cellInitData.bgpri = 2;
  pWork->TVTCall = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                        pWork->cellRes[CHAR_OBJ],
                                        pWork->cellRes[PLT_OBJ],
                                        pWork->cellRes[ANM_OBJ],
                                        &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->TVTCall , TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->TVTCall, TRUE );
  GFL_CLACT_WK_SetObjMode(pWork->TVTCall, GX_OAM_MODE_XLU);


  cellInitData.pos_x = 128;
  cellInitData.pos_y = 192/2;
  cellInitData.anmseq = 18;
  cellInitData.softpri = 0;
  cellInitData.bgpri = 2;
  pWork->TVTCallName = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                            pWork->cellRes[CHAR_OBJ],
                                            pWork->cellRes[PLT_OBJ],
                                            pWork->cellRes[ANM_OBJ],
                                            &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->TVTCallName , TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->TVTCallName, FALSE );
  GFL_CLACT_WK_SetObjMode(pWork->TVTCallName, GX_OAM_MODE_XLU);


  cellInitData.pos_x = 128;
  cellInitData.pos_y = 192/2;
  cellInitData.anmseq = 25;
  cellInitData.softpri = 0;
  cellInitData.bgpri = 2;
  pWork->HiName = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                       pWork->cellRes[CHAR_OBJ],
                                       pWork->cellRes[PLT_OBJ],
                                       pWork->cellRes[ANM_OBJ],
                                       &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->HiName , TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->HiName, FALSE );
  GFL_CLACT_WK_SetObjMode(pWork->HiName, GX_OAM_MODE_XLU);


}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeInit(CG_WIRELESS_MENU* pWork)
{

  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pStrBufTVTName = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pExpStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );


  pWork->nameWin = GFL_BMPWIN_Create(GFL_BG_FRAME1_S,
                                     _WIRLESSMAIN_TV_NAME_ST_X, _WIRLESSMAIN_TV_NAME_ST_Y,
                                     _WIRLESSMAIN_TV_NAME_WIDTH, _WIRLESSMAIN_TV_NAME_HEIGHT,
                                     _SUBLIST_NORMAL_PAL,GFL_BMP_CHRAREA_GET_F);


  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_cg_wireless_dat, pWork->heapID );

  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_CG_COMM, pWork->heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_cg_comm_background_NCLR,
                                      PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_cg_comm_comm_bg_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  pWork->heapID);

    {
      NNSG2dPaletteData* pPal;
      void* pData;

      pData = GFL_ARCHDL_UTIL_LoadPalette( p_handle, NARC_cg_comm_comm_bg_NCLR, &pPal,  pWork->heapID);
      GFL_STD_MemCopy(pPal->pRawData,pWork->BackupPalette,_PALETTE_CHANGE_NUM*32);
      GFL_STD_MemCopy(pPal->pRawData,pWork->LightPalette,_PALETTE_CHANGE_NUM*32);
      _lightPaletteMake((u16*)pWork->LightPalette, Btn_PaletteTable, _PALETTE_CHANGE_NUM);

      GFL_HEAP_FreeMemory(pData);
    }

    // サブ画面BG0キャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_cg_comm_comm_bg_NCGR,
                                                                  GFL_BG_FRAME0_S, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_cg_comm_comm_base_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_cg_comm_comm_wireless_btn_NSCR,
                                              GFL_BG_FRAME2_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);



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



  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME1_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  _CreateButtonObj(pWork);
}






//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuInit(CG_WIRELESS_MENU* pWork)
{

  int aMsgBuff[]={CGEAR_WIRLESS_001,CGEAR_WIRLESS_002};

  _buttonWindowCreate(NELEMS(aMsgBuff), aMsgBuff, pWork,wind_wireless);

  pWork->pButton = GFL_BMN_Create( btn_wireless, _BttnCallBack, pWork,  pWork->heapID );
  pWork->touch = &_modeSelectMenuButtonCallback;

  _ReturnButtonStart(pWork);


  _CHANGE_STATE(pWork,_modeSelectMenuWait);

}

static void _workEnd(CG_WIRELESS_MENU* pWork)
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
  GFL_STR_DeleteBuffer(pWork->pExpStrBuf);
  GFL_STR_DeleteBuffer(pWork->pStrBufTVTName);
  GFL_BMPWIN_Delete(pWork->nameWin);

  //  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );

}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeout(CG_WIRELESS_MENU* pWork)
{
  if(WIPE_SYS_EndCheck()){
    switch(pWork->selectType){
    case CG_WIRELESS_RETURNMODE_TV:
      if(GAME_COMM_NO_NULL==GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(pWork->gsys))){
        _CHANGE_STATE(pWork, NULL);        // 終わり
      }
      break;
    default:
      _CHANGE_STATE(pWork, NULL);        // 終わり
      break;
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeoutStart(CG_WIRELESS_MENU* pWork)
{
  pWork->bFadeStart=TRUE;
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

  if(pWork->selectType == CG_WIRELESS_RETURNMODE_TV){
    if(GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(pWork->gsys)) != GAME_COMM_NO_NULL){
      NET_WHPIPE_SetBeaconCatchFunc(NULL,NULL);
      GameCommSys_ExitReq( GAMESYSTEM_GetGameCommSysPtr(pWork->gsys) );
    }
  }
  _CHANGE_STATE(pWork, _modeFadeout);        // 終わり
}



static void _modeAppWinFlashCallback(u32 param, fx32 currentFrame )
{
  CG_WIRELESS_MENU* pWork = (CG_WIRELESS_MENU*)param;
  {
    // セーブはしない
    //    if(WIRELESSSAVE_ON == CONFIG_GetWirelessSaveMode(SaveData_GetConfig(GAMEDATA_GetSaveControlWork(pWork->gamedata)))){
    //      if((pWork->selectType != CG_WIRELESS_RETURNMODE_NONE) || (pWork->selectType != CG_WIRELESS_RETURNMODE_PALACE)){
    //        _CHANGE_STATE(pWork, _modeReportInit);
    //        return;
    //      }
    //    }
  }
  _CHANGE_STATE(pWork, _modeFadeoutStart);        // 終わり
}

static void _modeAppWinFlash2(CG_WIRELESS_MENU* pWork)
{
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeAppWinFlash(CG_WIRELESS_MENU* pWork)
{
  GFL_CLWK_ANM_CALLBACK cbwk;
  int objno= pWork->bttnid;

  cbwk.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM ;  // CLWK_ANM_CALLBACK_TYPE
  cbwk.param = (u32)pWork;          // コールバックワーク
  cbwk.p_func = _modeAppWinFlashCallback; // コールバック関数

  if(_SELECTMODE_PALACE2 == pWork->bttnid){
    objno = _SELECTMODE_PALACE;
  }
  else if(_SELECTMODE_TV2 == pWork->bttnid){
    objno = _SELECTMODE_TV;
  }
  GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[objno],TRUE);
  GFL_CLACT_WK_StartAnmCallBack( pWork->buttonObj[objno], &cbwk );
  GFL_CLACT_WK_ResetAnm( pWork->buttonObj[objno] );

  _CHANGE_STATE(pWork,_modeAppWinFlash2);

#if 0

  //決定時アニメ
  int pltNo = Btn_PalettePos[pWork->bttnid];
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
    _CHANGE_STATE(pWork, _modeFadeoutStart);        // 終わり
  }
#endif



}



//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectMenuButtonCallback(int bttnid,CG_WIRELESS_MENU* pWork)
{
  GAME_COMM_SYS_PTR pComm = GAMESYSTEM_GetGameCommSysPtr(pWork->gsys);

  pWork->bttnid=bttnid;

  switch( bttnid ){
  case _SELECTMODE_PALACE:
  case _SELECTMODE_PALACE2:  
    {
      pWork->dbw->hilinkStateNo = Intrude_GetIntrudeStatus(pComm);
      pWork->selectType = CG_WIRELESS_RETURNMODE_PALACE;
      PMSND_PlaySystemSE(_SE_DESIDE);
      _CHANGE_STATE(pWork,_modeAppWinFlash);
    }
    break;
  case _SELECTMODE_TV:
  case _SELECTMODE_TV2:
    pWork->selectType = CG_WIRELESS_RETURNMODE_TV;


    PMSND_PlaySystemSE(_SE_DESIDE);
    _CHANGE_STATE(pWork,_modeAppWinFlash);
    break;
  case _SELECTMODE_EXIT:
    PMSND_PlaySystemSE(_SE_CANCEL);
    //    APP_TASKMENU_WIN_SetDecide(pWork->pAppWin, TRUE);
    pWork->selectType = CG_WIRELESS_RETURNMODE_NONE;
    _CHANGE_STATE(pWork,_modeAppWinFlash);        // 終わり
    break;
  default:
    break;
  }

  return TRUE;
}



static void _MessageDisp(int i,int message,int change,BOOL expand ,CG_WIRELESS_MENU* pWork)
{
  if(pWork->buttonWin[i] && change==FALSE){
    return;
  }
  if(pWork->buttonWin[i]){
    //GFL_BMPWIN_Delete(pWork->buttonWin[i]);
  }

  //  OS_TPrintf("%d %d\n",_msg_wireless[i].width, _msg_wireless[i].height);
  
  if(pWork->buttonWin[i] == NULL){
    pWork->buttonWin[i] = GFL_BMPWIN_Create(
      GFL_BG_FRAME3_S,
      _msg_wireless[i].leftx, _msg_wireless[i].lefty,
      _msg_wireless[i].width, _msg_wireless[i].height, _SUBLIST_NORMAL_PAL,GFL_BMP_CHRAREA_GET_F);
  }
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0 );
  
  if(expand){
    GFL_MSG_GetString(  pWork->pMsgData, message, pWork->pExpStrBuf );
    WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExpStrBuf );
  }
  else{
    GFL_MSG_GetString(  pWork->pMsgData, message, pWork->pStrBuf );
  }

  PRINTSYS_PrintColor(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0, 0,
                      pWork->pStrBuf, pWork->pFontHandle, APP_TASKMENU_ITEM_MSGCOLOR);

  GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
  GFL_BMPWIN_MakeScreen(pWork->buttonWin[i]);
}

static void _UpdateMessage(CG_WIRELESS_MENU* pWork)
{
  BOOL bChange = FALSE;
  int check_count=0;
  GAME_COMM_SYS_PTR pComm = GAMESYSTEM_GetGameCommSysPtr(pWork->gsys);
  int no = Intrude_GetIntrudeStatus(pComm);


  //Beacon変更チェック
  if( pWork->cross_change == FALSE ){
    check_count = 0;
    if( GAMEBEACON_Get_UpdateLogNo(&check_count) != GAMEBEACON_SYSTEM_LOG_MAX ){
      pWork->cross_change = TRUE; // 変更あり
    }
  }

  if( pWork->unionnum != pWork->unionnumOld ){
    bChange = TRUE;
  }
  if(pWork->bitold != pWork->bit){
    bChange = TRUE;
  }
  _MessageDisp(_UNIONROOM_MSG, CGEAR_WIRLESS_003,FALSE,FALSE,pWork);
  _MessageDisp(_FUSHIGI_MSG,   CGEAR_WIRLESS_005,FALSE,FALSE,pWork);
  _MessageDisp(_SURE_MSG,      CGEAR_WIRLESS_013,FALSE,FALSE,pWork);


  switch(no){
  case INTRUDE_CONNECT_NULL:              ///<接続されていない
    {
      WORDSET_RegisterNumber( pWork->pWordSet, 0,  pWork->unionnum, 2,STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
      _MessageDisp(_UNIONROOM_NUM, CGEAR_WIRLESS_004, bChange,TRUE,pWork);
    }
    if(pWork->bit & GAME_COMM_STATUS_BIT_WIRELESS_FU){
      _MessageDisp(_FUSHIGI_ONOFF, CGEAR_WIRLESS_006,bChange,TRUE,pWork);
    }
    else{
      _MessageDisp(_FUSHIGI_ONOFF, CGEAR_WIRLESS_007,bChange,TRUE,pWork);
    }
    if(pWork->cross_change){
      _MessageDisp(_SURE_ONOFF, CGEAR_WIRLESS_006,bChange,TRUE,pWork);
    }
    else{
      _MessageDisp(_SURE_ONOFF, CGEAR_WIRLESS_007,bChange,TRUE,pWork);
    }
    break;
  default:
    _buttonWindowDelete2(_UNIONROOM_NUM, pWork);
    _buttonWindowDelete2(_FUSHIGI_ONOFF, pWork);
    _buttonWindowDelete2(_SURE_ONOFF, pWork);
    break;
  }


  if(no==INTRUDE_CONNECT_MISSION_TARGET){///<ハイリンク接続時(ミッションのターゲットになっている)
    int no2 = Intrude_GetMissionEntryNum(pComm);
    if(pWork->palace_num != no2){
      //人数変更があった場合 NULLにして再描画させる
      pWork->palace_state = INTRUDE_CONNECT_NULL;
    }
  }
  if(pWork->palace_state != no){
    switch(no){
    case INTRUDE_CONNECT_NULL:              ///<接続されていない
      GFL_CLACT_WK_SetDrawEnable( pWork->HiName, FALSE );
      _buttonWindowDeleteF3S(_HI_NAMAE, pWork); //20100604 add Saito
      GFL_MSG_GetString(  pWork->pMsgData, CGEAR_WIRLESS_001, pWork->pStrBuf );
      _buttonWindowCreate2(0,  pWork, wind_wireless1);
      break;
    case INTRUDE_CONNECT_INTRUDE:           ///<ハイリンク接続時(ミッションが行われていない)
      GFL_CLACT_WK_SetDrawEnable( pWork->HiName, FALSE );
      _buttonWindowDeleteF3S(_HI_NAMAE, pWork); //20100604 add Saito
      GFL_MSG_GetString(  pWork->pMsgData, CGEAR_WIRLESS_016, pWork->pStrBuf );
      _buttonWindowCreate2(0,  pWork, wind_wireless2);
      break;
    case INTRUDE_CONNECT_MISSION_TARGET:    ///<ハイリンク接続時(ミッションのターゲットになっている)
      {
        int no2 = Intrude_GetMissionEntryNum(pComm);
        pWork->palace_num = no2;
        GFL_CLACT_WK_SetDrawEnable( pWork->HiName, FALSE );
        _buttonWindowDeleteF3S(_HI_NAMAE, pWork); //20100604 add Saito
        WORDSET_RegisterNumber( pWork->pWordSet, 0,  no2, 1,STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
        GFL_MSG_GetString(  pWork->pMsgData, CGEAR_WIRLESS_017, pWork->pExpStrBuf );
        WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExpStrBuf );
        _buttonWindowCreate2(0,  pWork, wind_wireless2);
      }
      break;
    case INTRUDE_CONNECT_MISSION_PARTNER:   ///<ハイリンク接続時(協力者である)
      GFL_MSG_GetString(  pWork->pMsgData, CGEAR_WIRLESS_018, pWork->pStrBuf );
      _buttonWindowCreate2(0,  pWork, wind_wireless2);
      if( Intrude_GetTargetName(pComm,pWork->pStrBuf) ){
        _buttonWindowCreate2(_HI_NAMAE,  pWork, wind_wireless_name);
        GFL_CLACT_WK_SetDrawEnable( pWork->HiName, TRUE );
      }
      break;
    }
    pWork->palace_state=no;
  }
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME3_S);


}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(CG_WIRELESS_MENU* pWork)
{
  if(WIPE_SYS_EndCheck()){
    GFL_BMN_Main( pWork->pButton );
  }

  _UpdatePalletAnime(pWork);
  _UpdateMessage(pWork);

}


//------------------------------------------------------------------------------
/**
 * @brief   はいいいえウインドウ
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _YesNoStart(CG_WIRELESS_MENU* pWork)
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
  GFL_MSG_GetString(pWork->pMsgData, CGEAR_WIRLESS_008, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pWork->appitem[0].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  pWork->appitem[1].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, CGEAR_WIRLESS_009, pWork->appitem[1].str);
  pWork->appitem[1].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pWork->appitem[1].type = APP_TASKMENU_WIN_TYPE_NORMAL;
  pWork->pAppTask = APP_TASKMENU_OpenMenu(&appinit,pWork->pAppTaskRes);
  APP_TASKMENU_SetDisableKey(pWork->pAppTask, TRUE);  //キー抑制
  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
  GFL_STR_DeleteBuffer(pWork->appitem[1].str);
  G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , -8 );
}


static void _ReturnButtonStart(CG_WIRELESS_MENU* pWork)
{
  int i;

  pWork->appitem[0].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, CGEAR_WIRLESS_010, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  pWork->appitem[0].type = APP_TASKMENU_WIN_TYPE_RETURN;
  //  pWork->pAppWin =APP_TASKMENU_WIN_Create( pWork->pAppTaskRes,
  //                                           pWork->appitem, 32-10, 24-4, 10, pWork->heapID);


  GFL_STR_DeleteBuffer(pWork->appitem[0].str);

}

//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了待ち
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _infoMessageEndCheck(CG_WIRELESS_MENU* pWork)
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
 * @brief   説明ウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _infoMessageDisp(CG_WIRELESS_MENU* pWork)
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

  pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pStrBuf, pWork->pFontHandle,
                                        MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);

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
static void _modeReportInit(CG_WIRELESS_MENU* pWork)
{

  GFL_BG_ClearScreenCodeVReq(GFL_BG_FRAME1_S,0);
  //  GFL_BG_SetVisible(GFL_BG_FRAME1_S,VISIBLE_OFF);

  GFL_MSG_GetString( pWork->pMsgData, CGEAR_WIRLESS_011, pWork->pStrBuf );

  _infoMessageDisp(pWork);


  _CHANGE_STATE(pWork,_modeReportWait);
}


static void _FadeWait(CG_WIRELESS_MENU* pWork)
{
  if(GFL_FADE_CheckFade()){
    return;
  }
  _CHANGE_STATE(pWork,NULL);
}



//------------------------------------------------------------------------------
/**
 * @brief   セーブ中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReporting(CG_WIRELESS_MENU* pWork)
{

  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  {
    SAVE_RESULT svr = GAMEDATA_SaveAsyncMain(pWork->gamedata);

    if(svr == SAVE_RESULT_OK){
      BmpWinFrame_Clear(pWork->infoDispWin, WINDOW_TRANS_OFF);
      GFL_BMPWIN_ClearScreen(pWork->infoDispWin);
      GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);

      //   GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, 1);

      _CHANGE_STATE(pWork,_modeFadeoutStart);
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait2(CG_WIRELESS_MENU* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){
      GFL_MSG_GetString( pWork->pMsgData, CGEAR_WIRLESS_012, pWork->pStrBuf );
      _infoMessageDisp(pWork);
      //セーブ開始
      GAMEDATA_SaveAsyncStart(pWork->gamedata);
      _CHANGE_STATE(pWork,_modeReporting);
    }
    else{
      GFL_BG_ClearScreen(GFL_BG_FRAME1_M);
      pWork->selectType = CG_WIRELESS_RETURNMODE_NONE;
      _CHANGE_STATE(pWork,_modeFadeoutStart);
    }
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 );
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeNetworkOn3(CG_WIRELESS_MENU* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    if(selectno==0){
      //      GFL_BG_SetVisible(GFL_BG_FRAME2_S,VISIBLE_ON);
      _CHANGE_STATE(pWork, _modeSelectMenuInit);
    }
    else{
      //  GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
      pWork->selectType = CG_WIRELESS_RETURNMODE_NONE;
      _CHANGE_STATE(pWork, _modeFadeoutStart);
    }
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_OBJ , 0 );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   通信ONにする確認画面
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeNetworkOn2(CG_WIRELESS_MENU* pWork)
{
  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  _YesNoStart(pWork);
  _CHANGE_STATE(pWork,_modeNetworkOn3);
}

//------------------------------------------------------------------------------
/**
 * @brief   通信ONにする確認画面
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeNetworkOn(CG_WIRELESS_MENU* pWork)
{
  GFL_BG_ClearScreenCodeVReq(GFL_BG_FRAME1_S,0);
  //  GFL_BG_SetVisible(GFL_BG_FRAME2_S,VISIBLE_OFF);

  GFL_MSG_GetString( pWork->pMsgData, CGEAR_WIRLESS_011, pWork->pStrBuf );

  _infoMessageDisp(pWork);
  _CHANGE_STATE(pWork, _modeNetworkOn2);
}


//--------------------------------------------------------------
//	パレットアニメーションの更新
//--------------------------------------------------------------
static void _UpdatePalletAnime(CG_WIRELESS_MENU* pWork )
{
  GAME_COMM_SYS_PTR pComm = GAMESYSTEM_GetGameCommSysPtr(pWork->gsys);
  //  if(GAME_COMM_STATUS_BIT_WIRELESS_TR & pWork->bit){
  //  if(Intrude_CheckPalaceConnect(pComm)){

  int no = Intrude_GetIntrudeStatus(pComm);
  switch(no){
  case INTRUDE_CONNECT_NULL:              ///<接続されていない
  case INTRUDE_CONNECT_INTRUDE:           ///<ハイリンク接続時(ミッションが行われていない)
    if(20 == GFL_CLACT_WK_GetAnmSeq(pWork->buttonObj[0])){
      GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[0],FALSE);
      GFL_CLACT_WK_SetAnmSeq( pWork->buttonObj[0], 12 );
    }
    break;
  case INTRUDE_CONNECT_MISSION_TARGET:    ///<ハイリンク接続時(ミッションのターゲットになっている)
  case INTRUDE_CONNECT_MISSION_PARTNER:   ///<ハイリンク接続時(協力者である)
    if(12 == GFL_CLACT_WK_GetAnmSeq(pWork->buttonObj[0])){
      GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[0],TRUE);
      GFL_CLACT_WK_SetAnmSeq( pWork->buttonObj[0], 20 );
    }
    break;
  }
}

static void _setTVTParentMac(CG_WIRELESS_MENU* pWork, int index)
{
  {
    int i;
    u8 selfMacAdr[6];
    GFLNetInitializeStruct* pIn = GFL_NET_GetNETInitStruct();
    {
      u8 *macAddress = GFL_NET_GetBeaconMacAddress( index );
      pWork->dbw->aTVT.mode = CTM_CHILD;
      GFL_STD_MemCopy(macAddress, pWork->dbw->aTVT.macAddress, 6);
    }
  }
}

static void _resetTVTMac(CG_WIRELESS_MENU* pWork)
{
  {
    int i;
    u8 macAddress[6]={0,0,0,0,0,0};
    GFLNetInitializeStruct* pIn = GFL_NET_GetNETInitStruct();
    {
      pWork->dbw->aTVT.mode = CTM_PARENT;
      GFL_STD_MemCopy(macAddress, pWork->dbw->aTVT.macAddress, 6);
    }
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   テレビトランシーバーのBEACONを拾ったら、親の名前をセット
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _setTVTParentName(CG_WIRELESS_MENU* pWork)
{
  int index = WIH_DWC_TVTCallCheck(GAMEDATA_GetWiFiList(pWork->gamedata));

  if(-1 != index ){
    CTVT_COMM_BEACON *beacon = GFL_NET_GetBeaconData(index);
    STRBUF* pName = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
    GFL_STR_SetStringCodeOrderLength(pName,
                                     CTVT_BCON_GetName(beacon), CTVT_COMM_NAME_LEN);
    if(FALSE==GFL_CLACT_WK_GetDrawEnable( pWork->TVTCallName )){
      //    if(FALSE==GFL_STR_CompareBuffer(pName, pWork->pStrBufTVTName)){
      GFL_STR_SetStringCodeOrderLength(pWork->pStrBufTVTName,
                                       CTVT_BCON_GetName(beacon), CTVT_COMM_NAME_LEN);
      GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->nameWin), 0 );
      PRINTSYS_PrintColor(GFL_BMPWIN_GetBmp(pWork->nameWin), 0, 0,
                          pWork->pStrBufTVTName, pWork->pFontHandle, APP_TASKMENU_ITEM_MSGCOLOR);
      GFL_BMPWIN_TransVramCharacter(pWork->nameWin);
      GFL_BMPWIN_MakeScreen(pWork->nameWin);
      GFL_CLACT_WK_SetDrawEnable( pWork->TVTCallName, TRUE );
      GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);

      _setTVTParentMac(pWork, index);  //接続者を記録
      
    }
    GFL_STR_DeleteBuffer(pName);
    {
      FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( pWork->gamedata );
      FSND_RequestTVTRingTone( fsnd);
    }
    if(15 == GFL_CLACT_WK_GetAnmSeq(pWork->TVTCall)){
      GFL_CLACT_WK_SetAnmSeq(pWork->TVTCall, 16);
    }
    if(11 == GFL_CLACT_WK_GetAnmSeq(pWork->buttonObj[1])){
      GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[1],TRUE);
      GFL_CLACT_WK_SetAnmSeq( pWork->buttonObj[1], 19 );
    }
  }
  else{
    FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( pWork->gamedata );
    FSND_StopTVTRingTone(fsnd);
    GFL_CLACT_WK_SetDrawEnable( pWork->TVTCallName, FALSE );
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->nameWin), 0 );
    GFL_BMPWIN_TransVramCharacter(pWork->nameWin);
    _resetTVTMac(pWork);
    if(19 == GFL_CLACT_WK_GetAnmSeq(pWork->buttonObj[1])){
      GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[1],FALSE);
      GFL_CLACT_WK_SetAnmSeq( pWork->buttonObj[1], 11 );
    }
    if(16 == GFL_CLACT_WK_GetAnmSeq(pWork->TVTCall)){
      GFL_CLACT_WK_SetAnmSeq(pWork->TVTCall, 15);
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait(CG_WIRELESS_MENU* pWork)
{
  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  _YesNoStart(pWork);
  _CHANGE_STATE(pWork,_modeReportWait2);
}



//-------------------------------------------------------------
/**
 * @brief   人数を集めるか
 * @param   bssdesc   グループ情報
 * @retval  none
 */
//-------------------------------------------------------------
static void _ConnectNumCheck(void* pWk, WMBssDesc *bssdesc,int serviceNo,int num)
{
  int i;
  CG_WIRELESS_MENU* pWork=pWk;

  if((serviceNo < WB_NET_UNION) || (WB_NET_UNION_GURUGURU < serviceNo)){
    return;  //ユニオン以外収集しない
  }
  for(i = 0; i < _CONNECT_COUNT_MAX ; i++){
    if(pWork->aConnectNum[i].downSec!=0){
      if(0==GFL_STD_MemComp(pWork->aConnectNum[i].bssid, bssdesc->bssid, WM_SIZE_BSSID)){
        pWork->aConnectNum[i].downSec  = DEFAULT_TIMEOUT_FRAME;
        pWork->aConnectNum[i].connectNum = num;
        return;
      }
    }
  }
  for(i = 0; i < _CONNECT_COUNT_MAX ; i++){
    if(pWork->aConnectNum[i].downSec==0){
      GFL_STD_MemCopy(bssdesc->bssid,pWork->aConnectNum[i].bssid,  WM_SIZE_BSSID);
      pWork->aConnectNum[i].downSec = DEFAULT_TIMEOUT_FRAME;
      pWork->aConnectNum[i].connectNum = num;
      return;
    }
  }
}


//-------------------------------------------------------------
/**
 * @brief   人数を集めるカウンタの計算
 * @param   bssdesc   グループ情報
 * @retval  none
 */
//-------------------------------------------------------------

static void _downNumCheck(CG_WIRELESS_MENU* pWork)
{
  int i;

  for(i = 0; i < _CONNECT_COUNT_MAX ; i++){
    if(pWork->aConnectNum[i].downSec!=0){
      pWork->aConnectNum[i].downSec--;
    }
  }
}



//-------------------------------------------------------------
/**
 * @brief   ユニオンルーム施設の人数を集めるか
 * @param   none
 * @retval  none
 */
//-------------------------------------------------------------

static int _GetUnionConnectNum(CG_WIRELESS_MENU* pWork)
{
  int i,num=0;

  for(i = 0; i < _CONNECT_COUNT_MAX ; i++){
    if(pWork->aConnectNum[i].downSec!=0){
      num += pWork->aConnectNum[i].connectNum;
    }
  }
  return num;
}



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
  CG_WIRELESS_MENU *pWork=work;

  GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank
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
///	セルアクター　初期化データ
//--------------------------------------------------------------
static const GFL_CLSYS_INIT _CLSYS_Init =
{
  0, 0,
  0, 512,
  GFL_CLSYS_OAMMAN_INTERVAL, 128-GFL_CLSYS_OAMMAN_INTERVAL,
  GFL_CLSYS_OAMMAN_INTERVAL, 128-GFL_CLSYS_OAMMAN_INTERVAL, //通信アイコン部分
  0,
  100,
  100,
  100,
  100,
  16, 16,
};


//------------------------------------------------------------------------------
/**
 * @brief   PROCスタート
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT CG_WirelessMenuProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_CG_WIRELESS_WORK* pParentWork =pwk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, 0x18000 );

  {
    CG_WIRELESS_MENU *pWork = GFL_PROC_AllocWork( proc, sizeof( CG_WIRELESS_MENU ), HEAPID_IRCBATTLE );
    GFL_STD_MemClear(pWork, sizeof(CG_WIRELESS_MENU));
    pWork->heapID = HEAPID_IRCBATTLE;
    pWork->gamedata = pParentWork->gameData;
    pWork->gsys = pParentWork->gsys;

    GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
    GXS_DispOn();
    GX_DispOn();

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
    GFL_CLACT_SYS_Create(	&_CLSYS_Init, &_defVBTbl, pWork->heapID );
    pWork->cellUnit = GFL_CLACT_UNIT_Create( 40 , 0 , pWork->heapID );
    pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

    _createSubBg(pWork);
    _modeInit(pWork);
    pWork->pWordSet = WORDSET_CreateEx( 11, 200, pWork->heapID );

    pWork->pMsgTcblSys = GFL_TCBL_Init( pWork->heapID , pWork->heapID , 1 , 0 );
    pWork->SysMsgQue = PRINTSYS_QUE_Create( pWork->heapID );
    pWork->pAppTaskRes =
      APP_TASKMENU_RES_Create( GFL_BG_FRAME1_S, _SUBLIST_NORMAL_PAL,
                               pWork->pFontHandle, pWork->SysMsgQue, pWork->heapID  );

    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG0 , 15, 4 );
    WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

    _CHANGE_STATE(pWork,_modeSelectMenuInit);
    pWork->dbw = pwk;
    {
      GFLNetInitializeStruct* pIn = GFL_NET_GetNETInitStruct();
      pWork->dbw->aTVT.mode = CTM_PARENT;
      pWork->dbw->aTVT.gameData = pWork->gamedata;
    }
    GFL_NET_ReloadIconTopOrBottom(FALSE, pWork->heapID);
    //ユニオンルーム検査関数追加
    NET_WHPIPE_SetBeaconCatchFunc(&_ConnectNumCheck,pWork);
  }

  GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);

  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT CG_WirelessMenuProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  CG_WIRELESS_MENU* pWork = mywk;
  GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;
  StateFunc* state = pWork->state;


  if(state != NULL){
    state(pWork);
    retCode = GFL_PROC_RES_CONTINUE;
  }

  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }

  if(pWork->selectType != CG_WIRELESS_RETURNMODE_TV){
    _setTVTParentName(pWork);
  }
  _downNumCheck(pWork);

  GFL_CLACT_SYS_Main();

  if(GFL_NET_IsInit()){
    pWork->bitold =  pWork->bit;
    pWork->bit = WIH_DWC_GetAllBeaconTypeBit(GAMEDATA_GetWiFiList(pWork->gamedata));

    pWork->unionnumOld = pWork->unionnum;
    pWork->unionnum = _GetUnionConnectNum(pWork);
  }

  GFL_TCBL_Main( pWork->pMsgTcblSys );
  PRINTSYS_QUE_Main(pWork->SysMsgQue);
  GFL_BG_SetScroll( GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, pWork->yoffset );
  pWork->yoffset--;


  if(WIPE_SYS_EndCheck()){
    if(!pWork->bFadeStart){
      GAMESYSTEM_CommBootAlways( pWork->gsys );
    }
  }

  if(WIPE_SYS_EndCheck()){
    if(GAMESYSTEM_IsBatt10Sleep(pWork->gsys)){
      retCode= GFL_PROC_RES_FINISH;
      pWork->selectType = CG_WIRELESS_RETURNMODE_NONE;
      WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
      WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
    }
  }

  return retCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT CG_WirelessMenuProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  CG_WIRELESS_MENU* pWork = mywk;
  EVENT_CG_WIRELESS_WORK* pParentWork =pwk;
  int i;

  if(!WIPE_SYS_EndCheck()){
    return GFL_PROC_RES_CONTINUE;
  }
  NET_WHPIPE_SetBeaconCatchFunc(NULL,NULL);

  _workEnd(pWork);
  pParentWork->selectType = pWork->selectType;

  GFL_PROC_FreeWork(proc);

  GFL_TCBL_Exit(pWork->pMsgTcblSys);
  GFL_BG_FreeBGControl(_SUBSCREEN_BGPLANE);
  PRINTSYS_QUE_Clear(pWork->SysMsgQue);
  PRINTSYS_QUE_Delete(pWork->SysMsgQue);

  for(i=0;i<_SELECTMODE_MAX; i++){
    GFL_CLACT_WK_Remove(pWork->buttonObj[i]);
  }
  GFL_CLACT_WK_Remove( pWork->TVTCall);
  GFL_CLACT_WK_Remove( pWork->TVTCallName);
  GFL_CLACT_WK_Remove( pWork->HiName );


  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_OBJ] );
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_OBJ] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_OBJ] );
  GFL_TCB_DeleteTask( pWork->g3dVintr );
  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  GFL_CLACT_SYS_Delete();

  if(pWork->pStream){
    PRINTSYS_PrintStreamDelete( pWork->pStream );
  }
  WORDSET_Delete(pWork->pWordSet);

  if(pWork->pAppTask){
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
  if(pWork->infoDispWin){
    GFL_BMPWIN_Delete(pWork->infoDispWin);
  }

  APP_TASKMENU_RES_Delete( pWork->pAppTaskRes );


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
const GFL_PROC_DATA CG_WirelessMenuProcData = {
  CG_WirelessMenuProcInit,
  CG_WirelessMenuProcMain,
  CG_WirelessMenuProcEnd,
};


