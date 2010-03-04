//============================================================================================
/**
 * @file	debug_ohno.c
 * @brief	デバッグ用関数
 * @author	ohno
 * @date	2006.11.29
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"
#include "debug_ohno.h"
#include "debug_field.h"
#include "system/main.h"
#include "net/network_define.h"
#include "net/dwc_raputil.h"
#include "net/delivery_beacon.h"
#include "net/delivery_irc.h"
#include "wmi.naix"

#include "savedata/mystery_data.h"
#include "debug/debug_mystery_card.h"

#include "savedata/bsubway_savedata.h"
#include "savedata/battle_examination.h"
#include "waza_tool/wazano_def.h"
#include "battle/bsubway_battle_data.h"

#define _MAXNUM   (4)         // 最大接続人数
#define _MAXSIZE  (80)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数


FS_EXTERN_OVERLAY(dev_wifi);

typedef BOOL (*NetTestFunc)(void* pCtl);


struct _DEBUG_OHNO_CONTROL{
  u32 debug_heap_id;
  GFL_PROCSYS * psys;
  NetTestFunc funcNet;
  int bMoveRecv;
  BOOL bParent;
  int beaconIncCount;  //ビーコンの中身を替えるカウンタ
  DELIVERY_BEACON_WORK* pDBWork;
  DELIVERY_BEACON_INIT aInit;
  int counter;
  DELIVERY_IRC_WORK* pIRCWork;
  DELIVERY_IRC_INIT aIRCInit;
};


//------------------------------------------------------------------
//  受信テーブル
//------------------------------------------------------------------

static void _RecvMoveData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _RecvTalkData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);



static BOOL NetTestNone(void* pCtl);
static BOOL NetTestSendTiming(void* pCtl);
static BOOL NetTestRecvTiming(void* pCtl);
static BOOL NetTestEndStart(void* pCtl);
static BOOL NetTestMoveStart(void* pCtl);
static BOOL NetTestMoveSend(void* pCtl);
static BOOL NetTestEnd(void* pCtl);

static void _connectCallBack(void* pWork);
static void* _netBeaconGetFunc(void* pWork);
static int _netBeaconGetSizeFunc(void* pWork);
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
//static void FatalError_Disp(GFL_NETHANDLE* pNet,int errNo, void* pWork);
static void _netConnectFunc(void* pWork,int hardID);    ///< ハードで接続した時に呼ばれる
static void _netNegotiationFunc(void* pWork,int hardID);    ///< ネゴシエーション完了時にコール
static BOOL _NetTestChild(void* pCtl);
static BOOL _NetTestParent(void* pCtl);

static void _endCallBack(void* pWork);
static u8* _recvMemory(int netID, void* pWork, int size);
static void _debug_bsubData(BSUBWAY_PARTNER_DATA* pData);


static const NetRecvFuncTable _CommPacketTbl[] = {
  {_RecvMoveData,         _recvMemory},    ///NET_CMD_MOVE
  {_RecvTalkData,         NULL},    ///NET_CMD_TALK
};


typedef struct{
  int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

typedef struct{
  int x;
  int y;
  int z;
  int rot;
  int anm;
} _testMoveStruct;

static _testBeaconStruct _testBeacon = { WB_NET_DEBUG_OHNO_SERVICEID };


static GFLNetInitializeStruct aGFLNetInit = {
  _CommPacketTbl,  // 受信関数テーブル
  NELEMS(_CommPacketTbl), // 受信テーブル要素数
  _netConnectFunc,    ///< ハードで接続した時に呼ばれる
  _netNegotiationFunc,    ///< ネゴシエーション完了時にコール
  NULL,   // ユーザー同士が交換するデータのポインタ取得関数
  NULL,   // ユーザー同士が交換するデータのサイズ取得関数
  _netBeaconGetFunc,  // ビーコンデータ取得関数
  _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
  _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,            // 普通のエラーが起こった場合 通信終了
  FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  _endCallBack,  // 通信切断時に呼ばれる関数
  NULL,  // オート接続で親になった場合
#if GFL_NET_WIFI
  NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL,  ///< wifiフレンドリスト削除コールバック
  NULL,   ///< DWC形式の友達リスト
  NULL,  ///< DWCのユーザデータ（自分のデータ）
  0,   ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
  SYASHI_NETWORK_GGID,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK,  //IRC用にcreateされるHEAPID
  GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
  4,                            // 最大接続人数
  88,                  //最大送信バイト数
  _BCON_GET_NUM,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIRELESS,  //通信種別
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_DEBUG_OHNO_SERVICEID,  //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
  500,
  0,
};

//--------------------------------------------------------------
/**
 * @brief   ビーコンデータ取得関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------

static void* _netBeaconGetFunc(void* pWork)
{
  return &_testBeacon;
}

///< ビーコンデータサイズ取得関数
static int _netBeaconGetSizeFunc(void* pWork)
{
  return sizeof(_testBeacon);
}

///< ビーコンデータ取得関数
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)
{
  if(myNo != beaconNo){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   移動データ受信のための領域確保関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------
static u8 dummymem[4*sizeof(_testBeacon)];

static u8* _recvMemory(int netID, void* pWork, int size)
{
  return &dummymem[netID*sizeof(_testBeacon)];
}

//--------------------------------------------------------------
/**
 * @brief   ビーコンデータ取得関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------
/*
static void FatalError_Disp(GFL_NETHANDLE* pNet,int errNo, void* pWork)
{
	// 通信不能なエラーが起こった場合呼ばれる 切断するしかない
	OS_TPrintf("通信不能エラーが発生 ErrorNo = %d\n",errNo);
}
 */
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

