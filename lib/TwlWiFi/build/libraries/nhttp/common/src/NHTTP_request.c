/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_request.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-01#$
  $Rev: 1192 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/
#include "d_nhttp_common.h"
#include "NHTTPlib_int_socket.h"
#include "NHTTP_request.h"
#include "NHTTP_bgnend.h"
#include "NHTTP_list.h"
#include "NHTTP_nonport.h"

const char NHTTPi_strMultipartBound[] = STR_POSTBOUND;

/*-------------------------------------------------------------------------*

  Name:         NHTTPi_InitRequestInfo

  Description:  NHTTPReqInfo構造体の初期設定
                 
  Arguments:    reqInfo_p  NHTTPReqInfo構造体へのポインタ

  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_InitRequestInfo(NHTTPReqInfo* reqInfo_p)
{
    reqInfo_p->reqCurrent = NULL;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_CreateRequest

  Description:  引数で指定されたパラメータが設定されたNHTTPReqオブジェクトを生成する

  Arguments:    url             接続先URL(NULL終端ASCII文字列)へのポインタ
                method          実行するHTTPリクエストメソッド
                buf             HTTPボディ受信バッファへのポインタ
                len             HTTPボディ受信バッファの長さ
                callback	HTTP通信終了時コールバック関数へのポインタ
                param           HTTP通信終了時コールバック関数に与えられるパラメータ
                
  Returns:      NULL            処理に失敗した
                NULL以外        確保されたNHTTPReqオブジェクトへのポインタ
 *-------------------------------------------------------------------------*/
