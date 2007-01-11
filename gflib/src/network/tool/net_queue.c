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
#include "../net_command.h"
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
    int cs = GFL_NET_CommandGetPacketSize(pQueue->command);

    if( cs == GFL_NET_COMMAND_SIZE_VARIABLE ){
        if(pSendBuff->size < 3){
            pQueue->bHeadSet = FALSE;
            return TRUE;
        }
    }
    else{
        if(pSendBuff->size < 1){
            pQueue->bHeadSet = FALSE;
            return TRUE;
        }
    }
    _setSendData(pSendBuff,pQueue->command);
    if(cs == GFL_NET_COMMAND_SIZE_VARIABLE){
        _setSendData(pSendBuff,(u8)((pQueue->size >> 8)  & 0xff));
        _setSendData(pSendBuff,(u8)(pQueue->size & 0xff));
    }
    else{
        pQueue->size = (u16)cs;
    }
    pQueue->bHeadSet = TRUE;
    return FALSE;
}

//==============================================================================
/**
 * @brief   キューから送信バッファに移す
 * @param   pQueue  キュー
 * @param   pSendBuff   送信バッファ
 * @param   pSendRing   送信リングバッファ
 * @param   bNextPlus   データ末尾にヘッダーだけを含めるかどうか
 * @retval  FALSE ヘッダーが入らなかった場合
 * @retval  TRUE  はいった
 */
//==============================================================================

