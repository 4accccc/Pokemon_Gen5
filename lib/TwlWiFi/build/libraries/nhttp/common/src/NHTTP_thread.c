/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_thread.c

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
#include "NHTTPlib_int_socket.h"
#include "d_nhttp_common.h"
#include "d_nhttp_private.h"
#include "NHTTP_bgnend.h"
#include "NHTTP_list.h"
#include "NHTTP_response.h"
#include "NHTTP_request.h"
#include "NHTTP_thread.h"
#include "NHTTP_control.h"
#include "NHTTP_recvbuf.h"
#include "NHTTP_nonport.h"


/*---------------------------------------------------------------------------*
    Constants
 *---------------------------------------------------------------------------*/
static const char STR_POST_DISPOS[] = "Content-Disposition: form-data; name=\"";
static const char STR_POST_TYPE_BIN[] = "Content-Type: application/octet-stream\r\nContent-Transfer-Encoding: binary\r\n";
static const char STR_POST_TYPE_URLENCODE[] = "Content-Type: application/x-www-form-urlencoded\r\n";
static const char STR_POST_TYPE_MULTIPART[] = "Content-Type: multipart/form-data; boundary=";

#define TMP_PROXY_HEADER_BUF_SIZE       (512)   // PROXYサーバー接続
#define TMP_CONTENT_LENGTH_BUF_SIZE     (12)    // ContentLengthを文字列に置き換えるための一時バッファサイズ

/*---------------------------------------------------------------------------*
    Forward references for static functions
 *---------------------------------------------------------------------------*/
