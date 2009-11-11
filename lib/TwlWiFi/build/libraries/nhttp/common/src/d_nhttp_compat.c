/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     d_nhttp_compat.h

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
#include "nhttp.h"

int         (NHTTPStartup)(NHTTPAlloc alloc, NHTTPFree free, u32 threadprio);
void        (NHTTPCleanupAsync)(NHTTPCleanupCallback callback);
void        (NHTTPCleanup)(void);
NHTTPError  (NHTTPGetError)(void);
int         (NHTTPSSLGetError)(void);
NHTTPRequestHandle 
            (NHTTPCreateRequest)(const char*      url_p,
                                 NHTTPReqMethod   method,
                                 char*            buf_p,
                                 u32              len,
                                 NHTTPReqCallback callback,
                                 void*            param_p);

NHTTPRequestHandle
            (NHTTPCreateRequestEx)(const char*      url_p,
                                   NHTTPReqMethod   method,
                                   char*            buf_p,
                                   u32              len,
                                   NHTTPReqCallback callback,
                                   void*            param_p,
                                   NHTTPBufFull     bufFull,
                                   NHTTPFreeBuf     freeBuf);

void        (NHTTPDeleteRequest)(NHTTPRequestHandle req);
int         (NHTTPAddHeaderField)(NHTTPConnectionHandle handle, const char* label_p, const char* value_p);
int         (NHTTPAddPostDataAscii)(NHTTPConnectionHandle handle, const char* label_p, const char* value_p);
int         (NHTTPAddPostDataBinary)(NHTTPConnectionHandle handle, const char* label_p, const void* value_p, u32 length);
int         (NHTTPAddPostDataRaw)(NHTTPConnectionHandle handle, const void* value_p, u32 length);
int         (NHTTPSetPostDataCallback)(NHTTPRequestHandle req, NHTTPPostSend callback);
int         (NHTTPSetPostDataEncoding)(NHTTPConnectionHandle handle, NHTTPEncodingType type);
int         (NHTTPSendRequestAsync)(NHTTPRequestHandle req);
int         (NHTTPCancelRequestAsync)(int id);
int         (NHTTPGetProgress)(u32* received_p, u32* contentlen_p);
void        (NHTTPDestroyResponse)(NHTTPResponseHandle res);
int         (NHTTPGetBodyAll)(NHTTPResponseHandle res, char** value_pp);
int         (NHTTPGetResultCode)(NHTTPConnectionHandle handle);
int         (NHTTPSetVerifyOption)(NHTTPConnectionHandle handle, u32 verifyOption);
int         (NHTTPSetProxy)(NHTTPConnectionHandle handle, const char* proxy_p, int port, const char* username_p, const char* password_p);
int         (NHTTPSetProxyDefault)(NHTTPConnectionHandle handle);
int         (NHTTPSetBasicAuthorization)(NHTTPConnectionHandle handle, const char* username_p, const char* password_p);
int         (NHTTPSetClientCert)(NHTTPConnectionHandle handle, const char* clientCertData_p, size_t clientCertSize, const char* privateKeyData_p, size_t privateKeySize);
int         (NHTTPRemoveClientCert)(NHTTPConnectionHandle handle);
int         (NHTTPSetRootCA)(NHTTPConnectionHandle handle, const char* rootCAData_p, size_t rootCASize);
int         (NHTTPDisableVerifyOptionForDebug)(NHTTPConnectionHandle handle, u32 verifyOption);
int         (NHTTPSetBuiltinRootCA)(NHTTPConnectionHandle handle, u32 rootCAId);
int         (NHTTPSetBuiltinClientCert)(NHTTPConnectionHandle handle, u32 clientCertId);
#ifdef NHTTP_USE_NSSL
int         (NHTTPSetRootCADefault)(NHTTPConnectionHandle handle);
int         (NHTTPSetClientCertDefault)(NHTTPConnectionHandle handle);
#endif
NHTTPConnectionHandle
            (NHTTPCreateConnection)(const char* url_p, NHTTPReqMethod method, char* buf_p, u32 len, NHTTPConnectionCallback callback, void* userParam_p);
