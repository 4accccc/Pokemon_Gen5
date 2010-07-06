/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - tests - wget-simple
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-08#$
  $Rev: 1183 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitroWiFi/nhttp.h>

#ifdef USE_NSSL
#include <twlWiFi/nssl.h>
#endif

#include "netconnect.h"

#define SITEDEFS_APCLASS    "WiFi.LAN.1.AP.1"

#define URL                 "https://nokonoko.nintendo.co.jp/1M.html"
//#define URL                   "https://www.ccc.go.jp/"
//#define URL                 "http://nokonoko.nintendo.co.jp/10M.bin"
//#define URL                 "http://192.168.102.65/10M.bin"
//#define URL                 "http://192.168.102.3/10M.bin"
//#define URL                 "http://ccs.cdn.shop.wii.com/ccs/download/0000000100000002/00000009"

#define DISPLAY_INTERVAL    1000

static void DoDownload(void);


/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    内部関数プロトタイプ宣言
 *---------------------------------------------------------------------------*/

static void DumpAllocatedMemory(void);
static void* AllocForNhttp(u32 size, int alignment);
static void FreeForNhttp(void* ptr);
static void* AllocForNssl(u32 size);
static void FreeForNssl(void* ptr);
static int ConnectionCallback( NHTTPConnectionHandle handle, NHTTPConnectionEvent event, void* arg );
static void DoDownload(void);


static void* AllocForNhttp(u32 size, int alignment)
{
#ifndef SDK_DEBUG
#pragma unused(alignment)
#endif
    void *ptr;
    OSIntrMode  enable = OS_DisableInterrupts();
    SDK_ASSERT(alignment <= 32);
    ptr = OS_Alloc(size);
    (void)OS_RestoreInterrupts(enable);
    return ptr;
}

static void FreeForNhttp(void* ptr)
{
    OSIntrMode  enable = OS_DisableInterrupts();
    OS_Free(ptr);
    (void)OS_RestoreInterrupts(enable);
}

static void* AllocForNssl(u32 size)
{
    void *ptr;
    OSIntrMode  enable = OS_DisableInterrupts();
    ptr = OS_Alloc(size);
    (void)OS_RestoreInterrupts(enable);
    return ptr;
}

static void FreeForNssl(void* ptr)
{
    OSIntrMode  enable = OS_DisableInterrupts();
    OS_Free(ptr);
    (void)OS_RestoreInterrupts(enable);
}

static int ConnectionCallback( NHTTPConnectionHandle handle, NHTTPConnectionEvent event, void* arg )
{
    (void)handle;
    switch ( event )
    {
    case NHTTP_EVENT_BODY_RECV_FULL:
    case NHTTP_EVENT_BODY_RECV_DONE:
        {
            NHTTPBodyBufArg*    bbArg = (NHTTPBodyBufArg*)arg;
            bbArg->offset = 0;
        }
    case NHTTP_EVENT_POST_SEND:
        {
            NHTTPPostSendArg*   psArg = (NHTTPPostSendArg*)arg;
        }
    }
    return NHTTP_ERROR_NONE;
}

static void DoDownload(void)
{
    int     result;
    static char recvBuffer[4096] ATTRIBUTE_ALIGN(32);
    
    result = NHTTPStartup(AllocForNhttp, FreeForNhttp, 12);
    
    {
        NHTTPConnectionHandle   handle;
        NHTTPError              err;
        OSTick                  tickStart, tickPrevious;
        u32                     receivedCurrent = 0, receivedPrevious = 0;
        u32                     contentLength;
        u32                     averageSpeed = 0, currentSpeed = 0, maxSpeed = 0;

        OS_TPrintf("=================================================================================\n"
                   " Target URL: %s\n", URL);
        OS_TPrintf("---------------------------------------------------------------------------------\n");
        handle = NHTTPCreateConnection(URL, NHTTP_REQMETHOD_GET,
                                       recvBuffer, sizeof(recvBuffer),
                                       ConnectionCallback, NULL);
#ifdef USE_NSSL
        NHTTPDisableVerifyOptionForDebug(handle, NSSL_VERIFY_COMMON_NAME|NSSL_VERIFY_SUBJECT_ALT_NAME|NSSL_VERIFY_ROOT_CA);
#endif
        err = NHTTPStartConnection(handle);
        
        
        tickStart = OS_GetTick();
        while((err = NHTTPGetConnectionError(handle)) == NHTTP_ERROR_BUSY)
        {
            NHTTPConnectionStatus   status = NHTTPGetConnectionStatus(handle);
            OSTick tickNow;

            OS_Sleep(DISPLAY_INTERVAL);
            tickNow = OS_GetTick();
            if (NHTTPGetConnectionProgress(handle, &receivedCurrent, &contentLength) == NHTTP_ERROR_NONE)
            {
                averageSpeed = (u32)((f32)receivedCurrent*8 / (f32)OS_TicksToMilliSeconds(tickNow - tickStart));
                currentSpeed = (u32)((f32)(receivedCurrent - receivedPrevious)*8 / (f32)OS_TicksToMilliSeconds(tickNow - tickPrevious));
                
                OS_TPrintf("% 10u / % 10u bytes received. (average % 4u kbps, current % 4u kbps)\n",
                           receivedCurrent, contentLength, averageSpeed, currentSpeed);
                
                tickPrevious = OS_GetTick();
                receivedPrevious = receivedCurrent;
                if (maxSpeed < currentSpeed)
                {
                    maxSpeed = currentSpeed;
                }
            }
        }
        OS_TPrintf("---------------------------------------------------------------------------------\n");
        if (err == NHTTP_ERROR_NONE)
        {
            averageSpeed = (u32)((f32)receivedCurrent*8 / (f32)OS_TicksToMilliSeconds(OS_GetTick() - tickStart));
            OS_TPrintf("Result: average % 4u kbps (%u sec), max % 4u kbps (%u msec)\n"
                       "\n" , averageSpeed, (u32)OS_TicksToSeconds(OS_GetTick() - tickStart), maxSpeed, DISPLAY_INTERVAL);
        }
        else
        {
            OS_TPrintf("err = %d\n", err);
        }
        NHTTPDeleteConnection(handle);
    }
    NHTTPCleanup();
}


void NitroMain(void)
{
#ifdef USE_NSSL
    NSSLConfig nsslConfig = { NSSL_ID_MAX, AllocForNssl, FreeForNssl };
#endif

    NcGlobalInit();
    NcSetDevice(WCM_DEVICEID_NWM);
    NcStart(SITEDEFS_APCLASS);
    
#ifdef USE_NSSL
    NSSL_Init(&nsslConfig);
#endif
    {
        DoDownload();
    }
#ifdef USE_NSSL
    NSSL_Finish();
#endif

    OS_TPrintf("Finished.\n");
    OS_Terminate();
}
