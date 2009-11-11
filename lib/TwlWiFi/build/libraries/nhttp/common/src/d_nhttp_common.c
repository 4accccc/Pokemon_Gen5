/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     d_nhttp_common.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include "d_nhttp_common.h"
#include "NHTTP_bgnend.h"
#include "NHTTP_list.h"
#include "NHTTP_request.h"
#include "NHTTP_nonport.h"
#include "NHTTP_thread.h"

static NHTTPConnection*         l_nhttp_connection_list_p = NULL;
static NHTTPStaticResource      NHTTPiStaticResource[1];
static NHTTPSysInfo             sysInfo;
static NHTTPSysInfo*            sysInfo_p = NULL;

enum {
    NHTTP_LIST_ORDER_SEARCH,
    NHTTP_LIST_ORDER_SEARCH_FROM_REQ,
    NHTTP_LIST_ORDER_SEARCH_FROM_RES,
    NHTTP_LIST_ORDER_COMMIT,
    NHTTP_LIST_ORDER_OMIT,

    NHTTP_LIST_ORDER_END
} NHTTPOrder;


/*---------------------------------------------------------------------------*
    Forward references for static functions
 *---------------------------------------------------------------------------*/
static NHTTPStaticResource* NHTTPi_GetStaticResource(void);

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetStaticResource

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
NHTTPStaticResource* NHTTPi_GetStaticResource(void)
{
    NHTTPStaticResource    *unique = NHTTPiStaticResource;
    /* 初回使用時のみ初期化し, 決して解放しない */
    if (!unique->initialized)
    {
        DEBUG_PRINTF("**********************************\n");
        DEBUG_PRINTF("[%s]\n", __func__);
        DEBUG_PRINTF("**********************************\n");
        
        NHTTPi_InitMutex(&unique->mutex); // not used @ NTR
#if defined(NHTTP_FOR_RVL)
        OSInitCond(&unique->cond);
#elif defined(NHTTP_FOR_NTR)
        OS_InitThreadQueue(&unique->queue);
#elif defined(NHTTP_FOR_RVL_MINI)
        UTL_InitSignal(&unique->signal);
#else
#error
#endif
        unique->initialized = TRUE;
    }
    return unique;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_WaitForCompletion

  Description:  完了待ち
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
void NHTTPi_WaitForCompletion(NHTTPConnection *connection_p)
{
    NHTTPStaticResource    *unique = NHTTPi_GetStaticResource();
    
#if defined(NHTTP_FOR_RVL) || defined(NHTTP_FOR_RVL_MINI)
    NHTTPi_LockMutex(&unique->mutex);
#endif
    {
        while (*(volatile BOOL*)&connection_p->running)
        {
            DEBUG_PRINTF("##################################\n");
            DEBUG_PRINTF("##################################\n");
            DEBUG_PRINTF("##################################\n");
            DEBUG_PRINTF("[%s] reqID[%d]\n", __func__, connection_p->reqId);
            DEBUG_PRINTF("[%s] reqID[%d]\n", __func__, connection_p->reqId);
            DEBUG_PRINTF("[%s] reqID[%d]\n", __func__, connection_p->reqId);
            DEBUG_PRINTF("##################################\n");
            DEBUG_PRINTF("##################################\n");
            DEBUG_PRINTF("##################################\n");
            
#if defined(NHTTP_FOR_RVL)
            OSWaitCond(&unique->cond, &unique->mutex);
#elif defined(NHTTP_FOR_NTR)
            OS_SleepThread(&unique->queue);
#elif defined(NHTTP_FOR_RVL_MINI)
            UTL_WaitSignal(&unique->signal, &unique->mutex);
#else
#error
#endif
        }
    }
#if defined(NHTTP_FOR_RVL) || defined(NHTTP_FOR_RVL_MINI)
    NHTTPi_UnlockMutex(&unique->mutex);
#endif
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_NotifyCompletion

  Description:  完了通知処理
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
void NHTTPi_NotifyCompletion(NHTTPConnection *connection_p)
{
    NHTTPStaticResource    *unique = NHTTPi_GetStaticResource();
    
#if defined(NHTTP_FOR_RVL) || defined(NHTTP_FOR_RVL_MINI)
    NHTTPi_LockMutex(&unique->mutex);
#endif
    {
        DEBUG_PRINTF("**********************************\n");
        DEBUG_PRINTF("[%s] reqID[%d]\n", __func__, connection_p->reqId);
        DEBUG_PRINTF("**********************************\n");
        
        connection_p->running = FALSE;
#if defined(NHTTP_FOR_RVL)
        OSSignalCond(&unique->cond); 
#elif defined(NHTTP_FOR_NTR)
        OS_WakeupThread(&unique->queue);
#elif defined(NHTTP_FOR_RVL_MINI)
        UTL_BroadcastSignal(&unique->signal);
#else
#error
#endif
    }
#if defined(NHTTP_FOR_RVL) || defined(NHTTP_FOR_RVL_MINI)
    NHTTPi_UnlockMutex(&unique->mutex);
#endif
}


/*-------------------------------------------------------------------------*
  Name:         NHTTPi_SearchConnection

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
static void* NHTTPi_ControlConnectionList(NHTTPMutexInfo* mutexInfo_p, void* handle_p, u32 order)
{

    NHTTPConnection*	ret_p = NULL;

    NHTTPi_lockReqList(mutexInfo_p);
    if (order == NHTTP_LIST_ORDER_COMMIT)
    {
        NHTTPConnection*	connection_p = (NHTTPConnection*)handle_p;
        
        connection_p->next_p = l_nhttp_connection_list_p;
        l_nhttp_connection_list_p = connection_p;
        ret_p = handle_p;
    }
    else
    {
        NHTTPConnection**	connection_pp = &l_nhttp_connection_list_p;
        
        while (*connection_pp != NULL)
        {
            switch (order)
            {
            case NHTTP_LIST_ORDER_SEARCH:
                if (*connection_pp == handle_p)
                {
                    ret_p = *connection_pp;
                }
                break;
            case NHTTP_LIST_ORDER_SEARCH_FROM_REQ:
                if ((*connection_pp)->req_p == handle_p)
                {
                    ret_p = *connection_pp;
                }
                break;
            case NHTTP_LIST_ORDER_SEARCH_FROM_RES:
                if ((*connection_pp)->res_p == handle_p)
                {
                    ret_p = *connection_pp;
                }
                break;
            case NHTTP_LIST_ORDER_OMIT:
                if (*connection_pp == handle_p)
                {
                    ret_p = *connection_pp;
                    *connection_pp = (*connection_pp)->next_p;
                }
                break;
            default:
                break;
            }
            if (ret_p != NULL)
            {
                break;
            }
            connection_pp = &((*connection_pp)->next_p);
        }
    }
    NHTTPi_unlockReqList(mutexInfo_p);
    return ret_p;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_CommitConnectionList

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern int NHTTPi_CommitConnectionList(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{
    return ((NHTTPi_ControlConnectionList(mutexInfo_p, connection_p, NHTTP_LIST_ORDER_COMMIT) != NULL) ? 0 : -1);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_OmitConnectionList

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern int NHTTPi_OmitConnectionList(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{
    return ((NHTTPi_ControlConnectionList(mutexInfo_p, connection_p, NHTTP_LIST_ORDER_OMIT) != NULL) ? 0 : -1);
}


/*-------------------------------------------------------------------------*
  Name:         NHTTPi_Connection2Request

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern NHTTPReq* NHTTPi_Connection2Request(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{
    NHTTPConnection*	a_connection_p = NHTTPi_GetConnection(mutexInfo_p, connection_p);

    if (a_connection_p != NULL)
    {
        return a_connection_p->req_p;
    }
    else
    {
        return NULL;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_Connection2Response

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern NHTTPRes* NHTTPi_Connection2Response(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{
    NHTTPConnection*	a_connection_p = NHTTPi_GetConnection(mutexInfo_p, connection_p);

    if (a_connection_p != NULL)
    {
        return a_connection_p->res_p;
    }
    else
    {
        return NULL;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_Request2Connection

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern NHTTPConnection* NHTTPi_Request2Connection(NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p)
{
    return (NHTTPConnection*)NHTTPi_ControlConnectionList(mutexInfo_p, req_p, NHTTP_LIST_ORDER_SEARCH_FROM_REQ);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_Response2Connection

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern NHTTPConnection* NHTTPi_Response2Connection(NHTTPMutexInfo* mutexInfo_p, NHTTPRes* res_p)
{
    return (NHTTPConnection*)NHTTPi_ControlConnectionList(mutexInfo_p, res_p, NHTTP_LIST_ORDER_SEARCH_FROM_RES);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetConnection

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern NHTTPConnection* NHTTPi_GetConnection(NHTTPMutexInfo* mutexInfo_p, NHTTPConnectionHandle connection_handle_p)
{
    return (NHTTPConnection*)NHTTPi_ControlConnectionList(mutexInfo_p, connection_handle_p, NHTTP_LIST_ORDER_SEARCH);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetRequest

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern NHTTPReq* NHTTPi_GetRequest(NHTTPMutexInfo* mutexInfo_p, void* handle_p)
{
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle_p);

    if (connection_p != NULL)
    {
        return connection_p->req_p;
    }
    else
    {
        return (NHTTPReq*)handle_p;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetResponse

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern NHTTPRes* NHTTPi_GetResponse(NHTTPMutexInfo* mutexInfo_p, void* handle_p)
{
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle_p);

    if (connection_p != NULL)
    {
        return connection_p->res_p;
    }
    else
    {
        return (NHTTPRes*)handle_p;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetConnectionListLength

  Description:  Connection listの長さ取得関数
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern u32 NHTTPi_GetConnectionListLength(void)
{
    NHTTPConnection*	connection_p = l_nhttp_connection_list_p;
    u32			len = 0;
    for (; connection_p != NULL; connection_p = connection_p->next_p)
    {
        len++;
    }
    return len;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_PostSendCallback

  Description:  
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern int NHTTPi_PostSendCallback(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p, const char* label_p, int offset)
{
    int	ret = -1;
    
    if (NHTTPi_GetConnection(mutexInfo_p, connection_p) != NULL)
    {
        NHTTPRes*	res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);
        
        if ((res_p != NULL) && (connection_p->connectionCallback != NULL))
        {
            NHTTPPostSendArg	arg;
            arg.label = label_p;
            arg.buf = connection_p->postDataBuf.value_p;
            arg.size = connection_p->postDataBuf.length;
            arg.offset = (u32)offset;
           
            ret = connection_p->connectionCallback(connection_p, NHTTP_EVENT_POST_SEND, &arg);
            (void)NHTTPi_SetPostDataBuffer(mutexInfo_p, connection_p, arg.buf, arg.size);
        }
    }
    return ret;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_BufferFullCallback

  Description:  Body受信時バッファ溢れコールバック
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_BufferFullCallback(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{

    if (NHTTPi_GetConnection(mutexInfo_p, connection_p) != NULL)
    {
        NHTTPRes*	res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);
        
        if ((res_p != NULL) && (connection_p->connectionCallback != NULL))
        {
            NHTTPBodyBufArg	arg;

            arg.buf = res_p->recvBuf_p;
            arg.size = res_p->recvBufLen;
            arg.offset = (u32)res_p->bodyLen;
            
            (void)connection_p->connectionCallback(connection_p, NHTTP_EVENT_BODY_RECV_FULL, &arg);
            NHTTPi_SetBodyBuffer(mutexInfo_p, connection_p, arg.buf, arg.size, arg.offset);
        }
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_ReceivedCallback

  Description:  Body受信終了時コールバック
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_ReceivedCallback(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{

    if (NHTTPi_GetConnection(mutexInfo_p, connection_p) != NULL)
    {
        NHTTPRes*	res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);
        
        if ((res_p != NULL) && (connection_p->connectionCallback != NULL))
        {
            NHTTPBodyBufArg	arg;
    
            arg.buf = res_p->recvBuf_p;
            arg.size = res_p->recvBufLen;
            arg.offset = (u32)res_p->bodyLen;
            
            (void)connection_p->connectionCallback(connection_p, NHTTP_EVENT_BODY_RECV_DONE, &arg);
            (void)NHTTPi_SetBodyBuffer(mutexInfo_p, connection_p, arg.buf, arg.size, arg.offset);
        }
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_CompleteCallback

  Description:  リクエスト終了コールバック
                 
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_CompleteCallback(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{

    if (NHTTPi_GetConnection(mutexInfo_p, connection_p) != NULL)
    {
        if (connection_p->connectionCallback != NULL)
        {
            (void)connection_p->connectionCallback(connection_p, NHTTP_EVENT_COMPLETE, NULL);
        }
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_SetPostDataBuffer

  Description:  送信バッファセット
  
  Arguments:    handle  - 接続ハンドル
                buf     - 送信バッファ
                length  - バッファサイズ
                
  Returns:      送信バッファがセットできれば 0, そうでなければ -1
 *-------------------------------------------------------------------------*/
extern NHTTPError NHTTPi_SetPostDataBuffer(NHTTPMutexInfo* mutexInfo_p, NHTTPConnectionHandle handle, void* value_p, u32 length)
{
    NHTTPConnection* connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
    
    if (connection_p == NULL)  return NHTTP_ERROR_SYSTEM;
        
    connection_p->postDataBuf.value_p = value_p;
    connection_p->postDataBuf.length = length;
    return NHTTP_ERROR_NONE;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_SetBodyBuffer

  Description:  受信バッファセット
  
  Arguments:    handle  - 接続ハンドル
                buf     - 受信バッファ
                length  - バッファサイズ
                
  Returns:      受信バッファがセットできれば 0, そうでなければ -1
 *-------------------------------------------------------------------------*/
extern NHTTPError NHTTPi_SetBodyBuffer(NHTTPMutexInfo* mutexInfo_p, NHTTPConnectionHandle handle, void* buf_p, u32 length, u32 offset)
{

    NHTTPConnection* connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
    
    if (connection_p != NULL)
    {
        NHTTPRes* res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);
        
        if (res_p != NULL)
        {
            res_p->recvBuf_p = buf_p;
            res_p->recvBufLen = length;
            res_p->bodyLen = (s32)offset;
            return NHTTP_ERROR_NONE;
        }
        else
        {
            return NHTTP_ERROR_SYSTEM;
        }
    }
    else
    {
        return NHTTP_ERROR_SYSTEM;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_InitSystemInfo

  Description:  NHTTPシステム情報初期設定
  
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
static void NHTTPi_InitSystemInfo(NHTTPSysInfo* sysInfo_p)
{
    NHTTPi_InitBgnEndInfo(NHTTPi_GetBgnEndInfoP(sysInfo_p));
    NHTTPi_InitListInfo(NHTTPi_GetListInfoP(sysInfo_p));
    NHTTPi_InitRequestInfo(NHTTPi_GetReqInfoP(sysInfo_p));
    NHTTPi_InitMutexInfo(NHTTPi_GetMutexInfoP(sysInfo_p));
    NHTTPi_InitThreadInfo(NHTTPi_GetThreadInfoP(sysInfo_p));
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetSystemInfoP

  Description:  NHTTPシステム情報ポインタ取得
  
  Arguments:
  
  Returns:      NHTTPSysInfo*
 *-------------------------------------------------------------------------*/
extern NHTTPSysInfo* NHTTPi_GetSystemInfoP()
{
    /* 初期設定 */
    if( sysInfo_p == NULL )
    {
        sysInfo_p = &sysInfo;
        NHTTPi_InitSystemInfo( sysInfo_p );
    }
    return sysInfo_p;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetBgnEndInfoP

  Description:  NHTTPBgnEndInfoポインタ取得
  
  Arguments:
  
  Returns:      NHTTPBgnEndInfo*
 *-------------------------------------------------------------------------*/
extern NHTTPBgnEndInfo* NHTTPi_GetBgnEndInfoP(NHTTPSysInfo* sysInfo_p)
{
    return &(sysInfo_p->bgnEndInfo);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetListInfoP

  Description:  NHTTPListInfoポインタ取得
  
  Arguments:
  
  Returns:      NHTTPListInfo*
 *-------------------------------------------------------------------------*/
extern NHTTPListInfo* NHTTPi_GetListInfoP(NHTTPSysInfo* sysInfo_p)
{
    return &(sysInfo_p->listInfo);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetReqInfoP

  Description:  NHTTPReqInfoポインタ取得
  
  Arguments:
  
  Returns:      NHTTPReqInfo*
 *-------------------------------------------------------------------------*/
extern NHTTPReqInfo* NHTTPi_GetReqInfoP(NHTTPSysInfo* sysInfo_p)
{
    return &(sysInfo_p->reqInfo);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetThreadInfoP

  Description:  NHTTPThreadInfoポインタ取得
  
  Arguments:
  
  Returns:      NHTTPThreadInfo*
 *-------------------------------------------------------------------------*/
extern NHTTPThreadInfo* NHTTPi_GetThreadInfoP(NHTTPSysInfo* sysInfo_p)
{
    return &(sysInfo_p->threadInfo);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetMutexInfoP

  Description:  NHTTPMutexInfoポインタ取得
  
  Arguments:
  
  Returns:      NHTTPMutexInfo*
 *-------------------------------------------------------------------------*/
extern NHTTPMutexInfo* NHTTPi_GetMutexInfoP(NHTTPSysInfo* sysInfo_p)
{
    return &(sysInfo_p->mutexInfo);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_SetVirtualContentLength

  Description:  chunked sizeを含む実質的な contentLengthセット関数
  
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_SetVirtualContentLength(NHTTPConnection* connection_p, u32 contentLength)
{
    if (connection_p != NULL)
    {
        connection_p->virtualContentLength = contentLength;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetVirtualContentLength

  Description:  chunked sizeを含む実質的な contentLength取得関数
  
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern u32 NHTTPi_GetVirtualContentLength(const NHTTPConnection* connection_p)
{
    if (connection_p != NULL)
    {
        return connection_p->virtualContentLength;
    }
    else
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_InitConnectionList

  Description:  接続ハンドルの先頭ポインタの初期化
  
  Arguments:
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_InitConnectionList()
{
    // 正常に接続ハンドルが消されていればNULLになっている
    if (l_nhttp_connection_list_p) {
        // このワーニングが表示された場合は
        // 旧仕様であればNHTTPDestroyResponseの呼び忘れ
        // 新仕様であればNHTTPDeleteConnectionの呼び忘れ
        // による解放忘れがあります。
        u32	connectionListLength = NHTTPi_GetConnectionListLength();
        if (connectionListLength > 0)
        {
            PRINTF("*warning: %d connections rests! Please free connections.\n", connectionListLength);
        }
    }
    NHTTPi_ASSERT(l_nhttp_connection_list_p == NULL);
    
    l_nhttp_connection_list_p = NULL; 
}

/*-------------------------------------------------------------------------*

  Name:         NHTTPi_InitMutexInfo

  Description:  NHTTPMutexInfo構造体の初期設定
                 
  Arguments:    mutexInfo_p  NHTTPMutexInfo構造体へのポインタ

  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_InitMutexInfo(NHTTPMutexInfo* mutexInfo_p)
{
    mutexInfo_p->isInitMutex = FALSE;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_initLockReqList
  Description:  ミューテックスを初期化
 *-------------------------------------------------------------------------*/
extern void NHTTPi_initLockReqList(NHTTPMutexInfo* mutexInfo_p)
{
    if (mutexInfo_p->isInitMutex == FALSE)
    {
        NHTTPi_InitMutex(&(mutexInfo_p->mutex));
        mutexInfo_p->isInitMutex = TRUE;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_exitLockReqList
  Description:  ミューテックスを削除
 *-------------------------------------------------------------------------*/
extern void NHTTPi_exitLockReqList()
{
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_lockReqList
  Description:  ミューテックスを取得
 *-------------------------------------------------------------------------*/
extern void NHTTPi_lockReqList(NHTTPMutexInfo* mutexInfo_p)
{
    NHTTPi_LockMutex(&(mutexInfo_p->mutex));
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_unlockReqList
  Description:  ミューテックスを解放
 *-------------------------------------------------------------------------*/
extern void NHTTPi_unlockReqList(NHTTPMutexInfo* mutexInfo_p)
{
    OS_UnlockMutex(&(mutexInfo_p->mutex));
}

extern int NHTTPi_addRef(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{
    NHTTPi_ASSERT(connection_p->ref >= 0);
    NHTTPi_lockReqList(mutexInfo_p);
    connection_p->ref++;
    NHTTPi_unlockReqList(mutexInfo_p);
    return connection_p->ref;
}

extern int NHTTPi_decRef(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{
    NHTTPi_ASSERT(connection_p->ref > 0);
    NHTTPi_lockReqList(mutexInfo_p);
    connection_p->ref--;
    NHTTPi_unlockReqList(mutexInfo_p);
    return connection_p->ref;
}