NHTTPError  (NHTTPDeleteConnection)(NHTTPConnectionHandle handle);
NHTTPError  (NHTTPWaitForConnection)(NHTTPConnectionHandle handle);
NHTTPError  (NHTTPStartConnection)(NHTTPConnectionHandle handle);
NHTTPError  (NHTTPCancelConnection)(NHTTPConnectionHandle handle);
NHTTPConnectionStatus (NHTTPGetConnectionStatus)(NHTTPConnectionHandle handle);
int         (NHTTPGetBodyBuffer)(NHTTPConnectionHandle handle, char** value_pp, u32* size_p);
void*       (NHTTPGetUserParam)(NHTTPConnectionHandle handle);
NHTTPError  (NHTTPGetConnectionError)(NHTTPConnectionHandle handle);
int         (NHTTPGetConnectionSSLError)(NHTTPConnectionHandle handle);
NHTTPError  (NHTTPGetConnectionProgress)(NHTTPConnectionHandle handle, u32* received_p, u32* contentlen_p);
void*       (NHTTPCallAlloc)(u32 size, int align);
void        (NHTTPCallFree)(void* ptr_p);
NHTTPError  (NHTTPSetSocketBufferSize)(NHTTPConnectionHandle handle, u32 size);

//

int (NHTTPStartup)(NHTTPAlloc alloc, NHTTPFree free, u32 threadprio)
{
    return NHTTP_Startup( alloc, free, threadprio);
}

void (NHTTPCleanupAsync)(NHTTPCleanupCallback callback)
{
    NHTTP_CleanupAsync(callback);
}

void (NHTTPCleanup)(void)
{
    NHTTP_Cleanup();
}

NHTTPError (NHTTPGetError)(void)
{
    return NHTTP_GetError();
}

int (NHTTPSSLGetError)(void)
{
    return NHTTP_SSLGetError();
}

NHTTPRequestHandle (NHTTPCreateRequest)(const char*      url_p,
                                        NHTTPReqMethod   method,
                                        char*            buf_p,
                                        u32              len,
                                        NHTTPReqCallback callback,
                                        void*            param_p)
{
    return NHTTP_CreateRequest(url_p, method, buf_p, len, callback, param_p);
}

NHTTPRequestHandle (NHTTPCreateRequestEx)(const char*      url_p,
                                          NHTTPReqMethod   method,
                                          char*            buf_p,
                                          u32              len,
                                          NHTTPReqCallback callback,
                                          void*            param_p,
                                          NHTTPBufFull     bufFull,
                                          NHTTPFreeBuf     freeBuf)
{
    return NHTTP_CreateRequestEx(url_p, method, buf_p, len, callback, param_p, bufFull, freeBuf);
}

void (NHTTPDeleteRequest)(NHTTPRequestHandle req)
{
    NHTTP_DeleteRequest(req);
}

int (NHTTPAddHeaderField)(NHTTPConnectionHandle handle, const char* label_p, const char* value_p)
{
    return NHTTP_AddHeaderField(handle, label_p, value_p);
}

int (NHTTPAddPostDataAscii)(NHTTPConnectionHandle handle, const char* label_p, const char* value_p)
{
    return NHTTP_AddPostDataAscii(handle, label_p, value_p);
}

int (NHTTPAddPostDataBinary)(NHTTPConnectionHandle handle, const char* label_p, const void* value_p, u32 length)
{
    return NHTTP_AddPostDataBinary(handle, label_p, value_p, length);
}

int (NHTTPAddPostDataRaw)(NHTTPConnectionHandle handle, const void* value_p, u32 length)
{
    return NHTTP_AddPostDataRaw(handle, value_p, length);
}

int (NHTTPSetPostDataCallback)(NHTTPRequestHandle req, NHTTPPostSend callback)
{
    return NHTTP_SetRequestPostDataCallback(req, callback);
}

int (NHTTPSetPostDataEncoding)(NHTTPConnectionHandle handle, NHTTPEncodingType type)
{
    return NHTTP_SetPostDataEncoding(handle, type);
}

int (NHTTPSendRequestAsync)(NHTTPRequestHandle req)
{
    return NHTTP_SendRequestAsync(req);
}

int (NHTTPCancelRequestAsync)(int id)
{
    return NHTTP_CancelRequestAsync(id);
}

int (NHTTPGetProgress)(u32* received_p, u32* contentlen_p)
{
    return NHTTP_GetProgress(received_p, contentlen_p);
}

void (NHTTPDestroyResponse)(NHTTPResponseHandle res)
{
    NHTTP_DestroyResponse(res);
}

int (NHTTPGetBodyAll)(NHTTPResponseHandle res, char** value_pp)
{
    return NHTTP_GetBodyAll(res, value_pp);
}

int (NHTTPGetResultCode)(NHTTPConnectionHandle handle)
{
    return NHTTP_GetResultCode(handle);
}

int (NHTTPSetVerifyOption)(NHTTPConnectionHandle handle, u32 verifyOption)
{
    return NHTTP_SetVerifyOption(handle, verifyOption);
}

