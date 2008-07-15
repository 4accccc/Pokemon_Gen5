//=============================================================================
/**
 * @file	net_queue.c
 * @brief	（送信）キューの仕組みを管理する関数
 * @author	katsumi ohno
 * @date	06/01/29
 */
//=============================================================================

#include "gflib.h"
#include "../net_def.h"
#include "net_command.h"
#include "net_ring_buff.h"
#include "net_queue.h"


//==============================================================================
/**
 * @brief   空いているキューを返す
 * @param   pQueueMgr キューマネージャーのポインタ
 * @retval  SEND_QUEUE  キューCELL
 */
//==============================================================================

static SEND_QUEUE* _freeQueue(SEND_QUEUE_MANAGER* pQueueMgr)
{
    SEND_QUEUE* pFree = pQueueMgr->heapTop;
    int i;

    for(i = 0;i < pQueueMgr->max; i++){
        if(pFree->command == GFL_NET_CMD_FREE){
            return pFree;
        }
        pFree++;
    }
    return NULL;   // コマンドが無い
}

//==============================================================================
/**
 * @brief   キューに何か入っているかどうかを確認する
 * @param   pQueueMgr キューマネージャーのポインタ
 * @retval  TRUE   空
 * @retval  FALSE  入っている
 */
//==============================================================================

BOOL GFL_NET_QueueIsEmpty(SEND_QUEUE_MANAGER* pQueueMgr)
{
    return (0 == GFL_NET_QueueGetNowNum(pQueueMgr));
}

//==============================================================================
/**
 * @brief   キューの数を得る
 * @param   pQueueMgr キューマネージャーのポインタ
 * @return  使用しているキューの数
 */
//==============================================================================

int GFL_NET_QueueGetNowNum(SEND_QUEUE_MANAGER* pQueueMgr)
{
    SEND_QUEUE* pFree = pQueueMgr->heapTop;
    int i,j = 0;

    for(i = 0;i < pQueueMgr->max; i++){
        if(pFree->command != GFL_NET_CMD_FREE){
            j++;
        }
        pFree++;
    }
    return j;
}

//==============================================================================
/**
 * @brief   キューを消す
 * @param   pTerm   送るキュー管理
 * @retval  TRUE   消せた
 * @retval  FALSE  消すことができなかった
 */
//==============================================================================

