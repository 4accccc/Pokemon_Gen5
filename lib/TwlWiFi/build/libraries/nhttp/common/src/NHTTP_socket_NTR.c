/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_socket_NTR.c

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

#if defined(NHTTP_USE_NSSL) && defined(NHTTP_USE_CPSSSL)
#error You cannot define NHTTP_USE_NSSL and NHTTP_USE_CPSSSL at the same time.
#endif

#if defined(SDK_WIFI_INET) && defined(NHTTP_USE_CPSSSL)
#error You cannot define SDK_WIFI_INET and NHTTP_USE_CPSSSL at the same time.
#endif

#include <nitroWiFi.h>
#if defined(NHTTP_USE_NSSL)
#include <twlWiFi/nssl.h>
#endif

#include "d_nhttp_common.h"

#include "NHTTP_bgnend.h"
#include "NHTTP_nonport.h"
#include "NHTTPlib_int_socket.h"

#define NHTTP_SOC_CLOSE_TIMEOUT     10000
#define NHTTP_SOC_CLOSE_INTERVAL    500


SDK_WEAK_SYMBOL BOOL DWC_GetProxySetting( DWCProxySetting* );

#if defined(NHTTP_USE_CPSSSL)
static int NHTTPi_CpsAuthCallback(int result, SOCSslConnection* con, int level)
{
#pragma unused(con)
#pragma unused(level)
    {
        DEBUG_PRINTF("[%s] %d s:<%s>\n", __func__, level, con->subject);
        DEBUG_PRINTF("[%s]     /CN=%s/\n", __func__, con->cn);
        DEBUG_PRINTF("[%s]   i:<%s>\n", __func__, con->issuer);
    }
    if (result & SOC_CERT_OUTOFDATE)
    {
        DEBUG_PRINTF("[%s] Certificate is out-of-date\n", __func__);
        result &= ~SOC_CERT_OUTOFDATE;
    }
    if (result & SOC_CERT_BADSERVER)
    {
        DEBUG_PRINTF("[%s] Server name does not match\n", __func__);
    }
    switch (result & SOC_CERT_ERRMASK)
    {
    case SOC_CERT_NOROOTCA:
        DEBUG_PRINTF("[%s] No root CA installed\n", __func__);
        break;
    case SOC_CERT_BADSIGNATURE:
        DEBUG_PRINTF("[%s] Bad signature\n", __func__);
        break;
    case SOC_CERT_UNKNOWN_SIGALGORITHM:
        DEBUG_PRINTF("[%s] Unknown signature algorithm\n", __func__);
        break;
    case SOC_CERT_UNKNOWN_PUBKEYALGORITHM:
        DEBUG_PRINTF("[%s] Unknown public key alrorithm\n", __func__);
        break;
    }
    if (result)
        DEBUG_PRINTF("[%s] not authorized\n", __func__);
    else
        DEBUG_PRINTF("[%s] authorized\n", __func__);
    return result;
}

static int NHTTPi_CpsAuthCallbackForDebug(int, SOCSslConnection*, int)
{
    return 0;
}

static void NHTTPi_SetSSLseed(void)
{
    static u32 ssl_seed[OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32)];

    OS_GetLowEntropyData(ssl_seed);
    SOC_AddRandomSeed(ssl_seed, sizeof(ssl_seed));
}
#endif

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_DisableVerifyOptionForDebug
  Description:  
  Arguments:    i_sslId      - 
                i_verifyOption - 
  Returns:      int          - 
 *---------------------------------------------------------------------------*/
