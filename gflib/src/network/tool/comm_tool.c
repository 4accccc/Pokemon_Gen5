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

typedef struct _COMM_TOOL_WORK_t COMM_TOOL_WORK;


typedef struct{
    u8 no;
    u8 listNo;
}_ListResult;

typedef struct _COMM_TOOL_WORK_t{
    u8* timingSyncBuff;   ///< 通信相手の同期コマンド番号
    u8 timingSyncEnd;     ///< 同期コマンド用
    u8 timingSendE;       ///< 送ったかどうか
    u8 timingSyncMy;      ///< 自分が送ったNO
    u8 timingSendM;       ///< 送ったかどうか
};

//static COMM_TOOL_WORK* _pCT = NULL;  ///<　ワーク構造体のポインタ


//==============================================================================
/**
 * 初期化
 * @param[in]   int heapID      確保するHEAPID
 * @param[in]   int num         通信人数
 * @retval      COMM_TOOL_WORK  通信ツール構造体ポインタ
 */
//==============================================================================

COMM_TOOL_WORK* CommToolInitialize(const int heapID, const int num)
{
    int i;
    COMM_TOOL_WORK* pCT;

    pCT = sys_AllocMemory(heapID, sizeof(COMM_TOOL_WORK));
    pCT->timingSync = sys_AllocMemory(heapID, num);
    MI_CpuFill8(pCT->timingSync, 0xff , num);
    pCT->timingSyncEnd = 0xff;
    pCT->timingSyncMy = 0xff;
    pCT->timingSend = FALSE;
    return pCT;

}

//==============================================================================
/**
 * タイミングコマンドを受信した   CS_TIMING_SYNC
 * @param[in]   int netID  通信発信者ID
 * @param[in]   int size   受信データサイズ
 * @param[in]   int pData  受信データポインタ
 * @param[in,out]   void* pWork  使用者が必要なworkのポインタ
 * @param[in,out]   GFNetHandle* pNet  通信ハンドルのポインタ
 * @retval      none
 */
//==============================================================================

void CommRecvTimingSync(const int netID, const int size, const void* pData,
                        void* pWork, GFNetHandle* pNet)
{
    u8* pBuff = pData;
    u8 syncNo = pBuff[0];
    u8 sendBuff[2];
    int i;

    if(CommGetCurrentID(pNet) == COMM_PARENT_ID){
        sendBuff[0] = netID;
        sendBuff[1] = syncNo;
        CommSendFixSizeData_ServerSide(CS_TIMING_SYNC_INFO, &sendBuff);
        OHNO_PRINT("同期受信 %d %d\n",netID,syncNo);
        pNet->pCT->timingSync[netID] = syncNo;     // 同期コマンド用
        for(i = 0; i < COMM_MACHINE_MAX; i++){
            if(CommIsConnect(pNet, i)){
                if(syncNo != pNet->pCT->timingSync[i]){
                    // 同期していない
                    return;
                }
            }
        }
        GF_NT_SendData(pNet, CS_TIMING_SYNC_END, &syncNo);
    }
}


//==============================================================================
/**
 * タイミングコマンドINFOを受信した   CS_TIMING_SYNC_INFO
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================

void CommRecvTimingSyncInfo(int netID, int size, void* pData, void* pWork)
{
    // 子機が状況を把握できるようにコマンドが送られてくる
    u8* pBuff = pData;
    
    _pCT->timingSync[pBuff[0]] = pBuff[1];     // 同期コマンド用
}

//==============================================================================
/**
 * タイミングコマンドENDを受信した   CS_TIMING_SYNC_END
 * @param   callback用引数
 * @retval  none
 */
//==============================================================================

void CommRecvTimingSyncEnd(int netID, int size, void* pData, void* pWork)
{
    u8* pBuff = pData;
    u8 syncNo = pBuff[0];

    OHNO_PRINT("全員同期コマンド受信 %d %d\n", syncNo);
    _pCT->timingSyncEnd = syncNo;     // 同期コマンド用
}

//==============================================================================
/**
 * タイミングコマンドを発行する
 * @param   no   タイミング取りたい番号
 * @retval  none
 */
//==============================================================================

void CommTimingSyncStart(u8 no)
{
    OHNO_PRINT("コマンド発行 %d \n",no);
    _pCT->timingSyncMy = no;
    _pCT->timingSend = TRUE;
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

void CommTimingSyncSend(void)
{
    if(pNet->pCT){
        if(pNet->pCT->timingSendM){
            if(CommSendFixSizeData(CS_TIMING_SYNC, &_pCT->timingSyncMy)){
                OHNO_PRINT("コマンド送信 %d \n",_pCT->timingSyncMy);
                _pCT->timingSend = FALSE;
            }
        }
        if(pNet->pCT->timingSendE){
            if(CommSendFixSizeData(CS_TIMING_SYNC, &_pCT->timingSyncMy)){
                OHNO_PRINT("コマンド送信 %d \n",_pCT->timingSyncMy);
                _pCT->timingSend = FALSE;
            }
        }
    }

}

//==============================================================================
/**
 * タイミングコマンドが届いたかどうかを確認する
 * @param   no   届く番号
 * @retval  届いていたらTRUE
 */
//==============================================================================

BOOL CommIsTimingSync(u8 no)
{
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

//==============================================================================
/**
 * 接続時に相手に送りたいデータを入れる  10byte
 * @param   netID   id   自分の予定のID
 * @param   pData   送りたいデータ
 * @retval  失敗した場合は０
 */
//==============================================================================

void CommToolTempDataReset(void)
{
    int i;
    for(i = 0;i < COMM_MACHINE_MAX;i++){
        _pCT->tempRecv[i] = FALSE;
    }

}

//==============================================================================
/**
 * 接続時に相手に送りたいデータを入れる  10byte
 * @param   netID   id   自分の予定のID
 * @param   pData   送りたいデータ
 * @retval  失敗した場合は０
 */
//==============================================================================

BOOL CommToolSetTempData(int netID,const void* pData)
{
    if(_pCT){
        MI_CpuCopy8(pData, _pCT->tempWork[netID], _TEMPDATA_SIZE);
        CommSendFixSizeData(CS_TOOL_TEMP, _pCT->tempWork[netID]);
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * 接続時に相手に送ったデータを得る  10byte
 * @param   netID   id   相手の予定のID
 * @retval  失敗した場合は０
 */
//==============================================================================

const void* CommToolGetTempData(int netID)
{
    if(_pCT->tempRecv[netID]){
        return &_pCT->tempWork[netID];
    }
    return NULL;
}

//==============================================================================
/**
 * テンポラリーデータを受信する
 * @param   netID   id   相手の予定のID
 * @retval  失敗した場合は０
 */
//==============================================================================

void CommToolRecvTempData(int netID, int size, void* pData, void* pWork)
{
    _pCT->tempRecv[netID] = TRUE;
    MI_CpuCopy8(pData, _pCT->tempWork[netID], _TEMPDATA_SIZE);
}

//==============================================================================
/**
 * テンポラリーデータサイズを返す
 * @param   netID   id   相手の予定のID
 * @retval  失敗した場合は０
 */
//==============================================================================

int CommToolGetTempDataSize(void)
{
    return _TEMPDATA_SIZE;
}
