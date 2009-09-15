/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     backup.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_BACKUP_H_
#define NITRO_CARD_BACKUP_H_


#include <nitro/card/types.h>

#include <nitro/mi/dma.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* constants */

// バックアップデバイスタイプ
#define	CARD_BACKUP_TYPE_DEVICE_SHIFT	0
#define	CARD_BACKUP_TYPE_DEVICE_MASK	0xFF
#define	CARD_BACKUP_TYPE_DEVICE_EEPROM	1
#define	CARD_BACKUP_TYPE_DEVICE_FLASH	2
#define	CARD_BACKUP_TYPE_DEVICE_FRAM	3
#define	CARD_BACKUP_TYPE_SIZEBIT_SHIFT	8
#define	CARD_BACKUP_TYPE_SIZEBIT_MASK	0xFF
#define	CARD_BACKUP_TYPE_VENDER_SHIFT	16
#define	CARD_BACKUP_TYPE_VENDER_MASK	0xFF
#define	CARD_BACKUP_TYPE_DEFINE(type, size, vender)	\
	(((CARD_BACKUP_TYPE_DEVICE_ ## type) << CARD_BACKUP_TYPE_DEVICE_SHIFT) |	\
	((size) << CARD_BACKUP_TYPE_SIZEBIT_SHIFT) |	\
	((vender) << CARD_BACKUP_TYPE_VENDER_SHIFT))

typedef enum CARDBackupType
{
    CARD_BACKUP_TYPE_EEPROM_4KBITS = CARD_BACKUP_TYPE_DEFINE(EEPROM, 9, 0),
    CARD_BACKUP_TYPE_EEPROM_64KBITS = CARD_BACKUP_TYPE_DEFINE(EEPROM, 13, 0),
    CARD_BACKUP_TYPE_EEPROM_512KBITS = CARD_BACKUP_TYPE_DEFINE(EEPROM, 16, 0),
    CARD_BACKUP_TYPE_EEPROM_1MBITS = CARD_BACKUP_TYPE_DEFINE(EEPROM, 17, 0),
    CARD_BACKUP_TYPE_FLASH_2MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 18, 0),
    CARD_BACKUP_TYPE_FLASH_4MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 19, 0),
    CARD_BACKUP_TYPE_FLASH_8MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 20, 0),
    CARD_BACKUP_TYPE_FLASH_16MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 21, 0),
    CARD_BACKUP_TYPE_FLASH_64MBITS = CARD_BACKUP_TYPE_DEFINE(FLASH, 23, 0),
    CARD_BACKUP_TYPE_FRAM_256KBITS = CARD_BACKUP_TYPE_DEFINE(FRAM, 15, 0),
    CARD_BACKUP_TYPE_NOT_USE = 0
}
CARDBackupType;

#define CARD_BACKUP_TYPE_FLASH_64MBITS_EX (CARDBackupType)CARD_BACKUP_TYPE_DEFINE(FLASH, 23, 1)


// (ライブラリ内部で使用)

// コンポーネントおよび ensata エミュレータへの PXI コマンドリクエスト
typedef enum CARDRequest
{
    CARD_REQ_INIT = 0,                 /* initialize (setting from ARM9) */
    CARD_REQ_ACK,                      /* request done (acknowledge from ARM7) */
    CARD_REQ_IDENTIFY,                 /* CARD_IdentifyBackup */
    CARD_REQ_READ_ID,                  /* CARD_ReadRomID (TEG && ARM9) */
    CARD_REQ_READ_ROM,                 /* CARD_ReadRom (TEG && ARM9) */
    CARD_REQ_WRITE_ROM,                /* (reserved) */
    CARD_REQ_READ_BACKUP,              /* CARD_ReadBackup */
    CARD_REQ_WRITE_BACKUP,             /* CARD_WriteBackup */
    CARD_REQ_PROGRAM_BACKUP,           /* CARD_ProgramBackup */
    CARD_REQ_VERIFY_BACKUP,            /* CARD_VerifyBackup */
    CARD_REQ_ERASE_PAGE_BACKUP,        /* CARD_EraseBackupPage */
    CARD_REQ_ERASE_SECTOR_BACKUP,      /* CARD_EraseBackupSector */
    CARD_REQ_ERASE_CHIP_BACKUP,        /* CARD_EraseBackupChip */
    CARD_REQ_READ_STATUS,              /* CARD_ReadStatus */
    CARD_REQ_WRITE_STATUS,             /* CARD_WriteStatus */
    CARD_REQ_ERASE_SUBSECTOR_BACKUP,   /* CARD_EraseBackupSubSector */
    CARD_REQ_MAX
}
CARDRequest;