static void _RecvMoveData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  DEBUG_OHNO_CONTROL* pDOC = pWork;

  pDOC->bMoveRecv = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   会話コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------

static void _RecvTalkData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
}


//==============================================================================
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================

static void _changeState(DEBUG_OHNO_CONTROL* pDOC, NetTestFunc state)
{
  pDOC->funcNet = state;
}

//==============================================================================
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(DEBUG_OHNO_CONTROL* pDOC,NetTestFunc state, int line)
{
  NET_PRINT("df_state: %d\n",line);
  _changeState(pDOC, state);
}
#endif

#ifdef GFL_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pDOC ,state,  __LINE__)
#else  //GFL_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pDOC ,state )
#endif //GFL_NET_DEBUG



//--------------------------------------------------------------
/**
 * @brief   接続完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
#if 0
static BOOL _connectCallBack(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pWork;
  //   OS_TPrintf("GFL_NET_ChangeoverConnect OK\n");
  _CHANGE_STATE( NetTestSendTiming );
}
#endif
//--------------------------------------------------------------
/**
 * @brief   自動接続
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
static BOOL NetTestAutoConnect(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;

  if(pDOC->bParent){
    GFL_NET_InitServer();
    _CHANGE_STATE( _NetTestParent );
  }
  else{
    u8 mac[]={0xff,0xff,0xff,0xff,0xff,0xff};
    GFL_NET_InitClientAndConnectToParent(mac);

    //    GFL_NET_ChangeoverConnect(NULL);
    _CHANGE_STATE( _NetTestChild );
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   自動接続親機側
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
static BOOL _NetTestParent(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;

  if(GFL_NET_GetConnectNum()==4){
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,15,WB_NET_DEBUG_OHNO_SERVICEID);
    _CHANGE_STATE( NetTestRecvTiming );
  }
  return FALSE;
}



//--------------------------------------------------------------
/**
 * @brief   自動接続子機側
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------

static BOOL _NetTestChild2(void* pCtl)
{
   DEBUG_OHNO_CONTROL* pDOC = pCtl;
 
  if(GFL_NET_HANDLE_IsNegotiation( GFL_NET_HANDLE_GetCurrentHandle() ) ){
    int id = 1 - GFL_NET_HANDLE_GetNetHandleID( GFL_NET_HANDLE_GetCurrentHandle());
    if(GFL_NET_HANDLE_IsNegotiation( GFL_NET_GetNetHandle(id) ) ){
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,15,WB_NET_DEBUG_OHNO_SERVICEID);
      _CHANGE_STATE( NetTestRecvTiming );
    }
  }
  return FALSE;
}

static BOOL _NetTestChild(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;

  
  if(GFL_NET_HANDLE_RequestNegotiation()){
    _CHANGE_STATE( _NetTestChild2 );
  }
  return FALSE;
}



//--------------------------------------------------------------
/**
 * @brief   自動接続
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
static BOOL NetTestRecvTiming(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;

  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),15, WB_NET_DEBUG_OHNO_SERVICEID)){
    NET_PRINT("TIMOK\n");
    _CHANGE_STATE( NetTestMoveStart );
  }
  return FALSE;
}

//タイミングを取り続けるテスト

static int debugTiming = 1;
static BOOL _TimingTest(void* pCtl);

static BOOL _TimingTestNext(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;
  if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),debugTiming, WB_NET_DEBUG_OHNO_SERVICEID)){
    NET_PRINT("TIMOK %d\n",debugTiming);
    debugTiming++;
    _CHANGE_STATE( _TimingTest );
  }
  GF_ASSERT(!GFL_NET_IsError());
  return FALSE;
}


static BOOL _TimingTest(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;
  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,debugTiming,WB_NET_DEBUG_OHNO_SERVICEID);

  GF_ASSERT(!GFL_NET_IsError());

  _CHANGE_STATE(_TimingTestNext);
  return FALSE;
}



//--------------------------------------------------------------
/**
 * @brief   構造体転送テスト
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
static BOOL NetTestMoveStart(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;
  _testMoveStruct test={1,2,3,4,5};

  if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),NET_CMD_MOVE, sizeof(_testMoveStruct),&test)){
    _CHANGE_STATE( NetTestMoveSend );
  }
  return FALSE;
}


//--------------------------------------------------------------
/**
 * @brief   構造体転送テスト
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
static BOOL NetTestMoveSend(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;

  if(pDOC->bMoveRecv){
    NET_PRINT(" MOVE Recv\n");

    GFL_NET_SetNoChildErrorCheck(TRUE);
    _CHANGE_STATE( _TimingTest );

    //      _CHANGE_STATE( NetTestEndStart );
  }
  return FALSE;
}


//--------------------------------------------------------------
/**
 * @brief   初期化完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------

static void _endCallBack(void* pWork)
{
  DEBUG_OHNO_CONTROL* pDOC = pWork;
  NET_PRINT("endCallBack終了\n");
  _CHANGE_STATE( NetTestEnd );
}

//--------------------------------------------------------------
/**
 * @brief   自動接続
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
static BOOL NetTestEndStart(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;
  //    GFL_NET_Exit(_endCallBack);
  GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),GFL_NET_CMD_EXIT_REQ, 0, NULL);
  //    GFL_NET_SendData();
  _CHANGE_STATE( NetTestNone );
  return FALSE;
}


//--------------------------------------------------------------
/**
 * @brief   初期化完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------

static void _initCallBack(void* pWork)
{
  DEBUG_OHNO_CONTROL* pDOC = pWork;
  _CHANGE_STATE( NetTestAutoConnect );
}

//--------------------------------------------------------------
/**
 * @brief   なにもしない
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
static BOOL NetTestNone(void* pCtl)
{
  return FALSE;
}

static BOOL netinit(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;

  if( GFL_NET_IsInit() == FALSE ){  // もう通信している場合終了処理
    GFL_NET_Init(&aGFLNetInit, _initCallBack, pDOC);
  }
  return FALSE;
}



//--------------------------------------------------------------
/**
 * @brief   終了
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
static BOOL NetTestEnd(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;
  _CHANGE_STATE( netinit );
  return FALSE;
}




//------------------------------------------------------------------
//  デバッグ用初期化関数
//------------------------------------------------------------------

static GFL_PROC_RESULT DebugOhnoMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk,BOOL bParent)
{

  DEBUG_OHNO_CONTROL * pDOC;
  HEAPID			heapID = HEAPID_OHNO_DEBUG;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, heapID, 0x30000 );

  pDOC = GFL_PROC_AllocWork( proc, sizeof(DEBUG_OHNO_CONTROL), heapID );
  GFL_STD_MemClear(pDOC, sizeof(DEBUG_OHNO_CONTROL));
  pDOC->debug_heap_id = heapID;
  pDOC->bParent=bParent;
  
  _CHANGE_STATE( netinit );

  return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**   デバッグ用メイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  DEBUG_OHNO_CONTROL * testmode = mywk;

  if(testmode->funcNet){
    if(testmode->funcNet(mywk)){
      return GFL_PROC_RES_FINISH;
    }
  }
  return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**  デバッグ用Exit
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  GFL_PROC_FreeWork(proc);
  GFL_HEAP_DeleteHeap( HEAPID_OHNO_DEBUG );

  return GFL_PROC_RES_FINISH;
}


static GFL_PROC_RESULT NetFourAutoProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  return DebugOhnoMainProcInit(proc, seq, pwk, mywk,FALSE);
}


//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugOhnoMainProcData = {
  NetFourAutoProc_Init,
  DebugOhnoMainProcMain,
  DebugOhnoMainProcEnd,
};



///<
//--------------------------------------------------------------
/**
 * @brief   ハードで接続した時に呼ばれる
 * @param   pWork    デバッグワーク
 * @param   つながった機器のID
 * @retval  none
 */
