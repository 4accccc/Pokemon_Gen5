/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_recvbuf.h

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
#ifndef __NHTTP_RECVBUF_H__
#define __NHTTP_RECVBUF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nhttp.h"

/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern int NHTTPi_loadFromHdrRecvBuf            ( const NHTTPRes* res_p, char* dst_p, int pos, int len );
extern int NHTTPi_skipSpaceHdrRecvBuf           ( const NHTTPRes* res_p, int pos, const int limit );
extern int NHTTPi_compareTokenN_HdrRecvBuf      ( const NHTTPRes* res_p, int pos, const int limit, const char* str_p, const char extdeli );
extern int NHTTPi_findNextLineHdrRecvBuf        ( const NHTTPRes* res_p, int pos, const int limit, int* colon_pos_p, int* return_code_size_p );
extern int NHTTPi_isRecvBufFull                 ( const NHTTPRes* res_p, const int pos );
extern int NHTTPi_RecvBuf                       ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket, const int pos, const int flags );
extern int NHTTPi_RecvBufN                      ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p, const SOCKET socket, const int pos, int len, const int flags );
    

#ifdef __cplusplus
}
#endif

#endif // __NHTTP_RECVBUF_H__