int (NHTTPSetProxy)(NHTTPConnectionHandle handle, const char* proxy_p, int port, const char* username_p, const char* password_p)
{
    return NHTTP_SetProxy(handle, proxy_p, port, username_p, password_p);
}

int (NHTTPSetProxyDefault)(NHTTPConnectionHandle handle)
{
    return NHTTP_SetProxyDefault(handle);
}

int (NHTTPSetBasicAuthorization)(NHTTPConnectionHandle handle, const char* username_p, const char* password_p)
{
    return NHTTP_SetBasicAuthorization(handle, username_p, password_p);
}

int (NHTTPSetClientCert)(NHTTPConnectionHandle handle, const char* clientCertData_p, size_t clientCertSize, const char* privateKeyData_p, size_t privateKeySize)
{
    return NHTTP_SetClientCert(handle, clientCertData_p, clientCertSize, privateKeyData_p, privateKeySize);
}

int (NHTTPRemoveClientCert)(NHTTPConnectionHandle handle)
{
    return NHTTP_RemoveClientCert(handle);
}

int (NHTTPSetRootCA)(NHTTPConnectionHandle handle, const char* rootCAData_p, size_t rootCASize)
{
    return NHTTP_SetRootCA(handle, rootCAData_p, rootCASize);
}

int (NHTTPDisableVerifyOptionForDebug)(NHTTPConnectionHandle handle, u32 verifyOption)
{
    return NHTTP_DisableVerifyOptionForDebug(handle, verifyOption);
}

int (NHTTPSetBuiltinRootCA)(NHTTPConnectionHandle handle, u32 rootCAId)
{
    return NHTTP_SetBuiltinRootCA(handle, rootCAId);
}

int (NHTTPSetBuiltinClientCert)(NHTTPConnectionHandle handle, u32 clientCertId)
{
    return NHTTP_SetBuiltinClientCert(handle, clientCertId);
}

#ifdef NHTTP_USE_NSSL
int (NHTTPSetRootCADefault)(NHTTPConnectionHandle handle)
{
    return NHTTP_SetRootCADefault(handle);
}

int (NHTTPSetClientCertDefault)(NHTTPConnectionHandle handle)
{
    return NHTTP_SetClientCertDefault(handle);
}
#endif

NHTTPConnectionHandle (NHTTPCreateConnection)(const char* url_p, NHTTPReqMethod method, char* buf_p, u32 len, NHTTPConnectionCallback callback, void* userParam_p)
{
    return NHTTP_CreateConnection(url_p, method, buf_p, len, callback, userParam_p);
}

NHTTPError (NHTTPDeleteConnection)(NHTTPConnectionHandle handle)
{
    return NHTTP_DeleteConnection(handle);
}

NHTTPError (NHTTPWaitForConnection)(NHTTPConnectionHandle handle)
{
    return NHTTP_WaitForConnection(handle);
}

NHTTPError (NHTTPStartConnection)(NHTTPConnectionHandle handle)
{
    return NHTTP_StartConnection(handle);
}

NHTTPError (NHTTPCancelConnection)(NHTTPConnectionHandle handle)
{
    return NHTTP_CancelConnection(handle);
}

NHTTPConnectionStatus (NHTTPGetConnectionStatus)(NHTTPConnectionHandle handle)
{
    return NHTTP_GetConnectionStatus(handle);
}

int (NHTTPGetBodyBuffer)(NHTTPConnectionHandle handle, char** value_pp, u32* size_p)
{
    return NHTTP_GetBodyBuffer(handle, value_pp, size_p);
}

void* (NHTTPGetUserParam)(NHTTPConnectionHandle handle)
{
    return NHTTP_GetUserParam(handle);
}

NHTTPError (NHTTPGetConnectionError)(NHTTPConnectionHandle handle)
{
    return NHTTP_GetConnectionError(handle);
}

int (NHTTPGetConnectionSSLError)(NHTTPConnectionHandle handle)
{
    return NHTTP_GetConnectionSSLError(handle);
}

NHTTPError (NHTTPGetConnectionProgress)(NHTTPConnectionHandle handle, u32* received_p, u32* contentlen_p)
{
    return NHTTP_GetConnectionProgress(handle, received_p, contentlen_p);
}

void* (NHTTPCallAlloc)(u32 size, int align)
{
    return NHTTP_CallAlloc(size, align);
}

void (NHTTPCallFree)(void* ptr_p)
{
    NHTTP_CallFree(ptr_p);
}
NHTTPError (NHTTPSetSocketBufferSize)(NHTTPConnectionHandle handle, u32 size)
{
    return NHTTP_SetSocketBufferSize(handle, size);
}
