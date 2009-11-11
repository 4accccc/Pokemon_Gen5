/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     d_nhttp_private.h

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
#include "nhttp.h"
#include "NHTTPlib_int.h"
#include "d_nhttp_common.h"
#include "NHTTP_bgnend.h"
#include "NHTTP_request.h"
#include "NHTTP_response.h"
#include "NHTTP_nonport.h"
#include "d_nhttp_private.h"

/*-------------------------------------------------------------------------*
  Name:         NHTTP_CreateConnection

  Description:  新規の接続ハンドルを生成
                 
  Arguments:    url_p		- 接続先の URL 文字列
                method		- 接続メソッド
                buf_p		- ボディ受信バッファ または NULL
                len		- ボディ受信バッファ長 または 0
                callback	- イベント通知コールバック または NULL
                userParam_p	- 任意のユーザ定義引数 または NULL
                
  Returns:      新規に生成された接続ハンドル、または NULL
 *-------------------------------------------------------------------------*/
extern NHTTPConnectionHandle NHTTP_CreateConnection(const char* url_p, NHTTPReqMethod method, char* buf_p, u32 len, NHTTPConnectionCallback callback, void* userParam_p)
{
    NHTTPConnection*	connection_p = NULL;
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);

    DEBUG_PRINTF("[%s]\n", __func__);

    //
    // メモリ確保
    //
    // 一時受信バッファ(tmpBuf)用に 32バイトアライメント
    connection_p = (NHTTPConnectionHandle)NHTTPi_alloc(sizeof(NHTTPConnection), 32);
    
    // 確保失敗
    if (!connection_p)
    {
        DEBUG_PRINTF("[%s][line:%d] NHTTP_ERROR_ALLOC\n", __func__, __LINE__);
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        return NULL;
    }

    // ※接続ハンドル構造体の初期化
    NHTTPi_memclr(connection_p, sizeof(NHTTPConnection));
    
    DEBUG_PRINTF("[%s] : connection %p %p\n", __func__,
                 connection_p,
                 connection_p->tmpBuf);
    
    connection_p->req_p = NHTTPi_CreateRequest(bgnEndInfo_p, url_p, method, buf_p, len, userParam_p, NULL, NULL);

    // NHTTP_CreatecondRequest失敗
    if (!connection_p->req_p)
    {
        NHTTPi_free(connection_p);
        return NULL;
    }
    connection_p->res_p = connection_p->req_p->res_p;
    // ステータスを待ち状態に
    connection_p->status = NHTTP_STATUS_IDLE;
    // callback設定
    connection_p->connectionCallback = callback;
    // postDataBuf初期設定
    connection_p->postDataBuf.value_p = NULL;
    connection_p->postDataBuf.length = 0;
    
    connection_p->reqId = -1;
    // リストに登録
    NHTTPi_CommitConnectionList(mutexInfo_p, connection_p);
    // エラーをBUSYに設定
    connection_p->nhttpError = NHTTP_ERROR_BUSY;
    // SSLエラーはまだ無いのでENONEに設定
    connection_p->sslError = NHTTP_SSL_ENONE;
    
#if 0
//noma 2006.11.08
// 旧仕様対応
    // soRecvSize初期設定
    connection_p->soRecvSize = 0;
#endif
    
    connection_p->running = TRUE;
    
    NHTTPi_SetVirtualContentLength(connection_p, 0);
    connection_p->reqCallback = NULL;
    // 一時受信バッファ初期設定
    connection_p->tmpBufOffset = 0;
    connection_p->tmpBufRest = 0;
    NHTTPi_memclr(&(connection_p->tmpBuf[0]), LEN_TMP_BUF);
    
    connection_p->ref = 1;
    
    return (void*)connection_p;
}

/*---------------------------------------------------------------------------*
  Name        : NHTTP_DeleteConnection
  
  Description : 接続ハンドルを解放
  
  Arguments   : handle  - 接続ハンドル
  
  Returns     : 接続ハンドルを開放できれば 0, そうでなければ -1
 *---------------------------------------------------------------------------*/
extern NHTTPError NHTTP_DeleteConnection(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
    
    DEBUG_PRINTF("[%s]\n", __func__);

    if (connection_p == NULL)  return NHTTP_ERROR_SYSTEM;

    return NHTTPi_DeleteConnection(connection_p);
}

