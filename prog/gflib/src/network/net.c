//=============================================================================
/**
 * @file	net.c
 * @brief	GFL通信ライブラリー
 * @author	GAME FREAK Inc.
 * @date    2006.12.14
 */
//=============================================================================

#include "gflib.h"

#include "ui/ui_def.h"
#include "net_def.h"
#include "net_local.h"
#include "device/net_whpipe.h"
#include "device/dwc_rapcommon.h"
#include "net_system.h"
#include "net_command.h"
#include "net_state.h"
#include "wm_icon.h"
#include "device/dwc_rap.h"


#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "net_handle.h"

#define  GFL_NET_OVERLAY  (0)




#if GFL_NET_WIFI
FS_EXTERN_OVERLAY(wifi);
#endif
FS_EXTERN_OVERLAY(wireless);


// 通信で使用するCreateHEAP量

#define _HEAPSIZE_NET              (0xE000)          ///< NET関連のメモリ領域
#define _HEAPSIZE_WIFI             (MYDWC_HEAPSIZE+0xA000)  ///< DWCが使用する領域

static GFL_NETSYS* _pNetSys = NULL; // 通信のメモリーを管理するstatic変数

// コンポーネント転送終了の確認用  イクニューモンコンポーネント処理を移動させるときはこれも移動
static PTRIchneumonCallback pIchneumonCallback = NULL;
static u8 bLoadIchneumon = FALSE;

//==============================================================================
/**
 * @brief    boot時の通信初期化
 *           WIFIのIPL初期設定が起動時に必要な為
 * @param    heapID    使用するtempメモリID
 * @param    errorFunc エラー時に呼び出す関数
 * @return   none
 */
//==============================================================================
void GFL_NET_Boot(HEAPID heapID, NetErrorFunc errorFunc)
{
#if GFL_NET_WIFI
    //WIFIのIPL初期設定
    GFL_NET_WifiStart( heapID, errorFunc );
#endif
}

//==============================================================================
/**
 * @brief 通信初期化
 * @param[in]   pNetInit  通信初期化構造体のポインタ
 * @return none
 */
//==============================================================================
void GFL_NET_Init(const GFLNetInitializeStruct* pNetInit,NetStepEndCallback callback)
{
    int index;
    // 通信ヒープ作成

#if GFL_NET_OVERLAY
    if(!pNetInit->bWiFi){
        GFL_OVERLAY_Load( FS_OVERLAY_ID( wireless ) );
    }
#if GFL_NET_WIFI
    else{
        GFL_OVERLAY_Load( FS_OVERLAY_ID( wifi ) );
    }
#endif
#endif
    
    GFL_HEAP_CreateHeap( pNetInit->baseHeapID, pNetInit->netHeapID, _HEAPSIZE_NET );
    if(pNetInit->bWiFi){
        GFL_HEAP_CreateHeap( pNetInit->baseHeapID, pNetInit->wifiHeapID, _HEAPSIZE_WIFI);
    }
    {
        GFL_NETSYS* pNet = GFL_HEAP_AllocClearMemory(pNetInit->netHeapID, sizeof(GFL_NETSYS));
        _pNetSys = pNet;

        NET_PRINT("size %d addr %x\n",sizeof(GFLNetInitializeStruct),(u32)&pNet->aNetInit);

        GFL_STD_MemCopy(pNetInit, &pNet->aNetInit, sizeof(GFLNetInitializeStruct));
        GFL_NET_StateDeviceInitialize(pNetInit->netHeapID, callback);
      //  for(index = 0; index < pNet->aNetInit.maxConnectNum; index++){
        //    _pNetSys->pKey[index] = GFL_UI_KEY_Init(pNet->aNetInit.netHeapID);
       // }

        GFL_NET_COMMAND_Init( pNetInit->recvFuncTable, pNetInit->recvFuncTableNum, pNetInit->pWork);
    }
    GFL_NET_WirelessIconEasyXY(pNetInit->iconX,pNetInit->iconY,pNetInit->bWiFi, pNetInit->netHeapID);
    NET_PRINT("GFL_NET_Init\n");
}

