//============================================================================================
/**
 * @file	  event_debug_item.h
 * @brief	  デバッグアイテムイベント
 * @author	k.ohno
 * @date	  2009.06.30
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"
#include "arc_def.h"

#include "item/item.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "system/bmp_winframe.h"

#include "message.naix"
#include "msg/msg_d_field.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "field/fieldmap.h"
#include "./event_fieldmap_control.h"
#include "event_debug_item.h"
#include "font/font.naix" //NARC_font_large_nftr
#include "sound/pm_sndsys.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "system/main.h"			//GFL_HEAPID_APP参照

//============================================================================================
//============================================================================================

#define DEBUG_ITEMDISP_FRAME (GFL_BG_FRAME1_M)

typedef struct _DEBUGITEM_PARAM EVENT_DEBUGITEM_WORK;
typedef void (StateFunc)(EVENT_DEBUGITEM_WORK* wk);

struct _DEBUGITEM_PARAM {
  StateFunc* state;      ///< ハンドルのプログラム状態
  GMEVENT * event;
  GAMESYS_WORK * gsys;
  FIELD_MAIN_WORK * fieldmap;
  SAVE_CONTROL_WORK *ctrl;

	GFL_BMPWIN* win;
 // BMPMENULIST_WORK* lw;		// BMPメニューワーク
//  BMP_MENULIST_DATA*   menulist;
  GFL_MSGDATA *MsgManager;			// 名前入力メッセージデータマネージャー
  WORDSET			*WordSet;								// メッセージ展開用ワークマネージャー
  STRBUF*  pStrBuf;
	STRBUF*  pExpStrBuf;
  GFL_FONT 			*fontHandle;
	int itemid;
	int itemnum;
	int curpos;
	HEAPID heapID;
	u32 bgchar;
};

#define _DISP_INITX (1)
#define _DISP_INITY (18)
#define _DISP_SIZEX (30)
#define _DISP_SIZEY (4)
#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (11)  // メッセージフォント
#define	FBMP_COL_WHITE		(15)
#define WINCLR_COL(col)	(((col)<<4)|(col))

//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(EVENT_DEBUGITEM_WORK* pWork,StateFunc* state);
static void _changeStateDebug(EVENT_DEBUGITEM_WORK* pWork,StateFunc* state, int line);
static void _windowRewrite(EVENT_DEBUGITEM_WORK* wk, int type);
static void _itemNumSelectMenu(EVENT_DEBUGITEM_WORK* wk);


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

static void _changeState(EVENT_DEBUGITEM_WORK* wk,StateFunc state)
{
  wk->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(EVENT_DEBUGITEM_WORK* wk,StateFunc state, int line)
{
  NET_PRINT("EvDebugItem: %d\n",line);
  _changeState(wk, state);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   アイテム種類変更
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _itemKindSelectMenu(EVENT_DEBUGITEM_WORK* wk)
{
	int id = wk->itemid;

	
	if(GFL_UI_KEY_GetRepeat()==PAD_KEY_UP){
		wk->itemid++;
	}
	if(GFL_UI_KEY_GetRepeat()==PAD_KEY_DOWN){
		wk->itemid--;
	}
	if(GFL_UI_KEY_GetTrg()==PAD_KEY_RIGHT){
		wk->itemid += 10;
	}
	if(GFL_UI_KEY_GetTrg()==PAD_KEY_LEFT){
		wk->itemid -= 10;
	}
	if(wk->itemid > ITEM_DATA_MAX){
		wk->itemid = 0;
	}
	if(wk->itemid < 0){
		wk->itemid = ITEM_DATA_MAX;
	}
	if(id != wk->itemid){
		_windowRewrite(wk,0);
	}
	if(GFL_UI_KEY_GetTrg()==PAD_BUTTON_DECIDE){
		_windowRewrite(wk,1);
		_CHANGE_STATE(wk,_itemNumSelectMenu);
	}
	if(GFL_UI_KEY_GetTrg()==PAD_BUTTON_CANCEL){
		_CHANGE_STATE(wk,NULL);  //終了
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   アイテム数変更
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _itemNumSelectMenu(EVENT_DEBUGITEM_WORK* wk)
{
	int id = wk->itemnum;
	

	if(GFL_UI_KEY_GetRepeat()==PAD_KEY_UP){
		wk->itemnum++;
	}
	if(GFL_UI_KEY_GetRepeat()==PAD_KEY_DOWN){
		wk->itemnum--;
	}
	if(GFL_UI_KEY_GetTrg()==PAD_KEY_RIGHT){
		wk->itemnum += 10;
	}
	if(GFL_UI_KEY_GetTrg()==PAD_KEY_LEFT){
		wk->itemnum -= 10;
	}
	if(wk->itemnum > 999){
		wk->itemnum = 0;
	}
	if(wk->itemnum < 0){
		wk->itemnum = 999;
	}
	if(id != wk->itemnum){
		_windowRewrite(wk,1);
	}
	
	if(GFL_UI_KEY_GetTrg()==PAD_BUTTON_DECIDE){  //追加
		MYITEM_PTR pMyItem = GAMEDATA_GetMyItem(GAMESYSTEM_GetGameData(wk->gsys));
		MYITEM_AddItem(pMyItem,wk->itemid, wk->itemnum,wk->heapID);
		PMSND_PlaySystemSE( SEQ_SE_DECIDE3 );
		_windowRewrite(wk,0);
		_CHANGE_STATE(wk,_itemKindSelectMenu);
		
	}
	if(GFL_UI_KEY_GetTrg()==PAD_BUTTON_START){
		_CHANGE_STATE(wk,NULL);  //終了
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   絵の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _graphicInit(EVENT_DEBUGITEM_WORK* wk)
{
#if 0
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000,
      0x8000,
      GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( DEBUG_ITEMDISP_FRAME, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( DEBUG_ITEMDISP_FRAME );
    GFL_BG_LoadScreenReq( DEBUG_ITEMDISP_FRAME );
    GFL_BG_SetPriority( DEBUG_ITEMDISP_FRAME, 0 );
		GFL_BG_SetVisible( DEBUG_ITEMDISP_FRAME, VISIBLE_ON );
  }
#endif

	GFL_FONTSYS_SetDefaultColor();

	
	wk->bgchar = BmpWinFrame_GraphicSetAreaMan(DEBUG_ITEMDISP_FRAME,
																						 _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, wk->heapID);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
																0x20*_BUTTON_MSG_PAL, 0x20, wk->heapID);
}


//------------------------------------------------------------------------------
/**
 * @brief   ウインドウの生成
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _windowCreate(EVENT_DEBUGITEM_WORK* wk)
{

	wk->win = GFL_BMPWIN_Create(
		DEBUG_ITEMDISP_FRAME,
    _DISP_INITX, _DISP_INITY,
    _DISP_SIZEX, _DISP_SIZEY,
    _BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_B );

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win), WINCLR_COL(FBMP_COL_WHITE) );
  GFL_BMPWIN_MakeScreen( wk->win );
	BmpWinFrame_Write( wk->win, WINDOW_TRANS_ON, GFL_ARCUTIL_TRANSINFO_GetPos(wk->bgchar), _BUTTON_WIN_PAL );

}



static void _windowRewrite(EVENT_DEBUGITEM_WORK* wk, int type)
{
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win), WINCLR_COL(FBMP_COL_WHITE) );
	GFL_MSG_GetString(  wk->MsgManager, DEBUG_FIELD_STR35, wk->pStrBuf );
	//id0 アイテム番号
	//id1 アイテム名
	//id2 アイテム数
  WORDSET_RegisterNumber(wk->WordSet, 0, wk->itemid,
                         3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
	WORDSET_ExpandStr( wk->WordSet, wk->pExpStrBuf, wk->pStrBuf );
	
	if(type==1){
		GFL_FONTSYS_SetColor( 1, 2, 15 );
	}
	else{
		GFL_FONTSYS_SetColor( 3, 4, 15 );
	}
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win), 1, 10, wk->pExpStrBuf, wk->fontHandle);


	GFL_MSG_GetString(  wk->MsgManager, DEBUG_FIELD_STR33, wk->pStrBuf );
	//id0 アイテム番号
	//id1 アイテム名
	//id2 アイテム数
	WORDSET_RegisterItemName( wk->WordSet, 1, wk->itemid);
	WORDSET_ExpandStr( wk->WordSet, wk->pExpStrBuf, wk->pStrBuf  );

	GFL_FONTSYS_SetColor( 1, 2, 15 );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win), 50, 10, wk->pExpStrBuf, wk->fontHandle);

	

	GFL_MSG_GetString(  wk->MsgManager, DEBUG_FIELD_STR34, wk->pStrBuf );

  WORDSET_RegisterNumber(wk->WordSet, 2, wk->itemnum,
                         3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
	WORDSET_ExpandStr( wk->WordSet, wk->pExpStrBuf, wk->pStrBuf );

	if(type==1){
		GFL_FONTSYS_SetColor( 3, 4, 15 );
	}
	else{
		GFL_FONTSYS_SetColor( 1, 2, 15 );
	}
	PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win), 160, 10, wk->pExpStrBuf, wk->fontHandle);

	GFL_BMPWIN_TransVramCharacter(wk->win);
//	GFL_BG_LoadScreenV_Req(DEBUG_ITEMDISP_FRAME);
}


//------------------------------------------------------------------------------
/**
 * @brief   デバッグプロセス初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT DebugItemMakeProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_DEBUGITEM_WORK* wk = pwk;
	wk->curpos = 0;
  wk->MsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
																	 NARC_message_d_field_dat, wk->heapID );
	wk->pStrBuf = GFL_STR_CreateBuffer(100,wk->heapID);
	wk->pExpStrBuf = GFL_STR_CreateBuffer(100,wk->heapID);
  wk->WordSet    = WORDSET_Create( wk->heapID );
  wk->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr ,
																		GFL_FONT_LOADTYPE_FILE , FALSE , wk->heapID );

	GFL_UI_KEY_SetRepeatSpeed(1,6);
	_graphicInit(wk);
	_windowCreate(wk);
	_windowRewrite(wk,0);
	_CHANGE_STATE(wk, _itemKindSelectMenu);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   デバッグプロセスMain
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT DebugItemMakeProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_DEBUGITEM_WORK* wk = pwk;
	int retCode = GFL_PROC_RES_FINISH;
  StateFunc* state = wk->state;

	if(state != NULL){
    state(wk);
    retCode = GFL_PROC_RES_CONTINUE;
  }
	return retCode;
	
}

//------------------------------------------------------------------------------
/**
 * @brief   デバッグプロセスEnd
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT DebugItemMakeProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_DEBUGITEM_WORK* wk = pwk;

	GFL_BG_FreeCharacterArea(DEBUG_ITEMDISP_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(wk->bgchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(wk->bgchar));
	GFL_BMPWIN_ClearScreen(wk->win);
	BmpWinFrame_Clear(wk->win, WINDOW_TRANS_OFF);
	GFL_BG_LoadScreenV_Req(DEBUG_ITEMDISP_FRAME);
	GFL_BMPWIN_Delete(wk->win);
	GFL_MSG_Delete(wk->MsgManager);
	GFL_STR_DeleteBuffer(wk->pStrBuf);
	GFL_STR_DeleteBuffer(wk->pExpStrBuf);
  WORDSET_Delete(wk->WordSet);
  GFL_FONT_Delete(wk->fontHandle);
	return GFL_PROC_RES_FINISH;

}



// プロセス定義データ
const GFL_PROC_DATA DebugItemMakeProcData = {
  DebugItemMakeProc_Init,
  DebugItemMakeProc_Main,
  DebugItemMakeProc_End,
};




//============================================================================================
//
//		サブイベントENUM
//
//============================================================================================

enum{
	_INIT_DEBUGITEM,
	_MAIN_DEBUGITEM,
	_END_DEBUGITEM,


};

//============================================================================================
//
//		サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_DebugItemMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_DEBUGITEM_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;
	GFL_PROC_RESULT ret;


  switch (*seq) {
  case _INIT_DEBUGITEM:
		ret = DebugItemMakeProc_Init(NULL,seq,work,work);
		if(GFL_PROC_RES_FINISH == ret){
			(*seq)++;
		}
    break;
  case _MAIN_DEBUGITEM:
		ret = DebugItemMakeProc_Main(NULL,seq,work,work);
		if(GFL_PROC_RES_FINISH == ret){
			(*seq)++;
		}
		break;
  case _END_DEBUGITEM:
		ret = DebugItemMakeProc_End(NULL,seq,work,work);
		if(GFL_PROC_RES_FINISH == ret){
			(*seq)++;
			return GMEVENT_RES_FINISH;
		}
	default:
		break;
	}
	return GMEVENT_RES_CONTINUE;

}


//------------------------------------------------------------------------------
/**
 * @brief   アイテムを作る
 * @param   heapSize    WIFI用のメモリ確保サイズ
 * @retval  none
 */
//------------------------------------------------------------------------------


void EVENT_DebugItemMake(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap,GMEVENT * event, HEAPID heapID)
{
  GMEVENT * child_event;
  EVENT_DEBUGITEM_WORK * wk;

  GMEVENT_Change( event, EVENT_DebugItemMain, sizeof(EVENT_DEBUGITEM_WORK) );
  wk = GMEVENT_GetEventWork(event);
	GFL_STD_MemClear(wk, sizeof(EVENT_DEBUGITEM_WORK));
  wk->ctrl = SaveControl_GetPointer();
  wk->gsys = gsys;
  wk->fieldmap = fieldmap;
  wk->event = event;
	wk->heapID = heapID;
	wk->itemnum = 1;
}

