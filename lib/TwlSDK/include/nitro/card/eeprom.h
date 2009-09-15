/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     eeprom.h

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
#ifndef NITRO_CARD_EEPROM_H_
#define NITRO_CARD_EEPROM_H_


#include <nitro/card/backup.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_IsBackupEeprom

  Description:  現在設定されているバックアップデバイス種別がEEPROMか判定

  Arguments:    None.

  Returns:      現在設定されているバックアップデバイス種別がEEPROMならTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsBackupEeprom(void)
{
    const CARDBackupType t = CARD_GetCurrentBackupType();
    return (((t >> CARD_BACKUP_TYPE_DEVICE_SHIFT) &
              CARD_BACKUP_TYPE_DEVICE_MASK) == CARD_BACKUP_TYPE_DEVICE_EEPROM);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadEeprom

  Description:  同期EEPROM読み込み (チップコマンド "read" に相当)

  Arguments:    src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ReadEeprom(u32 src, void *dst, u32 len)
{
    return CARD_ReadBackup(src, dst, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadEepromAsync

  Description:  非同期EEPROM読み込み (チップコマンド "read" に相当)

  Arguments:    src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadEepromAsync(u32 src, void *dst, u32 len,
                                     MIDmaCallback callback, void *arg)
{
    CARD_ReadBackupAsync(src, dst, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteEeprom

  Description:  同期EEPROM書き込み (チップコマンド "program" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteEeprom(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlashAsync

  Description:  非同期EEPROM書き込み (チップコマンド "write" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteEepromAsync(u32 dst, const void *src, u32 len,
                                      MIDmaCallback callback, void *arg)
{
    CARD_ProgramBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyEeprom

  Description:  同期EEPROMベリファイ

  Arguments:    dst        比較先オフセット
                src        比較元メモリアドレス
                len        比較サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_VerifyEeprom(u32 dst, const void *src, u32 len)
{
    return CARD_VerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyEepromAsync

  Description:  非同期EEPROMベリファイ

  Arguments:    dst        比較先オフセット
                src        比較元メモリアドレス
                len        比較サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_VerifyEepromAsync(u32 dst, const void *src, u32 len,
                                       MIDmaCallback callback, void *arg)
{
    CARD_VerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyEeprom

  Description:  同期EEPROM書き込み + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyEeprom(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramAndVerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyEepromAsync

  Description:  非同期EEPROM書き込み + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyEepromAsync(u32 dst, const void *src, u32 len,
                                               MIDmaCallback callback, void *arg)
{
    CARD_ProgramAndVerifyBackupAsync(dst, src, len, callback, arg);
}


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_EEPROM_H_