extern int NHTTPi_DisableVerifyOptionForDebug(NHTTPSSLId i_sslId, u32 i_verifyOption)
{
#if defined(NHTTP_USE_NSSL)
    return NSSL_DisableVerifyOptionForDebug(i_sslId, i_verifyOption);
#else
#pragma unused(i_sslId)
#pragma unused(i_verifyOption)
    return 0;
#endif /* NHTTP_USE_NSSL */
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_checkKeepAlive
  Description:  keepAlive
  Arguments:    req_p  処理対象のHTTPReqオブジェクトへのポインタ
  Returns:      0
                -1
 *---------------------------------------------------------------------------*/
extern int NHTTPi_checkKeepAlive(const SOCKET socket)
{
    int result = 0;
    
    if (socket == INVALID_SOCKET) {
        result = -1;
    } else {
        char dummy;
        int recvResult = SOC_Recv(socket, &dummy, 1, SOC_MSG_DONTWAIT);
        
        DEBUG_PRINTF("[%s] recvResult %d, dummy %c\n", __func__, recvResult, dummy);
        
        if (recvResult != SOC_EAGAIN) {
            result = -1;
        }
    }
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SocOpen
  Description:  新しいソケットを作成
  Arguments:    req_p  処理対象のHTTPReqオブジェクトへのポインタ
  Returns:      成功した場合は有効なソケットを返す
                失敗した場合は -1 INVALID_SOCKET を返す 
 *---------------------------------------------------------------------------*/
extern SOCKET NHTTPi_SocOpen(const NHTTPReq* req_p)
{
    // ソケットを作成
    SOCKET socket;
    
    socket = SOC_Socket(SOC_PF_INET, SOC_SOCK_STREAM, 0);

#ifndef SDK_FINALROM
    if (!IS_SOCKET(socket))
    {
        DEBUG_PRINTF("[%s] cannot create socket\n", __func__);
    }
    else
    {
        DEBUG_PRINTF("[%s] socket %08x opened ssl=%08x\n", __func__, socket, req_p->isSSL);

        if (req_p->isSSL)
        {
            DEBUG_PRINTF("[%s] going SSL...\n", __func__);
        }
    }
#endif

#ifdef SDK_WIFI_INET
    {
        u32    soRecvSize = 0;
    
        // 旧仕様対応
        if (req_p != NULL)
        {
            soRecvSize = req_p->soRecvSize;
        }
        if (IS_SOCKET(socket) && soRecvSize > 0)
        {
            DEBUG_PRINTF("[%s] set RecvSize %d\n", __func__, soRecvSize);
            SO_SetSockOpt(socket, SO_SOL_SOCKET, SO_SO_RCVBUF, &soRecvSize, sizeof(soRecvSize));
        }
    }
#elif defined(SDK_FINALROM)
#pragma unused(req_p)
#endif
    return socket;
}

#ifndef SDK_WIFI_INET
extern void SOCLi_TrashSocket(void);
#endif
/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SocClose
  Description:  ソケットを閉じる
  Arguments:    o_req  処理対象のHTTPReqオブジェクトへのポインタ
                i_socket ソケットID
  Returns:      SOCloseの値と同様 成功時 0 失敗時 負の値
 *---------------------------------------------------------------------------*/
extern int NHTTPi_SocClose(NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket)
{
#pragma unused(mutexInfo_p)

    int     error;   

#if defined(NHTTP_USE_NSSL)
    if (req_p && req_p->sslId > 0)
    {
        NSSL_Shutdown(req_p->sslId);
        req_p->sslId = 0;
    }
#endif

    error = SOC_Close(socket);
    if (error == 0 || error == SOC_EINPROGRESS)
    {
        int elapsed;
        for (
            elapsed = 0;
            ((error = SOC_Close(socket)) == SOC_EINPROGRESS) && (elapsed <= NHTTP_SOC_CLOSE_TIMEOUT);
            elapsed += NHTTP_SOC_CLOSE_INTERVAL
            )
        {
            DEBUG_PRINTF("[%s] waiting closed...\n", __func__);
            OS_Sleep(NHTTP_SOC_CLOSE_INTERVAL);
#ifndef SDK_WIFI_INET
            SOCLi_TrashSocket();
#endif
        }
    }

#if defined(NHTTP_USE_CPSSSL)
    if (error != SOC_EINPROGRESS && req_p && req_p->sslId != NULL)
    {
        // keep SOCSslConnection util complete closing socket
        NHTTPi_free(req_p->sslId);
        req_p->sslId = NULL;
    }
#endif

    if (error < 0) {
        DEBUG_PRINTF("[%s] socket %08x cannot be closed\n", __func__, socket);
    }
    return error;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SocConnect
  Description:  指定されたソケットへ接続する(非同期モードは使えない)
  Arguments:    i_req    処理対象のHTTPReqオブジェクトへのポインタ
                i_socket   ソケットID
                i_hostaddr 接続先ホストアドレス
                i_port     ポート番号
  Returns:      成功した場合    NHTTPi_NB_SUCCESS
                キャンセル時    NHTTPi_NB_ERROR_CANCELED
                失敗した場合    NHTTPi_NB_ERROR_NETWORK
  *---------------------------------------------------------------------------*/
extern int NHTTPi_SocConnect(NHTTPBgnEndInfo* bgnEndInfo_p, NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket, const u32 hostaddr, const int port)
{
#pragma unused(bgnEndInfo_p)
#pragma unused(mutexInfo_p)

    int                 error;
    SOCSockAddrIn       scadr;

    scadr.len       = sizeof(SOCSockAddrIn);
    scadr.family    = SOC_PF_INET;
    scadr.port      = SOC_HtoNs((unsigned short)port);
    scadr.addr.addr = hostaddr;
    
#if defined(NHTTP_USE_CPSSSL)
    if (req_p->isSSL && !req_p->isProxy)
    {
        error = NHTTPi_SocSSLConnect(bgnEndInfo_p, mutexInfo_p, req_p, socket);
        if (error < 0)
        {
            return error;
        }
    }
#endif

    error = SOC_Connect(socket, &scadr); 
    
    if (error < 0)
    {
        return (req_p->doCancel ? NHTTPi_NB_ERROR_CANCELED : NHTTPi_NB_ERROR_NETWORK);
    }

#if defined(NHTTP_USE_NSSL)
    if (req_p->isSSL && !req_p->isProxy)
    {
        return NHTTPi_SocSSLConnect(bgnEndInfo_p, mutexInfo_p, req_p, socket);
    }
#endif
    return NHTTPi_NB_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SocSSLConnect
  Description:  指定されたソケットへSSLを接続する
  Arguments:    req_p		処理対象のHTTPReqオブジェクトへのポインタ
                socket_p	ソケットID
  Returns:      成功した場合    NHTTPi_NB_SUCCESS
                失敗した場合    NHTTPi_NB_ERROR_NETWORK
 *---------------------------------------------------------------------------*/
extern int NHTTPi_SocSSLConnect(NHTTPBgnEndInfo* bgnEndInfo_p, NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket)
{
#if defined(NHTTP_USE_NSSL)
#pragma unused(mutexInfo_p)
    BOOL done = FALSE;

    /* Create client descriptor */
    DEBUG_PRINTF("[%s] NSSL_NewClient verifyOption 0x%x\n", __func__, req_p->verifyOption);
    req_p->sslId = NSSL_NewClient(socket, req_p->verifyOption, req_p->hostname_p);
    if (req_p->sslId <= 0)
    {
        DEBUG_PRINTF("[%s] cannot create ssl handle\n", __func__);
        return NHTTPi_NB_ERROR_NETWORK;
    }

    if (bgnEndInfo_p->forDebugCallback != NULL)
    {
        // SSLDisableVerifyOptionForDebug用設定が仕込まれている場合は呼ぶ
        if (req_p->disableVerifyOption != NHTTP_SSL_VERIFY_NONE)
        {
            bgnEndInfo_p->forDebugCallback(req_p->sslId, req_p->disableVerifyOption);
        }
    }
    
    // 指定クライアント証明書と秘密鍵をセット
    if (req_p->useClientCertDefault == TRUE)
    {
#if defined(NHTTP_FOR_RVL) //TODO
        if (NSSL_SetBuiltinClientCert(req_p->sslId, req_p->clientCertId) != NSSL_ENONE)
#endif
        {
            // CLIENTCERTの設定に失敗した
            return NHTTPi_NB_ERROR_SSL_CLIENTCERT;
        }
    }
    else if ((req_p->clientCertData_p != NULL) && (req_p->clientCertSize >= 0) &&
             (req_p->privateKeyData_p != NULL) && (req_p->privateKeySize >= 0))
    {
        if (NSSL_SetClientCert(req_p->sslId,
                req_p->clientCertData_p, req_p->clientCertSize,
                req_p->privateKeyData_p, req_p->privateKeySize) != NSSL_ENONE)
        {
            return NHTTPi_NB_ERROR_SSL_CLIENTCERT;
        }
    }

    /* Setup root cert */
    if (req_p->useBuiltinRootCA == TRUE)
    {
        if (NSSL_SetBuiltinRootCA(req_p->sslId, (NSSLRootCAId)req_p->rootCAId) < 0)
        {
            DEBUG_PRINTF("[%s] cannot set builtin root CA\n", __func__);
            return NHTTPi_NB_ERROR_SSL_ROOTCA;
        }
    }
    else if (req_p->rootCAData_p != NULL && req_p->rootCASize > 0)
    {
        if (NSSL_SetRootCA(req_p->sslId, req_p->rootCAData_p, req_p->rootCASize) != NSSL_ENONE)
        {
            DEBUG_PRINTF("[%s] cannot set root CA\n", __func__);
            return NHTTPi_NB_ERROR_SSL_ROOTCA;
        }
    }
    else
    {
        if (NSSL_SetBuiltinRootCA(req_p->sslId, bgnEndInfo_p->defaultRootCAId) < 0)
        {
            DEBUG_PRINTF("[%s] cannot set default builtin root CA\n", __func__);
            return NHTTPi_NB_ERROR_SSL_ROOTCA;
        }
    }
    while (!done)
    {
        int result = NSSL_DoHandshake(req_p->sslId);
        NHTTPi_SetSSLError(bgnEndInfo_p, result);
        switch (result)
        {
        case NSSL_ENONE:
            done = 1;
            break;

        case NSSL_EWANT_READ:
        case NSSL_EWANT_WRITE:
        case NSSL_EWANT_CONNECT:
            // for NONBLOCK socket
            OS_TPrintf("NSSL_DoHandshake should retry: 0x%08x\n", result);
            break;

        default:
            OS_TPrintf("NSSL_DoHandshake Error: 0x%08x\n", result);
            done = 1;
            break;
        }
        if (result != NSSL_ENONE)
        {
            return NHTTPi_NB_ERROR_NETWORK;
        }
    }
    return NHTTPi_NB_SUCCESS;
#elif defined(NHTTP_USE_CPSSSL)
#pragma unused(bgnEndInfo_p)
#pragma unused(mutexInfo_p)

    SOCSslConnection* pSslCon = (SOCSslConnection*)req_p->sslId;
    if (pSslCon == NULL)
    {
        pSslCon = req_p->sslId = NHTTPi_alloc(sizeof(SOCSslConnection), 4);
        NHTTPi_memclr(pSslCon, sizeof(SOCSslConnection));
    }
    
    pSslCon->ca_info = (SOCCaInfo **) req_p->rootCAData_p;
    pSslCon->ca_builtins = (int) req_p->rootCASize;
    pSslCon->server_name = req_p->hostname_p;
    if (req_p->disableVerifyOption != NHTTP_SSL_VERIFY_NONE)
    {
        pSslCon->auth_callback = NHTTPi_CpsAuthCallbackForDebug;
    }
    else
    {
        pSslCon->auth_callback = NHTTPi_CpsAuthCallback;
    }

    if (SOC_EnableSsl(socket, pSslCon) < 0)
    {
        DEBUG_PRINTF("[%s] cannot enable SSL\n", __func__);
        // [TODO] suitable error code
        return NHTTPi_NB_ERROR_NETWORK;
    }

    NHTTPi_SetSSLseed();
    
    return NHTTPi_NB_SUCCESS;
#else
#pragma unused(bgnEndInfo_p)
#pragma unused(mutexInfo_p)
#pragma unused(req_p)
#pragma unused(socket)
    return NHTTPi_NB_SUCCESS;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SocRecv_sub
  Description:  指定されたソケットからデータを受信
  Arguments:    socket	ソケットID
                buf_p
                len
                flags              
  Returns:      
 *---------------------------------------------------------------------------*/
static int NHTTPi_SocRecv_sub(NHTTPConnection* connection_p, const SOCKET socket, char* buf_p, const int len, const int flags)
{
    char*	tmpBuf_p = (char*)(&(connection_p->tmpBuf[0]));
    int		ret = 0;

    NHTTPi_ASSERT(((u32)tmpBuf_p & 0x1f) == 0);
    if (len > 0) {
        // 一時受信バッファへの読み込み
        if (connection_p->tmpBufRest == 0) {
            int	rec = SOC_Recv(socket, tmpBuf_p, LEN_TMP_BUF, flags);
            if (rec > 0) {
                connection_p->tmpBufRest = (u32)rec;
                connection_p->tmpBufOffset = 0;
            } else {	// 例外
                return rec;
            }
        }
        if (connection_p->tmpBufRest > 0) {
            int	size = (int)((len > connection_p->tmpBufRest) ? connection_p->tmpBufRest : len);
            
            NHTTPi_memcpy(buf_p, &(tmpBuf_p[connection_p->tmpBufOffset]), size);
            connection_p->tmpBufRest -= size;

            if (connection_p->tmpBufRest == 0) {
                NHTTPi_memclr(tmpBuf_p, LEN_TMP_BUF);
                connection_p->tmpBufOffset = 0;
            } else {
                connection_p->tmpBufOffset += size;
            }
            ret = size;
        }
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SocRecv
  Description:  指定されたソケットからデータを受信
  Arguments:    req_p	処理対象のHTTPReqオブジェクトへのポインタ
                socket	ソケットID
                buf_p	受信バッファ
                len	受信バッファサイズ
                flags
  Returns:      成功した場合    受信サイズ
                キャンセル時    NHTTPi_NB_ERROR_CANCELED
                失敗した場合    NHTTPi_NB_ERROR_NETWORK
 *---------------------------------------------------------------------------*/
extern int NHTTPi_SocRecv(NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket, char* buf_p, const int len, const int flags)
{
    int error;

#if defined(NHTTP_USE_NSSL)
    if (req_p->sslId > 0) {
        error = NSSL_Read(req_p->sslId, buf_p, (size_t)len);
    } else {
#endif
    NHTTPConnection*	connection_p = NHTTPi_Request2Connection(mutexInfo_p, req_p);

    if (connection_p != NULL) {
        error = NHTTPi_SocRecv_sub(connection_p, socket, buf_p, len, flags);
    } else {
        return NHTTPi_NB_ERROR_NETWORK;
    }
#if defined(NHTTP_USE_NSSL)
    }
#endif
   
    if (error < 0)
    {
        if (req_p->doCancel)
        {
            return NHTTPi_NB_ERROR_CANCELED;
        }
        
        // まだ接続されていない
        if (error == SOC_ENOTCONN)
        {
            return 0;
        }
        
        return NHTTPi_NB_ERROR_NETWORK;
    }
    
    return error;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SocSend_sub
  Description:  指定されたソケットを通してデータを送信
                バッファのキャッシュラインを考慮し
                32バイトアライメントをかけ 先端/中間/終端の３回に分け送信
  Arguments:    socket	ソケットID
                buf_p
                len
                flags
  Returns:      
 *---------------------------------------------------------------------------*/
static int NHTTPi_SocSend_sub(const SOCKET socket, const char* buf_p, const int len, const int flags)
{
    return SOC_Send(socket, buf_p, (int)len, flags);
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SocSend
  Description:  指定されたソケットを通してデータを送信
  Arguments:    req_p	処理対象のHTTPReqオブジェクトへのポインタ
                socket	ソケットID
                buf_p	送信バッファ
                len	送信サイズ
                flags  
  Returns:      成功した場合    送信サイズ
                キャンセル時    NHTTPi_NB_ERROR_CANCELED
                失敗した場合    NHTTPi_NB_ERROR_NETWORK
 *---------------------------------------------------------------------------*/
extern int NHTTPi_SocSend(const NHTTPReq* req_p, const SOCKET socket, const char* buf_p, const int len, const int flags)
{
    int error;

#if defined(NHTTP_USE_NSSL)
    if (req_p->sslId > 0)
    {
        error = NSSL_Write(req_p->sslId, buf_p, (size_t)len);
    }
    else
    {
#endif
        error = NHTTPi_SocSend_sub(socket, buf_p, len, flags);
        DEBUG_PRINTF("[%s] error %d [%s] req_p->sslId %d\n", __func__, error, buf_p, req_p->sslId );
#if defined(NHTTP_USE_NSSL)
    }
#endif
    if (error < 0)
    {
        if (req_p->doCancel)
        {
            return NHTTPi_NB_ERROR_CANCELED;
        }
        
        // まだ接続されていない
        if (error == SOC_ENOTCONN)
        {
            return 0;
        }
        
        return NHTTPi_NB_ERROR_NETWORK;
    }
    
    return error;
}

/*---------------------------------------------------------------------------*
  Name:         NHTTPi_SocCancel
  Description:  指定されたソケットがブロック中ならキャンセル
  Arguments:    i_req  処理対象のHTTPReqオブジェクトへのポインタ
                i_socket ソケットID
  Returns:      なし
 *---------------------------------------------------------------------------*/
extern void NHTTPi_SocCancel(NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket)
{
    (void)req_p;
    
    NHTTPi_lockReqList(mutexInfo_p);
   
    if (IS_SOCKET(socket))
    {
        //2006.11.08
        // SSLはKeepAliveが指定されることが無く、Cancelが通った後の
        // SocCloseで必ずSSLShutdownが呼ばれるので、
        // SSLShutdownを呼ぶ必要はありません
        SOC_Shutdown(socket, SOC_SHUT_RDWR);
    }
    
    NHTTPi_unlockReqList(mutexInfo_p);
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_resolveHostname
  Description:  ホスト名を解決
  Arguments:    req_p  処理対象のNHTTPReqオブジェクトへのポインタ
  Returns:      !=0  IPv4アドレス
 *-------------------------------------------------------------------------*/
extern u32 NHTTPi_resolveHostname(NHTTPReq* req_p, const char* hostname_p)
{
    u32         addr = 0;
#ifdef NHTTP_USE_ADDRINFO
    SOAddrInfo* addrInfo;
#else
    SOCInAddr   inAddr;
    SOCHostEnt* hostEnt = NULL;
#endif

    (void)req_p;
    
#ifdef NHTTP_USE_ADDRINFO
    if (SOGetAddrInfo(hostname_p, NULL, NULL, &addrInfo) == 0)
    {
        SOSockAddr* soAddr = (SOSockAddr*)addrInfo->addr;
        NHTTPi_memcpy(&addr, &soAddr->data[2], 4);
        SOFreeAddrInfo(addrInfo);
    }
    else
    {
        addr = 0;
    }
#else
    /* 
       IP アドレス表記を GetHostByName が処理してくれるかは
       プラットフォーム依存
     */
    if (SOC_InetAtoN(hostname_p, &inAddr))
    {
        addr = inAddr.addr;
    }
    else
    {
        hostEnt = SOC_GetHostByName(hostname_p);
        if (hostEnt && hostEnt->length > 0)
        {
            NHTTPi_memcpy(&addr, hostEnt->addrList[0], SOC_ALEN);
        }
    }
#endif
    return addr;
}

BOOL DWC_GetProxySetting( DWCProxySetting* )
{
#ifdef SDK_TWL
    OS_TWarning("The DWC-base is not linked. Proxy servers in network settings are not used.");
#endif
    return FALSE;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

