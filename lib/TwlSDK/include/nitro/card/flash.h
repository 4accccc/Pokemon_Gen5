/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     flash.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_FLASH_H_
#define NITRO_CARD_FLASH_H_


#include <nitro/card/backup.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_IsBackupFlash

  Description:  現在設定されているバックアップデバイス種別がFLASHか判定

  Arguments:    None.

  Returns:      現在設定されているバックアップデバイス種別がFLASHならTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsBackupFlash(void)
{
    const CARDBackupType t = CARD_GetCurrentBackupType();
    return (((t >> CARD_BACKUP_TYPE_DEVICE_SHIFT) &
              CARD_BACKUP_TYPE_DEVICE_MASK) == CARD_BACKUP_TYPE_DEVICE_FLASH);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadFlash

  Description:  同期FLASH読み込み (チップコマンド "read" に相当)

  Arguments:    src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ReadFlash(u32 src, void *dst, u32 len)
{
    return CARD_ReadBackup(src, dst, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadFlashAsync

  Description:  非同期FLASH読み込み (チップコマンド "read" に相当)

  Arguments:    src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadFlashAsync(u32 src, void *dst, u32 len,
                                    MIDmaCallback callback, void *arg)
{
    CARD_ReadBackupAsync(src, dst, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlash

  Description:  同期FLASH書き込み (チップコマンド "write" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteFlash(u32 dst, const void *src, u32 len)
{
    return CARD_WriteBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlashAsync

  Description:  非同期FLASH書き込み (チップコマンド "write" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteFlashAsync(u32 dst, const void *src, u32 len,
                                     MIDmaCallback callback, void *arg)
{
    CARD_WriteBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyFlash

  Description:  同期FLASHベリファイ

  Arguments:    dst        比較先オフセット
                src        比較元メモリアドレス
                len        比較サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_VerifyFlash(u32 dst, const void *src, u32 len)
{
    return CARD_VerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyFlashAsync

  Description:  非同期FLASHベリファイ

  Arguments:    dst        比較先オフセット
                src        比較元メモリアドレス
                len        比較サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_VerifyFlashAsync(u32 dst, const void *src, u32 len,
                                      MIDmaCallback callback, void *arg)
{
    CARD_VerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFlash

  Description:  同期FLASH書き込み + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyFlash(u32 dst, const void *src, u32 len)
{
    return CARD_WriteAndVerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFlashAsync

  Description:  同期FLASH書き込み + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyFlashAsync(u32 dst, const void *src, u32 len,
                                              MIDmaCallback callback, void *arg)
{
    CARD_WriteAndVerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseFlashSector

  Description:  同期セクタ消去.

  Arguments:    dst        消去先オフセット
                           セクタサイズの整数倍である必要がある
                len        消去サイズ
                           セクタサイズの整数倍である必要がある

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_EraseFlashSector(u32 dst, u32 len)
{
    return CARD_EraseBackupSector(dst, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_EraseFlashSectorAsync

  Description:  非同期セクタ消去

  Arguments:    dst        消去先オフセット
                           セクタサイズの整数倍である必要がある
                len        消去サイズ
                           セクタサイズの整数倍である必要がある
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_EraseFlashSectorAsync(u32 dst, u32 len,
                                           MIDmaCallback callback, void *arg)
{
    CARD_EraseBackupSectorAsync(dst, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramFlash

  Description:  同期FLASHプログラム (チップコマンド "program" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ProgramFlash(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramFlashAsync

  Description:  非同期FLASHプログラム (チップコマンド "program" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ProgramFlashAsync(u32 dst, const void *src, u32 len,
                                       MIDmaCallback callback, void *arg)
{
    CARD_ProgramBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramAndVerifyFlash

  Description:  同期FLASHプログラム + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ProgramAndVerifyFlash(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramAndVerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ProgramAndVerifyFlashAsync

  Description:  非同期FLASHプログラム + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ProgramAndVerifyFlashAsync(u32 dst, const void *src, u32 len,
                                                MIDmaCallback callback, void *arg)
{
    CARD_ProgramAndVerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlashSector

  Description:  セクタ消去 + プログラム

  Arguments:    dst        転送先オフセット
                           セクタサイズの整数倍である必要がある
                src        転送元メモリアドレス
                len        転送サイズ
                           セクタサイズの整数倍である必要がある

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteFlashSector(u32 dst, const void *src, u32 len)
{
    return CARD_WriteBackupSector(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFlashSectorAsync

  Description:  セクタ消去 + プログラム

  Arguments:    dst        転送先オフセット
                           セクタサイズの整数倍である必要がある
                src        転送元メモリアドレス
                len        転送サイズ
                           セクタサイズの整数倍である必要がある
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteFlashSectorAsync(u32 dst, const void *src, u32 len,
                                           MIDmaCallback callback, void *arg)
{
    CARD_WriteBackupSectorAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFlashSector

  Description:  セクタ消去 + プログラム + ベリファイ

  Arguments:    dst        転送先オフセット
                           セクタサイズの整数倍である必要がある
                src        転送元メモリアドレス
                len        転送サイズ
                           セクタサイズの整数倍である必要がある

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyFlashSector(u32 dst, const void *src, u32 len)
{
    return CARD_WriteAndVerifyBackupSector(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFlashSectorAsync

  Description:  セクタ消去 + プログラム + ベリファイ

  Arguments:    dst        転送先オフセット
                           セクタサイズの整数倍である必要がある
                src        転送元メモリアドレス
                len        転送サイズ
                           セクタサイズの整数倍である必要がある
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyFlashSectorAsync(u32 dst, const void *src, u32 len,
                                                    MIDmaCallback callback, void *arg)
{
    CARD_WriteAndVerifyBackupSectorAsync(dst, src, len, callback, arg);
}


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_CARD_EEPROM_H_ */
