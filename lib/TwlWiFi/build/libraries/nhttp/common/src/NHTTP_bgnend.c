/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_bgnend.c

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
#include "NHTTPlib_int.h"
#include "NHTTPlib_int_socket.h"
#include "NHTTP_bgnend.h"
#include "NHTTP_request.h"
#include "NHTTP_nonport.h"
#include "NHTTP_list.h"


/*-------------------------------------------------------------------------*

  Name:         NHTTPi_InitBgnEndInfo

  Description:  NHTTPBgnEndInfo構造体の初期設定
                 
  Arguments:    bgnEndInfo_p  NHTTPBgnEndInfo構造体へのポインタ

  Returns:      
 *-------------------------------------------------------------------------*/
extern void NHTTPi_InitBgnEndInfo(NHTTPBgnEndInfo* bgnEndInfo_p)
{
    bgnEndInfo_p->alloc = NULL;
    bgnEndInfo_p->free = NULL;
    bgnEndInfo_p->isOpened = FALSE;
    DEBUG_PRINTF("[%s][%d] INVALID_SOCKET \n", __func__, __LINE__);
    bgnEndInfo_p->socket = INVALID_SOCKET;
    NHTTPi_SetSSLError(bgnEndInfo_p, NHTTP_SSL_ENONE);
    NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_NONE);
    bgnEndInfo_p->isThreadEnd = FALSE;
    bgnEndInfo_p->commThreadStack = NULL;
    bgnEndInfo_p->forDebugCallback = NULL;
}

/*-------------------------------------------------------------------------*

  Name:         NHTTPi_alloc

  Description:	NHTTPライブラリ内部で使用されるメモリブロック確保関数
                 
  Arguments:	size	確保サイズ
  		align	領域のアライメント

  Returns:	void*	領域の確保に成功した場合先頭ポインタを返す
  			領域の確保に失敗した場合は NULLを返す
 *-------------------------------------------------------------------------*/
extern void* NHTTPi_alloc(u32 size, int align)
{
    NHTTPAlloc	alloc = NHTTPi_GetSystemInfoP()->bgnEndInfo.alloc;

    if (alloc != NULL)
    {
        return alloc(size, align);
    }
    else
    {
        return NULL;
    }
}

/*-------------------------------------------------------------------------*

  Name:         NHTTPi_free

  Description:	NHTTPライブラリ内部で使用されるメモリブロック解放関数
                 
  Arguments:	ptr_p	NHTTPi_allocで確保されたメモリブロックの先頭ポインタ

  Returns:
 *-------------------------------------------------------------------------*/
extern void NHTTPi_free(void* ptr_p)
{
    NHTTPFree	free = NHTTPi_GetSystemInfoP()->bgnEndInfo.free;

    if (free != NULL)
    {
        free(ptr_p);
    }
}

/*-------------------------------------------------------------------------*

  Name:         NHTTPi_IsOpened

  Description:	NHTTPスレッドが生成されているかチェックする関数
                 
  Arguments:	bgnEndInfo_p	NHTTPBgnEndInfo構造体へのポインタ

  Returns:	TRUE	生成済み
  		FALSE	未生成
 *-------------------------------------------------------------------------*/
extern BOOL NHTTPi_IsOpened(NHTTPBgnEndInfo* bgnEndInfo_p)
{
    return (bgnEndInfo_p->isOpened);
}

/*-------------------------------------------------------------------------*

  Name:         NHTTPi_SetError

  Description:	NHTTPライブラリ内で最後に発生したエラーセット関数
                 
  Arguments:	bgnEndInfo_p	NHTTPBgnEndInfo構造体へのポインタ

  Returns:	TRUE	生成済み
  		FALSE	未生成
 *-------------------------------------------------------------------------*/
extern void NHTTPi_SetError(NHTTPBgnEndInfo* bgnEndInfo_p, NHTTPError error)
{
    bgnEndInfo_p->error = error;
}