//--------------------------------------------------------------
void _netConnectFunc(void* pWork,int hardID)
{
  NET_PRINT("_netConnectFunc %d\n", hardID);

}

//--------------------------------------------------------------
/**
 * @brief   ネゴシエーション完了時にコール
 * @param   pWork    デバッグワーク
 * @param   つながった機器のID
 * @retval  none
 */
//--------------------------------------------------------------
void _netNegotiationFunc(void* pWork,int NetID)
{

  NET_PRINT("_netNegotiationFunc %d\n", NetID);

}



static GFL_PROC_RESULT NetFourParentProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  return DebugOhnoMainProcInit(proc, seq, pwk, mywk,TRUE);
}

static GFL_PROC_RESULT NetFourChildProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  return DebugOhnoMainProcInit(proc, seq, pwk, mywk,FALSE);
}






// プロセス定義データ
const GFL_PROC_DATA NetFourParentProcData = {
  NetFourParentProc_Init,
  DebugOhnoMainProcMain,
  DebugOhnoMainProcEnd,
};

// プロセス定義データ
const GFL_PROC_DATA NetFourChildProcData = {
  NetFourChildProc_Init,
  DebugOhnoMainProcMain,
  DebugOhnoMainProcEnd,
};

//--------------------------------------------------------------------
//ここから不思議な贈り物
//--------------------------------------------------------------------

