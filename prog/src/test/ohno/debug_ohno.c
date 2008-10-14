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
#include "wmi.naix"

static DEBUG_OHNO_CONTROL * DebugOhnoControl;


//------------------------------------------------------------------
//  受信テーブル
//------------------------------------------------------------------

static void _RecvMoveData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _RecvTalkData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);



static BOOL NetTestNone(void* pCtl);
static BOOL NetTestSendTiming(void* pCtl);
static BOOL NetTestRecvTiming(void* pCtl);
static BOOL NetTestEndStart(void* pCtl);

static void _connectCallBack(void* pWork);
static void* _netBeaconGetFunc(void);
static int _netBeaconGetSizeFunc(void);
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
//static void FatalError_Disp(GFL_NETHANDLE* pNet,int errNo, void* pWork);

static const NetRecvFuncTable _CommPacketTbl[] = {
    {_RecvMoveData,         NULL},    ///NET_CMD_MOVE
    {_RecvTalkData,         NULL},    ///NET_CMD_TALK
};


typedef struct{
    int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { WB_NET_DEBUG_OHNO_SERVICEID };


static GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), // 受信テーブル要素数
    NULL,   // ユーザー同士が交換するデータのポインタ取得関数
    NULL,   // ユーザー同士が交換するデータのサイズ取得関数
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
    0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    TRUE,     // CRC計算
    FALSE,     // MP通信＝親子型通信モードかどうか
    FALSE,  //wifi通信を行うかどうか
    TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_DEBUG_OHNO_SERVICEID,  //GameServiceID
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

static void* _netBeaconGetFunc(void)
{
	return &_testBeacon;
}

///< ビーコンデータサイズ取得関数
static int _netBeaconGetSizeFunc(void)
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

static void _connectCallBack(void* pWork)
{
    DEBUG_OHNO_CONTROL* pDOC = pWork;
    OS_TPrintf("ネゴシエーション完了\n");
    _CHANGE_STATE( NetTestSendTiming );
}

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
    
    GFL_NET_ChangeoverConnect(_connectCallBack); // 自動接続
    _CHANGE_STATE( NetTestNone );

    return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   自動接続
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
static BOOL NetTestSendTiming(void* pCtl)
{
    DEBUG_OHNO_CONTROL* pDOC = pCtl;
    
    GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,15);

    _CHANGE_STATE( NetTestRecvTiming );
    
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
    
    if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),15)){
        NET_PRINT("TIMOK\n");
        _CHANGE_STATE( NetTestEndStart );
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
    GFL_NET_Exit(_endCallBack);
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

void _initCallBack(void* pWork)
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

//--------------------------------------------------------------
/**
 * @brief   終了
 * @param   pCtl    デバッグワーク
 * @retval  PROC終了時にはTRUE
 */
//--------------------------------------------------------------
BOOL NetTestEnd(void* pCtl)
{
    return TRUE;
}








//------------------------------------------------------------------
//  デバッグ用初期化関数
//------------------------------------------------------------------

static GFL_PROC_RESULT DebugOhnoMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DEBUG_OHNO_CONTROL * testmode;
	HEAPID			heapID = HEAPID_OHNO_DEBUG;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, heapID, 0x30000 );

	testmode = GFL_PROC_AllocWork( proc, sizeof(DEBUG_OHNO_CONTROL), heapID );
	GFL_STD_MemClear(testmode, sizeof(DEBUG_OHNO_CONTROL));
	testmode->debug_heap_id = heapID;

    if( GFL_NET_IsInit() == FALSE ){  // もう通信している場合終了処理
        GFL_NET_Init(&aGFLNetInit, _initCallBack, testmode);
    }
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


//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugOhnoMainProcData = {
	DebugOhnoMainProcInit,
	DebugOhnoMainProcMain,
	DebugOhnoMainProcEnd,
};

