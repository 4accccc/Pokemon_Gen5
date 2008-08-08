/*---------------------------------------------------------------------------*
  Project:  NitroDWC Libraries
  File:     dwc_reliable.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/
#include <nitro.h>

// the original header
//---------------------------------------------------------
#include <base/dwc_report.h>
#include <base/dwc_error.h>
#include <base/dwc_memfunc.h>

#include <base/dwc_base_gamespy.h>
#include <dwci_reliable.h>

#define DWC_ASSERT          SDK_ASSERT
#define DWC_ASSERTMSG       SDK_ASSERTMSG
#define DWCi_Np_CpuCopy32   MI_CpuCopy8

/* -----------------------------------------------------------------
  内部変数
  -------------------------------------------------------------------*/
const int       RETRY_MAX_COUNT = 3;
const int       QUEUE_MAX_HEAP  = 1024 * 32;
const int       QUEUE_MAX_ACCEPT_BORDER  = QUEUE_MAX_HEAP - (QUEUE_MAX_HEAP >> 2);

typedef struct _tagDWCiSendQueue
{
    GT2Connection               connection;
    void*                       buffer;
    u32                         size;
    struct _tagDWCiSendQueue*   next;
}
DWCiSendQueue;

static struct
{
    DWCiSendQueue*  queue;
    int             usedheap;
}
s_queueinfo;


/*
 * キューを初期化します
 */
void DWCi_InitReliableQueue()
{
    s_queueinfo.queue = NULL;
    s_queueinfo.usedheap = 0;
}

/*
 * キューにたまったデータをクリアします
 */
void DWCi_CleanupReliableQueue()
{
    DWCiSendQueue*   it;

    for ( it = s_queueinfo.queue; it != NULL;  )
    {
        s_queueinfo.queue = it->next;
        DWC_Free( DWC_ALLOCTYPE_BASE, it->buffer, it->size );
        DWC_Free( DWC_ALLOCTYPE_BASE, it, 0 );
        it = s_queueinfo.queue;
    }

    s_queueinfo.queue = NULL;
    s_queueinfo.usedheap = 0;
}

/*
 * キューにデータを追加できる状態かどうかを返します
 */
BOOL DWCi_IsReliableQueueFree()
{

    if ( s_queueinfo.usedheap >= QUEUE_MAX_ACCEPT_BORDER )
    {
        return FALSE;
    }

    return TRUE;
}

/*
 * キューにたまったデータを処理します
 *
 * @param connection 通信に使用するgt2connection
 *
 * @retval TRUE     キューが空
 * @retval FALSE    キューにまだたまっている
 */
BOOL DWCi_ProcessReliableQueue()
{
    return DWCi_ProcessReliableQueueEx( gt2Send );
}

BOOL DWCi_ProcessReliableQueueEx( DWCiGT2SendFunc func )
{
    DWCiSendQueue*   it;

    // キューにたまっているものがあれば先に送信する
    for ( it = s_queueinfo.queue; it != NULL; it = it->next )
    {
        int retrycount;

        for ( retrycount = 0; retrycount < RETRY_MAX_COUNT; retrycount++ )
        {
            GT2Result res = func( it->connection, it->buffer, (int)it->size, TRUE );

            if ( res == GT2Success )
            {
                s_queueinfo.usedheap -= it->size;

                DWC_Free( DWC_ALLOCTYPE_BASE, it->buffer, it->size );
                it->buffer = NULL;
                it->size = 0;
                it->connection = NULL;

                break;
            }
        }

        if ( retrycount == RETRY_MAX_COUNT )
        {
            // リトライ回数が規定値に達したら今回の呼び出しでの送信を諦める
            DWC_Printf( DWC_REPORTFLAG_TRANSPORT, "DWCi_ProcessReliableQueue() retry count exceeded!\n" );
            break;
        }
    }

    // 送信済みのアイテムをキューから消す
    for ( it = s_queueinfo.queue; it != NULL;  )
    {
        if ( it->buffer == NULL )
        {
            s_queueinfo.queue = it->next;
            DWC_Free( DWC_ALLOCTYPE_BASE, it, 0 );
            it = s_queueinfo.queue;
        }
        else
        {
            break;
        }
    }

    return s_queueinfo.queue == NULL;
}


/*
 * キューにReliable送信するデータを追加します
 *
 * バッファの内容は内部にコピーされるのでこの関数を呼び出した後は
 * 解放しても構いません。
 *
 * @param buffer    バッファ
 * @param size      バッファのサイズ
 *
 * @retval  TRUE    追加できた
 * @retval  FALSE   メモリー不足により追加できなかった
 *                  デバッグビルドではアサートされる
 */
BOOL DWCi_InsertReliableQueue( GT2Connection connection, const u8* buffer, int size )
{
    DWCiSendQueue*   it;
    DWCiSendQueue**  target = &s_queueinfo.queue;
    
    // 使用量制限以上のメモリーを消費したら失敗
    if ( s_queueinfo.usedheap + size > QUEUE_MAX_HEAP )
    {
        DWC_ASSERTMSG( s_queueinfo.usedheap + size < QUEUE_MAX_HEAP, "DWCi_InsertReliableQueue() exceeds to heap limit.\n" );
        return FALSE;
    }

    // 末尾のアイテムを取得する
    for ( it = s_queueinfo.queue; it != NULL; it = it->next )
    {
        target = &it->next;
    }

    *target = DWC_Alloc( DWC_ALLOCTYPE_BASE, sizeof(DWCiSendQueue) );
    if ( (*target) == NULL )
    {
        // メモリーの確保に失敗した
        DWC_ASSERTMSG( (*target)->buffer != NULL, "DWCi_InsertReliableQueue() failed to allocate from heap.\n" );
        return FALSE;
    }

    (*target)->buffer = DWC_Alloc( DWC_ALLOCTYPE_BASE, (u32)size );
    if ( (*target)->buffer == NULL)
    {
        // メモリーの確保に失敗した
        DWC_Free( DWC_ALLOCTYPE_BASE, (*target), sizeof(DWCiSendQueue) );
        (*target) = NULL;
        DWC_ASSERTMSG( (*target)->buffer != NULL, "DWCi_InsertReliableQueue() failed to allocate from heap.\n" );
        return FALSE;
    }

    (*target)->connection = connection;
    (*target)->size = (u32)size;
    (*target)->next = NULL;
    DWCi_Np_CpuCopy32( buffer, (*target)->buffer, (u32)size );

    s_queueinfo.usedheap += size;
    
    return TRUE;
}
