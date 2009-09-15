/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NWM - libraries
  File:     nwm_cmd.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>

#include "nwm_common_private.h"
#include "nwm_arm9_private.h"

static u32 *NwmGetCommandBuffer4Arm7(void);

/*---------------------------------------------------------------------------*
  Name:         NWMi_SetCallbackTable

  Description:  各非同期関数に対応したコールバック関数を登録する。

  Arguments:    id          -   非同期関数のAPI ID。
                callback    -   登録するコールバック関数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWMi_SetCallbackTable(NWMApiid id, NWMCallbackFunc callback)
{
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    SDK_NULL_ASSERT(sys);

    sys->callbackTable[id] = callback;
}


/*---------------------------------------------------------------------------*
  Name:         NWMi_SetReceiveCallbackTable

  Description:  データフレーム受信用コールバック関数を登録する。

  Arguments:    id          -   非同期関数のAPI ID。
                callback    -   登録するコールバック関数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWMi_SetReceiveCallbackTable(NWMFramePort port, NWMCallbackFunc callback)
{
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    SDK_NULL_ASSERT(sys);

    sys->recvCallbackTable[port] = callback;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_SendCommand

  Description:  FIFO経由でリクエストをARM7に送信する。
                u32型のパラメータをいくつか伴うコマンドの場合は、
                パラメータを列挙して指定する。

  Arguments:    id          -   リクエストに対応したAPI ID。
                paramNum    -   仮想引数の数。
                ...         -   仮想引数。

  Returns:      int -   NWM_RETCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
NWMRetCode NWMi_SendCommand(NWMApiid id, u16 paramNum, ...)
{
    va_list vlist;
    s32     i;
    int     result;
    u32    *tmpAddr;
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    // コマンド送信用のバッファを確保
    tmpAddr = NwmGetCommandBuffer4Arm7();
    if (tmpAddr == NULL)
    {
        NWM_WARNING("Failed to get command buffer.\n");
        return NWM_RETCODE_FIFO_ERROR;
    }

    // API ID
    *(u16 *)tmpAddr = (u16)id;

    // 指定数個の引数を追加
    va_start(vlist, paramNum);
    for (i = 0; i < paramNum; i++)
    {
        tmpAddr[i + 1] = va_arg(vlist, u32);
    }
    va_end(vlist);

    DC_StoreRange(tmpAddr, NWM_APIFIFO_BUF_SIZE);

    // FIFOで通知
    result = PXI_SendWordByFifo(PXI_FIFO_TAG_WMW, (u32)tmpAddr, FALSE);

    (void)OS_SendMessage(&sys->apibufQ.q, tmpAddr, OS_MESSAGE_BLOCK);

    if (result < 0)
    {
        NWM_WARNING("Failed to send command through FIFO.\n");
        return NWM_RETCODE_FIFO_ERROR;
    }

    return NWM_RETCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_SendCommandDirect

  Description:  FIFO経由でリクエストをARM7に送信する。
                ARM7 に送るコマンドを直接指定する。

  Arguments:    data        -   ARM7 に送るコマンド。
                length      -   ARM7 に送るコマンドのサイズ。

  Returns:      int -   NWM_RETCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
NWMRetCode NWMi_SendCommandDirect(void *data, u32 length)
{
    int     result;
    u32    *tmpAddr;
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    SDK_ASSERT(length <= NWM_APIFIFO_BUF_SIZE);

    // コマンド送信用のバッファを確保
    tmpAddr = NwmGetCommandBuffer4Arm7();
    if (tmpAddr == NULL)
    {
        NWM_WARNING("Failed to get command buffer.\n");
        return NWM_RETCODE_FIFO_ERROR;
    }

    // ARM7 に送るコマンドを専用バッファにコピーする
    MI_CpuCopy8(data, tmpAddr, length);

    DC_StoreRange(tmpAddr, length);

    // FIFOで通知
    result = PXI_SendWordByFifo(PXI_FIFO_TAG_WMW, (u32)tmpAddr, FALSE);

    (void)OS_SendMessage(&sys->apibufQ.q, tmpAddr, OS_MESSAGE_BLOCK);

    if (result < 0)
    {
        NWM_WARNING("Failed to send command through FIFO.\n");
        return NWM_RETCODE_FIFO_ERROR;
    }

    return NWM_RETCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         NwmGetCommandBuffer4Arm7

  Description:  ARM7 向けのコマンド用バッファをプールから確保する

  Arguments:    None.

  Returns:      確保できればその値, 確保できなければ NULL
 *---------------------------------------------------------------------------*/
u32 *NwmGetCommandBuffer4Arm7(void)
{
    u32    *tmpAddr = NULL;
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    do {
        if (FALSE == OS_ReceiveMessage(&sys->apibufQ.q, (OSMessage *)&tmpAddr, OS_MESSAGE_NOBLOCK))
        {
            return NULL;
        }

        // invalidate entire apibuf
        DC_InvalidateRange(sys->apibuf, NWM_APIBUF_NUM * NWM_APIFIFO_BUF_SIZE);

        // リングバッファが使用可能な状態(キューが一杯でない)かを確認
        DC_InvalidateRange(tmpAddr, sizeof(u16));

        if ((*((u16 *)tmpAddr) & NWM_API_REQUEST_ACCEPTED) == 0)
        {
            // return pending buffer to tail of queue
            (void)OS_SendMessage(&sys->apibufQ.q, tmpAddr, OS_MESSAGE_BLOCK);
            tmpAddr = NULL;
            continue;
        }
    } while(tmpAddr == NULL);

    return tmpAddr;
}

