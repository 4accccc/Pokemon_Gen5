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

#include <nitro.h>
#include <nnsys.h>
#include <dwc.h>
#include "gf_macro.h"
#include "gf_standard.h"
#include "assert.h"
#include "heapsys.h"
#include "strbuf.h"
#include "net.h"
#include "ui.h"

#include "device/wh_config.h"

#include "net_def.h"
#include "device/net_wireless.h"
#include "device/dwc_rap.h"
#include "net_system.h"
#include "net_command.h"
#include "net_state.h"

#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "tool/net_tool.h"

//==============================================================================
// ワーク
//==============================================================================



///< 親機のみで必要なデータ
struct _NET_PARENTSYS_t{
    u8 negoCheck[GFL_NET_MACHINE_MAX];     ///< 各子機のネゴシエーション状態
    u8 dsmpChangeBuff[GFL_NET_MACHINE_MAX];   ///< 通信相手の通信モード変更バッファ
};

//==============================================================================
// 定義
//==============================================================================

#define _PACKETSIZE_DEFAULT         (512)  // パケットサイズの最大サイズ

#define _START_TIME (50)          /// 開始時間
#define _CHILD_P_SEARCH_TIME (32) /// 子機として親を探す時間
#define _EXIT_SENDING_TIME (5)
#define _SEND_NAME_TIME (10)

#define _WIFIMODE_PARENT (-1)  // 親としてwifi起動

//==============================================================================
// static宣言
//==============================================================================

// ステートの初期化

static void _changeStateDebug(GFL_NETHANDLE* pHandle, PTRStateFunc state, int time, int line);  // ステートを変更する
static void _changeState(GFL_NETHANDLE* pHandle, PTRStateFunc state, int time);  // ステートを変更する
static void _changeoverChildSearching(GFL_NETHANDLE* pNetHandle);

#ifdef GFL_NET_DEBUG
#if 1
#define   _CHANGE_STATE(state, time)  _changeStateDebug(pNetHandle ,state, time, __LINE__)
#else
#define   _CHANGE_STATE(state, time)  _changeState(pNetHandle ,state, time)
#endif
#else  //GFL_NET_DEBUG
#define   _CHANGE_STATE(state, time)  _changeState(pNetHandle ,state, time)
#endif //GFL_NET_DEBUG


// その他一般的なステート
static void _stateEnd(GFL_NETHANDLE* pNetHandle);             // 終了処理
static void _stateConnectEnd(GFL_NETHANDLE* pNetHandle);      // 切断処理開始
static void _stateNone(GFL_NETHANDLE* pNetHandle);            // 何もしない


//==============================================================================
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================

static void _changeState(GFL_NETHANDLE* pHandle,PTRStateFunc state, int time)
{
    pHandle->state = state;
    pHandle->timer = time;
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
static void _changeStateDebug(GFL_NETHANDLE* pHandle,PTRStateFunc state, int time, int line)
{
    NET_PRINT("net_state: %d\n",line);
    _changeState(pHandle, state, time);
}
#endif

//==============================================================================
/**
 * @brief   通信管理ステートの初期化処理
 * @param   MYSTATUS* pMyStatus
 * @retval  none
 */
//==============================================================================

static void _commStateInitialize(GFL_NETHANDLE* pNetHandle,int serviceNo)
{
    void* pWork;

    if(pNetHandle!=NULL){   // すでに動作中の場合必要ない
        return;
    }
//    CommVRAMDInitialize();
    // 初期化

    pNetHandle->timer = _START_TIME;
    pNetHandle->bFirstParent = TRUE;  // 親の初めての起動の場合TRUE
    pNetHandle->limitNum = 2;         // 一人は最低でも接続可能
    pNetHandle->negotiation = _NEGOTIATION_CHECK;
    pNetHandle->serviceNo = serviceNo;

    //CommRandSeedInitialize(&pNetHandle->sRand);  //@@OO乱数初期化を入れる
    GFL_NET_CommandInitialize(NULL, 0, NULL);
/*
    if((serviceNo != COMM_MODE_UNION) && (serviceNo != COMM_MODE_PARTY) &&
       (serviceNo != COMM_MODE_MYSTERY)){
        WirelessIconEasy();  // アイコン初期化
    }
*/
}


//==============================================================================
/**
 * @brief   通信管理ステートの終了処理
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _stateFinalize(GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle==NULL){  // すでに終了している
        return;
    }
    GFL_NET_CommandFinalize();
//    WirelessIconEasyEnd();   //@@OO 後で入れる予定
    GFL_NET_WLVRAMDFinalize();
    _GFL_NET_SetNETWL(NULL);
    pNetHandle->state = NULL;
    GFL_UI_SleepEnable(GFL_UI_SLEEP_NET);  // スリープ許可
}

//==============================================================================
/**
 * @brief   動いているかどうか
 * @param   none
 * @retval  動いている場合TRUE
 */
//==============================================================================

static BOOL _stateIsMove(GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle->state){
        return TRUE;
    }
    return FALSE;
}



