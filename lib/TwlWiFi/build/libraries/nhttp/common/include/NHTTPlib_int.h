/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTPlib.int.h

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
#ifndef __NHTTP_NHTTPLIB_INT_H__
#define __NHTTP_NHTTPLIB_INT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "d_def.h"
#if	defined(NHTTP_FOR_RVL)
#include <types.h>
#include <revolution/ncd.h>
#include <ssl.h>
typedef OSMutex NHTTPMutex;
typedef SSLId   NHTTPSSLId;
#define NHTTP_SSL_ENONE                 SSL_ENONE
#define NHTTP_SSL_VERIFY_NONE           SSL_VERIFY_NONE
#define NHTTP_SSL_ROOTCA_DEFAULT        SSL_ROOTCA_NINTENDO_0
#define NHTTP_SSL_CLIENTCERT_DEFAULT    SSL_CLIENTCERT_NINTENDO_0
#elif defined(NHTTP_FOR_NTR)

#include <nitro/types.h>

#if defined(NHTTP_USE_NSSL)
#include <twlWiFi/nssl.h>
typedef OSMutex NHTTPMutex;
typedef NSSLId  NHTTPSSLId;
#define NHTTP_SSL_ENONE                 NSSL_ENONE
#define NHTTP_SSL_VERIFY_NONE           NSSL_VERIFY_NONE
#define NHTTP_SSL_ROOTCA_DEFAULT        NSSL_ROOTCA_NINTENDO_2
#define NHTTP_SSL_CLIENTCERT_DEFAULT    0 //TODO
#elif defined(NHTTP_USE_CPSSSL)
typedef OSMutex NHTTPMutex;
typedef void*   NHTTPSSLId;
#define NHTTP_SSL_ENONE                 0
#define NHTTP_SSL_VERIFY_NONE           0
#endif

#else
//gen rvliopTypes.hを読み込まないように
#define REVOLUTION_RVLIOPTYPES_H_
#include <iop_types.h>
#include <revolution/utl/signal.h>
#endif

/*---------------------------------------------------------------------------*
 * Definitions
 *---------------------------------------------------------------------------*/

#define LOWERCASE( CHR ) ( (((CHR) >= 'A') & ((CHR) <= 'Z'))? ((CHR)-('A'-'a')) : (CHR) )
#define MIN( AA, BB )	 ( ((AA)>(BB))? (BB) : (AA) )

// 送信バッファの大きさ
// >=14 であること
#define LEN_COMMBUF     (1024 * 16)

// URL 用バッファの大きさ
#define LEN_URLBUF      512

// multipart-mixedのデリミタ文字列
// はじめの2文字は"--"であること
#define STR_POSTBOUND   "--t9Sf4yfjf1RtvDu3AA"
// はじめの"--"を除いた文字数
#define LEN_POSTBOUND   18
// PROXYアドレスの長さ
#define LEN_PROXY_ADDR  256
// host名の最大長
#define LEN_HOST_NAME   256
// Basic認証で使用するユーザー名の最大長
#define LEN_USERNAME  32
// Basic認証で使用するパスワードの最大長
#define LEN_PASSWORD  32
// Basic認証で使用するユーザー名とパスワードのBase64Encodeした際の最大長
#define LEN_BASIC_USERNAME_PASSWORD ((4*(LEN_USERNAME + LEN_PASSWORD + 1 + 2))/3 + 1)
// SORecv用一時受信バッファサイズ
#define	LEN_TMP_BUF	(32768)

/*---------------------------------------------------------------------------*
 * Types/Declarations
 *---------------------------------------------------------------------------*/
// ソケットwrapperで使う内部エラーコード
enum _NHTTPi_NB_ERROR {
    NHTTPi_NB_SUCCESS                   = 0,
    NHTTPi_NB_ERROR_TIMEOUT             = -1000,
    NHTTPi_NB_ERROR_NETWORK             = -1001,
    NHTTPi_NB_ERROR_CANCELED            = -1002,
    NHTTPi_NB_ERROR_BUFFERFULL          = -1003,
    NHTTPi_NB_ERROR_SSL_ROOTCA          = -1004,
    NHTTPi_NB_ERROR_SSL_CLIENTCERT      = -1005
};  

typedef enum {
    NHTTP_CURRENT_THREAD_IS_NHTTP_THREAD,
    NHTTP_CURRENT_THREAD_IS_NOT_NHTTP_THREAD
} NHTTPThreadIs;
   
typedef struct _NHTTPi_REQLIST {
    struct _NHTTPi_REQLIST*     prev_p;
    struct _NHTTPi_REQLIST*     next_p;
    int                         id;
    NHTTPReq*                   req_p;
    void*                       socket_p;
} NHTTPi_REQLIST;

typedef struct _NHTTPi_DATALIST {
    struct _NHTTPi_DATALIST*    prev_p;
    struct _NHTTPi_DATALIST*    next_p;
    const char*                 label_p;
    const char*                 value_p;
    u32                         length;
    BOOL                        isBinary;
} NHTTPi_DATALIST;

typedef struct _NHTTPi_HDRBUFLIST {
    struct _NHTTPi_HDRBUFLIST*  next_p;
    char                        block[ NHTTP_HDRRECVBUF_BLOCKLEN ];
} NHTTPi_HDRBUFLIST;

