//=============================================================================
/**
 * @file	  c_gear.c
 * @brief	  コミュニケーションギア
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "c_gear.h"
#include "system/main.h"  //HEAPID
#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "sound/pm_sndsys.h"

#include "c_gear.naix"

#include "msg/msg_c_gear.h"

#define _NET_DEBUG (1)  //デバッグ時は１
#define _BRIGHTNESS_SYNC (0)  // フェードのＳＹＮＣは要調整

// サウンド仮想ラベル
#define GEAR_SE_DECIDE_ (SEQ_SE_DP_DECIDE)
#define GEAR_SE_CANCEL_ (SEQ_SE_DP_SELECT)


//--------------------------------------------
// 画面構成定義
//--------------------------------------------
#define _WINDOW_MAXNUM (4)   //ウインドウのパターン数

#define _MESSAGE_BUF_NUM	( 100*2 )

#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (3)    // ウインドウ高さ
#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (13)  // メッセージフォント

#define	_BUTTON_WIN_CGX_SIZE   ( 18+12 )
#define	_BUTTON_WIN_CGX	( 2 )


#define	_BUTTON_FRAME_CGX		( _BUTTON_WIN_CGX + ( 23 * 16 ) )	// 通信システムウィンドウ転送先

#define	FBMP_COL_WHITE		(15)


//-------------------------

#define PANEL_Y1 (6)
#define PANEL_Y2 (4)
#define PANEL_X1 (-2)

#define PANEL_HEIDHT1 (C_GEAR_PANEL_HEIGHT-1)
#define PANEL_HEIGHT2 (C_GEAR_PANEL_HEIGHT)

#define PANEL_WIDTH (C_GEAR_PANEL_WIDTH)
#define PANEL_SIZEXY (4)   //



//--------------------------


typedef struct {
  int leftx;
  int lefty;
  int width;
  int height;
} _WINDOWPOS;




static const GFL_UI_TP_HITTBL bttndata[] = {  //上下左右
	//タッチパネル全部
	{	PANEL_Y2 * 8,  PANEL_Y2 * 8 + (PANEL_SIZEXY * 8 * PANEL_HEIGHT2), 0,32*8-1 },
  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};



//-------------------------------------------------------------------------
///	文字表示色定義(default)	-> gflib/fntsys.hへ移動
//------------------------------------------------------------------

#define WINCLR_COL(col)	(((col)<<4)|(col))



//--------------------------------------------
// 内部ワーク
//--------------------------------------------

enum _BATTLETYPE_SELECT {
  _SELECTBT_SINGLE = 0,
  _SELECTBT_DOUBLE,
  _SELECTBT_TRI,
  _SELECTBT_EXIT
};


enum _IBMODE_SELECT {
  _SELECTMODE_BATTLE = 0,
  _SELECTMODE_POKE_CHANGE,
  _SELECTMODE_EXIT
};

enum _IBMODE_ENTRY {
  _ENTRYNUM_DOUBLE = 0,
  _ENTRYNUM_FOUR,
  _ENTRYNUM_EXIT,
};

enum _IBMODE_CHANGE {
  _CHANGE_FRIENDCHANGE = 0,
  _CHANGE_EXIT,
};


#define GEAR_MAIN_FRAME   (GFL_BG_FRAME2_S)
#define GEAR_BMPWIN_FRAME   (GFL_BG_FRAME1_S)
#define GEAR_BUTTON_FRAME   (GFL_BG_FRAME0_S)


typedef void (StateFunc)(C_GEAR_WORK* pState);
typedef BOOL (TouchFunc)(int no, C_GEAR_WORK* pState);


struct _C_GEAR_WORK {
  StateFunc* state;      ///< ハンドルのプログラム状態
  TouchFunc* touch;
  int selectType;   // 接続タイプ
  HEAPID heapID;
  GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
  GFL_BUTTON_MAN* pButton;
  GFL_MSGDATA *pMsgData;  //
  WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
  GFL_FONT* pFontHandle;
  STRBUF* pStrBuf;
  u32 bgchar;  //GFL_ARCUTIL_TRANSINFO
  //    BMPWINFRAME_AREAMANAGER_POS aPos;
  GFL_ARCUTIL_TRANSINFO subchar;
  int windowNum;
  BOOL IsIrc;
  GAMESYS_WORK *gameSys_;
  FIELD_MAIN_WORK *fieldWork_;
  GMEVENT* event_;
	CGEAR_SAVEDATA* pCGSV;
};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(C_GEAR_WORK* pWork,StateFunc* state);
static void _changeStateDebug(C_GEAR_WORK* pWork,StateFunc* state, int line);
static void _buttonWindowCreate(int num,int* pMsgBuff,C_GEAR_WORK* pWork);
static void _modeSelectMenuInit(C_GEAR_WORK* pWork);
static void _modeSelectMenuWait(C_GEAR_WORK* pWork);
static void _modeSelectEntryNumInit(C_GEAR_WORK* pWork);
static void _modeSelectEntryNumWait(C_GEAR_WORK* pWork);
static void _modeReportInit(C_GEAR_WORK* pWork);
static void _modeReportWait(C_GEAR_WORK* pWork);
static BOOL _modeSelectMenuButtonCallback(int bttnid,C_GEAR_WORK* pWork);
static BOOL _modeSelectEntryNumButtonCallback(int bttnid,C_GEAR_WORK* pWork);
static BOOL _modeSelectBattleTypeButtonCallback(int bttnid,C_GEAR_WORK* pWork);
static void _modeSelectBattleTypeInit(C_GEAR_WORK* pWork);
static BOOL _modeSelectChangeButtonCallback(int bttnid,C_GEAR_WORK* pWork);
static void _modeSelectChangWait(C_GEAR_WORK* pWork);
static void _modeSelectChangeInit(C_GEAR_WORK* pWork);



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

static void _changeState(C_GEAR_WORK* pWork,StateFunc state)
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
static void _changeStateDebug(C_GEAR_WORK* pWork,StateFunc state, int line)
{
  NET_PRINT("ircbtl: %d\n",line);
  _changeState(pWork, state);
}
#endif



//------------------------------------------------------------------------------
/**
 * @brief   パネルタイプをスクリーンに書き込む
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _gearPanelBgScreenMake(C_GEAR_WORK* pWork,int xs,int ys, CGEAR_PANELTYPE_ENUM type)
{
  int ypos[2] = {PANEL_Y1,PANEL_Y2};
	int x,y,i,j;
	int typepos[] = {0,0x60,0x64,0x68};
	int palpos[] =  {0,0x1000,0x2000,0x3000};
	u16* pScrAddr = GFL_BG_GetScreenBufferAdrs(GEAR_BUTTON_FRAME );
	int xscr = PANEL_X1 + xs * PANEL_SIZEXY;
	int yscr = ypos[ xs % 2 ] + ys * PANEL_SIZEXY;

	for(y = yscr, i = 0; i < PANEL_SIZEXY; y++, i++){
		for(x = xscr, j = 0; j < PANEL_SIZEXY; x++, j++){
			if((x >= 0) && (x < 32)){
				int charpos = typepos[type] + i * 0x20 + j;
				int scr = x + (y * 32);
				if(type == CGEAR_PANELTYPE_NONE){
					charpos = 0;
				}
				pScrAddr[scr] = palpos[type] + charpos;
		//		NET_PRINT("x%d y%d  %d \n",x,y,palpos[type] + charpos);
			}
		}
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   ギアのパネルをセーブデータにしたがって作る
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _gearPanelBgCreate(C_GEAR_WORK* pWork)
{
  int x , y;
  int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};

  for(x = 0; x < PANEL_WIDTH; x++){   // XはPANEL_WIDTH回
    for(y = 0; y < yloop[ x % 2]; y++){ //Yは xの％２でyloopの繰り返し
      _gearPanelBgScreenMake(pWork, x, y, CGEAR_SV_GetPanelType(pWork->pCGSV,x,y));
    }
  }
	GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
}

//------------------------------------------------------------------------------
/**
 * @brief   ギアのBGを作る
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _gearBgCreate(C_GEAR_WORK* pWork)
{
  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_c_gear_c_gear_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  pWork->heapID);
    // サブ画面BGキャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_c_gear_c_gear_NCGR,
                                                                  GEAR_MAIN_FRAME, 0, 0, pWork->heapID);

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
                                           NARC_c_gear_c_gear01_NSCR,
                                           GEAR_MAIN_FRAME, 0,
                                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                           pWork->heapID);

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
                                           NARC_c_gear_c_gear00_NSCR,
                                           GEAR_BMPWIN_FRAME, 0,
                                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                           pWork->heapID);

		// パネル作成
		_gearPanelBgCreate(pWork);

    //パレットアニメシステム作成
//    ConnectBGPalAnm_Init(&pWork->cbp, p_handle, NARC_ircbattle_connect_anm_NCLR, pWork->heapID);
    GFL_ARC_CloseDataHandle( p_handle );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   サブ画面の設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _createSubBg(C_GEAR_WORK* pWork)
{
  int i = 0;
  for(i = GFL_BG_FRAME0_S;i <= GFL_BG_FRAME3_S ; i++)
  {
    GFL_BG_SetVisible( i, VISIBLE_OFF );
  }
  {
    int frame = GEAR_MAIN_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, 2 );
  //  GFL_BG_FillCharacter( frame, 0x00, 1, 0 );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GEAR_BMPWIN_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, 0 );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }

  {
    int frame = GEAR_BUTTON_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, 1 );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  
  //  G2S_SetBlendAlpha( GEAR_MAIN_FRAME, GEAR_BMPWIN_FRAME , 3, 16 );
 //   G2S_SetBlendAlpha( GEAR_MAIN_FRAME, GEAR_BUTTON_FRAME , 16, 16 );
  G2S_SetBlendAlpha( GEAR_MAIN_FRAME|GEAR_BUTTON_FRAME, GEAR_BMPWIN_FRAME , 3, 16 );

}


//------------------------------------------------------------------------------
/**
 * @brief   受け取った数のウインドウを等間隔に作る 幅は3char
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _buttonWindowCreate(int num,int* pMsgBuff,C_GEAR_WORK* pWork)
{
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

static void _buttonWindowDelete(C_GEAR_WORK* pWork)
{
  int i;

  GFL_BMN_Delete(pWork->pButton);
  pWork->pButton = NULL;
  for(i=0;i < pWork->windowNum;i++){
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0 );
    GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
    GFL_BMPWIN_Delete(pWork->buttonWin[i]);
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
  C_GEAR_WORK *pWork = p_work;
  u32 friendNo;
	u32 touchx,touchy;
	int xp,yp;

  switch( event ){
  case GFL_BMN_EVENT_TOUCH:		///< 触れた瞬間
		if(GFL_UI_TP_GetPointCont(&touchx,&touchy)){
			int ypos[2] = {PANEL_Y1,PANEL_Y2};
			int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};
			touchx = touchx / 8;
			touchy = touchy / 8;
			xp = (touchx - PANEL_X1) / PANEL_SIZEXY;
			yp = (touchy - ypos[xp % 2]) / PANEL_SIZEXY;
			if((xp < C_GEAR_PANEL_WIDTH) && (yp < yloop[ xp % 2 ])){
				int type = CGEAR_SV_GetPanelType(pWork->pCGSV,xp,yp);
				CGEAR_SV_SetPanelType(pWork->pCGSV,xp,yp,(type+1) % CGEAR_PANELTYPE_MAX);

				_gearPanelBgScreenMake(pWork, xp, yp, CGEAR_SV_GetPanelType(pWork->pCGSV,xp,yp));
				GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );

			}
		}
    break;

  default:
    break;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeInit(C_GEAR_WORK* pWork)
{
  _createSubBg(pWork);
  _gearBgCreate(pWork);
  pWork->IsIrc=FALSE;

//  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
//  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
//  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_c_gear_dat, pWork->heapID );
  _CHANGE_STATE(pWork,_modeSelectMenuInit);
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuInit(C_GEAR_WORK* pWork)
{
  pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );
  pWork->touch = &_modeSelectMenuButtonCallback;

  _CHANGE_STATE(pWork,_modeSelectMenuWait);
}

static void _workEnd(C_GEAR_WORK* pWork)
{
  GFL_FONTSYS_SetDefaultColor();

	if(pWork->pButton){
		GFL_BMN_Delete(pWork->pButton);
	}
	
  //    _buttonWindowDelete(pWork);
//  GFL_BG_FillCharacterRelease( GEAR_MAIN_FRAME, 1, 0);
//  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
//                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));

  GFL_BG_FreeCharacterArea(GEAR_MAIN_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subchar));
  
  GFL_BG_FreeBGControl(GEAR_BUTTON_FRAME);
  GFL_BG_FreeBGControl(GEAR_BMPWIN_FRAME);
  GFL_BG_FreeBGControl(GEAR_MAIN_FRAME);

  if(pWork->pMsgData)
  {
    GFL_MSG_Delete( pWork->pMsgData );
  }
  if(pWork->pFontHandle){
    GFL_FONT_Delete(pWork->pFontHandle);
  }
  if(pWork->pStrBuf)
  {
    GFL_STR_DeleteBuffer(pWork->pStrBuf);
  }
  GFL_BG_SetVisible( GEAR_BUTTON_FRAME, VISIBLE_OFF );
  GFL_BG_SetVisible( GEAR_BMPWIN_FRAME, VISIBLE_OFF );
  GFL_BG_SetVisible( GEAR_MAIN_FRAME, VISIBLE_OFF );

}



//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectMenuButtonCallback(int bttnid,C_GEAR_WORK* pWork)
{
  switch( bttnid ){
  case _SELECTMODE_BATTLE:
    _CHANGE_STATE(pWork,_modeSelectEntryNumInit);
    _buttonWindowDelete(pWork);
    return TRUE;
  case _SELECTMODE_POKE_CHANGE:
    _CHANGE_STATE(pWork,_modeSelectChangeInit);
    _buttonWindowDelete(pWork);
    return TRUE;
  case _SELECTMODE_EXIT:
    _CHANGE_STATE(pWork,NULL);        // 終わり
    _buttonWindowDelete(pWork);
    return TRUE;
  default:
    break;
  }
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(C_GEAR_WORK* pWork)
{
  GFL_BMN_Main( pWork->pButton );

}


//------------------------------------------------------------------------------
/**
 * @brief   交換画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectChangeInit(C_GEAR_WORK* pWork)
{
  int aMsgBuff[]={gear_001, gear_001};

  _buttonWindowCreate(NELEMS(aMsgBuff),aMsgBuff,pWork);

  pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );
  pWork->touch = &_modeSelectChangeButtonCallback;

  _CHANGE_STATE(pWork,_modeSelectChangWait);

}

//------------------------------------------------------------------------------
/**
 * @brief   交換画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectChangeButtonCallback(int bttnid,C_GEAR_WORK* pWork)
{
  switch(bttnid){
  case _CHANGE_FRIENDCHANGE:
    //    pWork->selectType = EVENTIRCBTL_ENTRYMODE_FRIEND;
    _buttonWindowDelete(pWork);
    _CHANGE_STATE(pWork,_modeReportInit);
    return TRUE;
  default:
    break;
  }
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   交換画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectChangWait(C_GEAR_WORK* pWork)
{
  GFL_BMN_Main( pWork->pButton );

}






//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectEntryNumInit(C_GEAR_WORK* pWork)
{
  int aMsgBuff[]={gear_001,gear_001,gear_001};

  _buttonWindowCreate(3,aMsgBuff,pWork);

  pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );
  pWork->touch = &_modeSelectEntryNumButtonCallback;

  _CHANGE_STATE(pWork,_modeSelectEntryNumWait);

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectEntryNumButtonCallback(int bttnid,C_GEAR_WORK* pWork)
{
  switch(bttnid){
  case _ENTRYNUM_DOUBLE:
    _buttonWindowDelete(pWork);
    _CHANGE_STATE(pWork,_modeSelectBattleTypeInit);
    return TRUE;
  case _ENTRYNUM_FOUR:
    //    pWork->selectType = EVENTIRCBTL_ENTRYMODE_MULTH;
    _buttonWindowDelete(pWork);
    _CHANGE_STATE(pWork,_modeReportInit);
    return TRUE;
  case _ENTRYNUM_EXIT:
    _buttonWindowDelete(pWork);
    _CHANGE_STATE(pWork,_modeSelectMenuInit);
    return TRUE;
  default:
    break;
  }
  return FALSE;
}



//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectBattleTypeInit(C_GEAR_WORK* pWork)
{
  int aMsgBuff[]={gear_001,gear_001,gear_001,gear_001};

  _buttonWindowCreate(4,aMsgBuff,pWork);

  pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );
  pWork->touch = &_modeSelectBattleTypeButtonCallback;

  _CHANGE_STATE(pWork,_modeSelectEntryNumWait);

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面タッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _modeSelectBattleTypeButtonCallback(int bttnid,C_GEAR_WORK* pWork)
{
  switch(bttnid){
  case _SELECTBT_SINGLE:
    //    pWork->selectType = EVENTIRCBTL_ENTRYMODE_SINGLE;
    _buttonWindowDelete(pWork);
    _CHANGE_STATE(pWork,_modeReportInit);
    break;
  case _SELECTBT_DOUBLE:
    //    pWork->selectType = EVENTIRCBTL_ENTRYMODE_DOUBLE;
    _buttonWindowDelete(pWork);
    _CHANGE_STATE(pWork,_modeReportInit);
    break;
  case _SELECTBT_TRI:
    //    pWork->selectType = EVENTIRCBTL_ENTRYMODE_TRI;
    _buttonWindowDelete(pWork);
    _CHANGE_STATE(pWork,_modeReportInit);
    break;
  default:
    _buttonWindowDelete(pWork);
    _CHANGE_STATE(pWork,_modeSelectMenuInit);
    break;
  }
  return TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectEntryNumWait(C_GEAR_WORK* pWork)
{
  GFL_BMN_Main( pWork->pButton );
}


//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportInit(C_GEAR_WORK* pWork)
{

  //    GAMEDATA_Save(GAMESYSTEM_GetGameData(GMEVENT_GetGameSysWork(event)));

  _CHANGE_STATE(pWork,_modeReportWait);
}

//------------------------------------------------------------------------------
/**
 * @brief   セーブ確認画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeReportWait(C_GEAR_WORK* pWork)
{
  pWork->IsIrc = TRUE;  //赤外線接続開始
  _CHANGE_STATE(pWork,NULL);
}


//------------------------------------------------------------------------------
/**
 * @brief   スタート
 * @retval  none
 */
//------------------------------------------------------------------------------
C_GEAR_WORK* CGEAR_Init( CGEAR_SAVEDATA* pCGSV )
{
  C_GEAR_WORK *pWork = NULL;

  //GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_CGEAR, 0x8000 );

  pWork = GFL_HEAP_AllocClearMemory( HEAPID_FIELDMAP, sizeof( C_GEAR_WORK ) );
  pWork->heapID = HEAPID_FIELDMAP;
	pWork->pCGSV = pCGSV;

  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, _BRIGHTNESS_SYNC);
	
	_CHANGE_STATE( pWork, _modeInit);
  return pWork;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_Main( C_GEAR_WORK* pWork )
{
  StateFunc* state = pWork->state;
  if(state != NULL){
    state(pWork);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_Exit( C_GEAR_WORK* pWork )
{

//  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, _BRIGHTNESS_SYNC);
  _workEnd(pWork);

  GFL_HEAP_FreeMemory(pWork);
//  GFL_HEAP_DeleteHeap(HEAPID_CGEAR);

}