static void _handleDelete(GFL_NETHANDLE* pNetHandle)
{
    GFI_NET_DeleteNetHandle(pNetHandle);
}


//==============================================================================
/**
 * @brief   イクニューモン初期化後通信起動
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _deviceInitialize(GFL_NETHANDLE* pNetHandle)
{
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();
    GFL_NETWL* pWL;
    
    if(!GFL_NET_WLIsVRAMDInitialize()){
        return;  //
    }

    NET_PRINT("%x - %x %x\n",(u32)pNetIni,(u32)pNetIni->beaconGetFunc,(u32)pNetIni->beaconGetSizeFunc);
    
    pWL = GFL_NET_WLGetHandle(pNetIni->netHeapID, pNetIni->gsid, pNetIni->maxConnectNum);
    _GFL_NET_SetNETWL(pWL);

    GFL_NET_WLInitialize(pNetIni->netHeapID, pNetIni->beaconGetFunc, pNetIni->beaconGetSizeFunc,
                         pNetIni->beaconCompFunc, pNetIni->bNetwork);

    GFL_NET_SystemReset();         // 今までの通信バッファをクリーンにする

    NET_PRINT("再起動    -- \n");
    _CHANGE_STATE(_handleDelete, 0);
}


//==============================================================================
/**
 * @brief   通信デバイスを初期化する
 * @param   none
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateDeviceInitialize(GFL_NETHANDLE* pNetHandle)
{
    GFL_UI_SleepDisable(GFL_UI_SLEEP_NET);  // スリープ禁止

    GFL_NET_WLVRAMDInitialize();
    GFL_NET_CommandInitialize(NULL, 0, NULL);

    _commStateInitialize(pNetHandle, 0);
    _CHANGE_STATE(_deviceInitialize, 0);
}

//==============================================================================
/**
 * @brief   子機待機状態  親機に情報を送信
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _childSendNego(GFL_NETHANDLE* pNetHandle)
{

    if(GFL_NET_SystemIsError()){
        //NET_PRINT("エラーの場合戻る\n");
     //   _CHANGE_STATE(_battleChildReset, 0);
    }
  //  if(CommIsConnect(CommGetCurrentID()) && ( COMM_PARENT_ID != CommGetCurrentID())){
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

static void _parentFindCallback(GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle){
        pNetHandle->timer+=60;  //子機時間を延長
    }
}

//==============================================================================
/**
 * @brief   子機待機状態  親機に許可もらい中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _childConnecting(GFL_NETHANDLE* pNetHandle)
{
    GFL_NET_WLParentBconCheck();

    if(GFL_NET_WLChildMacAddressConnect(pNetHandle->aMacAddress,_parentFindCallback,pNetHandle)){
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

void GFL_NET_StateConnectMacAddress(GFL_NETHANDLE* pNetHandle)
{
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

    GFL_NET_SystemChildModeInit(TRUE, 512);

    if(pNetIni->bMPMode){
        GFL_NET_SystemSetTransmissonTypeMP();
    }
    
    _CHANGE_STATE(_childConnecting, 0);
}

//==============================================================================
/**
 * @brief   子機待機状態  ビーコン収集
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _childScanning(GFL_NETHANDLE* pNetHandle)
{
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();
    GFL_NET_WLParentBconCheck();

    if(pNetIni->bMPMode){
        GFL_NET_SystemSetTransmissonTypeMP();
    }
}

//==============================================================================
/**
 * @brief   子機開始  ビーコンの収集に入る
 * @param   connectIndex 接続する親機のIndex
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateBeaconScan(GFL_NETHANDLE* pNetHandle)
{
    GFL_NET_SystemChildModeInit(TRUE,512);

    _CHANGE_STATE(_childScanning, 0);
}



//==============================================================================
/**
 * @brief   通信管理ステートの処理
 * @param
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateMainProc(GFL_NETHANDLE* pHandle)
{
    if(pHandle){
        PTRStateFunc state = GFL_NET_GetStateFunc(pHandle);
        if(state != NULL){
            state(pHandle);
        }
    }
}

//==============================================================================
/**
 * @brief   親機として待機中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _parentWait(GFL_NETHANDLE* pHandle)
{
}

//==============================================================================
/**
 * @brief   親機として初期化を行う
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _parentInit(GFL_NETHANDLE* pNetHandle)
{
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

    if(!GFL_NET_WLIsVRAMDInitialize()){
        return;
    }

    if(GFL_NET_SystemParentModeInit(TRUE, _PACKETSIZE_DEFAULT,TRUE)){
        if(pNetIni->bMPMode){
            GFL_NET_SystemSetTransmissonTypeMP();
        }
        else{
            GFL_NET_SystemSetTransmissonTypeDS();
        }
        pNetHandle->negotiation = _NEGOTIATION_OK;  // 自分は認証完了
        pNetHandle->creatureNo = 0;
        _CHANGE_STATE(_parentWait, 0);
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

void GFL_NET_StateConnectParent(GFL_NETHANDLE* pNetHandle,HEAPID heapID)
{
    pNetHandle->pParent = GFL_HEAP_AllocMemory(heapID, sizeof(NET_PARENTSYS));
    GFL_STD_MemClear(pNetHandle->pParent, sizeof(NET_PARENTSYS));

    _CHANGE_STATE(_parentInit, 0);
}
//==============================================================================
/**
 * @brief   子機再スタート
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _changeoverChildRestart(GFL_NETHANDLE* pNetHandle)
{
    u32 rand;
    
    if(!GFL_NET_WLIsStateIdle()){  // 終了処理がきちんと終わっていることを確認
        return;
    }
    // 今度はビーコンを残したまま

    if(GFL_NET_SystemChildModeInitAndConnect(512,_parentFindCallback,pNetHandle)){
        rand = MATH_Rand32(&pNetHandle->sRand, (_CHILD_P_SEARCH_TIME/2))+(_CHILD_P_SEARCH_TIME/2);
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


static void _changeoverParentRestart(GFL_NETHANDLE* pNetHandle)
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

static BOOL _getErrorCheck(GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle->stateError!=0){
        return TRUE;
    }
    return pNetHandle->bErrorAuto;
}


//==============================================================================
/**
 * @brief   親として接続中
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _changeoverParentConnect(GFL_NETHANDLE* pNetHandle)
{

    if(!_getErrorCheck(pNetHandle)){
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


static void _changeoverParentWait(GFL_NETHANDLE* pNetHandle)
{
    if(GFL_NET_SystemIsChildsConnecting()){   // 自分以外がつながったら親機固定

        NET_PRINT("親機 -- つながり\n");
//        pNetHandle->bFirstParent = TRUE;  // 親機として繋がったのでフラグを戻しておく
        //WirelessIconEasy();  //@@OO
        _CHANGE_STATE(_changeoverParentConnect, 0);
        return;
    }

    if(GFL_NET_WLIsParentBeaconSent()){  // ビーコンを送り終わったらしばらく待つ
        if(pNetHandle->timer!=0){
            pNetHandle->timer--;
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

static void _changeoverParentInit(GFL_NETHANDLE* pNetHandle)
{
    if(!GFL_NET_WLIsStateIdle()){  // 終了処理がきちんと終わっていることを確認
        return;
    }
    // 親機になってみる
    if(GFL_NET_SystemParentModeInit(FALSE,  _PACKETSIZE_DEFAULT,TRUE))  {
        pNetHandle->bFirstParent = FALSE;
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

static void _changeoverChildSearching(GFL_NETHANDLE* pNetHandle)
{
    int realParent;

    GFL_NET_WLParentBconCheck();  // bconの検査

    if(GFL_NET_SystemGetCurrentID()!=COMM_PARENT_ID){  // 子機として接続が完了した
        NET_PRINT("子機になった\n");
        _CHANGE_STATE(_stateNone, 0);
        return;
    }

    if(pNetHandle->timer != 0){
        NET_PRINT("time %d\n",pNetHandle->timer);
        pNetHandle->timer--;
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

void GFL_NET_StateChangeoverConnect(GFL_NETHANDLE* pNetHandle,HEAPID heapID)
{


//    GFL_NET_SystemChildModeInit(TRUE,512);
    GFL_NET_SystemChildModeInitAndConnect(512,_parentFindCallback,pNetHandle);
    
//    if(GFL_NET_WLChildMacAddressConnect(pNetHandle->aMacAddress)){


    _CHANGE_STATE(_changeoverChildSearching, 30);


}

//==============================================================================
/**
 * @brief   子機の終了を待ち、親機が終了する
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _stateEndParentWait(GFL_NETHANDLE* pNetHandle)
{
    if(!GFL_NET_WLIsChildsConnecting()){
        _stateFinalize(pNetHandle);
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
    if(!pNetHandle->pParent){
        return;
    }
    NET_PRINT("EXITコマンド受信\n");

    GFL_NET_SendData(pNetHandle, GFL_NET_CMD_EXIT, NULL);

    _CHANGE_STATE(_stateEndParentWait, 0);


#if 0
    if(CommGetCurrentID() == COMM_PARENT_ID){
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
    if(GFL_NET_SystemGetCurrentID() != COMM_PARENT_ID){
        GFL_NET_Disconnect();
    }
}

//==============================================================================
/**
 * @brief   ネゴシエーション用コールバック CS_COMM_NEGOTIATION
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateRecvNegotiation(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    // 親機が受け取ってフラグを立てる
    u8 retCmd[2];
    const u8 *pGet = (const u8*)pData;
    int i;

    if(!pNetHandle->pParent){
        return;
    }
    retCmd[1] = pGet[0];
    retCmd[0] = 0;

    for(i = 1 ; i < GFL_NET_MACHINE_MAX;i++){
        if(pNetHandle->pParent->negoCheck[i] == FALSE){
            pNetHandle->pParent->negoCheck[i] = TRUE;
            retCmd[0] = i;         //このiがIDになる
            break;
        }
    }
    NET_PRINT("------NegoRet を送信 %d\n",retCmd[0]);
    GFL_NET_SendData(pNetHandle, GFL_NET_CMD_NEGOTIATION_RETURN, retCmd);
    pNetHandle->negotiationID[i] = TRUE;

}

//==============================================================================
/**
 * @brief   ネゴシエーション用コールバック CS_COMM_NEGOTIATION_RETURN
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateRecvNegotiationReturn(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pMsg = pData;

    NET_PRINT("接続認証\n");
    if(pNetHandle->creatureNo == pMsg[1]){   // 親機から接続認証が来た
        NET_PRINT("接続認証 OK\n");
        pNetHandle->negotiation = _NEGOTIATION_OK;
        pNetHandle->creatureNo = pMsg[0];
    }
    GF_ASSERT(pMsg[0] < GFL_NET_MACHINE_MAX);
    pNetHandle->negotiationID[pMsg[0]]=TRUE;
}

//==============================================================================
/**
 * @brief  終了処理中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _stateEnd(GFL_NETHANDLE* pNetHandle)
{
    if(GFL_NET_SystemIsInitialize()){
        return;
    }
    _stateFinalize(pNetHandle);
}

//==============================================================================
/**
 * @brief  終了処理開始
 * @param   none
 * @retval  none
 */