// コマンドリクエストの動作タイプ
typedef enum CARDRequestMode
{
    CARD_REQUEST_MODE_RECV,            /* データ受信 */
    CARD_REQUEST_MODE_SEND,            /* データ送信 (単発のベリファイを含む) */
    CARD_REQUEST_MODE_SEND_VERIFY,     /* データ送信 + ベリファイ */
    CARD_REQUEST_MODE_SPECIAL          /* セクタ消去などの特殊操作 */
}
CARDRequestMode;

// リトライ最大回数
#define	CARD_RETRY_COUNT_MAX	10

// PXI protocol definition
#define CARD_PXI_COMMAND_MASK              0x0000003f   // command part
#define CARD_PXI_COMMAND_SHIFT             0
#define CARD_PXI_COMMAND_PARAM_MASK        0x01ffffc0   // parameter part
#define CARD_PXI_COMMAND_PARAM_SHIFT       6

//---- PXI command
#define CARD_PXI_COMMAND_TERMINATE         0x0001       // arm9->arm7 terminate command
#define CARD_PXI_COMMAND_PULLED_OUT        0x0011       // arm7->arm9 pulled out message
#define CARD_PXI_COMMAND_RESET_SLOT        0x0002       // arm7->arm9 reset-slot message


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_IdentifyBackup

  Description:  カードのバックアップデバイス種別を指定

  Arguments:    type        CARDBackupType型のデバイス種別

  Returns:      読み出しテストに成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL    CARD_IdentifyBackup(CARDBackupType type);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCurrentBackupType

  Description:  現在指定しているバックアップデバイス種別を取得

  Arguments:    None.

  Returns:      現在指定しているバックアップデバイス種別
 *---------------------------------------------------------------------------*/
