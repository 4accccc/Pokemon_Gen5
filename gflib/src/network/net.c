//=============================================================================
/**
 * @file	net.c
 * @brief	GFL通信ライブラリー
 * @author	GAME FREAK Inc.
 * @date    2006.12.14
 */
//=============================================================================

#include "gflib.h"

#include "device/wh_config.h"

#include "net_def.h"
#include "device/net_wireless.h"
#include "device/dwc_rapcommon.h"
#include "net_system.h"
#include "net_command.h"
#include "net_state.h"
#include "wm_icon.h"

#include "device/dwc_rap.h"


#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "tool/net_tool.h"

// 通信で使用するCreateHEAP量

#define _HEAPSIZE_NET              (0xE000)          ///< NET関連のメモリ領域
#define _HEAPSIZE_WIFI             (MYDWC_HEAPSIZE+0xA000)  ///< DWCが使用する領域

/// @brief  通信システム管理構造体
struct _GFL_NETSYS{
    GFLNetInitializeStruct aNetInit;  ///< 初期化構造体のコピー
    GFL_NETHANDLE* pNetHandle[GFL_NET_HANDLE_MAX]; ///< 通信ハンドル
    GFL_NETHANDLE* pNetHandleInit;  ///< 初期化用ハンドル
    GFL_NETWL* pNetWL;   ///<  ワイヤレスマネージャーポインタ
};

static GFL_NETSYS* _pNetSys = NULL; // 通信のメモリーを管理するstatic変数

// コンポーネント転送終了の確認用  イクニューモンコンポーネント処理を移動させるときはこれも移動
static volatile u8   startCheck;	

//==============================================================================
/**
 * @brief       netHandleを管理マネージャーに追加
 * @param       pNet      通信システム管理構造体
 * @param       pHandle   通信ハンドル
 * @return      追加した番号
 */
//==============================================================================
static int _addNetHandle(GFL_NETSYS* pNet, GFL_NETHANDLE* pHandle)
{
    int i;

    for(i = 0;i < GFL_NET_HANDLE_MAX; i++){
        if(pNet->pNetHandle[i] == NULL){
            pNet->pNetHandle[i] = pHandle;
            return i;
        }
    }
    OS_TPanic("no handle");
    return 0;
}

//==============================================================================
/**
 * @brief       netHandleの番号を得る
 * @param       pNet      通信システム管理構造体
 * @param       pHandle   通信ハンドル
 * @return      追加した番号
 */
//==============================================================================
static int _numNetHandle(GFL_NETSYS* pNet, GFL_NETHANDLE* pHandle)
{
    int i;

    for(i = 0;i < GFL_NET_HANDLE_MAX; i++){
        if(pNet->pNetHandle[i] == pHandle){
            return i;
        }
    }
    return -1;
}

//==============================================================================
/**
 * @brief       netHandleの中身を消す
 * @param       pNetHandle   通信ハンドル
 * @return      追加した番号
 */
//==============================================================================
static void _deleteHandle(GFL_NETHANDLE* pNetHandle)
{
    if(pNetHandle->pParent){
        GFL_HEAP_FreeMemory(pNetHandle->pParent);
    }
    GFL_NET_TOOL_End(pNetHandle->pTool);
    GFL_HEAP_FreeMemory(pNetHandle);
}

//==============================================================================
/**
 * @brief       netHandleを消す 非公開関数
 * @param       pHandle   通信ハンドル
 * @return      none
 */
//==============================================================================
void GFI_NET_DeleteNetHandle(GFL_NETHANDLE* pHandle)
{
    int i;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    for(i = 0;i < GFL_NET_HANDLE_MAX;i++){
        if(pNet->pNetHandle[i]==pHandle){
            if(pNet->pNetHandleInit == pHandle){
                pNet->pNetHandleInit = NULL;
            }
            _deleteHandle(pNet->pNetHandle[i]);
            pNet->pNetHandle[i] = NULL;
        }
    }
}

//==============================================================================
/**
 * @brief       netHandleを全て消す
 * @param       pNet      通信システム管理構造体
 * @return      none
 */