static BOOL _dataCopyQueue(SEND_QUEUE* pQueue, SEND_BUFF_DATA* pSendBuff,
                           RingBuffWork* pSendRing, BOOL bNextPlus)
{
    int i;
    int size;
    int cs = GFL_NET_CommandGetPacketSize(pQueue->command);
    if( cs == GFL_NET_COMMAND_SIZE_VARIABLE){
        size = 3;
    }
    else{
        size = 1;
    }
    if((pSendBuff->size < (pQueue->size + size)) && (!bNextPlus)){  // キューのほうがでかい場合
        return FALSE;
    }
    if(pQueue->bHeadSet != TRUE){
        if(_dataHeadSet(pQueue, pSendBuff)){
            return FALSE;
        }
    }
//    if((pSendBuff->size < pQueue->size) && (!bNextPlus)){  // キューのほうがでかい場合
//        return FALSE;  // 前に移動
//    }
    if(pSendBuff->size < pQueue->size){  // キューのほうがでかい場合
        if(pQueue->bRing){
            GFL_NET_RingGets(pSendRing, pSendBuff->pData, pSendBuff->size);
        }
        else{
          //  NET_PRINT("-----%x-%x--%d\n",(int)pQueue->pData, (int)pSendBuff->pData , pSendBuff->size);
//            MI_CpuCopy8( pQueue->pData, pSendBuff->pData, pSendBuff->size);
            
            for(i = 0;i < pSendBuff->size;i++){
                pSendBuff->pData[i] = pQueue->pData[i];
            }
        }
//        NET_PRINT("-----%d-%d--\n",pQueue->command, pQueue->size);
        
        pQueue->pData += pSendBuff->size;  // アドレスを進める
        pQueue->size -= pSendBuff->size;  // 送信サイズ減らす
        pSendBuff->size = -1;  // 残りを-1にする
        return TRUE;
    }
    // 同じもしくは小さい場合
    if(pQueue->bRing){
        GFL_NET_RingGets(pSendRing, pSendBuff->pData, pQueue->size);
    }
    else{
        MI_CpuCopy8( pQueue->pData, pSendBuff->pData, pQueue->size);
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
 * @param   pDataArea  送信データ
 * @param   size    サイズ
 * @param   bFast  優先度が高いデータ?
 * @param   bSave  保存するかどうか
 * @retval  TRUE 蓄えた
 * @retval  FALSE キューに入らなかった
 */
//==============================================================================

BOOL GFL_NET_QueuePut(SEND_QUEUE_MANAGER* pQueueMgr,int command, u8* pDataArea,
                      int size, BOOL bFast, BOOL bSave)
{
    SEND_QUEUE* pLast;
    SEND_QUEUE* pFree = _freeQueue(pQueueMgr);
    SEND_TERMINATOR* pTerm;
    int cSize;

    bFast = TRUE;
    if(pFree== NULL){
        NET_PRINT("---キューが無い\n");
        return FALSE;
    }
    
    GF_ASSERT(size < 65534 && "65534以上は分割");
    cSize = GFL_NET_CommandGetPacketSize(command);

    if(GFL_NET_COMMAND_SIZE_VARIABLE == cSize){
        cSize = size;
    }
    if(bSave){
        int rest = GFL_NET_RingDataRestSize(pQueueMgr->pSendRing);
        if((cSize+3) >= rest){  // 送信バッファをオーバーしてしまう
            NET_PRINT("送信バッファオーバー com = %d size = %d / %d\n", command, cSize, rest);
            return FALSE;
        }
        GFL_NET_RingPuts(pQueueMgr->pSendRing, pDataArea, cSize);
        GFL_NET_RingEndChange(pQueueMgr->pSendRing);
        pFree->bRing = TRUE;
    }
    pFree->size = cSize;
    pFree->command = command;
    pFree->pData = pDataArea;
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

BOOL GFL_NET_QueueGetData(SEND_QUEUE_MANAGER* pQueueMgr, SEND_BUFF_DATA *pSendBuff, BOOL bNextPlus)
{
    int ret;
    int i;
    int bNextPlusFirst = TRUE;

    while(pSendBuff->size > 0){
        SEND_QUEUE* pQueue = _queueGet(pQueueMgr);
        if(NULL == pQueue){
            break;
        }
        _queueNext(pQueueMgr);
        // ヘッダー送信できない
        if(!_dataCopyQueue(pQueue, pSendBuff, pQueueMgr->pSendRing, bNextPlusFirst)){
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
        bNextPlusFirst = bNextPlus;
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

void GFL_NET_QueueManagerInitialize(SEND_QUEUE_MANAGER* pQueueMgr, int queueMax, RingBuffWork* pSendRing, int heapid)
{
    MI_CpuFill8(pQueueMgr, 0 ,sizeof(SEND_QUEUE_MANAGER));
    pQueueMgr->heapTop = GFL_HEAP_AllocMemory(heapid,
                                         sizeof(SEND_QUEUE)*queueMax);
    MI_CpuFill8(pQueueMgr->heapTop, 0 ,sizeof(SEND_QUEUE)*queueMax);
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
 * @brief   キューMANAGERのテスト
 * @param   none
 * @retval  none
 */
//==============================================================================
#ifdef PM_DEBUG

void GFL_NET_QueueDebugTest(void)
{
    SEND_QUEUE_MANAGER mgr;
    u8 data1[30];
    u8 data2[22];
    SEND_BUFF_DATA buffData;
    u8 dummy[100];
    u8 ringbuff[50];
    int i;
    RingBuffWork sendRing;

    MI_CpuFill8(data1,2,30);
    MI_CpuFill8(data2,4,22);
    MI_CpuFill8(ringbuff,3,50);
    GFL_NET_RingInitialize(&sendRing, ringbuff, 50);

    // ３本キュー作成
    GFL_NET_QueueManagerInitialize(&mgr,3,&sendRing);

    // ----- テスト１
    GFL_NET_QueuePut(&mgr, GFL_NET_CMD_DEBUG_VARIABLE, data1, 30, TRUE, TRUE);

    for(i = 0; i < 50; i++){
        if(i < 30){
            GF_ASSERT(ringbuff[i] == 2);
        }
        else{
            GF_ASSERT(ringbuff[i] == 3);
        }
    }

    
    GF_ASSERT(mgr.fast.pTop != NULL);
    GF_ASSERT(mgr.fast.pLast != NULL);

    buffData.size = 25;
    buffData.pData = dummy;
    MI_CpuFill8(dummy, 0xff, 100);
    GF_ASSERT(!GFL_NET_QueueGetData(&mgr, &buffData, TRUE));
    GF_ASSERT(buffData.size==-1);
    for(i = 0; i < 100; i++){
        if(i == 0){
            GF_ASSERT(dummy[i] == GFL_NET_CMD_DEBUG_VARIABLE);
        }
        else if(i == 1){
            GF_ASSERT(dummy[i] == 0 && "i=1");
        }
        else if(i == 2){
            GF_ASSERT(dummy[i] == 30 && "i=2");
        }
        else if(i < 25){
            GF_ASSERT(dummy[i] == 2 && "i<25");
        }
        else{
            GF_ASSERT(dummy[i] == 0xff && "i>=25");
        }
    }
    buffData.size = 25;
    buffData.pData = dummy;
    MI_CpuFill8(dummy, 0xff, 100);
    GF_ASSERT(GFL_NET_QueueGetData(&mgr, &buffData, TRUE));
    GF_ASSERT(buffData.size == (25-8));

    GF_ASSERT(mgr.pNow == NULL);
    GF_ASSERT(mgr.fast.pTop == NULL);
    GF_ASSERT(mgr.fast.pLast == NULL);

    for(i = 0; i < 100; i++){
        if(i < 8){
            GF_ASSERT(dummy[i] == 2 && "i < 5");
        }
        else if(i < 25){
            GF_ASSERT(dummy[i] == GFL_NET_CMD_NONE && "GFL_NET_CMD_NONE");
        }
        else{
            GF_ASSERT(dummy[i] == 0xff && "i >= 5");
        }
    }
    buffData.size = 25;
    buffData.pData = dummy;
    MI_CpuFill8(dummy, 0xff, 100);
    GF_ASSERT(GFL_NET_QueueGetData(&mgr, &buffData, TRUE));
    GF_ASSERT(buffData.size == (25));

    GF_ASSERT(mgr.fast.pTop == NULL);
    GF_ASSERT(mgr.fast.pLast == NULL);

    for(i = 0; i < 100; i++){
        if(i < 25){
            GF_ASSERT(dummy[i] == GFL_NET_CMD_NONE && "i<25 GFL_NET_CMD_NONE");
        }
        else{
            GF_ASSERT(dummy[i] == 0xff && "i  all");
        }
    }

    // テスト２
    GF_ASSERT(GFL_NET_QueuePut(&mgr, GFL_NET_CMD_DEBUG_VARIABLE, data1, 10, TRUE, FALSE));

    GF_ASSERT(mgr.fast.pTop != NULL);
    GF_ASSERT(mgr.fast.pLast != NULL);
    GF_ASSERT(mgr.fast.pLast == mgr.fast.pTop);

    GF_ASSERT(GFL_NET_QueuePut(&mgr, GFL_NET_CMD_DEBUG_VARIABLE, data2, 10, TRUE, FALSE));

    GF_ASSERT(mgr.fast.pTop != NULL);
    GF_ASSERT(mgr.fast.pTop->next != NULL);
    GF_ASSERT(mgr.fast.pTop->next->next == NULL);


    GF_ASSERT(GFL_NET_QueuePut(&mgr, GFL_NET_CMD_DEBUG_VARIABLE, data1, 10, TRUE, FALSE));

    GF_ASSERT(mgr.fast.pTop != NULL);
    GF_ASSERT(mgr.fast.pTop->next != NULL);
    GF_ASSERT(mgr.fast.pTop->next->next != NULL);

    // これはキューに入らない
//    GF_ASSERT(!GFL_NET_QueuePut(&mgr, GFL_NET_CMD_DEBUG_VARIABLE, data1, 10, TRUE, TRUE));


    buffData.size = 12;
    buffData.pData = dummy;
    MI_CpuFill8(dummy, 0xff, 100);
    GF_ASSERT(!GFL_NET_QueueGetData(&mgr, &buffData, TRUE));
    GF_ASSERT(buffData.size == -1);

    GF_ASSERT(mgr.fast.pTop != NULL);
    GF_ASSERT(mgr.fast.pLast != NULL);

    for(i = 0; i < 100; i++){
        if(i == 0){
            GF_ASSERT(dummy[i] == GFL_NET_CMD_DEBUG_VARIABLE);
        }
        else if(i == 1){
            GF_ASSERT(dummy[i] == 0);
        }
        else if(i == 2){
            GF_ASSERT(dummy[i] == 10);
        }
        else if(i < 12){
            GF_ASSERT(dummy[i] == 2);
        }
        else{
            GF_ASSERT(dummy[i] == 0xff);
        }
    }
    buffData.size = 12;
    buffData.pData = dummy;
    MI_CpuFill8(dummy, 0xff, 100);
    GF_ASSERT(!GFL_NET_QueueGetData(&mgr, &buffData, TRUE));
    GF_ASSERT(buffData.size == -1);

    GF_ASSERT(mgr.pNow != NULL);
    GF_ASSERT(mgr.fast.pTop != NULL);
    GF_ASSERT(mgr.fast.pLast != NULL);

    for(i = 0; i < 100; i++){
        if(i == 0){
            GF_ASSERT(dummy[i] == 2);
        }
        else if(i == 1){
            GF_ASSERT(dummy[i] == GFL_NET_CMD_DEBUG_VARIABLE);
        }
        else if(i == 2){
            GF_ASSERT(dummy[i] == 0);
        }
        else if(i == 3){
            GF_ASSERT(dummy[i] == 10);
        }
        else if(i < 12){
            GF_ASSERT(dummy[i] == 4);
        }
        else{
            GF_ASSERT(dummy[i] == 0xff);
        }
    }

    // わざと小さく読み込むと、続きだけが入っている
    buffData.size = 4;
    buffData.pData = dummy;
    MI_CpuFill8(dummy, 0xff, 100);
    GF_ASSERT(GFL_NET_QueueGetData(&mgr, &buffData, TRUE));
    GF_ASSERT(buffData.size == 2);

    GF_ASSERT(mgr.pNow != NULL);
    GF_ASSERT(mgr.fast.pTop == NULL);
    GF_ASSERT(mgr.fast.pLast == NULL);

    for(i = 0; i < 100; i++){
        if(i < 2){
            GF_ASSERT(dummy[i] == 4);
        }
        else if(i < 4){
            GF_ASSERT(dummy[i] == GFL_NET_CMD_NONE);
        }
        else{
            GF_ASSERT(dummy[i] == 0xff);
        }
    }

    /// 最後は３番目のコマンドが頭から入っている
    buffData.size = 4;
    buffData.pData = dummy;
    MI_CpuFill8(dummy, 0xff, 100);
    GF_ASSERT(!GFL_NET_QueueGetData(&mgr, &buffData, TRUE));
    GF_ASSERT(buffData.size == -1);

    GF_ASSERT(mgr.pNow != NULL);
    GF_ASSERT(mgr.fast.pTop == NULL);
    GF_ASSERT(mgr.fast.pLast == NULL);

    for(i = 0; i < 100; i++){
        if(i == 0){
            GF_ASSERT(dummy[i] == GFL_NET_CMD_DEBUG_VARIABLE);
        }
        else if(i == 1){
            GF_ASSERT(dummy[i] == 0);
        }
        else if(i == 2){
            GF_ASSERT(dummy[i] == 10);
        }
        else if(i < 4){
            GF_ASSERT(dummy[i] == 2);
        }
        else{
            GF_ASSERT(dummy[i] == 0xff);
        }
    }
    GFL_NET_QueueManagerFinalize(&mgr);

//   GF_ASSERT(0 && "ok");  //テスト完了
}

#endif