//不思議な贈り物のデータをセット
static void _fushigiDataSet(DEBUG_OHNO_CONTROL * pDOC)
{
  DOWNLOAD_GIFT_DATA* pDG;

  
  pDOC->aInit.NetDevID = WB_NET_MYSTERY;   // //通信種類
  pDOC->aInit.datasize = sizeof(DOWNLOAD_GIFT_DATA);   //データ全体サイズ
  pDOC->aInit.pData = GFL_HEAP_AllocClearMemory(HEAPID_OHNO_DEBUG,pDOC->aInit.datasize);     // データ
  pDOC->aInit.ConfusionID = 12;
  pDOC->aInit.heapID = HEAPID_OHNO_DEBUG;

  pDG = (DOWNLOAD_GIFT_DATA* )pDOC->aInit.pData;

  DEBUG_MYSTERY_SetGiftCommonData( &pDG->data, 12, FALSE );
  DEBUG_MYSTERY_SetGiftPokeData(&pDG->data,12);
  pDG->version = 0  //０なので全員受信できる
  pDG->event_text[0] = L'て';
  pDG->event_text[1] = L'す';
  pDG->event_text[2] = L'と';
  pDG->event_text[3] = 0xffff;

}

//親機なのでビーコン送信を行っているだけ
static BOOL _loop(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;
  DELIVERY_BEACON_Main(pDOC->pDBWork);
  return FALSE;
}

//不思議な贈り物送信最初
static GFL_PROC_RESULT NetDeliverySendProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  DEBUG_OHNO_CONTROL * pDOC;

  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OHNO_DEBUG, 0x30000 );
  pDOC = GFL_PROC_AllocWork( proc, sizeof(DEBUG_OHNO_CONTROL), HEAPID_OHNO_DEBUG );
  GFL_STD_MemClear(pDOC, sizeof(DEBUG_OHNO_CONTROL));

  {
    _fushigiDataSet(pDOC);
    pDOC->pDBWork=DELIVERY_BEACON_Init(&pDOC->aInit);
    GF_ASSERT(DELIVERY_BEACON_SendStart(pDOC->pDBWork));
  }
  _CHANGE_STATE( _loop ); // 

  return GFL_PROC_RES_FINISH;
}


