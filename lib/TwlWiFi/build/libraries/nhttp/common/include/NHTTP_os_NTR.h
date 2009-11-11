/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_os_NTR.h

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
#ifndef __NHTTP_OS_NTR_H__
#define __NHTTP_OS_NTR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/os.h>
#include "d_nhttp_common.h"

/*---------------------------------------------------------------------------*
 * Definitions
 *---------------------------------------------------------------------------*/
#define	NHTTPi_ASSERT(exp)	SDK_ASSERT(exp)

/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern void NHTTPi_InitMutexInfo        ( NHTTPMutexInfo* mutexInfo_p );
extern void NHTTPi_InitMutex            ( NHTTPMutex* mutex_p );
extern void NHTTPi_LockMutex            ( NHTTPMutex* mutex_p );
extern void NHTTPi_UnlockMutex          ( NHTTPMutex* mutex_p );
extern BOOL NHTTPi_createCommThread     ( NHTTPThreadInfo* threadInfo_p, u32 priority, u32* stack_p );
extern void NHTTPi_destroyCommThread    ( NHTTPThreadInfo* threadInfo_p, NHTTPBgnEndInfo* bgnEndInfo_p );
extern void NHTTPi_idleCommThread       ( NHTTPThreadInfo* threadInfo_p );
extern void NHTTPi_kickCommThread       ( NHTTPThreadInfo* threadInfo_p );
extern void NHTTPi_CheckCurrentThread   ( NHTTPThreadInfo* threadInfo_p, NHTTPThreadIs is );
extern void NHTTPi_CommThreadProc       ( void* param );

#ifdef __cplusplus
}
#endif

#endif // __NHTTP_OS_NTR_H__
