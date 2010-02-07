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
#include "net\network_define.h"
#include "net\dwc_raputil.h"
#include "wmi.naix"

static DEBUG_OHNO_CONTROL * DebugOhnoControl;

FS_EXTERN_OVERLAY(dev_wifi);


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
  0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
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