//トライアルハウスダミーデータ
static void _debug_bsubDataMain(DEBUG_OHNO_CONTROL * pDOC)
{
  BATTLE_EXAMINATION_SAVEDATA* pDG;

  
  pDOC->aInit.NetDevID = WB_NET_BATTLE_EXAMINATION;   // //通信種類
  pDOC->aInit.datasize = sizeof(BATTLE_EXAMINATION_SAVEDATA);   //データ全体サイズ
  pDOC->aInit.pData = GFL_HEAP_AllocClearMemory(HEAPID_OHNO_DEBUG,pDOC->aInit.datasize);     // データ
  pDOC->aInit.ConfusionID = 12;
  pDOC->aInit.heapID = HEAPID_OHNO_DEBUG;

  pDG = (BATTLE_EXAMINATION_SAVEDATA* )pDOC->aInit.pData;

  pDG->bActive = BATTLE_EXAMINATION_MAGIC_KEY;
  _debug_bsubData( &pDG->trainer[0] );
  _debug_bsubData( &pDG->trainer[1] );
  _debug_bsubData( &pDG->trainer[2] );
  _debug_bsubData( &pDG->trainer[3] );
  _debug_bsubData( &pDG->trainer[4] );
  pDG->titleName[0] = L'て';
  pDG->titleName[1] = L'す';
  pDG->titleName[2] = 0xffff;
  pDG->crc = GFL_STD_CrcCalc(pDOC->aInit.pData, pDOC->aInit.datasize-2);

}




//トライアルハウスもビーコン配信
static GFL_PROC_RESULT NetDeliveryTriSendProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  DEBUG_OHNO_CONTROL * pDOC;

  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OHNO_DEBUG, 0x30000 );
  pDOC = GFL_PROC_AllocWork( proc, sizeof(DEBUG_OHNO_CONTROL), HEAPID_OHNO_DEBUG );
  GFL_STD_MemClear(pDOC, sizeof(DEBUG_OHNO_CONTROL));

  {
    //@todo ここにトライアルハウステストデータを入れる
    _debug_bsubDataMain(pDOC);

    
    pDOC->pDBWork=DELIVERY_BEACON_Init(&pDOC->aInit);
    GF_ASSERT(DELIVERY_BEACON_SendStart(pDOC->pDBWork));
  }
  _CHANGE_STATE( _loop ); // 

  return GFL_PROC_RES_FINISH;
}



//-------------------------------------------------------------------WIRLESS



// こちらは受信テスト
static BOOL _getTime(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;
  int i,j;

  DELIVERY_BEACON_Main(pDOC->pDBWork);
  
  pDOC->counter++;

  if(DELIVERY_BEACON_RecvCheck(pDOC->pDBWork) ){  // もう通信している場合終了処理
    OS_TPrintf("受信完了 %d\n",pDOC->counter);

    for(j=0;j<pDOC->aInit.datasize;){
      for(i=0;i<16;i++){
        OS_TPrintf("%x ",pDOC->aInit.pData[j]);
        j++;
      }
      OS_TPrintf("\n");
    }
    DELIVERY_BEACON_End(pDOC->pDBWork);
    _CHANGE_STATE(NetTestNone);
  }
  return FALSE;
}

//受信のために領域確保
static void _fushigiDataRecv(DEBUG_OHNO_CONTROL * pDOC)
{
  DOWNLOAD_GIFT_DATA* pDG;

  pDOC->aInit.NetDevID = WB_NET_MYSTERY;   // //通信種類
  pDOC->aInit.datasize = sizeof(DOWNLOAD_GIFT_DATA);   //データ全体サイズ
  pDOC->aInit.pData = GFL_HEAP_AllocClearMemory(HEAPID_OHNO_DEBUG,pDOC->aInit.datasize);     // データ
  pDOC->aInit.ConfusionID = 12;
  pDOC->aInit.heapID = HEAPID_OHNO_DEBUG;

}

//受信のために領域確保
static void _traialDataRecv(DEBUG_OHNO_CONTROL * pDOC)
{
  DOWNLOAD_GIFT_DATA* pDG;

  pDOC->aInit.NetDevID = WB_NET_BATTLE_EXAMINATION;   // //通信種類
  pDOC->aInit.datasize = sizeof(BATTLE_EXAMINATION_SAVEDATA);   //データ全体サイズ
  pDOC->aInit.pData = GFL_HEAP_AllocClearMemory(HEAPID_OHNO_DEBUG,pDOC->aInit.datasize);     // データ
  pDOC->aInit.ConfusionID = 12;
  pDOC->aInit.heapID = HEAPID_OHNO_DEBUG;

}

