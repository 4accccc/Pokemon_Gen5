//=============================================================================
/**
 * @file	comm_state.c
 * @brief	通信状態を管理するサービス  通信の上位にある
 *          スレッドのひとつとして働き、自分の通信状態や他の機器の
 *          開始や終了を管理する
 * @author	k.ohno
 * @date    2006.01.12
 */
//=============================================================================

#include "gflib.h"

#include "device/wh_config.h"

#include "net_def.h"
#include "device/net_wireless.h"
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

#define _HEAPSIZE_BATTLE           (0x7080)  // バトル機領域
#define _HEAPSIZE_UNDERGROUND      (0xE000)  // 地下領域
#define _HEAPSIZE_UNION            (0x7080)  // ユニオンルーム領域
#define _HEAPSIZE_POKETCH          (0x7000)  // ぽけっち機領域
#define _HEAPSIZE_PARTY            (0x7000)  // パーティーサーチ
#define _HEAPSIZE_WIFI            (0x2A000+0x7000)  //DWCRAP が使用する領域
#define _HEAPSIZE_DPW              (0x100)   // 世界交換　タワー用

#define _PACKETSIZE_BATTLE         (512)//(1376)  // バトル機領域
#define _PACKETSIZE_UNDERGROUND     (500)  // 地下領域
#define _PACKETSIZE_UNION          (512)//(1280)  // ユニオンルーム領域
#define _PACKETSIZE_POKETCH          (32)  // ぽけっち領域
#define _PACKETSIZE_PARTY         (32)  // ぽけっち領域


#define _START_TIME (50)     // 開始時間
#define _CHILD_P_SEARCH_TIME (32) ///子機として親を探す時間
#define _PARENT_WAIT_TIME (40) ///親としてのんびり待つ時間
#define _FINALIZE_TIME (2)
#define _EXIT_SENDING_TIME (5)
#define _EXIT_SENDING_TIME2 (15)
#define _PARENT_END_TIME (2)
#define _SEND_NAME_TIME (10)
#define _PARENTSCAN_PA (3)  // 親機として検索する確立は1/3

#define _RETRY_COUNT_UNION  (3)  // ユニオンルームで子機が接続に要する回数

#define _TCB_COMMCHECK_PRT   (10)    ///< フィールドを歩く通信の監視ルーチンのPRI




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

// ネゴシエーション用確認KEY
static u8 _negotiationMsg[]={"FREAK"};
static u8 _negotiationMsgReturnOK[]={" GAME"};
static u8 _negotiationMsgReturnNG[]={" FULL"};


//==============================================================================
/**
 * 通信管理ステートの変更
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
 * 通信管理ステートの変更
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
 * 通信管理ステートの初期化処理
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

    //CommRandSeedInitialize(&_pCommState->sRand);  //@@OO乱数初期化を入れる
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
 * 通信管理ステートの終了処理
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
 * 動いているかどうか
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
 * イクニューモン初期化後通信起動
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
    
    pWL = GFL_NET_WLGetHandle(pNetIni->allocNo, pNetIni->gsid, pNetIni->maxConnectNum);
    _GFL_NET_SetNETWL(pWL);

    GFL_NET_WLInitialize(pNetIni->allocNo, pNetIni->beaconGetFunc, pNetIni->beaconGetSizeFunc,
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
 * 子機待機状態  親機に情報を送信
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
 * 親機ビーコンを拾った場合のコールバック
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
 * 子機待機状態  親機に許可もらい中
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
 * マックアドレスを指定して子機接続開始
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
 * 子機待機状態  ビーコン収集
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
 * 子機開始  ビーコンの収集に入る
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
 * 通信管理ステートの処理
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
 * 親機として待機中
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _parentWait(GFL_NETHANDLE* pHandle)
{
}

//==============================================================================
/**
 * 親機として初期化を行う
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

    if(GFL_NET_SystemParentModeInit(TRUE, _PACKETSIZE_BATTLE,TRUE)){
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
 * 親としての通信処理開始
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
 * 子機再スタート
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
//    if(GFL_NET_SystemChildModeInit(FALSE,512)){
        rand = MATH_Rand32(&pNetHandle->sRand, (_CHILD_P_SEARCH_TIME/2))+(_CHILD_P_SEARCH_TIME/2);
        NET_PRINT("子機開始 %d \n",rand);
        _CHANGE_STATE(_changeoverChildSearching, rand);
    }
}

//==============================================================================
/**
 * 親機になりbcon放出
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
 * ここから先エラーの検査を通信が処理するかどうかを設定
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
 * 親として接続中
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
 * 親機になりbcon放出
 * @param   none
 * @retval  none
 */
//==============================================================================


static void _changeoverParentWait(GFL_NETHANDLE* pNetHandle)
{
    if(GFL_NET_SystemIsChildsConnecting()){   // 自分以外がつながったら親機固定

        NET_PRINT("親機 -- つながり\n");
//        _pCommState->bFirstParent = TRUE;  // 親機として繋がったのでフラグを戻しておく
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
 * 親機開始
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
    if(GFL_NET_SystemParentModeInit(FALSE,  _PACKETSIZE_UNION,TRUE))  {
        pNetHandle->bFirstParent = FALSE;
        NET_PRINT("親機\n");
        _CHANGE_STATE(_changeoverParentWait, 30);
    }
}

//==============================================================================
/**
 * 子機となって親機を探し中
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
 * 親子自動切り替え通信処理開始
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
 * 子機の終了を待ち、親機が終了する
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
 * 終了コマンド 子機が親機にやめるように送信  全員の子機に送り返すGFL_NET_CMD_EXIT_REQ
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
        _pCommState->disconnectIndex = 0;
        _CHANGE_STATE(_stateWifiMatchEnd, 0);
    }
    else{
        _pCommState->disconnectIndex = 1;
        _CHANGE_STATE(_stateWifiMatchEnd, 0);
    }
    _pCommState->bWifiDisconnect = TRUE;
#endif
}

//==============================================================================
/**
 * 終了コマンド   子機が親機にやめるように送信 ぶっつりきる GFL_NET_CMD_EXIT
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
 * ネゴシエーション用コールバック CS_COMM_NEGOTIATION
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
 * ネゴシエーション用コールバック CS_COMM_NEGOTIATION_RETURN
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
 * 通信処理終了手続き開始
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
 * 何もしないステート
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

