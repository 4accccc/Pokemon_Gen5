/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_bgnend.h

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
#ifndef __NHTTP_BGNEND_H__
#define __NHTTP_BGNEND_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "d_nhttp_common.h"
    
/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern void             NHTTPi_InitBgnEndInfo   ( NHTTPBgnEndInfo* bgnEndInfo_p );
extern void*            NHTTPi_alloc            ( u32 size, int align );
extern void             NHTTPi_free             ( void* ptr_p );
extern BOOL             NHTTPi_IsOpened         ( NHTTPBgnEndInfo* bgnEndInfo_p );
extern void             NHTTPi_SetError         ( NHTTPBgnEndInfo* bgnEndInfo_p, NHTTPError error );
extern void             NHTTPi_SetSSLError      ( NHTTPBgnEndInfo* bgnEndInfo_p, int sslError );
extern int              NHTTPi_GetSSLError      ( NHTTPBgnEndInfo* bgnEndInfo_p );
extern BOOL             NHTTPi_Startup          ( NHTTPSysInfo* sysInfo_p, NHTTPAlloc alloc, NHTTPFree free, u32 threadprio );
extern void             NHTTPi_CleanupAsync     ( NHTTPSysInfo* sysInfo_p, NHTTPCleanupCallback callback );
extern NHTTPError       NHTTPi_GetError         ( NHTTPBgnEndInfo* bgnEndInfo_p );

#ifdef __cplusplus
}
#endif

#endif // __NHTTP_BGNEND_H__