CARDBackupType CARD_GetCurrentBackupType(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupTotalSize

  Description:  現在指定しているバックアップデバイスの全体サイズを取得

  Arguments:    None.

  Returns:      バックアップデバイスの全体サイズ
 *---------------------------------------------------------------------------*/
u32     CARD_GetBackupTotalSize(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupSectorSize

  Description:  現在指定しているバックアップデバイスのセクタサイズを取得

  Arguments:    None.

  Returns:      バックアップデバイスのセクタサイズ
 *---------------------------------------------------------------------------*/
u32     CARD_GetBackupSectorSize(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetBackupPageSize

  Description:  現在指定しているバックアップデバイスのページサイズを取得

  Arguments:    None.

  Returns:      バックアップデバイスのページサイズ
 *---------------------------------------------------------------------------*/
u32     CARD_GetBackupPageSize(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_LockBackup

  Description:  バックアップデバイスアクセスのためにカードバスをロックする

  Arguments:    lock_id     OS_GetLockID関数で確保した排他制御用ID

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_LockBackup(u16 lock_id);

/*---------------------------------------------------------------------------*
  Name:         CARD_UnlockBackup

  Description:  バックアップデバイスアクセスのためにロックしたカードバスを解放する

  Arguments:    lock_id     CARD_LockBackup関数で使用した排他制御用ID

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_UnlockBackup(u16 lock_id);

/*---------------------------------------------------------------------------*
  Name:         CARD_TryWaitBackupAsync

  Description:  バックアップデバイスアクセス関数が完了したか判定

  Arguments:    None.

  Returns:      バックアップデバイスアクセス関数が完了していればTRUE
 *---------------------------------------------------------------------------*/
BOOL    CARD_TryWaitBackupAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_WaitBackupAsync

  Description:  バックアップデバイスアクセス関数が完了するまで待機

  Arguments:    None.

  Returns:      最後に呼び出したバックアップデバイスアクセス関数が
                CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
BOOL    CARD_WaitBackupAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_CancelBackupAsync

  Description:  処理中のバックアップデバイスアクセス関数に中止を要求する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_CancelBackupAsync(void);


// internal chip command as Serial-Bus-Interface

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestStreamCommand

  Description:  データを転送するコマンドのリクエストを発行

  Arguments:    src        転送元オフセットまたはメモリアドレス
                dst        転送先オフセットまたはメモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE
                req_type   コマンドリクエストタイプ
                req_retry  コマンドリクエストの失敗時リトライ最大回数
                req_mode   コマンドリクエストの動作モード

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL    CARDi_RequestStreamCommand(u32 src, u32 dst, u32 len,
                                   MIDmaCallback callback, void *arg, BOOL is_async,
                                   CARDRequest req_type, int req_retry, CARDRequestMode req_mode);

/*---------------------------------------------------------------------------*
  Name:         CARDi_RequestWriteSectorCommand

  Description:  セクタ消去 + プログラムのリクエストを発行

  Arguments:    src        転送元メモリアドレス
                dst        転送先オフセット
                len        転送サイズ
                verify     ベリファイを行う場合は TRUE
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
BOOL    CARDi_RequestWriteSectorCommand(u32 src, u32 dst, u32 len, BOOL verify,
                                        MIDmaCallback callback, void *arg, BOOL is_async);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadBackup

  Description:  チップコマンド "read" を発行

  Arguments:    src        転送元オフセットまたはメモリアドレス
                dst        転送先オフセットまたはメモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_ReadBackup(u32 src, void *dst, u32 len,
                                 MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len,
                                      callback, arg, is_async,
                                      CARD_REQ_READ_BACKUP, 1, CARD_REQUEST_MODE_RECV);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProgramBackup

  Description:  チップコマンド "program" を使用する読み込みコマンドを発行

  Arguments:    src        転送元オフセットまたはメモリアドレス
                dst        転送先オフセットまたはメモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_ProgramBackup(u32 dst, const void *src, u32 len,
                                    MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_PROGRAM_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SEND);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteBackup

  Description:  チップコマンド "write" を使用する書き込みコマンドを発行

  Arguments:    src        転送元オフセットまたはメモリアドレス
                dst        転送先オフセットまたはメモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_WriteBackup(u32 dst, const void *src, u32 len,
                                  MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_WRITE_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SEND);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_VerifyBackup

  Description:  チップコマンド "read" を使用するベリファイコマンドを発行

  Arguments:    src        転送元オフセットまたはメモリアドレス
                dst        転送先オフセットまたはメモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_VerifyBackup(u32 dst, const void *src, u32 len,
                                   MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_VERIFY_BACKUP, 1, CARD_REQUEST_MODE_SEND);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProgramAndVerifyBackup

  Description:  チップコマンド "program" を使用するプログラム + ベリファイコマンドを発行

  Arguments:    src        転送元オフセットまたはメモリアドレス
                dst        転送先オフセットまたはメモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_ProgramAndVerifyBackup(u32 dst, const void *src, u32 len,
                                             MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_PROGRAM_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SEND_VERIFY);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteAndVerifyBackup

  Description:  チップコマンド "write" を使用するプログラム + ベリファイコマンドを発行

  Arguments:    src        転送元オフセットまたはメモリアドレス
                dst        転送先オフセットまたはメモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_WriteAndVerifyBackup(u32 dst, const void *src, u32 len,
                                           MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand((u32)src, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_WRITE_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SEND_VERIFY);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSector

  Description:  チップコマンド "erase sector" を使用するセクタ消去コマンドを発行

  Arguments:    dst        消去先オフセット
                len        消去サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_EraseBackupSector(u32 dst, u32 len,
                                        MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand(0, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_ERASE_SECTOR_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SPECIAL);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupSubSector

  Description:  チップコマンド "erase subsector" を使用するセクタ消去コマンドを発行.

  Arguments:    dst        消去先オフセット
                len        消去サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したら TRUE.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_EraseBackupSubSector(u32 dst, u32 len,
                                           MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand(0, (u32)dst, len, callback, arg, is_async,
                                      CARD_REQ_ERASE_SUBSECTOR_BACKUP, CARD_RETRY_COUNT_MAX,
                                      CARD_REQUEST_MODE_SPECIAL);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EraseBackupChip

  Description:  チップコマンド "erase chip" を使用するセクタ消去コマンドを発行

  Arguments:    callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期操作を指定されていれば TRUE

  Returns:      処理が成功したらTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_EraseBackupChip(MIDmaCallback callback, void *arg, BOOL is_async)
{
    return CARDi_RequestStreamCommand(0, 0, 0, callback, arg, is_async,
                                      CARD_REQ_ERASE_CHIP_BACKUP, 1, CARD_REQUEST_MODE_SPECIAL);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadBackupAsync

  Description:  非同期バックアップリード (チップコマンド "read" に相当)

  Arguments:    src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadBackupAsync(u32 src, void *dst, u32 len, MIDmaCallback callback, void *arg)
{
    (void)CARDi_ReadBackup(src, dst, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadBackup

  Description:  同期バックアップリード (チップコマンド "read" に相当)

  Arguments:    src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ReadBackup(u32 src, void *dst, u32 len)
{
    return CARDi_ReadBackup(src, dst, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramBackupAsync

  Description:  非同期バックアッププログラム (チップコマンド "program" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ProgramBackupAsync(u32 dst, const void *src, u32 len,
                                        MIDmaCallback callback, void *arg)
{
    (void)CARDi_ProgramBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramBackup

  Description:  同期バックアッププログラム (チップコマンド "program" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ProgramBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_ProgramBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteBackupAsync

  Description:  非同期バックアップライト (チップコマンド "write" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteBackupAsync(u32 dst, const void *src, u32 len,
                                      MIDmaCallback callback, void *arg)
{
    (void)CARDi_WriteBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteBackup

  Description:  同期バックアップライト (チップコマンド "write" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_WriteBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyBackupAsync

  Description:  非同期バックアップベリファイ (チップコマンド "read" に相当)

  Arguments:    src        比較元オフセット
                dst        比較先メモリアドレス
                len        比較サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_VerifyBackupAsync(u32 dst, const void *src, u32 len,
                                       MIDmaCallback callback, void *arg)
{
    (void)CARDi_VerifyBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyBackup

  Description:  同期バックアップベリファイ (チップコマンド "read" に相当)

  Arguments:    src        比較元オフセット
                dst        比較先メモリアドレス
                len        比較サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_VerifyBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_VerifyBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramAndVerifyBackupAsync

  Description:  非同期バックアッププログラム + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ProgramAndVerifyBackupAsync(u32 dst, const void *src, u32 len,
                                                 MIDmaCallback callback, void *arg)
{
    (void)CARDi_ProgramAndVerifyBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramAndVerifyBackup

  Description:  同期バックアッププログラム + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ProgramAndVerifyBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_ProgramAndVerifyBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyBackupAsync

  Description:  非同期バックアップライト + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyBackupAsync(u32 dst, const void *src, u32 len,
                                               MIDmaCallback callback, void *arg)
{
    (void)CARDi_WriteAndVerifyBackup(dst, src, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyBackup

  Description:  同期バックアップライト + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyBackup(u32 dst, const void *src, u32 len)
{
    return CARDi_WriteAndVerifyBackup(dst, src, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupSectorAsync

  Description:  非同期セクタ消去

  Arguments:    dst        消去先オフセット
                len        消去サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_EraseBackupSectorAsync(u32 dst, u32 len, MIDmaCallback callback, void *arg)
{
    (void)CARDi_EraseBackupSector(dst, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupSector

  Description:  同期セクタ消去

  Arguments:    dst        消去先オフセット
                len        消去サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_EraseBackupSector(u32 dst, u32 len)
{
    return CARDi_EraseBackupSector(dst, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupSubSectorAsync

  Description:  非同期サブセクタ消去.

  Arguments:    dst        消去先オフセット
                len        消去サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_EraseBackupSubSectorAsync(u32 dst, u32 len, MIDmaCallback callback, void *arg)
{
    (void)CARDi_EraseBackupSubSector(dst, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupSubSector

  Description:  同期サブセクタ消去.

  Arguments:    dst        消去先オフセット
                len        消去サイズ

  Returns:      CARD_RESULT_SUCCESS で完了すれば TRUE, それ以外は FALSE.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_EraseBackupSubSector(u32 dst, u32 len)
{
    return CARDi_EraseBackupSubSector(dst, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupChipAsync

  Description:  非同期チップ消去

  Arguments:    callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_EraseBackupChipAsync(MIDmaCallback callback, void *arg)
{
    (void)CARDi_EraseBackupChip(callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseBackupChip

  Description:  同期セクタ消去

  Arguments:    None.

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_EraseBackupChip(void)
{
    return CARDi_EraseBackupChip(NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteBackupSectorAsync

  Description:  セクタ単位消去 + セクタ単位プログラム

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteBackupSectorAsync(u32 dst, const void *src, u32 len,
                                            MIDmaCallback callback, void *arg)
{
    (void)CARDi_RequestWriteSectorCommand((u32)src, dst, len, FALSE, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteBackupSector

  Description:  セクタ単位消去 + セクタ単位プログラム

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteBackupSector(u32 dst, const void *src, u32 len)
{
    return CARDi_RequestWriteSectorCommand((u32)src, dst, len, FALSE, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyBackupSectorAsync

  Description:  セクタ単位消去 + セクタ単位プログラム + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyBackupSectorAsync(u32 dst, const void *src, u32 len,
                                                     MIDmaCallback callback, void *arg)
{
    (void)CARDi_RequestWriteSectorCommand((u32)src, dst, len, TRUE, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyBackupSector

  Description:  セクタ単位消去 + セクタ単位プログラム + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyBackupSector(u32 dst, const void *src, u32 len)
{
    return CARDi_RequestWriteSectorCommand((u32)src, dst, len, TRUE, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_AccessStatus

  Description:  ステータスリードまたはライト (テスト用)

  Arguments:    command    CARD_REQ_READ_STATUSまたはCARD_REQ_WRITE_STATUS
                value      CARD_REQ_WRITE_STATUSであれば書き込む値

  Returns:      成功すれば0以上の値、失敗すれば負の値
 *---------------------------------------------------------------------------*/
int CARDi_AccessStatus(CARDRequest command, u8 value);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadStatus

  Description:  ステータスリード (テスト用)

  Arguments:    None.

  Returns:      成功すれば0以上の値、失敗すれば負の値
 *---------------------------------------------------------------------------*/
SDK_INLINE int CARDi_ReadStatus(void)
{
    return CARDi_AccessStatus(CARD_REQ_READ_STATUS, 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WriteStatus

  Description:  ステータスライト (テスト用)

  Arguments:    value      書き込む値

  Returns:      CARD_RESULT_SUCCESS で完了すれば TRUE, それ以外は FALSE.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARDi_WriteStatus(u8 value)
{
    return (CARDi_AccessStatus(CARD_REQ_WRITE_STATUS, value) >= 0);
}


#ifdef __cplusplus
}
#endif // extern "C"


#endif // NITRO_CARD_BACKUP_H_
