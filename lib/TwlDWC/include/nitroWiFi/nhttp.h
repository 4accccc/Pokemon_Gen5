/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - include
  File:     nhttp.h

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
#ifndef __NHTTP_H__
#define __NHTTP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
 * Includes
 *---------------------------------------------------------------------------*/

#if defined(RVL_OS)
#define NHTTP_FOR_RVL
#elif defined(SDK_NITRO) || defined(SDK_TWL)
#define NHTTP_FOR_NTR
#else
#define NHTTP_FOR_RVL_MINI
#endif

#if defined(NHTTP_FOR_RVL)
#include <revolution.h>
#include <stdio.h>
#include <string.h>
#ifdef NHTTP_WITH_SSL_BSAFE
#include <ssl.h>
#endif // NHTTP_WITH_SSL_BSAFE

#elif defined(NHTTP_FOR_NTR)

#include <nitro.h>
#include <nitroWiFi.h>
#ifdef NHTTP_WITH_SSL_BSAFE
#include <twlWiFi/nssl.h>
#endif // NHTTP_WITH_SSL_BSAFE

#elif #defined(NHTTP_FOR_RVL_MINI)

#include <nhttp/NHTTPPrivate.h>
#ifdef NHTTP_WITH_SSL_BSAFE
#include <ssl.h>
#endif // NHTTP_WITH_SSL_BSAFE

#else

#error

#endif  // RVL_OS

/*---------------------------------------------------------------------------*
 * Definitions
 *---------------------------------------------------------------------------*/
#define NHTTP_COMMTHREAD_STACK          (8*1024)        // [byte]
#define NHTTP_HTTP_PORT                 80
#define NHTTP_HTTPS_PORT                443
#define NHTTP_STR_HTTPVER               "1.1"	
#define NHTTP_HDRRECVBUF_INILEN         1024            // [byte]
#define NHTTP_HDRRECVBUF_BLOCKSHIFT     9
#define NHTTP_HDRRECVBUF_BLOCKLEN       (1<<NHTTP_HDRRECVBUF_BLOCKSHIFT) // [byte]
#define NHTTP_HDRRECVBUF_BLOCKMASK      (NHTTP_HDRRECVBUF_BLOCKLEN-1)

/*---------------------------------------------------------------------------*
 * Types/Declarations
 *---------------------------------------------------------------------------*/
typedef enum {
    NHTTP_ERROR_SYSTEM = -1,    
     
    NHTTP_ERROR_NONE = 0,       // 0
    NHTTP_ERROR_ALLOC,          // 1
    NHTTP_ERROR_TOOMANYREQ,     // 2
    NHTTP_ERROR_SOCKET,         // 3
    NHTTP_ERROR_DNS,            // 4
    NHTTP_ERROR_CONNECT,        // 5
    NHTTP_ERROR_BUFFULL,        // 6
    NHTTP_ERROR_HTTPPARSE,      // 7
    NHTTP_ERROR_CANCELED,       // 8
    NHTTP_ERROR_SDK,            // 9
    NHTTP_ERROR_NETWORK,        // 10
    NHTTP_ERROR_UNKNOWN,        // 11
    NHTTP_ERROR_DNS_PROXY,      // 12
    NHTTP_ERROR_CONNECT_PROXY,  // 13
    NHTTP_ERROR_SSL,            // 14
    NHTTP_ERROR_BUSY,           // 15
    NHTTP_ERROR_SSL_ROOTCA,     // 16
    NHTTP_ERROR_SSL_CLIENTCERT, // 17

    NHTTP_ERROR_MAX,
    
    /* obsolete names */
    NHTTP_ERROR_REVOLUTIONSDK   = NHTTP_ERROR_SDK,
    NHTTP_ERROR_REVOLUTIONWIFI  = NHTTP_ERROR_NETWORK,
    NHTTP_ERROR_NITROSDK        = NHTTP_ERROR_SDK,
    NHTTP_ERROR_NITROWIFI       = NHTTP_ERROR_NETWORK
} NHTTPError;

typedef enum {
    NHTTP_REQMETHOD_GET,
    NHTTP_REQMETHOD_POST,
    NHTTP_REQMETHOD_HEAD,
    NHTTP_REQMETHOD_MAX
} NHTTPReqMethod;

typedef enum
{
    NHTTP_ENCODING_TYPE_AUTO,     /* default */
    NHTTP_ENCODING_TYPE_URL,      /* application/x-www-form-urlencoded */
    NHTTP_ENCODING_TYPE_MULTIPART,/* multipart/form-data */
    NHTTP_ENCODING_TYPE_MAX
} NHTTPEncodingType;

