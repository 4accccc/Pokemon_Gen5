/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NCFG - libraries
  File:     backup.c

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
#include <nitro.h>
#include <nitro/nvram/nvram.h>
#include <nitroWiFi/ncfg.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define BASE_ADDRESS_OFFSET       0x20
#define BASE_ADDRESS_MAGIC_NUMBER 0xffffffff

#define FORMAT_VERSION_NUMBER_OFFSET     0x1FE

#define SLOT_EX_ADDRESS_OFFSET (-(512*3))

#define NCFG_NVRAM_BUSY_RETRY_MAX         5
#define NCFG_NVRAM_BUSY_RETRY_INTERVAL_MS 1

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static u32 BaseAddress = BASE_ADDRESS_MAGIC_NUMBER;
static u8 FormatVersionNumber = 0;

/*---------------------------------------------------------------------------*
    内部関数プロトタイプ
 *---------------------------------------------------------------------------*/
static BOOL getSlotInfo(s32 index, u32* address, u32* size);
static NVRAMResult ReadNVRAM(u32 address, u32 size, void* pData);
static NVRAMResult WriteNVRAM(u32 address, u32 size, const void* pData);
static NVRAMResult VerifyNVRAM(u32 address, u32 size, const void* pData);

/*---------------------------------------------------------------------------*
  Name:         NCFGi_InitBackupMemory

  Description:  NVRAM とのやりとりの初期化を行う

  Arguments:    なし

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFGi_InitBackupMemory(void)
{
    static u8 nvramBuf[32]   ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);
    static u8 nvramBuf2[32]  ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);
    s32 result;

    if ( BaseAddress != BASE_ADDRESS_MAGIC_NUMBER )
    {
        // すでに初期化済み
        return NCFG_RESULT_SUCCESS;
    }

    NVRAMi_Init();
    result = NCFG_RESULT_SUCCESS;

    DC_InvalidateRange(nvramBuf, 32);
    if ( ReadNVRAM(FORMAT_VERSION_NUMBER_OFFSET, 32, nvramBuf) == NVRAM_RESULT_SUCCESS )
    {
        // ネットワーク設定のバージョン番号
        // 0xff: Nitro 互換, 0x20: TWL 互換
        FormatVersionNumber = nvramBuf[0];
    }
    else
    {
        result = NCFG_RESULT_FAILURE;
        goto error;
    }

    // 再入されても問題が起こらないように nvramBuf を分ける
    DC_InvalidateRange(nvramBuf2, 32);
    if ( ReadNVRAM(BASE_ADDRESS_OFFSET, 32, nvramBuf2) == NVRAM_RESULT_SUCCESS )
    {
        // *(u16*)nvramBuf2 * 8 が本体設定の保存アドレス
        // NTR 互換設定はそこから 1024bytes 前にある
        BaseAddress = (u32)((*(u16*)(&nvramBuf2[0]) * 8) - 0x400);
    }
    else
    {
        result = NCFG_RESULT_FAILURE;
        goto error;
    }

error:
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         NCFG_GetFormatVersion

  Description:  NVRAM に格納されているネットワーク設定のフォーマットの
                バージョンを取得する。

  Arguments:    なし

  Returns:      u8      -   ネットワーク設定のフォーマットバージョン
                            上位ニブルがメジャーバージョン
                            下位ニブルがマイナーバージョン
                            エラーの場合は 0
 *---------------------------------------------------------------------------*/
u8 NCFG_GetFormatVersion(void)
{
    s32 result;
    u8  version;

    // 初回であれば FormatVersionNumber を読み込む
    result = NCFGi_InitBackupMemory();
    if ( result != NCFG_RESULT_SUCCESS )
    {
        return 0;
    }

    version = FormatVersionNumber;
    if ( version == 0xff )
    {
        // Nitro 互換のフォーマットの場合は 0x10 を返す
        version = 0x10;
    }

    return version;
}