static BOOL _deleteQueue(SEND_TERMINATOR* pTerm)
{
    if(pTerm->pTop != NULL){
        if(pTerm->pTop->next!= NULL){
            pTerm->pTop = pTerm->pTop->next;
            pTerm->pTop->prev = NULL;
        }
        else{
            pTerm->pTop = NULL;
            pTerm->pLast = NULL;
        }
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   送信データの一バイトをバッファに入れる
 * @param   pSendBuff   送信バッファ管理構造体ポインタ
 * @param   byte        セットするデータ
 * @return  none
 */
//==============================================================================

static void _setSendData(SEND_BUFF_DATA* pSendBuff ,u8 byte)
{
    *pSendBuff->pData = byte;
    pSendBuff->pData++;
    pSendBuff->size--;
}

//==============================================================================
/**
 * @brief   送信データのヘッダー部分をバッファに入れる
 * @param   pQueue  キューのポインタ
 * @param   pSendBuff   送信バッファ管理構造体ポインタ
 * @retval  FALSE  蓄えた
 * @retval  TRUE   入らなかった
 */
//==============================================================================

static BOOL _dataHeadSet(SEND_QUEUE* pQueue, SEND_BUFF_DATA* pSendBuff)
{
    // 1byte コマンド番号
    _setSendData(pSendBuff,pQueue->command);

    // 1-2byte サイズ
    if(pQueue->size < 127){  // １２７未満は普通に格納
        _setSendData(pSendBuff,(u8)(pQueue->size));
    }
    else{   //１２７以上は最上位BITを立てて２バイトで格納
        _setSendData(pSendBuff,(u8)((pQueue->size >> 8)  & 0xff) | 0x80);
        _setSendData(pSendBuff,(u8)(pQueue->size & 0xff));
    }
    // 1byte 受け取りBIT
    _setSendData(pSendBuff,(u8)(pQueue->recvBit));

    return FALSE;
}

//==============================================================================
/**
 * @brief   キューから送信バッファに移す
 * @param   pQueue  キュー
 * @param   pSendBuff   送信バッファ
 * @param   pSendRing   送信リングバッファ
 * @retval  FALSE ヘッダーが入らなかった場合
 * @retval  TRUE  はいった
 */
//==============================================================================

static BOOL _dataCopyQueue(SEND_QUEUE* pQueue, SEND_BUFF_DATA* pSendBuff,
                           RingBuffWork* pSendRing)
{
    int i;
    int size ;
    if(pSendBuff->size < (pQueue->size + _GFL_NET_QUEUE_HEADERBYTE) ){  // キューのほうがでかい場合
        return FALSE;
    }
    _dataHeadSet(pQueue, pSendBuff);

    if(pSendBuff->size < pQueue->size){  // キューのほうがでかい場合
        if(pQueue->pQData==NULL){
            GFL_NET_RingGets(pSendRing, pSendBuff->pData, pSendBuff->size);
        }
        else{
            MI_CpuCopy8(pQueue->pQData,pSendBuff->pData, pSendBuff->size);
            pQueue->pQData += pSendBuff->size;  // アドレスを進める
        }
        pQueue->size -= pSendBuff->size;  // 送信サイズ減らす
        pSendBuff->size = -1;  // 残りを-1にする
        return TRUE;
    }
    // 同じもしくは小さい場合
    if(pQueue->pQData==NULL){
        GFL_NET_RingGets(pSendRing, pSendBuff->pData, pQueue->size);
    }
    else{
        MI_CpuCopy8( pQueue->pQData, pSendBuff->pData, pQueue->size);
    }
    pSendBuff->pData += pQueue->size;  // アドレスを進める
    pSendBuff->size -= pQueue->size; // サイズは減らす
    return TRUE;
}


//==============================================================================
/**
 * @brief   キューを蓄える
 * @param   pQueueMgr キューマネージャーのポインタ
 * @param   command   送信コマンド
 * @param   pDataArea 送信データ
 * @param   size      サイズ
 * @param   bFast     優先度が高いデータ?
 * @param   bSave     保存するかどうか
 * @param   recvNo    受け取るBIT
 * @retval  TRUE      蓄えた
 * @retval  FALSE     キューに入らなかった
 */
//==============================================================================

BOOL GFL_NET_QueuePut(SEND_QUEUE_MANAGER* pQueueMgr,int command, u8* pDataArea, int size, BOOL bFast, BOOL bSave,int recvBit)
{
    SEND_QUEUE* pLast;
    SEND_QUEUE* pFree = _freeQueue(pQueueMgr);
    SEND_TERMINATOR* pTerm;
    int cSize = size;

    bFast = TRUE;
    if(pFree== NULL){
        GF_ASSERT( 0 );  // 多く取ってあるはずのキューが足りない 通信の設計を見直す必要がある
        return FALSE;
    }
    
    GF_ASSERT((u16)size < (0xffff/2));
    if(bSave){
        int rest = GFL_NET_RingDataRestSize(pQueueMgr->pSendRing);
        if((cSize+_GFL_NET_QUEUE_HEADERBYTE) >= rest){  // 送信バッファをオーバーしてしまう  通信の設計を見直す必要がある
            GF_ASSERT( 0 );
            return FALSE;
        }
        GFL_NET_RingPuts(pQueueMgr->pSendRing, pDataArea, cSize);
        GFL_NET_RingEndChange(pQueueMgr->pSendRing);
        pFree->pQData = NULL;
    }
    else{
        pFree->pQData = pDataArea;
    }
    pFree->size = cSize;
    pFree->command = command;
    pFree->recvBit = recvBit;
    
    if(bFast == TRUE){
        pTerm = &pQueueMgr->fast;
    }
    else{
        pTerm = &pQueueMgr->stock;
    }
    if(pTerm->pLast==NULL){
        pTerm->pLast = pFree;
        pTerm->pTop = pFree;
    }
    else{
        pTerm->pLast->next = pFree;
        pFree->prev = pTerm->pLast;
        pTerm->pLast = pFree;
    }
    return TRUE;
}

//==============================================================================
/**
 * @brief   最新のキューを出す
 * @param   pQueueMgr キューマネージャーのポインタ
 * @retval  キュー
 */
//==============================================================================

static SEND_QUEUE* _queueGet(SEND_QUEUE_MANAGER* pQueueMgr)
{
    SEND_TERMINATOR* pTerm;

    if(pQueueMgr->pNow != NULL){
        return pQueueMgr->pNow;
    }
    if(pQueueMgr->fast.pTop != NULL){
        return pQueueMgr->fast.pTop;
    }
    if(pQueueMgr->stock.pTop != NULL){
        return pQueueMgr->stock.pTop;
    }
    return NULL;
}


//==============================================================================
/**
 * @brief   キューを一個削り次に進める
 * @param   pQueueMgr キューマネージャーのポインタ
 * @retval  none
 */
//==============================================================================

static void _queueNext(SEND_QUEUE_MANAGER* pQueueMgr)
{
    SEND_TERMINATOR* pTerm;

    if(pQueueMgr->pNow != NULL){
        pQueueMgr->pNow = NULL;
    }
    else{
        if(!_deleteQueue( &pQueueMgr->fast )){
            BOOL ret = _deleteQueue( &pQueueMgr->stock );
            GF_ASSERT(ret);
        }
    }
}

//==============================================================================
/**
 * @brief   指定バイト分バッファに入れる
 * @param   pQueueMgr  キューマネージャーのポインタ
 * @param   pSendBuff  送信バッファ構造体のポインタ
 * @param   bNextPlus   データ末尾にヘッダーだけを含めるかどうか
 * @retval  TRUE  続きデータがない場合
 * @retval  FALSE データが連続している場合
 */
//==============================================================================

BOOL GFL_NET_QueueGetData(SEND_QUEUE_MANAGER* pQueueMgr, SEND_BUFF_DATA *pSendBuff)
{
    int ret;
    int i;

    while(pSendBuff->size > 0){
        SEND_QUEUE* pQueue = _queueGet(pQueueMgr);
        if(NULL == pQueue){
            break;
        }
        _queueNext(pQueueMgr);
        // ヘッダー送信できない
        if(!_dataCopyQueue(pQueue, pSendBuff, pQueueMgr->pSendRing)){
            pQueueMgr->pNow = pQueue;  // 次の最初で送信できるようにセット
            break;
        }
        if( -1 == pSendBuff->size ){  // データが詰まっている場合
            pQueueMgr->pNow = pQueue;  // 次の最初で送信できるようにセット
            return FALSE;
        }
        else{
            MI_CpuFill8(pQueue, 0, sizeof(SEND_QUEUE));  // キューを消す
        }
    }
    for(i = 0 ; i < pSendBuff->size; i++){
        *pSendBuff->pData = GFL_NET_CMD_NONE;
        pSendBuff->pData++;
    }
    return TRUE;
}



//==============================================================================
/**
 * @brief   キューMANAGERの初期化
 * @param   pQueueMgr キューマネージャーのポインタ
 * @param   queueMax  キュー数
 * @param   pSendRing 実データを保存する場合のリングバッファワーク
 * @param   heapid    メモリー確保ID
 * @retval  none
 */
//==============================================================================

void GFL_NET_QueueManagerInitialize(SEND_QUEUE_MANAGER* pQueueMgr, int queueMax, RingBuffWork* pSendRing, HEAPID heapid)
{
    MI_CpuFill8(pQueueMgr, 0 ,sizeof(SEND_QUEUE_MANAGER));
    pQueueMgr->heapTop = GFL_HEAP_AllocClearMemory(heapid,
                                         sizeof(SEND_QUEUE)*queueMax);
    pQueueMgr->max = queueMax;
    pQueueMgr->pSendRing = pSendRing;
}


//==============================================================================
/**
 * @brief   キューMANAGERのリセット
 * @param   pQueueMgr キューマネージャーのポインタ
 * @retval  none
 */
//==============================================================================

void GFL_NET_QueueManagerReset(SEND_QUEUE_MANAGER* pQueueMgr)
{
    MI_CpuFill8(pQueueMgr->heapTop, 0 ,sizeof(SEND_QUEUE) * pQueueMgr->max);
    pQueueMgr->fast.pTop = NULL;     // すぐ送る送信キュー
    pQueueMgr->fast.pLast = NULL;     // すぐ送る送信キュー
    pQueueMgr->stock.pTop = NULL;    // 後で送ればいいキュー
    pQueueMgr->stock.pLast = NULL;    // 後で送ればいいキュー
    pQueueMgr->pNow = NULL;   // 今送っている最中のキュー
}

//==============================================================================
/**
 * @brief   キューMANAGERの終了
 * @param   pQueueMgr キューマネージャーのポインタ
 * @retval  none
 */
//==============================================================================

void GFL_NET_QueueManagerFinalize(SEND_QUEUE_MANAGER* pQueueMgr)
{
    GFL_HEAP_FreeMemory( pQueueMgr->heapTop );
}

//==============================================================================
/**
 * @brief   キューが存在するかどうか
 * @param   pQueueMgr  キューマネージャーのポインタ
 * @param   command    調べるコマンド
 * @retval  TRUE ある
 * @retval  FALSE ない
 */
//==============================================================================

BOOL GFL_NET_QueueIsCommand(SEND_QUEUE_MANAGER* pQueueMgr, int command)
{
    int i;
    SEND_QUEUE* pNext = pQueueMgr->heapTop;

    for(i = 0; i < pQueueMgr->max; i++){
        if(pNext->command == command){
            return TRUE;
        }
        pNext++;
    }
    return FALSE;
}

//==============================================================================
/**
 * @brief   送信回数をインクリメント
 * @param   pQueueMgr  キューマネージャーのポインタ
 * @param   inc        送信回数を入れるポインタ
 */
//==============================================================================

void GFL_NET_QueueInc(SEND_QUEUE_MANAGER* pQueueMgr, u8* inc)
{
    *inc = (u8)pQueueMgr->incNo;
    pQueueMgr->incNo++;
}