/*-------------------------------------------------------------------------*

  Name:         NHTTPi_SetSSLError

  Description:	NHTTPライブラリ内で最後に発生した SSLエラーセット関数
                 
  Arguments:	o_bgnEndInfo_p	NHTTPBgnEndInfo構造体へのポインタ

  Returns:	TRUE	生成済み
  		FALSE	未生成
 *-------------------------------------------------------------------------*/
extern void NHTTPi_SetSSLError(NHTTPBgnEndInfo* bgnEndInfo_p, int sslError)
{
    bgnEndInfo_p->sslError = sslError;
}

/*-------------------------------------------------------------------------*

  Name:         NHTTPi_GetSSLError

  Description:	NHTTPライブラリ内で最後に発生した SSLエラー取得関数
                 
  Arguments:	bgnEndInfo_p	NHTTPBgnEndInfo構造体へのポインタ

  Returns:	TRUE	生成済み
  		FALSE	未生成
 *-------------------------------------------------------------------------*/
extern int NHTTPi_GetSSLError(NHTTPBgnEndInfo* bgnEndInfo_p)
{
    return (bgnEndInfo_p->sslError);
}

/*-------------------------------------------------------------------------*

  Name:         NHTTPi_Startup

  Description:  NHTTPライブラリを初期化する
                 
  Arguments:	sysInfo_p	NHTTPシステム情報ポインタ
                alloc		NHTTPライブラリ内部で使用されるメモリブロック確保関数へのポインタ
                free		NHTTPライブラリ内部で使用されるメモリブロック解放関数へのポインタ
                threadprio	HTTPリクエストスレッドの優先度(0から31の範囲、0が最高優先度)

  Returns:      TRUE	処理に成功した
                FALSE	処理に失敗した
 *-------------------------------------------------------------------------*/
extern BOOL NHTTPi_Startup(NHTTPSysInfo* sysInfo_p, NHTTPAlloc alloc, NHTTPFree free, u32 threadprio)
{
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPListInfo*	listInfo_p = NHTTPi_GetListInfoP(sysInfo_p);
    NHTTPReqInfo*	reqInfo_p = NHTTPi_GetReqInfoP(sysInfo_p);
    NHTTPMutexInfo*	mutexInfo_p = NHTTPi_GetMutexInfoP(sysInfo_p);
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
        
    NHTTPi_ASSERT(!bgnEndInfo_p->isOpened);
    NHTTPi_ASSERT(alloc);
    NHTTPi_ASSERT(free);
    NHTTPi_ASSERT((threadprio >= 0) && (threadprio <= 31));
    
    // グローバル変数を初期化
    bgnEndInfo_p->alloc = alloc;
    bgnEndInfo_p->free = free;
    bgnEndInfo_p->error = NHTTP_ERROR_NONE;
    bgnEndInfo_p->sslError = NHTTP_SSL_ENONE;
    bgnEndInfo_p->isThreadEnd = FALSE;
    bgnEndInfo_p->defaultRootCAId = 0;

    NHTTPi_InitListInfo(listInfo_p);
    NHTTPi_InitRequestInfo(reqInfo_p);
    NHTTPi_initLockReqList(mutexInfo_p);

    // 単方向接続ハンドルリストの初期化
    NHTTPi_InitConnectionList();
    
    DEBUG_PRINTF("[%s][%d] INVALID_SOCKET \n", __func__, __LINE__);
    bgnEndInfo_p->socket = INVALID_SOCKET;
    
    // スレッド用にメモリを予約する
    bgnEndInfo_p->commThreadStack = (u32*)NHTTPi_alloc(NHTTP_COMMTHREAD_STACK, 8);
    if (!bgnEndInfo_p->commThreadStack)
    {
        DEBUG_PRINTF("[%s][line:%d] NHTTP_ERROR_ALLOC\n", __func__, __LINE__);
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_ALLOC);
        NHTTPi_exitLockReqList();
        return FALSE;
    }
    
    // スレッドを生成
    if (NHTTPi_createCommThread(threadInfo_p, threadprio, bgnEndInfo_p->commThreadStack) == FALSE)
    {
        NHTTPi_SetError(bgnEndInfo_p, NHTTP_ERROR_SDK);
        NHTTPi_free(bgnEndInfo_p->commThreadStack);
        bgnEndInfo_p->commThreadStack = NULL;
        NHTTPi_exitLockReqList();
        return FALSE;
    }

