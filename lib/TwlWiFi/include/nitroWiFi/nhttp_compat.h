/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - include
  File:     nhttp_compat.h

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
#ifndef __NHTTP_COMPAT_H__
#define __NHTTP_COMPAT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
#define NHTTPStartup(alloc, free, threadprio)   \
            NHTTP_Startup(alloc, free, threadprio)
#define NHTTPCleanupAsync(callback) \
            NHTTP_CleanupAsync(callback)
#define NHTTPCleanup()  \
            NHTTP_Cleanup()
#define NHTTPGetError() \
            NHTTP_GetError()
#define NHTTPAddHeaderField(handle, label, value)  \
            NHTTP_AddHeaderField(handle, label, value)
#define NHTTPAddPostDataAscii(handle, label, value)    \
            NHTTP_AddPostDataAscii(handle, label, value)
#define NHTTPAddPostDataBinary( handle, label, value, length ) \
            NHTTP_AddPostDataBinary( handle, label, value, length )
#define NHTTPAddPostDataRaw( handle, value, length )   \
            NHTTP_AddPostDataRaw( handle, value, length )
#define NHTTPSetPostDataEncoding( handle, type)    \
            NHTTP_SetPostDataEncoding( handle, type)
#define NHTTPSetProxy(handle, proxy_p, port, username_p, password_p)    \
            NHTTP_SetProxy(handle, proxy_p, port, username_p, password_p)
#define NHTTPSetProxyDefault(handle)   \
            NHTTP_SetProxyDefault(handle)
#define NHTTPSetBasicAuthorization(handle, user, password) \
            NHTTP_SetBasicAuthorization(handle, user, password)
#define NHTTPGetProgress(received, contentlen) \
            NHTTP_GetProgress(received, contentlen)
#define NHTTPGetHeaderField(handle, label, value)  \
            NHTTP_GetHeaderField(handle, label, value)
#define NHTTPGetHeaderAll(handle, value)   \
            NHTTP_GetHeaderAll(handle, value)
#define NHTTPGetResultCode(handle) \
            NHTTP_GetResultCode(handle)
#define NHTTPSetVerifyOption(handle, verifiOption) \
            NHTTP_SetVerifyOption(handle, verifiOption)
#define NHTTPSetClientCert(handle, clientCertData, clientCertSize, privateKeyData, privateKeySize) \
            NHTTP_SetClientCert(handle, clientCertData, clientCertSize, privateKeyData, privateKeySize)
#define NHTTPSetClientCertDefault(handle)  \
            NHTTP_SetClientCertDefault(handle)
#define NHTTPRemoveClientCert(handle)  \
            NHTTP_RemoveClientCert(handle)
#define NHTTPSetRootCA(handle, rootCAData, rootCASize) \
            NHTTP_SetRootCA(handle, rootCAData, rootCASize)
#define NHTTPSetRootCADefault(handle)  \
            NHTTP_SetRootCADefault(handle)
#define NHTTPSSLGetError() \
            NHTTP_SSLGetError()
#define NHTTPCreateConnection(url, method, bodyBuffer, bodyLength, callback, userParam)    \
            NHTTP_CreateConnection(url, method, bodyBuffer, bodyLength, callback, userParam)
#define NHTTPDeleteConnection(handle)  \
            NHTTP_DeleteConnection(handle)
#define NHTTPWaitForConnection(handle) \
            NHTTP_WaitForConnection(handle)
#define NHTTPStartConnection(handle)   \
            NHTTP_StartConnection(handle)
#define NHTTPCancelConnection(handle)  \
            NHTTP_CancelConnection(handle)
#define NHTTPGetConnectionStatus(handle)   \
            NHTTP_GetConnectionStatus(handle)
#define NHTTPGetBodyBuffer(handle, bodyBuffer, size)   \
            NHTTP_GetBodyBuffer(handle, bodyBuffer, size)
#define NHTTPGetUserParam(handle)  \
            NHTTP_GetUserParam(handle)
#define NHTTPGetConnectionError(handle)    \
            NHTTP_GetConnectionError(handle)
#define NHTTPGetConnectionSSLError(handle) \
            NHTTP_GetConnectionSSLError(handle)
#define NHTTPGetConnectionProgress(handle, received, contentlen)   \
            NHTTP_GetConnectionProgress(handle, received, contentlen)
#define NHTTPCallAlloc(size, align)    \
            NHTTP_CallAlloc(size, align)
#define NHTTPCallFree(ptr) \
            NHTTP_CallFree(ptr)
#define NHTTPSetSocketBufferSize(handle, size) \
            NHTTP_SetSocketBufferSize(handle, size)
#define NHTTPSetBuiltinRootCA(handle, rootCAId)    \
            NHTTP_SetBuiltinRootCA(handle, rootCAId)
#define NHTTPSetBuiltinClientCert(handle, clientCertId)   \
            NHTTP_SetBuiltinClientCert (handle, clientCertId)
#define NHTTPDisableVerifyOptionForDebug(handle, verifyOption) \
            NHTTP_DisableVerifyOptionForDebug(handle, verifyOption)

/*---------------------------------------------------------------------------*
 * Old Function Prototypes 
 *---------------------------------------------------------------------------*/
#define NHTTPCreateRequest(url, method, buf, len, callback, param) \
            NHTTP_CreateRequest(url, method, buf, len, callback, param)
#define NHTTPCreateRequestEx(url, method, buf, len, callback, param, bufFull, freeBuf) \
            NHTTP_CreateRequestEx(url, method, buf, len, callback, param, bufFull, freeBuf)
#define NHTTPDeleteRequest(req)    \
            NHTTP_DeleteRequest(req)
#define NHTTPSendRequestAsync(req) \
            NHTTP_SendRequestAsync(req)
#define NHTTPCancelRequestAsync(id)    \
            NHTTP_CancelRequestAsync(id)
#define NHTTPDestroyResponse(res)  \
            NHTTP_DestroyResponse(res)
#define NHTTPGetBodyAll(res, value)    \
            NHTTP_GetBodyAll(res, value)
#define NHTTPSetRequestPostDataCallback(req, callback) \
            NHTTP_SetRequestPostDataCallback(req, callback)
#define NHTTPAddRequestHeaderField(req, label, value)  \
            NHTTP_AddRequestHeaderField(req, label, value)
#define NHTTPAddRequestPostDataAscii(req, label, value)    \
            NHTTP_AddRequestPostDataAscii(req, label, value)
#define NHTTPAddRequestPostDataBinary(req, label, value, length)   \
            NHTTP_AddRequestPostDataBinary(req, label, value, length)
#define NHTTPAddRequestPostDataRaw(req, value, length) \
            NHTTP_AddRequestPostDataRaw(req, value, length)
#define NHTTPSetRequestPostDataEncoding (req, type)    \
            NHTTP_SetRequestPostDataEncoding (req, type)
#define NHTTPGetResponseHeaderField(res, label, value) \
            NHTTP_GetResponseHeaderField(res, label, value)
#define NHTTPGetResponseHeaderAll(res, value)  \
            NHTTP_GetResponseHeaderAll(res, value)

#ifdef __cplusplus
}
#endif

#endif  // __NHTTP_COMPAT_H__