typedef enum {
    NHTTP_STATUS_ERROR = -1,
    NHTTP_STATUS_IDLE = 0,
    NHTTP_STATUS_CONNECTING,
    NHTTP_STATUS_SENDING,
    NHTTP_STATUS_RECEIVING_HEADER,
    NHTTP_STATUS_RECEIVING_BODY,
    NHTTP_STATUS_COMPLETE,
    
    NHTTP_STATUS_MAX
} NHTTPConnectionStatus;
    
typedef enum {
    NHTTP_EVENT_NONE,		//0
    NHTTP_EVENT_POST_SEND,	//1
    NHTTP_EVENT_BODY_RECV_FULL,	//2
    NHTTP_EVENT_BODY_RECV_DONE,	//3
    NHTTP_EVENT_COMPLETE,	//4
    
    NHTTP_CALLBACK_EVENT_MAX
} NHTTPConnectionEvent;
    
typedef struct _NHTTPRes  NHTTPRes;
typedef struct _NHTTPReq  NHTTPReq;
typedef NHTTPReq* NHTTPRequestHandle;
typedef NHTTPRes* NHTTPResponseHandle;
typedef void* NHTTPConnectionHandle;
    
//For event arg
typedef struct _NHTTPPostSendArg {
    const char*	label;
    char*	buf;
    u32		size;
    u32         offset;
} NHTTPPostSendArg;
typedef struct _NHTTPBodyBufArg {
    char*	buf;
    u32		size;
    u32		offset;
} NHTTPBodyBufArg;

typedef void* (*NHTTPAlloc)          ( u32 size, int align );
typedef void  (*NHTTPFree)           ( void* ptr );
typedef void  (*NHTTPCleanupCallback)( void );
typedef void  (*NHTTPReqCallback)    ( NHTTPError error, NHTTPResponseHandle res, void* param );
typedef char* (*NHTTPBufFull)        ( char** buf, u32* size, u32 contentLength, NHTTPAlloc alloc, NHTTPFree free, void* param );
typedef void  (*NHTTPFreeBuf)        ( char* buf, NHTTPFree free, void* param );
typedef int   (*NHTTPPostSend)       ( const char* label, char** value, u32* length, int offset, void* param);
typedef int   (*NHTTPConnectionCallback)( NHTTPConnectionHandle handle, NHTTPConnectionEvent event, void* arg );
    
/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern int                      NHTTP_Startup              ( NHTTPAlloc alloc, NHTTPFree free, u32 threadprio );
extern void                     NHTTP_CleanupAsync         ( NHTTPCleanupCallback callback );
extern void                     NHTTP_Cleanup              ( void );
extern NHTTPError               NHTTP_GetError             ( void );

extern int                      NHTTP_AddHeaderField       ( NHTTPConnectionHandle handle, const char* label, const char* value );
extern int                      NHTTP_AddPostDataAscii     ( NHTTPConnectionHandle handle, const char* label, const char* value );
extern int                      NHTTP_AddPostDataBinary    ( NHTTPConnectionHandle handle, const char* label, const void* value, u32 length );
extern int                      NHTTP_AddPostDataRaw       ( NHTTPConnectionHandle handle, const void* value, u32 length );

extern int                      NHTTP_SetPostDataEncoding  ( NHTTPConnectionHandle handle, NHTTPEncodingType type);
extern int                      NHTTP_SetProxy             ( NHTTPConnectionHandle handle, const char* proxy, int port, const char* user, const char* password);
extern int                      NHTTP_SetProxyDefault      ( NHTTPConnectionHandle handle );
extern int                      NHTTP_SetBasicAuthorization( NHTTPConnectionHandle handle, const char* user, const char* password );


extern int                      NHTTP_GetProgress          ( u32* received, u32* contentlen );
extern int                      NHTTP_GetHeaderField       ( NHTTPConnectionHandle handle, const char* label, char** value );
extern int                      NHTTP_GetHeaderAll         ( NHTTPConnectionHandle handle, char** value );

extern int                      NHTTP_GetResultCode        ( NHTTPConnectionHandle handle );   
extern int                      NHTTP_SetVerifyOption      ( NHTTPConnectionHandle handle, u32 verifiOption );
extern int                      NHTTP_SetClientCert        ( NHTTPConnectionHandle handle, const char* clientCertData, size_t clientCertSize, const char* privateKeyData, size_t privateKeySize );
extern int                      NHTTP_SetClientCertDefault ( NHTTPConnectionHandle handle);
extern int                      NHTTP_RemoveClientCert     ( NHTTPConnectionHandle handle);
extern int                      NHTTP_SetRootCA            ( NHTTPConnectionHandle handle, const char* rootCAData, size_t rootCASize );
extern int                      NHTTP_SetRootCADefault     ( NHTTPConnectionHandle handle );
extern int                      NHTTP_SSLGetError          ( void );

