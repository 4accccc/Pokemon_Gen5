//=============================================================================
/**
 * @file	ircbattlematch.c
 * @bfief	赤外線マッチング
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	09/02/24
 */
//=============================================================================

#include <gflib.h>
#include "arc_def.h"
#include "net/network_define.h"

#include "ircbattlematch.h"
#include "system/main.h"

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

#include "msg/msg_ircbattle.h"
#include "ircbattle.naix"
#include "net_app/connect_anm.h"
#include "../../field/event_ircbattle.h"


#define _NET_DEBUG (1)  //デバッグ時は１
#define _WORK_HEAPSIZE (0x1000)  // 調整が必要
#define _BRIGHTNESS_SYNC (2)  // フェードのＳＹＮＣは要調整

// サウンドが出来るまでの仮想
#define _SE_DESIDE (0)
#define _SE_CANCEL (0)
static void Snd_SePlay(int a){}


//--------------------------------------------
// 画面構成定義
//--------------------------------------------
#define _WINDOW_MAXNUM (3)   //ウインドウのパターン数

#define _MESSAGE_BUF_NUM	( 100*2 )

#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (5)    // ウインドウ高さ
#define _BUTTON_WIN_PAL   (14)  // ウインドウ
#define _BUTTON_MSG_PAL   (13)  // メッセージフォント
#define _START_PAL   (12)  // スタートウインドウ

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
  { ((0x20-_BUTTON_WIN_WIDTH)/2), (0x18-(2+_BUTTON_WIN_HEIGHT)), _BUTTON_WIN_WIDTH,_BUTTON_WIN_HEIGHT},
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

typedef void (StateFunc)(IRC_BATTLE_MATCH* pState);

//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(IRC_BATTLE_MATCH* pWork,StateFunc* state);
static void _changeStateDebug(IRC_BATTLE_MATCH* pWork,StateFunc* state, int line);
static void* _netBeaconGetFunc(void* pWork);
static int _netBeaconGetSizeFunc(void* pWork);
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
static void _endCallBack(void* pWork);
static void _buttonWindowCreate(int num,int* pMsgBuff,IRC_BATTLE_MATCH* pWork);
static void _ircMatchStart(IRC_BATTLE_MATCH* pWork);
static void _fadeInWait(IRC_BATTLE_MATCH* pWork);
static void _ircInitWait(IRC_BATTLE_MATCH* pWork);
static void _ircMatchWait(IRC_BATTLE_MATCH* pWork);
static void _ircStartTiming(IRC_BATTLE_MATCH* pWork);
static void _modeSelectEntryNumInit(IRC_BATTLE_MATCH* pWork);
static void _modeSelectEntryNumWait(IRC_BATTLE_MATCH* pWork);
static void _modeReportInit(IRC_BATTLE_MATCH* pWork);
static void _modeReportWait(IRC_BATTLE_MATCH* pWork);
static BOOL _modeSelectEntryNumButtonCallback(int bttnid,IRC_BATTLE_MATCH* pWork);
static void _connectCallBack(void* pWork, int netID);
static void _RecvFirstData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _RecvResultData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _ircPreConnect(IRC_BATTLE_MATCH* pWork);


//--------------------------------------------
// 内部ワーク
//--------------------------------------------
///通信コマンドテーブル
static const NetRecvFuncTable _PacketTbl[] = {
  {_RecvFirstData,         NULL},    ///NET_CMD_FIRST
  {_RecvResultData,          NULL},  ///NET_CMD_RESULT
};

#define _MAXNUM   (2)         // 最大接続人数
#define _MAXSIZE  (100)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数