//受信の初期化
static GFL_PROC_RESULT NetDeliveryRecvProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  DEBUG_OHNO_CONTROL * pDOC;

  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OHNO_DEBUG, 0x30000 );
  pDOC = GFL_PROC_AllocWork( proc, sizeof(DEBUG_OHNO_CONTROL), HEAPID_OHNO_DEBUG );
  GFL_STD_MemClear(pDOC, sizeof(DEBUG_OHNO_CONTROL));

  
  {
    _fushigiDataRecv(pDOC);
    
    pDOC->pDBWork=DELIVERY_BEACON_Init(&pDOC->aInit);
    GF_ASSERT(DELIVERY_BEACON_RecvStart(pDOC->pDBWork));
  }
  pDOC->counter=0;
  _CHANGE_STATE( _getTime ); // 


  return GFL_PROC_RES_FINISH;
}


// プロセス定義データ ビーコン配信
const GFL_PROC_DATA NetDeliverySendProcData = {
  NetDeliverySendProc_Init,
  DebugOhnoMainProcMain,
  DebugOhnoMainProcEnd,
};


// プロセス定義データ  ビーコン受信
const GFL_PROC_DATA NetDeliveryRecvProcData = {
  NetDeliveryRecvProc_Init,
  DebugOhnoMainProcMain,
  DebugOhnoMainProcEnd,
};



// プロセス定義データ  トライアルハウスビーコン配信
const GFL_PROC_DATA NetDeliveryTriSendProcData = {
  NetDeliveryTriSendProc_Init,
  DebugOhnoMainProcMain,
  DebugOhnoMainProcEnd,
};








//--------------------------------------------------------------------IRC

//-------------------------------------------------------------------IRC




static BOOL _getIRCTime(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;
  int i,j,k;

  DELIVERY_IRC_Main(pDOC->pIRCWork);
  
  pDOC->counter++;
  k = DELIVERY_IRC_RecvCheck(pDOC->pIRCWork);
  if( k ){  // もう通信している場合終了処理
    OS_TPrintf("受信完了 %d %d\n",pDOC->counter,k);

    for(j=0;j<pDOC->aIRCInit.datasize;){
      for(i=0;i<16;i++){
        OS_TPrintf("%x ",pDOC->aIRCInit.pData[j]);
        j++;
      }
      OS_TPrintf("\n");
    }
    DELIVERY_IRC_End(pDOC->pIRCWork);
    _CHANGE_STATE(NetTestNone);
  }
  return FALSE;
}


static void _fushigiDataIRCRecv(DEBUG_OHNO_CONTROL * pDOC)
{
  DOWNLOAD_GIFT_DATA* pDG;

  pDOC->aIRCInit.NetDevID = WB_NET_MYSTERY;   // //通信種類
  pDOC->aIRCInit.datasize = sizeof(DOWNLOAD_GIFT_DATA);   //データ全体サイズ
  pDOC->aIRCInit.pData = GFL_HEAP_AllocClearMemory(HEAPID_OHNO_DEBUG,pDOC->aIRCInit.datasize);     // データ
  pDOC->aIRCInit.ConfusionID = 12;
  pDOC->aIRCInit.heapID = HEAPID_OHNO_DEBUG;

}



static GFL_PROC_RESULT NetDeliveryIRCRecvProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  DEBUG_OHNO_CONTROL * pDOC;

  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OHNO_DEBUG, 0x30000 );
  pDOC = GFL_PROC_AllocWork( proc, sizeof(DEBUG_OHNO_CONTROL), HEAPID_OHNO_DEBUG );
  GFL_STD_MemClear(pDOC, sizeof(DEBUG_OHNO_CONTROL));

  
  {
    _fushigiDataIRCRecv(pDOC);
    
    pDOC->pIRCWork=DELIVERY_IRC_Init(&pDOC->aIRCInit);
    GF_ASSERT(DELIVERY_IRC_RecvStart(pDOC->pIRCWork));
  }
  pDOC->counter=0;
  _CHANGE_STATE( _getIRCTime ); // 


  return GFL_PROC_RES_FINISH;
}