/*---------------------------------------------------------------------------*
  Name:         NCFGi_WriteFormatVersion

  Description:  NVRAM に格納されているネットワーク設定のフォーマットの
                バージョンを設定する。

  Arguments:    version -   書き込むネットワーク設定フォーマットバージョン

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFGi_WriteFormatVersion(u8 version);
s32 NCFGi_WriteFormatVersion(u8 version)
{
    static u8 cache_safe_version; // スタックに取られては困るので static
    s32 result;

    // 初回であれば BaseAddress を読み込む
    result = NCFGi_InitBackupMemory();
    if ( result != NCFG_RESULT_SUCCESS )
    {
        return result;
    }

    if ( version == 0x10 )
    {
        // Nitro 互換のフォーマットの場合は 0xff を書き込む
        version = 0xff;
    }

    FormatVersionNumber = version;

    cache_safe_version = version;
    DC_StoreRange(&cache_safe_version, 1);
    result = WriteNVRAM(FORMAT_VERSION_NUMBER_OFFSET, 1, &cache_safe_version);

    if ( result != NVRAM_RESULT_SUCCESS )
    {
        return NCFG_RESULT_FAILURE;
    }

    return NCFG_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NCFG_ReadBackupMemory

  Description:  NVRAM から生の設定データを読み込む
                読み込むデータは強制的にキャッシュを処理されるので注意

  Arguments:    buf   - データ格納バッファ (キャッシュセーフである必要がある)
                size  - データ格納バッファのサイズ
                index - 読み込むデータの番号

  Returns:      s32     -   読み込んだデータのサイズ（非負の整数値）か、
                            以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFG_ReadBackupMemory(void* buf, u32 size, s32 index)
{
    s32 result;
    u32 address;
    u32 readSize;

    // 初回であれば BaseAddress を読み込む
    result = NCFGi_InitBackupMemory();
    if ( result != NCFG_RESULT_SUCCESS )
    {
        return result;
    }

    if ( buf == NULL )
    {
        result = NCFG_RESULT_FAILURE;
        goto error;
    }

    // 読み込み操作
    if ( !getSlotInfo(index, &address, &readSize) )
    {
        // 非対応の index
        result = NCFG_RESULT_FAILURE;
        goto error;
    }

    if ( readSize > size )
    {
        readSize = size;
    }

//    OS_TPrintf("ReadBackupMemory: 0x%08x, %d\n", address, readSize);

    // 読み出すメモリのキャッシュを強制破棄
    DC_InvalidateRange(buf, readSize);

    if ( ReadNVRAM(address, (u16)readSize, (u8*)buf) == NVRAM_RESULT_SUCCESS )
    {
        result = (s32)readSize;
    }
    else
    {
        result = NCFG_RESULT_FAILURE;
    }

error:
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         NCFG_WriteBackupMemory

  Description:  NVRAM に生の設定データを書き込む
                書き込むデータは強制的にキャッシュを処理されるので注意

  Arguments:    index - 書き込むデータの番号
                buf   - データ格納バッファ
                size  - データ格納バッファのサイズ

  Returns:      s32     -   書き込んだデータのサイズ（非負の整数値）か、
                            以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFG_WriteBackupMemory(s32 index, const void* buf, u32 size)
{
    s32 result;
    u32 address;
    u32 writeSize;
    int iRetry;

    // 初回であれば BaseAddress を読み込む
    result = NCFGi_InitBackupMemory();
    if ( result != NCFG_RESULT_SUCCESS )
    {
        return result;
    }

    if ( buf == NULL )
    {
        result = NCFG_RESULT_FAILURE;
        goto error;
    }

    // 書き込み操作
    if ( !getSlotInfo(index, &address, &writeSize) )
    {
        // 非対応の index
        result = NCFG_RESULT_FAILURE;
        goto error;
    }

    if ( writeSize > size )
    {
        writeSize = size;
    }

//    OS_TPrintf("WriteBackupMemory: 0x%08x, %d\n", address, writeSize);

    // 書き出すメモリのキャッシュを強制ストア
    DC_StoreRange(buf, writeSize);

    result = NCFG_RESULT_FAILURE; // リトライ回数オーバー時のエラー値
    for ( iRetry = 0; iRetry < NCFG_NVRAM_WRITE_RETRY_MAX; iRetry++ )
    {
        s32 nvramResult;

        // NVRAM への書き込み
        nvramResult = WriteNVRAM(address, (u16)writeSize, (u8*)buf);
        if ( nvramResult != NVRAM_RESULT_SUCCESS )
        {
            result = NCFG_RESULT_FAILURE;
            break;
        }

        // 書き込み内容の確認
        nvramResult = VerifyNVRAM(address, (u16)writeSize, (u8*)buf);
        if ( nvramResult == NVRAM_RESULT_SUCCESS )
        {
            result = (s32)writeSize;
            break;
        }
        if ( nvramResult != NVRAM_RESULT_VERIFY_ERROR )
        {
            result = NCFG_RESULT_FAILURE;
            break;
        }

        // VERIFY ERROR の場合はリトライする
    }

error:
    return result;
}

BOOL getSlotInfo(s32 index, u32* address, u32* size)
{
    BOOL result;

    SDK_ASSERT(address && size);

    switch (index)
    {
    case NCFG_SLOT_1:
    case NCFG_SLOT_2:
    case NCFG_SLOT_3:
    case NCFG_SLOT_RESERVED:
        *address = BaseAddress + (index - NCFG_SLOT_1) * NCFG_SLOT_SIZE;
        *size = NCFG_SLOT_SIZE;
        result = TRUE;
        break;

    case NCFG_SLOT_EX_1:
    case NCFG_SLOT_EX_2:
    case NCFG_SLOT_EX_3:
        if ( NCFG_GetFormatVersion() >= NCFG_FORMAT_VERSION_TWL )
        {
            *address = BaseAddress + SLOT_EX_ADDRESS_OFFSET + (index - NCFG_SLOT_EX_1) * NCFG_SLOT_EX_SIZE;
            *size = NCFG_SLOT_EX_SIZE;
            result = TRUE;
        }
        else
        {
            // 非対応
            *address = BASE_ADDRESS_MAGIC_NUMBER;
            *size = 0;
            result = FALSE;
        }
        break;

    default:
        *address = BASE_ADDRESS_MAGIC_NUMBER;
        *size = 0;
        result = FALSE;
        break;
    }

    return result;
}

static NVRAMResult ReadNVRAM(u32 address, u32 size, void* pData)
{
    NVRAMResult nvramResult;
    s32 i;

    for ( i = 0; i < NCFG_NVRAM_BUSY_RETRY_MAX; i++ )
    {
        nvramResult = NVRAMi_Read(address, size, pData);
        if ( nvramResult != NVRAM_RESULT_BUSY )
        {
            break;
        }
        if ( OS_IsTickAvailable() && OS_IsAlarmAvailable() )
        {
            OS_Sleep(NCFG_NVRAM_BUSY_RETRY_INTERVAL_MS);
        }
        else
        {
            OS_TWarning("[NCFG] Tick or Alarm System is not initialized.");
        }
    }
    if ( i == NCFG_NVRAM_BUSY_RETRY_MAX )
    {
        OS_TWarning("[NCFG] Failed to retry NVRAM BUSY\n");
    }

    return nvramResult;
}
static NVRAMResult WriteNVRAM(u32 address, u32 size, const void* pData)
{
    NVRAMResult nvramResult;
    s32 i;

    for ( i = 0; i < NCFG_NVRAM_BUSY_RETRY_MAX; i++ )
    {
        nvramResult = NVRAMi_Write(address, size, pData);
        if ( nvramResult != NVRAM_RESULT_BUSY )
        {
            break;
        }
        if ( OS_IsTickAvailable() && OS_IsAlarmAvailable() )
        {
            OS_Sleep(NCFG_NVRAM_BUSY_RETRY_INTERVAL_MS);
        }
        else
        {
            OS_TWarning("[NCFG] Tick or Alarm System is not initialized.");
        }
    }
    if ( i == NCFG_NVRAM_BUSY_RETRY_MAX )
    {
        OS_TWarning("[NCFG] Failed to retry NVRAM BUSY.");
    }

    return nvramResult;
}
static NVRAMResult VerifyNVRAM(u32 address, u32 size, const void* pData)
{
    NVRAMResult nvramResult;
    s32 i;

    for ( i = 0; i < NCFG_NVRAM_BUSY_RETRY_MAX; i++ )
    {
        nvramResult = NVRAMi_Verify(address, size, pData);
        if ( nvramResult != NVRAM_RESULT_BUSY )
        {
            break;
        }
        if ( OS_IsTickAvailable() && OS_IsAlarmAvailable() )
        {
            OS_Sleep(NCFG_NVRAM_BUSY_RETRY_INTERVAL_MS);
        }
        else
        {
            OS_TWarning("[NCFG] Tick or Alarm System is not initialized.");
        }
    }
    if ( i == NCFG_NVRAM_BUSY_RETRY_MAX )
    {
        OS_TWarning("[NCFG] Failed to retry NVRAM BUSY.");
    }

    return nvramResult;
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
