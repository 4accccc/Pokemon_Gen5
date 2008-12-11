//=============================================================================
/**
 * @file	net_handle.c
 * @brief	GFL通信ライブラリー  通信ハンドル
 * @author	GAME FREAK Inc.  k.ohno
 * @date    2008.2.26
 */
//=============================================================================


#include "net.h"
#include "net_def.h"
#include "net_local.h"
#include "net_system.h"
#include "net_command.h"

#include "tool/net_ring_buff.h"
#include "tool/net_queue.h"
#include "net_handle.h"

#define _NEGOTIATION_START_TIME (10)


static u16 _setNegotiationBit(u16 checkbit,const int netID,const BOOL onoff);
static BOOL _getNegotiationBit(const u16 checkbit,const int netID);


//==============================================================================
//	構造体定義
//==============================================================================
///ネゴシエーション開始を親に送信する時に送るデータ
typedef struct{
	u8 MacAddress[6];		///<送信者のMACアドレス
	u8 netid;				///<送信者のnetid
	u8 padding;				//パディング
}NEGOTIATION_REQ_DATA;

///ネゴシエーション受信を親から送られてくるデータ
typedef struct{
	u8 parent_MacAddress[6];	///<親のMACアドレス
	u8 child_MacAddress[6];		///<ネゴシエーションを送信してきた子のMACアドレス
	u8 child_netid;				///<ネゴシエーションを送信してきた子のnetid
    u8 bInit;                   ///<初期化したかどうか
    u8 padding[2];				//パディング
}NEGOTIATION_RETURN_DATA;

//==============================================================================
/**
 * @brief       indexからnetidへの結びつきに変換
 * @param       pNet      通信システム管理構造体
 * @param       pHandle   通信ハンドル
 * @return      追加した番号
 */
//==============================================================================
static int _indexToNetID(int index)
{
    if(index >= GFL_NET_MACHINE_MAX){
        return GFL_NET_NETID_SERVER;
    }
    return index;
}

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

    if(GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER) == pHandle){
        return GFL_NET_NETID_SERVER;
    }
    for(i = 0;i < GFL_NET_HANDLE_MAX; i++){
        if(GFL_NET_GetNetHandle(i) == pHandle){
            return i;
        }
    }
    GF_ASSERT(0);
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
 * @param   netID    通信ID 0-15 + GFL_NET_NETID_SERVER
 * @return  GFL_NETHANDLE  通信ハンドルのポインタ
 */
//==============================================================================
GFL_NETHANDLE* GFL_NET_InitHandle(int netID)
{
    GFL_NETSYS* pNet;
    GFL_NETHANDLE* pHandle;
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();

    pNet = _GFL_NET_GetNETSYS();
    pHandle = GFL_NET_GetNetHandle(netID);
    if(netID == GFL_NET_NETID_SERVER){
        int i;
        for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
            if(pHandle->negoBuff[i]!=NULL){
                GFL_HEAP_FreeMemory(pHandle->negoBuff[i]);
            }
        }
    }
    GFL_STD_MemClear(pHandle, sizeof(GFL_NETHANDLE));
    if(netID == GFL_NET_NETID_SERVER){
        int i;
        for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
            pHandle->negoBuff[i] = GFL_HEAP_AllocClearMemory(pNetIni->netHeapID ,sizeof(NEGOTIATION_RETURN_DATA));
        }
    }
    pHandle->negotiationType = _NEG_TYPE_FIRST;
    pHandle->timingSyncEnd = 0xff;
    pHandle->timingSyncMy = 0xff;
    pHandle->timingSendFlg = FALSE;
    return pHandle;
}

//==============================================================================
/**
 * @brief       netHandleを全て初期化する
 * @param       pNet      通信システム管理構造体
 * @return      none
 */
//==============================================================================
void GFI_NET_HANDLE_InitAll(GFL_NETSYS* pNet)
{
    int netID;

    for(netID = 0;netID < GFL_NET_MACHINE_MAX;netID++){
        GFL_NET_InitHandle(netID);
    }
    GFL_NET_InitHandle(GFL_NET_NETID_SERVER);
}

//==============================================================================
/**
 * @brief       netHandleを全て消す
 * @param       pNet      通信システム管理構造体
 * @return      none
 */
//==============================================================================
void GFI_NET_HANDLE_DeleteAll(GFL_NETSYS* pNet)
{
    GFL_NETHANDLE* pHandle;
    int i;

    for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
        _deleteHandle(GFL_NET_GetNetHandle(i));
    }
    pHandle=GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);
    for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
        if(pHandle->negoBuff[i]!=NULL){
            GFL_HEAP_FreeMemory(pHandle->negoBuff[i]);
            pHandle->negoBuff[i]=NULL;
        }
    }
    _deleteHandle(GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER));
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
    if(GFL_NET_NETID_SERVER == netID){
        return &pNet->aNetHandle[GFL_NET_HANDLE_MAX-1];
    }
    GF_ASSERT(netID < GFL_NET_HANDLE_MAX);
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
    return GFL_NET_GetNetHandle(GFL_NET_SystemGetCurrentID());
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
 * @brief       ネゴシエーションが完了しているのが何人か SERVERは数えない
 * @return      すんでいる人数 
 */