//==============================================================================
static void _deleteAllNetHandle(GFL_NETSYS* pNet)
{
    int i;

    for(i = 0;i < GFL_NET_HANDLE_MAX;i++){
        if(pNet->pNetHandle[i]!=NULL){
            _deleteHandle(pNet->pNetHandle[i]);
            pNet->pNetHandle[i] = NULL;
        }
    }
}

//==============================================================================
/**
 * @brief       idに沿ったnetHandleを出す  非公開関数
 * @param       netID    id
 * @return      netHandle
 */
//==============================================================================
GFL_NETHANDLE* GFL_NET_GetNetHandle(int netID)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    return pNet->pNetHandle[netID];
}

//==============================================================================
/**
 * @brief       netHandle のstateハンドルを得る 非公開関数
 * @param       pNet      通信システム管理構造体
 * @return      PTRStateFunc
 */
//==============================================================================
PTRStateFunc GFL_NET_GetStateFunc(GFL_NETHANDLE* pHandle)
{
    return pHandle->state;
}

//==============================================================================
/**
 * @brief       自分のネゴシエーションがすんでいるかどうか
 * @param[in]   pHandle   通信ハンドル
 * @return      すんでいる場合TRUE   まだの場合FALSE
 */
//==============================================================================
BOOL GFL_NET_IsNegotiation(GFL_NETHANDLE* pHandle)
{
    if(pHandle){
        return (pHandle->negotiation == _NEGOTIATION_OK);
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief       ネゴシエーション開始を親に送信する
 * @param[in]   pHandle   通信ハンドル
 * @return      送信に成功したらTRUE
 */
//==============================================================================
BOOL GFL_NET_RequestNegotiation(GFL_NETHANDLE* pHandle)
{
    u8 id = GFL_NET_SystemGetCurrentID();
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
//    id = id * 8 + _numNetHandle(pNet, pHandle);
    pHandle->creatureNo = id;

    if(GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID())){
        return GFL_NET_SendData(pHandle, GFL_NET_CMD_NEGOTIATION, &id);
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief       toolのworkポインタを得る 非公開関数
 * @param[in]   pHandle  通信ハンドル
 * @return      NET_TOOLSYSポインタ
 */
//==============================================================================
NET_TOOLSYS* _NETHANDLE_GetTOOLSYS(GFL_NETHANDLE* pHandle)
{
    return pHandle->pTool;
}

//==============================================================================
/**
 * @brief    通信のboot時初期化
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
void GFL_NET_Init(const GFLNetInitializeStruct* pNetInit)
{
    // 通信ヒープ作成
    GFL_HEAP_CreateHeap( pNetInit->baseHeapID, pNetInit->netHeapID, _HEAPSIZE_NET );
    if(pNetInit->bWiFi){
        GFL_HEAP_CreateHeap( pNetInit->baseHeapID, pNetInit->wifiHeapID, _HEAPSIZE_WIFI);
    }
    {
        GFL_NETSYS* pNet = GFL_HEAP_AllocMemory(pNetInit->netHeapID, sizeof(GFL_NETSYS));
        _pNetSys = pNet;

        GFL_STD_MemClear(pNet, sizeof(GFL_NETSYS));

        NET_PRINT("size %d addr %x",sizeof(GFLNetInitializeStruct),(u32)&pNet->aNetInit);

        GFL_STD_MemCopy(pNetInit, &pNet->aNetInit, sizeof(GFLNetInitializeStruct));
        if(pNetInit->bNetwork){
            GFL_NETHANDLE* pNetHandle = GFL_NET_CreateHandle();
            GFL_NET_StateDeviceInitialize(pNetHandle);
            pNet->pNetHandleInit = pNetHandle;
        }

        GFL_NET_COMMAND_Init( pNetInit->recvFuncTable, pNetInit->recvFuncTableNum, pNetInit->pWork);
    }
    WirelessIconEasy(pNetInit->bWiFi, pNetInit->netHeapID);
}

//==============================================================================
/**
 * @brief 通信初期化が完了したかどうかを確認する
 * @retval TRUE   初期化終了
 * @retval FALSE  初期化がまだ終わっていない
 */
//==============================================================================
BOOL GFL_NET_IsInit(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    if(pNet->pNetHandleInit!=NULL){
        return FALSE;
    }
    return TRUE;
}

//==============================================================================
/**
 * @brief  通信終了
 * @retval  TRUE  終了しました
 * @retval  FALSE 終了しません 時間を空けてもう一回呼んでください
 */
//==============================================================================
BOOL GFL_NET_Exit(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    HEAPID netHeapID = pNet->aNetInit.netHeapID;
    HEAPID wifiHeapID = pNet->aNetInit.wifiHeapID;
    BOOL bWiFi = pNet->aNetInit.bWiFi;

    if(GFL_NET_SystemIsInitialize()){
        return FALSE;
    }
    WirelessIconEasyEnd();
    _deleteAllNetHandle(pNet);
    GFL_HEAP_FreeMemory(pNet);
    _pNetSys = NULL;

    if(bWiFi){
        GFL_HEAP_DeleteHeap(wifiHeapID);
    }
    GFL_HEAP_DeleteHeap(netHeapID);
    return TRUE;
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
void GFL_NET_AddBeaconServiceID(GFL_NETHANDLE* pNet, GameServiceID gsid)
{
}

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
 * @return  none
 */
//==============================================================================

void GFL_NET_Main(void)
{
    int i;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    if(pNet==NULL){
        return;
    }
    for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
        if(pNet->pNetHandle[i]!=NULL){
            GFL_NET_StateMainProc(pNet->pNetHandle[i]);  // この内部でhandleを消すことがある
        }
        if(pNet->pNetHandle[i]!=NULL){
            GFL_NET_TOOL_TimingSyncSend(pNet->pNetHandle[i]);
            GFL_NET_StateTransmissonMain(pNet->pNetHandle[i]);
        }
    }
    GFL_NET_SystemUpdateData();
    
}

//==============================================================================
/**
 * @brief   通信ハンドルを作る   通信一個単位のワークエリアの事
 * @return  GFL_NETHANDLE  通信ハンドルのポインタ
 */
//==============================================================================
GFL_NETHANDLE* GFL_NET_CreateHandle(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    GFL_NETHANDLE* pHandle = GFL_HEAP_AllocMemory(pNet->aNetInit.netHeapID, sizeof(GFL_NETHANDLE));
    GFL_STD_MemClear(pHandle, sizeof(GFL_NETHANDLE));
    _addNetHandle(pNet, pHandle);
    pHandle->pTool = GFL_NET_TOOL_Init(pNet->aNetInit.netHeapID, pNet->aNetInit.maxConnectNum);
    return pHandle;
}


//==============================================================================
/**
 * @brief   子機になり指定した親機に接続する
 * @param   GFL_NETHANDLE  通信ハンドルのポインタ
 * @param   macAddress     マックアドレスのバッファ
 * @return  none
 */
//==============================================================================
void GFL_NET_InitClientAndConnectToParent(GFL_NETHANDLE* pHandle,u8* macAddress)
{
    GFL_STD_MemCopy(macAddress, pHandle->aMacAddress, sizeof(pHandle->aMacAddress));
    GFL_NET_StateConnectMacAddress(pHandle, TRUE);
}

//==============================================================================
/**
 * @brief   指定した親機に接続する
 * @param   GFL_NETHANDLE  通信ハンドルのポインタ
 * @param   macAddress     マックアドレスのバッファ
 * @return  none
 */
//==============================================================================
void GFL_NET_ConnectToParent(GFL_NETHANDLE* pHandle,u8* macAddress)
{
    GFL_STD_MemCopy(macAddress, pHandle->aMacAddress, sizeof(pHandle->aMacAddress));
    GFL_NET_StateConnectMacAddress(pHandle, FALSE);
}

//==============================================================================
/**
 * @brief   子機になりビーコンを集める
 * @param   GFL_NETHANDLE  通信ハンドルのポインタ
 * @return  none
 */
//==============================================================================
void GFL_NET_StartBeaconScan(GFL_NETHANDLE* pHandle)
{
    GFL_NET_StateBeaconScan(pHandle);
}

//==============================================================================
/**
 * @brief    親機になり待ち受ける
 * @param    GFL_NETHANDLE  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
void GFL_NET_InitServer(GFL_NETHANDLE* pHandle)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

#if GFL_NET_WIFI
    if(pNet->aNetInit.bWiFi){
        GFL_NET_StateConnectWifiParent(pHandle, pNet->aNetInit.netHeapID);
    }
    else{
        GFL_NET_StateConnectParent(pHandle, pNet->aNetInit.netHeapID);
    }
#else
    GFL_NET_StateConnectParent(pHandle, pNet->aNetInit.netHeapID);
#endif
}

//==============================================================================
/**
 * @brief    親機子機を繰り返し、誰でもいいので接続する
 * @param    GFL_NETHANDLE  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
void GFL_NET_ChangeoverConnect(GFL_NETHANDLE* pHandle)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    GFL_NET_StateChangeoverConnect(pHandle, pNet->aNetInit.netHeapID);
}

//==============================================================================
/**
 * @brief    Wi-Fiロビーへ接続する
 * @param    GFL_NETHANDLE        通信ハンドルのポインタ
 * @param    GFL_WIFI_FRIENDLIST  フレンドリスト構造体のポインタ
 * @return   none
 */
//==============================================================================

void GFL_NET_WifiLogin(GFL_NETHANDLE* pHandle)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    GFL_NET_StateWifiEnterLogin(pHandle,
                                pNet->aNetInit.netHeapID, pNet->aNetInit.wifiHeapID);


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
    return pNetHandle->creatureNo;
}

//==============================================================================
/**
 * @brief 現在の接続台数を得る
 * @retval  int  接続数
 */
//==============================================================================
int GFL_NET_GetConnectNum(void)
{
    return GFL_NET_SystemGetConnectNum();
}

//==============================================================================
/**
 * @brief IDの通信と接続しているかどうかを返す
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @param   NetID id            ネットID
 * @retval  BOOL  接続していたらTRUE
 */
//==============================================================================
BOOL GFL_NET_IsConnectMember(GFL_NETHANDLE* pNet,NetID id)
{
    return pNet->negotiationID[id];
}

//==============================================================================
/**
 * @brief   ネゴシエーション済み人数を得る
 * @param[in,out]   NetHandle* pNet     通信ハンドルのポインタ
 * @return   人数
 */
//==============================================================================
int GFL_NET_GetNegotiationConnectNum(GFL_NETHANDLE* pNet)
{
    int j=0,i;

    for(i = 0; i < GFL_NET_HANDLE_MAX ; i++){
        if( pNet->negotiationID[i] ){
            j++;
        }
    }
    return j;
}

//==============================================================================
/**
 * @brief 通信切断する
 * @retval  none
 */
//==============================================================================
void GFL_NET_Disconnect(void)
{
    int i,userNo = -1;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    if(pNet){
        for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
            if(pNet->pNetHandle[i]){
                //            GFL_NET_SendData(pNet->pNetHandle[i],GFL_NET_CMD_EXIT,NULL); ///終了を全員に送信
                userNo = i;
            }
        }
    }
    if(userNo != -1){
        GFL_NET_StateExit(pNet->pNetHandle[userNo]);
    }
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

        
        WirelessIconEasy(pNet->aNetInit.bWiFi, pNet->aNetInit.netHeapID);
    }
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
    return TRUE;
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
                                  FALSE, pNet->creatureNo ,GFL_NET_SENDID_ALLUSER);
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
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
BOOL GFL_NET_SendDataEx(GFL_NETHANDLE* pNet,const NetID sendID,const u16 sendCommand, const u32 size,const void* data, const BOOL bFast, const BOOL bRepeat)
{
    if(bRepeat && !GFL_NET_SystemIsSendCommand(sendCommand,pNet->creatureNo)){
        return GFL_NET_SystemSendData(sendCommand, data, size, bFast, pNet->creatureNo ,sendID);
    }
    else if(bRepeat){
        return FALSE;
    }
    return GFL_NET_SystemSendData(sendCommand, data, size, bFast, pNet->creatureNo ,sendID);
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
    return TRUE;
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
    GFL_NET_TOOL_TimingSyncStart(pNet,no);
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
    return GFL_NET_TOOL_IsTimingSync(pNet, no);
}

