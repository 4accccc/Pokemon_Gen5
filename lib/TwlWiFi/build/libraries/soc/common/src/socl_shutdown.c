/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - libraries
  File:     socl_shutdown.c

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

static int  SOCLi_ShutdownCallBack(void* arg);

/*---------------------------------------------------------------------------*
  Name:         SOCL_Shutdown

  Description:  ソケットの Shutdown 処理を行なう．
  
  Arguments:    s		ソケット
  
  Returns:      正or 0: 成功
                負    : エラー
                  現状サポートしているエラー値は以下
                    - :  
                  エラー値は追加される場合があるため、他の負の値も全て汎用
                  エラーとして扱うこと．
 *---------------------------------------------------------------------------*/
int SOCL_Shutdown(int s)
{
    SOCLSocket*     socket = (SOCLSocket*)s;
    SOCLiSocketSendCommandPipe*     send_pipe;
    SOCLiCommandPacket*         command;
    s32 result;

    if (SOCL_SocketIsInvalid(socket))
    {
        return SOCL_EINVAL;     // 規定外の値
    }

    if (!SOCL_SocketIsCreated(socket))
    {
        return SOCL_ENETRESET;  // 初期化されていない
    }

    if (!SOCL_SocketIsConnected(socket) || SOCL_SocketIsClosing(socket))
    {
        return SOCL_ENOTCONN;   // 接続されていない
    }

    socket->state |= SOCL_STATUS_CLOSING;

    // 送信スレッドにコマンドを投げ、コールバック関数内部で CPS_Shutdown を
    // 起動する．ソケットタイプが NOBLOCK になっているならコマンドを確保
    // できない場合にエラーとなる
    //
    // 現在の実装は時間の問題のためこのようにしているが、destructor を使用し
    // て無理にソケットを切断するという処理に変更する必要があると思われる
    send_pipe = socket->send_pipe;

    if (send_pipe && send_pipe->exe_socket)
    {
        command = SOCLi_CreateCommandPacket(SOCLi_ShutdownCallBack, send_pipe->exe_socket, socket->flag_block);
        if (NULL == command)
        {
            return SOCL_EMFILE;
        }

        // コマンド実行
        result = SOCLi_ExecCommandPacketInSendPipe(send_pipe->exe_socket, command);
    }
    else
    {
        result = 0;
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_ShutdownCallBack

  Description:  Shutdown 処理のコールバック
  
  Arguments:    arg  コマンドブロックへのポインタ
  
  Returns:      SOCLi_ExecCommand* に渡される値(BLOCKモードのとき)
 *---------------------------------------------------------------------------*/
static int SOCLi_ShutdownCallBack(void* arg)
{
    SOCLiCommandShutdown*   cpacket = (SOCLiCommandShutdown*)arg;
    SOCLSocket*         socket = (SOCLSocket*)cpacket->h.socket;

    if (SOCL_SocketIsTCP(socket))
    {
        CPS_TcpShutdown();
    }

    return SOCL_ESUCCESS;   // 成功
}