static GFLNetInitializeStruct aGFLNetInit = {
  _PacketTbl,  // 受信関数テーブル
  NELEMS(_PacketTbl), // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
  _connectCallBack,    ///< ネゴシエーション完了時にコール
  NULL,   // ユーザー同士が交換するデータのポインタ取得関数
  NULL,   // ユーザー同士が交換するデータのサイズ取得関数
  IrcBattleBeaconGetFunc,  // ビーコンデータ取得関数
  IrcBattleBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
  IrcBattleBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,            // 普通のエラーが起こった場合 通信終了
  FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL, //_endCallBack,  // 通信切断時に呼ばれる関数
  NULL,  // オート接続で親になった場合
  NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL,  ///< wifiフレンドリスト削除コールバック
  NULL,   ///< DWC形式の友達リスト
  NULL,  ///< DWCのユーザデータ（自分のデータ）
  0,   ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
  0x532,//0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_IRC,   //※check　赤外線通信用にcreateされるHEAPID
  GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
  _MAXNUM,     // 最大接続人数
  _MAXSIZE,  //最大送信バイト数
  _BCON_GET_NUM,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_IRC,  //wifi通信を行うかどうか
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_COMPATI_CHECK,  //GameServiceID
  0xfffe,	// 赤外線タイムアウト時間
  0,//MP通信親機送信バイト数
  0,//dummy
};




struct _IRC_BATTLE_MATCH {
  EVENT_IRCBATTLE_WORK* pBattleWork;
	StateFunc* state;      ///< ハンドルのプログラム状態
  int selectType;   // 接続タイプ
  HEAPID heapID;
  GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
  GFL_MSGDATA *pMsgData;  //
  WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
  GFL_FONT* pFontHandle;
  STRBUF* pStrBuf;
  BMPWINFRAME_AREAMANAGER_POS aPos;
  int windowNum;
  BOOL IsIrc;
  BMPMENU_WORK* pYesNoWork;
  //    GAMESYS_WORK *gameSys_;
  //    FIELD_MAIN_WORK *fieldWork_;
  u32 connect_bit;
  BOOL connect_ok;
  BOOL receive_ok;
  u32 receive_result_param;
  u32 receive_first_param;
  GFL_ARCUTIL_TRANSINFO bgchar;
  GFL_ARCUTIL_TRANSINFO bgchar2;
  GFL_ARCUTIL_TRANSINFO subchar;
  GFL_ARCUTIL_TRANSINFO subchar3;
  CONNECT_BG_PALANM cbp;		// Wifi接続画面のBGパレットアニメ制御構造体
  BOOL bParent;
};



enum{
  _START_TIMING=12,
};




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

static void _changeState(IRC_BATTLE_MATCH* pWork,StateFunc state)
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
static void _changeStateDebug(IRC_BATTLE_MATCH* pWork,StateFunc state, int line)
{
  NET_PRINT("ircmatch: %d\n",line);
  _changeState(pWork, state);
}
#endif

static void _endCallBack(void* pWork)
{
  IRC_BATTLE_MATCH *commsys = pWork;

  OS_TPrintf("endCallBack終了\n");
  commsys->connect_ok = FALSE;
  commsys->connect_bit = 0;
}