//==============================================================================
/**
 * @brief     DSモードに変更する
 * @param     NetHandle* pNet  通信ハンドル
 * @return    none
 */
//==============================================================================
void GFL_NET_ChangeDsMode(GFL_NETHANDLE* pNet)
{
    u8 bDSMode = TRUE;
    GFL_NET_SendData(pNet, GFL_NET_CMD_DSMP_CHANGE, &bDSMode);
}

//==============================================================================
/**
 * @brief     MPモードに変更する
 * @param     NetHandle* pNet  通信ハンドル
 * @return    none
 */
//==============================================================================
void GFL_NET_ChangeMpMode(GFL_NETHANDLE* pNet)
{
    u8 bDSMode = FALSE;
    GFL_NET_SendData(pNet, GFL_NET_CMD_DSMP_CHANGE, &bDSMode);
}

//==============================================================================
/**
 * @brief     割り込み中に行う処理を実行
 * @param     none
 * @return    none
 */
//==============================================================================
void GFL_NET_VBlankFunc(void)
{
    WirelessIconEasyFunc();
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
BOOL GFL_NET_IsWifiLobby(GFL_NETHANDLE* pNetHandle)
{
    if(2 > GFL_NET_StateWifiIsMatched(pNetHandle)){
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
BOOL GFL_NET_StartRandomMatch(GFL_NETHANDLE* pNetHandle)
{
    return GFL_NET_StateStartWifiRandomMatch( pNetHandle );
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
    GF_ASSERT(pNet->aNetInit.maxConnectNum < GFL_NET_MACHINE_MAX);
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
    GF_ASSERT(pNet->aNetInit.maxSendSize < ((WM_SIZE_MP_DATA_MAX-4)/pNet->aNetInit.maxConnectNum));
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

void GFI_NET_FatalErrorFunc(GFL_NETHANDLE* pNetHandle,int errorNo)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    if(pNet->aNetInit.errorFunc != NULL){
        pNet->aNetInit.errorFunc(pNetHandle, errorNo);
    }
}

//==============================================================================
/**
 * @brief    SSIDを得る関数
 * @return   SSID
 */
//==============================================================================

u8* GFI_NET_GetSSID(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    if(pNet->aNetInit.getSSID != NULL){
        return pNet->aNetInit.getSSID();
    }
    return NULL;
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
void GFL_NET_SetAutoErrorCheck(GFL_NETHANDLE* pNetHandle,BOOL bAuto)
{
    GFL_NET_STATE_SetAutoErrorCheck(pNetHandle, bAuto);
}

//==============================================================================
/**
 * @brief   子機がいない場合にエラーにするかどうかを設定する
 * @param   bFlg    切断=エラーにする
 * @param   bAuto  TRUEで検査開始
 * @retval  none
 */
//==============================================================================
void GFL_NET_SetNoChildErrorCheck(GFL_NETHANDLE* pNetHandle,BOOL bFlg)
{
    GFL_NET_STATE_SetNoChildErrorCheck(pNetHandle, bFlg);
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
    if (result != WVR_RESULT_SUCCESS) {
        OS_TPanic("WVR_StartUpAsync error.[%08xh]\n", result);
    }
    else{
		NET_PRINT("WVR Trans VRAM-D.\n");
	}
    startCheck = 2;
}

//==============================================================================
/**
 * @brief    イクニューモンライブラリのみを初期化する
 * @return   none
 */
//==============================================================================

void GFL_NET_InitIchneumon(void)
{
    int ans;
    //************************************


    GFL_UI_SleepDisable(GFL_UI_SLEEP_NET);  // スリープ禁止
    // 無線ライブラリ駆動開始
	// イクニューモンコンポーネントをVRAM-Dに転送する
    startCheck = 1;
    ans = WVR_StartUpAsync(GX_VRAM_ARM7_128_D, _startUpCallback, NULL);
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
 * @brief    イクニューモンライブラリの初期化が終わったかどうか
 * @retval   TREU   終了
 * @retval   FALSE  まだ終わっていない
 */
//==============================================================================

BOOL GFL_NET_IsInitIchneumon(void)
{
    return (startCheck==2);
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
    startCheck = 0;
    GFL_UI_SleepEnable(GFL_UI_SLEEP_NET);  // スリープ許可
}

//==============================================================================
/**
 * @brief    イクニューモンライブラリの終了をしらせる
 * @retval   none
 */
//==============================================================================


void GFL_NET_ExitIchneumon(void)
{
    NET_PRINT("VRAMD Finalize\n");
    WVR_TerminateAsync(_endCallback,NULL);  // イクニューモン切断
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

