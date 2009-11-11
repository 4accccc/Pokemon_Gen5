/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_thread.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-19#$
  $Rev: 1094 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef __NHTTP_THREAD_H__
#define __NHTTP_THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "d_nhttp_common.h"

#define	TMP_BODY_BUF_SIZE               (512)   // 受信バッファ再確保
#define TMP_HEADER_BUF_SIZE             (14)    // ヘッダ受信用

// 送信コールバック
enum {
    SEND_TYPE_RAW_e,
    SEND_TYPE_MULTIPART_e,
    SEND_TYPE_URLENCODE_e,
    SEND_TYPE_MAX_e
};

enum {
    NHTTP_SEND_SUCCESS_e,       // 送信成功
    NHTTP_SEND_ERROR_e,         // 送信エラー
    NHTTP_SEND_NOTCONN_e,       // 接続されていない
    NHTTP_SEND_ERROR_POST_CALLBACK_e,  // POSTデータ送信コールバックエラー
    
    NHTTP_SEND_MAX_e
};

typedef struct _NHTTPThreadData {
    int         id;
    char        preHostname[LEN_HOST_NAME];
    char        tmpBodyBuf[TMP_BODY_BUF_SIZE]; // 受信バッファ再確保
    char        tmpHeaderBuf[TMP_HEADER_BUF_SIZE];
    u16         padding;
    u32         hostaddr;
    u32         preHostaddr;
    int         port;
    int         prePort;
    int         sendBufwp;
    int         recvBufwp;
    int         contentLength;
    NHTTPError  result;
    BOOL        restartConnect;
    BOOL        isKeepAlive;
    BOOL        isChunked;

} NHTTPThreadData;

/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern void     NHTTPi_InitThreadInfo   ( NHTTPThreadInfo* threadInfo_p );
extern BOOL     NHTTPi_GetProgress       ( NHTTPSysInfo* sysInfo_p, u32* received_p, u32* contentlen_p );
extern void     NHTTPi_CommThreadProcMain(void);
extern void     NHTTPi_IsCreateCommThreadMessageQueueOn ( NHTTPThreadInfo* threadInfo_p );
extern void     NHTTPi_IsCreateCommThreadMessageQueueOff( NHTTPThreadInfo* threadInfo_p );
extern int      NHTTPi_IsCreateCommThreadMessageQueue   ( NHTTPThreadInfo* threadInfo_p );
    
#ifdef __cplusplus
}
#endif

#endif // __NHTTP_THREAD_H__
