/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     d_nhttp_common.h

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

#ifndef __D_NHTTP_COMMON_H__
#define __D_NHTTP_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "d_def.h"
#include "nhttp.h"
#include "NHTTPlib_int.h"
#if	defined(SRDRVSYS_IOP)
#include "d_mutex_iop.h"
#include <revolution/utl/terminate.h>
#endif	// SRDRVSYS_IOP


/*---------------------------------------------------------------------------*
 * Types/Declarations
 *---------------------------------------------------------------------------*/
#define MSGARRAY	3
// Callback
typedef int (*NHTTPForDebugCallback)(NHTTPSSLId i_sslId, u32 i_verifyOption);
    
typedef struct _NHTTPMutexInfo {
#if defined(SRDRVSYS_IOP)
    dMuIOP_Mutex	mutex;
#else  // SRDRVSYS_IOP
    OSMutex		mutex;
#endif // SRDRVSYS_IOP
    int			isInitMutex;
} NHTTPMutexInfo;
typedef struct _NHTTPThreadInfo {
#if defined(SRDRVSYS_IOP)
    IOSMessageQueueId	commThreadMessageQueue;
    IOSMessage		commThreadMessageArray[MSGARRAY];
    IOSThreadId		commThread;
#else  // SRDRVSYS_IOP
    OSMessageQueue	commThreadMessageQueue;
    OSMessage		commThreadMessageArray[MSGARRAY];
    OSThread		commThread;
#endif // SRDRVSYS_IOP
    int			isCreateCommThreadMessageQueue;
    u16			padding[8];
    u16			commBuf[(LEN_COMMBUF/sizeof(u16))] ATTRIBUTE_ALIGN(32);
} NHTTPThreadInfo;
    
//
typedef struct _NHTTPBgnEndInfo {
#if defined(NHTTP_FOR_RVL)
    NCDIpConfig			ipConfig ATTRIBUTE_ALIGN(32);
#endif
    NHTTPAlloc			alloc;
    NHTTPFree			free;
    BOOL			isOpened;
    SOCKET			socket;
    volatile int		sslError;        // SSLのエラー
    volatile NHTTPError		error;
    volatile int		isThreadEnd;
    u32*			commThreadStack;
    NHTTPForDebugCallback       forDebugCallback;
    u32				defaultRootCAId;
} NHTTPBgnEndInfo;

//
typedef struct _NHTTPReqInfo {
    volatile NHTTPi_REQLIST*	reqCurrent;	// 現在実行中の要求
} NHTTPReqInfo;

//
typedef struct _NHTTPListInfo {
    volatile NHTTPi_REQLIST*	reqQueue;	// キューイングされているリクエストのリスト
    volatile int		idRequest;       // 次にキューイングされるリクエストに与えられるid (通算のリクエスト数)
} NHTTPListInfo;
    
//    
    
//for nhttp system
typedef struct _NHTTPSysInfo {
    NHTTPBgnEndInfo	bgnEndInfo ATTRIBUTE_ALIGN(32);
    NHTTPListInfo	listInfo;
    NHTTPReqInfo	reqInfo;
    NHTTPMutexInfo	mutexInfo;
    NHTTPThreadInfo	threadInfo;
    u16				padding[8];
} NHTTPSysInfo;

/* NHTTPCleanup() でも解放されない永続的なリソース群 */
typedef struct _NHTTPStaticResource
{
    BOOL        initialized;
    NHTTPMutex          mutex;
#if defined(NHTTP_FOR_RVL)
    OSCond              cond;
#elif  defined(NHTTP_FOR_NTR)
    OSThreadQueue       queue;
#else
    UTLSignal           signal;
#endif
} NHTTPStaticResource;

extern u32 NHTTPiDefaultRootCAId;
   
/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern int              NHTTPi_CommitConnectionList     ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p );
extern int              NHTTPi_OmitConnectionList       ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p );
extern NHTTPReq*        NHTTPi_Connection2Request       ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p );
extern NHTTPRes*        NHTTPi_Connection2Response      ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p );
extern NHTTPConnection* NHTTPi_Request2Connection       ( NHTTPMutexInfo* mutexInfo_p, NHTTPReq* req_p );
extern NHTTPConnection* NHTTPi_Response2Connection      ( NHTTPMutexInfo* mutexInfo_p, NHTTPRes* res_p );
extern NHTTPConnection* NHTTPi_GetConnection            ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnectionHandle connection_handle_p );
extern NHTTPReq*        NHTTPi_GetRequest               ( NHTTPMutexInfo* mutexInfo_p, void* handle_p );
extern NHTTPRes*        NHTTPi_GetResponse              ( NHTTPMutexInfo* mutexInfo_p, void* handle_p );
extern u32              NHTTPi_GetConnectionListLength  ( void );

extern int              NHTTPi_PostSendCallback         ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p, const char* label_p, int offset );
extern void             NHTTPi_BufferFullCallback       ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p );
extern void             NHTTPi_ReceivedCallback         ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p );
extern void             NHTTPi_CompleteCallback         ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p );
    
extern NHTTPError       NHTTPi_SetBodyBuffer            ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnectionHandle handle, void* buf_p, u32 length, u32 offset );
extern NHTTPError       NHTTPi_SetPostDataBuffer        ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnectionHandle handle, void* value_p, u32 length );

extern NHTTPSysInfo*    NHTTPi_GetSystemInfoP           ( void );
extern NHTTPBgnEndInfo* NHTTPi_GetBgnEndInfoP           ( NHTTPSysInfo* sysInfo_p );
extern NHTTPListInfo*   NHTTPi_GetListInfoP             ( NHTTPSysInfo* sysInfo_p );
extern NHTTPReqInfo*    NHTTPi_GetReqInfoP              ( NHTTPSysInfo* sysInfo_p );
extern NHTTPThreadInfo* NHTTPi_GetThreadInfoP           ( NHTTPSysInfo* sysInfo_p );
extern NHTTPMutexInfo*  NHTTPi_GetMutexInfoP            ( NHTTPSysInfo* sysInfo_p );
extern void             NHTTPi_SetVirtualContentLength  ( NHTTPConnection* connection_p, u32 contentLength );
extern u32              NHTTPi_GetVirtualContentLength  ( const NHTTPConnection* connection_p );

extern void             NHTTPi_initLockReqList          ( NHTTPMutexInfo* mutexInfo_p );
extern void             NHTTPi_exitLockReqList          ( void );
extern void             NHTTPi_lockReqList              ( NHTTPMutexInfo* mutexInfo_p );
extern void             NHTTPi_unlockReqList            ( NHTTPMutexInfo* mutexInfo_p );

extern void             NHTTPi_NotifyCompletion         ( NHTTPConnection *connection_p );
extern void             NHTTPi_WaitForCompletion        ( NHTTPConnection *connection_p );
extern void             NHTTPi_InitConnectionList       ( void );

extern int              NHTTPi_addRef                   ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p );
extern int              NHTTPi_decRef                   ( NHTTPMutexInfo* mutexInfo_p, NHTTPConnection* connection_p );


#ifdef __cplusplus
}
#endif

#endif // __D_NHTTP_COMMON_H__