//==============================================================================
int GFL_NET_HANDLE_GetNumNegotiation(void)
{
    GFL_NETSYS* pNet = _GFL_NET_GetNETSYS();
    int i,num=0;

    if(pNet){
        for(i = 0;i < GFL_NET_MACHINE_MAX;i++){
            if(GFL_NET_HANDLE_IsNegotiation(GFL_NET_GetNetHandle(i))){
                num++;
            }
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
			NEGOTIATION_REQ_DATA send_data;
			
            OS_TPrintf("GFL_NET_CMD_NEGOTIATION %d\n",id);
            OS_GetMacAddress(send_data.MacAddress);
            send_data.netid = id;
            return GFL_NET_SendData(pHandle, GFL_NET_CMD_NEGOTIATION, sizeof(NEGOTIATION_REQ_DATA), &send_data); //NETIDを送る
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

    if(GFL_NET_HANDLE_GetNetHandleID(pNet) == GFL_NET_NETID_SERVER){
        pNet->timingR[netID] = syncNo;       ///< 子機からの受け取り記録

        NET_PRINT("同期受信 %d %d \n",netID,syncNo);

        for(i = 0; i < GFL_NET_MACHINE_MAX; i++){  // 子機の部分のハンドルを検査
            if(GFL_NET_HANDLE_IsNegotiation(GFL_NET_GetNetHandle(i))){
                if(pNet->timingR[i] != syncNo){
                    NET_PRINT("同期してない%d %d\n", i, syncNo);
                    pNet->timingSendFlg = FALSE;
                    return;
                }
            }
            else if(GFL_NET_SystemIsConnect(i) == TRUE){
				OS_TPrintf("ネゴシエーションが済んでいない %d %d\n", i, syncNo);
				return;
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

    NET_PRINT("全員同期コマンド受信 %d %d %d\n", netID, syncNo,GFL_NET_HANDLE_GetNetHandleID(pNet));
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
    if(GFL_NET_HANDLE_GetNetHandleID(pNet) == GFL_NET_NETID_SERVER){
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
 * @brief   親から子機人数分のネゴシエーションを返す
 * @retval  none
 */
//==============================================================================

static BOOL _sendParentNegotiation(GFL_NETHANDLE* pNetHandle)
{
    int j;

    for(j = 0;j < GFL_NET_MACHINE_MAX; j++){
        NEGOTIATION_RETURN_DATA* send_data = (NEGOTIATION_RETURN_DATA*)pNetHandle->negoBuff[j];
        if(send_data->bInit){
            if(_getNegotiationBit(pNetHandle->negotiationBit, j) == FALSE){
                if(GFL_NET_SendDataEx(pNetHandle,GFL_NET_SENDID_ALLUSER,
                                      GFL_NET_CMD_NEGOTIATION_RETURN,
                                      sizeof(NEGOTIATION_RETURN_DATA),pNetHandle->negoBuff[j],
                                      TRUE, FALSE, TRUE)){
                    _setNegotiationBit(pNetHandle->negotiationBit, j, TRUE);
                }
                else{
                    return FALSE;
                }
            }
        }
    }
    pNetHandle->negotiationBit = 0;
    return TRUE;
}

//==============================================================================
/**
 * @brief   ハンドル処理のメイン
 * @retval  none
 */
//==============================================================================

void GFL_NET_HANDLE_MainProc(void)
{
    int i,j, num=0, netID;
    GFL_NETHANDLE* pNetHandle;
    
    for(i = 0;i < GFL_NET_HANDLE_MAX;i++){
        netID = _indexToNetID(i);
        pNetHandle = GFL_NET_GetNetHandle(netID);
        if(FALSE == GFL_NET_IsSendEnable(pNetHandle)){
            continue;
        }
        if(!pNetHandle->timingSendFlg){
            continue;
        }
        if(GFL_NET_HANDLE_GetNetHandleID(pNetHandle) == GFL_NET_NETID_SERVER){
            if(GFL_NET_SendData(pNetHandle, GFL_NET_CMD_TIMING_SYNC_END, 1, &pNetHandle->timingSyncMy)){
                NET_PRINT("同期終了を送信\n");
                pNetHandle->timingSendFlg = FALSE;
            }
        }
        else{
            if(GFL_NET_SendData(pNetHandle, GFL_NET_CMD_TIMING_SYNC, 1, &pNetHandle->timingSyncMy)){
                NET_PRINT("TIMING_SYNC %d\n",netID);
                pNetHandle->timingSendFlg = FALSE;
            }
        }
        if(GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER) == pNetHandle){
            NET_PRINT("OYA \n");

        }
    }
    if(GFL_NET_SystemGetCurrentID()==GFL_NET_NO_PARENTMACHINE){
        pNetHandle = GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER);

        for(j = 0;j < GFL_NET_MACHINE_MAX; j++){
            NEGOTIATION_RETURN_DATA* send_data = (NEGOTIATION_RETURN_DATA*)pNetHandle->negoBuff[j];
            if(send_data->bInit){
                if(_getNegotiationBit(pNetHandle->negotiationReturn,j) == FALSE){
                    if(_sendParentNegotiation(pNetHandle)){
                        pNetHandle->negotiationReturn = _setNegotiationBit(pNetHandle->negotiationReturn, j, TRUE);
                    }
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
    const NEGOTIATION_REQ_DATA *recv_data = (NEGOTIATION_REQ_DATA*)pData;
    GFLNetInitializeStruct* pNetIni = _GFL_NET_GetNETInitStruct();
    NEGOTIATION_RETURN_DATA* send_data;

    
    if(GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER) == pNetHandle){
        send_data=(NEGOTIATION_RETURN_DATA*)pNetHandle->negoBuff[netID];
        if(!send_data->bInit){
            send_data->bInit=TRUE;
            GFL_STD_MemCopy(recv_data->MacAddress, send_data->child_MacAddress, sizeof(recv_data->MacAddress));
            NET_PRINT("RecvNegotiation %d %d\n",netID, recv_data->netid);
            send_data->child_netid = recv_data->netid;
            OS_GetMacAddress(send_data->parent_MacAddress);
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
    const u8* pMsg = (u8*)pData;
	const NEGOTIATION_RETURN_DATA *recv_data = (NEGOTIATION_RETURN_DATA*)pData;
    GFLNetInitializeStruct* pNetInit = _GFL_NET_GetNETInitStruct();
	
    if(GFL_NET_GetNetHandle(GFL_NET_NETID_SERVER) == pNetHandle){
        pNetHandle->negotiationType = _NEG_TYPE_CONNECT;
        return;
    }
    else{



    #if 0	//※check　条件文がおかしいので必ず失敗する。とりあえず前のに戻す
		u8 mac_address[6];
		
        OS_GetMacAddress(mac_address);
		if(recv_data->netid == GFL_NET_HANDLE_GetNetHandleID(GFL_NET_HANDLE_GetCurrentHandle())
				&& GFL_STD_MemComp(recv_data->MacAddress, mac_address, 6) == 0){
	        pNetHandle = GFL_NET_GetNetHandle(recv_data->netid);
    	    pNetHandle->negotiationType = _NEG_TYPE_CONNECT;
        	NET_PRINT("接続認証 OK %d\n",recv_data->netid);
        }
        else{
			GF_ASSERT(0 && "認証失敗");
		}
	#else
        pNetHandle = GFL_NET_GetNetHandle(recv_data->child_netid);
        pNetHandle->negotiationType = _NEG_TYPE_CONNECT;
        NET_PRINT("接続認証 OK %d\n",recv_data->child_netid);
	#endif

        if(pNetInit->connectNegotiationFunc){  // コールバック呼び出し
            pNetInit->connectNegotiationFunc(pWork, recv_data->child_netid);
        }
	//	GFL_NET_IW_ParentMacAddressSet(recv_data->parent_MacAddress);
    }
}

//==============================================================================
/**
 * @brief   ネゴシエーション用符号の操作
 * @param   pNetHandle  ハンドル
 * @param   netID    通信ID
 * @param   onoff    ONOFF
 * @retval  none
 */
//==============================================================================

static u16 _setNegotiationBit(u16 negotiationBit,const int netID,const BOOL onoff)
{
    GF_ASSERT(netID!=GFL_NET_NETID_SERVER);
    {
        u8 bit = 0x01 << netID;
        if(onoff){
            negotiationBit |= bit;
        }
        else{
            negotiationBit ^= bit;
        }
    }
    return negotiationBit;
}

//==============================================================================
/**
 * @brief   ネゴシエーション用符号の確認
 * @param   pNetHandle  ハンドル
 * @param   netID    通信ID
 * @retval  ONならTRUE
 */
//==============================================================================

static BOOL _getNegotiationBit(u16 checkbit,const int netID)
{
    GF_ASSERT(netID!=GFL_NET_NETID_SERVER);
    {
        u8 bit = 0x01 << netID;
        if( checkbit & bit){
            return TRUE;
        }
    }
    return FALSE;
}

