//=============================================================================
/**
 * @file	net_state.c
 * @brief	通信状態を管理するサービス  通信の上位にある
 *          スレッドのひとつとして働き、自分の通信状態や他の機器の
 *          開始や終了を管理する
 * @author	k.ohno
 * @date    2006.01.12
 */
//=============================================================================

#include "gflib.h"

#include "device/wih.h"

#include "net_def.h"
#include "device/net_whpipe.h"
#include "device/dwc_rap.h"
#include "net_system.h"
#include "net_command.h"
#include "net_state.h"
#include "net_auto.h"

#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "net_handle.h"
#include "wm_icon.h"

//==============================================================================
// ワーク
//==============================================================================



///< 親機のみで必要なデータ
struct _NET_PARENTSYS_t{
    u8 negoCheck[GFL_NET_MACHINE_MAX];     ///< 各子機のネゴシエーション状態
};

typedef struct _NET_WORK_STATE  GFL_NETSTATE;


/// コールバック関数の書式 内部ステート遷移用
typedef void (*PTRStateFunc)(GFL_NETSTATE* pState);

struct _NET_WORK_STATE{
  NET_PARENTSYS sParent;  ///< 親の情報を保持するポインタ
  PTRStateFunc state;      ///< ハンドルのプログラム状態
  GFL_STD_RandContext sRand; ///< 親子機ネゴシエーション用乱数キー
  PSyncroCallback pSCallBack;  ///< 自動接続時に同期が完了した場合呼ばれる
  u8 aMacAddress[6];       ///< 接続先MACアドレス格納バッファ  アライメントが必要!!
  u16 timer;           ///< 進行タイマー
  u8 autoConnectNum;     ///< 自動接続に必要な人数
  u8 bFirstParent;     ///< 繰り返し親子切り替えを行う場合の最初の親状態
  u8 stateError;
  u8 bErrorAuto;
  u8 bDisconnectError;
  u8 bDisconnectState;  ///< 切断状態に入っている場合TRUE
  u8 ResetStateType;   ///< 通信リセットがかかるときの種類
};

static GFL_NETSTATE* _pNetState = NULL;

//==============================================================================
// 定義
//==============================================================================

#define _PACKETSIZE_DEFAULT         (512)  // パケットサイズの最大サイズ

#define _START_TIME (50)          /// 開始時間
#define _CHILD_P_SEARCH_TIME (32) /// 子機として親を探す時間
#define _EXIT_SENDING_TIME (5)
#define _SEND_NAME_TIME (10)

#define _WIFIMODE_PARENT (-1)  // 親としてwifi起動
#define _WIFIMODE_RANDOM (-2)  // ランダムマッチ接続

//==============================================================================
// static宣言
//==============================================================================

// ステートの初期化

static void _changeStateDebug(GFL_NETSTATE* pState, PTRStateFunc state, int time, int line);  // ステートを変更する
static void _changeState(GFL_NETSTATE* pState, PTRStateFunc state, int time);  // ステートを変更する
static void _changeoverChildSearching(GFL_NETSTATE* pState);
static void _errorDispCheck(GFL_NETSTATE* pState);

#ifdef GFL_NET_DEBUG
#if 1
#define   _CHANGE_STATE(state, time)  _changeStateDebug(_pNetState ,state, time, __LINE__)
#else
#define   _CHANGE_STATE(state, time)  _changeState(_pNetState ,state, time)
#endif
#else  //GFL_NET_DEBUG
#define   _CHANGE_STATE(state, time)  _changeState(_pNetState ,state, time)
#endif //GFL_NET_DEBUG


// その他一般的なステート
static void _stateEnd(GFL_NETSTATE* pState);             // 終了処理
static void _stateConnectEnd(GFL_NETSTATE* pState);      // 切断処理開始
static void _stateNone(GFL_NETSTATE* pState);            // 何もしない


//==============================================================================
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================

static void _changeState(GFL_NETSTATE* pState,PTRStateFunc state, int time)
{
    pState->state = state;
    pState->timer = time;
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
static void _changeStateDebug(GFL_NETSTATE* pState,PTRStateFunc state, int time, int line)
{
    NET_PRINT("net_state: %d\n",line);
    _changeState(pState, state, time);
}
#endif

//==============================================================================
/**
 * @brief   通信管理ステートの初期化処理
 * @param   MYSTATUS* pMyStatus
 * @retval  none
 */
//==============================================================================

static void _commStateInitialize(HEAPID heapID)
{
    // 初期化
    _pNetState = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_NETSTATE));
    _pNetState->timer = _START_TIME;
    _pNetState->bFirstParent = TRUE;  // 親の初めての起動の場合TRUE
    GFL_NET_COMMAND_Init(NULL, 0, NULL);
}

static void _endIchneumon(void* pWork, BOOL bRet)
{

}

