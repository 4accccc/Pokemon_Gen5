//=============================================================================
/**
 * @file	net_tool.c
 * @brief	通信を使った汎用関数
 * @author	GAME FREAK Inc.
 * @date    2006.11.15
 */
//=============================================================================

#include "gflib.h"

#include "../net_def.h"
#include "net_tool.h"
#include "net_command.h"
#include "../net_system.h"

//==============================================================================
// static宣言
//==============================================================================

struct _NET_TOOLSYS_t{
    u8* timingSyncBuff;   ///< 通信相手の同期コマンド番号
    u8 timingSyncEnd;     ///< 同期コマンド用
    u8 timingSendE;       ///< 送ったかどうか
    u8 timingSyncMy;      ///< 自分が送ったNO
    u8 timingSendM;       ///< 送ったかどうか
} ;

//==============================================================================
/**
 * @brief   ネットワークツール初期化
 * @param   heapID    ヒープ確保を行うID
 * @param   num       初期化人数
 * @return  NET_TOOLSYS  ネットワークツールシステムワーク
 */
//==============================================================================

NET_TOOLSYS* GFL_NET_Tool_sysInit(const int heapID, const int num)
{
    int i;
    NET_TOOLSYS* pCT;

    pCT = GFL_HEAP_AllocMemory(heapID, sizeof(NET_TOOLSYS));
    pCT->timingSyncBuff = GFL_HEAP_AllocMemory(heapID, num);
    MI_CpuFill8(pCT->timingSyncBuff, 0xff , num);
    pCT->timingSyncEnd = 0xff;
    pCT->timingSyncMy = 0xff;
    pCT->timingSendE = FALSE;
    pCT->timingSendM = FALSE;
    return pCT;
}

//==============================================================================
/**
 * @brief   ネットワークツール終了
 * @param   pCT    ネットワークツールシステムワーク
 * @return  none
 */
//==============================================================================

void GFL_NET_Tool_sysEnd(NET_TOOLSYS* pCT)
{
    if(pCT){
        GFL_HEAP_FreeMemory(pCT->timingSyncBuff);
        GFL_HEAP_FreeMemory(pCT);
    }
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

void GFL_NET_ToolRecvTimingSync(const int netID, const int size, const void* pData,
                                void* pWork, GFL_NETHANDLE* pNet)
{
    const u8* pBuff = (const u8*)pData;
    u8 syncNo = pBuff[0];
    u8 sendBuff[2];
    int i;
    NET_TOOLSYS* pCT = _NETHANDLE_GetTOOLSYS(pNet);

    if(pNet->pParent){
        sendBuff[0] = netID;
        sendBuff[1] = syncNo;
        NET_PRINT("同期受信 %d %d\n",netID,syncNo);
        pCT->timingSyncBuff[netID] = syncNo;     // 同期コマンド用
        for(i = 0; i < GFL_NET_MACHINE_MAX; i++){
            if(GFL_NET_IsConnectMember(pNet, i)){
                NET_PRINT("メンバ確認%d\n",i);
                if(syncNo != pCT->timingSyncBuff[i]){
                    NET_PRINT("メンバ確認 同期してない%d %d %d\n", i, syncNo, pCT->timingSyncBuff[i]);
                    // 同期していない
                    pCT->timingSendE = FALSE;
                    return;
                }
            }
        }
        NET_PRINT("メンバ確認完了\n");
        pCT->timingSyncMy = syncNo;
        pCT->timingSendE = TRUE;
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

void GFL_NET_ToolRecvTimingSyncEnd(const int netID, const int size, const void* pData,
                                 void* pWork, GFL_NETHANDLE* pNet)
{
    const u8* pBuff = (const u8*)pData;
    u8 syncNo = pBuff[0];
    NET_TOOLSYS* _pCT = _NETHANDLE_GetTOOLSYS(pNet);

    NET_PRINT("全員同期コマンド受信 %d %d\n", netID, syncNo);
    _pCT->timingSyncEnd = syncNo;     // 同期コマンド用
}

//==============================================================================
/**
 * @brief   タイミングコマンドを発行する
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================

void GFL_NET_ToolTimingSyncStart(GFL_NETHANDLE* pNet, const u8 no)
{
    NET_TOOLSYS* _pCT = _NETHANDLE_GetTOOLSYS(pNet);

    NET_PRINT("コマンド発行 %d \n",no);
    _pCT->timingSyncMy = no;
    _pCT->timingSendM = TRUE;
}

//==============================================================================
/**
 * @brief   タイミングコマンドを送信
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================

void GFL_NET_ToolTimingSyncSend(GFL_NETHANDLE* pNet)
{
    NET_TOOLSYS* pCT = _NETHANDLE_GetTOOLSYS(pNet);

    if(FALSE == GFL_NET_IsSendEnable(pNet)){
        return;
    }
    
    if(pCT->timingSendM){
        if(GFL_NET_SendData(pNet, GFL_NET_CMD_TIMING_SYNC, &pCT->timingSyncMy)){
            pCT->timingSendM = FALSE;
        }
    }
    if(pCT->timingSendE){
        if(GFL_NET_SendData(pNet, GFL_NET_CMD_TIMING_SYNC_END, &pCT->timingSyncMy)){
            NET_PRINT("同期終了を送信\n");
            pCT->timingSendE = FALSE;
        }
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

BOOL GFL_NET_ToolIsTimingSync(GFL_NETHANDLE* pNet, const u8 no)
{
    NET_TOOLSYS* pCT = _NETHANDLE_GetTOOLSYS(pNet);

    NET_PRINT("確認 %d\n",pCT->timingSyncEnd);
    if(pCT->timingSyncEnd == no){
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   タイミングコマンドが届いたかどうかを確認する
 * @param   no   届く番号
 * @retval  届いていたらTRUE
 */
//==============================================================================

int GFL_NET_ToolGetTimingSyncNo(GFL_NETHANDLE* pNet, int netID)
{
    NET_TOOLSYS* pCT = _NETHANDLE_GetTOOLSYS(pNet);
    return pCT->timingSyncBuff[netID];
}