typedef struct _NHTTTPi_PostDataBuf {
    char*                       value_p;
    u32                         length;
} NHTTTPi_PostDataBuf;
    
typedef struct _NHTTPConnection	NHTTPConnection;
struct _NHTTPConnection
{
    NHTTPConnectionStatus       status;
    NHTTPError                  nhttpError;
    int                         sslError;

    BOOL                        running;

    /* 旧仕様の各種リソースハンドル */
    NHTTPReq                    *req_p;
    NHTTPRes                    *res_p;
    int                         reqId;
    NHTTPConnectionCallback     connectionCallback;
    NHTTPConnection*            next_p;
    NHTTTPi_PostDataBuf         postDataBuf;
    
    // 旧仕様互換 NHTTPBufFullコールバック関数用
    u32                         virtualContentLength;   //chunked size含む
    // 旧仕様互換 _NHTTPReqから移動
    NHTTPReqCallback            reqCallback;
    u16                         padding[6];

    // SORec用一時受信バッファ
    u16                         tmpBuf[(LEN_TMP_BUF/sizeof(u16))] ATTRIBUTE_ALIGN(32);
    u32                         tmpBufOffset;
    u32                         tmpBufRest;
    
    int                         ref;
};

struct _NHTTPRes {
    volatile int        headerLen;      // ヘッダ(先頭から\r\n\r\nまで)の長さ[byte]
    volatile int        bodyLen;        // ボディの長さ[byte]
    volatile int        totalBodyLen;   // ボディの長さ[byte]
    volatile int        contentLength;
    int                 isSuccess;
    int                 isHeaderParse;
    int                 httpStatus;     // httpStatus
    unsigned long       recvBufLen;     // recvBuf_pの全長
    char*               allHeader_p;
    char*               foundHeader_p;
    char*               recvBuf_p;      // エンティティボディ受信バッファ
    NHTTPBufFull	bufFull;	// 受信バッファ溢れコールバック関数
    NHTTPFreeBuf	freeBuf;	// 受信バッファ解放コールバック関数
    NHTTPi_HDRBUFLIST*  hdrBufBlock_p;
    char                hdrBufFirst[ NHTTP_HDRRECVBUF_INILEN ];
    void*               param_p;          // createRequestで設定したparam
};

struct _NHTTPReq {
    volatile BOOL       doCancel;	// 中断終了要請

    BOOL                isStarted;      // リクエスト実行中
    BOOL                isSSL;          // TRUEなら https:// 接続
    BOOL                isProxy;        // PROXY経由での接続
    BOOL                isRawData;      // POSTデータが生データ
    
    int                 hostLen;        // URL_p中 topからhost名終わりまでの文字数
    int                 hostDescLen;    // URL_p中 topから最初の「/」までの文字数
                                        //  http://abcd.efghi.com:8080/ijk.html
                                        //  |                    |    |
                                        //  0              nLenHost nLenHostDesc
    NHTTPReqMethod      method;         // リクエストメソッド
    int                 port;           // ポート
    char*               URL_p;          // リクエストURL
    char*               hostname_p;     // ホスト名
    NHTTPRes*           res_p;          
    NHTTPi_DATALIST*    listHeader_p;   
    NHTTPi_DATALIST*    listPost_p;
    
    char                tagPost[ LEN_POSTBOUND + 2 ];
    
    // Basic認証
    char                basicUsernamePassword[ LEN_BASIC_USERNAME_PASSWORD];
    u16                 padding1;
    int                 basicUsernamePasswordLen;
    
    // SSL接続時に使用
    NHTTPSSLId          sslId;
    const char*         clientCertData_p;
    size_t              clientCertSize;
    const char*         privateKeyData_p;
    size_t              privateKeySize;
    const char*         rootCAData_p;
    size_t              rootCASize;
    int                 useBuiltinRootCA;
    int                 useClientCertDefault;
    u32                 verifyOption;
    NHTTPEncodingType   encodingType;

    // 2007.06.15追加
    u32                 disableVerifyOption;    // SSLDisableVerifyOptionForDebug
    u32                 rootCAId;               // SSLSetBuiltinRootCA
    u32                 clientCertId;           // SSLSetBuiltinClientCert
    
    // PROXY経由での接続で使用
    char                proxyAddr[ LEN_PROXY_ADDR ];
    int                 proxyPort;
    char                proxyUsernamePassword[ LEN_BASIC_USERNAME_PASSWORD];
    u16                 padding2;
    int                 proxyUsernamePasswordLen;

    // 旧仕様対応のためNHTTPConnectionから移動
    u32                 soRecvSize;
    
    // 生POSTデータ
    const char*         rawPostData_p;
    u32                 rawPostDataLen;
    
    NHTTPPostSend       postSendCallback;    // POST送信コールバック関数
};

enum {
    NHTTPi_CommThreadSendData_begin_e,  // ソケットを開くところまで戻る
    NHTTPi_CommThreadSendData_error_e,  // 送信失敗
    
    NHTTPi_CommThreadSendData_end_e     // 通常終了
};

#define SOCKET 			int
    
       
    
#ifdef __cplusplus
}
#endif

#endif // __NHTTP_NHTTPLIB_INT_H__
