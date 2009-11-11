/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     nhttp_request.h

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
#ifndef __NHTTP_REQUEST_H__
#define __NHTTP_REQUEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "d_nhttp_common.h"
    
/*---------------------------------------------------------------------------*
 * Types/Declarations
 *---------------------------------------------------------------------------*/

extern const char   NHTTPi_strMultipartBound[];  

/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern void     NHTTPi_InitRequestInfo          ( NHTTPReqInfo* reqInfo_p );
extern int      NHTTPi_SendRequestAsync         ( NHTTPSysInfo* sysInfo_p, NHTTPReq* req_p );
extern void     NHTTPi_cancelAllRequests        ( NHTTPSysInfo* sysInfo_p );
extern BOOL     NHTTPi_CancelRequestAsync       ( NHTTPSysInfo* sysInfo_p, int id );
extern void     NHTTPi_DestroyRequest           ( NHTTPSysInfo* sysInfo_p, NHTTPReq* req_p );
extern int      NHTTPi_destroyRequestObject     ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p );
extern NHTTPReq* NHTTPi_CreateRequest           ( NHTTPBgnEndInfo* bgnEndInfo_p, const char* url, NHTTPReqMethod method, char* buf, u32 len, void* param, NHTTPBufFull bufFull, NHTTPFreeBuf freeBuf);

#ifdef __cplusplus
}
#endif

#endif // __NHTTP_REQUEST_H__
