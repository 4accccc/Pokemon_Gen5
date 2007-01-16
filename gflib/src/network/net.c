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

#include "net.h"
#include "net_def.h"
#include "device/net_wireless.h"
#include "net_system.h"
#include "net_command.h"
#include "net_state.h"

#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "tool/net_tool.h"


struct _GFL_NETSYS{
    GFLNetInitializeStruct aNetInit;  ///< 初期化構造体
    GFL_NETHANDLE* pNetHandle[GFL_NET_MACHINE_MAX];

    GFL_NETWL* pNetWL;
    
    int heapID;  ///< 通信の確保メモリーの場所
    
} ;


static GFL_NETSYS* _pNetSys = NULL; // 通信のメモリーを管理するstatic変数






static int _addNetHandle(GFL_NETSYS* pNet, GFL_NETHANDLE* pHandle)
{
    int i;

    for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
        if(pNet->pNetHandle[i] == NULL){
            pNet->pNetHandle[i] = pHandle;
            return i;
        }
    }
    OS_TPanic("no handle");
    return 0;
}

void GFL_NET_DeleteNetHandle(GFL_NETHANDLE* pHandle)
{
    int i;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();


    
    for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
        if(pNet->pNetHandle[i]==pHandle){
            GFL_HEAP_FreeMemory(pNet->pNetHandle[i]->pTool);
            GFL_HEAP_FreeMemory(pNet->pNetHandle[i]);
            pNet->pNetHandle[i] = NULL;
        }
    }
}

static void _deleteAllNetHandle(GFL_NETSYS* pNet)
{
    int i;

    for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
        if(pNet->pNetHandle[i]!=NULL){
            GFL_HEAP_FreeMemory(pNet->pNetHandle[i]->pTool);
            GFL_HEAP_FreeMemory(pNet->pNetHandle[i]);
            pNet->pNetHandle[i] = NULL;
        }
    }
}

GFL_NETHANDLE* GFL_NET_GetNetHandle(int netID)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    return pNet->pNetHandle[netID];
}

PTRStateFunc GFL_NET_GetStateFunc(GFL_NETHANDLE* pHandle)
{
    return pHandle->state;
}

//==============================================================================
/**
 * @brief       自分のネゴシエーションがすんでいるかどうか
 * @param[in]   pHandle   通信ハンドル
 * @param[in]   netID     ネットID
 * @return      none
 */
//==============================================================================
BOOL GFL_NET_IsNegotiation(GFL_NETHANDLE* pHandle)
{
    if(pHandle){
        return (pHandle->negotiation == _NEGOTIATION_OK);
    }
    return _NEGOTIATION_NG;
}

//==============================================================================
/**
 * @brief       toolのworkポインタを得る
 * @param[in]   pNetInit  通信初期化構造体のポインタ
 * @param[in]   netID     ネットID
 * @return      none
 */
//==============================================================================
NET_TOOLSYS* _NETHANDLE_GetTOOLSYS(GFL_NETHANDLE* pHandle)
{
    return pHandle->pTool;
}

//==============================================================================
/**
 * @brief 通信初期化
 * @param[in]   pNetInit  通信初期化構造体のポインタ
 * @param[in]   netID     ネットID
 * @return none
 */
//==============================================================================
void GFL_NET_Initialize(const GFLNetInitializeStruct* pNetInit,int heapID)
{
    GFL_NETSYS* pNet = GFL_HEAP_AllocMemory(heapID, sizeof(GFL_NETSYS));
    _pNetSys = pNet;

    GFL_STD_MemClear(pNet, sizeof(GFL_NETSYS));

    OS_TPrintf("size %d addr %x",sizeof(GFLNetInitializeStruct),(u32)&pNet->aNetInit);

    GFL_STD_MemCopy(pNetInit, &pNet->aNetInit, sizeof(GFLNetInitializeStruct));
    pNet->heapID = heapID;
    if(pNetInit->bNetwork){
        GFL_NETHANDLE* pNetHandle = GFL_NET_CreateHandle();
        GFL_NET_StateDeviceInitialize(pNetHandle);
    }
}

//==============================================================================
/**
 * @brief  通信終了
 * @retval  TRUE  終了しました
 * @retval  FALSE 終了しません 時間を空けてもう一回呼んでください
 */