//==============================================================================
/**
 * @brief   通信管理ステートの終了処理
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _stateFinalize(GFL_NETSTATE* pState)
{
    if(pState==NULL){  // すでに終了している
        return;
    }
    GFL_NET_COMMAND_Exit();
    GFL_NET_WirelessIconEasyEnd();
    GFL_NET_ExitIchneumon(_endIchneumon,NULL);
    _GFL_NET_SetNETWL(NULL);
    GFL_HEAP_FreeMemory(_pNetState);
    _pNetState = NULL;
}

//==============================================================================
/**
 * @brief   動いているかどうか
 * @param   none
 * @retval  動いている場合TRUE
 */
//==============================================================================

static BOOL _stateIsMove(GFL_NETSTATE* pState)
{
    if(pState->state){
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   イクニューモン初期化後通信起動
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _deviceInitialize(GFL_NETSTATE* pState)
{
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();
    GFL_NETWL* pWL;
    
    if(!GFL_NET_IsIchneumon()){
        return;  //
    }

    NET_PRINT("%x - %x %x\n",(u32)pNetIni,(u32)pNetIni->beaconGetFunc,(u32)pNetIni->beaconGetSizeFunc);
    if(!pNetIni->bWiFi){
        pWL = GFL_NET_WLGetHandle(pNetIni->netHeapID, pNetIni->gsid, pNetIni->maxConnectNum);
        _GFL_NET_SetNETWL(pWL);

        GFL_NET_WLInitialize(pNetIni->netHeapID, pNetIni->beaconGetFunc, pNetIni->beaconGetSizeFunc,
                             pNetIni->beaconCompFunc);
    }
    GFL_NET_SystemReset();         // 今までの通信バッファをクリーンにする

    NET_PRINT("再起動    -- \n");
    _CHANGE_STATE(_stateNone, 0);
}

static void _initIchneumon(void* pWork, BOOL bRet)
{
    if(bRet){
        _CHANGE_STATE(_deviceInitialize, 0);
    }
}

//==============================================================================
/**
 * @brief   通信デバイスを初期化する
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateDeviceInitialize(HEAPID heapID)
{
    GFL_UI_SleepDisable(GFL_UI_SLEEP_NET);  // スリープ禁止

    GFL_NET_InitIchneumon(_initIchneumon, NULL);
    _commStateInitialize(heapID);
}

//==============================================================================
/**
 * @brief   子機待機状態  親機に情報を送信
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _childSendNego(GFL_NETSTATE* pState)
{
    //@@OOエラー処理
    if(GFL_NET_SystemIsError()){
        //NET_PRINT("エラーの場合戻る\n");
     //   _CHANGE_STATE(_battleChildReset, 0);
    }
  //  if(CommIsConnect(CommGetCurrentID()) && ( GFL_NET_PARENT_NETID != CommGetCurrentID())){
   //     _CHANGE_STATE(_battleChildWaiting, 0);
   // }
}


//==============================================================================
/**
 * @brief   親機ビーコンを拾った場合のコールバック
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _parentFindCallback(void)
{
    _pNetState->timer+=60;  //子機時間を延長
}

//==============================================================================
/**
 * @brief   子機待機状態  親機に許可もらい中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _childConnecting(GFL_NETSTATE* pState)
{
    GFL_NET_WLParentBconCheck();

    if(GFL_NET_SystemChildModeInitAndConnect(FALSE, pState->aMacAddress, _PACKETSIZE_DEFAULT,_parentFindCallback)){
        _CHANGE_STATE(_childSendNego, _SEND_NAME_TIME);
    }

}

//==============================================================================
/**
 * @brief   マックアドレスを指定して子機接続開始
 * @param   connectIndex 接続する親機のIndex
 * @param   bAlloc       メモリーの確保
 * @retval  none
 */
//==============================================================================

static void _childAutoConnect(GFL_NETSTATE* pState)
{
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

    if(GFL_NET_SystemChildModeInit(TRUE, 512)){
        if(pNetIni->bMPMode){
 //           GFL_NET_SystemSetTransmissonTypeMP();
        }
        
        _CHANGE_STATE(_childConnecting, 0);
    }
}

//==============================================================================
/**
 * @brief   マックアドレスを指定して子機接続開始
 * @param   macAddress    アドレス
 * @param   bInit         初期化するのかどうか
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateConnectMacAddress(u8* macAddress, BOOL bInit)
{
    GF_ASSERT(((u32)_pNetState->aMacAddress%2)==0);
    GFL_STD_MemCopy(macAddress, _pNetState->aMacAddress, sizeof(_pNetState->aMacAddress));
    if(bInit){
        _CHANGE_STATE(_childAutoConnect, 0);
    }
    else{
        _CHANGE_STATE(_childConnecting, 0);
    }
}

//==============================================================================
/**
 * @brief   子機待機状態  ビーコン収集
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _childScanning(GFL_NETSTATE* pState)
{
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();
    GFL_NET_WLParentBconCheck();

    if(pNetIni->bMPMode){
//        GFL_NET_SystemSetTransmissonTypeMP();
    }
}

//==============================================================================
/**
 * @brief   子機待機状態  初期化待ち
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _childIniting(GFL_NETSTATE* pState)
{
    if(GFL_NET_SystemChildModeInit(TRUE,512)){
        _CHANGE_STATE(_childScanning, 0);
    }
}

//==============================================================================
/**
 * @brief   子機開始  ビーコンの収集に入る
 * @param   connectIndex 接続する親機のIndex
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateBeaconScan(void)
{
    _CHANGE_STATE(_childIniting, 0);
}


//==============================================================================
/**
 * @brief   通信管理ステートの処理
 * @param
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateMainProc(void)
{
    PTRStateFunc state = _pNetState->state;
    if(state != NULL){
        state(_pNetState);
    }
    _errorDispCheck(_pNetState);
}

//==============================================================================
/**
 * @brief   親機として待機中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _parentWait(GFL_NETSTATE* pState)
{
}

//==============================================================================
/**
 * @brief   親機初期化待機中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _parentInitWait(GFL_NETSTATE* pState)
{
    if(GFI_NET_SystemParentModeInitProcess()){
        _CHANGE_STATE(_parentWait, 0);
    }
}

//==============================================================================
/**
 * @brief   親機として初期化を行う
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _parentInit(GFL_NETSTATE* pState)
{
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

    if(!GFL_NET_IsIchneumon()){
        return;
    }

    if(GFI_NET_SystemParentModeInit(TRUE, _PACKETSIZE_DEFAULT)){
//        pState->negotiation = _NEGOTIATION_OK;  // 自分は認証完了
//        pState->creatureNo = 0;
        _CHANGE_STATE(_parentInitWait, 0);
    }
}

//==============================================================================
/**
 * @brief   親としての通信処理開始
 * @param   pNetHandle  ハンドル
 * @param   heapID      HEAPID
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateCreateParent(HEAPID heapID)
{
//    _pNetState->pParent = GFL_HEAP_AllocClearMemory( heapID, sizeof(NET_PARENTSYS));

    //_pNetState->negotiation = _NEGOTIATION_OK;  // 自分は認証完了
   // _pNetState->creatureNo = 0;
    _CHANGE_STATE(_parentInitWait, 0);
}

//==============================================================================
/**
 * @brief   親としての通信処理開始
 * @param   pNetHandle  ハンドル
 * @param   heapID      HEAPID
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateConnectParent(HEAPID heapID)
{
    _CHANGE_STATE(_parentInit, 0);
}


//==============================================================================
/**
 * @brief   子機再スタート
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _changeoverChildRestart(GFL_NETSTATE* pState)
{
    u32 rand;
    
    if(!GFL_NET_WLIsStateIdle()){  // 終了処理がきちんと終わっていることを確認
        return;
    }
    // 今度はビーコンを残したまま

    
    if(GFL_NET_SystemChildModeInitAndConnect(TRUE, NULL,_PACKETSIZE_DEFAULT,_parentFindCallback)){
        rand = GFL_STD_Rand(&pState->sRand, (_CHILD_P_SEARCH_TIME));
        NET_PRINT("子機開始 %d \n",rand);
        _CHANGE_STATE(_changeoverChildSearching, rand);
    }
}

//==============================================================================
/**
 * @brief   親機になりbcon放出
 * @param   none
 * @retval  none
 */
//==============================================================================


static void _changeoverParentRestart(GFL_NETSTATE* pState)
{
    if( GFL_NET_WLSwitchParentChild() ){
        _CHANGE_STATE(_changeoverChildRestart, 0);
        NET_PRINT("親機おわり\n");
    }
}


//==============================================================================
/**
 * @brief   ここから先エラーの検査を通信が処理するかどうかを設定
 * @param   bFlg  TRUEで検査開始
 * @retval  none
 */
//==============================================================================

static BOOL _getErrorCheck(GFL_NETSTATE* pState)
{
    if(pState->stateError!=0){
        return TRUE;
    }
    return pState->bErrorAuto;
}


//==============================================================================
/**
 * @brief   親として接続中
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _changeoverParentConnect(GFL_NETSTATE* pState)
{

    if(!_getErrorCheck(pState)){
        if(!GFL_NET_SystemIsChildsConnecting()){   // 自分以外がつながってないばあいもう一回
            NET_PRINT("親機しっぱい\n");
            if( GFL_NET_WLSwitchParentChild() ){
                _CHANGE_STATE(_changeoverChildRestart, 0);
            }
        }
    }
    else{
        if(GFL_NET_SystemIsError()){
            NET_PRINT("------エラーの場合Reset\n");
//            _CHANGE_STATE(_ChildReset, 0);   //@@OO
            return;
        }
    }
}



//==============================================================================
/**
 * @brief   親機になりbcon放出
 * @param   none
 * @retval  none
 */
//==============================================================================


static void _changeoverParentWait(GFL_NETSTATE* pState)
{
    if(GFL_NET_SystemIsChildsConnecting()){   // 自分以外がつながったら親機固定

        NET_PRINT("親機 -- つながり\n");
//        pNetHandle->bFirstParent = TRUE;  // 親機として繋がったのでフラグを戻しておく
        //WirelessIconEasy();  //@@OO
        {
//            GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
//            GFL_NETHANDLE* pHandleServer;
  //          pHandleServer = GFL_NET_CreateHandle();
            GFL_NET_CreateServer();   // サーバ
            GFI_NET_AutoParentConnectFunc();
        }
        _CHANGE_STATE(_changeoverParentConnect, 0);
        return;
    }

    if(GFL_NET_WLIsParentBeaconSent()){  // ビーコンを送り終わったらしばらく待つ
        if(pState->timer!=0){
            pState->timer--;
            return;
        }
    }
    else{
        return;
    }
    _CHANGE_STATE(_changeoverParentRestart, 0);
}


//==============================================================================
/**
 * @brief   親機開始
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _changeoverParentInit(GFL_NETSTATE* pState)
{
    if(!GFL_NET_WLIsStateIdle()){  // 終了処理がきちんと終わっていることを確認
        return;
    }
    // 親機になってみる
    if(GFI_NET_SystemParentModeInit(FALSE,  _PACKETSIZE_DEFAULT))  {
        pState->bFirstParent = FALSE;
        NET_PRINT("親機\n");
        _CHANGE_STATE(_changeoverParentWait, 30);
    }
}

//==============================================================================
/**
 * @brief   子機となって親機を探し中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _changeoverChildSearching(GFL_NETSTATE* pState)
{
    int realParent;

    GFL_NET_WLParentBconCheck();  // bconの検査

    if(GFL_NET_SystemGetCurrentID()!=GFL_NET_PARENT_NETID){  // 子機として接続が完了した
        NET_PRINT("子機になった\n");
        _CHANGE_STATE(_stateNone, 0);
        return;
    }

    if(pState->timer != 0){
        NET_PRINT("time %d\n",pState->timer);
        pState->timer--;
        return;
    }

    if(!GFL_NET_WLSwitchParentChild()){
        return;
    }
    NET_PRINT("子機おわり\n");
    _CHANGE_STATE(_changeoverParentInit, 0);
}

//==============================================================================
/**
 * @brief   親子自動切り替え通信処理開始
 * @param   pNetHandle  ハンドル
 * @param   heapID      HEAPID
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateChangeoverConnect(HEAPID heapID)
{
    GFL_NET_SystemChildModeInitAndConnect(TRUE, NULL,_PACKETSIZE_DEFAULT,_parentFindCallback);
    _CHANGE_STATE(_changeoverChildSearching, 30);
}

//==============================================================================
/**
 * @brief   子機の終了を待ち、親機が終了する
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _stateEndParentWait(GFL_NETSTATE* pState)
{
    if(!GFL_NET_WLIsChildsConnecting()){
        _stateFinalize(pState);
    }
    _CHANGE_STATE(_stateConnectEnd, _EXIT_SENDING_TIME);
}

//==============================================================================
/**
 * @brief   終了コマンド 子機が親機にやめるように送信  全員の子機に送り返すGFL_NET_CMD_EXIT_REQ
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateRecvExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    NET_PRINT("EXITコマンド受信\n");

    GFL_NET_SendData(pNetHandle, GFL_NET_CMD_EXIT, NULL);

    _CHANGE_STATE(_stateEndParentWait, 0);


#if 0
    if(CommGetCurrentID() == GFL_NET_PARENT_NETID){
        pNetHandle->disconnectType = 0;
        _CHANGE_STATE(_stateWifiMatchEnd, 0);
    }
    else{
        pNetHandle->disconnectType = 1;
        _CHANGE_STATE(_stateWifiMatchEnd, 0);
    }
    pNetHandle->bWifiDisconnect = TRUE;
#endif
}

//==============================================================================
/**
 * @brief   終了コマンド   子機が親機にやめるように送信 ぶっつりきる GFL_NET_CMD_EXIT
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateRecvExitStart(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    if(GFL_NET_SystemGetCurrentID() != GFL_NET_PARENT_NETID){
        GFL_NET_Disconnect();
    }
}

//==============================================================================
/**
 * @brief  終了処理中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _stateEnd(GFL_NETSTATE* pState)
{
    if(GFL_NET_SystemIsInitialize()){
        return;
    }
    _stateFinalize(pState);
}

//==============================================================================
/**
 * @brief  終了処理開始
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _stateConnectEnd(GFL_NETSTATE* pState)
{
    if(pState->timer != 0){
        pState->timer--;
    }
    if(!GFL_NET_WLSwitchParentChild()){
        return;
    }
    if(pState->timer != 0){
        return;
    }
    NET_PRINT("切断する");
    GFL_NET_SystemFinalize();
    _CHANGE_STATE(_stateEnd, 0);
}

//==============================================================================
/**
 * @brief   通信処理終了手続き開始
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateExit(void)
{
    if(_pNetState && _pNetState->bDisconnectState){
        return;
    }
    _pNetState->bDisconnectState = TRUE;
    _CHANGE_STATE(_stateConnectEnd, _EXIT_SENDING_TIME);
}

//==============================================================================
/**
 * @brief   何もしないステート
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _stateNone(GFL_NETSTATE* pState)
{
    // なにもしていない
}


///< DSMP状態遷移の定義
enum{
    _TRANS_NONE,
    _TRANS_LOAD,
    _TRANS_LOAD_END,
    _TRANS_SEND,
    _TRANS_SEND_END,
};

//==============================================================================
/**
 * @brief   DS通信MP通信の切り替え  GFL_NET_CMD_DSMP_CHANGE
 * @param   none
 * @retval  残り数
 */
//==============================================================================

void GFL_NET_StateRecvDSMPChange(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff = pData;
    int i;

    GFLNetInitializeStruct* pInit = _GFL_NET_GetNETInitStruct();

    pInit->bMPMode = pBuff[0];
    NET_PRINT("CommRecvDSMPChange 受信 %d\n",pBuff[0]);
}

//--------------------------wifi------------------------------------------------
#if GFL_NET_WIFI

//==============================================================================
/**
 * @brief   エラーが発生した状態
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================

static void _wifiErrorLoop(GFL_NETSTATE* pState)
{
}
static void _wifiDisconnectLoop(GFL_NETSTATE* pState)
{
}
static void _wifiFailedLoop(GFL_NETSTATE* pState)
{
}

//==============================================================================
/**
 * @brief   タイムアウトエラーが発生した状態
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================

static void _wifiTimeoutLoop(GFL_NETSTATE* pState)
{
    int ret;
    ret = mydwc_step();
    
	if( ret < 0 ){
        // タイムアウト時でもエラーが発生したらエラーループに移動する
        _CHANGE_STATE(_wifiErrorLoop, 0);
    }
}

//==============================================================================
/**
 * @brief   アプリケーション上での接続中の処理
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================

static void _wifiApplicationConnect(GFL_NETSTATE* pState)
{
    int ret,errCode;

    GFL_NET_SystemSetWifiConnect(TRUE);

    ret = mydwc_step();

    if(ret >= DWC_ERROR_FRIENDS_SHORTAGE){
        _CHANGE_STATE(_wifiFailedLoop, 0);
    }
    else if( ret < 0 ){        // エラー発生。
        _CHANGE_STATE(_wifiErrorLoop, 0);
    }		
    else if( ret == 1 ) {     // タイムアウト（相手から１０秒以上メッセージが届かない）
        _CHANGE_STATE(_wifiTimeoutLoop, 0);
    }
    else if(ret == 2){
        if(pNetHandle->bDisconnectError){
            _CHANGE_STATE(_wifiErrorLoop, 0);
        }
        else{
            _CHANGE_STATE(_wifiDisconnectLoop, 0);
        }
    }
}

//==============================================================================
/**
 * @brief   エラーコードコンバート
 * @param   ret   エラーコード
 * @retval  none
 */
//==============================================================================

static void _errcodeConvert(GFL_NETSTATE* pState,int ret)
{
    // エラー発生。	
    int errorcode;
    if( (ret == ERRORCODE_HEAP) || (ret == ERRORCODE_0)){
        errorcode = ret;
    }
    else {
        errorcode = -ret;
    }
    pNetHandle->errorCode = errorcode;
    NET_PRINT("エラーが発生しました。エラーコード(%d)\n", errorcode);
}

//==============================================================================
/**
 * @brief   マッチング相手を探し中
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================

static void _wifiMachingLoop(GFL_NETSTATE* pState)
{
    int ret = mydwc_stepmatch( 0 );

    if(ret >= DWC_ERROR_FRIENDS_SHORTAGE){
        _CHANGE_STATE(_wifiFailedLoop, 0);
    }
    else if( ret < 0 ){
        _errcodeConvert(pNetHandle, ret);
        _CHANGE_STATE(_wifiErrorLoop, 0);
    }
    else if ( ret == STEPMATCH_SUCCESS )  {
        NET_PRINT("対戦相手が見つかりました。\n");
        _CHANGE_STATE(_wifiApplicationConnect, 0);
    }
    else if ( ret == STEPMATCH_CANCEL ){
        NET_PRINT("キャンセルしました。\n");	
        _CHANGE_STATE(_wifiDisconnectLoop, 0);
    }
    else if( ret == STEPMATCH_FAIL){
        _CHANGE_STATE(_wifiFailedLoop, 0);
    }
    
}

//==============================================================================
/**
 * @brief   キャンセル中  親をやめる時や切り替える時に使用
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================

static void _wifiBattleCanceling(GFL_NETSTATE* pState)
{
    int ret2;
    int ret = mydwc_stepmatch( 1 );  // キャンセル中
				
    if( ret < 0 ){
        // エラー発生。
        _errcodeConvert(pNetHandle, ret);
        _CHANGE_STATE(_wifiErrorLoop, 0);
    }
    else{
        if(pNetHandle->wifiTargetNo == _WIFIMODE_RANDOM ){
            ret2 = mydwc_startmatch();
        }
        else{
            ret2 = mydwc_startgame( pNetHandle->wifiTargetNo );
        }
        if(ret2 != 0 ){
            GFL_NET_SystemReset();   // 今までの通信バッファをクリーンにする
            NET_PRINT(" %d番目の友達もしくはサーバにに接続\n", pNetHandle->wifiTargetNo);
            _CHANGE_STATE(_wifiMachingLoop, 0);
        }
        else{
            NET_PRINT(" 状態を変更できなかった もういっかい\n");
        }
    }
}

//==============================================================================
/**
 * @brief   WIFI指定接続を開始する
 * @param   pNetHandle   GFL_NETHANDLE
 * @param   target    つなぎにいく親機の番号
 * @retval  TRUE      成功
 * @retval  FALSE     失敗
 */
//==============================================================================
int GFL_NET_StateStartWifiPeerMatch( GFL_NETSTATE* pState, int target )
{
    GF_ASSERT(!(target < 0));

    if( !GFL_NET_SystemWifiApplicationStart(target) ){
        return FALSE;
    }
    pNetHandle->wifiTargetNo = target;
    _CHANGE_STATE(_wifiBattleCanceling,0);  // 今の状態を破棄
    return TRUE;
}

//==============================================================================
/**
 * @brief   WIFIランダムマッチを開始する
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  TRUE      成功
 * @retval  FALSE     失敗
 */
//==============================================================================
int GFL_NET_StateStartWifiRandomMatch(void )
{
    _pNetState->wifiTargetNo = _WIFIMODE_RANDOM;
    _CHANGE_STATE(_wifiBattleCanceling,0);  // 今の状態を破棄
    return TRUE;
}

//==============================================================================
/**
 * @brief   親としての通信処理開始
 * @param   pNetHandle  ハンドル
 * @param   heapID      HEAPID
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateConnectWifiParent(HEAPID heapID)
{
    _CHANGE_STATE(_stateNone, 0);
}



//==============================================================================
/**
 * @brief   マッチングが完了したかどうかを判定
 * @param   pNetHandle   GFL_NETHANDLE
 * @param   target: 負なら親、０以上ならつなぎにいく親機の番号
 * @retval  1. 完了　　0. 接続中   2. エラーやキャンセルで中断
 */
//==============================================================================
int GFL_NET_StateWifiIsMatched(void)
{
	if( _pNetState->state == _wifiMachingLoop ) return 0;
	if( _pNetState->state == _wifiApplicationConnect ) return 1;
    if( _pNetState->state == _wifiTimeoutLoop ) return 3;
    if( _pNetState->state == _wifiDisconnectLoop ) return 4;
    if( _pNetState->state == _wifiFailedLoop ) return 5;
    return 2;
}

//==============================================================================
/**
 * @brief   親機として待機中
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================

static void _wifiLoginLoop(GFL_NETSTATE* pState)
{
    if( mydwc_startgame( _WIFIMODE_PARENT ) ){   //最初はVCT待ち状態になるので、親状態起動
        pNetHandle->bWifiDisconnect = FALSE;
        GFL_NET_SystemReset();
        NET_PRINT("VCT参加者を募集します\n");
        _CHANGE_STATE(_wifiMachingLoop, 0);
    }
}

//==============================================================================
/**
 * @brief   マッチングを終了する
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================


static void _stateWifiMatchEnd(GFL_NETSTATE* pState)
{
    int ret;

    GFL_NET_SystemSetWifiConnect(FALSE);
    if(mydwc_disconnect( pNetHandle->disconnectType )){
        mydwc_returnLobby();
        _CHANGE_STATE(_wifiLoginLoop, 0);
        return;
    }
    ret = mydwc_step();
    if( ret < 0 ){
        NET_PRINT(" エラー発生。");
        _CHANGE_STATE(_wifiErrorLoop, 0);
    }
}

//==============================================================================
/**
 * @brief   WIFIでマッチングを切る場合の処理
 * @param   bAuto  自分から強制切断するときはFALSE 接続状況にあわせる場合TRUE
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateWifiMatchEnd(BOOL bAuto, GFL_NETSTATE* pState)
{
    if(pNetHandle == NULL){  // すでに終了している
        return;
    }
    if(bAuto){
        if(GFL_NET_SystemGetCurrentID() == GFL_NET_PARENT_NETID){
            pNetHandle->disconnectType = 0;
        }
        else{
            pNetHandle->disconnectType = 1;
        }
    }
    else{
        pNetHandle->disconnectType = 0;
    }
    _CHANGE_STATE(_stateWifiMatchEnd, 0);
}

//==============================================================================
/**
 * @brief   WIFI終了コマンド   子機が親機にやめるように送信 ぶっつりきる CS_WIFI_EXIT
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateRecvWifiExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETSTATE* pState)
{
    GFL_NET_StateWifiMatchEnd(TRUE, pNetHandle);
    pNetHandle->bWifiDisconnect = TRUE;
}


//==============================================================================
/**
 * @brief   WIFIでマッチングを切る場合の処理 切断コマンドを送信
 * @param   none
 * @retval  none
 */
//==============================================================================
void GFL_NET_StateSendWifiMatchEnd(GFL_NETSTATE* pState)
{
    u8 id = GFL_NET_SystemGetCurrentID();

    GFL_NET_SendData(pNetHandle, GFL_NET_CMD_WIFI_EXIT,&id);
}

//==============================================================================
/**
 * @brief   Wifi切断コマンドを受け取った場合TRUE
 * @param   none
 * @retval  WifiLogoutに移行した場合TRUE
 */
//==============================================================================

BOOL GFL_NET_StateIsWifiDisconnect(GFL_NETSTATE* pState)
{
    return pNetHandle->bWifiDisconnect;
}

//==============================================================================
/**
 * @brief   WifiLoginに移行したかどうかの確認
 * @param   none
 * @retval  WifiLoginに移行した場合TRUE
 */
//==============================================================================

BOOL GFL_NET_StateIsWifiLoginState(GFL_NETSTATE* pState)
{
    u32 stateAddr = (u32)pNetHandle->state;

    if(stateAddr == (u32)_wifiLoginLoop){
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   WifiLoginMatchに移行したかどうかの確認
 * @param   none
 * @retval  WifiLoginMatchに移行した場合TRUE
 */
//==============================================================================

BOOL GFL_NET_StateIsWifiLoginMatchState(GFL_NETSTATE* pState)
{
    u32 stateAddr = (u32)pNetHandle->state;

    if((stateAddr == (u32)_wifiMachingLoop) || (stateAddr == (u32)_wifiLoginLoop)){
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   WifiErrorの場合その番号を返す  エラーになってるかどうかを確認してから引き出すこと
 * @param   none
 * @retval  Error番号
 */
//==============================================================================

int GFL_NET_StateGetWifiErrorNo(GFL_NETSTATE* pState)
{
    return pNetHandle->errorCode;
}

//==============================================================================
/**
 * @brief   WIFIでログアウトを行う場合の処
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateWifiLogout(GFL_NETSTATE* pState)
{
    if(pNetHandle==NULL){  // すでに終了している
        return;
    }
    GFL_UI_SoftResetEnable(GFL_UI_SOFTRESET_WIFI);
    _CHANGE_STATE(_stateConnectEnd, _EXIT_SENDING_TIME);
}


//==============================================================================
/**
 * @brief   WifiErrorかどうか
 * @param   none
 * @retval  Error
 */
//==============================================================================

BOOL GFL_NET_StateIsWifiError(GFL_NETSTATE* pState)
{
    if(pNetHandle){
        u32 stateAddr = (u32)pNetHandle->state;
        if(stateAddr == (u32)_wifiErrorLoop){
            return TRUE;
        }
        if((stateAddr == (u32)_wifiTimeoutLoop) && pNetHandle->bDisconnectError){
            return TRUE;
        }
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   子機待機状態  親機を選択中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _wifiApplicationConnecting(GFL_NETSTATE* pState)
{
    // 接続中
    int ret = mydwc_connect();

    if( ret < 0 ){
        _errcodeConvert(pNetHandle, ret);
        _CHANGE_STATE(_wifiErrorLoop, 0);
    }
    else if ( ret > 0 ) 
    {
        NET_PRINT("WiFiコネクションに接続しました。\n	");
        _CHANGE_STATE(_wifiLoginLoop, 0);
    }
}


//==============================================================================
/**
 * @brief   wifiで無線ルーターに接続して、ログイン開始する
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _wifiLoginInit(GFL_NETSTATE* pState)
{
    if(GFL_NET_SystemWiFiModeInit(_PACKETSIZE_DEFAULT,
                                  pNetHandle->netHeapID, pNetHandle->wifiHeapID)){
        _CHANGE_STATE(_wifiApplicationConnecting, 0);
    }
}

//==============================================================================
/**
 * @brief   wifiマッチング状態へログインする
 * @param   pNetHandle  GFL_NETHANDLE
 * @param   netHeapID   netLibで確保使用するID
 * @param   wifiHeapID  NitroWifi&NitroDWC&VCTで確保するメモリ
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateWifiEnterLogin(HEAPID netHeapID, HEAPID wifiHeapID)
{
    if(GFL_NET_SystemIsInitialize()){
        return;      // つながっている場合今は除外する
    }
    GFL_UI_SoftResetDisable(GFL_UI_SOFTRESET_WIFI);
    _commStateInitialize(netHeapID);
    _CHANGE_STATE(_wifiLoginInit, 0);
}
#endif  //GFL_NET_WIFI

//--------------------------errfunc------------------------------------------------

//==============================================================================
/**
 * @brief   自動的にエラー検出を行うかどうか
 *          (TRUEの場合エラーをみつけるとブルースクリーン＋再起動になる)
 * @param   bAuto  TRUEで検査開始 FALSEでエラー処理を行わない
 * @retval  none
 */
//==============================================================================

void GFL_NET_STATE_SetAutoErrorCheck(BOOL bAuto)
{
    if(_pNetState){
        _pNetState->bErrorAuto = bAuto;
    }
}

//==============================================================================
/**
 * @brief   子機がいない場合にエラーにするかどうかを設定する
 * @param   bFlg    切断=エラーにする
 * @param   bAuto  TRUEで検査開始
 * @retval  none
 */
//==============================================================================

void GFL_NET_STATE_SetNoChildErrorCheck(BOOL bFlg)
{
    if(_pNetState){
        _pNetState->bDisconnectError = bFlg;
    }
    GFL_NET_WLSetDisconnectOtherError(bFlg);
}

//==============================================================================
/**
 * @brief   エラー状態に入る
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _setErrorReset(GFL_NETSTATE* pState,u8 type)
{
    if(pState){
        pState->ResetStateType = type;
    }
}

//==============================================================================
/**
 * @brief   リセットエラー状態になったかどうか確認する
 * @param   none
 * @retval  リセットエラー状態ならTRUE
 */
//==============================================================================

static u8 _isResetError(GFL_NETSTATE* pState)
{
    if(pState){
        return pState->ResetStateType;
    }
    return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   エラーを感知して 通信エラー用ウインドウを出す
 * @param   heapID    メモリー確保するHEAPID
 * @param   bgl       GF_BGL_INI
 * @retval  none
 */
//--------------------------------------------------------------

static void _errorDispCheck(GFL_NETSTATE* pState)
{
    if(_getErrorCheck(pState)){
#if GFL_NET_WIFI
        if(GFL_NET_SystemIsError() || GFL_NET_StateGetWifiErrorNo(pState) || (pState->stateError!=0)){
#else
        if(GFL_NET_SystemIsError() || (pState->stateError!=0)){
#endif
            if(!_isResetError(pState)){   // リセットエラー状態で無い場合
                //Snd_Stop();
                //SaveData_DivSave_Cancel(pNetHandle->pSaveData); // セーブしてたら止める
                //sys.tp_auto_samp = 1;  // サンプリングも止める

                _setErrorReset(pState,GFL_NET_ERROR_RESET_SAVEPOINT);  // エラーリセット状態になる
                GFI_NET_FatalErrorFunc(GFL_NET_ERROR_RESET_SAVEPOINT);
                while(1){            }
            }
        }
    }
}


#define DEBUG_OHNO 0
#if DEBUG_OHNO



//--------------------------------------------------------------
/**
 * @brief   fatalエラー用関数  この関数内から抜けられない
 * @param   no   エラー番号
 * @retval  none
 */
//--------------------------------------------------------------

void CommFatalErrorFunc(int no)
{
    int i=0;
    
    ComErrorWarningResetCall(HEAPID_BASE_SYSTEM,COMM_ERRORTYPE_POWEROFF, no);
    while(1){
        i++;
    }
}

void CommFatalErrorFunc_NoNumber( void )
{
    int i=0;
    
    ComErrorWarningResetCall( HEAPID_BASE_SYSTEM, 4, 0 );
    while(1){
        i++;
    }
}

//--------------------------------------------------------------
/**
 * @brief   エラーを感知して 通信エラー用ウインドウを出す
 * @param   heapID    メモリー確保するHEAPID
 * @param   bgl       GF_BGL_INI
 * @retval  none
 */
//--------------------------------------------------------------

void CommErrorCheck(int heapID, GF_BGL_INI* bgl)
{
// この関数では処理しないことになりました
}


//--------------------------------------------------------------
/**
 * @brief   リセットできる状態かどうか
 * @param   none
 * @retval  TRUE ならリセット
 */
//--------------------------------------------------------------

BOOL CommStateIsResetEnd(void)
{
    if(CommMPIsConnectStalth() || !CommStateIsInitialize()){ // 通信終了
        return TRUE;
    }
    if(!CommMPIsConnect()){
        return TRUE;
    }
    return FALSE;
}




//==============================================================================
/**
 * @brief   エラーにする場合この関数を呼ぶとエラーになります
 * @param   エラー種類
 * @retval  受け付けた場合TRUE
 */
//==============================================================================

BOOL CommStateSetError(GFL_NETSTATE* pState,int no)
{

    if(pNetHandle){
        pNetHandle->stateError = no;
        CommSystemShutdown();
        return TRUE;
    }
    return FALSE;
}

#endif  //DEBUG_OHNO

