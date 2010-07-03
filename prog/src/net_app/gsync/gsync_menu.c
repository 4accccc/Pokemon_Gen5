//=============================================================================
/**
 * @file	  gsync_menu.c
 * @bfief	  ゲームシンク最初の表示部分
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/15
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
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
#include "system/time_icon.h"
#include "sound/pm_sndsys.h"

#include "msg/msg_gsync.h"
#include "msg/msg_wifi_system.h"
#include "../../field/event_gsync.h"
#include "gsync.naix"
#include "cg_comm.naix"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK
#include "app/app_keycursor.h"  //APP_TASKMENU_INITWORK
#include "system/palanm.h"

#include "net/wih_dwc.h"
#include "gsync_poke.cdat"
#define _PALETTE_R(pal)  (pal & 0x001f)
#define _PALETTE_G(pal)  ((pal & 0x03e0) >> 5)
#define _PALETTE_B(pal)  ((pal & 0x7c00) >> 10)


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


#if DEBUG_ONLY_FOR_ohno
#define _NET_DEBUG (1)
#else
#define _NET_DEBUG (0)
#endif
#define _WORK_HEAPSIZE (0x1000)  // 調整が必要

// SE
#define _SE_DESIDE (SEQ_SE_SYS_69)
#define _SE_CANCEL (SEQ_SE_SYS_70)


//--------------------------------------------
// 画面構成定義
//--------------------------------------------
#define _WINDOW_MAXNUM (4)   //ウインドウのパターン数

#define _MESSAGE_BUF_NUM	( 100*2 )

#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (3)    // ウインドウ高さ

#define _BUTTON_MSG_PAL2   (8)  // メッセージフォント
#define _SUBLIST_NORMAL_PAL   (9)   //サブメニューの通常パレット 9 10 11
#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (13)  // メッセージフォント

//BG面とパレット番号
#define _SUBSCREEN_BGPLANE	(GFL_BG_FRAME1_S)
#define _SUBSCREEN_PALLET	(0xE)




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
  _SELECTMODE_GSYNC = 0,
  _SELECTMODE_UTIL,
  _SELECTMODE_EXIT,
  _SELECTMODE_MAX,
};


#define _SUBMENU_LISTMAX (2)


typedef void (StateFunc)(GAMESYNC_MENU* pState);
typedef BOOL (TouchFunc)(int no, GAMESYNC_MENU* pState);


struct _GAMESYNC_MENU {
  StateFunc* state;      ///< ハンドルのプログラム状態
  TouchFunc* touch;
  int selectType;   // 接続タイプ
  HEAPID heapID;
  GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
  GFL_BUTTON_MAN* pButton;
  GFL_MSGDATA *pMsgData;  //
  GFL_MSGDATA *pMsgWiFiData;  //
  WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
  GFL_FONT* pFontHandle;
  STRBUF*  pExpStrBuf;
  STRBUF* pStrBuf;
  u32 bgchar;  //GFL_ARCUTIL_TRANSINFO
  u32 subchar;

  u8 BackupPalette[16 * _PALETTE_CHANGE_NUM *2];
  u8 LightPalette[16 * _PALETTE_CHANGE_NUM *2];
  u16 TransPalette[16 ];

  GFL_BMPWIN* infoDispWin;
  PRINT_STREAM* pStream;
  APP_KEYCURSOR_WORK* pKeyCursor;
  GFL_TCBLSYS *pMsgTcblSys;
  PRINT_QUE*            SysMsgQue;
  APP_TASKMENU_WORK* pAppTask;
  APP_TASKMENU_ITEMWORK appitem[_SUBMENU_LISTMAX];
  APP_TASKMENU_RES* pAppTaskRes;
  //  APP_TASKMENU_WIN_WORK* pAppWin;
  EVENT_GSYNC_WORK * dbw;
  int windowNum;
  GAMEDATA* gamedata;
  GAMESYS_WORK *gsys;
  int yoffset;
  int anmCnt;  //決定時アニメカウント
  int bttnid;
  u16 anmCos;
  GAME_COMM_STATUS_BIT bit;
  GAME_COMM_STATUS_BIT bitold;
  void* pVramOBJ;
  void* pVramBG;
  TIMEICON_WORK* pTimeIcon;
  BOOL endStart;
  u32 cellRes[CEL_RESOURCE_MAX];
  GFL_CLWK* buttonObj[_SELECTMODE_MAX];
  GFL_CLUNIT	*cellUnit;
  GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル


};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(GAMESYNC_MENU* pWork,StateFunc* state);
static void _changeStateDebug(GAMESYNC_MENU* pWork,StateFunc* state, int line);
static void _buttonWindowCreate(int num,int* pMsgBuff,GAMESYNC_MENU* pWork, _WINDOWPOS* pos);
static void _modeSelectMenuInit(GAMESYNC_MENU* pWork);
static void _modeSelectMenuWait(GAMESYNC_MENU* pWork);

static void _modeReportInit(GAMESYNC_MENU* pWork);
static void _modeReportWait(GAMESYNC_MENU* pWork);
static BOOL _modeSelectMenuButtonCallback(int bttnid,GAMESYNC_MENU* pWork);
static void _modeSelectBattleTypeInit(GAMESYNC_MENU* pWork);

static void _buttonWindowDelete(GAMESYNC_MENU* pWork);
static void _modeFadeout(GAMESYNC_MENU* pWork);
static void _modeFadeoutStart(GAMESYNC_MENU* pWork);


static void _infoMessageDispClear(GAMESYNC_MENU* pWork);
static void _infoMessageEnd(GAMESYNC_MENU* pWork);
static BOOL _infoMessageEndCheck(GAMESYNC_MENU* pWork);
static void _infoMessageDisp(GAMESYNC_MENU* pWork);
static void _infoMessageDispHeight(GAMESYNC_MENU* pWork,int height,BOOL bStream);
static void _hitAnyKey(GAMESYNC_MENU* pWork);
static void _PaletteFade(GAMESYNC_MENU* pWork,BOOL bFade);
static void _YesNoStart(GAMESYNC_MENU* pWork);



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

static void _changeState(GAMESYNC_MENU* pWork,StateFunc state)
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
static void _changeStateDebug(GAMESYNC_MENU* pWork,StateFunc state, int line)
{
  NET_PRINT("gsy: %d\n",line);
  _changeState(pWork, state);
}
#endif



static void _createSubBg(GAMESYNC_MENU* pWork)
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
 * @brief   タイムアイコンを出す
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _MESSAGE_WindowTimeIconStart(GAMESYNC_MENU* pWork)
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
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeoutStart(GAMESYNC_MENU* pWork)
{
  pWork->endStart=TRUE;
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );


  switch(pWork->selectType){
  case GAMESYNC_RETURNMODE_SYNC:
  case GAMESYNC_RETURNMODE_UTIL:
    if(GFL_NET_IsInit()){
      GameCommSys_ExitReq( GAMESYSTEM_GetGameCommSysPtr(pWork->gsys) );
    }
    break;
  default:
    break;
  }

  _CHANGE_STATE(pWork, _modeFadeout);        // 終わり
}





static void _wifiConnectYesNoWait(GAMESYNC_MENU* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    if(selectno==0){
      _CHANGE_STATE(pWork, _modeReportInit);
    }
    else{
      GFL_BG_ClearScreen(GFL_BG_FRAME1_S);
      pWork->selectType = GAMESYNC_RETURNMODE_NONE;
      _CHANGE_STATE(pWork, _modeSelectMenuInit); //戻る
    }
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
    _PaletteFade(pWork, FALSE);
  }
}

static void _wifiConnectYesNoCheck(GAMESYNC_MENU* pWork)
{
  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  _YesNoStart(pWork);
  _CHANGE_STATE(pWork,_wifiConnectYesNoWait);
}


static void _modeAppWinFlashCallback(u32 param, fx32 currentFrame )
{
  GAMESYNC_MENU* pWork = (GAMESYNC_MENU*)param;
  {

    GFL_CLACT_WK_StopAnmCallBack(pWork->buttonObj[pWork->bttnid]);
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->buttonObj[pWork->bttnid] , FALSE );

    if(pWork->selectType == GAMESYNC_RETURNMODE_UTIL){
      if(pWork->infoDispWin){
        GFL_BMPWIN_Delete(pWork->infoDispWin);
        pWork->infoDispWin=NULL;
      }

      if( OS_IsRunOnTwl() ){//DSIは呼ぶことが出来ない
        GFL_MSG_GetString( pWork->pMsgWiFiData, dwc_message_0017, pWork->pStrBuf );
        _infoMessageDispHeight(pWork,10, FALSE);
        _PaletteFade(pWork, TRUE);
        _CHANGE_STATE(pWork,_hitAnyKey);
        return;
      }
      else{  //DSには確認メッセージが必要
        GFL_MSG_GetString( pWork->pMsgData, GAMESYNC_008, pWork->pStrBuf );
        _infoMessageDisp(pWork);
        _PaletteFade(pWork, TRUE);
        _CHANGE_STATE(pWork,_wifiConnectYesNoCheck);
        return;
      }
    }
#if DEBUG_ONLY_FOR_ohno
    if((pWork->selectType == GAMESYNC_RETURNMODE_UTIL)||
       (pWork->selectType == GAMESYNC_RETURNMODE_SYNC))
#else
    if((pWork->selectType == GAMESYNC_RETURNMODE_UTIL)||
       (pWork->selectType == GAMESYNC_RETURNMODE_SYNC))
#endif
    {
      _CHANGE_STATE(pWork, _modeReportInit);
    }
    else{
      _CHANGE_STATE(pWork, _modeFadeoutStart);        // 終わり
    }
  }
}

static void _modeAppWinFlash2(GAMESYNC_MENU* pWork)
{
}



//------------------------------------------------------------------------------
/**
 * @brief   タッチした際に画面が点滅
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeButtonFlash(GAMESYNC_MENU* pWork)
{
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
    //    APP_TASKMENU_WIN_Delete( pWork->pAppWin );
    //   pWork->pAppWin = NULL;

    if(WIRELESSSAVE_ON == CONFIG_GetWirelessSaveMode(SaveData_GetConfig(pWork->dbw->ctrl))){
      _CHANGE_STATE(pWork, _modeReportInit);
    }
    else{
      _CHANGE_STATE(pWork, _modeFadeoutStart);        // 終わり
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   受け取った数のウインドウを等間隔に作る 幅は3char
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _buttonWindowCreate(int num,int* pMsgBuff,GAMESYNC_MENU* pWork, _WINDOWPOS* pos)
{
  int i;
  u32 cgx;
  int frame = GFL_BG_FRAME3_S;

  pWork->windowNum = num;

  GFL_FONTSYS_SetDefaultColor();


  for(i = 0;i < _WINDOW_MAXNUM;i++){
    if(pWork->buttonWin[i]){
      GFL_BMPWIN_ClearScreen(pWork->buttonWin[i]);
      BmpWinFrame_Clear(pWork->buttonWin[i], WINDOW_TRANS_OFF);
      GFL_BMPWIN_Delete(pWork->buttonWin[i]);
    }
    pWork->buttonWin[i] = NULL;
  }

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL2, 0x20, pWork->heapID);


  
  for(i=0;i < num;i++){
    pWork->buttonWin[i] = GFL_BMPWIN_Create(
      frame,
      pos[i].leftx, pos[i].lefty,
      pos[i].width, pos[i].height,
      _BUTTON_MSG_PAL2,GFL_BMP_CHRAREA_GET_F);
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0 );
    // システムウインドウ枠描画
    GFL_MSG_GetString(  pWork->pMsgData, pMsgBuff[i], pWork->pStrBuf );

    GFL_FONTSYS_SetColor(15, 2, 0);

    PRINTSYS_Print(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0, 0,
                   pWork->pStrBuf, pWork->pFontHandle);
    GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
    GFL_BMPWIN_MakeScreen(pWork->buttonWin[i]);
  }
  if(pWork->pButton){
    GFL_BMN_Delete(pWork->pButton);
  }
  pWork->pButton = NULL;
  GFL_BG_LoadScreenReq(frame);
  //  GFL_BG_SetVisible(frame,VISIBLE_ON);
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

static void _buttonWindowDelete(GAMESYNC_MENU* pWork)
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
  GAMESYNC_MENU *pWork = p_work;
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

static void _CreateButtonObj(GAMESYNC_MENU* pWork)
{
  int i;
  u8 buffx[]={ 128,    128,  224};
  u8 buffy[]={ 192/2 , 192/2, 177};
  u8 buttonno[]={14,13, 0};


  for(i=0;i<_SELECTMODE_MAX;i++){
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = buffx[i];
    cellInitData.pos_y = buffy[i];
    cellInitData.anmseq = buttonno[i];
    cellInitData.softpri = 0;
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
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeInit(GAMESYNC_MENU* pWork)
{

  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_gsync_dat, pWork->heapID );
  pWork->pMsgWiFiData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_system_dat, pWork->heapID );

  //    GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );

  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_CG_COMM, pWork->heapID );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_cg_comm_comm_bg_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_cg_comm_background_NCLR,
                                      PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);

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

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_cg_comm_comm_wifi_btn_NSCR,
                                              GFL_BG_FRAME2_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);

    GFL_ARC_CloseDataHandle(p_handle);
  }



  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME1_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);



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
  _CreateButtonObj(pWork);

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuInit(GAMESYNC_MENU* pWork)
{

  int aMsgBuff[]={GAMESYNC_001};

  _buttonWindowCreate(NELEMS(aMsgBuff), aMsgBuff, pWork,wind_wifi);

  pWork->pButton = GFL_BMN_Create( btn_wifi, _BttnCallBack, pWork,  pWork->heapID );
  pWork->touch = &_modeSelectMenuButtonCallback;


  _CHANGE_STATE(pWork,_modeSelectMenuWait);

}

static void _workEnd(GAMESYNC_MENU* pWork)
{
  GFL_FONTSYS_SetDefaultColor();

  _buttonWindowDelete(pWork);
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_S, 1, 0);
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
  GFL_MSG_Delete( pWork->pMsgData );
  GFL_MSG_Delete( pWork->pMsgWiFiData );
  GFL_FONT_Delete(pWork->pFontHandle);
  GFL_STR_DeleteBuffer(pWork->pStrBuf);
  //  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );

}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理 + 通信終了確認
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeout(GAMESYNC_MENU* pWork)
{
  if(WIPE_SYS_EndCheck()){
    switch(pWork->selectType){
    case GAMESYNC_RETURNMODE_SYNC:
    case GAMESYNC_RETURNMODE_UTIL:
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
 * @brief   キーを押したら最初に戻る
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _hitAnyKey(GAMESYNC_MENU* pWork)
{
  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  if(GFL_UI_TP_GetTrg()){
    _infoMessageEnd(pWork);
    pWork->bitold = 0;   //ビットをリセットする事で表示を元に戻す
    PMSND_PlaySystemSE(_SE_DESIDE);
    _PaletteFade(pWork, FALSE);
    _CHANGE_STATE(pWork, _modeSelectMenuInit); //戻る
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeAppWinFlash(GAMESYNC_MENU* pWork)
{
  GFL_CLWK_ANM_CALLBACK cbwk;

  cbwk.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM ;  // CLWK_ANM_CALLBACK_TYPE
  cbwk.param = (u32)pWork;          // コールバックワーク
  cbwk.p_func = _modeAppWinFlashCallback; // コールバック関数
  GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[pWork->bttnid],TRUE);
  GFL_CLACT_WK_StartAnmCallBack( pWork->buttonObj[pWork->bttnid], &cbwk );
  GFL_CLACT_WK_ResetAnm( pWork->buttonObj[pWork->bttnid] );

  _CHANGE_STATE(pWork,_modeAppWinFlash2);

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectMenuButtonCallback(int bttnid,GAMESYNC_MENU* pWork)
{
  pWork->bttnid=bttnid;

  switch( bttnid ){
  case _SELECTMODE_GSYNC:
//    if(GAME_COMM_SBIT_WIFI_ALL & pWork->bit){
    PMSND_PlaySystemSE(_SE_DESIDE);
    GFL_CLACT_WK_SetAnmSeq( pWork->buttonObj[0], 14 );
    _CHANGE_STATE(pWork,_modeAppWinFlash);
    pWork->selectType = GAMESYNC_RETURNMODE_SYNC;
//    }
    //    else{
    //      PMSND_PlaySystemSE(_SE_CANCEL);
    //    }
    break;
  case _SELECTMODE_UTIL:
    PMSND_PlaySystemSE(_SE_DESIDE);
    pWork->selectType = GAMESYNC_RETURNMODE_UTIL;
    _CHANGE_STATE(pWork,_modeAppWinFlash);
    break;
  case _SELECTMODE_EXIT:
    PMSND_PlaySystemSE(_SE_CANCEL);
    //    APP_TASKMENU_WIN_SetDecide(pWork->pAppWin, TRUE);
    pWork->selectType = GAMESYNC_RETURNMODE_NONE;
    _CHANGE_STATE(pWork,_modeAppWinFlash);        // 終わり
    break;
  default:
    break;
  }


  return TRUE;
}

static void _UpdatePalletAnimeSingle(GAMESYNC_MENU* pWork , u16 anmCnt , u8 srcPltNo,  u8 distPltNo)
{
  int i;
  const u16* pal = (u16*)&pWork->BackupPalette[32*srcPltNo];
  const u16* lpal = (u16*)&pWork->LightPalette[32*distPltNo];

  for(i = 0;i<16;i++){
    //1～0に変換
    const u32 br = _PALETTE_R(pal[i]);
    const u32 bg = _PALETTE_G(pal[i]);
    const u32 bb = _PALETTE_B(pal[i]);

    const fx16 cos = (FX_CosIdx(anmCnt)+FX16_ONE)/2;
    const u8 r = br + (((_PALETTE_R(lpal[i])-br)*cos)>>FX16_SHIFT);
    const u8 g = bg + (((_PALETTE_G(lpal[i])-bg)*cos)>>FX16_SHIFT);
    const u8 b = bb + (((_PALETTE_B(lpal[i])-bb)*cos)>>FX16_SHIFT);
    u16 palx[16];

    pWork->TransPalette[i] = GX_RGB(r, g, b);
    //    OS_TPrintf("%d pal %x  %x\n",i,pal[i],pWork->TransPalette[i]);
    {
      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB ,
                                          srcPltNo * 32,  pWork->TransPalette , 32 );
    }
  }
}

static void _changeGsyncAnime(GAMESYNC_MENU* pWork , int no)
{
  if(no != GFL_CLACT_WK_GetAnmSeq(pWork->buttonObj[0])){
    GFL_CLACT_WK_SetAutoAnmFlag(pWork->buttonObj[0],TRUE);
    GFL_CLACT_WK_SetAnmSeq( pWork->buttonObj[0], no );
  }
}


//--------------------------------------------------------------
//	パレットアニメーションの更新
//--------------------------------------------------------------
static void _UpdatePalletAnime(GAMESYNC_MENU* pWork )
{
  //プレートアニメ
  if(pWork->bit != pWork->bitold){
    if(GAME_COMM_STATUS_BIT_WIFI & pWork->bit){  //緑 wep登録
      _changeGsyncAnime(pWork,24);
    }
    else if(GAME_COMM_STATUS_BIT_WIFI_ZONE & pWork->bit){  //青 スポット
      _changeGsyncAnime(pWork,22);
    }
    else if(GAME_COMM_STATUS_BIT_WIFI_FREE & pWork->bit){  //黄色  フリー
//      _changeGsyncAnime(pWork,23);  //黄色
      _changeGsyncAnime(pWork,21);  //赤
    }
    else if(GAME_COMM_STATUS_BIT_WIFI_LOCK & pWork->bit){   //赤  鍵あり
      _changeGsyncAnime(pWork,21);
    }
    else{
      _changeGsyncAnime(pWork,14);
    }
  }



}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(GAMESYNC_MENU* pWork)
{
  if(WIPE_SYS_EndCheck()){
    GFL_BMN_Main( pWork->pButton );
  }
  _UpdatePalletAnime(pWork);
}



#if 1


static void _PaletteFadeSingle(GAMESYNC_MENU* pWork, int type, int palettenum)
{
  u32 addr;
  PALETTE_FADE_PTR pP = PaletteFadeInit(pWork->heapID);
  PaletteFadeWorkAllocSet(pP, type, 16 * 32, pWork->heapID);
  PaletteWorkSet_VramCopy( pP, type, 0, palettenum*32);
  SoftFadePfd(pP, type, 0, 16 * palettenum, 6, 0);
  addr = (u32)PaletteWorkTransWorkGet( pP, type );

  DC_FlushRange((void*)addr, palettenum * 32);
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



static void _PaletteFade(GAMESYNC_MENU* pWork,BOOL bFade)
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
 * @brief   はいいいえウインドウ
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _YesNoStart(GAMESYNC_MENU* pWork)
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
  GFL_MSG_GetString(pWork->pMsgData, GAMESYNC_005, pWork->appitem[0].str);
  pWork->appitem[0].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pWork->appitem[1].str = GFL_STR_CreateBuffer(100, pWork->heapID);
  GFL_MSG_GetString(pWork->pMsgData, GAMESYNC_006, pWork->appitem[1].str);
  pWork->appitem[1].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  pWork->pAppTask			= APP_TASKMENU_OpenMenu(&appinit,pWork->pAppTaskRes);
  APP_TASKMENU_SetDisableKey(pWork->pAppTask, TRUE);  //キー抑制
  GFL_STR_DeleteBuffer(pWork->appitem[0].str);
  GFL_STR_DeleteBuffer(pWork->appitem[1].str);

}


//------------------------------------------------------------------------------
/**
 * @brief   メッセージの画面消去
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _infoMessageDispClear(GAMESYNC_MENU* pWork)
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

static void _infoMessageEnd(GAMESYNC_MENU* pWork)
{
  _infoMessageDispClear(pWork);
  GFL_BMPWIN_Delete(pWork->infoDispWin);
  pWork->infoDispWin=NULL;
}

//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了待ち
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _infoMessageEndCheck(GAMESYNC_MENU* pWork)
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
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _infoMessageDispHeight(GAMESYNC_MENU* pWork,int height,BOOL bStream)
{
  GFL_BMPWIN* pwin;


  if(pWork->infoDispWin==NULL){
    pWork->infoDispWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME1_S ,
      1 , 3, 30 , height ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->infoDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  if(bStream){
    pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pStrBuf, pWork->pFontHandle,
                                          MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);
  }
  else{
    PRINTSYS_Print(GFL_BMPWIN_GetBmp(pwin), 0, 0, pWork->pStrBuf, pWork->pFontHandle);
  }

  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
}


static void _infoMessageDisp(GAMESYNC_MENU* pWork)
{
  _infoMessageDispHeight( pWork,4,TRUE);
}


//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportInit(GAMESYNC_MENU* pWork)
{
  _PaletteFade(pWork, TRUE);

  GFL_BG_ClearScreenCodeVReq(GFL_BG_FRAME1_S,0);

  GFL_MSG_GetString( pWork->pMsgData, GAMESYNC_004, pWork->pStrBuf );

  _infoMessageDisp(pWork);


  _CHANGE_STATE(pWork,_modeReportWait);
}


static void _FadeWait(GAMESYNC_MENU* pWork)
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
static void _modeReporting2(GAMESYNC_MENU* pWork)
{
  SAVE_RESULT svr = GAMEDATA_SaveAsyncMain(pWork->gamedata);

  if(svr == SAVE_RESULT_OK){
    _infoMessageEnd(pWork);
    _PaletteFade(pWork, FALSE);
    _CHANGE_STATE(pWork,_modeFadeoutStart);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   セーブ中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReporting(GAMESYNC_MENU* pWork)
{
  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  GAMEDATA_SaveAsyncStart(pWork->gamedata);
  _CHANGE_STATE(pWork,_modeReporting2);
}

//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait2(GAMESYNC_MENU* pWork)
{

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){
      if(SaveControl_IsOverwritingOtherData( GAMEDATA_GetSaveControlWork(pWork->gamedata))){
        GFL_MSG_GetString( pWork->pMsgData, GSYNC_027, pWork->pStrBuf );
        _infoMessageDisp(pWork);
        _CHANGE_STATE(pWork,  _hitAnyKey );
      }
      else{
        GFL_MSG_GetString( pWork->pMsgData, GAMESYNC_007, pWork->pStrBuf );
#if 0   //20100603 del Saito        
        _infoMessageDisp(pWork);
#else
        //一括表示
        _infoMessageDispHeight( pWork,4,FALSE);
#endif
        _MESSAGE_WindowTimeIconStart(pWork);
        //セーブ開始
        _CHANGE_STATE(pWork,_modeReporting);
      }
    }
    else{
      GFL_BG_ClearScreen(GFL_BG_FRAME1_S);
      pWork->selectType = GAMESYNC_RETURNMODE_NONE;
      _CHANGE_STATE(pWork, _modeSelectMenuInit); //戻る
      _PaletteFade(pWork, FALSE);
      
    }
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait(GAMESYNC_MENU* pWork)
{
  if(!_infoMessageEndCheck(pWork)){
    return;
  }
  _YesNoStart(pWork);
  _CHANGE_STATE(pWork,_modeReportWait2);
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
  //  GAMESYNC_MENU *pWork=work;

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
static GFL_PROC_RESULT GameSyncMenuProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_GSYNC_WORK* pParentWork =pwk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, 0x18000 );

  {
    GAMESYNC_MENU *pWork = GFL_PROC_AllocWork( proc, sizeof( GAMESYNC_MENU ), HEAPID_IRCBATTLE );
    GFL_STD_MemClear(pWork, sizeof(GAMESYNC_MENU));
    pWork->heapID = HEAPID_IRCBATTLE;
    pWork->gamedata = pParentWork->gameData;
    pWork->gsys = pParentWork->gsys;

    GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);

    //初期マスター輝度設定　WiFi設定から戻ってきたときのために仕込んでおきます    20100606 add Saito
    {
      int brightness;
      if (pParentWork->white_in) brightness = 16;
      else brightness = -16;
      
      GX_SetMasterBrightness(brightness);
      GXS_SetMasterBrightness(brightness);
    }

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

    pWork->pVramOBJ = GFL_HEAP_AllocMemory(pWork->heapID, 16*2*16);  //バックアップ
    pWork->pVramBG = GFL_HEAP_AllocMemory(pWork->heapID, 16*2*16);  //バックアップ
    pWork->pMsgTcblSys = GFL_TCBL_Init( pWork->heapID , pWork->heapID , 3 , 0 );
    pWork->pKeyCursor = APP_KEYCURSOR_Create( 15, FALSE, TRUE, pWork->heapID );
    pWork->SysMsgQue = PRINTSYS_QUE_Create( pWork->heapID );
    pWork->pAppTaskRes =
      APP_TASKMENU_RES_Create( GFL_BG_FRAME1_S, _SUBLIST_NORMAL_PAL,
                               pWork->pFontHandle, pWork->SysMsgQue, pWork->heapID  );

    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG0 , 15, 4 );
#if 0                                                                                     //20100606 del Saito
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
#else
    {
      int fade_col;
      if (pParentWork->white_in) fade_col = WIPE_FADE_WHITE;
      else fade_col = WIPE_FADE_BLACK;
      WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                    fade_col , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
      //リクエストしたら、フェードカラーをデフォルトに戻しておく
      pParentWork->white_in = FALSE;
    }
#endif
    GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
    _CHANGE_STATE(pWork,_modeSelectMenuInit);
    pWork->dbw = pwk;
    GFL_NET_ReloadIconTopOrBottom(FALSE, pWork->heapID);
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
static GFL_PROC_RESULT GameSyncMenuProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMESYNC_MENU* pWork = mywk;
  GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;

  StateFunc* state = pWork->state;
  if(state != NULL){
    state(pWork);
    retCode = GFL_PROC_RES_CONTINUE;
  }

  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }
  //  if(pWork->pAppWin){
  //    APP_TASKMENU_WIN_Update( pWork->pAppWin );
  //  }
  if(GFL_NET_IsInit()){
    pWork->bitold =  pWork->bit;
    pWork->bit = WIH_DWC_GetAllBeaconTypeBit(NULL);
  }

  GFL_TCBL_Main( pWork->pMsgTcblSys );
  PRINTSYS_QUE_Main(pWork->SysMsgQue);
  GFL_BG_SetScroll( GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, pWork->yoffset );
  pWork->yoffset--;
  GFL_CLACT_SYS_Main();

  if(pWork->endStart==FALSE){
    GAMESYSTEM_CommBootAlways(pWork->gsys);
  }
  

  if(WIPE_SYS_EndCheck()){
    if(GAMESYSTEM_IsBatt10Sleep(pWork->gsys)){
      retCode= GFL_PROC_RES_FINISH;
      pWork->selectType = GAMESYNC_RETURNMODE_NONE;
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
static GFL_PROC_RESULT GameSyncMenuProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMESYNC_MENU* pWork = mywk;
  EVENT_GSYNC_WORK* pParentWork =pwk;
  int i;

  if(!WIPE_SYS_EndCheck()){
    return GFL_PROC_RES_CONTINUE;
  }

  for(i=0;i<_SELECTMODE_MAX; i++){
    GFL_CLACT_WK_Remove(pWork->buttonObj[i]);
  }
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_OBJ] );
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_OBJ] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_OBJ] );
  GFL_TCB_DeleteTask( pWork->g3dVintr );
  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  GFL_CLACT_SYS_Delete();
  if(pWork->pTimeIcon){
    TIMEICON_Exit(pWork->pTimeIcon);
    pWork->pTimeIcon=NULL;
  }
  if(pWork->pStream){
    PRINTSYS_PrintStreamDelete( pWork->pStream );
  }
  if(pWork->pAppTask){
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
  _workEnd(pWork);
  pParentWork->selectType = pWork->selectType;

  GFL_PROC_FreeWork(proc);

  GFL_HEAP_FreeMemory(pWork->pVramOBJ);
  GFL_HEAP_FreeMemory(pWork->pVramBG);
  APP_KEYCURSOR_Delete( pWork->pKeyCursor );
  GFL_BG_FreeBGControl(_SUBSCREEN_BGPLANE);
  PRINTSYS_QUE_Clear(pWork->SysMsgQue);
  PRINTSYS_QUE_Delete(pWork->SysMsgQue);
  if(pWork->infoDispWin){
    GFL_BMPWIN_Delete(pWork->infoDispWin);
  }
  GFL_TCBL_Exit(pWork->pMsgTcblSys);

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
const GFL_PROC_DATA GameSyncMenuProcData = {
  GameSyncMenuProcInit,
  GameSyncMenuProcMain,
  GameSyncMenuProcEnd,
};