#if defined(NHTTP_FOR_RVL)
    {
        s32 err = NCDGetCurrentIpConfig(&(bgnEndInfo_p->ipConfig));
        if( err < 0 )
        {
            OSReport("NCDGetCurrentIpConfig err = %d\n", err);
            OSHalt("NCDGetCurrentIpConfig");
        }
    }
#endif

    bgnEndInfo_p->isOpened = TRUE;
    return TRUE;
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_CleanupAsync

  Description:  NHTTPライブラリの解放を要求する

  Arguments:	o_bgnEndInfo_p	NHTTPBgnEndInfo構造体へのポインタ
                i_callback	NHTTPライブラリの解放処理が完了した際に呼び出されるコールバック関数へのポインタ

  Returns:      なし
 *-------------------------------------------------------------------------*/
extern void NHTTPi_CleanupAsync(NHTTPSysInfo* sysInfo_p, NHTTPCleanupCallback callback)
{
    NHTTPBgnEndInfo*	bgnEndInfo_p = NHTTPi_GetBgnEndInfoP(sysInfo_p);
    NHTTPThreadInfo*	threadInfo_p = NHTTPi_GetThreadInfoP(sysInfo_p);
    
    DEBUG_PRINTF("[%s]\n", __func__);
    
    NHTTPi_ASSERT(bgnEndInfo_p->isOpened);

    NHTTPi_CheckCurrentThread(threadInfo_p, NHTTP_CURRENT_THREAD_IS_NOT_NHTTP_THREAD);
    
    // 現在実行中の要求をすべて中断し、メモリを解放する
    NHTTPi_cancelAllRequests(sysInfo_p);
    NHTTPi_destroyCommThread(threadInfo_p, bgnEndInfo_p);
    NHTTPi_free(bgnEndInfo_p->commThreadStack);
    bgnEndInfo_p->commThreadStack = NULL;
    NHTTPi_exitLockReqList();
    bgnEndInfo_p->isOpened = FALSE;
    if (callback)
    {
        callback();
    }
    
    {
        u32	connectionListLength = NHTTPi_GetConnectionListLength();

        if (connectionListLength > 0)
        {
            PRINTF("*warning: %d connections rests! Please free connections.\n", connectionListLength);
        }
    }

    // リクエストがひとつでKeepAlive時にソケットがcloseされなくなるバグ修正
    // KeepAliveでSSLはあり得ないのでNHTTPi_SocCloseでなくても良い
    if (IS_SOCKET(bgnEndInfo_p->socket))
    {
        NHTTPi_SocClose(NULL, NULL, bgnEndInfo_p->socket);
        DEBUG_PRINTF("[%s][%d] INVALID_SOCKET \n", __func__, __LINE__);
        
        bgnEndInfo_p->socket = INVALID_SOCKET;
    }
}

/*-------------------------------------------------------------------------*
  Name:         NHTTPi_GetError

  Description:  NHTTPライブラリ内で最後に発生したエラーの状況を取得する

  Arguments:	i_bgnEndInfo_p	NHTTPBgnEndInfo構造体へのポインタ

  Returns:      NHTTPErr	この関数を呼び出す直前にNHTTPライブラリ内で発生したエラーの状況
 *-------------------------------------------------------------------------*/
extern NHTTPError NHTTPi_GetError(NHTTPBgnEndInfo* bgnEndInfo_p)
{
    NHTTPi_ASSERT(bgnEndInfo_p->isOpened);
    return bgnEndInfo_p->error;
}