extern NHTTPConnectionHandle    NHTTP_CreateConnection     ( const char* url, NHTTPReqMethod method, char* bodyBuffer, u32 bodyLength, NHTTPConnectionCallback callback, void* userParam );
extern NHTTPError               NHTTP_DeleteConnection     ( NHTTPConnectionHandle handle );
extern NHTTPError               NHTTP_WaitForConnection    ( NHTTPConnectionHandle handle );
extern NHTTPError               NHTTP_StartConnection      ( NHTTPConnectionHandle handle );
extern NHTTPError               NHTTP_CancelConnection     ( NHTTPConnectionHandle handle );
extern NHTTPConnectionStatus    NHTTP_GetConnectionStatus  ( NHTTPConnectionHandle handle );
extern int                      NHTTP_GetBodyBuffer        ( NHTTPConnectionHandle handle, char** bodyBuffer, u32* size );
extern void*                    NHTTP_GetUserParam         ( NHTTPConnectionHandle handle );
extern NHTTPError               NHTTP_GetConnectionError   ( NHTTPConnectionHandle handle );
extern int                      NHTTP_GetConnectionSSLError( NHTTPConnectionHandle handle );
extern NHTTPError               NHTTP_GetConnectionProgress( NHTTPConnectionHandle handle, u32* received, u32* contentlen );
extern void*                    NHTTP_CallAlloc            ( u32 size, int align );
extern void                     NHTTP_CallFree             ( void* ptr );
extern NHTTPError               NHTTP_SetSocketBufferSize  ( NHTTPConnectionHandle handle, u32 size );

extern int                      NHTTP_SetBuiltinRootCA     ( NHTTPConnectionHandle handle, u32 rootCAId );
extern int                      NHTTP_SetBuiltinClientCert ( NHTTPConnectionHandle handle, u32 clientCertId );   
extern int                      NHTTP_DisableVerifyOptionForDebug( NHTTPConnectionHandle handle, u32 verifyOption );

extern int                      NHTTP_SetBuiltinRootCAAsDefault( u32 rootCAId );

/*---------------------------------------------------------------------------*
 * Old Function Prototypes 
 *---------------------------------------------------------------------------*/
extern NHTTPRequestHandle       NHTTP_CreateRequest        ( const char* url, NHTTPReqMethod method, char* buf, u32 len, NHTTPReqCallback callback, void* param );
extern NHTTPRequestHandle       NHTTP_CreateRequestEx      ( const char* url, NHTTPReqMethod method, char* buf, u32 len, NHTTPReqCallback callback, void* param, NHTTPBufFull bufFull, NHTTPFreeBuf freeBuf );
extern void                     NHTTP_DeleteRequest        ( NHTTPRequestHandle req );
extern int                      NHTTP_SendRequestAsync     ( NHTTPRequestHandle req );
extern int                      NHTTP_CancelRequestAsync   ( int id );
extern void                     NHTTP_DestroyResponse      ( NHTTPResponseHandle res );
extern int                      NHTTP_GetBodyAll           ( NHTTPResponseHandle res, char** value );
extern int                      NHTTP_SetRequestPostDataCallback ( NHTTPRequestHandle req, NHTTPPostSend callback);

extern int                      NHTTP_AddRequestHeaderField      ( NHTTPRequestHandle req, const char* label, const char* value );
extern int                      NHTTP_AddRequestPostDataAscii    ( NHTTPRequestHandle req, const char* label, const char* value );
extern int                      NHTTP_AddRequestPostDataBinary   ( NHTTPRequestHandle req, const char* label, const void* value, u32 length );
extern int                      NHTTP_AddRequestPostDataRaw      ( NHTTPRequestHandle req, const void* value, u32 length );
extern int                      NHTTP_SetRequestPostDataEncoding ( NHTTPRequestHandle req, NHTTPEncodingType type);

extern int                      NHTTP_GetResponseHeaderField     ( NHTTPResponseHandle res, const char* label, char** value );
extern int                      NHTTP_GetResponseHeaderAll       ( NHTTPResponseHandle res, char** value );

extern int                      NHTTP_SetRequestRootCA(NHTTPReq* req_p, const char* rootCAData_p, size_t rootCASize);

// old prefix support (temporary)
#include "nhttp_compat.h"

#ifdef __cplusplus
}
#endif

#endif  // __NHTTP_H__
