//=============================================================================
/**
 * @file	comm_tool.c
 * @brief	通信を使った汎用関数
 * @author	Katsumi Ohno
 * @date    2006.11.15
 */
//=============================================================================

#include "gflib.h"



//==============================================================================
// static宣言
//==============================================================================



typedef struct{
    u8 no;
    u8 listNo;
}_ListResult;

typedef struct _NET_TOOLSYS_t{
    u8* timingSyncBuff;   ///< 通信相手の同期コマンド番号
    u8 timingSyncEnd;     ///< 同期コマンド用
    u8 timingSendE;       ///< 送ったかどうか
    u8 timingSyncMy;      ///< 自分が送ったNO
    u8 timingSendM;       ///< 送ったかどうか
};

//==============================================================================
/**
 * @brief   ネットワークツール初期化
 * @param   heapID    ヒープ確保を行うID
 * @param   num       初期化人数
 * @return  NET_TOOLSYS  ネットワークツールシステムワーク
 */
//==============================================================================

NET_TOOLSYS* GFL_NET_TOOL_sysInit(const int heapID, const int num)
{
    int i;
    NET_TOOLSYS* pCT;

    pCT = GFL_HEAP_AllocMemory(heapID, sizeof(NET_TOOLSYS));
    pCT->timingSync = GFL_HEAP_AllocMemory(heapID, num);
    MI_CpuFill8(pCT->timingSync, 0xff , num);
    pCT->timingSyncEnd = 0xff;
    pCT->timingSyncMy = 0xff;
    pCT->timingSend = FALSE;
    return pCT;
}

//==============================================================================
/**
 * @brief   ネットワークツール終了
 * @param   pCT    ネットワークツールシステムワーク
 * @return  none
 */
//==============================================================================

void GFL_NET_TOOL_sysEnd(NET_TOOLSYS* pCT)
{
    GFL_HEAP_FreeMemory(pCT->timingSync);
    GFL_HEAP_FreeMemory(pCT);
}

//==============================================================================
/**
 * @brief  タイミングコマンドを受信した   CS_TIMING_SYNC
 * @param[in]   netID  通信発信者ID
 * @param[in]   size   受信データサイズ
 * @param[in]   pData  受信データポインタ
 * @param[in,out]   pWork  使用者が必要なworkのポインタ
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @retval      none
 */
//==============================================================================

void GFL_NET_TOOL_RecvTimingSync(const int netID, const int size, const void* pData,
                                 void* pWork, GFL_NETHandle* pNet)
{
    u8* pBuff = pData;
    u8 syncNo = pBuff[0];
    u8 sendBuff[2];
    int i;
    NET_TOOLSYS* _pCT = _NET_GetTOOLSYS(pNet);

    if(GFL_NET_GetCurrentID(pNet) == COMM_PARENT_ID){
        sendBuff[0] = netID;
        sendBuff[1] = syncNo;
        OHNO_PRINT("同期受信 %d %d\n",netID,syncNo);
        pCT->timingSync[netID] = syncNo;     // 同期コマンド用
        for(i = 0; i < COMM_MACHINE_MAX; i++){
            if(CommIsConnect(pNet, i)){
                if(syncNo != pCT->timingSync[i]){
                    // 同期していない
                    return;
                }
            }
        }
        pCT->timingSendE = TRUE;
    }
}


//==============================================================================
/**
 * タイミングコマンドENDを受信した   CS_TIMING_SYNC_END
 * @param[in]   netID  通信発信者ID
 * @param[in]   size   受信データサイズ
 * @param[in]   pData  受信データポインタ
 * @param[in,out]   pWork  使用者が必要なworkのポインタ
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @retval      none
 */
//==============================================================================

void CommRecvTimingSyncEnd(const int netID, const int size, const void* pData,
                                 void* pWork, GFL_NETHandle* pNet)
{
    u8* pBuff = pData;
    u8 syncNo = pBuff[0];
    NET_TOOLSYS* _pCT = _NET_GetTOOLSYS(pNet);

    NET_PRINT("全員同期コマンド受信 %d %d\n", netID, syncNo);
    _pCT->timingSyncEnd = syncNo;     // 同期コマンド用
}