//==============================================================================
static void _stateConnectEnd(GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle->timer != 0){
        pNetHandle->timer--;
    }
    if(!GFL_NET_WLSwitchParentChild()){
        return;
    }
    if(pNetHandle->timer != 0){
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

void GFL_NET_StateExit(GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle->bDisconnectState){
        return;
    }
    pNetHandle->bDisconnectState = TRUE;
    _CHANGE_STATE(_stateConnectEnd, _EXIT_SENDING_TIME);
}

//==============================================================================
/**
 * @brief   何もしないステート
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _stateNone(GFL_NETHANDLE* pNetHandle)
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
 * @brief   DSMPモード変更のMain関数
 * @param   pNet  通信ハンドル
 * @return  noen
 */
//==============================================================================

void GFL_NET_StateTransmissonMain(GFL_NETHANDLE* pNet)
{
    BOOL bCatch=FALSE;

    switch(pNet->dsmpChange){
      case _TRANS_LOAD:
        bCatch = GFL_NET_SendData(pNet,GFL_NET_CMD_DSMP_CHANGE_REQ,&pNet->dsmpChangeType);
        if(bCatch){
            pNet->dsmpChange = _TRANS_LOAD_END;
        }
        break;
      case _TRANS_SEND:
        if(GFL_NET_SendData(pNet,GFL_NET_CMD_DSMP_CHANGE_END,&pNet->dsmpChangeType)){
            GFL_NET_SystemSetTransmissonType(pNet->dsmpChangeType);  // 切り替える  親機はここで切り替えない
            pNet->dsmpChange = _TRANS_NONE;
        }
        break;
    }
}

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

    if(!pNetHandle->pParent){
        return;
    }
    NET_PRINT("CommRecvDSMPChange 受信\n");
    pNetHandle->dsmpChange = _TRANS_LOAD;
    pNetHandle->dsmpChangeType = pBuff[0];
}

