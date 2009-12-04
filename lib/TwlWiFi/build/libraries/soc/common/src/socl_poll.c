/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - libraries
  File:     socl_poll.c

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
#include <nitroWiFi/socl.h>
#include <nitroWiFi/soc.h>

/*---------------------------------------------------------------------------*
  Name:         SOCL_GetStatus

  Description:  指定したソケットの状態を調べます．

  Arguments:    s       ソケット記述子
  
  Returns:      以下のものが OR されたものとなる
                SOC_POLLNVAL		0x80	ソケットが無効
                SOC_POLLHUP		0x40	接続処理中あるいは接続中でない
                SOC_POLLERR		0x20	エラー発生
                SOC_POLLWRNORM		0x08	書き込み可能.
                SOC_POLLRDNORM		0x01	読み込み可能.
 *---------------------------------------------------------------------------*/
int SOCL_GetStatus(int s)
{
    SOCLSocket*     socket = (SOCLSocket*)s;
    int         result = 0;
    OSIntrMode  enable;

    if (SOCL_SocketIsInvalid(socket))
    {
        result |= SOC_POLLNVAL;
    }
    else
    {
        if (SOCL_SocketIsError(socket))
        {
            result |= SOC_POLLERR;
        }

        if (SOCL_SocketIsUDP(socket) || SOCL_SocketIsConnected(socket))
        {
            enable = OS_DisableInterrupts();
            if (SOCLi_GetReadBufferOccpiedSize(socket) > 0)
            {
                result |= SOC_POLLRDNORM;
            }

            if (SOCLi_GetWriteBufferFreeSize(socket) > 0)
            {
                result |= SOC_POLLWRNORM;
            }
            (void)OS_RestoreInterrupts(enable);
        }

        if (SOCL_SocketIsTCP(socket))
        {
            // 接続状態であるかどうか確認しフラグをメンテナンスする
            // READ 可能な状態なら切断状態にはしない
            if (SOCL_SocketIsConnected(socket) && socket->cps_socket.state != CPS_STT_ESTABLISHED &&
                !(result & SOC_POLLRDNORM))
            {
                socket->state &= ~(SOCL_STATUS_CONNECTING | SOCL_STATUS_CONNECTED);
            }

            if (!SOCL_SocketIsConnecting(socket) && !SOCL_SocketIsConnected(socket))
            {
                result |= SOC_POLLHUP;
            }
        }

    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_GetReadBufferOccpiedSize

  Description:  指定したソケットの着信しているデータのサイズを調べます．

  Arguments:    socket   ソケット
  
  Returns:      サイズ
 *---------------------------------------------------------------------------*/
s32 SOCLi_GetReadBufferOccpiedSize(SOCLSocket* socket)
{
    SOCLiSocketRecvCommandPipe*     recv_pipe = socket->recv_pipe;
    s32 size = 0;

    if (recv_pipe)
    {
        if (SOCL_SocketIsUDP(socket))
        {
            SOCLiSocketUdpData*     udpout = recv_pipe->udpdata.out;
            size = (s32) (udpout ? udpout->size : 0);
        }
        else if (SOCL_SocketIsTCP(socket))
        {
            size = (s32) socket->cps_socket.rcvbufp - (s32) recv_pipe->consumed;
        }
    }

    return size;
}