extern NHTTPReq* NHTTPi_CreateRequest(NHTTPBgnEndInfo*	bgnEndInfo_p,
                                     const char*	url_p,
                                     NHTTPReqMethod	method,
                                     char*		buf_p,
                                     u32		len,
                                     void*		param_p,
                                     NHTTPBufFull	bufFull,
                                     NHTTPFreeBuf	freeBuf)
{
    // 要求オブジェクトを準備
    // スレッド開始はまだ
    int i;
    int suflen, urllen, prelen;
    int nhostenc, deccount;
    int declen, hostend;
    int port;
    int decode_urllen;
    
    char	a_url[LEN_URLBUF];
    char*	prefix_p;
    NHTTPReq*	req_p = NULL;

    NHTTPi_ASSERT(NHTTPi_IsOpened(bgnEndInfo_p));
    NHTTPi_ASSERT(url_p);
    
    switch (method)
    {
    case NHTTP_REQMETHOD_GET:
    case NHTTP_REQMETHOD_POST:
    case NHTTP_REQMETHOD_HEAD:
        break;
    default:
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_UNKNOWN);
        goto errend;
    }
    
    //
    // メモリ確保
    //
    req_p = (NHTTPReq*)NHTTPi_alloc(sizeof(NHTTPReq), 4);

    if (!req_p)
    {
        DEBUG_PRINTF("[%s][line:%d] NHTTP_ERROR_ALLOC\n", __func__, __LINE__);
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        goto errend;
    }
    
    NHTTPi_memclr(req_p, sizeof(NHTTPReq));
    
    req_p->res_p = (NHTTPRes*)NHTTPi_alloc(sizeof(NHTTPRes), 4);

    if (!req_p->res_p)
    {
        DEBUG_PRINTF("[%s][line:%d] NHTTP_ERROR_ALLOC\n", __func__, __LINE__);
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        goto errend;
    }
    
    NHTTPi_memclr(req_p->res_p, sizeof(NHTTPRes));
    req_p->res_p->recvBuf_p = buf_p;
    req_p->res_p->recvBufLen = len;
    req_p->res_p->bufFull = bufFull;
    req_p->res_p->freeBuf = freeBuf;

    //
    // URLの解析
    //
    urllen = (int)NHTTPi_strnlen(url_p, sizeof(a_url) - 1);
    
    // "http://"
    if (urllen <= 7)
    {                
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
        goto errend;
    }
    
    // 一時URL領域にコピー
    NHTTPi_memclr(a_url, sizeof(a_url));
    NHTTPi_memcpy(a_url, url_p, urllen);
    
    req_p->port = NHTTP_HTTP_PORT;
    suflen = 7;
    
    if (NHTTPi_strnicmp(a_url, "http://", 7) != 0)
    {
        if (NHTTPi_strnicmp(a_url, "https://", 8) != 0)
        {
            NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
            goto errend;
        }        
        req_p->isSSL = TRUE;
        req_p->port = NHTTP_HTTPS_PORT;
        suflen = 8;
    }

    prefix_p = a_url + suflen;
    prelen = urllen - suflen;
    
    if (prelen <= 0)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
        goto errend;
    }

    // host,port部分だけ%xxをデコード後の文字数を求めるために
    // host,port部分の%xxの数を求める
    // host,port以外の%xxは接続先に素通し
    for (i = 0, nhostenc = 0, deccount = 0; (i < prelen) && (prefix_p[i] != '/'); i++)
    {
        if (deccount == 2)
        {
            deccount--;
        }
        else if (deccount == 1)
        {
            signed char c = (signed char)NHTTPi_strToHex(&prefix_p[i-1], 2);
            deccount--;
            if (c < 0)
            {
                NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
                goto errend;
            }
            if (c == '/')
            { 
                nhostenc--;
                break;
            }
        }
        else
        {
            if (prefix_p[i] == '%')
            {
                deccount = 2;
                nhostenc++;
            }
        }
    }
    
    if (deccount)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
        goto errend;
    }

    decode_urllen = suflen+prelen-nhostenc*2+1;
    
    // ワークエリアを確保して、host,portの%xxだけデコードしたURLをコピー
    req_p->URL_p = (char*)NHTTPi_alloc( (size_t)decode_urllen, 4 );

    if (!req_p->URL_p)
    {
        DEBUG_PRINTF("[%s][line:%d] NHTTP_ERROR_ALLOC\n", __func__, __LINE__);
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        goto errend;
    }
    NHTTPi_memclr(req_p->URL_p, decode_urllen);
    NHTTPi_memcpy(req_p->URL_p, a_url, suflen);
    
    for (i=0,declen=0,deccount=0,hostend=FALSE; i<prelen; i++)
    {
        if (deccount == 2)
        {
            deccount--;
        }
        else if (deccount == 1)
        {
            char c = (char)NHTTPi_strToHex(&prefix_p[i-1], 2);
            deccount--;
            req_p->URL_p[ suflen+declen-1 ] = c;
            if (c == '/')
            {
                hostend = TRUE;
            }
        }
        else
        {
            if (prefix_p[i] == '/')
            {
                hostend = TRUE;
            }
            if (!hostend & (prefix_p[i] == '%' ))
            {
                deccount = 2;
            }
            else
            {
                req_p->URL_p[ suflen+declen ] = prefix_p[i];
            }
            declen++;
        }
    }
    
    // URL_pの終端のNULL文字
    req_p->URL_p[ suflen + declen ] = 0;
    DEBUG_PRINTF("[%s] URL_p [%s]\n", __func__, req_p->URL_p);

    prefix_p = &req_p->URL_p[ suflen ];
    prelen = declen;
    
    // URLからホスト名だけを取り出す
    for (i=0; i<prelen; i++)
    {
        if ((prefix_p[i] == '/') || (prefix_p[i] == ':'))
        {
            req_p->hostLen = i + suflen;
            break;
        }
    }
    
    if (i == prelen)
    {
        req_p->hostDescLen = req_p->hostLen = i + suflen;
    }
    else
    {
        if (prefix_p[i] == '/')
        {
            req_p->hostDescLen = req_p->hostLen;
        }
        else if (prefix_p[i] == ':')
        {
            // ポート番号を取り出す
            for ( ; i<prelen; i++ )
            {
                if (prefix_p[i] == '/')
                {
                    req_p->hostDescLen = i + suflen;
                    break;
                }
            }

            if (i == prelen)
            {
                req_p->hostDescLen = i + suflen;
            }
            else
            {
                port = NHTTPi_strtonum( &req_p->URL_p[req_p->hostLen+1], (int)(req_p->hostDescLen - (req_p->hostLen+1)) );

                if (port < 0)
                {
                    port = req_p->port;
                }
                else if (port > 65535)
                {
                    NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_DNS);
                    goto errend;
                }
                
                req_p->port = (unsigned short)port;
            }
        }
    }

    // ワークエリアを確保してHost名を保存
    {
        size_t a_hostnameLen = (u32)(req_p->hostLen - (req_p->isSSL ? 8 : 7));
        req_p->hostname_p = (char*)NHTTPi_alloc(a_hostnameLen + 1, 4);
        
        if (!req_p->hostname_p)
        {
            DEBUG_PRINTF("[%s][line:%d] NHTTP_ERROR_ALLOC\n", __func__, __LINE__);
            NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
            goto errend;
        }
        
        NHTTPi_memclr(req_p->hostname_p, (int)(a_hostnameLen + 1));
        NHTTPi_memcpy(req_p->hostname_p, (req_p->URL_p + (req_p->isSSL ? 8 : 7)), (int)a_hostnameLen);
        // hostname_pの終端のNULL文字
        //req_p->hostname_p[a_hostnameLen] = '\0';
        DEBUG_PRINTF("[%s] hostname [%s]\n", __func__, req_p->hostname_p);
    }
    
    // その他
    NHTTPi_memcpy(req_p->tagPost, NHTTPi_strMultipartBound, sizeof(req_p->tagPost));
    //req_p->param = param;
    req_p->method = method;
