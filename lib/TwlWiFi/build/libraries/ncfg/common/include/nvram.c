/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - spi - nvram - DEMOlib
  File:     nvram.c

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

#include    "nvram.h"
#include    <nitro/hw/common/armArch.h>
#include    <nitro/pxi.h>
#include    <nitro/mi.h>
#include    <nitro/os.h>
#include    <nitro/spi/common/type.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
enum
{
    NVRAM_STATE_BEFORE_INITIALIZE   =   0,
    NVRAM_STATE_INITIALIZING,
    NVRAM_STATE_INITIALIZED

};

typedef enum NVRAMLock
{
    NVRAM_LOCK_OFF  =   0,
    NVRAM_LOCK_READ,
    NVRAM_LOCK_WRITE,
    NVRAM_LOCK_VERIFY,
    NVRAM_LOCK_MAX

} NVRAMLock;

#ifdef  SDK_DEBUG
#define     NVRAMi_Warning      OS_TWarning
//#define NVRAM_DEBUG
#else
#define     NVRAMi_Warning(...) ((void)0)
#endif

#ifdef  NVRAM_DEBUG
#define     NVRAMi_Printf       OS_TPrintf
#else
#define     NVRAMi_Printf(...)  ((void)0)
#endif

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
typedef struct NVRAMWork
{
    NVRAMLock       lock;
    NVRAMCallback   callback;
    void*           callbackArg;
    OSAlarm         alarm;
    OSTick          tick;
    u32             seq;
    u32             arg[3];

} NVRAMWork;

/*---------------------------------------------------------------------------*
    変数定義
 *---------------------------------------------------------------------------*/
static u32          nvramInitialized    =   NVRAM_STATE_BEFORE_INITIALIZE;
static NVRAMWork    nvramw;
static u32          nvramTemp[HW_CACHE_LINE_SIZE / sizeof(u32)] ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
static BOOL     NVRAMi_IsInitialized(void);
static void     NVRAMi_ReadCallback(u32 data, BOOL err);
static void     NVRAMi_WriteCallback(u32 data, BOOL err);
static void     NVRAMi_VerifyCallback(u32 data, BOOL err);
static void     NVRAMi_Callback(PXIFifoTag tag, u32 data, BOOL err);
static void     NVRAMi_SyncCallback(NVRAMResult result, void* arg);
static void     NVRAMi_AlarmHandler(void* arg);
static BOOL     NVRAMi_VerifyDataArray(const u8* data0, const u8* data1, u32 size);

static BOOL     NVRAMi_WriteEnable(void);
static BOOL     NVRAMi_WriteDisable(void);
static BOOL     NVRAMi_ReadStatusRegister(u8* pData);
static BOOL     NVRAMi_ReadDataBytes(u32 address, u32 size, u8* pData);
static BOOL     NVRAMi_PageWrite(u32 address, u16 size, const u8* pData);
static BOOL     NVRAMi_SoftwareReset(void);

/*---------------------------------------------------------------------------*
  Name:         NVRAM_Init
  Description:  NVRAM ライブラリを初期化する。
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NVRAM_Init(void)
{
    OSIntrMode  e   =   OS_DisableInterrupts();

    /* 初期化状態確認 */
    if (nvramInitialized != NVRAM_STATE_BEFORE_INITIALIZE)
    {
        (void)OS_RestoreInterrupts(e);
        return;
    }
    nvramInitialized    =   NVRAM_STATE_INITIALIZING;
    (void)OS_RestoreInterrupts(e);

    /* PXI 初期設定 */
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_NVRAM, PXI_PROC_ARM7))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_NVRAM, NVRAMi_Callback);

    /* ワーク用変数初期化 */
    MI_CpuFillFast(&nvramw, 0, sizeof(nvramw));

    /* アラーム初期化 */
    if (OS_IsTickAvailable() == FALSE)
    {
        OS_InitTick();
    }
    if (OS_IsAlarmAvailable() == FALSE)
    {
        OS_InitAlarm();
    }
    OS_CreateAlarm(&(nvramw.alarm));

    /* 初期化処理完了 */
    nvramInitialized    =   NVRAM_STATE_INITIALIZED;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAM_ReadAsync
  Description:  NVRAM からデータを読み出す。
                非同期関数なので、実際の処理結果はコールバック時に渡される。
  Arguments:    address -   NVRAM 上のオフセットを指定。
                size    -   読み出すサイズをバイト単位で指定。
                pData   -   読み出したデータを格納するバッファを指定。
                            値は ARM7 が直接書き出すのでキャッシュに注意。
                callback -  非同期処理完了時に呼び出すコールバック関数を指定。
                arg     -   コールバック関数に渡すパラメータを指定。
  Returns:      NVRAMResult -   非同期処理開始処理の結果を返す。
 *---------------------------------------------------------------------------*/