//==============================================================================
/**
 * @brief 通信初期化が完了したかどうかを確認する  今のところIchneumonと同じ
 * @retval TRUE   初期化終了
 * @retval FALSE  初期化がまだ終わっていない
 */
//==============================================================================
BOOL GFL_NET_IsInit(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    if(pNet==NULL){
        return FALSE;
    }
    if(!pNet->aNetInit.bWiFi){
        if( _GFL_NET_GetNETWL()){
            return TRUE;
        }
    }
#if GFL_NET_WIFI
    else{
        return GFL_NET_DWC_IsInit();
    }
#endif
    return FALSE;
}

//==============================================================================
/**
 * @brief   通信終了コールバック
 */
//==============================================================================
static void _netEndCallback(void* pWork)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    HEAPID netHeapID = pNet->aNetInit.netHeapID;
    HEAPID wifiHeapID = pNet->aNetInit.wifiHeapID;
    BOOL bWiFi = pNet->aNetInit.bWiFi;
    int index;

    pNet->pNetEndCallback(pNet->aNetInit.pWork);
    
//    for(index = 0; index < pNet->aNetInit.maxConnectNum;index++){
  //      GFL_HEAP_FreeMemory(pNet->pKey[index]);
    //}
    GFL_NET_HANDLE_DeleteAll(pNet);
    GFL_HEAP_FreeMemory(pNet);
    _pNetSys = NULL;

    if(bWiFi){
        GFL_HEAP_DeleteHeap(wifiHeapID);
    }
    GFL_HEAP_DeleteHeap(netHeapID);

#if GFL_NET_OVERLAY
    if(!bWiFi){
        GFL_OVERLAY_Unload( FS_OVERLAY_ID( wireless ) );
    }
#if GFL_NET_WIFI
    else{
        GFL_OVERLAY_Unload( FS_OVERLAY_ID( wifi ) );
    }
#endif
#endif
}

//==============================================================================
/**
 * @brief   通信終了
 * @param   netEndCallback    通信が終了した際に呼ばれるコールバックです
 */
//==============================================================================
void GFL_NET_Exit(NetEndCallback netEndCallback)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    pNet->pNetEndCallback = netEndCallback;
    GFL_NET_StateExit(_netEndCallback);
    GFL_NET_WirelessIconEasyEnd();
}

//-----ビーコン関連
//==============================================================================
/**
 * @brief ビーコンを受け取るサービスを追加指定  （いくつかのサービスのビーコンを拾いたい時に）
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @param   GameServiceID gsid  ゲームサービスID  通信の種類
 * @retval  none
 */
//==============================================================================
//void GFL_NET_AddBeaconServiceID(GFL_NETHANDLE* pNet, GameServiceID gsid)
//{
//}

//==============================================================================
/**
 * @brief   受信ビーコンデータを得る
 * @param   index  ビーコンバッファindex
 * @return  ビーコンデータの先頭ポインタ なければNULL
 */
//==============================================================================
void* GFL_NET_GetBeaconData(int index)
{
    if(index >= GFL_NET_MACHINE_MAX){
        return NULL;
    }
    return GFL_NET_WLGetUserBss(index);
}

//==============================================================================
/**
 * @brief   受信ビーコンに対応したMACアドレスを得る
 * @param   index  ビーコンバッファindex
 * @return  ビーコンデータの先頭ポインタ なければNULL
 */
//==============================================================================
u8* GFL_NET_GetBeaconMacAddress(int index)
{
    if(index >= GFL_NET_MACHINE_MAX){
        return NULL;
    }
    return GFL_NET_WLGetUserMacAddress(index);
}

//--------接続・切断

//==============================================================================
/**
 * @brief   通信のメイン実行関数
 * @retval  TRUE  処理を行ってよい場合
 * @retval  FALSE 同期が取れていないので処理を行わない場合
 */
//==============================================================================