//    req_p->pCallback = callback;

    // SSL
    req_p->sslId = 0;
    req_p->clientCertData_p = NULL;
    req_p->clientCertSize = 0;
    req_p->privateKeyData_p = NULL;
    req_p->privateKeySize = 0;
    req_p->rootCAData_p = NULL;
    req_p->rootCASize = 0;
    req_p->useClientCertDefault = FALSE;
    
    req_p->verifyOption = NHTTP_SSL_VERIFY_NONE;
    req_p->disableVerifyOption = NHTTP_SSL_VERIFY_NONE;

    // resにもparamを渡しておく
    req_p->res_p->param_p = param_p;

    // PROXY
    req_p->isProxy = FALSE;

    // soRecvSize初期設定
    req_p->soRecvSize = 0;
    
    return req_p;
    
errend:
    
    if (req_p)
    {
        if (req_p->URL_p)
        {
            NHTTPi_free(req_p->URL_p);
        }

        if (req_p->hostname_p)
        {
            NHTTPi_free(req_p->hostname_p);
        }

        if (req_p->res_p)
        {
            NHTTPi_free(req_p->res_p);
        }
        
        NHTTPi_free(req_p);
    }
    
    return NULL;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_DestroyRequest
  Description:  引数で指定されたNHTTPReqオブジェクトが使用しているメモリを解放する
  Arguments:    req	処理対象のHTTPReqオブジェクトへのポインタ
  Returns:      なし
 *-------------------------------------------------------------------------*/
extern void NHTTPi_DestroyRequest(NHTTPSysInfo* sysInfo_p, NHTTPReq* req_p)
{
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);

    NHTTPi_ASSERT(NHTTPi_IsOpened(NHTTPi_GetBgnEndInfoP(sysInfo_p)));
    NHTTPi_ASSERT(req_p);
    NHTTPi_ASSERT(!req_p->isStarted);

    /** 相互参照解除 */
    {
        NHTTPConnection* connection_p = NHTTPi_Request2Connection(mutexInfo_p, req_p);
        
        if (connection_p != NULL)
        {
            connection_p->res_p = NULL;
        }
    }
    
    NHTTPi_free(req_p->res_p);
    NHTTPi_destroyRequestObject(mutexInfo_p, req_p);
}

/*-------------------------------------------------------------------------*
  Name:         deleteStrList
  Description:  指定したリストを削除 
  Arguments:    list_p
 *-------------------------------------------------------------------------*/