//赤外線用テストデータセット
static void _fushigiDataIRCSet(DEBUG_OHNO_CONTROL * pDOC)
{
  DOWNLOAD_GIFT_DATA* pDG;

  pDOC->aIRCInit.NetDevID = WB_NET_MYSTERY;   // //通信種類
  pDOC->aIRCInit.datasize = sizeof(DOWNLOAD_GIFT_DATA);   //データ全体サイズ
  pDOC->aIRCInit.pData = GFL_HEAP_AllocClearMemory(HEAPID_OHNO_DEBUG,pDOC->aIRCInit.datasize);     // データ
  pDOC->aIRCInit.ConfusionID = 12;
  pDOC->aIRCInit.heapID = HEAPID_OHNO_DEBUG;

  pDG = (DOWNLOAD_GIFT_DATA* )pDOC->aIRCInit.pData;

  DEBUG_MYSTERY_SetGiftCommonData( &pDG->data, 12, FALSE );
  DEBUG_MYSTERY_SetGiftPokeData(&pDG->data,12);
  pDG->version = 12;
  pDG->event_text[0] = L'I';
  pDG->event_text[1] = L'R';
  pDG->event_text[2] = L'C';
  pDG->event_text[3] = 0xffff;

  {
    int i,j;
    for(j=0;j<pDOC->aIRCInit.datasize;){
      for(i=0;i<16;i++){
        OS_TPrintf("%x ",pDOC->aIRCInit.pData[j]);
        j++;
      }
      OS_TPrintf("\n");
    }
  }
  

}


static BOOL _loopirc(void* pCtl)
{
  DEBUG_OHNO_CONTROL* pDOC = pCtl;
  DELIVERY_IRC_Main(pDOC->pIRCWork);
  return FALSE;
}


static GFL_PROC_RESULT NetDeliveryIRCSendProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  DEBUG_OHNO_CONTROL * pDOC;

  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_OHNO_DEBUG, 0x30000 );
  pDOC = GFL_PROC_AllocWork( proc, sizeof(DEBUG_OHNO_CONTROL), HEAPID_OHNO_DEBUG );
  GFL_STD_MemClear(pDOC, sizeof(DEBUG_OHNO_CONTROL));

  {

    _fushigiDataIRCSet(pDOC);
    
    pDOC->pIRCWork=DELIVERY_IRC_Init(&pDOC->aIRCInit);
    GF_ASSERT(DELIVERY_IRC_SendStart(pDOC->pIRCWork));
  }
  _CHANGE_STATE( _loopirc ); // 

  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ   赤外線配信部分
const GFL_PROC_DATA NetDeliveryIRCSendProcData = {
  NetDeliveryIRCSendProc_Init,
  DebugOhnoMainProcMain,
  DebugOhnoMainProcEnd,
};



// プロセス定義データ  赤外線受信テスト
const GFL_PROC_DATA NetDeliveryIRCRecvProcData = {
  NetDeliveryIRCRecvProc_Init,
  DebugOhnoMainProcMain,
  DebugOhnoMainProcEnd,
};




///------------------------------バトル検定








#if 0
//--------------------------------------------------------------
/// バトルサブウェイロム用トレーナーデータ構造体
/// トレーナー名、会話データは、gmmで管理（トレーナーIDから取得）
//--------------------------------------------------------------
typedef struct
{
	u16		tr_type;				//トレーナータイプ
	u16		use_poke_cnt;			//使用可能ポケモン数	
	u16		use_poke_table[1];		//使用可能ポケモンINDEXテーブル（可変長）
}BSUBWAY_TRAINER_ROM_DATA;

//--------------------------------------------------------------
/// バトルサブウェイロム用ポケモンデータ構造体
//--------------------------------------------------------------
typedef struct
{
	u16		mons_no;				//モンスターナンバー
	u16		waza[WAZA_TEMOTI_MAX];	//所持技
	u8		exp_bit;				//努力値振り分けビット
	u8		chr;					//性格
	u16		item_no;				//装備道具
	u16		form_no;				//フォルムナンバー
}BSUBWAY_POKEMON_ROM_DATA;

//--------------------------------------------------------------
/// Wifiデータ
/// サブウェイ用ポケモンデータ型
/// セーブデータとやり取りするのでsavedata/b_tower.hにtypedef定義を切り
/// 不完全ポインタでやり取りできるようにしておく
//(Dpw_Bt_PokemonData)
//--------------------------------------------------------------
struct _BSUBWAY_POKEMON
{
	union{
		struct{
			u16	mons_no:11;	///<モンスターナンバー
			u16	form_no:5;	///<モンスターナンバー
		};
		u16	mons_param;
	};
	u16	item_no;	///<装備道具