BOOL GFL_NET_Main(void)
{
    int i;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    if(pNet==NULL){
        return TRUE;
    }
    GFL_NET_WirelessIconEasyFunc();
    GFL_NET_StateMainProc();
    GFL_NET_HANDLE_MainProc();
    GFL_NET_SystemUpdateData();
    return GFL_NET_SystemCheckDataSharing();
}


//==============================================================================
/**
 * @brief   子機になり指定した親機に接続する
 * @param   GFL_NETHANDLE  通信ハンドルのポインタ
 * @param   macAddress     マックアドレスのバッファ
 * @return  none
 */
//==============================================================================
void GFL_NET_InitClientAndConnectToParent(u8* macAddress)
{
    GFL_NET_StateConnectMacAddress(macAddress,TRUE);
}

//==============================================================================
/**
 * @brief   指定した親機に接続する
 * @param   GFL_NETHANDLE  通信ハンドルのポインタ
 * @param   macAddress     マックアドレスのバッファ
 * @return  none
 */
//==============================================================================
void GFL_NET_ConnectToParent(u8* macAddress)
{
    GFL_NET_StateConnectMacAddress(macAddress, FALSE);
}

//==============================================================================
/**
 * @brief   子機になりビーコンを集める
 * @param   @@OO エンドコールバックが必要
 * @return  none
 */
//==============================================================================
void GFL_NET_StartBeaconScan(void)
{
    GFL_NET_StateBeaconScan();
}

//==============================================================================
/**
 * @brief    親機になり待ち受ける
 * @return   none
 */
//==============================================================================
void GFL_NET_InitServer(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

#if GFL_NET_WIFI
    if(pNet->aNetInit.bWiFi){
        GFL_NET_StateConnectWifiParent(pNet->aNetInit.netHeapID);
    }
    else{
        GFL_NET_StateConnectParent(pNet->aNetInit.netHeapID);
    }
#else
    GFL_NET_StateConnectParent(pNet->aNetInit.netHeapID);
#endif
}


//==============================================================================
/**
 * @brief    親機になる
 * @param    GFL_NETHANDLE  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
void GFL_NET_CreateServer(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    GFL_NET_StateCreateParent(pNet->aNetInit.netHeapID);
}

//==============================================================================
/**
 * @brief    親機子機を繰り返し、誰でもいいので接続する
 * @param    none
 * @return   none
 */
//==============================================================================
void GFL_NET_ChangeoverConnect(NetStepEndCallback callback)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    GFL_NET_StateChangeoverConnect(pNet->aNetInit.netHeapID, callback);
}

//==============================================================================
/**
 * @brief    Wi-Fiロビーへ接続する
 * @param    GFL_NETHANDLE        通信ハンドルのポインタ
 * @param    GFL_WIFI_FRIENDLIST  フレンドリスト構造体のポインタ
 * @return   none
 */
//==============================================================================

void GFL_NET_WifiLogin(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    GFL_NET_StateWifiEnterLogin(pNet->aNetInit.netHeapID, pNet->aNetInit.wifiHeapID);
}

//==============================================================================
/**
 * @brief このハンドルの通信番号を得る
 * @param[in,out]  NetHandle* pNet     通信ハンドルのポインタ
 * @retval  NetID     通信ID
 */
//==============================================================================
NetID GFL_NET_GetNetID(GFL_NETHANDLE* pNetHandle)
{
    return GFL_NET_HANDLE_GetNetHandleID(pNetHandle);
}

//==============================================================================
/**
 * @brief 現在の接続台数を得る
 * @retval  int  接続数
 */
//==============================================================================
int GFL_NET_GetConnectNum(void)
{
    return GFL_NET_HANDLE_GetNumNegotiation();
}

//==============================================================================
/**
 * @brief IDの通信と接続しているかどうかを返す
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @param   NetID id            ネットID
 * @retval  BOOL  接続していたらTRUE
 */
//==============================================================================
BOOL GFL_NET_IsConnectMember(NetID id)
{
    return GFL_NET_HANDLE_IsNegotiation(GFL_NET_GetNetHandle(id));
}

