/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - libraries
  File:     internal_ex.c

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

#include    <twl/rtc/common/fifo_ex.h>
#include    <nitro/pxi.h>
#include	"private.h"

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static BOOL     RtcexSendPxiCommand(u32 command);

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_ReadRawCounterAsync

  Description:  未加工のアップカウンタレジスタ設定値を非同期で取得する。
                ARM7からの応答はPXIコールバック(tag:RTC)にて通知される。
                データはOS_GetSystemWork()->real_time_clockに格納される。

  Arguments:    None.

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
BOOL
RTCEXi_ReadRawCounterAsync(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_COUNTER);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_ReadRawFoutAsync

  Description:  未加工のFOUT1/FOUT2レジスタ設定値を非同期で取得する。
                ARM7からの応答はPXIコールバック(tag:RTC)にて通知される。
                データはOS_GetSystemWork()->real_time_clockに格納される。

  Arguments:    None.

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_ReadRawFoutAsync(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_FOUT);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_WriteRawFoutAsync

  Description:  未加工のFOUT1/FOUT2設定値を非同期でデバイスに書き込む。
                ARM7からの応答はPXIコールバック(tag:RTC)にて通知される。
                OS_GetSystemWork()->real_time_clockにあるデータが書き込まれる。

  Arguments:    None.

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_WriteRawFoutAsync(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_WRITE_FOUT);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_ReadRawAlarmEx1Async

  Description:  未加工のアラーム拡張１レジスタ設定値を非同期で取得する。
                ARM7からの応答はPXIコールバック(tag:RTC)にて通知される。
                データはOS_GetSystemWork()->real_time_clockに格納される。

  Arguments:    None.

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_ReadRawAlarmEx1Async(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_ALARM_EX1);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_WriteRawAlarmEx1Async

  Description:  未加工のアラーム拡張１レジスタを非同期でデバイスに書き込む。
                ARM7からの応答はPXIコールバック(tag:RTC)にて通知される。
                OS_GetSystemWork()->real_time_clockにあるデータが書き込まれる。

  Arguments:    None.

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_WriteRawAlarmEx1Async(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_WRITE_ALARM_EX1);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_ReadRawAlarmEx2Async

  Description:  未加工のアラーム拡張２レジスタ設定値を非同期で取得する。
                ARM7からの応答はPXIコールバック(tag:RTC)にて通知される。
                データはOS_GetSystemWork()->real_time_clockに格納される。

  Arguments:    None.

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_ReadRawAlarmEx2Async(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_ALARM_EX2);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_WriteRawAlarmEx2Async

  Description:  未加工のアラーム拡張２レジスタを非同期でデバイスに書き込む。
                ARM7からの応答はPXIコールバック(tag:RTC)にて通知される。
                OS_GetSystemWork()->real_time_clockにあるデータが書き込まれる。

  Arguments:    None.

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_WriteRawAlarmEx2Async(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_WRITE_ALARM_EX2);
}

/*---------------------------------------------------------------------------*
  Name:         RtcexSendPxiCommand

  Description:  指定コマンドをPXI経由でARM7に送信する。

  Arguments:    None.

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL RtcexSendPxiCommand(u32 command)
{
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_RTC,
                               ((command << RTC_PXI_COMMAND_SHIFT) & RTC_PXI_COMMAND_MASK), 0))
    {
        return FALSE;
    }
    return TRUE;
}