//==============================================================================
/**
 * @brief   DS通信MP通信の切り替え
 * @param   none
 * @retval  残り数
 */
//==============================================================================

void GFL_NET_StateRecvDSMPChangeReq(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff = pData;
    int i;

    if(pNetHandle->pParent){
        return;
    }
    pNetHandle->dsmpChangeType = pBuff[0];
    pNetHandle->dsmpChange = _TRANS_SEND;
    NET_PRINT("CommRecvDSMPChangeReq 受信\n");
}

//==============================================================================
/**
 * @brief   DS通信MP通信の切り替え 終了処理 GFL_NET_CMD_DSMP_CHANGE_END
 * @param   none
 * @retval  残り数
 */
//==============================================================================

void GFL_NET_StateRecvDSMPChangeEnd(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pBuff = pData;
    int i;

    if(!pNetHandle->pParent){
        return;
    }
    NET_PRINT("CommRecvDSMPChangeEND 受信\n");

    if(pNetHandle->dsmpChange == _TRANS_LOAD_END){
        GFL_NET_SystemSetTransmissonType(pBuff[0]);  // 切り替える
        pNetHandle->dsmpChange = _TRANS_NONE;
    }
}

//--------------------------wifi------------------------------------------------

//==============================================================================
/**
 * @brief   エラーが発生した状態
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================

static void _wifiErrorLoop(GFL_NETHANDLE* pNetHandle)
{
}
static void _wifiDisconnectLoop(GFL_NETHANDLE* pNetHandle)
{
}
static void _wifiFailedLoop(GFL_NETHANDLE* pNetHandle)
{
}

//==============================================================================
/**
 * @brief   タイムアウトエラーが発生した状態
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================

static void _wifiTimeoutLoop(GFL_NETHANDLE* pNetHandle)
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

static void _wifiApplicationConnect(GFL_NETHANDLE* pNetHandle)
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

static void _errcodeConvert(GFL_NETHANDLE* pNetHandle,int ret)
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

static void _wifiMachingLoop(GFL_NETHANDLE* pNetHandle)
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

static void _wifiBattleCanceling(GFL_NETHANDLE* pNetHandle)
{

    int ret = mydwc_stepmatch( 1 );  // キャンセル中
				
    if( ret < 0 ){
        // エラー発生。
        _errcodeConvert(pNetHandle, ret);
        _CHANGE_STATE(_wifiErrorLoop, 0);
    }
    else{
        if( mydwc_startgame( pNetHandle->wifiTargetNo ) ){
            GFL_NET_SystemReset();   // 今までの通信バッファをクリーンにする
            if( pNetHandle->wifiTargetNo < 0 ){
                OS_TPrintf("ゲーム参加者を募集します。\n");
            } else {
                OS_TPrintf(" %d番目の友達に接続します。\n", pNetHandle->wifiTargetNo);
            }
            _CHANGE_STATE(_wifiMachingLoop, 0);
        }
        else{
            NET_PRINT(" 状態を変更できなかった もういっかい\n");
        }
    }
}

//==============================================================================
/**
 * @brief   アプリケーションを開始する為に相手につなぐ
 * @param   pNetHandle   GFL_NETHANDLE
 * @param   target    負なら親、０以上ならつなぎにいく親機の番号
 * @retval  TRUE      成功
 * @retval  FALSE     失敗
 */
