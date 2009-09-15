/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     fram.h

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
#ifndef NITRO_CARD_FRAM_H_
#define NITRO_CARD_FRAM_H_


#include <nitro/card/backup.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_IsBackupFram

  Description:  現在設定されているバックアップデバイス種別がFRAMか判定

  Arguments:    None.

  Returns:      現在設定されているバックアップデバイス種別がFRAMならTRUE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsBackupFram(void)
{
    const CARDBackupType t = CARD_GetCurrentBackupType();
    return (((t >> CARD_BACKUP_TYPE_DEVICE_SHIFT) &
              CARD_BACKUP_TYPE_DEVICE_MASK) == CARD_BACKUP_TYPE_DEVICE_FRAM);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadFram

  Description:  同期リード

  Arguments:    src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_ReadFram(u32 src, void *dst, u32 len)
{
    return CARD_ReadBackup(src, dst, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadFramAsync

  Description:  非同期リード

  Arguments:    src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadFramAsync(u32 src, void *dst, u32 len,
                                   MIDmaCallback callback, void *arg)
{
    CARD_ReadBackupAsync(src, dst, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFram

  Description:  同期ライト (チップコマンド "program" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteFram(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteFramAsync

  Description:  非同期ライト (チップコマンド "program" に相当)

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteFramAsync(u32 dst, const void *src, u32 len,
                                    MIDmaCallback callback, void *arg)
{
    CARD_ProgramBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyFram

  Description:  同期ベリファイ (チップコマンド "read" に相当)

  Arguments:    src        比較元オフセット
                dst        比較先メモリアドレス
                len        比較サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_VerifyFram(u32 dst, const void *src, u32 len)
{
    return CARD_VerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_VerifyFramAsync

  Description:  非同期ベリファイ (チップコマンド "read" に相当)

  Arguments:    src        比較元オフセット
                dst        比較先メモリアドレス
                len        比較サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_VerifyFramAsync(u32 dst, const void *src, u32 len,
                                     MIDmaCallback callback, void *arg)
{
    CARD_VerifyBackupAsync(dst, src, len, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFram

  Description:  同期ライト + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ

  Returns:      CARD_RESULT_SUCCESSで完了すればTRUE、それ以外はFALSE
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_WriteAndVerifyFram(u32 dst, const void *src, u32 len)
{
    return CARD_ProgramAndVerifyBackup(dst, src, len);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_WriteAndVerifyFramAsync

  Description:  非同期ライト + ベリファイ

  Arguments:    dst        転送先オフセット
                src        転送元メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_WriteAndVerifyFramAsync(u32 dst, const void *src, u32 len,
                                             MIDmaCallback callback, void *arg)
{
    CARD_ProgramAndVerifyBackupAsync(dst, src, len, callback, arg);
}


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_FRAM_H_
