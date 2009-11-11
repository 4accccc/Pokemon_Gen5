/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     d_nhttp.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-08#$
  $Rev: 1183 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include "nhttp.h"
#include "NHTTPlib_int.h"
#include "NHTTPlib_int_socket.h"
#include "d_nhttp_common.h"
#include "NHTTP_control.h"
#include "NHTTP_request.h"
#include "NHTTP_response.h"
#include "NHTTP_thread.h"
#include "NHTTP_nonport.h"


#if defined(NHTTP_FOR_RVL)
/* ライブラリバージョン文字列を使用するためのヘッダとマクロ */
#include <revolution/revodefs.h>
#include <revolution/rexdefs.h>

REVOLUTION_LIB_VERSION( NHTTP );
REX_UNOFFICIAL_FUNC_VERSION( NHTTP );

/* 同じ文字列を何回もデバッグ出力しないためのチェック変数 */
static BOOL nhttpRegistered = FALSE;
static BOOL nhttpRegisteredUnofficial = FALSE;
#endif

/*-------------------------------------------------------------------------*
  Name:         NHTTP_Startup

  Description:  NHTTPライブラリを初期化する
                 
  Arguments:    Alloc           NHTTPライブラリ内部で使用されるメモリブロック確保関数へのポインタ
                Free            NHTTPライブラリ内部で使用されるメモリブロック解放関数へのポインタ
                threadprio      HTTPリクエストスレッドの優先度(0から31の範囲、0が最高優先度)

  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTP_Startup(NHTTPAlloc alloc, NHTTPFree free, u32 threadprio)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();

#if defined(NHTTP_FOR_RVL)
   if( !nhttpRegistered )
    {
        OSRegisterVersion( __NHTTPVersion );
        nhttpRegistered = TRUE;
    }
#endif
    
    // NHTTP_bgnend.c 参照
    return (NHTTPi_Startup(sysInfo_p, alloc, free, threadprio) ? 0 : -1);
}
/*-------------------------------------------------------------------------*
  Name:         NHTTP_CleanupAsync

  Description:  NHTTPライブラリの解放を要求する

  Arguments:    Callback	NHTTPライブラリの解放処理が完了した際に呼び出されるコールバック関数へのポインタ

  Returns:      なし
 *-------------------------------------------------------------------------*/
extern void NHTTP_CleanupAsync(NHTTPCleanupCallback callback)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    // NHTTP_bgnend.c 参照
    NHTTPi_CleanupAsync(sysInfo_p, callback);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_Cleanup

  Description:  NHTTPライブラリの解放を要求する

  Arguments:    なし

  Returns:      なし
 *-------------------------------------------------------------------------*/
extern void NHTTP_Cleanup(void)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    // NHTTP_bgnend.c 参照
    NHTTPi_CleanupAsync(sysInfo_p, NULL);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetError

  Description:  NHTTPライブラリ内で最後に発生したエラーの状況を取得する

  Arguments:    なし

  Returns:      NHTTPError	この関数を呼び出す直前にNHTTPライブラリ内で発生したエラーの状況
 *-------------------------------------------------------------------------*/
extern NHTTPError NHTTP_GetError(void)
{
    // NHTTP_bgnend.c 参照
    return NHTTPi_GetError(&(NHTTPi_GetSystemInfoP()->bgnEndInfo));
}
/*-------------------------------------------------------------------------*
  Name:         NHTTP_SSLGetError

  Description:  NHTTPライブラリ内で最後に発生したSSLエラーの状況を取得する

  Arguments:    なし

  Returns:      この関数を呼び出す直前にNHTTPライブラリ内で発生したSSLエラーの状況
 *-------------------------------------------------------------------------*/