//==============================================================================
/**
 * タイミングコマンドを発行する
 * @param[in,out]   pNet  通信ハンドルのポインタ
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================

void CommTimingSyncStart(GFL_NETHandle* pNet, u8 no)
{
    NET_TOOLSYS* _pCT = _NET_GetTOOLSYS(pNet);

    NET_PRINT("コマンド発行 %d \n",no);
    _pCT->timingSyncMy = no;
    _pCT->timingSendM = TRUE;
}

//==============================================================================
/**
 * タイミングコマンドを送信
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================


#ifdef PM_DEBUG
extern int DebugCommGetNum(int id);
static u8 _keytrg = 0;
#endif

void CommTimingSyncSend(GFL_NETHandle* pNet)
{
    NET_TOOLSYS* _pCT = _NET_GetTOOLSYS(pNet);

    if(pNet->pCT){
        if(pNet->pCT->timingSendM){
            if(CommSendFixSizeData(CS_TIMING_SYNC, &_pCT->timingSyncMy)){
                _pCT->timingSendM = FALSE;
            }
        }
        if(pNet->pCT->timingSendE){
            if(CommSendFixSizeData(CS_TIMING_SYNC_END, &_pCT->timingSyncMy)){
                _pCT->timingSendE = FALSE;
            }
        }

        GFL_NET_SendData(pNet, CS_TIMING_SYNC_END, &syncNo);


    }

}

//==============================================================================
/**
 * タイミングコマンドが届いたかどうかを確認する
 * @param   no   届く番号
 * @retval  届いていたらTRUE
 */
//==============================================================================

BOOL CommIsTimingSync(GFL_NETHandle* pNet, u8 no)
{
    NET_TOOLSYS* _pCT = _NET_GetTOOLSYS(pNet);

    if(_pCT==NULL){
        return TRUE;  // 通信してない場合同期しているとみなす
    }
    if(_pCT->timingSyncEnd == no){
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * タイミングコマンドが届いたかどうかを確認する
 * @param   no   届く番号
 * @retval  届いていたらTRUE
 */
//==============================================================================

int CommGetTimingSyncNo(int netID)
{
    return _pCT->timingSync[netID];
}

//------------------------------------------------------
/**
 * @brief   選択結果を受信する関数
 * @param   コールバック引数
 * @retval  none
 */
//------------------------------------------------------

void CommRecvListNo(int netID, int size, void* pData, void* pWork)
{
    _ListResult* pRet = pData;

    _pCT->listNo[netID].no = pRet->no;
    _pCT->listNo[netID].listNo = pRet->listNo;
}

//------------------------------------------------------
/**
 * @brief   選択結果を通信するコマンドサイズ
 * @param   no        選択ユニーク番号
 * @param   listNo    選択結果  0-255
 * @retval  none
 */
//------------------------------------------------------

int CommGetListNoSize(void)
{
    return sizeof(_ListResult);
}

//------------------------------------------------------
/**
 * @brief   選択結果を通信する
 * @param   no        選択ユニーク番号
 * @param   listNo    選択結果  0-255
 * @retval  none
 */
//------------------------------------------------------

void CommListSet(u8 no,u8 listNo)
{
    _ListResult ret;

    ret.no = no;
    ret.listNo = listNo;
    CommSendFixSizeData(CS_LIST_NO,&ret);
}

//------------------------------------------------------
/**
 * @brief   選択結果を受け取る
 * @param   recvNetID   どの人のデータを待ち受けるのか
 * @param   pBuff       受信バッファ
 * @retval  選択結果 もしくは -1
 */
//------------------------------------------------------

int CommListGet(int netID, u8 no)
{
    if(!_pCT){
        return INVALID_LIST_NO;
    }
    if(_pCT->listNo[netID].no == no){
        return _pCT->listNo[netID].listNo;
    }
    return INVALID_LIST_NO;
}


//------------------------------------------------------
/**
 * @brief   選択結果を消す
 * @param   none
 * @retval  none
 */
//------------------------------------------------------

void CommListReset(void)
{
    int i;

    for(i = 0;i < COMM_MACHINE_MAX;i++){
        MI_CpuFill8(&_pCT->listNo[i], 0, sizeof(_ListResult));
    }
}

