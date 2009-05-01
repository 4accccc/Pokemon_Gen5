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
#define _BRIGHTNESS_SYNC (2)  // フェードのＳＹＮＣは要調整

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


typedef struct {
  int leftx;
  int lefty;
  int width;
  int height;
} _WINDOWPOS;


static _WINDOWPOS wind4[]={
  { ((0x20-_BUTTON_WIN_WIDTH)/2), (0x18-(2+_BUTTON_WIN_HEIGHT)*4), _BUTTON_WIN_WIDTH,_BUTTON_WIN_HEIGHT},
  { ((0x20-_BUTTON_WIN_WIDTH)/2), (0x18-(2+_BUTTON_WIN_HEIGHT)*3), _BUTTON_WIN_WIDTH,_BUTTON_WIN_HEIGHT},
  { ((0x20-_BUTTON_WIN_WIDTH)/2), (0x18-(2+_BUTTON_WIN_HEIGHT)*2), _BUTTON_WIN_WIDTH,_BUTTON_WIN_HEIGHT},
  { ((0x20-_BUTTON_WIN_WIDTH)/2), (0x18-(2+_BUTTON_WIN_HEIGHT)  ), _BUTTON_WIN_WIDTH,_BUTTON_WIN_HEIGHT},
};


static const GFL_UI_TP_HITTBL bttndata[] = {
  //上下左右
  {	((0x18-(2+_BUTTON_WIN_HEIGHT)*4)*8),(((0x18-(2+_BUTTON_WIN_HEIGHT)*4)*8)+_BUTTON_WIN_HEIGHT*8)-1,
    (((0x20-_BUTTON_WIN_WIDTH)/2)*8),     ((((0x20-_BUTTON_WIN_WIDTH)/2)*8)+_BUTTON_WIN_WIDTH*8)-1  },
  {	((0x18-(2+_BUTTON_WIN_HEIGHT)*3)*8),(((0x18-(2+_BUTTON_WIN_HEIGHT)*3)*8)+_BUTTON_WIN_HEIGHT*8)-1,
    (((0x20-_BUTTON_WIN_WIDTH)/2)*8),     ((((0x20-_BUTTON_WIN_WIDTH)/2)*8)+_BUTTON_WIN_WIDTH*8)-1  },
  {	((0x18-(2+_BUTTON_WIN_HEIGHT)*2)*8),(((0x18-(2+_BUTTON_WIN_HEIGHT)*2)*8)+_BUTTON_WIN_HEIGHT*8)-1,
    (((0x20-_BUTTON_WIN_WIDTH)/2)*8),     ((((0x20-_BUTTON_WIN_WIDTH)/2)*8)+_BUTTON_WIN_WIDTH*8)-1  },
  {	((0x18-(2+_BUTTON_WIN_HEIGHT)*1)*8),(((0x18-(2+_BUTTON_WIN_HEIGHT)*1)*8)+_BUTTON_WIN_HEIGHT*8)-1,
    (((0x20-_BUTTON_WIN_WIDTH)/2)*8),     ((((0x20-_BUTTON_WIN_WIDTH)/2)*8)+_BUTTON_WIN_WIDTH*8)-1  },
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


#define GEAR_MAIN_FRAME   (GFL_BG_FRAME3_S)
#define GEAR_BMPWIN_FRAME   (GFL_BG_FRAME1_S)


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


static void _gearBgCreate(C_GEAR_WORK* pWork)
{
  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_c_gear_c_gear_base_NCLR,
                                      PALTYPE_SUB_BG, 0, 0x20,  pWork->heapID);
    // サブ画面BGキャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_c_gear_c_gear_NCGR,
                                                                  GEAR_MAIN_FRAME, 0, 0, pWork->heapID);

    // サブ画面BGスクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
                                           NARC_c_gear_c_gear_NSCR,
                                           GEAR_MAIN_FRAME, 0,
                                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                           pWork->heapID);


    //パレットアニメシステム作成
//    ConnectBGPalAnm_Init(&pWork->cbp, p_handle, NARC_ircbattle_connect_anm_NCLR, pWork->heapID);
    GFL_ARC_CloseDataHandle( p_handle );
  }
}


static void _createSubBg(C_GEAR_WORK* pWork)
{
  {
    int frame = GEAR_MAIN_FRAME;
    GFL_BG_BGCNT_HEADER AffineBgCntDat = {
      0, 0, 0x400, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x04000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &AffineBgCntDat, GFL_BG_MODE_AFFINE );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, 2 );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GEAR_BMPWIN_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, 0 );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   受け取った数のウインドウを等間隔に作る 幅は3char
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _buttonWindowCreate(int num,int* pMsgBuff,C_GEAR_WORK* pWork)
{
  int i;
  u32 cgx;
  int frame = GEAR_BMPWIN_FRAME;

  pWork->windowNum = num;

  for(i=0;i < num;i++){
    _WINDOWPOS* pos = wind4;

    pWork->buttonWin[i] = GFL_BMPWIN_Create(
      frame,
      pos[i].leftx, pos[i].lefty,
      pos[i].width, pos[i].height,
      _BUTTON_WIN_PAL, GFL_BMP_CHRAREA_GET_F);
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 0 );
    GFL_BMPWIN_MakeScreen(pWork->buttonWin[i]);
    GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
    //    BmpWinFrame_Write( pWork->buttonWin[i], WINDOW_TRANS_ON, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), _BUTTON_WIN_PAL );

    // システムウインドウ枠描画

    GFL_MSG_GetString(  pWork->pMsgData, pMsgBuff[i], pWork->pStrBuf );
    GFL_FONTSYS_SetColor( 0xf, 0xe, 0 );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 4, 4, pWork->pStrBuf, pWork->pFontHandle);
    GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);


  }
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

  switch( event ){
  case GFL_BMN_EVENT_TOUCH:		///< 触れた瞬間
    if(pWork->touch!=NULL){
      if(pWork->touch(bttnid, pWork)){
        PMSND_PlaySystemSE( GEAR_SE_DECIDE_ );
        return;
      }
      else{
      }
    }
    PMSND_PlaySystemSE( GEAR_SE_CANCEL_ );
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
 // int aMsgBuff[]={gear_001,gear_001,gear_001};

//  _buttonWindowCreate(NELEMS(aMsgBuff), aMsgBuff, pWork);

//  pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );
//  pWork->touch = &_modeSelectMenuButtonCallback;

//  _CHANGE_STATE(pWork,_modeSelectMenuWait);
}

static void _workEnd(C_GEAR_WORK* pWork)
{
  GFL_FONTSYS_SetDefaultColor();

  //    _buttonWindowDelete(pWork);
  GFL_BG_FillCharacterRelease( GEAR_BMPWIN_FRAME, 1, 0);
  GFL_BG_FillCharacterRelease( GEAR_MAIN_FRAME, 1, 0);
//  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
//                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));
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
C_GEAR_WORK* CGEAR_Init( void )
{
  C_GEAR_WORK *pWork = NULL;

  //GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_CGEAR, 0x8000 );

  pWork = GFL_HEAP_AllocClearMemory( HEAPID_FIELDMAP, sizeof( C_GEAR_WORK ) );
  pWork->heapID = HEAPID_FIELDMAP;
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

  _workEnd(pWork);

  GFL_HEAP_FreeMemory(pWork);
  
//  GFL_HEAP_DeleteHeap(HEAPID_CGEAR);

}


