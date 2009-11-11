/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTPlib_int_socket.h

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
#ifndef __NHTTP_NHTTPLIB_INT_SOCKET_H__
#define __NHTTP_NHTTPLIB_INT_SOCKET_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "d_nhttp_common.h"
#if	defined(NHTTP_FOR_RVL)
#include <revolution/soex.h>
#elif defined(NHTTP_FOR_NTR)
#include <nitroWiFi.h>
#else
#include <rvl/socket.h>
#endif

#define SOCKET 			int
#define INVALID_SOCKET          (-1)
#define IS_SOCKET(SOC)          ((SOC) >= 0)

extern int      NHTTPi_SocOpen          ( const NHTTPReq* req_p );
extern int      NHTTPi_SocClose         ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket );
extern int      NHTTPi_SocConnect       ( NHTTPBgnEndInfo* bgnEndInfo_p, NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket, const u32 hostaddr, const int port );
extern int      NHTTPi_SocSSLConnect    ( NHTTPBgnEndInfo* bgnEndInfo_p, NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket );
extern int      NHTTPi_SocRecv          ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket, char* buf_p, const int len, const int flags );
extern int      NHTTPi_SocSend          ( const NHTTPReq* req_p, const SOCKET socket, const char* buf_p, const int len, const int flags );
extern void     NHTTPi_SocCancel        ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket );
extern u32      NHTTPi_resolveHostname  ( NHTTPReq* req_p, const char* hostname_p );
extern int      NHTTPi_checkKeepAlive   (const SOCKET socket);

extern int      NHTTPi_InitSSL          (void);
extern int      NHTTPi_FinishSSL        (void);
extern int      NHTTPi_DisableVerifyOptionForDebug(NHTTPSSLId i_sslId, u32 i_verifyOption);


#ifdef __cplusplus
}
#endif

#endif // __NHTTP_NHTTPLIB_INT_SOCKET_H__