//==============================================================================
BOOL GFL_NET_Finalize(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    _deleteAllNetHandle(pNet);
    GFL_HEAP_FreeMemory(pNet);
    _pNetSys = NULL;
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
 * @brief 受信ビーコンデータを得る
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
 * @brief 受信ビーコンに対応したMACアドレスを得る
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
 * @brief   通信ハンドル分MainStatusをまわす
 * @return  GFL_NETHANDLE  通信ハンドルのポインタ
 */
//==============================================================================

void GFL_NET_MainProc(void)
{
    int i;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
        if(pNet->pNetHandle[i]!=NULL){
            GFL_NET_StateMainProc(pNet->pNetHandle[i]);
        }
        if(pNet->pNetHandle[i]!=NULL){
            GFL_NET_ToolTimingSyncSend(pNet->pNetHandle[i]);
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

    GFL_NETHANDLE* pHandle = GFL_HEAP_AllocMemory(pNet->heapID, sizeof(GFL_NETHANDLE));
    GFL_STD_MemClear(pHandle, sizeof(GFL_NETHANDLE));
    _addNetHandle(pNet, pHandle);
    pHandle->pTool = GFL_NET_Tool_sysInit(pNet->heapID, pNet->aNetInit.maxConnectNum);
    return pHandle;
}


//==============================================================================
/**
 * @brief 子機になり指定した親機に接続する
 * @param   GFL_NETHANDLE  通信ハンドルのポインタ
 * @param   macAddress     マックアドレスのバッファ
 * @return  none
 */
//==============================================================================
void GFL_NET_ClientConnectTo(GFL_NETHANDLE* pHandle,u8* macAddress)
{
    GFL_STD_MemCopy(macAddress, pHandle->aMacAddress, 6);
    GFL_STD_MemCopy(macAddress, pHandle->aMacAddress, sizeof(pHandle->aMacAddress));
    GFL_NET_StateConnectMacAddress(pHandle);
}

//==============================================================================
/**
 * @brief    親機になり待ち受ける
 * @param    GFL_NETHANDLE  通信ハンドルのポインタ
 * @return   none
 */
//==============================================================================
void GFL_NET_ServerConnect(GFL_NETHANDLE* pHandle)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    GFL_NET_StateConnectParent(pHandle, pNet->heapID);
}

//==============================================================================
/**
 * @brief 親機になり指定された子機の接続を待ち受ける
 * @param    GFL_NETHANDLE  通信ハンドルのポインタ
 * @param   macAddress     マックアドレスのバッファ
 * @param   num            子機人数
 * @return  none
 */
//==============================================================================
void GFL_NET_ServerConnectTo(GFL_NETHANDLE* pHandle,const u8* macAddress, const int num)
{
//    GFL_NET_StateConnectParentMacAddress(pHandle, macAddress, num);
}

//==============================================================================
/**
 * @brief 親子切り替え接続を行う
 * @param    GFL_NETHANDLE  通信ハンドルのポインタ
 * @return  none
 */
//==============================================================================
void GFL_NET_SwitchConnect(GFL_NETHANDLE* pHandle)
{
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
    int i;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    for(i = 0; i < GFL_NET_MACHINE_MAX;i++){
        if(pNet->pNetHandle[i] == pNetHandle){
            return i;
        }
    }
    OS_TPanic("no handle");
    return -1;//
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
 * @brief 通信切断する
 * @retval  none
 */
//==============================================================================
//
void GFL_NET_Disconnect(void)
{
    int i,userNo = 0;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
        if(pNet->pNetHandle[i]){
            GFL_NET_SendData(pNet->pNetHandle[i],GFL_NET_CMD_EXIT,NULL); ///終了を全員に送信
            userNo = i;
        }
    }
    GFL_NET_StateExit(pNet->pNetHandle[userNo]);
}

//==============================================================================
/**
 * @brief   接続台数変更
 * @param   NetHandle* pNet     通信ハンドルのポインタ
 * @param   u8 num     変更数
 * @retval  none
 */
//==============================================================================
//void GFL_NET_SetClientConnectNum(GFL_NETHANDLE* pNet,u8 num)
//{
//    
//}

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

    GFL_NET_SystemSendData(sendCommand,data,0);


    return TRUE;
}

//==============================================================================
/**
 * @brief 送信開始
 * @param[in,out]  pNet  通信ハンドル
 * @param[in]   sendID                     送信相手 全員へ送信する場合 NET_SENDID_ALLUSER
 * @param[in]   sendCommand                送信するコマンド
 * @param[in]   pCBSendEndFunc  送信完了をつたえるコールバック関数の登録
 * @param[in]   size                       送信データサイズ
 * @param[in]   data                     送信データポインタ
 * @param[in]   bDataCopy                 データをコピーする場合TRUE
 * @retval  TRUE   成功した
 * @retval  FALSE  失敗の場合
 */
//==============================================================================
BOOL GFL_NET_SendDataEx(GFL_NETHANDLE* pNet,const NetID sendID,const u16 sendCommand,
                    const CBSendEndFunc* pCBSendEndFunc,const u32 size,
                    const void* data, const BOOL bDataCopy)
{
    return TRUE;
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

//==============================================================================
/**
 * @brief   毎フレーム送りたいデータを登録する
 * @param[in,out]   NetHandle* pNet  通信ハンドル
 * @param   pGet      データ取得関数
 * @param   pRecv     受信関数
 * @param   size      サイズ
 * @return  none
 */
//==============================================================================
void GFL_NET_SetEveryTimeSendData(GFL_NETHANDLE* pNet, CBGetEveryTimeData* pGet, CBRecvEveryTimeData* pRecv,const int size)
{
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
    return TRUE;
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

