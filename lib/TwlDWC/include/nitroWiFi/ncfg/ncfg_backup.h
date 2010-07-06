/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NCFG - include
  File:     ncfg.h

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

#ifndef TWLWIFI_NCFG_BACKUP_H_
#define TWLWIFI_NCFG_BACKUP_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#include <nitro/types.h>

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/
typedef enum
{
    NCFG_RESULT_SUCCESS        = 0,
    NCFG_RESULT_FAILURE        = -1,

    NCFG_RESULT_INIT_OK          =      0,     // 正常終了
    NCFG_RESULT_INIT_OK_ERASE    = -10002,     // データが消えたが正常終了
    NCFG_RESULT_INIT_OK_INIT     = -10003,     // ユーザIDが消失したが正常終了
    NCFG_RESULT_INIT_ERROR_WRITE = -10000,     // バックアップメモリ書き込みエラー
    NCFG_RESULT_INIT_ERROR_READ  = -10001      // バックアップメモリ読み込みエラー
} NCFGResultCode;

#define NCFG_SLOT_EX_MASK 0x0010
typedef enum
{
    NCFG_SLOT_1    = 0,
    NCFG_SLOT_2    = 1,
    NCFG_SLOT_3    = 2,
    NCFG_SLOT_RESERVED = 3,
    NCFG_SLOT_EX_1 = (NCFG_SLOT_EX_MASK | 1),
    NCFG_SLOT_EX_2 = (NCFG_SLOT_EX_MASK | 2),
    NCFG_SLOT_EX_3 = (NCFG_SLOT_EX_MASK | 3)
} NCFGSlotIndex;

#define NCFG_SLOT_SIZE 256
#define NCFG_SLOT_EX_SIZE 512

#define NCFG_NVRAM_WRITE_RETRY_MAX 3

typedef enum
{
    NCFG_FORMAT_VERSION_UNKNOWN = 0x00,
    NCFG_FORMAT_VERSION_NITRO   = 0x10,
    NCFG_FORMAT_VERSION_TWL     = 0x20
} NCFGFormatVersion;


/*---------------------------------------------------------------------------*
    マクロ 定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    構造体 定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    関数 定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NCFGi_InitBackupMemory

  Description:  NVRAM とのやりとりの初期化を行う
                関連する関数を呼び出した際に自動的に呼ばれるので
                明示的な呼び出しは不要

  Arguments:    なし

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFGi_InitBackupMemory(void);

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
u8 NCFG_GetFormatVersion(void);

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
s32 NCFG_ReadBackupMemory(void* buf, u32 size, s32 index);

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
s32 NCFG_WriteBackupMemory(s32 index, const void* buf, u32 size);


/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* TWLWIFI_NCFG_BACKUP_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