//==============================================================================
int GFL_NET_StateWifiApplicationStart( GFL_NETHANDLE* pNetHandle, int target )
{
    if( !GFL_NET_SystemWifiApplicationStart(target) ){
        return FALSE;
    }
    pNetHandle->wifiTargetNo = target;
    _CHANGE_STATE(_wifiBattleCanceling,0);  // 今の状態を破棄
    return TRUE;
}

//==============================================================================
/**
 * @brief   マッチングが完了したかどうかを判定
 * @param   pNetHandle   GFL_NETHANDLE
 * @param   target: 負なら親、０以上ならつなぎにいく親機の番号
 * @retval  1. 完了　　0. 接続中   2. エラーやキャンセルで中断
 */
//==============================================================================
int GFL_NET_StateWifiIsMatched(GFL_NETHANDLE* pNetHandle)
{
	if( pNetHandle->state == _wifiMachingLoop ) return 0;
	if( pNetHandle->state == _wifiApplicationConnect ) return 1;
    if( pNetHandle->state == _wifiTimeoutLoop ) return 3;
    if( pNetHandle->state == _wifiDisconnectLoop ) return 4;
    if( pNetHandle->state == _wifiFailedLoop ) return 5;
    return 2;
}

//==============================================================================
/**
 * @brief   親機として待機中
 * @param   pNetHandle   GFL_NETHANDLE
 * @retval  none
 */