extern NHTTPError NHTTPi_DeleteConnection(NHTTPConnection* connection_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    
    DEBUG_PRINTF("[%s]\n", __func__);

    if (connection_p == NULL)  return NHTTP_ERROR_SYSTEM;

    // 要求中ならキャンセル
    if (connection_p->ref > 0 && connection_p->reqId >= 0)
    {
        NHTTPi_CancelRequestAsync(sysInfo_p, connection_p->reqId);
        connection_p->reqId = -1;
    }

    if (NHTTPi_decRef(mutexInfo_p, connection_p) > 0)
    {
        return NHTTP_ERROR_NONE;
    }
    
    if (connection_p->req_p != NULL)
    {
        // リクエスト開始後はリクエストの削除はスレッドに任せる
        if (!connection_p->req_p->isStarted)
        {
            NHTTPi_DestroyRequest(sysInfo_p, connection_p->req_p);
        }
    }
    if (connection_p->res_p != NULL)
    {
        // リクエスト開始前　またはリクエスト完了後
        if (connection_p->req_p != NULL)
        {
            // 待ってから削除
            NHTTP_WaitForConnection(connection_p);
        }
        NHTTPi_DestroyResponse(mutexInfo_p, connection_p->res_p);
    }
    // リストから削除
    (void)NHTTPi_OmitConnectionList(mutexInfo_p, connection_p);
    NHTTPi_free(connection_p);

    return NHTTP_ERROR_NONE;
}

/*---------------------------------------------------------------------------*
  Name        : NHTTP_WaitForConnection
  
  Description : 通信中状態の完了を待つ
  
  Arguments   : handle  - 接続ハンドル
  
  Returns     : 
 *---------------------------------------------------------------------------*/
extern NHTTPError NHTTP_WaitForConnection(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);

    DEBUG_PRINTF("[%s]\n", __func__);
    
    if (connection_p == NULL) return NHTTP_ERROR_SYSTEM;
    
    {
        NHTTPConnectionStatus status = NHTTP_GetConnectionStatus(handle);

        NHTTPi_CheckCurrentThread(threadInfo_p, NHTTP_CURRENT_THREAD_IS_NOT_NHTTP_THREAD);
        
        if ((status != NHTTP_STATUS_ERROR) &&
            (status != NHTTP_STATUS_IDLE) &&
            (NHTTP_GetConnectionError(handle) == NHTTP_ERROR_BUSY))
        {
            NHTTPi_WaitForCompletion(connection_p);
        }
    }
    return NHTTP_ERROR_NONE;
}

/*-------------------------------------------------------------------------*
  Name        : NHTTP_StartConnection
  
  Description : 現在の設定で接続ハンドルの通信を開始する
                開始された接続ハンドルはタスクリストの終端に追加され
                NHTTPライブラリ内部の通信スレッドの管理下に置かれる
                
  Arguments   : handle  - 接続ハンドル
  
  Returns     : 通信が開始できれば 0, そうでなければ -1
 *-------------------------------------------------------------------------*/
extern NHTTPError NHTTP_StartConnection(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
    
    DEBUG_PRINTF("[%s]\n", __func__);
    
    if (connection_p == NULL) return NHTTP_ERROR_SYSTEM; 
    if (connection_p->req_p == NULL) return NHTTP_ERROR_SYSTEM; 
    
    connection_p->reqId = NHTTPi_SendRequestAsync(sysInfo_p, connection_p->req_p);
    if (connection_p->reqId >= 0)
    {
        /* リクエストハンドルは通信開始とともにユーザ管理を離れる */
        connection_p->status = NHTTP_STATUS_CONNECTING;

        NHTTPi_addRef(mutexInfo_p, connection_p);

        // ↓相互参照できなくなるのでNULLを入れては駄目
        //connection_p->req_p = NULL;
    }
    
    return NHTTP_ERROR_NONE;
}

/*-------------------------------------------------------------------------*
  Name        : NHTTP_CancelConnection
  
  Description : 接続ハンドルの通信を中止する
  
  Arguments   : handle  - 接続ハンドル
  
  Returns     : 通信が中止できれば 0, そうでなければ -1
 *-------------------------------------------------------------------------*/
extern NHTTPError NHTTP_CancelConnection(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*    connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
    
    DEBUG_PRINTF("[%s][%d]\n", __func__, __LINE__);
    
    if (connection_p == NULL)  return NHTTP_ERROR_SYSTEM; 
    
    if (connection_p->reqId >= 0)
    {
        NHTTPi_CancelRequestAsync(sysInfo_p, connection_p->reqId);
        // ↓再開できるわけではないので-1にする必要はない
        //connection_p->reqId = -1;
        return NHTTP_ERROR_NONE;
    }
    else
    {
        return NHTTP_ERROR_SYSTEM;
    }
}

