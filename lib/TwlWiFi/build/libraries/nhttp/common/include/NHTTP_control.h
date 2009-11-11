/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_control.h

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
#ifndef __NHTTP_CONTROL_H__
#define __NHTTP_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nhttp.h"
#include "NHTTP_bgnend.h"

/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern BOOL             NHTTPi_AddRequestHeaderField    ( NHTTPReq* req_p, NHTTPBgnEndInfo* bgnEndInfo_p, const char* label_p, const char* value_p );
extern BOOL             NHTTPi_AddRequestPostDataAscii  ( NHTTPReq* req_p, NHTTPBgnEndInfo* bgnEndInfo_p, const char* label_p, const char* value_p );
extern BOOL             NHTTPi_AddRequestPostDataBinary ( NHTTPReq* req_p, NHTTPBgnEndInfo* bgnEndInfo_p, const char* label_p, const char* value_p, u32 length );
extern NHTTPi_DATALIST* NHTTPi_getHdrFromList   ( NHTTPi_DATALIST** o_list_pp );

#ifdef __cplusplus
}
#endif

#endif // __NHTTP_CONTROL_H__