static void deleteStrList(NHTTPi_DATALIST* list_p)
{
    NHTTPi_DATALIST *p = NULL;
    
    while (list_p)
    {
        if (list_p != list_p->prev_p)
        {
            p = list_p->prev_p->prev_p;
            NHTTPi_free(list_p->prev_p);
            list_p->prev_p = p;
        }
        else
        {
            NHTTPi_free(list_p);
            list_p = NULL;
        }
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_destroyRequestObject
  Description:  要求オブジェクトを破棄
                reqは実行中リストに入っていてはならない
                通常、リクエストが終了するときに、通信スレッドが呼ぶ
  Arguments:    req
  Returns       TRUE
 *-------------------------------------------------------------------------*/
extern int NHTTPi_destroyRequestObject(NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p)
{
    /* 相互参照解除 */
    {
        NHTTPConnection* connection_p = NHTTPi_Request2Connection(mutexInfo_p, req_p);
        
        if (connection_p != NULL)
        {
            connection_p->req_p = NULL;
        }
    }
    
    deleteStrList(req_p->listHeader_p);
    deleteStrList(req_p->listPost_p);
    
    NHTTPi_free(req_p->URL_p);
    NHTTPi_free(req_p->hostname_p);
    NHTTPi_free(req_p);
    // req_p->res_pは解放しない
    return TRUE;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_SendRequestAsync
  Description:  適切に設定されたNHTTPReqオブジェクトに従ってHTTPリクエストを実行する
  Arguments:    req	処理対象のHTTPReqオブジェクトへのポインタ
  Returns:      0以上	処理に成功した (値は実行中のHTTPリクエストに与えられるID)
                0未満	処理に失敗した
 *-------------------------------------------------------------------------*/
extern int NHTTPi_SendRequestAsync(NHTTPSysInfo* sysInfo_p, NHTTPReq* req_p)
{
    // リクエストをキューリストに加える
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    int			id;

    NHTTPi_ASSERT(NHTTPi_IsOpened(bgnEndInfo_p));
    NHTTPi_ASSERT(req_p);

    // 既に開始されていた
    if (req_p->isStarted)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_UNKNOWN);
        return -1;
    }
    
    NHTTPi_lockReqList(mutexInfo_p);
    {
        NHTTPListInfo*	listInfo_p = NHTTPi_GetListInfoP(sysInfo_p);
        id = NHTTPi_setReqQueue(listInfo_p, req_p);
        if ( id >= 0 )
        {
            req_p->isStarted = TRUE;
            NHTTPi_kickCommThread(threadInfo_p);
        }
        else
        {
            DEBUG_PRINTF("[%s][line:%d] NHTTP_ERROR_ALLOC\n", __func__, __LINE__);
            NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        }
    }
    
    NHTTPi_unlockReqList(mutexInfo_p);
    
    return id;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_CancelRequestAsync
  Description:  NHTTP_SendRequest関数から得られたIDを指定し、HTTPリクエストの停止を要求する
  Arguments:    id	中断するリクエストのid
  Returns:      TRUE	処理に成功した、あるいはIDで指定されたHTTPリクエストが存在しない
                FALSE	処理に失敗した
 *-------------------------------------------------------------------------*/
extern BOOL NHTTPi_CancelRequestAsync(NHTTPSysInfo* sysInfo_p, int id)
{
    // スレッドに中断要請を発行する
    int rc = FALSE;
    NHTTPReqInfo*       	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPMutexInfo*		mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    volatile NHTTPi_REQLIST*	reqCurrent_p = reqInfo_p->reqCurrent;

    NHTTPi_ASSERT(NHTTPi_IsOpened(NHTTPi_GetBgnEndInfoP(sysInfo_p)));
    
    NHTTPi_lockReqList(mutexInfo_p);
    
    if (reqCurrent_p)
    {
        if (reqCurrent_p->id == id)
        {
            // 中断を要請
            if (!reqCurrent_p->req_p->doCancel)
            {
                reqCurrent_p->req_p->doCancel = TRUE;
                NHTTPi_SocCancel(mutexInfo_p, reqCurrent_p->req_p, (SOCKET)reqCurrent_p->socket_p);
                rc = TRUE;
            }
        }
    }
    
    if (!rc)
    {
        NHTTPListInfo*	listInfo_p = NHTTPi_GetListInfoP(sysInfo_p);
        rc = NHTTPi_freeReqQueue(listInfo_p, mutexInfo_p, id);
    }
    
    NHTTPi_unlockReqList(mutexInfo_p);

    return rc;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_cancelAllRequests
  Description:  キューをクリアして、実行中のリクエストには中断を要請
 *-------------------------------------------------------------------------*/
extern void NHTTPi_cancelAllRequests(NHTTPSysInfo* sysInfo_p)
{
    NHTTPReqInfo*		reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPListInfo*		listInfo_p = NHTTPi_GetListInfoP(sysInfo_p);
    NHTTPMutexInfo*		mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    volatile NHTTPi_REQLIST*	reqCurrent_p = reqInfo_p->reqCurrent;

    NHTTPi_lockReqList(mutexInfo_p);
    
    if (reqCurrent_p)
    {
        if (!reqCurrent_p->req_p->doCancel)
        {
            reqCurrent_p->req_p->doCancel = TRUE;
            NHTTPi_SocCancel(mutexInfo_p, reqCurrent_p->req_p, (SOCKET)reqCurrent_p->socket_p);
        }
    }

    NHTTPi_allFreeReqQueue(listInfo_p, mutexInfo_p);
    NHTTPi_unlockReqList(mutexInfo_p);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