/*---------------------------------------------------------------------------*
  Name        : NHTTP_GetConnectionStatus

  Description : 指定された接続ハンドルの状態を取得

  Arguments   : handle        - 接続ハンドル
 *---------------------------------------------------------------------------*/
extern NHTTPConnectionStatus NHTTP_GetConnectionStatus(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);

    if (connection_p == NULL)
    {
        return NHTTP_STATUS_ERROR;
    }
    else
    {
        return connection_p->status;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetBodyBuffer

  Description:  HTTPボディバッファ先頭ポインタ取得

  Arguments:    handle  - 接続ハンドル
                value	- 取得結果のアドレスを格納するためのポインタへのポインタ

  Returns:      0以上	取得したHTTPボディの長さ(Byte単位)
                0未満	HTTPボディを取得できない
 *-------------------------------------------------------------------------*/
extern int NHTTP_GetBodyBuffer(NHTTPConnectionHandle handle, char** value_pp, u32* size_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
    
    if (connection_p != NULL)
    {
        NHTTPRes* res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);
        
        if (res_p != NULL)
        {
            *value_pp = res_p->recvBuf_p;
            *size_p = res_p->recvBufLen;
            return res_p->bodyLen;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetUserParam

  Description:  ユーザーパラメーターを取得
  
  Arguments:    handle  - 接続ハンドル
  
  Returns:      パラメータのポインタ
 *-------------------------------------------------------------------------*/
extern void* NHTTP_GetUserParam(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);

    if (connection_p != NULL)
    {
        NHTTPRes* res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);
        
        if (res_p != NULL)
        {
            return res_p->param_p;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetConnectionError

  Description:  エラーを取得
  
  Arguments:    handle  - 接続ハンドル
  
  Returns:      エラー値
 *-------------------------------------------------------------------------*/
extern NHTTPError NHTTP_GetConnectionError(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);

    if (connection_p != NULL)
    {
        return connection_p->nhttpError;
    }
    else
    {
        return NHTTP_ERROR_SYSTEM;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetConnectionSSLError

  Description:  SSLエラーを取得
  
  Arguments:    handle  - 接続ハンドル
  
  Returns:      SSLエラー値 
 *-------------------------------------------------------------------------*/
extern int NHTTP_GetConnectionSSLError(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
    
    if (connection_p != NULL)
    {
       return connection_p->sslError;
    }
    else
    {
        return -1;
    }
}
/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetConnectionSSLError

  Description:  進行度(受信バイト数およびContent-Lengthの値)を取得する
  
  Arguments:    handle  - 接続ハンドル
  
  Returns:      HTTPリクエスト進行度取得に成功できれば 0, そうでなければ -1
 *-------------------------------------------------------------------------*/
extern NHTTPError NHTTP_GetConnectionProgress(NHTTPConnectionHandle handle, u32* received_p, u32* contentlen_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
    
    if (connection_p != NULL)
    {
        NHTTPRes* res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);
        
        if (res_p != NULL)
        {
            *received_p = (u32)res_p->totalBodyLen;
//            *received = (u32)res_p->bodyLen;
            *contentlen_p = (u32)((res_p->contentLength == 0xffffffff)? 0 : res_p->contentLength);
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
  Name:         NHTTP_CallAlloc

  Description:  ユーザー指定の Alloc関数呼び出し
  
  Arguments:    size  - 確保要求サイズ
                align - バッファアライメント
  
  Returns:      成功できればポインタを, そうでなければ NULL
 *-------------------------------------------------------------------------*/
extern void* NHTTP_CallAlloc(u32 size, int align)
{
    return NHTTPi_alloc(size, align);
}
/*-------------------------------------------------------------------------*
  Name:         NHTTP_CallFree

  Description:  ユーザー指定の free関数呼び出し
  
  Arguments:    ptr - NHTTPCallAllocで確保した領域の先頭ポインタ
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTP_CallFree(void* ptr_p)
{
    NHTTPi_free(ptr_p);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_SetSocketBufferSize

  Description:  Socketが使用する内部バッファサイズの指定
  
  Arguments:    size - Socketが使用する内部バッファサイズ
  
  Returns:      
 *-------------------------------------------------------------------------*/
extern NHTTPError NHTTP_SetSocketBufferSize(NHTTPConnectionHandle handle, u32 size)
{
#if 1
//noma 2006.11.08
// 旧仕様対応
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);

    if (req_p == NULL)
    {
        return NHTTP_ERROR_SYSTEM;
    }

    req_p->soRecvSize = size;
#else
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
   
    if (connection_p == NULL)  return NHTTP_ERROR_SYSTEM;
    
    connection_p->soRecvSize = size;
#endif
    
    return NHTTP_ERROR_NONE;
}
