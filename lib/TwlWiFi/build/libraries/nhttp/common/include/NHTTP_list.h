/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_list.h

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
#ifndef __NHTTP_LIST_H__
#define __NHTTP_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "d_nhttp_common.h"

/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern void             NHTTPi_InitListInfo     ( NHTTPListInfo* listInfo_p );
extern int              NHTTPi_setReqQueue      ( NHTTPListInfo* listInfo_p, NHTTPReq const* req_p);
extern NHTTPi_REQLIST*  NHTTPi_findReqQueue     ( NHTTPListInfo* listInfo_p, const int id );
extern int              NHTTPi_freeReqQueue     ( NHTTPListInfo* listInfo_p, NHTTPMutexInfo* mutexInfo_p, const int id );
extern void             NHTTPi_allFreeReqQueue  ( NHTTPListInfo* listInfo_p, NHTTPMutexInfo* mutexInfo_p );
extern NHTTPi_REQLIST*  NHTTPi_getReqFromReqQueue( NHTTPListInfo* listInfo_p );

#ifdef __cplusplus
}
#endif

#endif // __NHTTP_LIST_H__