//==============================================================================
/**
 * @brief 新規接続禁止＆許可を設定
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @param   BOOL bEnable     TRUE=接続許可 FALSE=禁止
 * @retval  none
 */
//==============================================================================
void GFL_NET_SetClientConnect(GFL_NETHANDLE* pNet,BOOL bEnable)
{
}

//==============================================================================
/**
 * @brief   リセットできる状態かどうか
 * @retval  TRUEならリセット可能
 */
//==============================================================================

BOOL GFL_NET_IsResetEnable(void)
{
    if(_pNetSys==NULL){
        return TRUE;
    }
    if(GFL_NET_WLIsConnectStalth()){ // 通信終了
        return TRUE;
    }
    if(!GFL_NET_WLIsConnect()){
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   通信アイコンを再度リロードします
 * @retval  none
 */
//==============================================================================

void GFL_NET_ReloadIcon(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    if(pNet){
        GFL_NET_WirelessIconEasyXY( pNet->aNetInit.iconX, pNet->aNetInit.iconY, pNet->aNetInit.bWiFi, pNet->aNetInit.netHeapID);
    }
}


//==============================================================================
/**
 * @brief   親機マシンかどうか
 * @param   void
 * @retval  TRUE   親機
 * @retval  FALSE  子機
 */
//==============================================================================

BOOL GFL_NET_IsParentMachine(void)
{
    BOOL ret = FALSE;

    if(GFL_NET_SystemGetCurrentID() == GFL_NET_PARENT_NETID){
        ret = TRUE;
    }
    return ret;
}

//==============================================================================
/**
 * @brief   親機のハンドルかどうか
 * @param   void
 * @retval  TRUE   親機
 * @retval  FALSE  子機
 */
//==============================================================================
BOOL GFL_NET_IsParentHandle(GFL_NETHANDLE* pNetHandle)
{
    if(GFL_NET_HANDLE_GetNetHandleID(pNetHandle) == GFL_NET_PARENT_NETID){
        return TRUE;
    }
    return FALSE;
}


//--------送信

//==============================================================================
/**
 * @brief 送信開始を子機に伝える（親機専用関数）
 *        この関数以降設定した送信データが子機で使用される
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   NetID id            子機ID
 * @return  none
 */
//==============================================================================
void GFL_NET_SendEnableCommand(GFL_NETHANDLE* pNet,const NetID id)
{}

//==============================================================================
/**
 * @brief 送信許可が下りたかどうかを検査する（子機専用関数）
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @retval  TRUE   許可
 * @retval  FALSE  禁止
 */
//==============================================================================
BOOL GFL_NET_IsSendEnable(GFL_NETHANDLE* pNet)
{
    return (pNet->negotiationType == _NEG_TYPE_CONNECT);
}

//==============================================================================
/**
 * @brief 送信開始
 *        全員に無条件で送信  送信サイズは初期化時のテーブルから引き出す
 *        データは必ずコピーする
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   u16 sendCommand                送信するコマンド
 * @param[in]   void* data                     送信データポインタ
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
BOOL GFL_NET_SendData(GFL_NETHANDLE* pNet,const u16 sendCommand,const void* data)
{
    return GFL_NET_SystemSendData(sendCommand, data, 0,
                                  FALSE, GFL_NET_HANDLE_GetNetHandleID(pNet) ,GFL_NET_SENDID_ALLUSER, FALSE);
}

//==============================================================================
/**
 * @brief 送信開始
 * @param[in,out]  pNet  通信ハンドル
 * @param[in]   sendID                     送信相手 全員へ送信する場合 NET_SENDID_ALLUSER
 * @param[in]   sendCommand                送信するコマンド
 * @param[in]   size                       送信データサイズ
 * @param[in]   data                       送信データポインタ
 * @param[in]   bFast                      優先順位を高くして送信する場合TRUE
 * @param[in]   bRepeat                    このコマンドがキューにないときだけ送信
 * @param[in]   bSendBuffLock              送信バッファを呼ぶ側が保持する場合（通信側のメモリを消費しないので大きいデータを送信できます）
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
BOOL GFL_NET_SendDataEx(GFL_NETHANDLE* pNet,const NetID sendID,const u8 sendCommand, const u32 size,const void* data, const BOOL bFast, const BOOL bRepeat, const BOOL bSendBuffLock)
{
    if((GFL_NET_IsSendEnable(pNet)==FALSE) && (sendCommand >= GFL_NET_CMD_COMMAND_MAX)){
        // 認証が終わらないのに、メイン以外のコマンドを送ることはできない
        return FALSE;
    }
    if(bRepeat && GFL_NET_SystemIsSendCommand(sendCommand,GFL_NET_HANDLE_GetNetHandleID(pNet))){
        return FALSE;
    }
    return GFL_NET_SystemSendData(sendCommand, data, size, bFast, GFL_NET_HANDLE_GetNetHandleID(pNet) , sendID, bSendBuffLock);
}

//==============================================================================
/**
 * @brief 送信データが無いかどうか
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @retval  TRUE   無い場合
 * @retval  FALSE  残っている場合
 */
//==============================================================================
BOOL GFL_NET_IsEmptySendData(GFL_NETHANDLE* pNet)
{
    return GFL_NET_SystemIsEmptyQueue();
}



//--------その他、ツール類
//==============================================================================
/**
 * @brief タイミングコマンドを発行する
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   u8 no   タイミング取りたい番号
 * @return      none
 */
//==============================================================================
void GFL_NET_TimingSyncStart(GFL_NETHANDLE* pNet, const u8 no)
{
    GFL_NET_HANDLE_TimingSyncStart(pNet,no);
}

//==============================================================================
/**
 * @brief タイミングコマンドが届いたかどうかを確認する
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param[in]   no   届く番号
 * @retval  TRUE    タイミングが合致
 * @retval  FALSE   タイミングがそろっていない
 */
//==============================================================================
BOOL GFL_NET_IsTimingSync(GFL_NETHANDLE* pNet, const u8 no)
{
    return GFL_NET_HANDLE_IsTimingSync(pNet, no);
}


//==============================================================================
/**
 * @brief     モード変更関数
 * @param     NetHandle* pNet  通信ハンドル
 * @return    none
 */
//==============================================================================
static BOOL _changeModeRequest(GFL_NETHANDLE* pNetHandle, NetModeChangeFunc func,BOOL bDSMode)
{
    BOOL bReturn;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    bReturn = GFL_NET_SendData(pNetHandle, GFL_NET_CMD_DSMP_CHANGE, &bDSMode);
    if(FALSE == bReturn){
        if(func){
            func(pNet->aNetInit.pWork, FALSE);
        }
    }
    else{
        pNet->pNetModeChangeFunc = func;
    }
    return bReturn;
}

//==============================================================================
/**
 * @brief     DSモードに変更する
 * @param     NetHandle* pNet    通信ハンドル
 * @param     NetModeChangeFunc  変更した時に呼ばれるコールバック
 * @return    TRUE 成功   FALSE 失敗
 */
//==============================================================================
BOOL GFL_NET_ChangeDsMode(GFL_NETHANDLE* pNet, NetModeChangeFunc func)
{
    return _changeModeRequest(pNet, func, TRUE);
}

//==============================================================================
/**
 * @brief     MPモードに変更する
 * @param     NetHandle* pNet    通信ハンドル
 * @param     NetModeChangeFunc  変更した時に呼ばれるコールバック
 * @return    TRUE 成功   FALSE 失敗
 */
//==============================================================================
BOOL GFL_NET_ChangeMpMode(GFL_NETHANDLE* pNet, NetModeChangeFunc func)
{
    return _changeModeRequest(pNet, func, FALSE);
}


#if GFL_NET_WIFI //wifi

//==============================================================================
/**
 * @brief    DWCのユーザーデータを得る
 * @return   DWCUserDataのポインタ
 */
//==============================================================================

DWCUserData* GFI_NET_GetMyDWCUserData(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    return pNet->aNetInit.myUserData;
}

//==============================================================================
/**
 * @brief    DWCの友達データを得る
 * @return   DWCFriendDataのポインタ
 */
//==============================================================================

DWCFriendData* GFI_NET_GetMyDWCFriendData(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    return pNet->aNetInit.keyList;
}

//==============================================================================
/**
 * @brief    DWCの友達データ本数を得る
 * @return   DWCFriendDataの本数
 */
//==============================================================================
int GFI_NET_GetFriendNumMax(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    GF_ASSERT(pNet->aNetInit.maxBeaconNum < DWC_ACC_USERDATA_BUFSIZE);
    return pNet->aNetInit.maxBeaconNum;
}


//==============================================================================
/**
 * @brief    DWCUserデータをセーブしてほしい時のコールバック関数を呼び出す
 * @return   none
 */
//==============================================================================
void GFI_NET_NetWifiSaveUserDataFunc(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    if(pNet->aNetInit.wifiSaveFunc){
        pNet->aNetInit.wifiSaveFunc();
    }
}

//==============================================================================
/**
 * @brief    友達データをマージするための関数を呼び出す
 * @param    deletedIndex  消えるフレンドコード
 * @param    srcIndex      元になるフレンドコード
 * @return   none
 */
//==============================================================================
void GFI_NET_NetWifiMargeFrinedDataFunc(int deletedIndex,int srcIndex)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    if(pNet->aNetInit.wifiMargeFunc){
        pNet->aNetInit.wifiMargeFunc(deletedIndex, srcIndex);
    }
}

//==============================================================================
/**
 * @brief    WIFIロビーへ接続しているかどうか
 * @param    NetHandle* pNet  通信ハンドル
 * @retval   TRUE   ロビー接続
 * @retval   FALSE   まだ接続していない
 */
//==============================================================================
BOOL GFL_NET_IsWifiLobby(void)
{
    if(2 > GFL_NET_StateWifiIsMatched()){
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   ユーザーデータを初期化する
 * @param   DWCUserData  ユーザーデータ構造体
 * @retval  none
 */
//==============================================================================
void GFL_NET_WIFI_InitUserData(DWCUserData *pUserdata)
{
    mydwc_createUserData( pUserdata );
}

//==============================================================================
/**
 * @brief   WIFI接続要求が来たかどうか
 * @retval  TRUE  きた
 * @retval  FALSE こない
 */
//==============================================================================
BOOL GFL_NET_WIFI_IsNewPlayer(void)
{
    return mydwc_IsNewPlayer();
}

//==============================================================================
/**
 * @brief   物理的なWIFIコネクション番号を得る
 * @retval  -1    コネクションが張られてない
 * @retval  0以上 コネクションのID
 */
//==============================================================================
int GFL_NET_WIFI_GetLocalConnectNo(void)
{
    return mydwc_getaid();
}

//==============================================================================
/**
 * @brief   ランダムマッチ接続を開始する
 * @param   pNetHandle  ネットハンドル
 * @retval  TRUE  成功
 * @retval  FALSE 失敗
 */
//==============================================================================
BOOL GFL_NET_StartRandomMatch(void)
{
    return GFL_NET_StateStartWifiRandomMatch();
}

//==============================================================================
/**
 * @brief   通信を同期か非同期かにきりかえる
 * @param   TRUE  同期    FALSE 非同期
 * @return  なし
 */
//==============================================================================
void GFL_NET_SetWifiBothNet(BOOL flag)
{
    GFL_NET_SystemSetWifiBothNet(flag);
}

//==============================================================================
/**
 * @brief   ユーザーの使用ワークを再設定する
 * @param   ワークのポインタ
 * @return  なし
 */
//==============================================================================

void GFL_NET_SetUserWork(void* pWork)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    pNet->aNetInit.pWork = pWork;
}


#endif // GFL_NET_WIFI

//==============================================================================
/**
 * @brief    接続可能なマシンの台数＝自分含む数を返す
 * @return   接続人数
 */
//==============================================================================
int GFI_NET_GetConnectNumMax(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    GF_ASSERT(pNet->aNetInit.maxConnectNum <= GFL_NET_MACHINE_MAX);
    return pNet->aNetInit.maxConnectNum;
}


//==============================================================================
/**
 * @brief    送信最大サイズを得る
 * @return   送信サイズ
 */
//==============================================================================
int GFI_NET_GetSendSizeMax(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
#ifdef PM_DEBUG
    GF_ASSERT(pNet->aNetInit.maxConnectNum > 0);
//    GF_ASSERT(pNet->aNetInit.maxSendSize < ((WM_SIZE_MP_DATA_MAX-4)/pNet->aNetInit.maxConnectNum));
#endif
    return pNet->aNetInit.maxSendSize;
}

//==============================================================================
/**
 * @brief    GFI_NET_AutoParentConnectFunc
 * @param    pNetHandle  ネットハンドル
 * @param    errorNo     エラー番号
 * @return   DWCUserDataのポインタ
 */
//==============================================================================

void GFI_NET_AutoParentConnectFunc(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    if(pNet->aNetInit.autoParentConnect!=NULL){
        pNet->aNetInit.autoParentConnect(pNet->aNetInit.pWork);
    }
}

//==============================================================================
/**
 * @brief    FatalError実行
 * @param    pNetHandle  ネットハンドル
 * @param    errorNo     エラー番号
 * @return   DWCUserDataのポインタ
 */
//==============================================================================

void GFI_NET_FatalErrorFunc(int errorNo)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    if(pNet->aNetInit.errorFunc != NULL){
        pNet->aNetInit.errorFunc(GFL_NET_HANDLE_GetCurrentHandle(), errorNo, pNet->aNetInit.pWork);
    }
}

//==============================================================================
/**
 * @brief    通信管理構造体を得る  （通信内部使用 外部に公開するならGFL_NET_Initializeの戻り値にするほうがいい）
 * @return   ネットシステム構造体ポインタ
 */
//==============================================================================
GFL_NETSYS* _GFL_NET_GetNETSYS(void)
{
    return _pNetSys;
}

//==============================================================================
/**
 * @brief    通信管理初期化構造体を得る
 * @return   ネットシステム構造体ポインタ
 */
//==============================================================================
GFLNetInitializeStruct* _GFL_NET_GetNETInitStruct(void)
{
    return &_pNetSys->aNetInit;
}

//==============================================================================
/**
 * @brief    WL通信管理構造体を得る  （通信内部使用 割り込み用）
 * @return   ネットシステム構造体ポインタ
 */
//==============================================================================

GFL_NETWL* _GFL_NET_GetNETWL(void)
{
    return _pNetSys->pNetWL;
}

//==============================================================================
/**
 * @brief    WL通信管理構造体をセットする
 * @param    WL通信管理構造体
 * @return   none
 */
//==============================================================================

void _GFL_NET_SetNETWL(GFL_NETWL* pWL)
{
    _pNetSys->pNetWL = pWL;
}

//==============================================================================
/**
 * @brief   自動的にエラー検出を行うかどうか
 *          (TRUEの場合エラーをみつけるとブルースクリーン＋再起動になる)
 * @param   bAuto  TRUEで検査開始 FALSEでエラー処理を行わない
 * @retval  none
 */
//==============================================================================
void GFL_NET_SetAutoErrorCheck(BOOL bAuto)
{
    GFL_NET_STATE_SetAutoErrorCheck(bAuto);
}

//==============================================================================
/**
 * @brief   子機がいない場合にエラーにするかどうかを設定する
 * @param   bFlg    切断=エラーにする
 * @param   bAuto  TRUEで検査開始
 * @retval  none
 */
//==============================================================================
void GFL_NET_SetNoChildErrorCheck(BOOL bFlg)
{
    GFL_NET_STATE_SetNoChildErrorCheck(bFlg);
}


//------------------------------------------------------------------
/**
 * @brief   無線駆動制御ライブラリの非同期的な処理終了が通知されるコールバック関数。
 * @param   arg		WVR_StartUpAsyncコール時に指定した引数。未使用。
 * @param   result	非同期関数の処理結果。
 * @retval  none		
 */
//------------------------------------------------------------------
static void _startUpCallback(void *arg, WVRResult result)
{
    BOOL bResult = TRUE;
    if (result != WVR_RESULT_SUCCESS) {
        GF_ASSERT_MSG(0,"WVR_StartUpAsync error.[%08xh]\n", result);
        bResult = FALSE;
    }
    else{
        bLoadIchneumon = TRUE;
		NET_PRINT("WVR Trans VRAM-D.\n");
	}
    if(pIchneumonCallback){
        pIchneumonCallback(arg, bResult);
    }
}

//------------------------------------------------------------------
/**
 * @brief   無線駆動制御ライブラリの非同期的な処理終了が通知されるコールバック関数。
 * @param   arg		WVR_StartUpAsyncコール時に指定した引数。未使用。
 * @param   result	非同期関数の処理結果。
 * @retval  none		
 */
//------------------------------------------------------------------
static void _endCallback(void *arg, WVRResult result)
{
    BOOL bResult = TRUE;
    if (result != WVR_RESULT_SUCCESS) {
        GF_ASSERT_MSG(0,"WVR_StartUpAsync error.[%08xh]\n", result);
        bResult = FALSE;
    }
    if(pIchneumonCallback){
        pIchneumonCallback(arg, bResult);
    }
    GFL_UI_SleepEnable(GFL_UI_SLEEP_NET);  // スリープ許可
}

//==============================================================================
/**
 * @brief    イクニューモンライブラリのみを初期化する
 * @param    callback 終わったら呼ばれるコールバック
 * @param    pWork    呼ぶ側のワーク
 * @return   none
 */
//==============================================================================

void GFL_NET_InitIchneumon(PTRIchneumonCallback callback,void* pWork)
{
    int ans;
    //************************************
    GFL_UI_SleepDisable(GFL_UI_SLEEP_NET);  // スリープ禁止
    GF_ASSERT(callback);
    pIchneumonCallback = callback;
    // 無線ライブラリ駆動開始
	// イクニューモンコンポーネントをVRAM-Dに転送する
    ans = WVR_StartUpAsync(GX_VRAM_ARM7_128_D, _startUpCallback, pWork);
    if (WVR_RESULT_OPERATING != ans) {
        //イクニューモンを使用する前に VRAMDをdisableにする必要がある
        // そうでないとここにくる
        OS_TPanic("WVR_StartUpAsync failed. %d\n",ans);
    }
    else{
        NET_PRINT("WVRStart\n");
    }
}

//==============================================================================
/**
 * @brief    イクニューモンライブラリの終了をしらせる
 * @retval   none
 */
//==============================================================================


void GFL_NET_ExitIchneumon(PTRIchneumonCallback callback,void* pWork)
{
    NET_PRINT("VRAMD Finalize\n");
    GF_ASSERT(callback);
    bLoadIchneumon = FALSE;
    pIchneumonCallback = callback;
    WVR_TerminateAsync(_endCallback,NULL);  // イクニューモン切断
}

//==============================================================================
/**
 * @brief    イクニューモンライブラリが動いているかどうかを得る
 * @retval   TRUE なら動作中
 */
//==============================================================================

BOOL GFL_NET_IsIchneumon(void)
{
    return bLoadIchneumon;
}


//==============================================================================
/**
 * @brief    デバッグ用 メモリの中を分析する
 * @param    data  メモリ
 * @param    size  検査サイズ
 * @return   none
 */
//==============================================================================
#if 1
void debugcheck(u32* data,int size )
{
    int i;
    u8* p=(u8*)data;

    for(i = 0 ; i < size ; i++){
        if((*p == 0xfc) && (*p+1 == 0x0f) && (*p+2 == 0x00) && (*p+3 == 0x10)){
            OS_Panic("check");
        }
        p++;
    }
}
#endif 