	u16	waza[WAZA_TEMOTI_MAX];		///<所持技

	u32	id_no;					///<IDNo
	u32	personal_rnd;			///<個性乱数

	union{
		struct{
		u32	hp_rnd		:5;		///<HPパワー乱数
		u32	pow_rnd		:5;		///<POWパワー乱数
		u32	def_rnd		:5;		///<DEFパワー乱数
		u32	agi_rnd		:5;		///<AGIパワー乱数
		u32	spepow_rnd	:5;		///<SPEPOWパワー乱数
		u32	spedef_rnd	:5;		///<SPEDEFパワー乱数
		u32	ngname_f	:1;		///<NGネームフラグ
		u32				:1;		//1ch 余り
		};
		u32 power_rnd;
	};

	union{
		struct{
		u8	hp_exp;				///<HP努力値
		u8	pow_exp;			///<POW努力値
		u8	def_exp;			///<DEF努力値
		u8	agi_exp;			///<AGI努力値
		u8	spepow_exp;			///<SPEPOW努力値
		u8	spedef_exp;			///<SPEDEF努力値
		};
		u8 exp[6];
	};
	union{
		struct{
		u8	pp_count0:2;	///<技1ポイントアップ
		u8	pp_count1:2;	///<技2ポイントアップ
		u8	pp_count2:2;	///<技3ポイントアップ
		u8	pp_count3:2;	///<技4ポイントアップ
		};
		u8 pp_count;
	};

	u8	country_code;			///<国コード

	u8	tokusei;				///<特性
	u8	natuki;				///<なつき度

	///ニックネーム ((MONS_NAME_SIZE:10)+(EOM_SIZE:1))*(STRCODE:u16)=22
	STRCODE nickname[MONS_NAME_SIZE+EOM_SIZE];
};

//--------------------------------------------------------------
/// サブウェイトレーナーデータ
//--------------------------------------------------------------
typedef struct _BSUBWAY_TRAINER
{
	u32		player_id;	///<トレーナーのID
	u16		tr_type;	///<トレーナータイプ
	u16		dummy;		///<ダミー
	STRCODE	name[PERSON_NAME_SIZE+EOM_SIZE];
	
	u16		appear_word[4];						//登場メッセージ	
	u16		win_word[4];						//勝利メッセージ	
	u16		lose_word[4];						//敗退メッセージ
}BSUBWAY_TRAINER;

//--------------------------------------------------------------
/// バトルサブウェイ　対戦相手データ構造体
//--------------------------------------------------------------
struct _BSUBWAY_PARTNER_DATA
{
	BSUBWAY_TRAINER	bt_trd;			//トレーナーデータ
	struct _BSUBWAY_POKEMON	btpwd[4];		//持ちポケモンデータ
};


#endif


static void _debug_bsubData(BSUBWAY_PARTNER_DATA* pData)
{
  int i;

  pData->bt_trd.player_id =10;
  pData->bt_trd.tr_type = 20;
  pData->bt_trd.name[0]= L'で';
  pData->bt_trd.name[1]= L'ば';
  pData->bt_trd.name[2]= L'ぐ';
  pData->bt_trd.name[3]= 0xffff;
  pData->bt_trd.appear_word[0]=1;
  pData->bt_trd.appear_word[1]=2;
  pData->bt_trd.appear_word[2]=3;
  pData->bt_trd.appear_word[3]=4;
  pData->bt_trd.win_word[0]=1;
  pData->bt_trd.win_word[1]=2;
  pData->bt_trd.win_word[2]=3;
  pData->bt_trd.win_word[3]=4;
  pData->bt_trd.lose_word[0]=1;
  pData->bt_trd.lose_word[1]=2;
  pData->bt_trd.lose_word[2]=3;
  pData->bt_trd.lose_word[3]=4;
  for(i=0;i<4;i++){
    BSUBWAY_POKEMON* pPoke = &pData->btpwd[i];
    pPoke->mons_param = 1;
    pPoke->item_no=0;
    pPoke->waza[0]=WAZANO_HAPPAKATTAA;
    pPoke->waza[1]=WAZANO_NULL;
    pPoke->waza[2]=WAZANO_NULL;
    pPoke->waza[3]=WAZANO_NULL;
    pPoke->id_no = 12;
    pPoke->nickname[0]=0xffff;
  }
}

