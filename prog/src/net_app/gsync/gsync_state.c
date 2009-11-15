//=============================================================================
/**
 * @file    gsync_state.c
 * @brief   ゲームシンク
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date    09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "libdpw/dwci_ghttp.h"
#include "net_app/gsync.h"

#include "system/main.h"  //HEAPID
#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "sound/pm_sndsys.h"
#include "system/wipe.h"
#include "net/network_define.h"
#include "savedata/wifilist.h"
#include "msg/msg_d_ohno.h"

#define _TWLDWC_HTTP (1)

#if _TWLDWC_HTTP

//typedef struct _G_SYNC_WORK G_SYNC_WORK;

typedef void (StateFunc)(G_SYNC_WORK* pState);

#define POSTURL "http://219.118.175.21:10610/cgi-bin/cgeartest/gsync.cgi"
#define GETURL1 "http://219.118.175.21:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data1&mac=1"
#define GETURL2 "http://219.118.175.21:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data2&mac=1"
#define GETURL3 "http://219.118.175.21:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data3&mac=1"
#define GETURL4 "http://219.118.175.21:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data4&mac=1"
#define GETURL5 "http://219.118.175.21:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data5&mac=1"
#define GETURL6 "http://219.118.175.21:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data6&mac=1"


static void _changeState(G_SYNC_WORK* pWork,StateFunc* state);
static void _changeStateDebug(G_SYNC_WORK* pWork,StateFunc* state, int line);

#define _NET_DEBUG (1)

#ifdef _NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pWork ,state)
#endif //_NET_DEBUG



struct _G_SYNC_WORK {
  GFL_FONT      *fontHandle;
  GFL_MSGDATA   *mm;
  GFL_BMPWIN    *win;
  GFL_BMP_DATA  *bmp;
  HEAPID heapID;
  STRBUF      *strbuf;
  STRBUF      *strbufEx;
  WORDSET     *WordSet;               // メッセージ展開用ワークマネージャー

  SAVE_CONTROL_WORK* pSaveData;
  StateFunc* state;      ///< ハンドルのプログラム状態
  vu32 count;
  int req;
  int getdataCount;
  BOOL bEnd;
  DWCGHTTPPost post;
  u8 mac[6];
  u8 buffer[0x102];
};


static void _ghttpGetting(G_SYNC_WORK* pWork);
static void _ghttpPost(G_SYNC_WORK* pWork);
static void _ghttpPosting(G_SYNC_WORK* pWork);
static void _ghttpGet(G_SYNC_WORK* pWork);
static void _ghttpKeyWait(G_SYNC_WORK* pWork);
static void GETCallback(const char* buf, int buflen, DWCGHTTPResult result, void* param);


static void* _getMyUserData(void* pWork)
{
  G_SYNC_WORK *wk = (G_SYNC_WORK*)pWork;
  return WifiList_GetMyUserInfo(SaveData_GetWifiListData(wk->pSaveData));
}

static void* _getFriendData(void* pWork)
{
  G_SYNC_WORK *wk = (G_SYNC_WORK*)pWork;
  //    NET_PRINT("Friend %d\n",WifiList_GetFriendDataNum(SaveData_GetWifiListData(wk->pSaveData)));
  return WifiList_GetDwcDataPtr(SaveData_GetWifiListData(wk->pSaveData),0);
}



static GFLNetInitializeStruct aGFLNetInit = {
  NULL,  // 受信関数テーブル
  0, // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
  NULL,    ///< ネゴシエーション完了時にコール
  NULL,   // ユーザー同士が交換するデータのポインタ取得関数
  NULL,   // ユーザー同士が交換するデータのサイズ取得関数
  NULL,  // ビーコンデータ取得関数
  NULL,  // ビーコンデータサイズ取得関数
  NULL,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,            // 普通のエラーが起こった場合 通信終了
  NULL,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL,  // 通信切断時に呼ばれる関数
  NULL,  // オート接続で親になった場合
  NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL,  ///< wifiフレンドリスト削除コールバック
  _getFriendData,   ///< DWC形式の友達リスト
  _getMyUserData,  ///< DWCのユーザデータ（自分のデータ）
  GFL_NET_DWC_HEAPSIZE + 0x80000,   ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
  0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK,  //IRC用にcreateされるHEAPID
  GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
  1,     // 最大接続人数
  48,    // 最大送信バイト数
  32,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIFI,  //通信種別
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_WIFICLUB,  //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
    0,//MP親最大サイズ 512まで
    0,//dummy
};


static void _msgPrint(G_SYNC_WORK* pWork,int msg)
{
  GFL_BMP_Clear( pWork->bmp, 0xff );
  GFL_MSG_GetString(pWork->mm, msg, pWork->strbuf);
  PRINTSYS_Print( pWork->bmp, 0, 0, pWork->strbuf, pWork->fontHandle);
  GFL_BMPWIN_TransVramCharacter( pWork->win );
}

static void _msgPrintNo(G_SYNC_WORK* pWork,int msg,int no)
{
  GFL_BMP_Clear( pWork->bmp, 0xff );
  GFL_MSG_GetString(pWork->mm, msg, pWork->strbufEx);

  WORDSET_RegisterNumber(pWork->WordSet, 0, no,
                         5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( pWork->WordSet, pWork->strbuf,pWork->strbufEx );


  PRINTSYS_Print( pWork->bmp, 0, 0, pWork->strbuf, pWork->fontHandle);
  GFL_BMPWIN_TransVramCharacter( pWork->win );
}

//=============================================================================
/*!
 *  @brief  データ受信完了時のコールバック
 *
 *  @param  buf   [in] データバッファ
 *  @param  buflen  [in] bufのサイズ
 *  @param  param   [in] コールバック用パラメータ
 *
 *  @retval TRUE  buf開放
 *  @retval FALSE   buf開放せず
 */