NVRAMResult
NVRAM_ReadAsync(u32 address, u32 size, void* pData, NVRAMCallback callback, void* arg)
{
    OSIntrMode  e;

    SDK_NULL_ASSERT(pData);
    SDK_NULL_ASSERT(callback);

    /* ライブラリ初期化確認 */
    if (NVRAMi_IsInitialized() == FALSE)
    {
#ifdef  SDK_DEBUG
        NVRAMi_Warning("%s: NVRAM library is not initialized.\n", __FUNCTION__);
#endif
        return NVRAM_RESULT_ILLEGAL_STATE;
    }

#ifdef  SDK_DEBUG
    /* パラメータ確認 */
    if ((address >= SPI_NVRAM_ALL_SIZE) || ((address + size) > SPI_NVRAM_ALL_SIZE))
    {
        NVRAMi_Warning("%s: Source range (%u - %u) is over max size of NVRAM (%u).\n", __FUNCTION__, address, address + size, SPI_NVRAM_ALL_SIZE);
    }
    if ((((u32)pData % HW_CACHE_LINE_SIZE) != 0) || ((size % HW_CACHE_LINE_SIZE) != 0))
    {
        NVRAMi_Warning("%s: Destination buffer (%p - %p) is not aligned on %d bytes boundary.\n", __FUNCTION__, pData, (void*)((u32)pData + size), HW_CACHE_LINE_SIZE);
    }
#endif

    /* 排他状態確認 */
    e   =   OS_DisableInterrupts();
    if (nvramw.lock != NVRAM_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(e);
        return NVRAM_RESULT_BUSY;
    }
    nvramw.lock =   NVRAM_LOCK_READ;
    (void)OS_RestoreInterrupts(e);

    /* データ読み出しコマンド送信 */
    nvramw.callback     =   callback;
    nvramw.callbackArg  =   arg;
    nvramw.seq  =   0;
    e   =   OS_DisableInterrupts();
    if (TRUE == NVRAMi_ReadDataBytes(address, size, pData))
    {
        NVRAMi_Printf("[NVRAM] ReadDataBytes(%u, %u, %p) = TRUE\n", address, size, pData);
        (void)OS_RestoreInterrupts(e);
        return NVRAM_RESULT_SUCCESS;
    }
    else
    {
        NVRAMi_Printf("[NVRAM] ReadDataBytes(%u, %u, %p) = FALSE\n", address, size, pData);
        nvramw.lock =   NVRAM_LOCK_OFF;
        (void)OS_RestoreInterrupts(e);
        return NVRAM_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_ReadCallback
  Description:  データ読み出しシーケンス中の PXI 割り込みコールバック関数。
  Arguments:    data    -   メッセージデータ。
                err     -   PXI 転送エラーフラグ
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
NVRAMi_ReadCallback(u32 data, BOOL err)
{
    NVRAMResult result      =   NVRAM_RESULT_FATAL_ERROR;
    u16         pxiResult   =   (u16)((data & SPI_PXI_RESULT_DATA_MASK) >> SPI_PXI_RESULT_DATA_SHIFT);

    NVRAMi_Printf("[NVRAM] Callback(%u, %s)\n", pxiResult, ((err == TRUE) ? "TRUE" : "FALSE"));
    /* PXI 通信エラーを確認 */
    if ((err == FALSE) && (pxiResult == SPI_PXI_RESULT_SUCCESS))
    {
        result  =   NVRAM_RESULT_SUCCESS;
    }
    else
    {
        if (err == TRUE)
        {
            result  =   NVRAM_RESULT_SEND_ERROR;
        }
        else
        {
            switch (pxiResult)
            {
            case SPI_PXI_RESULT_INVALID_PARAMETER:
                result  =   NVRAM_RESULT_ILLEGAL_PARAMETER;
                break;
            case SPI_PXI_RESULT_ILLEGAL_STATUS:
                result  =   NVRAM_RESULT_ILLEGAL_STATE;
                break;
            case SPI_PXI_RESULT_EXCLUSIVE:
                result  =   NVRAM_RESULT_BUSY;
                break;
            }
        }
    }

    /* 排他状態をリセット */
    nvramw.lock =   NVRAM_LOCK_OFF;
    nvramw.seq  =   0;

    /* コールバック関数呼び出し */
    if (nvramw.callback != NULL)
    {
        NVRAMCallback   cb  =   nvramw.callback;
    
        nvramw.callback =   NULL;
        cb(result, nvramw.callbackArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NVRAM_Read
  Description:  NVRAM からデータを読み出す。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
  Arguments:    address -   NVRAM 上のオフセットを指定。
                size    -   読み出すサイズをバイト単位で指定。
                pData   -   読み出したデータを格納するバッファを指定。
                            値は ARM7 が直接書き出すのでキャッシュに注意。
  Returns:      NVRAMResult -   同期的な処理結果を返す。
 *---------------------------------------------------------------------------*/
NVRAMResult
NVRAM_Read(u32 address, u32 size, void* pData)
{
    NVRAMResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 割り込みハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
#ifdef  SDK_DEBUG
        NVRAMi_Warning("%s: Could not process in exception handler.\n", __FUNCTION__);
#endif
        return NVRAM_RESULT_ILLEGAL_STATE;
    }

    /* 応答取得用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   NVRAM_ReadAsync(address, size, pData, NVRAMi_SyncCallback, (void*)(&msgQ));
    if (result == NVRAM_RESULT_SUCCESS)
    {
        if (FALSE == OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK))
        {
            OS_Panic("%s: Failed to receive message.\n", __FUNCTION__);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAM_WriteAsync
  Description:  NVRAM にデータを書き込む。
                非同期関数なので、実際の処理結果はコールバック時に渡される。
  Arguments:    address -   NVRAM 上のオフセットを指定。
                size    -   書き込むサイズをバイト単位で指定。
                pData   -   書き込むデータが格納されているバッファを指定。
                            値は ARM7 が直接読み出すのでキャッシュに注意。
                callback -  非同期処理完了時に呼び出すコールバック関数を指定。
                arg     -   コールバック関数に渡すパラメータを指定。
  Returns:      NVRAMResult -   非同期処理開始処理の結果を返す。
 *---------------------------------------------------------------------------*/
NVRAMResult
NVRAM_WriteAsync(u32 address, u32 size, const void* pData, NVRAMCallback callback, void* arg)
{
    OSIntrMode  e;

    SDK_NULL_ASSERT(pData);
    SDK_NULL_ASSERT(callback);

    /* ライブラリ初期化確認 */
    if (NVRAMi_IsInitialized() == FALSE)
    {
#ifdef  SDK_DEBUG
        NVRAMi_Warning("%s: NVRAM library is not initialized.\n", __FUNCTION__);
#endif
        return NVRAM_RESULT_ILLEGAL_STATE;
    }

#ifdef  SDK_DEBUG
    /* パラメータ確認 */
    if ((address >= SPI_NVRAM_ALL_SIZE) || ((address + size) > SPI_NVRAM_ALL_SIZE))
    {
        NVRAMi_Warning("%s: Destination range (%u - %u) is over max size of NVRAM (%u).\n", __FUNCTION__, address, address + size, SPI_NVRAM_ALL_SIZE);
    }
#if 0
    // DC_StoreRange さえしっかり行われていれば、キャッシュラインに沿っている必要はない
    if ((((u32)pData % HW_CACHE_LINE_SIZE) != 0) || ((size % HW_CACHE_LINE_SIZE) != 0))
    {
        NVRAMi_Warning("%s: Source buffer (%p - %p) is not aligned on %d bytes boundary.\n", __FUNCTION__, pData, (void*)((u32)pData + size), HW_CACHE_LINE_SIZE)
    }
#endif
#endif

    /* 排他状態確認 */
    e   =   OS_DisableInterrupts();
    if (nvramw.lock != NVRAM_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(e);
        return NVRAM_RESULT_BUSY;
    }
    nvramw.lock =   NVRAM_LOCK_WRITE;
    (void)OS_RestoreInterrupts(e);

    /* ステータスレジスタ読み出しコマンド発行 */
    nvramw.callback     =   callback;
    nvramw.callbackArg  =   arg;
    nvramw.arg[0]   =   address;
    nvramw.arg[1]   =   size;
    nvramw.arg[2]   =   (u32)pData;
    nvramw.seq  =   0;
    e   =   OS_DisableInterrupts();
    if (TRUE == NVRAMi_WriteEnable())
    {
        NVRAMi_Printf("[NVRAM] WriteEnable() = TRUE\n");
        (void)OS_RestoreInterrupts(e);
        return NVRAM_RESULT_SUCCESS;
    }
    else
    {
        NVRAMi_Printf("[NVRAM] WriteEnable() = FALSE\n");
        nvramw.lock =   NVRAM_LOCK_OFF;
        (void)OS_RestoreInterrupts(e);
        return NVRAM_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_WriteCallback
  Description:  データ書き込みシーケンス中の PXI 割り込みコールバック関数。
  Arguments:    data    -   メッセージデータ。
                err     -   PXI 転送エラーフラグ
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
NVRAMi_WriteCallback(u32 data, BOOL err)
{
    NVRAMResult     result      =   NVRAM_RESULT_FATAL_ERROR;
    u16             pxiResult   =   (u16)((data & SPI_PXI_RESULT_DATA_MASK) >> SPI_PXI_RESULT_DATA_SHIFT);
    u16             tempSize    =   (u16)(SPI_NVRAM_PAGE_SIZE - (nvramw.arg[0] % SPI_NVRAM_PAGE_SIZE));

    tempSize    =   (u16)((tempSize > nvramw.arg[1]) ? nvramw.arg[1] : tempSize);

    NVRAMi_Printf("[NVRAM] Callback(%u, %s)\n", pxiResult, ((err == TRUE) ? "TRUE" : "FALSE"));
    switch (nvramw.seq)
    {
    case 0: /* ライトエネーブル結果 */
        if ((err == FALSE) && (pxiResult == SPI_PXI_RESULT_SUCCESS))
        {
            /* ページ書き込みコマンドを発行 */
            if (TRUE == NVRAMi_PageWrite(nvramw.arg[0], tempSize, (const u8*)nvramw.arg[2]))
            {
                NVRAMi_Printf("[NVRAM] PageWrite(%u, %u, %p) = TRUE\n", nvramw.arg[0], tempSize, nvramw.arg[2]);
                nvramw.seq  =   1;
                return;
            }
            NVRAMi_Printf("[NVRAM] PageWrite(%u, %u, %p) = FALSE\n", nvramw.arg[0], tempSize, nvramw.arg[2]);
            result  =   NVRAM_RESULT_SEND_ERROR;
        }
        else
        {
            if (err == TRUE)
            {
                result  =   NVRAM_RESULT_SEND_ERROR;
            }
            else if (pxiResult == SPI_PXI_RESULT_EXCLUSIVE)
            {
                result  =   NVRAM_RESULT_BUSY;
            }
        }
        break;
    
    case 1: /* ライト結果 */
        if ((err == FALSE) && (pxiResult == SPI_PXI_RESULT_SUCCESS))
        {
            /* 書き込みに成功した分ポインタを進める */
            nvramw.arg[0]   +=  tempSize;
            nvramw.arg[1]   -=  tempSize;
            nvramw.arg[2]   +=  tempSize;
            /* ライト完了待ちタイムアウト用に時間計測開始 */
            nvramw.tick     =   OS_GetTick();
            /* ステータスレジスタ読み出し */
            DC_InvalidateRange(nvramTemp, HW_CACHE_LINE_SIZE);
            if (TRUE == NVRAMi_ReadStatusRegister((u8*)nvramTemp))
            {
                NVRAMi_Printf("[NVRAM] ReadStatusRegister(%p) = TRUE\n", nvramTemp);
                nvramw.seq  =   2;
                return;
            }
            NVRAMi_Printf("[NVRAM] ReadStatusRegister(%p) = FALSE\n", nvramTemp);
            /* ステータスレジスタ読み出し結果コールバックを偽装するアラームを起動 */
            nvramw.seq  =   2;
            *((u8*)nvramTemp)   =   NVRAM_STATUS_REGISTER_WIP;
            DC_StoreRange(nvramTemp, HW_CACHE_LINE_SIZE);
            OS_SetAlarm(&(nvramw.alarm), NVRAM_RETRY_SPAN, NVRAMi_AlarmHandler, (void*)(SPI_PXI_RESULT_SUCCESS << SPI_PXI_RESULT_DATA_SHIFT));
            return;
        }
        else
        {
            if (err == TRUE)
            {
                result  =   NVRAM_RESULT_SEND_ERROR;
            }
            else
            {
                switch(pxiResult)
                {
                case SPI_PXI_RESULT_INVALID_PARAMETER:
                    result  =   NVRAM_RESULT_ILLEGAL_PARAMETER;
                    break;
                case SPI_PXI_RESULT_ILLEGAL_STATUS:
                    result  =   NVRAM_RESULT_ILLEGAL_STATE;
                    break;
                case SPI_PXI_RESULT_EXCLUSIVE:
                    /* ページ書き込みコマンドを再発行 */
                    if (TRUE == NVRAMi_PageWrite(nvramw.arg[0], tempSize, (const u8*)nvramw.arg[2]))
                    {
                        NVRAMi_Printf("[NVRAM] PageWrite(%u, %u, %p) = TRUE\n", nvramw.arg[0], tempSize, nvramw.arg[2]);
                        return;
                    }
                    NVRAMi_Printf("[NVRAM] PageWrite(%u, %u, %p) = FALSE\n", nvramw.arg[0], tempSize, nvramw.arg[2]);
                    result  =   NVRAM_RESULT_SEND_ERROR;
                    break;
                }
            }
        }
        break;
    
    case 2: /* ステータスレジスタ読み出し結果 */
        if ((err == FALSE) && (pxiResult == SPI_PXI_RESULT_SUCCESS))
        {
            if ((*((u8*)nvramTemp) & NVRAM_STATUS_REGISTER_ERSER) != 0)
            {
                /* ソフトリセットコマンド発行 */
                if (TRUE == NVRAMi_SoftwareReset())
                {
                    NVRAMi_Printf("[NVRAM] SoftwareReset() = TRUE\n");
                    nvramw.seq  =   4;
                    return;
                }
                NVRAMi_Printf("[NVRAM] SoftwareReset() = FALSE\n");
                /* ソフトリセット結果コールバックを偽装するアラーム起動 */
                nvramw.seq  =   4;
                OS_SetAlarm(&(nvramw.alarm), NVRAM_RETRY_SPAN, NVRAMi_AlarmHandler, (void*)(SPI_PXI_RESULT_EXCLUSIVE << SPI_PXI_RESULT_DATA_SHIFT));
                return;
            }
            else if ((*((u8*)nvramTemp) & NVRAM_STATUS_REGISTER_WIP) == 0)
            {
                if (tempSize > 0)
                {
                    /* 次のページ書き込みシーケンスに移行 */
                    if (TRUE == NVRAMi_WriteEnable())
                    {
                        NVRAMi_Printf("[NVRAM] WriteEnable() = TRUE\n");
                        nvramw.seq  =   0;
                        return;
                    }
                    NVRAMi_Printf("[NVRAM] WriteEnable() = FALSE\n");
                    result  =   NVRAM_RESULT_SEND_ERROR;
                    break;
                }
                /* ライトディセーブルコマンド発行 */
                if (TRUE == NVRAMi_WriteDisable())
                {
                    NVRAMi_Printf("[NVRAM] WriteDisable() = TRUE\n");
                    nvramw.seq  =   3;
                    return;
                }
                NVRAMi_Printf("[NVRAM] WriteDisable() = FALSE\n");
                /* ライトディセーブル結果コールバックを偽装するアラームを起動 */
                nvramw.seq  =   3;
                OS_SetAlarm(&(nvramw.alarm), NVRAM_RETRY_SPAN, NVRAMi_AlarmHandler, (void*)(SPI_PXI_RESULT_EXCLUSIVE << SPI_PXI_RESULT_DATA_SHIFT));
                return;
            }
        }
        else
        {
            if ((err == FALSE) && (pxiResult != SPI_PXI_RESULT_EXCLUSIVE))
            {
                break;
            }
        }
        /* ステータスレジスタ読み出しコマンドを再発行 */
        DC_InvalidateRange(nvramTemp, HW_CACHE_LINE_SIZE);
        if (TRUE == NVRAMi_ReadStatusRegister((u8*)nvramTemp))
        {
            NVRAMi_Printf("[NVRAM] ReadStatusRegister(%p) = TRUE\n", nvramTemp);
            return;
        }
        NVRAMi_Printf("[NVRAM] ReadStatusRegister(%p) = FALSE\n", nvramTemp);
        /* ステータスレジスタ読み出し結果コールバックを偽装するアラームを起動 */
        nvramw.seq  =   2;
        *((u8*)nvramTemp)   =   NVRAM_STATUS_REGISTER_WIP;
        DC_StoreRange(nvramTemp, HW_CACHE_LINE_SIZE);
        OS_SetAlarm(&(nvramw.alarm), NVRAM_RETRY_SPAN, NVRAMi_AlarmHandler, (void*)(SPI_PXI_RESULT_SUCCESS << SPI_PXI_RESULT_DATA_SHIFT));
        return;
    
    case 3: /* ライトディセーブル結果 */
        if ((err == FALSE) && (pxiResult == SPI_PXI_RESULT_SUCCESS))
        {
            result  =   NVRAM_RESULT_SUCCESS;
        }
        else
        {
            if ((err == FALSE) && (pxiResult != SPI_PXI_RESULT_EXCLUSIVE))
            {
                break;
            }
            /* ライトディセーブルコマンド再発行 */
            if (TRUE == NVRAMi_WriteDisable())
            {
                NVRAMi_Printf("[NVRAM] WriteDisable() = TRUE\n");
                return;
            }
            NVRAMi_Printf("[NVRAM] WriteDisable() = FALSE\n");
            /* ライトディセーブル結果コールバックを偽装するアラーム起動 */
            nvramw.seq  =   3;
            OS_SetAlarm(&(nvramw.alarm), NVRAM_RETRY_SPAN, NVRAMi_AlarmHandler, (void*)(SPI_PXI_RESULT_EXCLUSIVE << SPI_PXI_RESULT_DATA_SHIFT));
            return;
        }
        break;
    
    case 4: /* ソフトリセット結果 */
        if ((err == FALSE) && (pxiResult == SPI_PXI_RESULT_SUCCESS))
        {
            result  =   NVRAM_RESULT_WRITE_ERROR;
        }
        else
        {
            if ((err == FALSE) && (pxiResult != SPI_PXI_RESULT_EXCLUSIVE))
            {
                break;
            }
            /* ソフトリセットコマンドを再発行 */
            if (TRUE == NVRAMi_SoftwareReset())
            {
                NVRAMi_Printf("[NVRAM] SoftwareReset() = TRUE\n");
                return;
            }
            NVRAMi_Printf("[NVRAM] SoftwareReset() = FALSE\n");
            /* ソフトリセット結果コールバックを偽装するアラーム起動 */
            nvramw.seq  =   4;
            OS_SetAlarm(&(nvramw.alarm), NVRAM_RETRY_SPAN, NVRAMi_AlarmHandler, (void*)(SPI_PXI_RESULT_EXCLUSIVE << SPI_PXI_RESULT_DATA_SHIFT));
            return;
        }
        break;
    }

    /* 処理結果をコールバック */
    nvramw.lock =   NVRAM_LOCK_OFF;
    nvramw.seq  =   0;
    if (nvramw.callback != NULL)
    {
        NVRAMCallback   cb  =   nvramw.callback;
    
        nvramw.callback =   NULL;
        cb(result, nvramw.callbackArg);
    }
}


/*---------------------------------------------------------------------------*
  Name:         NVRAM_Write
  Description:  NVRAM にデータを書き込む。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
  Arguments:    address -   NVRAM 上のオフセットを指定。
                size    -   書き込むサイズをバイト単位で指定。
                pData   -   書き込むデータが格納されているバッファを指定。
                            値は ARM7 が直接読み出すのでキャッシュに注意。
  Returns:      NVRAMResult -   同期的な処理結果を返す。
 *---------------------------------------------------------------------------*/
NVRAMResult
NVRAM_Write(u32 address, u32 size, const void* pData)
{
    NVRAMResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 割り込みハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
#ifdef  SDK_DEBUG
        NVRAMi_Warning("%s: Could not process in exception handler.\n", __FUNCTION__);
#endif
        return NVRAM_RESULT_ILLEGAL_STATE;
    }

    /* 応答取得用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   NVRAM_WriteAsync(address, size, pData, NVRAMi_SyncCallback, (void*)(&msgQ));
    if (result == NVRAM_RESULT_SUCCESS)
    {
        if (FALSE == OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK))
        {
            OS_Panic("%s: Failed to receive message.\n", __FUNCTION__);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAM_VerifyAsync
  Description:  NVRAM のデータを一致確認する。
                非同期関数なので、実際の処理結果はコールバック時に渡される。
  Arguments:    address -   NVRAM 上のオフセットを指定。
                size    -   比較するデータサイズをバイト単位で指定。
                pData   -   比較するデータが格納されているバッファを指定。
                callback -  非同期処理完了時に呼び出すコールバック関数を指定。
                arg     -   コールバック関数に渡すパラメータを指定。
  Returns:      NVRAMResult -   非同期処理開始処理の結果を返す。
 *---------------------------------------------------------------------------*/
NVRAMResult
NVRAM_VerifyAsync(u32 address, u32 size, const void* pData, NVRAMCallback callback, void* arg)
{
    OSIntrMode  e;

    SDK_NULL_ASSERT(pData);
    SDK_NULL_ASSERT(callback);

    /* ライブラリ初期化確認 */
    if (NVRAMi_IsInitialized() == FALSE)
    {
#ifdef  SDK_DEBUG
        NVRAMi_Warning("%s: NVRAM library is not initialized.\n", __FUNCTION__);
#endif
        return NVRAM_RESULT_ILLEGAL_STATE;
    }

#ifdef  SDK_DEBUG
    /* パラメータ確認 */
    if ((address >= SPI_NVRAM_ALL_SIZE) || ((address + size) > SPI_NVRAM_ALL_SIZE))
    {
        NVRAMi_Warning("%s: Target range (%u - %u) is over max size of NVRAM (%u).\n", __FUNCTION__, address, address + size, SPI_NVRAM_ALL_SIZE);
    }
#endif

    /* 排他状態確認 */
    e   =   OS_DisableInterrupts();
    if (nvramw.lock != NVRAM_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(e);
        return NVRAM_RESULT_BUSY;
    }
    nvramw.lock =   NVRAM_LOCK_VERIFY;
    (void)OS_RestoreInterrupts(e);

    /* データ読み出しコマンド発行 */
    nvramw.callback     =   callback;
    nvramw.callbackArg  =   arg;
    nvramw.arg[0]   =   address;
    nvramw.arg[1]   =   size;
    nvramw.arg[2]   =   (u32)pData;
    nvramw.seq  =   0;
    DC_InvalidateRange(nvramTemp, HW_CACHE_LINE_SIZE);
    e   =   OS_DisableInterrupts();
    if (TRUE == NVRAMi_ReadDataBytes(address, ((size > HW_CACHE_LINE_SIZE) ? HW_CACHE_LINE_SIZE : size), (void*)nvramTemp))
    {
        NVRAMi_Printf("[NVRAM] ReadDataBytes(%u, %u, %p) = TRUE\n", address, ((size > HW_CACHE_LINE_SIZE) ? HW_CACHE_LINE_SIZE : size), nvramTemp);
        (void)OS_RestoreInterrupts(e);
        return NVRAM_RESULT_SUCCESS;
    }
    else
    {
        NVRAMi_Printf("[NVRAM] ReadDataBytes(%u, %u, %p) = FALSE\n", address, ((size > HW_CACHE_LINE_SIZE) ? HW_CACHE_LINE_SIZE : size), nvramTemp);
        nvramw.lock =   NVRAM_LOCK_OFF;
        (void)OS_RestoreInterrupts(e);
        return NVRAM_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_VerifyCallback
  Description:  データ一致確認シーケンス中の PXI 割り込みコールバック関数。
  Arguments:    data    -   メッセージデータ。
                err     -   PXI 転送エラーフラグ
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
NVRAMi_VerifyCallback(u32 data, BOOL err)
{
    NVRAMResult     result      =   NVRAM_RESULT_FATAL_ERROR;
    u16             pxiResult   =   (u16)((data & SPI_PXI_RESULT_DATA_MASK) >> SPI_PXI_RESULT_DATA_SHIFT);
    u16             tempSize    =   (u16)((nvramw.arg[1] > HW_CACHE_LINE_SIZE) ? HW_CACHE_LINE_SIZE : nvramw.arg[1]);

    NVRAMi_Printf("[NVRAM] Callback(%u, %s)\n", pxiResult, ((err == TRUE) ? "TRUE" : "FALSE"));
    if ((err == FALSE) && (pxiResult == SPI_PXI_RESULT_SUCCESS))
    {
        if (FALSE == NVRAMi_VerifyDataArray((const u8*)nvramTemp, (const u8*)(nvramw.arg[2]), (u32)tempSize))
        {
            result  =   NVRAM_RESULT_VERIFY_ERROR;
        }
        else
        {
            /* 読み出しに成功した分ポインタを進める */
            nvramw.arg[0]   +=  tempSize;
            nvramw.arg[1]   -=  tempSize;
            nvramw.arg[2]   +=  tempSize;
            /* 次の読み出しシーケンスに移行 */
            if (nvramw.arg[1] > 0)
            {
                DC_InvalidateRange(nvramTemp, HW_CACHE_LINE_SIZE);
                if (TRUE == NVRAMi_ReadDataBytes(nvramw.arg[0], ((nvramw.arg[1] > HW_CACHE_LINE_SIZE) ? HW_CACHE_LINE_SIZE : nvramw.arg[1]), (void*)nvramTemp))
                {
                    NVRAMi_Printf("[NVRAM] ReadDataBytes(%u, %u, %p) = TRUE\n", nvramw.arg[0], ((nvramw.arg[1] > HW_CACHE_LINE_SIZE) ? HW_CACHE_LINE_SIZE : nvramw.arg[1]), nvramTemp);
                    return;
                }
                NVRAMi_Printf("[NVRAM] ReadDataBytes(%u, %u, %p) = FALSE\n", nvramw.arg[0], ((nvramw.arg[1] > HW_CACHE_LINE_SIZE) ? HW_CACHE_LINE_SIZE : nvramw.arg[1]), nvramTemp);
                result  =   NVRAM_RESULT_SEND_ERROR;
            }
            else
            {
                /* 全一致確認完了 */
                result  =   NVRAM_RESULT_SUCCESS;
            }
        }
    }
    else
    {
        if (err == TRUE)
        {
            result  =   NVRAM_RESULT_SEND_ERROR;
        }
        else
        {
            switch(pxiResult)
            {
            case SPI_PXI_RESULT_INVALID_PARAMETER:
                result  =   NVRAM_RESULT_ILLEGAL_PARAMETER;
                break;
            case SPI_PXI_RESULT_ILLEGAL_STATUS:
                result  =   NVRAM_RESULT_ILLEGAL_STATE;
                break;
            case SPI_PXI_RESULT_EXCLUSIVE:
                /* データ読み出しコマンドを再発行 */
                DC_InvalidateRange(nvramTemp, HW_CACHE_LINE_SIZE);
                if (TRUE == NVRAMi_ReadDataBytes(nvramw.arg[0], tempSize, (void*)nvramTemp))
                {
                    NVRAMi_Printf("[NVRAM] ReadDataBytes(%u, %u, %p) = TRUE\n", nvramw.arg[0], tempSize, nvramTemp);
                    return;
                }
                NVRAMi_Printf("[NVRAM] ReadDataBytes(%u, %u, %p) = FALSE\n", nvramw.arg[0], tempSize, nvramTemp);
                result  =   NVRAM_RESULT_SEND_ERROR;
                break;
            }
        }
    }

    /* 処理結果をコールバック */
    nvramw.lock =   NVRAM_LOCK_OFF;
    nvramw.seq  =   0;
    if (nvramw.callback != NULL)
    {
        NVRAMCallback   cb  =   nvramw.callback;
    
        nvramw.callback =   NULL;
        cb(result, nvramw.callbackArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NVRAM_Verify
  Description:  NVRAM のデータを一致確認する。
                同期関数なので、割り込みハンドラ内からの呼び出しは禁止。
  Arguments:    address -   NVRAM 上のオフセットを指定。
                size    -   比較するデータサイズをバイト単位で指定。
                pData   -   比較するデータが格納されているバッファを指定。
  Returns:      NVRAMResult -   同期的な処理結果を返す。
 *---------------------------------------------------------------------------*/
NVRAMResult
NVRAM_Verify(u32 address, u32 size, const void* pData)
{
    NVRAMResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* 割り込みハンドラからの呼び出しは禁止 */
    if (OS_GetCurrentThread() == NULL)
    {
#ifdef  SDK_DEBUG
        NVRAMi_Warning("%s: Could not process in exception handler.\n", __FUNCTION__);
#endif
        return NVRAM_RESULT_ILLEGAL_STATE;
    }

    /* 応答取得用メッセージキューを初期化 */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* 非同期関数を実行し、コールバックを待機 */
    result  =   NVRAM_VerifyAsync(address, size, pData, NVRAMi_SyncCallback, (void*)(&msgQ));
    if (result == NVRAM_RESULT_SUCCESS)
    {
        if (FALSE == OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK))
        {
            OS_Panic("%s: Failed to receive message.\n", __FUNCTION__);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_IsInitialized
  Description:  NVRAM ライブラリが初期化済みかどうか調査する。
  Arguments:    None.
  Returns:      BOOL    -   初期化済みの場合に TRUE を返す。
                            初期化前、初期化中の場合には FALSE を返す。
 *---------------------------------------------------------------------------*/
static BOOL
NVRAMi_IsInitialized(void)
{
    return (BOOL)(nvramInitialized == NVRAM_STATE_INITIALIZED);
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_Callback
  Description:  PXI 割り込みコールバック関数。
  Arguments:    tag     -   PXI メッセージ種別。
                data    -   メッセージデータ。
                err     -   PXI 転送エラーフラグ
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
NVRAMi_Callback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)

    switch (nvramw.lock)
    {
    case NVRAM_LOCK_READ:
        NVRAMi_ReadCallback(data, err);
        break;
    case NVRAM_LOCK_WRITE:
        NVRAMi_WriteCallback(data, err);
        break;
    case NVRAM_LOCK_VERIFY:
        NVRAMi_VerifyCallback(data, err);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_SyncCallback
  Description:  同期関数共用のコールバック関数。休止しているスレッドに非同期
                処理結果を送ることで動作可能状態に復帰させる。
  Arguments:    result  -   NVRAM 操作の非同期処理結果が渡される。
                arg     -   同期関数呼び出しスレッドが受信待機しているメッセージ
                            キューへのポインタが渡される。
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
NVRAMi_SyncCallback(NVRAMResult result, void* arg)
{
    SDK_NULL_ASSERT(arg);

    if (FALSE == OS_SendMessage((OSMessageQueue*)arg, (OSMessage)result, OS_MESSAGE_NOBLOCK))
    {
        OS_Panic("%s: Failed to send message.\n", __FUNCTION__);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_AlarmHandler
  Description:  PXI コールバックを偽装するアラームハンドラ。
  Arguments:    arg     -   PXI コールバックの data として使用される。
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
NVRAMi_AlarmHandler(void* arg)
{
    NVRAMi_Callback(PXI_FIFO_TAG_NVRAM, (u32)arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_VerifyDataArray
  Description:  ２つのデータ列の一致確認を行う。
  Arguments:    data0   -   比較するデータ列へのポインタ。
                data1   -   比較するデータ列へのポインタ。
                size    -   比較するサイズをバイト単位で指定。
  Returns:      BOOL    -   一致が確認された場合に TRUE を返す。
                            一致していない場合には FALSE を返す。
 *---------------------------------------------------------------------------*/
static BOOL
NVRAMi_VerifyDataArray(const u8* data0, const u8* data1, u32 size)
{
    u32     i;

    for (i = 0; i < size; i ++)
    {
        if (data0[i] != data1[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
    INSTRUCTIONS
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         NVRAMi_WriteEnable
  Description:  NVRAMに「書き込み許可」命令を発行する。
  Arguments:    None.
  Returns:      BOOL    -   命令をPXI経由で正常に送信できた場合TRUE、
                            失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL
NVRAMi_WriteEnable(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_NVRAM_WREN << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_WriteDisable
  Description:  NVRAMに「書き込み禁止」命令を発行する。
  Arguments:    None.
  Returns:      BOOL    -   命令をPXI経由で正常に送信できた場合TRUE、
                            失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL
NVRAMi_WriteDisable(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_NVRAM_WRDI << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_ReadStatusRegister
  Description:  NVRAMに「ステータスレジスタ読み出し」命令を発行する。
  Arguments:    pData   -   読み出した値を格納する変数へのポインタ。
                            値はARM7が直接書き出すのでキャッシュに注意。
  Returns:      BOOL    -   命令をPXI経由で正常に送信できた場合TRUE、
                            失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL
NVRAMi_ReadStatusRegister(u8* pData)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_RDSR << 8) | ((u32)pData >> 24), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (1 << SPI_PXI_INDEX_SHIFT) | (((u32)pData >> 8) & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (2 << SPI_PXI_INDEX_SHIFT) | (((u32)pData << 8) & 0x0000ff00), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_ReadDataBytes
  Description:  NVRAMに「読み出し」命令を発行する。
  Arguments:    address -   NVRAM上の読み出し開始アドレス。24bitのみ有効。
                size    -   連続して読み出すバイト数。
                pData   -   読み出した値を格納する配列。
                            値はARM7が直接書き出すのでキャッシュに注意。
  Returns:      BOOL    -   命令をPXI経由で正常に送信できた場合TRUE、
                            失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL
NVRAMi_ReadDataBytes(u32 address, u32 size, u8* pData)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_READ << 8) |
                               ((address >> 16) & 0x000000ff), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (1 << SPI_PXI_INDEX_SHIFT) | (address & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (2 << SPI_PXI_INDEX_SHIFT) | (size >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [3]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (3 << SPI_PXI_INDEX_SHIFT) | (size & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [4]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (4 << SPI_PXI_INDEX_SHIFT) | ((u32)pData >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [5]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (5 << SPI_PXI_INDEX_SHIFT) | ((u32)pData & 0x0000ffff), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_PageWrite
  Description:  NVRAMに「ページ書き込み」命令を発行する。
                NVRAMの内部では、「ページ消去」と「ページ書き込み(条件付)」が
                連続して行われる。
  Arguments:    address -   NVRAM上の書き込み開始アドレス。24bitのみ有効。
                size    -   連続して書き込むバイト数。
                            address + size がページ境界(256バイト)を越えると
                            越えた分のデータは無視される。
                pData   -   書き込む値が格納されている配列。
                            ARM7が直接読み出すので、キャッシュからメモリ実体に
                            確実に書き出しておく必要がある。
  Returns:      BOOL    -   命令をPXI経由で正常に送信できた場合TRUE、
                            失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL
NVRAMi_PageWrite(u32 address, u16 size, const u8* pData)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_NVRAM_PW << 8) | ((address >> 16) & 0x000000ff), 0))
    {
        return FALSE;
    }

    // Send packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (1 << SPI_PXI_INDEX_SHIFT) | (address & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // Send packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM, (2 << SPI_PXI_INDEX_SHIFT) | (u32)size, 0))
    {
        return FALSE;
    }

    // Send packet [3]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               (3 << SPI_PXI_INDEX_SHIFT) | ((u32)pData >> 16), 0))
    {
        return FALSE;
    }

    // Send packet [4]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_END_BIT |
                               (4 << SPI_PXI_INDEX_SHIFT) | ((u32)pData & 0x0000ffff), 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NVRAMi_SoftwareReset
  Description:  NVRAMに「ソフトウェアリセット」命令を発行する。
                デバイスがLE25FW203Tの場合に有効。
                デバイスがM45PE40の場合は無視される。
  Arguments:    None.
  Returns:      BOOL    -   命令をPXI経由で正常に送信できた場合TRUE、
                            失敗した場合はFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL
NVRAMi_SoftwareReset(void)
{
    // Send packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_NVRAM,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_NVRAM_SR << 8), 0))
    {
        return FALSE;
    }

    return TRUE;
}
