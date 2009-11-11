/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_response.h

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
#ifndef __NHTTP_RESPONSE_H__
#define __NHTTP_RESPONSE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "d_nhttp_common.h"
    
/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern void     NHTTPi_DestroyResponse       ( NHTTPMutexInfo* mutexInfo_p, NHTTPRes* res_p );
extern int      NHTTPi_GetResponseHeaderAll  ( NHTTPBgnEndInfo* bgnEndInfo_p, NHTTPRes* res_p, char** value_pp );
extern int      NHTTPi_getHeaderValue        ( const NHTTPRes* res_p, const char* label_p, int* pos_p );
extern int      NHTTPi_GetResponseHeaderField( NHTTPRes* res_p, const char* label_p, char** value_pp );
extern int      NHTTPi_GetResponseBodyAll    ( const NHTTPRes* res_p, char** value_pp );

#ifdef __cplusplus
}
#endif

#endif // __NHTTP_RESPONSE_H__