//=============================================================================
static void GETCallback(const char* buf, int buflen, DWCGHTTPResult result, void* param)
{
  G_SYNC_WORK* pWork = param;

  pWork->bEnd = TRUE;
  OS_Printf("Recv Result : %d length %d\n", result,buflen);
  OS_Printf("Recv msg : %s \n", buf);
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(G_SYNC_WORK* pWork,StateFunc state)
{
  pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef _NET_DEBUG
static void _changeStateDebug(G_SYNC_WORK* pWork,StateFunc state, int line)
{
#ifdef DEBUG_ONLY_FOR_ohno
  OS_TPrintf("ghttp: %d\n",line);
#endif
  _changeState(pWork, state);
}
#endif


//=============================================================================
/*!
 *  @brief  データ送信完了時のコールバック
 *
 *  @param  buf   [in] データバッファ
 *  @param  buflen  [in] bufのサイズ
 *
 *  @retval TRUE  buf開放
 *  @retval FALSE   buf開放せず
*/
//=============================================================================
static void POSTCallback(const char* buf, int buflen, DWCGHTTPResult result, void* param)
{
  G_SYNC_WORK* pWork = param;

  OS_TPrintf("GHTTPのデータ送信完了\n");
  pWork->bEnd = TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpConnecting(G_SYNC_WORK* pWork)
{
  int len;

  if(GFL_NET_StateGetWifiStatus()!=GFL_NET_STATE_MATCHING){
    return;
  }


  // ghttpの初期化
  if(DWC_InitGHTTP(NULL) != TRUE){
    OS_Printf("error at SCInitialize()\n");
  }
  _msgPrint(pWork, DEBUG_OHNO_MSG0007);

  _CHANGE_STATE(_ghttpKeyWait);
}


static void _ghttpPost(G_SYNC_WORK* pWork)
{

  // send data
  //-------------------------------------------
  GFL_STD_MemClear(pWork->mac, 6);

  OS_GetMacAddress(pWork->mac);


  _msgPrint(pWork, DEBUG_OHNO_MSG0008);


  // post data
  //-------------------------------------------
  {
    DWC_GHTTPNewPost(&pWork->post);
//    DWC_GHTTPPostAddString(&pWork->post, "name", "sogabe");
//    DWC_GHTTPPostAddString(&pWork->post, "mac", "123456");
//    DWC_GHTTPPostAddString(&pWork->post, "save", "datatest");

    DWC_GHTTPPostAddFileFromMemory(&pWork->post, "name1", (const char*)"sogabe",6,
                                   "name.txt","application/octet-stream");

    DWC_GHTTPPostAddFileFromMemory(&pWork->post, "file1", (const char*)SaveControl_GetPointer(),0x80000,
                                   "test.bin","application/octet-stream");

//    DWC_GHTTPPostAddFileFromMemory(&pWork->post, "mac", (const char*)pWork->mac,6,
  //                                 "mac.bin","application/octet-stream");

  }

  pWork->count = OS_GetVBlankCount();
  OS_TPrintf("start %d\n",OS_GetVBlankCount());

  OS_TPrintf("DWC_GHTTP_HOST_LOOKUP       %d\n",DWC_GHTTP_HOST_LOOKUP       );
  OS_TPrintf("DWC_GHTTP_CONNECTING        %d\n",DWC_GHTTP_CONNECTING        );
  OS_TPrintf("DWC_GHTTP_SECURING_SESSION  %d\n",DWC_GHTTP_SECURING_SESSION  );
  OS_TPrintf("DWC_GHTTP_SENDING_REQUEST   %d\n",DWC_GHTTP_SENDING_REQUEST   );
  OS_TPrintf("DWC_GHTTP_POSTING           %d\n",DWC_GHTTP_POSTING           );
  OS_TPrintf("DWC_GHTTP_WAITING           %d\n",DWC_GHTTP_WAITING           );
  OS_TPrintf("DWC_GHTTP_RECEIVING_STATUS  %d\n",DWC_GHTTP_RECEIVING_STATUS  );
  OS_TPrintf("DWC_GHTTP_RECEIVING_HEADERS %d\n",DWC_GHTTP_RECEIVING_HEADERS );
  OS_TPrintf("DWC_GHTTP_RECEIVING_FILE    %d\n",DWC_GHTTP_RECEIVING_FILE    );
  OS_TPrintf("DWC_GHTTP_SOCKET_INIT       %d\n",DWC_GHTTP_SOCKET_INIT       );
  OS_TPrintf("DWC_GHTTP_LOOKUP_PENDING    %d\n",DWC_GHTTP_LOOKUP_PENDING    );

  pWork->bEnd = FALSE;


  pWork->req = DWC_PostGHTTPData(POSTURL, &pWork->post, POSTCallback, pWork);

  if( 0 <= pWork->req){
    _msgPrint(pWork, DEBUG_OHNO_MSG0008);

    _CHANGE_STATE(_ghttpPosting);
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   キー入力で動きを変える
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpKeyWait(G_SYNC_WORK* pWork)
{
  switch(GFL_UI_KEY_GetTrg())
  {
  case PAD_BUTTON_X:
    _CHANGE_STATE(_ghttpPost);
    break;
  case PAD_BUTTON_Y:

    pWork->count = OS_GetVBlankCount();
    OS_TPrintf("start %d  count %d\n",OS_GetVBlankCount(), pWork->getdataCount);
    pWork->getdataCount = 0;//読み込み時は分割しないとヒープが足りない

    _CHANGE_STATE(_ghttpGet);
    break;
  case PAD_BUTTON_B:
    _CHANGE_STATE(NULL);
    DWC_ShutdownGHTTP();
    break;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpPosting(G_SYNC_WORK* pWork)
{

  DWC_ProcessGHTTP();
  //OS_TPrintf("state %d\n",DWC_GetGHTTPState(pWork->req));

  if( pWork->bEnd ){
    vu32 ans = OS_GetVBlankCount() - pWork->count;
    int ansint = ans / 60;

    OS_TPrintf("end %d\n",OS_GetVBlankCount());

    pWork->bEnd = FALSE;
    _msgPrintNo(pWork, DEBUG_OHNO_MSG0009, ansint);
    _CHANGE_STATE(_ghttpKeyWait);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpGet(G_SYNC_WORK* pWork)
{
  char* bufadd[]={GETURL1,GETURL2,GETURL3,GETURL4,GETURL4,GETURL6};


  pWork->bEnd = FALSE;

  if( 0 <= DWC_GetGHTTPData(bufadd[pWork->getdataCount], GETCallback, (void*)pWork)){


    _msgPrint(pWork, DEBUG_OHNO_MSG0010);
    _CHANGE_STATE(_ghttpGetting);
  }

}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpGetting(G_SYNC_WORK* pWork)
{

  DWC_ProcessGHTTP();
  if( pWork->bEnd ){
    pWork->getdataCount++;
    if(pWork->getdataCount < 6){   //710703バイトは一回131072なので 6回
      _CHANGE_STATE(_ghttpGet);
    }
    else{
      vu32 ans = OS_GetVBlankCount() - pWork->count;
      int ansint = ans / 60;
      OS_TPrintf("end %d\n",OS_GetVBlankCount());
      _msgPrintNo(pWork, DEBUG_OHNO_MSG0011, ansint);
      _CHANGE_STATE(_ghttpKeyWait);
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   通信初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpInit(G_SYNC_WORK* pWork)
{

  GFL_NET_Init(&aGFLNetInit, NULL, pWork);
  GFL_NET_StateWifiEnterLogin();

  _msgPrint(pWork, DEBUG_OHNO_MSG0006);

  _CHANGE_STATE(_ghttpConnecting);


}

static const GFL_DISP_VRAM vramBank = {
    GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_128_B,        // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_0_F,     // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,       // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_0123_E,     // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_32K, // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
};
#endif


static GFL_PROC_RESULT GSYNCProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#if _TWLDWC_HTTP
  //GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PROC, 0x70000 );//テスト

  {
    G_SYNC_WORK* pWork = GFL_PROC_AllocWork( proc, sizeof(G_SYNC_WORK), HEAPID_PROC );

    pWork->pSaveData = SaveControl_GetPointer();  //デバッグ
    pWork->heapID = HEAPID_PROC;

    GFL_DISP_SetBank( &vramBank );

    //バックグラウンドの色を入れておく
    GFL_STD_MemFill16((void*)HW_BG_PLTT, 0x5ad6, 2);

    // 各種効果レジスタ初期化
    G2_BlendNone();

    // BGsystem初期化
    GFL_BG_Init( pWork->heapID );
    GFL_BMPWIN_Init( pWork->heapID );
    GFL_FONTSYS_Init();

      //ＢＧモード設定
  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &sysHeader );
  }

  // 個別フレーム設定
  {
    static const GFL_BG_BGCNT_HEADER bgcntText = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };

    GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

    GX_SetMasterBrightness(0);

    GFL_BG_SetBackGroundColor(GFL_BG_FRAME0_M ,0x7f);
    GFL_BG_SetBackGroundColor(GFL_BG_FRAME0_S ,0x7f);

//    GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
    GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );

//    void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
    GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

    GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
    GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
  }

  // 上下画面設定
    GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

    pWork->win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 4, 12, 24, 16, 0, GFL_BMP_CHRAREA_GET_F );
    pWork->bmp = GFL_BMPWIN_GetBmp( pWork->win);
    GFL_BMP_Clear( pWork->bmp, 0xff );
    GFL_BMPWIN_MakeScreen( pWork->win );
    GFL_BMPWIN_TransVramCharacter( pWork->win );

    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

    pWork->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
                                      GFL_FONT_LOADTYPE_FILE, FALSE, pWork->heapID );

    pWork->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_ohno_dat, pWork->heapID );
    pWork->strbuf = GFL_STR_CreateBuffer(64, pWork->heapID);
    pWork->strbufEx = GFL_STR_CreateBuffer(64, pWork->heapID);
    pWork->WordSet    = WORDSET_Create( pWork->heapID );

  //フォントパレット転送
    GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                  0, 0x20, HEAPID_PROC);


    _CHANGE_STATE(_ghttpInit);
  }
#endif
  return GFL_PROC_RES_FINISH;
}
static GFL_PROC_RESULT GSYNCProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#if _TWLDWC_HTTP
  G_SYNC_WORK* pWork = mywk;
  StateFunc* state = pWork->state;

  if( state ){
    state( pWork );
    return GFL_PROC_RES_CONTINUE;
  }
#endif
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT GSYNCProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
#if _TWLDWC_HTTP
  G_SYNC_WORK* pWork = mywk;

  GFL_STR_DeleteBuffer(pWork->strbuf);
  GFL_STR_DeleteBuffer(pWork->strbufEx);
  GFL_MSG_Delete(pWork->mm);
  GFL_FONT_Delete(pWork->fontHandle);

  WORDSET_Delete( pWork->WordSet );

  GFL_PROC_FreeWork(proc);
  GFL_HEAP_DeleteHeap(HEAPID_PROC);  //テスト
#endif
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA G_SYNC_ProcData = {
  GSYNCProc_Init,
  GSYNCProc_Main,
  GSYNCProc_End,
};

