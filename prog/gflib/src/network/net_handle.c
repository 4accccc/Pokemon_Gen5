//=============================================================================
/**
 * @file	net_handle.c
 * @brief	GFL通信ライブラリー  通信ハンドル
 * @author	GAME FREAK Inc.  k.ohno
 * @date    2008.2.26
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

#define _NEGOTIATION_START_TIME (10)

//==============================================================================
/**
 * @brief       netHandleの番号を得る
 * @param       pNet      通信システム管理構造体
 * @param       pHandle   通信ハンドル
 * @return      追加した番号
 */
//==============================================================================
int GFL_NET_HANDLE_GetNetHandleID(GFL_NETHANDLE* pHandle)
{
    int i;
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();

    for(i = 0;i < GFL_NET_HANDLE_MAX; i++){
        if(&pNet->aNetHandle[i] == pHandle){
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
    GFL_STD_MemClear(pNetHandle, sizeof(GFL_NETHANDLE));
}

//==============================================================================
/**
 * @brief   通信ハンドルを初期化する
 * @param   netID    通信ID
 * @return  GFL_NETHANDLE  通信ハンドルのポインタ
 */
//==============================================================================
GFL_NETHANDLE* GFL_NET_InitHandle(int netID)
{
    GFL_NETSYS* pNet;
    GFL_NETHANDLE* pHandle;
    GF_ASSERT(netID < GFL_NET_MACHINE_MAX);

    pNet = _GFL_NET_GetNETSYS();
    pHandle = &pNet->aNetHandle[netID+1];  //ハンドルの０は親 1-は子

    GFL_STD_MemClear(pHandle, sizeof(GFL_NETHANDLE));
    pHandle->negotiationType = _NEG_TYPE_FIRST;
    pHandle->timingSyncEnd = 0xff;
    pHandle->timingSyncMy = 0xff;
    pHandle->timingSendFlg = FALSE;
    return pHandle;
}

//==============================================================================
/**
 * @brief       netHandleを全て消す
 * @param       pNet      通信システム管理構造体
 * @return      none
 */
//==============================================================================
void GFL_NET_HANDLE_DeleteAll(GFL_NETSYS* pNet)
{
    int i;

    for(i = 0;i < GFL_NET_HANDLE_MAX;i++){
        _deleteHandle(&pNet->aNetHandle[i]);
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
    return &pNet->aNetHandle[netID];
}

//==============================================================================
/**
 * @brief   自分のマシンのハンドルを帰す
 * @retval  GFL_NETHANDLE
 */
//==============================================================================

GFL_NETHANDLE* GFL_NET_HANDLE_GetCurrentHandle(void)
{
    return GFL_NET_GetNetHandle(GFL_NET_SystemGetCurrentID()+1);
}

//==============================================================================
/**
 * @brief       自分のネゴシエーションがすんでいるかどうか
 * @param[in]   pHandle   通信ハンドル
 * @return      すんでいる場合TRUE   まだの場合FALSE
 */
//==============================================================================
BOOL GFL_NET_HANDLE_IsNegotiation(GFL_NETHANDLE* pHandle)
{
    if(pHandle){
        return (pHandle->negotiationType == _NEG_TYPE_CONNECT);
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief       ネゴシエーションが完了しているのが何人か
 * @return      すんでいる人数 
 */
//==============================================================================
int GFL_NET_HANDLE_GetNumNegotiation(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    int i,num=0;

    for(i = 1;i < GFL_NET_HANDLE_MAX;i++){
        if(GFL_NET_HANDLE_IsNegotiation(&pNet->aNetHandle[i])){
            num++;
        }
    }
    return num;
}

//==============================================================================
/**
 * @brief       ネゴシエーション開始を親に送信する
 * @param[in]   pHandle   通信ハンドル
 * @return      送信に成功したらTRUE  失敗したらもう一回送信
 */
//==============================================================================
BOOL GFL_NET_HANDLE_RequestNegotiation(void)
{
    GFL_NETHANDLE* pHandle = GFL_NET_HANDLE_GetCurrentHandle();
    u8 id = GFL_NET_HANDLE_GetNetHandleID(pHandle);
    GFL_NETSYS* pNetSys = _GFL_NET_GetNETSYS();

    if(GFL_NET_SystemIsConnect(GFL_NET_SystemGetCurrentID())){  // 物理的な接続
        if(pHandle->negotiationType == _NEG_TYPE_SEND){
            OS_TPrintf("GFL_NET_CMD_NEGOTIATION %d\n",id);
            return GFL_NET_SendData(pHandle, GFL_NET_CMD_NEGOTIATION, &id); //NETIDを送る
        }
        else if(pHandle->negotiationType < _NEG_TYPE_SEND){
            pHandle->negotiationType++;
        }
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief  タイミングコマンドを受信した   GFL_NET_CMD_TIMING_SYNC
 * @param[in]   netID  通信発信者ID
 * @param[in]   size   受信データサイズ
 * @param[in]   pData  受信データポインタ
 * @param[in,out]   pWork  使用者が必要なworkのポインタ
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @retval      none
 */
//==============================================================================

void GFL_NET_HANDLE_RecvTimingSync(const int netID, const int size, const void* pData,
                                void* pWork, GFL_NETHANDLE* pNet)
{
    const u8* pBuff = (const u8*)pData;
    u8 syncNo = pBuff[0];
    int i;

    if(GFL_NET_HANDLE_GetNetHandleID(pNet) == GFL_NET_PARENT_NETID){
        pNet->timing[netID] = syncNo;       ///< 子機からの受け取り記録

        NET_PRINT("同期受信 %d %d \n",netID,syncNo);

        for(i = 1; i < GFL_NET_HANDLE_MAX; i++){
            if(GFL_NET_HANDLE_IsNegotiation(GFL_NET_GetNetHandle(i))){
                if(pNet->timing[i] != syncNo){
                    NET_PRINT("同期してない%d %d\n", i, syncNo);
                    pNet->timingSendFlg = FALSE;
                    return;
                }
            }
        }
        NET_PRINT("メンバ確認完了\n");
        pNet->timingSendFlg = TRUE;
        pNet->timingSyncMy = syncNo;
    }
}


//==============================================================================
/**
 * @brief   タイミングコマンドENDを受信した   GFL_NET_CMD_TIMING_SYNC_END
 * @param[in]   netID  通信発信者ID
 * @param[in]   size   受信データサイズ
 * @param[in]   pData  受信データポインタ
 * @param[in,out]   pWork  使用者が必要なworkのポインタ
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @retval      none
 */
//==============================================================================

void GFL_NET_HANDLE_RecvTimingSyncEnd(const int netID, const int size, const void* pData,
                                 void* pWork, GFL_NETHANDLE* pNet)
{
    const u8* pBuff = (const u8*)pData;
    u8 syncNo = pBuff[0];

    NET_PRINT("全員同期コマンド受信 %d %d\n", netID, syncNo);
    pNet->timingSyncEnd = syncNo;     // 同期コマンド用
}

//==============================================================================
/**
 * @brief   タイミングコマンドを発行する
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================

void GFL_NET_HANDLE_TimingSyncStart(GFL_NETHANDLE* pNet, const u8 no)
{
    NET_PRINT("SyncStart %d \n",no);
    pNet->timingSyncMy = no;
    if(GFL_NET_HANDLE_GetNetHandleID(pNet) == GFL_NET_PARENT_NETID){
        pNet->timingSendFlg = FALSE;  // 親はみんなから集まってから送信
    }
    else{
        pNet->timingSendFlg = TRUE;  // 子はすぐに送信
    }
}

//==============================================================================
/**
 * @brief   タイミングコマンドが届いたかどうかを確認する
 * @param   pNet   ネットワークハンドル
 * @param   no     検査する番号
 * @retval  TRUE   届いていた
 * @retval  FALSE  届いていない
 */
//==============================================================================

BOOL GFL_NET_HANDLE_IsTimingSync(const GFL_NETHANDLE* pNet, const u8 no)
{
    if(pNet->timingSyncEnd == no){
        NET_PRINT("確認 %d\n",pNet->timingSyncEnd);
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   ハンドル処理のメイン
 * @retval  none
 */
//==============================================================================

void GFL_NET_HANDLE_MainProc(void)
{
    int i,num=0;
    GFL_NETHANDLE* pNet;
    
    for(i = 0;i < GFL_NET_HANDLE_MAX;i++){
        pNet = GFL_NET_GetNetHandle(i);
        if(FALSE == GFL_NET_IsSendEnable(pNet)){
            continue;
        }
        if(pNet->timingSendFlg){
            if(i == GFL_NET_PARENT_NETID){
                if(GFL_NET_SendData(pNet, GFL_NET_CMD_TIMING_SYNC_END, &pNet->timingSyncMy)){
                    NET_PRINT("同期終了を送信\n");
                    pNet->timingSendFlg = FALSE;
                 }
            }
            else{
                if(GFL_NET_SendData(pNet, GFL_NET_CMD_TIMING_SYNC, &pNet->timingSyncMy)){
                    NET_PRINT("TIMING_SYNC %d\n",i);
                    pNet->timingSendFlg = FALSE;
                }
            }
        }
    }
}

//==============================================================================
/**
 * @brief   ネゴシエーション用コールバック GFL_NET_CMD_NEGOTIATION
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================

void GFL_NET_HANDLE_RecvNegotiation(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    // 親機が受け取ってフラグを立てる
    u8 retCmd[2];
    const u8 *pGet = (const u8*)pData;
    int i;


    NET_PRINT("RecvNegotiation %d \n",netID);
    
    retCmd[1] = pGet[0];
    retCmd[0] = netID;

    pNetHandle->negotiationSendFlg = FALSE;
    if(GFL_NET_GetNetHandle(GFL_NET_PARENT_NETID) == pNetHandle){
        if(pNetHandle->negotiationBit == 0){
            GFL_NET_SendData(pNetHandle, GFL_NET_CMD_NEGOTIATION_RETURN, retCmd);
        }
    }
}

//==============================================================================
/**
 * @brief   ネゴシエーション用コールバック GFL_NET_CMD_NEGOTIATION_RETURN
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================

void GFL_NET_HANDLE_RecvNegotiationReturn(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const u8* pMsg = pData;

    if(GFL_NET_GetNetHandle(GFL_NET_PARENT_NETID) == pNetHandle){
        pNetHandle->negotiationType = _NEG_TYPE_CONNECT;
        return;
    }
    else{
        pNetHandle = GFL_NET_GetNetHandle(pMsg[0]);
        pNetHandle->negotiationType = _NEG_TYPE_CONNECT;
        OS_TPrintf("接続認証 OK %d\n",pMsg[0]);
    }
}