extern int NHTTP_SSLGetError(void)
{
    NHTTPBgnEndInfo*	bgnEndInfo_p = &(NHTTPi_GetSystemInfoP()->bgnEndInfo);
    NHTTPi_ASSERT(NHTTPi_IsOpened(bgnEndInfo_p));
    
    return NHTTPi_GetSSLError(bgnEndInfo_p);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_PostSendCallbackWrap

  Description:  [旧仕様互換用] POSTデータの送信をストリーミングで行うためのコールバックラップ関数

  Arguments:    connection_p	対象の接続ハンドル
                
  Returns:      
 *-------------------------------------------------------------------------*/
static int NHTTPi_PostSendCallbackWrap(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p, void* arg_p)
{
    if (connection_p != NULL)
    {
        NHTTPReq*	req_p = NHTTPi_Connection2Request(mutexInfo_p, connection_p);

        if (req_p != NULL)
        {
            NHTTPPostSend	postSend = req_p->postSendCallback;

            if (postSend != NULL)
            {
                NHTTPPostSendArg*	postSendArg_p = (NHTTPPostSendArg*)arg_p;

                return postSend(postSendArg_p->label, &(postSendArg_p->buf), &(postSendArg_p->size), (int)(postSendArg_p->offset), NHTTP_GetUserParam(connection_p));
            }
        }
    }
    return -1;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_BufFullCallbackWrap

  Description:  [旧仕様互換用] ボディ受信バッファが溢れた場合にコールされるコールバックラップ関数

  Arguments:    connection_p	対象の接続ハンドル
                
  Returns:      
 *-------------------------------------------------------------------------*/
static int NHTTPi_BufFullCallbackWrap(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p, void* arg_p)
{
    if (connection_p != NULL)
    {
        NHTTPRes*	res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);

        if (res_p != NULL)
        {
            NHTTPBufFull	bufFull = res_p->bufFull;

            if (bufFull != NULL)
            {
                NHTTPBodyBufArg*	bodyBufArg_p = (NHTTPBodyBufArg*)arg_p;
                char*			oldBuf_p = bodyBufArg_p->buf;
                u32			contentLength = NHTTPi_GetVirtualContentLength(connection_p);

                bodyBufArg_p->buf = bufFull(&oldBuf_p, &(bodyBufArg_p->size), contentLength, NHTTPi_alloc, NHTTPi_free, NHTTP_GetUserParam(connection_p));
                if ((bodyBufArg_p->buf != NULL) && (oldBuf_p != NULL))
                {
                    bodyBufArg_p->offset = 0;
                }
            }
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_CompleteCallbackWrap

  Description:  [旧仕様互換用] HTTP通信終了時に呼び出されるコールバックラップ関数

  Arguments:    connection_p	対象の接続ハンドル
                
  Returns:      
 *-------------------------------------------------------------------------*/
static int NHTTPi_CompleteCallbackWrap(NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p)
{
    if (connection_p != NULL)
    {
        NHTTPReqCallback	reqCallback = connection_p->reqCallback;

        if (reqCallback != NULL)
        {
            NHTTPRes*		res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);

            if (res_p != NULL)
            {
                NHTTPError	result = NHTTP_GetConnectionError(connection_p);

                reqCallback(result, res_p, NHTTP_GetUserParam(connection_p));
            }
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_TemplateConnectionCallback

  Description:  [旧仕様互換用]イベント通知コールバック

  Arguments:    handle	イベント通知対象の接続ハンドル
  		event	イベント種別
                arg_p	イベントごとに異なるコールバック情報構造体へのポインタ
                
  Returns:      
 *-------------------------------------------------------------------------*/
static int NHTTPi_TemplateConnectionCallback(NHTTPConnectionHandle handle, NHTTPConnectionEvent event, void* arg_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);
    int		ret = 0;

    switch (event)
    { 
    case NHTTP_EVENT_POST_SEND:
        ret = NHTTPi_PostSendCallbackWrap(mutexInfo_p, connection_p, arg_p);
        break;
    case NHTTP_EVENT_BODY_RECV_FULL:
        ret = NHTTPi_BufFullCallbackWrap(mutexInfo_p, connection_p, arg_p);
        break;
    case NHTTP_EVENT_BODY_RECV_DONE:
        //旧仕様ではない
        break;
    case NHTTP_EVENT_COMPLETE:
        ret = NHTTPi_CompleteCallbackWrap(mutexInfo_p, connection_p);
        break;
    default:
        break;
    }
    return ret;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_CreateRequest

  Description:  [旧仕様互換用] 引数で指定されたパラメータが設定されたNHTTPReqオブジェクトを生成する

  Arguments:    url_p		接続先URL(NULL終端ASCII文字列)へのポインタ
                method		実行するHTTPリクエストメソッド
                buf_p		HTTPボディ受信バッファへのポインタ
                len		HTTPボディ受信バッファの長さ
                callback	HTTP通信終了時コールバック関数へのポインタ
                param_p		HTTP通信終了時コールバック関数に与えられるパラメータ
                
  Returns:      NULL            処理に失敗した
                NULL以外        確保されたNHTTPReqオブジェクトへのポインタ
 *-------------------------------------------------------------------------*/
extern NHTTPRequestHandle NHTTP_CreateRequest(const char*      url_p,
                                              NHTTPReqMethod   method,
                                              char*            buf_p,
                                              u32              len,
                                              NHTTPReqCallback callback,
                                              void*            param_p)
{
    return NHTTP_CreateRequestEx(url_p, method, buf_p, len, callback, param_p, (NHTTPBufFull)NULL, (NHTTPFreeBuf)NULL);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_CreateRequestEx

  Description:  [旧仕様互換用] 引数で指定されたパラメータが設定されたNHTTPReqオブジェクトを生成する

  Arguments:    url_p		接続先URL(NULL終端ASCII文字列)へのポインタ
                method		実行するHTTPリクエストメソッド
                buf_p		HTTPボディ受信バッファへのポインタ
                len		HTTPボディ受信バッファの長さ
                callback	HTTP通信終了時コールバック関数へのポインタ
                param_p		HTTP通信終了時コールバック関数に与えられるパラメータ
                bufFull		HTTPボディ受信バッファ溢れコールバック関数ポインタ
                
  Returns:      NULL            処理に失敗した
                NULL以外        確保されたNHTTPReqオブジェクトへのポインタ
 *-------------------------------------------------------------------------*/
extern NHTTPRequestHandle NHTTP_CreateRequestEx(const char*      url_p,
                                                NHTTPReqMethod   method,
                                                char*            buf_p,
                                                u32              len,
                                                NHTTPReqCallback callback,
                                                void*            param_p,
                                                NHTTPBufFull     bufFull,
                                                NHTTPFreeBuf     freeBuf)
{
    NHTTPConnectionHandle	handle = NHTTP_CreateConnection(url_p, method, buf_p, len, &NHTTPi_TemplateConnectionCallback, param_p);
    NHTTPReq*			req_p = NULL;

    if (handle != NULL)
    {
        NHTTPSysInfo*		sysInfo_p = NHTTPi_GetSystemInfoP();
        NHTTPMutexInfo*		mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
        NHTTPConnection*	connection_p = NHTTPi_GetConnection(mutexInfo_p, handle);

        req_p = NHTTPi_Connection2Request(mutexInfo_p, connection_p);

        // 旧仕様用コールバック関数の再設定
        if (req_p != NULL)
        {
            if (req_p->res_p != NULL)
            {
                connection_p->reqCallback = callback;
                req_p->res_p->bufFull = bufFull;
                req_p->res_p->freeBuf = freeBuf;
                return req_p;
            }
            else
            {
                // エラー時は確保したコネクションを削除
                NHTTPi_DestroyRequest(sysInfo_p, req_p);
                (void)NHTTPi_OmitConnectionList(mutexInfo_p, connection_p);
                NHTTPi_free(connection_p);
                connection_p = NULL;
                req_p = NULL;
            }
        }
    }
    return (NHTTPRequestHandle)req_p;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_DeleteRequest
  
  Description:  [旧仕様互換用] 引数で指定されたNHTTPReqオブジェクトが使用しているメモリを解放する
  
  Arguments:    req	処理対象のHTTPReqHandle
  
  Returns:      なし
 *-------------------------------------------------------------------------*/
extern void NHTTP_DeleteRequest(NHTTPRequestHandle req)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_Request2Connection(mutexInfo_p, req);

    if (connection_p != NULL)
    {
        //接続ハンドルの開放
        NHTTP_DeleteConnection(connection_p);
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_AddHeaderField

  Description:  HTTPリクエスト実行時に送信するHTTPヘッダ項目を追加する

  Arguments:    handle	処理対象の NHTTPConnectionHandle
                label	追加するHTTPヘッダ項目のラベル(NULL終端ASCII文字列)へのポインタ
                value	追加するHTTPヘッダ項目の値(NULL終端ASCII文字列)へのポインタ

  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTP_AddHeaderField(NHTTPConnectionHandle handle, const char* label_p, const char* value_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    
    return NHTTP_AddRequestHeaderField(req_p, label_p, value_p);
}
/*-------------------------------------------------------------------------*
  Name:         NHTTP_AddRequestHeaderField

  Description:  [旧仕様互換用] HTTPリクエスト実行時に送信するHTTPヘッダ項目を追加する
 *-------------------------------------------------------------------------*/
extern int NHTTP_AddRequestHeaderField(NHTTPReq* req_p, const char* label_p, const char* value_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    
    if (req_p == NULL) return -1;
    
    // エラー時の関数名を表示するため判定をNHTTP_AddHeaderFieldからこちらに移動
    if (req_p->isStarted)
    {
        PRINTF("%s can be called before NHTTPStartConnection()\n", __func__);
        return -1;
    }

    // NHTTP_control.c 参照
    return (NHTTPi_AddRequestHeaderField(req_p, bgnEndInfo_p, label_p, value_p) ? 0 : -1);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_AddPostDataAscii

  Description:  HTTPリクエスト実行時に送信するPOST項目(ASCII文字列)を追加する

  Arguments:    handle	処理対象の NHTTPConnectionHandle
                label	追加するPOST項目のラベル(NULL終端ASCII文字列)へのポインタ
                value	追加するPOST項目の値(NULL終端ASCII文字列)へのポインタ

  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTP_AddPostDataAscii(NHTTPConnectionHandle handle, const char* label_p, const char* value_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    
    return (NHTTPi_AddRequestPostDataAscii(req_p, bgnEndInfo_p, label_p, value_p) ? 0 : -1);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_AddRequestPostDataAscii

  Description:  [旧仕様互換用]HTTPリクエスト実行時に送信するPOST項目(ASCII文字列)を追加する
 *-------------------------------------------------------------------------*/
extern int NHTTP_AddRequestPostDataAscii(NHTTPReq* req_p, const char* label_p, const char* value_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    
    if (req_p == NULL) return -1;
    
    // エラー時の関数名を表示するため判定をNHTTP_AddPostDataAsciiからこちらに移動
    if (req_p->isStarted)
    {
        PRINTF("%s can be called before NHTTPStartConnection()\n", __func__);
        return -1;
    }

    // NHTTP_control.c 参照
    return (NHTTPi_AddRequestPostDataAscii(req_p, bgnEndInfo_p, label_p, value_p) ? 0 : -1);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPAddPostDataBinary

  Description:  HTTPリクエスト実行時に送信するPOST項目(バイナリデータ)を追加する

  Arguments:    handle	処理対象の NHTTPConnectionHandle
                label	追加するPOST項目のラベル(NULL終端ASCII文字列)へのポインタ
                value	追加するPOST項目の値(バイナリデータ)へのポインタ
                length	追加するPOST項目の値の長さ(Byte単位)

  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTP_AddPostDataBinary(NHTTPConnectionHandle handle, const char* label_p, const void* value_p, u32 length)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    return ( 0 == NHTTP_AddRequestPostDataBinary(req_p, label_p, (const char*)value_p, length) ? 0 : -1);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_AddRequestPostDataBinary

  Description:  [旧仕様互換用] HTTPリクエスト実行時に送信するPOST項目(バイナリデータ)を追加する
 *-------------------------------------------------------------------------*/
extern int NHTTP_AddRequestPostDataBinary(NHTTPReq* req_p, const char* label_p, const void* value_p, u32 length)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    
    if (req_p == NULL) return -1;
        
    // エラー時の関数名を表示するため判定をNHTTP_AddPostDataBinaryからこちらに移動
    if (req_p->isStarted)
    {
        PRINTF("%s can be called before NHTTPStartConnection()\n", __func__);
        return -1;
    }

    // NHTTP_control.c 参照
    return (NHTTPi_AddRequestPostDataBinary(req_p, bgnEndInfo_p, label_p, (const char*)value_p, length) ? 0 : -1);
    
}

// 2006/09/28 noma
/*-------------------------------------------------------------------------*
  Name:         NHTTP_AddPostDataRaw

  Description:  HTTPリクエスト実行時にPOSTで生のデータを送信する

  Arguments:    handle	処理対象の NHTTPConnectionHandle
                value_p	追加するPOST項目の値(バイナリデータ)へのポインタ
                length	追加するPOST項目の値の長さ(Byte単位)

  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTP_AddPostDataRaw(NHTTPConnectionHandle handle, const void* value_p, u32 length)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    
    if (req_p == NULL) return -1;
    
    if (req_p->isStarted)
    {
        PRINTF("%s can be called before NHTTPStartConnection()\n", __func__);
        return -1;
    }
    
    // noma 2006.09.28
    // 既に NHTTPAddPostDataBinary か NHTTPAddPostDataAscii が呼ばれている
    if (req_p->listPost_p != NULL) {
        PRINTF("already called NHTTPAddPostDataAscii or NHTTPAddPostDataBinary (exclusive fucntion)\n");
        return -1;
    }
    
    req_p->isRawData = TRUE;
    req_p->rawPostData_p = value_p;
    req_p->rawPostDataLen = length;
    return 0;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_SetRequestPostDataCallback

  Description:  HTTPリクエスト実行時に、
                POSTでデータを送信する際に呼ばれるコールバックをセットする

  Arguments:    req             処理対象のHTTPReqオブジェクトへのポインタ
                callback        データ送信時に呼ばれるコールバック関数へのポインタ
                
  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTP_SetRequestPostDataCallback(NHTTPRequestHandle req, NHTTPPostSend callback)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_Request2Connection(mutexInfo_p, req);

    if (connection_p == NULL) return -1;

    {
        NHTTPReq*	req_p = NHTTPi_GetRequest(mutexInfo_p, connection_p);

        if (req_p == NULL) return -1;
        if (req_p->isStarted) return -1;

        req_p->postSendCallback = callback;
    }
    return 0;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_SetPostDataEncoding

  Description:  生成したリクエストのenctypeを明示的に指定する。

  Arguments:    handle	処理対象の NHTTPConnectionHandle
                type	指定したいリクエストのenctype
                NHTTP_ENCODING_TYPE_AUTO,      自動判定(デフォルト) 
                NHTTP_ENCODING_TYPE_URL,       application/x-www-form-urlencoded 
                NHTTP_ENCODING_TYPE_MULTIPART  multipart/form-data 

  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTP_SetPostDataEncoding(NHTTPConnectionHandle handle, NHTTPEncodingType type)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    if (req_p == NULL) return -1;
    if (req_p->isStarted)
    {
        PRINTF("%s can be called before NHTTPStartConnection()\n", __func__);
        return -1;
    }
        
#if 1 // noma 2006.09.28
    // 既に NHTTPAddPostDataRaw が呼ばれている
    if (req_p->isRawData)   return -1;
#endif
    
    req_p->encodingType = type;   
    return 0;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_SendRequestAsync

  Description:  適切に設定されたNHTTPReqオブジェクトに従ってHTTPリクエストを実行する

  Arguments:    req	処理対象のHTTPReqオブジェクトへのポインタ
  
  Returns:      0以上	処理に成功した (値は実行中のHTTPリクエストに与えられるID)
                0未満	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTP_SendRequestAsync(NHTTPRequestHandle req)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_Request2Connection(mutexInfo_p, req);

    if (connection_p != NULL)
    {
        if (NHTTP_StartConnection(connection_p) == NHTTP_ERROR_NONE)
        {
            return connection_p->reqId;
        }
    }
    return -1;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_CancelRequestAsync

  Description:  NHTTPSendRequest関数から得られたIDを指定し、HTTPリクエストの停止を要求する

  Arguments:    id	中断するリクエストのid
  
  Returns:      0 	処理に成功した、あるいはIDで指定されたHTTPリクエストが存在しない
                -1	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTP_CancelRequestAsync(int id)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();

    DEBUG_PRINTF("[%s] %d\n", __func__, id);
    
    // NHTTP_request.c 参照
    return (NHTTPi_CancelRequestAsync(sysInfo_p, id) ? 0 : -1);
}

/*---------------------------------------------------------------------------*
  Name:         NHTTP_GetProgress

  Description:  現在進行中のHTTPリクエスト処理の進行度(受信バイト数およびContent-Lengthの値)を取得する

  Arguments:    received_p	受信済みHTTP BODY領域サイズを格納する領域へのポインタ
                contentlen_p	Content-Lengthを格納する領域へのポインタ

  Returns:      0 	HTTPリクエスト進行度取得に成功した
                -1	HTTPリクエストキューが空で、かつHTTPリクエストが実行されていない
 *---------------------------------------------------------------------------*/
extern int NHTTP_GetProgress(u32* received_p, u32* contentlen_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    // NHTTP_thread.c 参照
    return (NHTTPi_GetProgress(sysInfo_p, received_p, contentlen_p) ? 0 : -1);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_DestroyResponse

  Description:  HTTPレスポンスオブジェクトの使用するメモリを全て解放する

  Arguments:    res	処理対象のNHTTPResponseHandle
        
  Returns:      なし
 *-------------------------------------------------------------------------*/
extern void NHTTP_DestroyResponse(NHTTPResponseHandle res)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_Response2Connection(mutexInfo_p, res);

    DEBUG_PRINTF("[%s]\n", __func__);
    
    if (connection_p != NULL)
    {
        NHTTPRes*	res_p = NHTTPi_Connection2Response(mutexInfo_p, connection_p);

        if (res_p != NULL)
        {
            NHTTPi_ASSERT(connection_p->req_p == NULL);
            NHTTPi_DestroyResponse(mutexInfo_p, connection_p->res_p);
        }
        // リストから削除
         DEBUG_PRINTF("[%s] NHTTPi_OmitConnectionList\n", __func__);
         (void)NHTTPi_OmitConnectionList(mutexInfo_p, connection_p);
        NHTTPi_free(connection_p);
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetHeaderField

  Description:  HTTPヘッダの情報を、ラベル名を指定して取得する

  Arguments:    res	処理対象のNHTTPResponseHandle
                label	取得を試みるヘッダ情報のラベル名
                value	取得結果のアドレスを格納するためのポインタへのポインタ

  Returns:      0以上	取得したHTTPヘッダ情報の文字数
                0未満	指定したラベルはHTTPヘッダ内に存在しない
 *-------------------------------------------------------------------------*/
extern int NHTTP_GetHeaderField(NHTTPConnectionHandle handle, const char* label_p, char** value_pp)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPRes*		res_p = NHTTPi_GetResponse(mutexInfo_p, handle);
    if (res_p == NULL) return -1;

    // NHTTP_response.c 参照
    return NHTTP_GetResponseHeaderField(res_p, label_p, value_pp);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetHeaderAll

  Description:  HTTPヘッダ全体を取得する

  Arguments:    res	処理対象のNHTTPResponseHandle
                value	取得結果のアドレスを格納するためのポインタへのポインタ

  Returns:      0以上	取得したHTTPヘッダの長さ(Byte単位)
                0未満	HTTPヘッダを取得できない
 *-------------------------------------------------------------------------*/
extern int NHTTP_GetHeaderAll(NHTTPConnectionHandle handle, char** value_pp)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPRes*		res_p = NHTTPi_GetResponse(mutexInfo_p, handle);
    
    if (res_p == NULL) return -1;
    // NHTTP_response.c 参照
    return NHTTPi_GetResponseHeaderAll(bgnEndInfo_p, res_p, value_pp);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetBodyAll

  Description:  HTTPボディ全体を取得する

  Arguments:    res	処理対象のNHTTPResponseHandle
                value	取得結果のアドレスを格納するためのポインタへのポインタ

  Returns:      0以上	取得したHTTPボディの長さ(Byte単位)
                0未満	HTTPボディを取得できない
 *-------------------------------------------------------------------------*/
extern int NHTTP_GetBodyAll(NHTTPResponseHandle res, char** value_pp)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_Response2Connection(mutexInfo_p, res);

    if (connection_p != NULL)
    {
        u32	size = 0;

        return NHTTP_GetBodyBuffer(connection_p, value_pp, &size);
    }
    else
    {
        return -1;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTP_GetResultCode

  Description:  HTTPリザルトを取得する

  Arguments:    handle	処理対象の NHTTPConnectionHandle
  
  Returns:      0以上	取得したHTTPリザルト
                0未満	HTTPリザルトを取得できない
 *-------------------------------------------------------------------------*/
extern int NHTTP_GetResultCode(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPRes*		res_p = NHTTPi_GetResponse(mutexInfo_p, handle);
    if (res_p == NULL) return -1;
    
    NHTTPi_ASSERT(NHTTPi_IsOpened(NHTTPi_GetBgnEndInfoP(NHTTPi_GetSystemInfoP())));
    
    if (!res_p->isHeaderParse) return -1;
    
    return res_p->httpStatus;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTP_SetVerifyOption

  Arguments:    handle	処理対象の NHTTPConnectionHandle
                verifyOption
                 下記のビットを立てることで検証内容を指定可能 
                 SSL_VERIFY_COMMON_NAME  … 証明書のcommonnameがi_serverNameと一致するか調べる
                                            証明書のcommonnameはワイルドカードをサポートしている
                 SSL_VERIFY_ROOT_CA      … 証明書をROOTCAで検証する
                 SSL_VERIFY_CHAIN        … 証明書チェインが正しいかを検証する
                 SSL_VERIFY_DATE         … 証明書の期限切れも調査する
                 
  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetVerifyOption(NHTTPConnectionHandle handle, u32 verifyOption)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    
    if (req_p == NULL) return -1;
    
    req_p->verifyOption = verifyOption;
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTP_SetProxy

  Description:  指定したNHTTPReqオブジェクトに対してproxyサーバーを設定して使用する

  Arguments:    handle	処理対象の NHTTPConnectionHandle
                proxy   proxyサーバー アドレス
                port    ポート                
                
  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetProxy(NHTTPConnectionHandle handle, const char* proxy_p, int port, const char* username_p, const char* password_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    int			proxyLen = 0;
        
    if ((req_p == NULL) || (proxy_p == NULL))
    {
        return -1;
    }
    
    proxyLen = (int)NHTTPi_strnlen(proxy_p, LEN_PROXY_ADDR);
    if (proxyLen >= LEN_PROXY_ADDR)
    {
        PRINTF("proxy-address exceeded 255 characters\n");
        return -1;
    }
    NHTTPi_memclr(req_p->proxyAddr, LEN_PROXY_ADDR);
    NHTTPi_memcpy(req_p->proxyAddr, proxy_p, proxyLen);
    req_p->proxyPort = port;
    
    // ユーザー名
    if ((username_p != NULL) && (password_p != NULL))
    {
        int username_len = 0;
        int password_len = 0;
        username_len = (int)NHTTPi_strnlen(username_p, LEN_USERNAME);
        password_len = (int)NHTTPi_strnlen(password_p, LEN_PASSWORD);
        
        if (username_len >= LEN_USERNAME) {
            PRINTF("username exceeded 31 characters\n");
            return -1;
        }
        
        if (password_len >= LEN_PASSWORD) {
            PRINTF("password exceeded 31 characters\n");
            return -1;
        }
        
        {
            char tmpbuf[ LEN_USERNAME + LEN_PASSWORD + 1];
            NHTTPi_memclr(tmpbuf, sizeof(tmpbuf));
            NHTTPi_memcpy(tmpbuf, username_p, username_len);
            NHTTPi_memcpy(&(tmpbuf[username_len]), ":", 1);
            NHTTPi_memcpy(&(tmpbuf[(username_len+1)]), password_p, password_len);
            req_p->proxyUsernamePasswordLen = NHTTPi_Base64Encode(req_p->proxyUsernamePassword, tmpbuf);
        }
    }
    // プロキシ使用
    req_p->isProxy = TRUE;
    
    PRINTF("Using proxy server %s:%d (%s/%s).\n",
           proxy_p, port,
           username_p ? username_p : "[no-auth]",
           password_p ? password_p : "[no-auth]");
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTP_SetProxyDefault

  Description:  指定したNHTTPReqオブジェクトに対してデフォルトのproxyサーバーを設定して使用する

  Arguments:    handle	処理対象の NHTTPConnectionHandle
  
  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetProxyDefault(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    int			err = NHTTP_ERROR_NONE;
    
#if defined(NHTTP_FOR_RVL)
    const NCDProxyServerProfile*	proxyServerProfile = NULL;

    // SSL？
    if (req_p->isSSL)
    {
        proxyServerProfile = &(bgnEndInfo_p->ipConfig.proxy.ssl);
    }
    else
    {
        proxyServerProfile = &(bgnEndInfo_p->ipConfig.proxy.http);
    }

    // プロキシ設定が存在する
    if (proxyServerProfile->mode == NCD_PROXY_MODE_NORMAL)
    {
        const char* username_p = NULL, *password_p = NULL;
        if (proxyServerProfile->authType == NCD_PROXY_AUTHTYPE_BASIC)
        {
            username_p = (const char*)proxyServerProfile->username;
            password_p = (const char*)proxyServerProfile->password;
        }
        
        err = NHTTPSetProxy(req_p, (char*)proxyServerProfile->server,
                            proxyServerProfile->port,
                            username_p, password_p);
    }
#elif defined(NHTTP_FOR_NTR)
    DWCProxySetting proxySetting;
    if (DWC_GetProxySetting( &proxySetting ))
    {
        const char* username_p = NULL, *password_p = NULL;
        if (proxySetting.authType == DWC_PROXY_AUTHTYPE_BASIC)
        {
            username_p = (char*)proxySetting.authId;
            password_p = (char*)proxySetting.authPass;
        }
        err = NHTTP_SetProxy(req_p, (char*)proxySetting.hostName,
                             proxySetting.port,
                             username_p, password_p);
    }
#else
    err = NHTTP_ERROR_UNKNOWN;
#endif    
    if (err < NHTTP_ERROR_NONE)
    {
        PRINTF("NHTTPSetProxy failed.(%d)\n", err);
        return -1;
    }
    else
    {
        // セット成功
        return 0;
    }
}


/*---------------------------------------------------------------------------*
  Name:         NHTTP_SetBasicAuthorization

  Description:  指定したNHTTPReqオブジェクトに対してBasic認証のユーザー名とパスワードを設定する

  Arguments:    handle	処理対象の NHTTPConnectionHandle
                username ユーザー名
                password パスワード
  
  Returns:      0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetBasicAuthorization(NHTTPConnectionHandle handle, const char* username_p, const char* password_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);

    if ((req_p == NULL) || (username_p == NULL) || (password_p == NULL))
    {
        return -1;
    }
    
    // ユーザー名
    {
        int username_len = 0;
        int password_len = 0;
        
        username_len = (int)NHTTPi_strnlen(username_p, LEN_USERNAME + 1);
        password_len = (int)NHTTPi_strnlen(password_p, LEN_PASSWORD + 1);
        if ((username_len <= LEN_USERNAME) && (password_len <= LEN_PASSWORD))
        {
            char tmpbuf[ LEN_USERNAME + LEN_PASSWORD + 1];
            
            NHTTPi_memclr(tmpbuf, LEN_USERNAME + LEN_PASSWORD + 1);
            NHTTPi_memcpy(tmpbuf, username_p, username_len);
            NHTTPi_memcpy(&(tmpbuf[username_len]), ":", 1);
            NHTTPi_memcpy(&(tmpbuf[(username_len+1)]), password_p, password_len);
            req_p->basicUsernamePasswordLen = NHTTPi_Base64Encode(req_p->basicUsernamePassword, tmpbuf);
        }
        else
        {
            return -1;
        }
    }
    
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:        NHTTP_SetClientCert
  
  Description:  ASN.1フォーマットのクライアント証明書と秘密鍵を使用する
    
  Arguments:    handle	処理対象の NHTTPConnectionHandle
  
  Returns:       0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetClientCert(NHTTPConnectionHandle handle, const char* clientCertData_p, size_t clientCertSize, const char* privateKeyData_p, size_t privateKeySize)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);

    if (req_p == NULL)
    {
        return -1;
    }

    req_p->useClientCertDefault = FALSE;
    req_p->clientCertData_p = clientCertData_p;
    req_p->clientCertSize = clientCertSize;
    req_p->privateKeyData_p = privateKeyData_p;
    req_p->privateKeySize = privateKeySize;
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:        NHTTP_RemoveClientCert
  
  Description:  クライアント証明書と秘密鍵を無効にする
  
  Arguments:    handle	処理対象の NHTTPConnectionHandle
  
  Returns:       0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_RemoveClientCert(NHTTPConnectionHandle handle)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);

    if (req_p == NULL)
    {
        return -1;
    }

    req_p->useClientCertDefault = FALSE;
    req_p->clientCertData_p = NULL;
    req_p->clientCertSize = 0;
    req_p->privateKeyData_p = NULL;
    req_p->privateKeySize = 0;
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:        NHTTP_SetRootCA
  
  Description:  ASN.1フォーマットのrootCAを使用する
  
  Arguments:    handle	処理対象の NHTTPConnectionHandle
                rootCAData      … ASN.1形式のデータ
                rootCASize      … データサイズ 
  
  Returns:       0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetRootCA(NHTTPConnectionHandle handle, const char* rootCAData_p, size_t rootCASize)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    
    return NHTTP_SetRequestRootCA(req_p, rootCAData_p, rootCASize);
}

extern int NHTTP_SetRequestRootCA(NHTTPReq* req_p, const char* rootCAData_p, size_t rootCASize)
{
    if (req_p == NULL)
    {
        return -1;
    }

    req_p->useBuiltinRootCA = FALSE;
    req_p->rootCAId = 0;
    req_p->rootCAData_p = rootCAData_p;
    req_p->rootCASize = rootCASize;
    
    return 0;
}


/*---------------------------------------------------------------------------*
  Name:         NHTTP_DisableVerifyOptionForDebug
  
  Description:  証明書の検証内容を一部無効にする。
                デバッグ用途のみ。この関数をリンクした状態で ROM 提出はできない。
  
  Arguments:    handle	処理対象の NHTTPConnectionHandle
                verifyOption
                 下記のビットを立てることで無効にする検証内容を指定可能 
                 SSL_VERIFY_COMMON_NAME  … 証明書のcommonnameがi_serverNameと一致するか調べる
                                            証明書のcommonnameはワイルドカードをサポートしている
                 SSL_VERIFY_ROOT_CA      … 証明書をROOTCAで検証する
                 SSL_VERIFY_CHAIN        … 証明書チェインが正しいかを検証する
                 SSL_VERIFY_DATE         … 証明書の期限切れも調査する
                 SSL_VERIFY_SUBJECT_ALT_NAME … commonnameの検証を行う際に、証明書にsubjectAltNameの
                                            記述がある場合はそちらを優先して使用する

  Returns:        0     処理に成功した
                 -1     処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_DisableVerifyOptionForDebug(NHTTPConnectionHandle handle, u32 verifyOption)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    
    // エラー時の関数名を表示するため判定をNHTTP_AddHeaderFieldからこちらに移動
    if (req_p->isStarted)
    {
        PRINTF("%s can be called before NHTTPStartConnection()\n", __func__);
        return -1;
    }
    
#if defined(NHTTP_FOR_RVL)
    if( !nhttpRegisteredUnofficial )
    {
        OSRegisterVersion( __NHTTPUnofficialVersion );
        nhttpRegisteredUnofficial = TRUE;
    }
#endif
    
    // SSLDisableVerifyOptionForDebugセット
    bgnEndInfo_p->forDebugCallback = NHTTPi_DisableVerifyOptionForDebug;
    
    if (req_p == NULL) return -1;
    
    req_p->disableVerifyOption = verifyOption;
    
    return 0;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTP_SetBuiltinRootCA
  
  Description:  指定したSSL接続で組み込みのrootCAを使用する
  
  Arguments:    handle          … 処理対象の NHTTPConnectionHandle
                rootCAId        … 使用する組み込みrootCA ID
  
  Returns:       0      処理に成功した
                 -1     処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetBuiltinRootCA(NHTTPConnectionHandle handle, u32 rootCAId)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);

    if (req_p == NULL)
    {
        return -1;
    }   
    
    req_p->useBuiltinRootCA = TRUE;
    req_p->rootCAId = rootCAId;
    req_p->rootCAData_p = NULL;
    req_p->rootCASize = 0;
    return 0;    
}

/*---------------------------------------------------------------------------*
  Name:         NHTTP_SetBuiltinClientCert
  
  Description:  指定したSSL接続で組み込みのクライアント証明書と秘密鍵を使用する
  
  Arguments:    handle          … 処理対象の NHTTPConnectionHandle
                clientCertId    … 使用する組み込みクライアント証明書ID
               
  Returns:       0      処理に成功した
                 -1     処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetBuiltinClientCert(NHTTPConnectionHandle handle, u32 clientCertId)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*		req_p = NHTTPi_GetRequest(mutexInfo_p, handle);

    if (req_p == NULL)
    {
        return -1;
    }
    
    req_p->clientCertId = clientCertId;
    req_p->useClientCertDefault = TRUE;
    req_p->clientCertData_p = NULL;
    req_p->clientCertSize = 0;
    req_p->privateKeyData_p = NULL;
    req_p->privateKeySize = 0;
    return 0;
}

#ifdef NHTTP_USE_NSSL
/*---------------------------------------------------------------------------*
  Name:        NHTTP_SetRootCADefault
  
  Description:  デフォルトのrootCAを使用する
  
  Arguments:    handle	処理対象の NHTTPConnectionHandle
  
  Returns:       0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetRootCADefault(NHTTPConnectionHandle handle)
{
    return NHTTP_SetBuiltinRootCA(handle, NHTTP_SSL_ROOTCA_DEFAULT);
}

/*---------------------------------------------------------------------------*
  Name:        NHTTPSetClientCertDefault
  
  Description:  デフォルトのクライアント証明書と秘密鍵を使用する
  
  Arguments:    handle	処理対象の NHTTPConnectionHandle
  
  Returns:       0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetClientCertDefault(NHTTPConnectionHandle handle)
{
    return NHTTP_SetBuiltinClientCert(handle, NHTTP_SSL_CLIENTCERT_DEFAULT);
}
#endif

/*---------------------------------------------------------------------------*
  Name:        NHTTP_SetBuiltinRootCAAsDefault
  
  Description:  デフォルトのrootCAを使用する
  
  Arguments:    rootCAId        … 使用する組み込みrootCA ID
  
  Returns:       0 	処理に成功した
                -1	処理に失敗した
 *---------------------------------------------------------------------------*/
extern int NHTTP_SetBuiltinRootCAAsDefault(u32 rootCAId)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    bgnEndInfo_p->defaultRootCAId = rootCAId;
    return 0;
}