//==============================================================================

static void _wifiLoginLoop(GFL_NETHANDLE* pNetHandle)
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


static void _stateWifiMatchEnd(GFL_NETHANDLE* pNetHandle)
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

void GFL_NET_StateWifiMatchEnd(BOOL bAuto, GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle == NULL){  // すでに終了している
        return;
    }
    if(bAuto){
        if(GFL_NET_SystemGetCurrentID() == COMM_PARENT_ID){
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

void GFL_NET_StateRecvWifiExit(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
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

void GFL_NET_StateSendWifiMatchEnd(GFL_NETHANDLE* pNetHandle)
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

BOOL GFL_NET_StateIsWifiDisconnect(GFL_NETHANDLE* pNetHandle)
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

BOOL GFL_NET_StateIsWifiLoginState(GFL_NETHANDLE* pNetHandle)
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

BOOL GFL_NET_StateIsWifiLoginMatchState(GFL_NETHANDLE* pNetHandle)
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

int GFL_NET_StateGetWifiErrorNo(GFL_NETHANDLE* pNetHandle)
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

void GFL_NET_StateWifiLogout(GFL_NETHANDLE* pNetHandle)
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

BOOL GFL_NET_StateIsWifiError(GFL_NETHANDLE* pNetHandle)
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

static void _wifiApplicationConnecting(GFL_NETHANDLE* pNetHandle)
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

static void _wifiLoginInit(GFL_NETHANDLE* pNetHandle)
{
    if(GFL_NET_SystemWiFiModeInit(_PACKETSIZE_DEFAULT,
                                  pNetHandle->netHeapID, pNetHandle->wifiHeapID,
                                  pNetHandle->pWiFiList)){
        _CHANGE_STATE(_wifiApplicationConnecting, 0);
    }
}

//==============================================================================
/**
 * @brief   wifiマッチング状態へログインする
 * @param   pNetHandle  GFL_NETHANDLE
 * @param   pWifiList   GFL_WIFI_FRIENDLIST
 * @param   baseHeapID  元になるheapid
 * @param   netHeapID   netLibで確保使用するID
 * @param   wifiHeapID  NitroWifi&NitroDWC&VCTで確保するメモリ
 * @retval  none
 */
//==============================================================================

void GFL_NET_StateWifiEnterLogin(GFL_NETHANDLE* pNetHandle, GFL_WIFI_FRIENDLIST* pWiFiList, HEAPID baseHeapID, HEAPID netHeapID, HEAPID wifiHeapID)
{
    if(GFL_NET_SystemIsInitialize()){
        return;      // つながっている場合今は除外する
    }
    GFL_UI_SoftResetDisable(GFL_UI_SOFTRESET_WIFI);

    _commStateInitialize(pNetHandle, 0);

    pNetHandle->baseHeapID = baseHeapID;
    pNetHandle->netHeapID = netHeapID;
    pNetHandle->wifiHeapID = wifiHeapID;
    pNetHandle->pWiFiList = pWiFiList;

    _CHANGE_STATE(_wifiLoginInit, 0);
}

//--------------------------errfunc------------------------------------------------
#define DEBUG_OHNO 0
#if DEBUG_OHNO

//==============================================================================
/**
 * @brief   ここから先エラーの検査を通信が処理するかどうかを設定
 * @param   bFlg    切断=エラーにする
 * @param   bAuto  TRUEで検査開始
 * @retval  none
 */
//==============================================================================

void CommStateSetErrorCheck(GFL_NETHANDLE* pNetHandle,BOOL bFlg,BOOL bAuto)
{
    if(pNetHandle){
        pNetHandle->bDisconnectError = bFlg;
        pNetHandle->bErrorAuto = bAuto;
        OHNO_PRINT("CommStateSetErrorCheck %d %d\n",pNetHandle->bDisconnectError,pNetHandle->bErrorAuto);
    }
    CommMPSetNoChildError(bFlg);  // 子機がいなくなったら再検索するためにERR扱いにする
    CommMPSetDisconnectOtherError(bFlg);
}


//==============================================================================
/**
 * @brief   ここから先エラーの検査を通信が処理するかどうかを設定
 * @param   bFlg  TRUEで検査開始
 * @retval  none
 */
//==============================================================================

BOOL CommStateGetErrorCheck(GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle){
        if(pNetHandle->stateError!=0){
            return TRUE;
        }
        return pNetHandle->bErrorAuto;
    }
    return FALSE;
//    CommMPSetDisconnectOtherError(bFlg);
}

//==============================================================================
/**
 * @brief   エラー状態に入る
 * @param   none
 * @retval  none
 */
//==============================================================================

void CommSetErrorReset(GFL_NETHANDLE* pNetHandle,u8 type)
{
    if(pNetHandle){
        pNetHandle->ResetStateType = type;
    }
}

//==============================================================================
/**
 * @brief   リセットエラー状態になったかどうか確認する
 * @param   none
 * @retval  リセットエラー状態ならTRUE
 */
//==============================================================================

u8 CommIsResetError(GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle){
        return pNetHandle->ResetStateType;
    }
    return FALSE;
}

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
 * @brief   エラーを感知して 通信エラー用ウインドウを出す
 * @param   heapID    メモリー確保するHEAPID
 * @param   bgl       GF_BGL_INI
 * @retval  none
 */
//--------------------------------------------------------------

void CommErrorDispCheck(GFL_NETHANDLE* pNetHandle,int heapID)
{
    if(CommStateGetErrorCheck()){
        if(CommIsError() || CommStateIsWifiError() || CommStateGetWifiDPWError()
           || (pNetHandle->stateError!=0)){
            if(!CommIsResetError()){   // リセットエラー状態で無い場合
                Snd_Stop();
                SaveData_DivSave_Cancel(pNetHandle->pSaveData); // セーブしてたら止める
                sys.tp_auto_samp = 1;  // サンプリングも止める

                if(pNetHandle->stateError == COMM_ERROR_RESET_GTS){
                    CommSetErrorReset(COMM_ERROR_RESET_GTS);  // エラーリセット状態になる
                }
                else if((pNetHandle->serviceNo == COMM_MODE_FUSIGI_WIFI)
                   || (pNetHandle->serviceNo == COMM_MODE_MYSTERY)){
                    CommSetErrorReset(COMM_ERROR_RESET_TITLE);  // エラーリセット状態になる
                }
                else{
                    CommSetErrorReset(COMM_ERROR_RESET_SAVEPOINT);  // エラーリセット状態になる
                }
            }
        }
    }
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

BOOL CommStateSetError(GFL_NETHANDLE* pNetHandle,int no)
{

    if(pNetHandle){
        pNetHandle->stateError = no;
        CommSystemShutdown();
        return TRUE;
    }
    return FALSE;
}

#endif  //DEBUG_OHNO