//--------------------------------------------------------------
/**
 * @brief   接続完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
static void _connectCallBack(void* pWk, int netID)
{
  IRC_BATTLE_MATCH *pWork = pWk;
  u32 temp;

  OS_TPrintf("ネゴシエーション完了 netID = %d\n", netID);
  pWork->connect_bit |= 1 << netID;
  temp = pWork->connect_bit;
  if(MATH_CountPopulation(temp) >= 2){
    OS_TPrintf("全員のネゴシエーション完了 人数bit=%x\n", pWork->connect_bit);
    pWork->connect_ok = TRUE;
  }
}



//--------------------------------------------------------------
/**
 * @brief   赤外線終了ワイヤレス開始コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------

static void _ircConnectEndCallback(void* pWk)
{
  IRC_BATTLE_MATCH *pWork = pWk;
  int no;

  // 画像交換
  if(1){
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCBATTLE, pWork->heapID );

    if(pWork->selectType==EVENTIRCBTL_ENTRYMODE_FRIEND || pWork->selectType==EVENTIRCBTL_ENTRYMODE_TRADE){
      no = NARC_ircbattle_c_start_NSCR;
    }
    else{
      no = NARC_ircbattle_b_start_NSCR;
    }


    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs( p_handle, no,
                                            GFL_BG_FRAME3_S, 0,
                                            _START_PAL*0x1000+GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar3), 0, 0,
                                            pWork->heapID);
    GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_ON );
    GFL_ARC_CloseDataHandle( p_handle );
  }



}

//--------------------------------------------------------------
/**
 * @brief   接続完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------

static void _wirelessConnectCallback(void* pWk)
{
  IRC_BATTLE_MATCH *pWork = pWk;
  int no;

#if 1 //

  _CHANGE_STATE(pWork,NULL);

#else
  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, _BRIGHTNESS_SYNC);
  _CHANGE_STATE(pWork,_ircStartTiming);
#endif
}


static void _wirelessPreConnectCallback(void* pWk,BOOL bParent)
{
  IRC_BATTLE_MATCH *pWork = pWk;

  pWork->bParent = bParent;
  //   GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, _BRIGHTNESS_SYNC);
  _CHANGE_STATE(pWork,_ircPreConnect);
}



//------------------------------------------------------------------------------
/**
 * @brief   BG領域設定
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createBg(IRC_BATTLE_MATCH* pWork)
{
  {
    static const GFL_DISP_VRAM vramBank = {
      GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
      GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
      GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
      GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
      GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
      GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット
      GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
      GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
      GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
      GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット
      GX_OBJVRAMMODE_CHAR_1D_32K,	// メインOBJマッピングモード
      GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
    };
    GFL_DISP_SetBank( &vramBank );
  }
  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &sysHeader );
  }
  GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);

  {
    int frame = GFL_BG_FRAME1_M;
    GFL_BG_BGCNT_HEADER bgcntText = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &bgcntText, GFL_BG_MODE_TEXT );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME0_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    //		GFL_BG_LoadScreenReq( frame );
    //        GFL_BG_ClearFrame(frame);
  }
  {
    int frame = GFL_BG_FRAME1_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    //		GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
    //        GFL_BG_ClearFrame(frame);
  }
  {
    int frame = GFL_BG_FRAME2_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME3_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

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

static void _msgWindowCreate(int* pMsgBuff,IRC_BATTLE_MATCH* pWork)
{
  int i=0;
  u32 cgx;
  int frame = GFL_BG_FRAME1_S;
  _WINDOWPOS* pos = wind4;
  //    GFL_FONTSYS_SetColor( 1, 1, 1 );

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);


  pWork->buttonWin[i] = GFL_BMPWIN_Create(
    frame,
    ((0x20-_BUTTON_WIN_WIDTH)/2), (0x18-(2+_BUTTON_WIN_HEIGHT)), _BUTTON_WIN_WIDTH,_BUTTON_WIN_HEIGHT,
    _BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), WINCLR_COL(FBMP_COL_WHITE) );
  GFL_BMPWIN_MakeScreen(pWork->buttonWin[i]);
  GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);
  BmpWinFrame_Write( pWork->buttonWin[i], WINDOW_TRANS_ON, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2), _BUTTON_WIN_PAL );

  GFL_MSG_GetString(  pWork->pMsgData, pMsgBuff[i], pWork->pStrBuf );
  //    GFL_FONTSYS_SetColor( 1, 1, 1 );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->buttonWin[i]), 4, 4, pWork->pStrBuf, pWork->pFontHandle);
  GFL_BMPWIN_TransVramCharacter(pWork->buttonWin[i]);

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

static void _buttonWindowDelete(IRC_BATTLE_MATCH* pWork)
{
  int i;

	if(pWork->buttonWin[0]){
		GFL_BMPWIN_Delete(pWork->buttonWin[0]);
	}
	pWork->buttonWin[0]=NULL;
}


static void _graphicInit(IRC_BATTLE_MATCH* pWork)
{
  GFL_BG_Init( pWork->heapID );
  GFL_BMPWIN_Init( pWork->heapID );
  GFL_FONTSYS_Init();
}


static void _graphicEnd(IRC_BATTLE_MATCH* pWork)
{
  GFL_BG_Exit();
  GFL_BMPWIN_Exit();

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeInit(IRC_BATTLE_MATCH* pWork)
{
  GFL_FONTSYS_SetDefaultColor();
  _graphicInit(pWork);
  _createBg(pWork);

  pWork->pStrBuf = GFL_STR_CreateBuffer( _MESSAGE_BUF_NUM, pWork->heapID );
  pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
  pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_ircbattle_dat, pWork->heapID );

  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCBATTLE, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_ircbattle_connect_NCLR,
                                      PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_ircbattle_connect_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  pWork->heapID);
    // メイン画面BG2キャラ転送
    pWork->bgchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle,
                                                                 NARC_ircbattle_connect_NCGR,
                                                                 GFL_BG_FRAME1_M, 0, 0, pWork->heapID);
    // メイン画面BG2スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_ircbattle_connect_01_NSCR,
                                              GFL_BG_FRAME1_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), 0, 0,
                                              pWork->heapID);
    // サブ画面BG0キャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_ircbattle_connect_sub_NCGR,
                                                                  GFL_BG_FRAME0_S, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_ircbattle_connect_sub_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);


    //パレットアニメシステム作成
    ConnectBGPalAnm_Init(&pWork->cbp, p_handle, NARC_ircbattle_connect_anm_NCLR, pWork->heapID);



    // サブ画面BG3キャラ転送
    pWork->subchar3 = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_ircbattle_start_NCGR,
                                                                  GFL_BG_FRAME3_S, 0, 0, pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramPalette(p_handle, NARC_ircbattle_start2_NCLR, PALTYPE_SUB_BG,
                                     0x20*_START_PAL, 0x20, pWork->heapID);


    GFL_ARC_CloseDataHandle( p_handle );
  }



  pWork->bgchar2 = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME1_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);

  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, _BRIGHTNESS_SYNC);

  {
		if(pWork->selectType==EVENTIRCBTL_ENTRYMODE_FRIEND || pWork->selectType==EVENTIRCBTL_ENTRYMODE_TRADE)
		{
			int aMsgBuff[]={IRCBTL_STR_17};
			_msgWindowCreate(aMsgBuff, pWork);
		}
		else
		{
			int aMsgBuff[]={IRCBTL_STR_09};
			_msgWindowCreate(aMsgBuff, pWork);
		}
  }

  GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );
  GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_ON );
  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
  GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_ON );


  _CHANGE_STATE(pWork,_fadeInWait);
}

//------------------------------------------------------------------------------
/**
 * @brief   フェードイン待ち
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _fadeInWait(IRC_BATTLE_MATCH* pWork)
{
  // ワイプ終了待ち
  if( GFL_FADE_CheckFade() ){
    _CHANGE_STATE(pWork,_ircMatchStart);
  }

}

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _ircMatchStart(IRC_BATTLE_MATCH* pWork)
{

  {
    GFLNetInitializeStruct net_ini_data;
    net_ini_data = aGFLNetInit;
    net_ini_data.bNetType = GFL_NET_TYPE_IRC_WIRELESS;
    switch(pWork->selectType){
    case EVENTIRCBTL_ENTRYMODE_SINGLE:
    case EVENTIRCBTL_ENTRYMODE_DOUBLE:
    case EVENTIRCBTL_ENTRYMODE_TRI:
			net_ini_data.gsid = WB_NET_IRCBATTLE;
      break;
    case EVENTIRCBTL_ENTRYMODE_MULTH:
			net_ini_data.gsid = WB_NET_IRCBATTLE;
      net_ini_data.maxConnectNum = 4;
      break;
    case EVENTIRCBTL_ENTRYMODE_FRIEND:
			net_ini_data.gsid = WB_NET_IRCFRIEND;
			break;
    case EVENTIRCBTL_ENTRYMODE_TRADE:
			net_ini_data.gsid = WB_NET_IRCTRADE;
			break;
    default:
      GF_ASSERT(0);
      break;
    }
    GFL_NET_Init(&net_ini_data, NULL, pWork);	//通信初期化
  }

  _CHANGE_STATE(pWork,_ircInitWait);
}

static void _workEnd(IRC_BATTLE_MATCH* pWork)
{
  GFL_FONTSYS_SetDefaultColor();

  _buttonWindowDelete(pWork);
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_S, 1, 0);
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar2),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar2));
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
  GFL_MSG_Delete( pWork->pMsgData );
  GFL_FONT_Delete(pWork->pFontHandle);
  GFL_STR_DeleteBuffer(pWork->pStrBuf);
  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );

  _graphicEnd(pWork);

}

//------------------------------------------------------------------------------
/**
 * @brief   ＩＲＣ接続待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _ircInitWait(IRC_BATTLE_MATCH* pWork)
{
  if(GFL_NET_IsInit() == TRUE){	//初期化終了待ち
    GFL_NET_ChangeoverConnect_IRCWIRELESS(_wirelessConnectCallback,_wirelessPreConnectCallback,_ircConnectEndCallback); // 専用の自動接続
    _CHANGE_STATE(pWork,_ircMatchWait);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ＩＲＣ接続待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _ircExitWait(IRC_BATTLE_MATCH* pWork)
{
  int ret = BmpMenu_YesNoSelectMain(pWork->pYesNoWork);
  if(ret == BMPMENU_NULL)
  {  // まだ選択中
    return;
  }
  else
  {
    if(ret == 0)
    { // はいを選択した場合
			EVENT_IrcBattleSetType(pWork->pBattleWork,EVENTIRCBTL_ENTRYMODE_EXIT);
			_buttonWindowDelete(pWork);
			GFL_NET_Exit(NULL);
			_CHANGE_STATE(pWork,NULL);
    }
    else
    {  // いいえを選択した場合
			int aMsgBuff[]={IRCBTL_STR_09};
			_buttonWindowDelete(pWork);
			_msgWindowCreate(aMsgBuff, pWork);
			_CHANGE_STATE(pWork,_ircMatchWait);
		}
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   キーを待って終了
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _ircEndKeyWait(IRC_BATTLE_MATCH* pWork)
{
	if(GFL_UI_KEY_GetTrg() != 0){
		
		
		EVENT_IrcBattleSetType(pWork->pBattleWork,EVENTIRCBTL_ENTRYMODE_RETRY);
		GFL_NET_Exit(NULL);
		_CHANGE_STATE(pWork,NULL);
	}
}


//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
#define	FLD_YESNO_WIN_PX	( 25 )
#define	FLD_YESNO_WIN_PY	( 13 )

static const BMPWIN_YESNO_DAT _yesNoBmpDatSys2 = {
	GFL_BG_FRAME2_S, FLD_YESNO_WIN_PX, FLD_YESNO_WIN_PY+6,
	12, 512
	};

static void _ircMatchWait(IRC_BATTLE_MATCH* pWork)
{
	if(GFL_NET_IsInit()){
		if(GFL_NET_NEG_TYPE_TYPE_ERROR==GFL_NET_HANDLE_GetNegotiationType(GFL_NET_HANDLE_GetCurrentHandle())){  ///< モードが異なる接続エラー
			int aMsgBuff[]={IRCBTL_STR_25};
			_buttonWindowDelete(pWork);
			_msgWindowCreate(aMsgBuff, pWork);
			_CHANGE_STATE(pWork,_ircEndKeyWait);
			return;
		}
	}
	if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL){
		int aMsgBuff[]={IRCBTL_STR_16};
		_buttonWindowDelete(pWork);

		GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
																	0x20*12, 0x20, pWork->heapID);

		_msgWindowCreate(aMsgBuff, pWork);
    BmpWinFrame_GraphicSet(
      GFL_BG_FRAME2_S, 512-24, 11, 0, pWork->heapID );

		GFL_FONTSYS_SetColor( 1, 2, 15 );

		pWork->pYesNoWork =
			BmpMenu_YesNoSelectInit(&_yesNoBmpDatSys2, 512-24, 11, 0, pWork->heapID );
		GFL_FONTSYS_SetDefaultColor();
		_CHANGE_STATE(pWork,_ircExitWait);
	}


}

//------------------------------------------------------------------------------
/**
 * @brief
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _ircPreConnect(IRC_BATTLE_MATCH* pWork)
{
  _buttonWindowDelete(pWork);

  if(pWork->bParent){
    int aMsgBuff[]={IRCBTL_STR_12};
    _msgWindowCreate(aMsgBuff, pWork);
  }
  else{
    int aMsgBuff[]={IRCBTL_STR_13};
    _msgWindowCreate(aMsgBuff, pWork);
  }
  _CHANGE_STATE(pWork,_ircMatchWait);
}

//------------------------------------------------------------------------------
/**
 * @brief   人数選択画面待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _ircStartTiming(IRC_BATTLE_MATCH* pWork)
{

  //  if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_START_TIMING) == TRUE){
  //    OS_TPrintf("タイミング取り成功\n");
  //  OS_TPrintf("接続人数 = %d\n", GFL_NET_GetConnectNum());
  //    }
  // ワイプ終了待ち

  
#if 1
  _CHANGE_STATE(pWork,NULL);

#else
  if( GFL_FADE_CheckFade() ){
    _CHANGE_STATE(pWork,NULL);
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   移動コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------
static void _RecvFirstData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  IRC_BATTLE_MATCH *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return;	//自分のデータは無視
  }

  //GFL_STD_MemCopy(pData, pWork->receive_first_param, size);
  pWork->receive_ok = TRUE;
  OS_TPrintf("最初のデータ受信コールバック netID = %d\n", netID);
}

//--------------------------------------------------------------
/**
 * @brief   キー情報コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------
static void _RecvResultData(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  IRC_BATTLE_MATCH *pWork = pWk;

  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return;	//自分のデータは無視
  }

  //GFL_STD_MemCopy(pData, pWork->receive_result_param, size);
  pWork->receive_ok = TRUE;
  OS_TPrintf("最後のデータ受信コールバック netID = %d\n", netID);
}




//------------------------------------------------------------------------------
/**
 * @brief   PROCスタート
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleMatchProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, 0x20000 );

  {
    IRC_BATTLE_MATCH *pWork = GFL_PROC_AllocWork( proc, sizeof( IRC_BATTLE_MATCH ), HEAPID_IRCBATTLE );
    GFL_STD_MemClear(pWork, sizeof(IRC_BATTLE_MATCH));
		pWork->pBattleWork = pwk;
    pWork->heapID = HEAPID_IRCBATTLE;
    pWork->selectType =  EVENT_IrcBattleGetType((EVENT_IRCBATTLE_WORK*) pwk);
    _CHANGE_STATE( pWork, _modeInit);
  }
  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleMatchProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  IRC_BATTLE_MATCH* pWork = mywk;
  GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;

  StateFunc* state = pWork->state;
  if(state != NULL){
    state(pWork);
    retCode = GFL_PROC_RES_CONTINUE;
  }
  ConnectBGPalAnm_Main(&pWork->cbp);

  return retCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleMatchProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  IRC_BATTLE_MATCH* pWork = mywk;

  _workEnd(pWork);

  ConnectBGPalAnm_End(&pWork->cbp);
  GFL_PROC_FreeWork(proc);
  GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);



  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA IrcBattleMatchProcData = {
  IrcBattleMatchProcInit,
  IrcBattleMatchProcMain,
  IrcBattleMatchProcEnd,
};