static int  NHTTPi_CheckHeaderEnd               ( char* buf_p, int bufwp );
static int  NHTTPi_SaveBuf                      ( NHTTPReq* req_p, char* commBuf_p, const SOCKET socket, int* bufwp_p, const char* src_p, const int srclen);
static int  NHTTPi_SkipLineBuf                  ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req, const SOCKET socket );
static BOOL NHTTPi_GetPostContentlength         ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const char* label_p, u32* contentLength, const int sendtype );
static int  NHTTPi_SendPostData                 ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, char* commBuf_p, const char* label_p, const SOCKET socket, int* bufwp_p, const int sendtype );
static BOOL NHTTPi_BufFull                      ( NHTTPMutexInfo* mutexInfo_p, NHTTPRes* res_p );
static int  NHTTPi_SendData                     ( NHTTPThreadData* threadData_p, const char* src_p, int srclen );       // データ送信
static int  NHTTPi_SendProxyAuthorization       ( NHTTPThreadData* threadData_p );      // プロキシ認証送信
static int  NHTTPi_SendBasicAuthorization       ( NHTTPThreadData* threadData_p );      // ベーシック認証送信
static int  NHTTPi_SendProxyConnectMethod       ( NHTTPThreadData* threadData_p );      // PROXY CONNECTメソッド送信
static int  NHTTPi_RecvProxyConnectHeader       ( NHTTPThreadData* threadData_p );      // PROXY CONNECTヘッダー受信
static int  NHTTPi_SendHeaderList               ( NHTTPThreadData* threadData_p );      // 付加ヘッダ送信
static int  NHTTPi_SendProcPostDataRaw          ( NHTTPThreadData* threadData_p );      // コールバックによる生データ送信
static int  NHTTPi_SendProcPostDataBinary       ( NHTTPThreadData* threadData_p );      // コールバックによるバイナリデータ送信
static int  NHTTPi_SendProcPostDataAscii        ( NHTTPThreadData* threadData_p );      // コールバックによるアスキーデータ送信
static void NHTTPi_ThreadInit                   ( NHTTPThreadData* threadData_p );      // スレッド初期化処理
static void NHTTPi_ThreadReqEnd                 ( NHTTPThreadData* threadData_p );      // スレッド内でのリクエスト終了処理
static BOOL NHTTPi_ThreadExecReqQueue           ( NHTTPThreadData* threadData_p );      // キューリストの先頭を取ってきて、実行中にする
static BOOL NHTTPi_ThreadHostAddrProc           ( NHTTPThreadData* threadData_p );      // ホストアドレス設定
static BOOL NHTTPi_ThreadConnectProc            ( NHTTPThreadData* threadData_p );      // ホストに接続
static int  NHTTPi_ThreadProxyProc              ( NHTTPThreadData* threadData_p );      // プロキシサーバー接続
static int  NHTTPi_ThreadSendProc               ( NHTTPThreadData* threadData_p );      // HTTPヘッダー送信
static int  NHTTPi_ThreadRecvHeaderProc         ( NHTTPThreadData* threadData_p );      // HTTPヘッダー受信
static int  NHTTPi_ThreadParseHeaderProc        ( NHTTPThreadData* threadData_p );      // HTTPヘッダー解析
static int  NHTTPi_ThreadRecvBodyProc           ( NHTTPThreadData* threadData_p );      // HTTPボディ受信

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_InitThreadInfo
  Description:  NHTTPThreadInfo構造体初期設定関数
  Arguments:    threadInfo_p	NHTTPThreadInfo構造体へのポインタ
  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_InitThreadInfo(NHTTPThreadInfo* threadInfo_p)
{
    NHTTPi_IsCreateCommThreadMessageQueueOff(threadInfo_p);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_IsCreateCommThreadMessageQueueOn

  Description:  スレッドのメッセージキューの生成フラグ ON関数
  
  Arguments:    
                
  Returns:
 *-------------------------------------------------------------------------*/
extern void NHTTPi_IsCreateCommThreadMessageQueueOn(NHTTPThreadInfo* threadInfo_p)
{
    threadInfo_p->isCreateCommThreadMessageQueue = TRUE;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_IsCreateCommThreadMessageQueueOff

  Description:  スレッドのメッセージキューの生成フラグ OFF関数
  
  Arguments:    
                
  Returns:
 *-------------------------------------------------------------------------*/
extern void NHTTPi_IsCreateCommThreadMessageQueueOff(NHTTPThreadInfo* threadInfo_p)
{
    threadInfo_p->isCreateCommThreadMessageQueue = FALSE;
}
/*-------------------------------------------------------------------------*
  Name:         NHTTPi_IsCreateCommThreadMessageQueue

  Description:  スレッドのメッセージキューの生成フラグ取得関数
  
  Arguments:    
                
  Returns:      スレッドのメッセージキューが生成済みなら	TRUE
  		スレッドのメッセージキューが未生成なら		FALSE
 *-------------------------------------------------------------------------*/
extern int NHTTPi_IsCreateCommThreadMessageQueue(NHTTPThreadInfo* threadInfo_p)
{
    return threadInfo_p->isCreateCommThreadMessageQueue;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetProgress
  Description:  現在進行中のHTTPリクエスト処理の進行度(受信バイト数およびContent-Lengthの値)を取得する
  Arguments:    sysInfo_p
                received_p	受信済みHTTP BODY領域サイズを格納する領域へのポインタ
                contentlen_p	Content-Lengthを格納する領域へのポインタ
  Returns:      TRUE	HTTPリクエスト進行度取得に成功した
                FALSE	HTTPリクエストキューが空で、かつHTTPリクエストが実行されていない
 *-------------------------------------------------------------------------*/
extern BOOL NHTTPi_GetProgress(NHTTPSysInfo* sysInfo_p, u32* received_p, u32* contentlen_p)
{
    BOOL			rc = FALSE;
    NHTTPMutexInfo*		mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReqInfo*		reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    volatile NHTTPi_REQLIST*	reqCurrent_p = reqInfo_p->reqCurrent;

    NHTTPi_ASSERT(NHTTPi_IsOpened(NHTTPi_GetBgnEndInfoP(sysInfo_p)));
    NHTTPi_ASSERT(received_p);
    NHTTPi_ASSERT(contentlen_p);
    
    *received_p = 0;
    *contentlen_p = 0;

    NHTTPi_lockReqList(mutexInfo_p);
    {
        if (reqCurrent_p)
        {
            if (reqCurrent_p->req_p->res_p->contentLength != 0)
            {
                *received_p = (u32)reqCurrent_p->req_p->res_p->bodyLen;
                *contentlen_p = (u32)((reqCurrent_p->req_p->res_p->contentLength == 0xffffffff)? 0 : reqCurrent_p->req_p->res_p->contentLength);
            }
            rc = TRUE;
        }
        else
        {
            NHTTPListInfo*	listInfo_p = NHTTPi_GetListInfoP(sysInfo_p);
            rc = (listInfo_p->reqQueue != NULL);
        }
    }
    NHTTPi_unlockReqList(mutexInfo_p);
    
    return rc;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_CheckHeaderEnd
  Description:  ヘッダー終端（CR,LF,CRLF）を調べる
  Arguments:    buf_p   調査対象バッファのポインタ
                bufwp   調査開始位置
  Returns:      TRUE    ヘッダー終端がある
                FALSE   ヘッダー終端がない
 *---------------------------------------------------------------------------*/
static int NHTTPi_CheckHeaderEnd(char* buf_p, const int bufwp)
{
    if ((buf_p[(bufwp - 2) & 3] == '\r') &&
        (buf_p[(bufwp - 1) & 3] == '\r'))
    {
        DEBUG_PRINTF("[%s]:CR\n", __func__);
        return TRUE;
    }
    else if ((buf_p[(bufwp - 2) & 3] == '\n') &&
             (buf_p[(bufwp - 1) & 3] == '\n'))
    {
        DEBUG_PRINTF("[%s]:LF\n", __func__);
        return TRUE;
    }
    else if ((buf_p[(bufwp - 4) & 3] == '\r') &&
             (buf_p[(bufwp - 3) & 3] == '\n') &&
             (buf_p[(bufwp - 2) & 3] == '\r') &&
             (buf_p[(bufwp - 1) & 3] == '\n'))
    {
        DEBUG_PRINTF("[%s]:CRLF\n", __func__);
        return TRUE;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SaveBuf
  Description:  送信データをcommBuf_pに貯め、いっぱいになったら送信
  Arguments:    req_p           リクエスト構造体のポインタ
                commBuf_p       送信データ蓄積バッファ
                socket          ソケットID
                bufwp_p         送信データの蓄積量
                src_p           送信内容
                srclen          送信内容のサイズ
  Returns:      0以下           エラー、またはキャンセル
                正の値          送信内容のサイズ
 *---------------------------------------------------------------------------*/
static int NHTTPi_SaveBuf(NHTTPReq* req_p, char* commBuf_p, const SOCKET socket, int* bufwp_p, const char* src_p, const int srclen)
{
    int		n;
    int		len;
    int		sendbyte;
    
    for (len = srclen; len > 0;)
    {
        if (req_p->doCancel)
        {
            DEBUG_PRINTF("[%s][%d] キャンセル処理が入りました\n", __func__, __LINE__);
            return -1;
        }
        
        n = LEN_COMMBUF - *bufwp_p;
        n = MIN(len, n);
        NHTTPi_memcpy(&commBuf_p[*bufwp_p], src_p, n);
        *bufwp_p += n;
        src_p += n;
        len -= n;
        
        if (*bufwp_p == LEN_COMMBUF)
        {
            sendbyte = NHTTPi_SocSend(req_p, socket, commBuf_p, LEN_COMMBUF, 0);
            if (sendbyte <= 0)
            {
                return sendbyte;
            }
            *bufwp_p -= sendbyte;
        }
    }
    
    return srclen;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SkipLineBuf
  Description:  改行の次まで受信して読み飛ばす
  Arguments:    req_p   リクエストポインタ
                socket  ソケットID
  Returns:      0以下   エラー
                正の値  受信バイト数
 *---------------------------------------------------------------------------*/
static int NHTTPi_SkipLineBuf(NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket)
{
    char buf[2];
    int p;
    int recvbyte;
    int rc = 0;

    buf[0] = 0;
    buf[1] = 0;
    for (p=0; !((buf[p&1] == '\r') && (buf[(p-1)&1] == '\n')); p++)
    {
        recvbyte = NHTTPi_SocRecv(mutexInfo_p, req_p, socket, &buf[p&1], 1, 0);
        if (recvbyte <= 0) return recvbyte;
        rc += recvbyte;
    }
    return rc;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_GetPostContentlength
  Description:  POSTデータ送信のサイズ調査
  Arguments:    mutexInfo_p
                req_p           リクエストポインタ
                label_p         調査対象のPOSTデータのラベル
                contentLength   調査結果のサイズ
                sendtype        POSTのタイプ
  Returns:      TRUE    調査成功
                FALSE   調査失敗
 *---------------------------------------------------------------------------*/
static BOOL NHTTPi_GetPostContentlength(NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const char* label_p, u32* contentLength, const int sendtype)
{
    int                 offset = 0;
    u32                 length = 0;
    char*               value_p = NULL;
    NHTTPConnection*	connection_p = NHTTPi_Request2Connection(mutexInfo_p, req_p);
    DEBUG_PRINTF("[%s]\n", __func__);

    if (connection_p == NULL) return FALSE;
    
    connection_p->postDataBuf.value_p = NULL;
    
    while (1)
    {
        // ここにキャンセルのチェックをいれて応答性を高めてみる
        if (req_p->doCancel)
        {
            DEBUG_PRINTF("%s キャンセル処理が入りました\n", __func__);
            return FALSE;
        }

        /** NHTTPCreateConnection使用 */
        connection_p->postDataBuf.length = 0;
        if (NHTTPi_PostSendCallback(mutexInfo_p, connection_p, label_p, offset) < 0)
        {
            return FALSE;
        }
        else
        {
            value_p = connection_p->postDataBuf.value_p;
            length = connection_p->postDataBuf.length;
        }

        if (length == 0)
        {
            break;
        }
        if (value_p == NULL)
        {
            return FALSE;
        }
        offset += length;
        
        switch (sendtype)
        {
        case SEND_TYPE_RAW_e:
        case SEND_TYPE_MULTIPART_e:
            *contentLength += length;
            break;
        case SEND_TYPE_URLENCODE_e:
            *contentLength += NHTTPi_getUrlEncodedSize2(value_p, (int)length);
            break;
        }
    }
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SendPostData
  Description:  POSTデータ送信
  Arguments:    mutexInfo_p
                req_p           リクエストポインタ
                commBuf_p       送信バッファ
                label_p         送信POSTデータのラベル
                socket          ソケットID
                bufwp_p         送信バッファ位置のポインタ
                sendtype        POSTのタイプ
  Returns:      NHTTP_SEND_ERROR_POST_CALLBACK_e        エラー
                NHTTP_SEND_NOTCONN_e                    未接続
                NHTTP_SEND_SUCCESS_e                    成功 
 *---------------------------------------------------------------------------*/
static int NHTTPi_SendPostData(NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, char* commBuf_p, const char* label_p, const SOCKET socket, int* bufwp_p, const int sendtype)
{
    int			offset = 0;
    u32			length = 0;
    char*		value_p = NULL;
    int			stat = 0;
    char		urlencode[3];
    int			p,n;
    NHTTPConnection*	connection_p = NHTTPi_Request2Connection(mutexInfo_p, req_p);
    
    DEBUG_PRINTF("[%s]\n", __func__);

    if (connection_p == NULL) return NHTTP_SEND_ERROR_POST_CALLBACK_e;

    connection_p->postDataBuf.value_p = NULL;
    
    while (1)
    {
        // ここにキャンセルのチェックをいれて応答性を高めてみる
        if (req_p->doCancel)
        {
            DEBUG_PRINTF("%s キャンセル処理が入りました\n", __func__);
            return NHTTP_SEND_ERROR_POST_CALLBACK_e;
        }

        connection_p->postDataBuf.length = 0;
        if (NHTTPi_PostSendCallback(mutexInfo_p, connection_p, label_p, offset) < 0)
        {
            return NHTTP_SEND_ERROR_POST_CALLBACK_e;
        }
        else
        {
            value_p = connection_p->postDataBuf.value_p;
            length = connection_p->postDataBuf.length;
        }

        if (length == 0)
        {
            break;
        }
        if (value_p == NULL)
        {
            return NHTTP_SEND_ERROR_POST_CALLBACK_e;
        }
        offset += length;
        // 送信
        switch (sendtype)
        {
        case SEND_TYPE_MULTIPART_e:
        case SEND_TYPE_RAW_e:
            stat = NHTTPi_SaveBuf(req_p, commBuf_p, socket, bufwp_p, value_p, (int)length);
            if (stat < 0)
            {
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return NHTTP_SEND_ERROR_e;
            }
            else if (stat == 0)
            {
                return NHTTP_SEND_NOTCONN_e;
            }
            break;
        case SEND_TYPE_URLENCODE_e:
            for (p=0; p < length; p++)
            {
                NHTTPi_memclr(urlencode, 3);
                n = NHTTPi_encodeUrlChar( urlencode, *(value_p + p) );
                stat = NHTTPi_SaveBuf(req_p, commBuf_p, socket, bufwp_p, urlencode, n);
                if (stat < 0)
                {
                    DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                    return NHTTP_SEND_ERROR_e;
                }
                else if (stat == 0)
                {
                    return NHTTP_SEND_NOTCONN_e;
                }
            }
            break;
        }
    }

    return NHTTP_SEND_SUCCESS_e;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_BufFull
  Description:  受信バッファ再確保
  Arguments:    osRvlInfo_p
                res_p
  Returns:      TRUE    再確保、または受信バッファがいっぱいでは無い
                FALSE   再確保できない
 *---------------------------------------------------------------------------*/
static BOOL NHTTPi_BufFull(NHTTPMutexInfo* mutexInfo_p, NHTTPRes* res_p)
{
    BOOL	ret = FALSE;
    BOOL	full = NHTTPi_isRecvBufFull(res_p, res_p->bodyLen);

    DEBUG_PRINTF("[%s]: %d(%d) %d %d(%d,%d)\n",
                 __func__,
                 (res_p->recvBufLen <= 0),
                 res_p->recvBufLen,
                 (res_p->recvBuf_p == NULL),
                 full,
                 res_p->recvBufLen,
                 res_p->bodyLen);
    
    if ((res_p->recvBufLen <= 0) || (res_p->recvBuf_p == NULL) || (full))
    {
        NHTTPConnection*	connection_p = NHTTPi_Response2Connection(mutexInfo_p, res_p);
        
        /** NHTTPCreateConnection使用 */
        if (connection_p != NULL)
        {
            NHTTPi_BufferFullCallback(mutexInfo_p, connection_p);
            if ((res_p->recvBuf_p != NULL) &&
                (res_p->recvBufLen > 0) &&
                (NHTTPi_isRecvBufFull(res_p, res_p->bodyLen) == FALSE))
            {
                ret = TRUE;
            }
        }
    }
    else if (full == FALSE)
    {
        DEBUG_PRINTF("[%d]:buf is not full\n", __LINE__);
        ret = TRUE;
    }
    DEBUG_PRINTF("========= end : %d\n", ret);
    
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SendData
  Description:  データ送信
  Arguments:    threadData_p
                src_p
                srclen
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int NHTTPi_SendData(NHTTPThreadData* threadData_p, const char* src_p, int srclen)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    char*		commBuf_p = (char*)&(threadInfo_p->commBuf[0]);
    int                 stat = 0;

    if (srclen == 0 ) {
        // 送るサイズが０の場合は送信しない
        // ※このまま処理が進むとNHTTP_SEND_NOTCONN_eになってしまうため
        return NHTTP_SEND_SUCCESS_e;
    }
    
    stat =  NHTTPi_SaveBuf( req_p, commBuf_p, bgnEndInfo_p->socket, &(threadData_p->sendBufwp), src_p, srclen);
    DEBUG_PRINTF("[%s][%d] [%s] stat [%d] srclen [%d]\n", __func__, threadData_p->id, src_p, stat, srclen);
    
    if (stat < 0)
    {
        return NHTTP_SEND_ERROR_e;
    }
    else if (stat == 0)
    {
        return NHTTP_SEND_NOTCONN_e;
    }
    else
    {
        return NHTTP_SEND_SUCCESS_e;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SendProxyAuthorization
  Description:  プロキシ認証送信
  Arguments:    threadData_p
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int NHTTPi_SendProxyAuthorization(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    int                 sendStatus;

    // サイズが０の場合は送信する必要なし
    if (req_p->proxyUsernamePasswordLen == 0)
    {
        return NHTTP_SEND_SUCCESS_e;
    }
        
    sendStatus = NHTTPi_SendData( threadData_p, "Proxy-Authorization: Basic ", 27);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, req_p->proxyUsernamePassword, req_p->proxyUsernamePasswordLen);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    
    return NHTTP_SEND_SUCCESS_e;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SendBasicAuthorization
  Description:  ベーシック認証送信
  Arguments:    threadData_p
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int NHTTPi_SendBasicAuthorization(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    int                 sendStatus;

    // サイズが０の場合は送信する必要なし
    if (req_p->basicUsernamePasswordLen == 0)
    {
        return NHTTP_SEND_SUCCESS_e;
    }
    
    sendStatus = NHTTPi_SendData(threadData_p, "Authorization: Basic ", 21);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, req_p->basicUsernamePassword, req_p->basicUsernamePasswordLen);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    
    return NHTTP_SEND_SUCCESS_e;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SendProxyConnectMethod
  Description:  PROXY CONNECTメソッド送信
  Arguments:    threadData_p
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int  NHTTPi_SendProxyConnectMethod(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    char*		commBuf_p = (char*)&(threadInfo_p->commBuf[0]);
    int                 sendStatus = NHTTP_SEND_SUCCESS_e;
    char                portstr[5];    // 最大でも5桁
    int                 portlen = 0;
    
    DEBUG_PRINTF("[%s] CONNECT port test %d\n",__func__, req_p->port);
    
    portlen = NHTTPi_intToStr(portstr, (u32)(req_p->port));

    // CONNECTメソッドを送信
    sendStatus = NHTTPi_SendData(threadData_p, "CONNECT ", 8);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, &req_p->URL_p[8], req_p->hostLen - 8 );
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, ":", 1);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, portstr, portlen);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, " HTTP/" NHTTP_STR_HTTPVER "\r\n", 11 );
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "Host: ", 6);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, &req_p->URL_p[8], req_p->hostLen - 8);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, ":", 1);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, portstr, portlen );
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "Content-Length: 0\r\nPragma: no-cache\r\n", 37);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    
    // 認証用のユーザ名とパスワードを送信
    sendStatus = NHTTPi_SendProxyAuthorization(threadData_p);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    
    // ヘッダ終端
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (threadData_p->sendBufwp > 0)
    {
        int stat = NHTTPi_SocSend(req_p, bgnEndInfo_p->socket, commBuf_p, threadData_p->sendBufwp, 0);
        if (stat < 0)
        {
            return NHTTP_SEND_ERROR_e;
        }
        if (stat == 0)
        {
            return NHTTP_SEND_NOTCONN_e;
        }
    }
    
    threadData_p->sendBufwp = 0;
    NHTTPi_memclr(commBuf_p, LEN_COMMBUF);
    
    return NHTTP_SEND_SUCCESS_e;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_RecvProxyConnectHeader
  Description:  PROXY CONNECTヘッダー受信
  Arguments:    threadData_p
  Returns:      TRUE  成功
                FALSE 失敗
 *---------------------------------------------------------------------------*/
static int  NHTTPi_RecvProxyConnectHeader(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    char*		commBuf_p = (char*)&(threadInfo_p->commBuf[0]);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPRes*           res_p = req_p->res_p;

    char                tmpbuf[TMP_PROXY_HEADER_BUF_SIZE];
    int                 i = 0;
    int                 recvbyte = 0;
    int                 endFlag = FALSE;
    int                 connectFlag = FALSE;
    int                 totalRecvbyte = 0;

    (void)threadData_p;
    
    DEBUG_PRINTF("[%s]\n",__func__);

    while (1)
    {
        // 受信できるだけしてしまう
        recvbyte = NHTTPi_SocRecv(mutexInfo_p, req_p, bgnEndInfo_p->socket, &(tmpbuf[totalRecvbyte]), (TMP_PROXY_HEADER_BUF_SIZE - totalRecvbyte), 0);
        totalRecvbyte += recvbyte;
        // 一応中身を総チェックする
        // HTTP/1.0 or HTTP/1.1 200
        res_p->httpStatus = NHTTPi_strToInt(&tmpbuf[9], 3);
        if ((NHTTPi_strnicmp(tmpbuf, "HTTP/", 5) == 0) &&
            (tmpbuf[8] == ' ') &&
            (res_p->httpStatus == 200))
        {
            connectFlag = TRUE;
        }
        
        // 改行コードが連続２回含まれているか調査
        for (i = 0, endFlag = FALSE; i < totalRecvbyte; i++)
        {
            if ((i > 1) &&
                (tmpbuf[i - 1] == '\r') &&
                (tmpbuf[i] == '\r'))
            {
                DEBUG_PRINTF("[%s]:CR %d\n", __func__,i);
                endFlag = TRUE;
            }
            else if ((i > 1) &&
                     (tmpbuf[i - 1] == '\n') &&
                     (tmpbuf[i] == '\n'))
            {
                DEBUG_PRINTF("[%s]:LF %d\n", __func__,i);
                endFlag = TRUE;
            } else if ((i > 3) &&
                       (tmpbuf[i - 3] == '\r') &&
                       (tmpbuf[i - 2] == '\n') &&
                       (tmpbuf[i - 1] == '\r') &&
                       (tmpbuf[i] == '\n'))
            {
                DEBUG_PRINTF("[%s]:CRLF %d\n", __func__,i);
                endFlag = TRUE;
            }
        }
        
        // 終了チェック
        if (endFlag)
        {
            if (connectFlag)
            {
                break;
            }
            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
            return FALSE;
        }

        // error
        // 受信できなかった
        if (recvbyte < 0)
        {
            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
            return FALSE;
        }

        // バッファいっぱい
        if (totalRecvbyte >= TMP_PROXY_HEADER_BUF_SIZE)
        {
            recvbyte = NHTTPi_SocRecv(mutexInfo_p, req_p, bgnEndInfo_p->socket, commBuf_p, 1, 0);
            // 受信失敗
            if (recvbyte < 0)
            {
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return FALSE;
            }
            // まだ受信 PROXY接続の返答ヘッダがこんなでかいワケがないのでエラー
            else if (recvbyte != 0)
            {
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return FALSE;
            }
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SendHeaderList
  Description:  付加ヘッダ送信
  Arguments:    threadData_p
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int NHTTPi_SendHeaderList(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPi_DATALIST*    datalist_p;
    int                 sendStatus;
    
    DEBUG_PRINTF("[%s][%d]\n", __func__, threadData_p->id);
    // 付加ヘッダ
    for (datalist_p = NHTTPi_getHdrFromList(&req_p->listHeader_p); datalist_p;  datalist_p = NHTTPi_getHdrFromList(&req_p->listHeader_p))
    {
        sendStatus = NHTTPi_SendData(threadData_p, datalist_p->label_p, NHTTPi_strlen(datalist_p->label_p));
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        sendStatus = NHTTPi_SendData(threadData_p,  ": ", 2);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        sendStatus = NHTTPi_SendData(threadData_p, datalist_p->value_p, NHTTPi_strlen(datalist_p->value_p));
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        
        NHTTPi_free(datalist_p);
    }
    
    return NHTTP_SEND_SUCCESS_e;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SendProcPostDataRaw
  Description:  コールバックによる生データ送信
  Arguments:    threadData_p
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int NHTTPi_SendProcPostDataRaw(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    char*		commBuf_p = (char*)&(threadInfo_p->commBuf[0]);
    u32                 contentLength = 0;
    int                 contentLengthBufLen = 0;
    char                contentLengthBuf[TMP_CONTENT_LENGTH_BUF_SIZE];
    int                 sendStatus;
    
    // サイズを調査
    if (!req_p->rawPostData_p)
    {
        if (!NHTTPi_GetPostContentlength(mutexInfo_p, req_p, NULL, &contentLength, SEND_TYPE_RAW_e))
        {
            return NHTTP_SEND_ERROR_POST_CALLBACK_e;
        }
    }
    else
    {
        contentLength = req_p->rawPostDataLen;
    }
    contentLengthBufLen = NHTTPi_intToStr(contentLengthBuf, contentLength);
    
    
    sendStatus = NHTTPi_SendData(threadData_p, "Content-Length: ", 16);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, contentLengthBuf, contentLengthBufLen);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    
    // 生データ送信
    if (!req_p->rawPostData_p)
    {
        int result = NHTTPi_SendPostData(mutexInfo_p, req_p, commBuf_p, NULL, bgnEndInfo_p->socket, &threadData_p->sendBufwp, SEND_TYPE_RAW_e);
        if (result != NHTTP_SEND_SUCCESS_e)
        {
            return result;
        }
    }
    else
    {
        sendStatus = NHTTPi_SendData(threadData_p, req_p->rawPostData_p, (int)req_p->rawPostDataLen);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    }
    
    return NHTTP_SEND_SUCCESS_e;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SendProcPostDataBinary
  Description:  コールバックによるバイナリデータ送信
  Arguments:    threadData_p
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int NHTTPi_SendProcPostDataBinary(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    char*		commBuf_p = (char*)&(threadInfo_p->commBuf[0]);
    u32                 contentLength = 0;
    int                 contentLengthBufLen = 0;
    char                contentLengthBuf[TMP_CONTENT_LENGTH_BUF_SIZE];
    int                 sendStatus;
    NHTTPi_DATALIST*    datalist_p;
   
    // バイナリデータあり　→　multipart/form-data
    for (datalist_p = req_p->listPost_p; datalist_p; datalist_p = datalist_p->next_p)
    {
        contentLength += sizeof(req_p->tagPost) + 2;	// [bound]CRLF
        contentLength += (sizeof(STR_POST_DISPOS) - 1) + NHTTPi_strlen(datalist_p->label_p) + 1 + 2;	// [label]'"'CRLF
        if (datalist_p->isBinary)
        {
            contentLength += (sizeof(STR_POST_TYPE_BIN) - 1);
        }
        contentLength += 2;	// CRLF
        
        if (!datalist_p->value_p)
        {
            if (!NHTTPi_GetPostContentlength(mutexInfo_p, req_p, datalist_p->label_p, &contentLength, SEND_TYPE_MULTIPART_e))
            {
                return NHTTP_SEND_ERROR_POST_CALLBACK_e;
            }
        }
        else
        {
            contentLength += datalist_p->length;
        }
        
        contentLength += 2;	// CRLF
        if (datalist_p == req_p->listPost_p->prev_p)
        {
            break;
        }
    }
    contentLength += sizeof(req_p->tagPost) + 2 + 2;	// [bound]"--"CRLF
    contentLengthBufLen = NHTTPi_intToStr(contentLengthBuf, contentLength);
                                
    sendStatus = NHTTPi_SendData(threadData_p, STR_POST_TYPE_MULTIPART, (sizeof(STR_POST_TYPE_MULTIPART) - 1));
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, &req_p->tagPost[2], sizeof(req_p->tagPost) - 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 

    sendStatus = NHTTPi_SendData(threadData_p, "Content-Length: ", 16);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, contentLengthBuf, contentLengthBufLen);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 

    //==========================================================================
    // データ本体を送信
    for (datalist_p = req_p->listPost_p; datalist_p; datalist_p = datalist_p->next_p)
    {
        sendStatus = NHTTPi_SendData(threadData_p, req_p->tagPost, sizeof(req_p->tagPost));
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        sendStatus = NHTTPi_SendData(threadData_p, STR_POST_DISPOS, (sizeof(STR_POST_DISPOS) - 1));
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        sendStatus = NHTTPi_SendData(threadData_p, datalist_p->label_p, NHTTPi_strlen(datalist_p->label_p));
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        sendStatus = NHTTPi_SendData(threadData_p, "\"" "\r\n", 3);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 

        if (datalist_p->isBinary)
        {
            sendStatus = NHTTPi_SendData(threadData_p, STR_POST_TYPE_BIN, (sizeof(STR_POST_TYPE_BIN) - 1));
            if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        }
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        
        if (!datalist_p->value_p)
        {
            int result = NHTTPi_SendPostData(mutexInfo_p, req_p, commBuf_p, datalist_p->label_p, bgnEndInfo_p->socket, &threadData_p->sendBufwp, SEND_TYPE_MULTIPART_e);
            if (result != NHTTP_SEND_SUCCESS_e)
            {
                return result;
            }
        }
        else
        {
            sendStatus = NHTTPi_SendData(threadData_p, datalist_p->value_p, (int)datalist_p->length);
            if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        }
        
        sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        
        if (datalist_p == req_p->listPost_p->prev_p)
            break;
    }

    sendStatus = NHTTPi_SendData(threadData_p, req_p->tagPost, sizeof(req_p->tagPost));
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "--" "\r\n", 4);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 

    return NHTTP_SEND_SUCCESS_e;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SendProcPostDataAscii
  Description:  コールバックによるアスキーデータ送信
  Arguments:    threadData_p
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int NHTTPi_SendProcPostDataAscii(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    char*		commBuf_p = (char*)&(threadInfo_p->commBuf[0]);
    
    u32                 contentLength = 0;
    int                 tmpBufLen = 0;
    char                tmpBuf[TMP_CONTENT_LENGTH_BUF_SIZE];
    int                 sendStatus;
    NHTTPi_DATALIST*    datalist_p;

    // バイナリデータなし　→　application/x-www-form-urlencoded
    // ex: label1=test+data%0D%0A&label2=testdata2
    for (datalist_p = req_p->listPost_p; datalist_p; datalist_p = datalist_p->next_p)
    {
        contentLength += NHTTPi_getUrlEncodedSize(datalist_p->label_p);
        contentLength += 1;	// '='

        if (!datalist_p->value_p)
        {
            if (!NHTTPi_GetPostContentlength(mutexInfo_p, req_p, datalist_p->label_p, &contentLength, SEND_TYPE_URLENCODE_e))
            {
                return NHTTP_SEND_ERROR_POST_CALLBACK_e;
            }
        }
        else
        {
            contentLength += NHTTPi_getUrlEncodedSize(datalist_p->value_p);
        }
        if (datalist_p == req_p->listPost_p->prev_p)
            break;
        contentLength += 1;	// '&'
    }

    tmpBufLen = NHTTPi_intToStr(tmpBuf, contentLength);
    
    sendStatus = NHTTPi_SendData(threadData_p, STR_POST_TYPE_URLENCODE, (sizeof(STR_POST_TYPE_URLENCODE) - 1));
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "Content-Length: ", 16);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, tmpBuf, tmpBufLen);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    sendStatus = NHTTPi_SendData(threadData_p, "\r\n", 2);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    
    //==========================================================================
    // データ本体を送信
    {
        int	p;
        for (datalist_p = req_p->listPost_p; datalist_p; datalist_p = datalist_p->next_p)
        {
            // label
            for (p=0; datalist_p->label_p[p]; p++)
            {
                tmpBufLen = NHTTPi_encodeUrlChar(tmpBuf, datalist_p->label_p[p]);
                sendStatus = NHTTPi_SendData(threadData_p, tmpBuf, tmpBufLen);
                if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
            }
            sendStatus = NHTTPi_SendData(threadData_p, "=", 1);
            if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
            
            if (!datalist_p->value_p)
            {
                int result = NHTTPi_SendPostData(mutexInfo_p, req_p, commBuf_p, datalist_p->label_p, bgnEndInfo_p->socket, &threadData_p->sendBufwp, SEND_TYPE_URLENCODE_e);
                if (result != NHTTP_SEND_SUCCESS_e)
                {
                    return result;
                }
            }
            else
            {
                for (p=0; datalist_p->value_p[p]; p++)
                {
                    tmpBufLen = NHTTPi_encodeUrlChar(tmpBuf, datalist_p->value_p[p]);
                    sendStatus = NHTTPi_SendData(threadData_p, tmpBuf, tmpBufLen);
                    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
                }
            }
            if (datalist_p == req_p->listPost_p->prev_p)
            {
                break;
            }
            sendStatus = NHTTPi_SendData(threadData_p, "&", 1);
            if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        }
    }
    return NHTTP_SEND_SUCCESS_e;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadInit
  Description:  HTTP通信スレッド　使用変数初期化
  Arguments:    threadData_p
 *---------------------------------------------------------------------------*/
static void NHTTPi_ThreadInit(NHTTPThreadData* threadData_p)
{
    DEBUG_PRINTF("[%s]\n", __func__);
    
    threadData_p->id = -1;
    NHTTPi_memclr(threadData_p->preHostname, LEN_HOST_NAME);
    NHTTPi_memclr(threadData_p->tmpBodyBuf, TMP_BODY_BUF_SIZE);
    threadData_p->hostaddr = 0xffffffff;
    threadData_p->preHostaddr = 0xffffffff;
    threadData_p->sendBufwp = 0;    
    threadData_p->isKeepAlive = FALSE;
    threadData_p->isChunked = FALSE;    
    threadData_p->restartConnect = FALSE;
    threadData_p->contentLength = 0;
    threadData_p->result = NHTTP_ERROR_NONE;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadReqEnd
  Description:  HTTP通信スレッド　終了処理
  Arguments:    threadData_p 
 *---------------------------------------------------------------------------*/
static void NHTTPi_ThreadReqEnd(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPRes*           res_p = req_p->res_p;
    NHTTPConnection*    connection_p = NHTTPi_Request2Connection(mutexInfo_p, req_p);
    
    DEBUG_PRINTF("[%s][%d] finalizing communication\n", __func__, threadData_p->id);
    
    // キャンセルチェック
    if (req_p->doCancel)
    {
        threadData_p->result = NHTTP_ERROR_CANCELED;
        // > Keep-Alive扱いになっているソケット(isKeepAlive == TRUE)が
        // > NHTTPi_CleanupAsync()関数を呼び出してもクローズされない。
        // バグを修正
        threadData_p->isKeepAlive = FALSE;
        DEBUG_PRINTF("[%s][%d] request was canceled\n", __func__, threadData_p->id );
        DEBUG_PRINTF("[%s][%d] isKeepAlive = %d \n", __func__,  __LINE__, threadData_p->isKeepAlive);
    }

    if (!threadData_p->isKeepAlive && IS_SOCKET(bgnEndInfo_p->socket))
    {
        DEBUG_PRINTF("NHTTPi_SocClose [%d]\n", __LINE__);
        if (NHTTPi_SocClose( mutexInfo_p, req_p, bgnEndInfo_p->socket ) < 0)
        {
            threadData_p->result = NHTTP_ERROR_NETWORK;
        }
        DEBUG_PRINTF("[%s][%d] INVALID_SOCKET (keep %d)\n", __func__,  __LINE__, threadData_p->isKeepAlive);
        bgnEndInfo_p->socket = INVALID_SOCKET;
    }
    
    if (threadData_p->result == NHTTP_ERROR_NONE)
    {
        res_p->isSuccess = TRUE;
    }
    else
    {
        res_p->isSuccess = FALSE;
        // エラー情報をセット
        NHTTPi_SetError(bgnEndInfo_p, threadData_p->result);
        DEBUG_PRINTF("[%s][%d] API error = %d\n",
                        __func__, threadData_p->id, threadData_p->result );
        
        if (res_p->recvBuf_p == &(threadData_p->tmpBodyBuf[0]))
        {
            res_p->recvBuf_p = NULL;
            res_p->recvBufLen = 0;
        }
    }
    
    // NHTTPGetConnectionError用
    if (connection_p != NULL)
    {
        connection_p->nhttpError = threadData_p->result;
    }
            
    // すべての終了処理が完了してからリクエスト削除
    NHTTPi_lockReqList(mutexInfo_p);
    {
        NHTTPi_free((void*)reqInfo_p->reqCurrent);
        reqInfo_p->reqCurrent = NULL;
    }
    NHTTPi_unlockReqList(mutexInfo_p);
    
    NHTTPi_destroyRequestObject(mutexInfo_p, req_p);
                
    if ((connection_p != NULL) && (res_p->isSuccess != FALSE))
    {
        connection_p->status = NHTTP_STATUS_COMPLETE;
    }
    
    NHTTPi_CompleteCallback(mutexInfo_p, connection_p);
    
    DEBUG_PRINTF("[%s][%d] complete threadData_p->result %d \n\n",
                    __func__, threadData_p->id, threadData_p->result);
    
    if (connection_p != NULL)
    {
        NHTTPi_NotifyCompletion(connection_p);
    }
    
    NHTTPi_DeleteConnection(connection_p);
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadExecReqQueue
  Description:  キューリストの先頭を取ってきて、実行中にする
                キューリストが空だったら、kickされるまで待つ
  Returns:      TRUE キューリストの先頭が取得でき、実行中に
                FALSE キューリストが空でkickされるまで待つ
 *---------------------------------------------------------------------------*/
static BOOL NHTTPi_ThreadExecReqQueue(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);

    DEBUG_PRINTF("[%s]\n", __func__);
        
    NHTTPi_lockReqList(mutexInfo_p);
    {
        // キューリストの先頭を取ってきて、実行中にする
        NHTTPListInfo*	listInfo_p = NHTTPi_GetListInfoP(sysInfo_p);
        NHTTPi_REQLIST*	list_p = NHTTPi_getReqFromReqQueue(listInfo_p);

        if (list_p)
        {
            NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
            
            threadData_p->id = list_p->id;
            reqInfo_p->reqCurrent = list_p;
        }
        else
        {
            threadData_p->id = -1;
        }
    }
    NHTTPi_unlockReqList(mutexInfo_p);
    
    if (threadData_p->id < 0)
    {
        NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
        // キューリストが空だったら、kickされるまで待つ
        NHTTPi_idleCommThread(threadInfo_p);
        return FALSE;
    }
    
    return TRUE;
}
/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadHostAddrProc
  Description:  ホストアドレス設定
                前回のホストアドレスと同一の場合は前回のホストアドレスを設定
  Arguments:    threadData_p
  Returns:      TRUE  成功
                FALSE 失敗
 *---------------------------------------------------------------------------*/
static BOOL NHTTPi_ThreadHostAddrProc(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    
    // 接続先ホスト名
    char* hostname_p = req_p->hostname_p;
    
    // PROXYが設定されている場合はPROXYサーバーが接続先となる
    if (req_p->isProxy)
    {
        hostname_p = req_p->proxyAddr;
    }
    
    // RVL版SOでは名前解決にキャッシュが効かないので
    // Keep-Alive指定時はホスト名が前回のものと一致した場合にホスト名解決をしない
    if ((NHTTPi_strlen(hostname_p) == 0) ||
        (NHTTPi_strcmp(hostname_p, threadData_p->preHostname) != 0))
    {
        DEBUG_PRINTF("[%s][%d] 前回のホスト名と不一致\n", __func__, threadData_p->id);
        // ホスト名を解決
        threadData_p->hostaddr = NHTTPi_resolveHostname(req_p, hostname_p);
        if (!threadData_p->hostaddr)
        {
            if (req_p->isProxy)
            {
                threadData_p->result = NHTTP_ERROR_DNS_PROXY;
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return FALSE;
            }
            else
            {
                threadData_p->result = NHTTP_ERROR_DNS;
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return FALSE;
            }
        }
    }
    else
    {
        // ホスト名が前回のものと一致したので、ホストアドレスも同値
        DEBUG_PRINTF("[%s][%d] ホスト名が前回のものと一致したので、ホストアドレスも同値\n", __func__, threadData_p->id);
        threadData_p->hostaddr = threadData_p->preHostaddr;
    }
    
    NHTTPi_memclr(threadData_p->preHostname, LEN_HOST_NAME);
    NHTTPi_memcpy(threadData_p->preHostname, hostname_p, NHTTPi_strnlen(hostname_p, LEN_HOST_NAME - 1));

    // PROXYが設定されている場合はポート番号もPROXYの設定を使用する。
    threadData_p->port = req_p->port;
    if (req_p->isProxy)
    {
        threadData_p->port = req_p->proxyPort;
    }
 
    // SSLなら毎回接続し直す。
    // (前回の要求におけるCAが今回のものと同一かどうか、ライブラリ側で保証できないため)
    // 接続先アドレス・ポート番号が異なる場合も接続し直す。
    if ((threadData_p->hostaddr != threadData_p->preHostaddr) ||
        (threadData_p->port != threadData_p->prePort) ||
        (req_p->isSSL == TRUE))
    {
        threadData_p->isKeepAlive = FALSE;
        DEBUG_PRINTF("[%s][%d] isKeepAlive = %d \n", __func__,  __LINE__, threadData_p->isKeepAlive);        
    }

    // ホストアドレスを覚えておく
    threadData_p->preHostaddr = threadData_p->hostaddr;
    // ポート番号を覚えておく
    threadData_p->prePort = threadData_p->port;
    
    return TRUE;
}
/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadConnectProc
  Description:  ホストに接続
                KeepAlive時は接続済みなため、ほぼ処理無し
  Arguments:    threadData_p
  Returns:      TRUE  成功
                FALSE 失敗
 *---------------------------------------------------------------------------*/
static BOOL NHTTPi_ThreadConnectProc(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);

    // 本当に接続か継続されているか調べる
    // 継続されていないのであればisKeepAliveは無効にして
    // 再接続しなければならない
    if (threadData_p->isKeepAlive == TRUE) {
        if (NHTTPi_checkKeepAlive(bgnEndInfo_p->socket) == -1) {
            threadData_p->isKeepAlive = FALSE;
        }
    }
        
    if (threadData_p->isKeepAlive == FALSE)
    {
        // ソケットが有効ならば終了
        if (IS_SOCKET(bgnEndInfo_p->socket))
        {
            DEBUG_PRINTF("NHTTPi_SocClose [%d]\n", __LINE__);
            if (NHTTPi_SocClose(mutexInfo_p, req_p, bgnEndInfo_p->socket) < 0)
            {
                // 終了失敗
                DEBUG_PRINTF("[%s][%d] INVALID_SOCKET \n", __func__, __LINE__);
                bgnEndInfo_p->socket = INVALID_SOCKET;
                threadData_p->result = NHTTP_ERROR_NETWORK;
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return FALSE;
            }
        }

        // ソケットを開く
        bgnEndInfo_p->socket = NHTTPi_SocOpen(req_p);
            
        if (!IS_SOCKET(bgnEndInfo_p->socket))
        {
            threadData_p->result = NHTTP_ERROR_SOCKET;
            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
            return FALSE;
        }
        
        NHTTPi_lockReqList(mutexInfo_p);
        {
            reqInfo_p->reqCurrent->socket_p = (void*)bgnEndInfo_p->socket;
        }
        NHTTPi_unlockReqList(mutexInfo_p);

        // キャンセルチェック
        if (req_p->doCancel)
        {
            DEBUG_PRINTF("[%s][%d] キャンセル処理が入りました\n", __func__, __LINE__);
            return FALSE;
        }
                   
        DEBUG_PRINTF("[%s][%d] Open TCP socket %d hostaddr %lx port %d\n",
                        __func__, threadData_p->id, bgnEndInfo_p->socket, threadData_p->hostaddr, threadData_p->port);
        if (NHTTPi_SocConnect(bgnEndInfo_p, mutexInfo_p, req_p, bgnEndInfo_p->socket, threadData_p->hostaddr, threadData_p->port) < 0)
        {
            // 接続失敗
            if (req_p->isProxy)
            {
                threadData_p->result = NHTTP_ERROR_CONNECT_PROXY;
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return FALSE;
            }
            else if (NHTTPi_GetSSLError(bgnEndInfo_p) != NHTTP_SSL_ENONE)
            {
                threadData_p->result = NHTTP_ERROR_SSL;
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return FALSE;
            }
            else
            {
                threadData_p->result = NHTTP_ERROR_CONNECT;
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return FALSE;
            }
        }
            
        DEBUG_PRINTF("[%s][%d] Connect TCP socket %d hostaddr %lx port %d\n",
                        __func__, threadData_p->id, bgnEndInfo_p->socket, threadData_p->hostaddr, threadData_p->port);
    }
    else
    {
        NHTTPi_lockReqList(mutexInfo_p);
        {
            DEBUG_PRINTF("[%s][%d] KeepAlive TCP socket %d hostaddr %lx port %d\n",
                            __func__, threadData_p->id, bgnEndInfo_p->socket, threadData_p->hostaddr, threadData_p->port);
            reqInfo_p->reqCurrent->socket_p = (void*)bgnEndInfo_p->socket;
        }
        NHTTPi_unlockReqList(mutexInfo_p);
    }
    
    return TRUE;
}
         
/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadProxyProc
  Description:  プロキシサーバー接続
  Arguments:    threadData_p
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int NHTTPi_ThreadProxyProc(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;

    int                 sendStatus = 0;
    
    threadData_p->result = NHTTP_ERROR_NETWORK;
            
    // 送信バッファ位置を初期化
    threadData_p->sendBufwp = 0;
#if defined(NHTTP_USE_NSSL) || defined(NHTTP_USE_CPSSSL)
    // PROXY経由のSSL接続は最初にCONNECTメソッドを送る必要がある
    if (req_p->isSSL && req_p->isProxy)
    {
        int SSLConnectResult = NHTTPi_NB_SUCCESS;
            
        DEBUG_PRINTF("[%s][%d] \n", __func__, threadData_p->id);
        /////////////////////////////////////////////////////
        //
        // PROXY経由のためのCONNECTメソッドを送信
        //
        sendStatus = NHTTPi_SendProxyConnectMethod(threadData_p);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        
        /////////////////////////////////////////////////////
        //
        //  PROXYサーバーの返答待ち
        //
        if (NHTTPi_RecvProxyConnectHeader(threadData_p) == FALSE)
        {
            return NHTTP_SEND_ERROR_e;
        }
        
        // SSL接続開始
        DEBUG_PRINTF("[%s][%d] SSL connection start\n", __func__, threadData_p->id);
        SSLConnectResult = NHTTPi_SocSSLConnect(bgnEndInfo_p, mutexInfo_p, req_p, bgnEndInfo_p->socket);
        // 接続成功
        if (SSLConnectResult == NHTTPi_NB_SUCCESS)
        {
            DEBUG_PRINTF("[%s][%d] SSL connection OK\n", __func__, threadData_p->id);
        }
        // ROOTCAの設定に失敗した
        else if (SSLConnectResult == NHTTPi_NB_ERROR_SSL_ROOTCA)
        {
            if (NHTTPi_GetSSLError(bgnEndInfo_p) != NHTTP_SSL_ENONE)
            {
                threadData_p->result = NHTTP_ERROR_SSL_ROOTCA;
            }
            return NHTTP_SEND_ERROR_e;
        }
        // CLIENTCERTの設定に失敗した
        else if (SSLConnectResult == NHTTPi_NB_ERROR_SSL_CLIENTCERT)
        {
            if (NHTTPi_GetSSLError(bgnEndInfo_p) != NHTTP_SSL_ENONE)
            {
                threadData_p->result = NHTTP_ERROR_SSL_CLIENTCERT;
            }
            return NHTTP_SEND_ERROR_e;
        }
        // その他のエラー
        else
        {
            DEBUG_PRINTF("[%s][%d] SSL connection NG\n", __func__, threadData_p->id);
            if (NHTTPi_GetSSLError(bgnEndInfo_p) != NHTTP_SSL_ENONE)
            {
                threadData_p->result = NHTTP_ERROR_SSL;
            }
            return NHTTP_SEND_ERROR_e;
        }
    }
#else
    (void)sendStatus;
#endif
    return NHTTP_SEND_SUCCESS_e;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadSendProc
  Description:  HTTPヘッダー送信
  Arguments:    threadData_p
  Returns:      NHTTP_SEND_SUCCESS_e    成功
                NHTTP_SEND_NOTCONN_e    まだ接続できていない
                NHTTP_SEND_ERROR_e      エラー
 *---------------------------------------------------------------------------*/
static int NHTTPi_ThreadSendProc( NHTTPThreadData* threadData_p )
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_Request2Connection(mutexInfo_p, req_p);
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    char*		commBuf_p = (char*)&(threadInfo_p->commBuf[0]);
    int                 urlLength = (int)NHTTPi_strlen( req_p->URL_p );
    int                 sendStatus = NHTTP_SEND_SUCCESS_e;
    BOOL                isMultipartForm = FALSE;
    NHTTPi_DATALIST*    datalist_p;
    
    DEBUG_PRINTF("[%s][%d]\n", __func__, threadData_p->id);
    
    threadData_p->result = NHTTP_ERROR_NETWORK;
    
    if (connection_p != NULL)
    {
        connection_p->status = NHTTP_STATUS_SENDING;
    }
    
    // 送信バッファ位置を初期化
    threadData_p->sendBufwp = 0;
    
    // HTTPメソッド送信
    switch (req_p->method)
    {
    case NHTTP_REQMETHOD_GET:
        sendStatus = NHTTPi_SendData(threadData_p, "GET ", 4);
        break;
    case NHTTP_REQMETHOD_POST:
        sendStatus = NHTTPi_SendData(threadData_p, "POST ", 5);
        break;
    case NHTTP_REQMETHOD_HEAD:
        sendStatus = NHTTPi_SendData(threadData_p, "HEAD ", 5);
        break;
    default:
        break;
    }
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 

    // PROXY経由の場合でSSLではない場合はURLすべてSENDする必要がある
    if (req_p->isProxy && !req_p->isSSL)
    {
        sendStatus = NHTTPi_SendData(threadData_p,  req_p->URL_p, urlLength );
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    }
    else
    {
        if (urlLength > req_p->hostDescLen)
        {
            sendStatus = NHTTPi_SendData(threadData_p, &req_p->URL_p[req_p->hostDescLen], urlLength - req_p->hostDescLen);
            if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        }
        else
        {
            sendStatus = NHTTPi_SendData(threadData_p, "/", 1);
            if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        }
    }
    sendStatus = NHTTPi_SendData(threadData_p, " HTTP/" NHTTP_STR_HTTPVER "\r\n", 11);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    
    // HOST行
    {
        int n = req_p->isSSL? 8 : 7;

        sendStatus = NHTTPi_SendData(threadData_p, "Host: ", 6);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        sendStatus = NHTTPi_SendData(threadData_p, &req_p->URL_p[n], req_p->hostLen - n);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
        sendStatus = NHTTPi_SendData(threadData_p,  "\r\n", 2);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    }
    
    // SSLはすでにPROXY Basic認証を送信済みなので、SSL通信以外はここで送信する
    if (req_p->isProxy && !req_p->isSSL)
    {
        // 認証用のユーザ名とパスワードを送信
        sendStatus = NHTTPi_SendProxyAuthorization(threadData_p);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    }
            
    // Basic認証
    sendStatus = NHTTPi_SendBasicAuthorization(threadData_p);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
                        
    // 付加ヘッダ
    sendStatus = NHTTPi_SendHeaderList(threadData_p);
    if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 

    // POST用ヘッダ
    if (req_p->method == NHTTP_REQMETHOD_POST)
    {
        int postResult = NHTTP_SEND_SUCCESS_e;
        if (req_p->isRawData)
        {
            DEBUG_PRINTF("[%s][%d] NHTTP_ENCODING_TYPE_RAW\n", __func__, threadData_p->id);
            postResult = NHTTPi_SendProcPostDataRaw(threadData_p);
        }
        else
        {
            NHTTPEncodingType a_encodingType = req_p->encodingType;
            // 自動判別
            if (a_encodingType == NHTTP_ENCODING_TYPE_AUTO)
            {
                DEBUG_PRINTF("[%s][%d] NHTTP_ENCODING_TYPE_AUTO\n", __func__, threadData_p->id);
                // バイナリデータが含まれるか？
                for (isMultipartForm = FALSE, datalist_p = req_p->listPost_p; datalist_p; datalist_p = datalist_p->next_p)
                {
                    if (datalist_p->isBinary)
                    {
                        isMultipartForm = TRUE;
                        break;
                    }
                    if (datalist_p == req_p->listPost_p->prev_p)
                    {
                        break;
                    }
                }
            }
            // multipart/form-data
            else if (a_encodingType == NHTTP_ENCODING_TYPE_MULTIPART)
            {
                DEBUG_PRINTF("[%s][%d] NHTTP_ENCODING_TYPE_MULTIPART\n", __func__, threadData_p->id);
                isMultipartForm = TRUE;
            }
            // application/x-www-form-urlencoded
            else
            {
                DEBUG_PRINTF("[%s][%d] NHTTP_ENCODING_TYPE_URL\n", __func__, threadData_p->id);
                isMultipartForm = FALSE;
            }

            if (isMultipartForm)
            {
                postResult = NHTTPi_SendProcPostDataBinary(threadData_p);
            }
            else
            {
                postResult = NHTTPi_SendProcPostDataAscii(threadData_p);
            }
        }

        // 送信に失敗
        if (postResult != NHTTP_SEND_SUCCESS_e)
        {
            if (postResult == NHTTP_SEND_ERROR_POST_CALLBACK_e)
            {
                // ストリーミング送信のPOSTデータ送信のコールバックが負の値を返した場合のエラー
                // 旧処理にあわせてNHTTP_ERROR_SOCKETを設定
                threadData_p->result = NHTTP_ERROR_SOCKET;
            }
            return postResult;
        }
    }
    else
    {
        sendStatus = NHTTPi_SendData(threadData_p,  "\r\n", 2);
        if (sendStatus != NHTTP_SEND_SUCCESS_e) return sendStatus; 
    }

    sendStatus = NHTTP_SEND_SUCCESS_e;
    // 残りがあれば送信
    if (threadData_p->sendBufwp > 0)
    {
        int	stat = NHTTPi_SocSend(req_p, bgnEndInfo_p->socket, commBuf_p, threadData_p->sendBufwp, 0);

        // 送信バッファ位置を初期化
        threadData_p->sendBufwp = 0;
        NHTTPi_memclr(commBuf_p, LEN_COMMBUF);
        
        if (stat < 0)
        {
            sendStatus =  NHTTP_SEND_ERROR_e;
        }
        if (stat == 0)
        {
            sendStatus = NHTTP_SEND_NOTCONN_e;
        }
    }
    
    // 送信バッファ位置を初期化
    threadData_p->sendBufwp = 0;
    NHTTPi_memclr(commBuf_p, LEN_COMMBUF);
    
    return sendStatus;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadRecvHeaderProc
  Description:  HTTPヘッダー受信
  Arguments:    threadData_p
  Returns:      TRUE  成功
                FALSE 失敗                
 *---------------------------------------------------------------------------*/
static int NHTTPi_ThreadRecvHeaderProc(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPRes*           res_p = req_p->res_p;
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_Request2Connection(mutexInfo_p, req_p);
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    
    char                tmpBuf[4] = {0,};
    int                 recvbyte = 0;
    NHTTPi_HDRBUFLIST*  headerBuf_p = NULL;

    
    if (connection_p != NULL)
    {
        connection_p->status = NHTTP_STATUS_RECEIVING_HEADER;
    }

    res_p->headerLen = 0;
    NHTTPi_memclr(threadData_p->tmpHeaderBuf, TMP_HEADER_BUF_SIZE);
    
    
    // 二重改行までrecvする
    for (headerBuf_p = res_p->hdrBufBlock_p, threadData_p->recvBufwp = 0; ; )
    {
        // キャンセルチェック
        if (req_p->doCancel)
        {
            DEBUG_PRINTF("[%s][%d] キャンセル処理が入りました\n", __func__, __LINE__);
            return FALSE;
        }
                
        if (threadData_p->recvBufwp < NHTTP_HDRRECVBUF_INILEN)
        {
            recvbyte = NHTTPi_SocRecv(mutexInfo_p, req_p, bgnEndInfo_p->socket, &res_p->hdrBufFirst[threadData_p->recvBufwp], 1, 0);
            tmpBuf[threadData_p->recvBufwp & 3] = res_p->hdrBufFirst[threadData_p->recvBufwp];
        }
        else
        {

            int wp = threadData_p->recvBufwp & NHTTP_HDRRECVBUF_BLOCKMASK;

            if (wp == 0)
            {
                        
                if (headerBuf_p)
                {
                    headerBuf_p->next_p = (NHTTPi_HDRBUFLIST*)NHTTPi_alloc( sizeof(NHTTPi_HDRBUFLIST), 4);
                    headerBuf_p = headerBuf_p->next_p;
                }
                else
                {
                    headerBuf_p = (NHTTPi_HDRBUFLIST*)NHTTPi_alloc( sizeof(NHTTPi_HDRBUFLIST), 4);
                    res_p->hdrBufBlock_p = headerBuf_p;
                }
                        
                if (!headerBuf_p)
                {
                    PRINTF("NHTTP_ERROR_ALLOC function : %s line : %d\n", __func__, __LINE__);
                    threadData_p->result = NHTTP_ERROR_ALLOC;
                    DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                    return FALSE;
                }
                headerBuf_p->next_p = NULL;
            }
            recvbyte = NHTTPi_SocRecv(mutexInfo_p, req_p, bgnEndInfo_p->socket, &headerBuf_p->block[wp], 1, 0);
            tmpBuf[threadData_p->recvBufwp & 3] = headerBuf_p->block[wp];
        }
             
        if (recvbyte <= 0)
        {
            threadData_p->result = NHTTP_ERROR_NETWORK;
            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
            return FALSE;
        }
                
        threadData_p->recvBufwp += recvbyte;
                
        //if ( threadData_p->recvBufwp >= 4 )
        {
            if (NHTTPi_CheckHeaderEnd(tmpBuf, threadData_p->recvBufwp))
            {
                // ヘッダ終わり
                res_p->headerLen = threadData_p->recvBufwp;
                break;
            }
        }
    } // for (headerBuf_p = res_p->hdrBufBlock_p, threadData_p->recvBufwp = 0; ; )
            
    if (res_p->headerLen == 0)
    {
        // エラー値セット（なんか内容にそぐわない気がしますが…DSと一致させるため）
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
        return FALSE;
    }
     
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadParseHeaderProc
  Description:  HTTPヘッダー解析
  Arguments:    threadData_p
  Returns:      TRUE  成功
                FALSE 失敗                
 *---------------------------------------------------------------------------*/
static int NHTTPi_ThreadParseHeaderProc(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPRes*           res_p = req_p->res_p;
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    char*		commBuf_p = (char*)&(threadInfo_p->commBuf[0]);
    int                 labelend;
    int                 n;
    
    // HTTPヘッダを解析
    // とりあえずStatus-Lineからstatus codeをとっておく
    if (!NHTTPi_loadFromHdrRecvBuf(res_p, threadData_p->tmpHeaderBuf, 0, 14))
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
        return FALSE;
    }
            
    //DEBUG_PRINTF("[%d] threadData_p->tmpHeaderBuf [%s]\n", id, threadData_p->tmpHeaderBuf);
            
    if (NHTTPi_strnicmp(threadData_p->tmpHeaderBuf, "HTTP/", 5) != 0)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
        return FALSE;
    }
    if (threadData_p->tmpHeaderBuf[8] != ' ')
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
        return FALSE;
    }
    res_p->httpStatus = NHTTPi_strToInt(&threadData_p->tmpHeaderBuf[9], 3);

    DEBUG_PRINTF("[%s][%d] httpStatus %d\n", __func__, threadData_p->id, res_p->httpStatus);
    
    if (res_p->httpStatus < 0)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
        return FALSE;
    }
            
    if (NHTTPi_findNextLineHdrRecvBuf(res_p, 12, res_p->headerLen, &labelend, NULL) < 0)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
        return FALSE;
    }
            
    // Content-Lengthがなければconnection resetされるまで受信
    threadData_p->contentLength = NHTTPi_getHeaderValue(res_p, "Content-Length", &n);		// この時点では文字数

    DEBUG_PRINTF("[%s][%d] threadData_p->contentLength %d\n", __func__, threadData_p->id, threadData_p->contentLength);
    
    if (threadData_p->contentLength == 0)
    {
        threadData_p->result = NHTTP_ERROR_NONE; // ここだけERROR_NONE
        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
        return FALSE;
    }
    
    if (threadData_p->contentLength > LEN_COMMBUF)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
        return FALSE;
    }
            
    if (threadData_p->contentLength > 0)
    {
        if (!NHTTPi_loadFromHdrRecvBuf(res_p, commBuf_p, n, threadData_p->contentLength))
        {
            threadData_p->result = NHTTP_ERROR_HTTPPARSE;
            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
            return FALSE;
        }
        threadData_p->contentLength = NHTTPi_strToInt( commBuf_p, threadData_p->contentLength );
        if (threadData_p->contentLength < 0)
        {
            threadData_p->result = NHTTP_ERROR_HTTPPARSE;
            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
            return FALSE;
        }
        res_p->contentLength = threadData_p->contentLength;
    }
    else
    {
        // Content-Lengthなし
        res_p->contentLength = 0xffffffff;
    }

    // ConnectionがKeep-Aliveかどうか
    // SSLなら無効
    //DEBUG_PRINTF("ConnectionがKeep-Aliveかどうか\n");
            
    if (req_p->isSSL)
    {
        threadData_p->isKeepAlive = FALSE;
    }
    else
    {
        int connectionHeaderStrLen = NHTTPi_getHeaderValue(res_p, "Connection", &n);
        if (connectionHeaderStrLen == 0)
        {
            threadData_p->result = NHTTP_ERROR_HTTPPARSE;
            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
            threadData_p->isKeepAlive = FALSE;
            return FALSE;
        }
        if (connectionHeaderStrLen > LEN_COMMBUF)
        {
            threadData_p->isKeepAlive = FALSE;
        }
        else
        {
            if (connectionHeaderStrLen > 0)
            {
                if (NHTTPi_compareTokenN_HdrRecvBuf(res_p, n, n+connectionHeaderStrLen, "Keep-Alive", 0) == 0)
                {
                    threadData_p->isKeepAlive = TRUE;
                }
                else
                {
                    threadData_p->isKeepAlive = FALSE;
                }
            }
            else
            {
                threadData_p->isKeepAlive = FALSE;
            }
        }
        DEBUG_PRINTF("[%s][%d] connectionHeaderStrLen %d LEN_COMMBUF %d isKeepAlive = %d \n",
                     __func__,  __LINE__, connectionHeaderStrLen, LEN_COMMBUF, threadData_p->isKeepAlive);
    }
    
    // Transfer-Encodingがchunkedかどうか
    //DEBUG_PRINTF("Transfer-Encodingがchunkedかどうか\n");
            
    threadData_p->isChunked = NHTTPi_getHeaderValue(res_p, "Transfer-Encoding", &n);
    if (threadData_p->isChunked == 0)
    {
        threadData_p->result = NHTTP_ERROR_HTTPPARSE;
        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
        return FALSE;
    }
    if (threadData_p->isChunked > LEN_COMMBUF)
    {
        threadData_p->isChunked = FALSE;
    }
    else
    {
        threadData_p->isChunked = (threadData_p->isChunked > 0) ? (NHTTPi_compareTokenN_HdrRecvBuf(res_p, n, n+threadData_p->isChunked, "chunked", ';') == 0): FALSE;
    }

    DEBUG_PRINTF("[%s][%d] header parsed\n", __func__, threadData_p->id);
    
    // NHTTP_REQMETHOD_HEAD時に正常に完了した場合でも NHTTP_ERROR_HTTPPARSE になっていたバグ修正
    // PARSEが完了したのでエラー無しに
    threadData_p->result = NHTTP_ERROR_NONE;
    // BODY受信に失敗してもヘッダーのパースが成功していればヘッダーが確認できるようにする
    res_p->isHeaderParse = TRUE;
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_ThreadRecvBodyProc
  Description:  HTTPボディ受信
  Arguments:    threadData_p 
  Returns:      TRUE  成功
                FALSE 失敗                
 *---------------------------------------------------------------------------*/
static int NHTTPi_ThreadRecvBodyProc(NHTTPThreadData* threadData_p)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPReq*           req_p = reqInfo_p->reqCurrent->req_p;
    NHTTPRes*           res_p = req_p->res_p;
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPConnection*	connection_p = NHTTPi_Request2Connection(mutexInfo_p, req_p);
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    char*		commBuf_p = (char*)&(threadInfo_p->commBuf[0]);
    int                 recvbyte = 0;
    char                tmpBuf[2];
    int                 n;
    
    DEBUG_PRINTF("[%s][%d] contentLength %d HttpStatus %d \n", __func__, threadData_p->id, threadData_p->contentLength, res_p->httpStatus);
    

    // サーバーは以下の条件ではメッセージボディを含んではならないため
    // クライアントはボディ受信をスキップする必要がある
    // HEADリクエスト
    // 1xx 204 304レスポンス
    if ((req_p->method == NHTTP_REQMETHOD_HEAD) ||
        (res_p->httpStatus == 204) ||
        (res_p->httpStatus == 304) ||
        (res_p->httpStatus >= 100 && res_p->httpStatus < 200))
    {
        return TRUE;
    }
            
    DEBUG_PRINTF("[%s][%d]  BODY受信\n", __func__, threadData_p->id );

    NHTTPi_SetVirtualContentLength(connection_p, 0);
    
    if (connection_p != NULL)
    {
        connection_p->status = NHTTP_STATUS_RECEIVING_BODY;
    }
    
    if (threadData_p->contentLength >= 0)
    {

        DEBUG_PRINTF("[%s][%d] contentLength >= 0 \n", __func__, threadData_p->id );
        
        NHTTPi_SetVirtualContentLength(connection_p, (u32)threadData_p->contentLength);
        
        // Content-Lengthあり
        for ( ; (threadData_p->contentLength > 0); )
        {
            if (threadData_p->result != NHTTP_ERROR_BUFFULL)
            {
                if (NHTTPi_BufFull(mutexInfo_p, res_p) == FALSE)
                {
                    threadData_p->result = NHTTP_ERROR_BUFFULL;
                    // 読み捨て用バッファセット
                    res_p->recvBuf_p = &(threadData_p->tmpBodyBuf[0]);
                    res_p->recvBufLen = TMP_BODY_BUF_SIZE;
                }
            }
                    
            if (threadData_p->result == NHTTP_ERROR_BUFFULL)
            {
                recvbyte = NHTTPi_RecvBufN(mutexInfo_p, req_p, bgnEndInfo_p->socket, 0, threadData_p->contentLength, 0);
            }
            else
            {
                recvbyte = NHTTPi_RecvBufN(mutexInfo_p, req_p, bgnEndInfo_p->socket, res_p->bodyLen, threadData_p->contentLength, 0);
            }

            if (recvbyte < 0)
            {
                DEBUG_PRINTF("[%s][%d] NHTTPi_SocRecv error = %d\n", __func__, threadData_p->id, recvbyte);
                
                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                return FALSE;
            }
            else if (recvbyte == 0)
            {
                DEBUG_PRINTF("[%s][%d] Connection Closed.\n", __func__, threadData_p->id);
                break;
            }
            DEBUG_PRINTF("[%s][%d] body %d received %d\n",  __func__, threadData_p->id, threadData_p->contentLength, recvbyte);
            
            if (threadData_p->result != NHTTP_ERROR_BUFFULL)
            {
                res_p->bodyLen += recvbyte;
                res_p->totalBodyLen += recvbyte;
            }
            threadData_p->contentLength -= recvbyte;
        }
                
        if (threadData_p->result != NHTTP_ERROR_BUFFULL)
        {
            if (threadData_p->contentLength != 0)
            {
                threadData_p->result = NHTTPi_isRecvBufFull(res_p, res_p->bodyLen)? NHTTP_ERROR_BUFFULL : NHTTP_ERROR_NETWORK;
            }
            else
            {
                threadData_p->result = NHTTP_ERROR_NONE;
            }
        }
    }
    else
    {
        threadData_p->result = NHTTP_ERROR_NETWORK;

        DEBUG_PRINTF("[%s][%d] threadData_p->contentLength < 0 \n", __func__, threadData_p->id );           

        // Content-Lengthなし
        if (threadData_p->isChunked)
        {
            // Transfer-Encoding: chunked
            int chunkSize = -1;

            DEBUG_PRINTF("[%s][%d] chunked \n", __func__, threadData_p->id);
            
            while (1)
            {
                // chunk sizeを得る
                // size=0ならlast chunk
                tmpBuf[0] = 0;
                tmpBuf[1] = 0;
                for (threadData_p->recvBufwp = 0; threadData_p->recvBufwp < LEN_COMMBUF; threadData_p->recvBufwp++)
                {
                    recvbyte = NHTTPi_SocRecv(mutexInfo_p, req_p, bgnEndInfo_p->socket, &(commBuf_p[threadData_p->recvBufwp]), 1, 0);
                    if (recvbyte < 0)
                    {
                        DEBUG_PRINTF("[%s][%d] NHTTPi_SocRecv error = %d\n", __func__, threadData_p->id, recvbyte);
                        DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                        return FALSE;
                    }
                                
                    tmpBuf[threadData_p->recvBufwp & 1] = commBuf_p[threadData_p->recvBufwp];
                    if ((tmpBuf[threadData_p->recvBufwp & 1] == ';') ||
                        ((tmpBuf[threadData_p->recvBufwp & 1] == '\n') && (tmpBuf[ (threadData_p->recvBufwp-1) & 1 ] == '\r')))
                    {
                        if (tmpBuf[threadData_p->recvBufwp & 1] == '\n')
                        {
                            n = threadData_p->recvBufwp - 1;
                        }
                        else
                        {
                            n = threadData_p->recvBufwp;
                            if (NHTTPi_SkipLineBuf(mutexInfo_p, req_p, bgnEndInfo_p->socket) <= 0)
                            {
                                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                                return FALSE;
                            }
                        }
                        
                        if (n == 0)
                        {
                            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                            return FALSE;
                        }
                                    
                        chunkSize = NHTTPi_strToHex(commBuf_p, n);
                        if (chunkSize < 0)
                        {
                            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                            return FALSE;
                        }
                                    
                        break;
                    }
                }
                if (threadData_p->recvBufwp == LEN_COMMBUF)
                {
                    DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                    threadData_p->result = NHTTP_ERROR_HTTPPARSE;
                    return FALSE;
                }

                if (chunkSize > 0)
                {
                    NHTTPi_SetVirtualContentLength(connection_p, (u32)chunkSize);
                            
                    while (chunkSize > 0)
                    {
                        if (threadData_p->result != NHTTP_ERROR_BUFFULL)
                        {
                            if (NHTTPi_BufFull(mutexInfo_p, res_p) == FALSE)
                            {
                                threadData_p->result = NHTTP_ERROR_BUFFULL;
                                // 読み捨て用バッファセット
                                res_p->recvBuf_p = &(threadData_p->tmpBodyBuf[0]);
                                res_p->recvBufLen = TMP_BODY_BUF_SIZE;
                            }
                        }
                        if (threadData_p->result == NHTTP_ERROR_BUFFULL)
                        {
                            recvbyte = NHTTPi_RecvBufN(mutexInfo_p, req_p, bgnEndInfo_p->socket, 0, chunkSize, 0);
                        }
                        else
                        {
                            recvbyte = NHTTPi_RecvBufN(mutexInfo_p, req_p, bgnEndInfo_p->socket, res_p->bodyLen, chunkSize, 0);
                        }
                        if (recvbyte <= 0)
                        {
                            DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                            return FALSE;
                        }

                        res_p->bodyLen += recvbyte;
                        res_p->totalBodyLen += recvbyte;
                        chunkSize -= recvbyte;
                                    
                        if (chunkSize == 0)
                        {
                            // CRLFを読み捨てる
                            recvbyte = NHTTPi_SocRecv(mutexInfo_p, req_p, bgnEndInfo_p->socket, commBuf_p, 2, 0);
                            if (recvbyte <= 0)
                            {
                                DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                                return FALSE;
                            }
                        }
                    }
                }
                else
                {
                    // last chunk
                    NHTTPi_SkipLineBuf(mutexInfo_p, req_p, bgnEndInfo_p->socket);
                    threadData_p->result = NHTTP_ERROR_NONE;
                    break;
                }
            } // while (1)
        }
        else
        {

            DEBUG_PRINTF("[%s][%d] connection resetされるまで受信\n", __func__, threadData_p->id);
            
            // connection resetされるまで受信
            for ( ; ; )
            {

                if (NHTTPi_BufFull(mutexInfo_p, res_p) == FALSE)
                {
                    threadData_p->result = NHTTP_ERROR_BUFFULL;
                    // 読み捨て用バッファセット
                    res_p->recvBuf_p = &(threadData_p->tmpBodyBuf[0]);
                    res_p->recvBufLen = TMP_BODY_BUF_SIZE;
                }
                if (threadData_p->result == NHTTP_ERROR_BUFFULL)
                {
                    recvbyte = NHTTPi_RecvBuf(mutexInfo_p, req_p, bgnEndInfo_p->socket, 0, 0);
                }
                else
                {
                    recvbyte = NHTTPi_RecvBuf(mutexInfo_p, req_p, bgnEndInfo_p->socket, res_p->bodyLen, 0);
                }
                if (recvbyte < 0)
                {
                    DEBUG_PRINTF("[%s][%d] NHTTPi_SocRecv error = %d\n", __func__, threadData_p->id, recvbyte);
                    DEBUG_PRINTF("[%s]****** line %d comm end ******\n", __func__, __LINE__);
                    return FALSE;
                }
                else if (recvbyte == 0)
                {
                    DEBUG_PRINTF("[%s][%d] Connection Closed.\n", __func__, threadData_p->id);
                    if (threadData_p->result != NHTTP_ERROR_BUFFULL)
                    {
                        threadData_p->result = NHTTP_ERROR_NONE;
                    }
                    break;
                }
                DEBUG_PRINTF("[%s][%d] body none received %d\n", __func__, threadData_p->id, recvbyte);
                res_p->bodyLen += recvbyte;
                res_p->totalBodyLen += recvbyte;
                            
            } // for ( ; !NHTTPi_isRecvBufFull( res_p, res_p->bodyLen ); )
        } // if (isChunked)
    } // if (threadData_p->contentLength < 0)

    /** Body受信完了コールバック */
    {
        NHTTPConnection*	connection_p = NHTTPi_Response2Connection(mutexInfo_p, res_p);
        if ((threadData_p->result == NHTTP_ERROR_NONE) && (connection_p != NULL))
        {
            NHTTPi_ReceivedCallback(mutexInfo_p, connection_p);
        }
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_CommThreadProc
  Description:  HTTP通信スレッド
                NHTTPi_Startup() から NHTTPi_CleanupAsync() まで存在する
  Arguments:    param
 *---------------------------------------------------------------------------*/
extern void NHTTPi_CommThreadProcMain(void)
{
    NHTTPSysInfo*	sysInfo_p = NHTTPi_GetSystemInfoP();
    NHTTPBgnEndInfo*    bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPThreadData     threadData;

    // スレッド変数初期化
    NHTTPi_ThreadInit(&threadData);

    while (!bgnEndInfo_p->isThreadEnd)
    {
        if (threadData.restartConnect == FALSE)
        {
            // キューリストの先頭を取ってきて、実行中にする
            // キューリストが空だったら、kickされるまで待つ
            if (NHTTPi_ThreadExecReqQueue(&threadData) == FALSE)
            {
                continue;
            }
            
            // キャンセルチェック
            if (reqInfo_p->reqCurrent->req_p->doCancel)
            {
                DEBUG_PRINTF("[%s][%d] キャンセル処理が入りました\n", __func__, __LINE__);
                NHTTPi_ThreadReqEnd(&threadData); // 旧 comm_end処理を実行して先頭に戻る
                continue;            
            }
            
            // ホスト名解決プロセス
            if (NHTTPi_ThreadHostAddrProc(&threadData) == FALSE)
            {
                NHTTPi_ThreadReqEnd(&threadData); // 旧 comm_end処理を実行して先頭に戻る
                continue;            
            }
        }
        
        if (threadData.restartConnect == TRUE)
        {
            threadData.restartConnect = FALSE;
            // 旧 comm_begin:
            // comm_beginと同様の役割を果たすため、
            // SEND時に未接続の場合は restartConnect をTRUEにしてココに戻ってきます
        }
        
        // 接続プロセス
        if (NHTTPi_ThreadConnectProc(&threadData) == FALSE)
        {
            NHTTPi_ThreadReqEnd(&threadData); // 旧 comm_end処理を実行して先頭に戻る
            continue;
        }

        // PROXY関連処理
        switch (NHTTPi_ThreadProxyProc(&threadData))
        {
        case NHTTP_SEND_SUCCESS_e:            // ・次のステップへ
            break;
        case NHTTP_SEND_NOTCONN_e:            // ・接続が確立していない
            threadData.restartConnect = TRUE; // 他はスキップしてcomm_beginから接続をやり直す
            continue;
            break;
        case NHTTP_SEND_ERROR_e:              // ・エラー
            NHTTPi_ThreadReqEnd(&threadData); // 旧 comm_end処理を実行して先頭に戻る
            continue;
            break;
        }

        // HTTPリクエスト送信
        switch (NHTTPi_ThreadSendProc(&threadData))
        {
        case NHTTP_SEND_SUCCESS_e:            // ・次のステップへ
            break;
        case NHTTP_SEND_NOTCONN_e:            // ・接続が確立していない
            threadData.restartConnect = TRUE; // 他はスキップしてcomm_beginから接続をやり直す
            continue;
            break;
        case NHTTP_SEND_ERROR_e:              // ・エラー
        case NHTTP_SEND_ERROR_POST_CALLBACK_e: 
            NHTTPi_ThreadReqEnd(&threadData); // 旧 comm_end処理を実行して先頭に戻る
            continue;
            break;
        }
        
        // キャンセルチェック
        if (reqInfo_p->reqCurrent->req_p->doCancel)
        {
            DEBUG_PRINTF("[%s][%d] キャンセル処理が入りました\n", __func__, __LINE__);
            NHTTPi_ThreadReqEnd(&threadData); // 旧 comm_end処理を実行して先頭に戻る
            continue;            
        }

        // ヘッダー受信
        if (NHTTPi_ThreadRecvHeaderProc(&threadData) == FALSE)
        {
            NHTTPi_ThreadReqEnd(&threadData); // 旧 comm_end処理を実行して先頭に戻る
            continue;
        }

        // ヘッダー解析
        if (NHTTPi_ThreadParseHeaderProc(&threadData) == FALSE)
        {
            NHTTPi_ThreadReqEnd(&threadData); // 旧 comm_end処理を実行して先頭に戻る
            continue;
        }

        // ボディ受信
        if (NHTTPi_ThreadRecvBodyProc(&threadData) == FALSE)
        {
            NHTTPi_ThreadReqEnd(&threadData); // 旧 comm_end処理を実行して先頭に戻る
            continue;
        }
        
        // 旧 comm_end:
        // 終了
        NHTTPi_ThreadReqEnd(&threadData); 
    }
}
