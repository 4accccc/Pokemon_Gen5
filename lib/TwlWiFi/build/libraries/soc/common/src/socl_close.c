/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - libraries
  File:     socl_close.c

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

static int  SOCLi_CloseCallBack(void* arg);
static void SOCLi_FreeCommandPipe(SOCLiSocketCommandPipe* pipe);

/*---------------------------------------------------------------------------*
  Name:         SOCL_IsClosed

  Description:  ソケットが Close されたかどうか判定する
                SOCL_Close は非同期的にクローズ処理を行なう．
                この処理が完了したかどうかこの関数で判定できる．
  
  Arguments:    s		ソケット
  
  Returns:      TRUE : クローズされた
                FALSE: 不正なソケットまたはクローズされていない
 *---------------------------------------------------------------------------*/
int SOCL_IsClosed(int s)
{
    SOCLSocket*     socket = (SOCLSocket*)s;

    // 規定内のハンドル値であり
    // ソケットが有効なエントリでなくしかも廃棄エントリにもないならクローズ完了とみなす
    if ((int)socket >= 0 && SOCL_SocketIsInvalid(socket) && !SOCL_SocketIsInTrash(socket))
    {
        return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_IsReleased

  Description:  SOCL_Close()が一度成功した上で、
                ソケットがCloseされたかどうか判定する。
                注意)SOCL_Close()を一度も成功していないソケットは、
                     リリース済みと判断します。

  Arguments:    s		ソケット
  
  Returns:      TRUE : ソケットはリリースされている。
                FALSE: ソケットはクローズ&リリース処理中である。
                       不正なソケットである。
 *---------------------------------------------------------------------------*/
BOOL SOCL_IsReleased(int s)
{
    SOCLSocket*  socket = (SOCLSocket*)s;
    OSIntrMode  enable;

    // 不正なソケットはFALSE
    if ((int)socket <= 0)
    {
        return FALSE;
    }

    enable= OS_DisableInterrupts();
    {
        // ソケットが廃棄エントリにあるならクローズ中とみなす
        if (SOCL_SocketIsInTrash(socket))
        {
            (void)OS_RestoreInterrupts(enable);
            return FALSE;
        }

        // ソケットがエントリにないなら既にクローズされたとみなす
        if (SOCL_SocketIsInvalid(socket))
        {
            (void)OS_RestoreInterrupts(enable);
            return TRUE;
        }

        // ソケットのフラグが WAIT_CLOSE ならクローズ中
        if (SOCL_SocketIsWaitingClose(socket))
        {
            (void)OS_RestoreInterrupts(enable);
            return FALSE;
        }
    }
    (void)OS_RestoreInterrupts(enable);

    // ソケットは有効であるが、WAIT_CLOSEでない場合はクローズされたとみなす
    return TRUE;
    // ↑なぜならソケットは、一度クローズ処理が開始してから、
    // ソケットの有効性とWAIT_CLOSE状態でないことが確認された場合
    // ソケットのリソースがSOCL_Create()で再利用されているから。
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_Close

  Description:  ソケットの Close 処理を行なう．
  
  Arguments:    s		ソケット
  
  Returns:      0  : 成功 (一度目の Close 呼び出しと Close 完了時に 0 が返る)
                負 : エラー
                  現状サポートしているエラー値は以下
                  SOCL_EINPROGRESS クローズ処理が継続中
 *---------------------------------------------------------------------------*/
int SOCL_Close(int s)
{
    SOCLSocket*         socket = (SOCLSocket*)s;
    SOCLiCommandPacket*     command;
    OSIntrMode  enable;

    // 規定外の値ならエラー
    if ((int)socket <= 0)
    {
        return SOCL_EINVAL;
    }

    enable= OS_DisableInterrupts();
    {
        // ソケットが廃棄エントリにあるならクローズ中とみなす
        if (SOCL_SocketIsInTrash(socket))
        {
            (void)OS_RestoreInterrupts(enable);
            return SOCL_EINPROGRESS;
        }

        // ソケットがエントリにないなら既にクローズされたとみなす
        if (SOCL_SocketIsInvalid(socket))
        {
            (void)OS_RestoreInterrupts(enable);
            return SOCL_ESUCCESS;
        }

        if (!SOCL_SocketIsCreated(socket))
        {
            (void)OS_RestoreInterrupts(enable);
            return SOCL_ENETRESET;  // 初期化されていない
        }

        // ソケットのフラグが WAIT_CLOSE ならクローズ中
        if (SOCL_SocketIsWaitingClose(socket))
        {
            (void)OS_RestoreInterrupts(enable);
            return SOCL_EINPROGRESS;
        }
    }
    (void)OS_RestoreInterrupts(enable);

    socket->state |= (SOCL_STATUS_CLOSING | SOCL_STATUS_WAIT_CLOSE);

    // TCP なら送信スレッドに NULL コマンドを投げ、全てのデータを送信後、送信スレッドを終了させる
    // NULL コマンドは BLOCK で投げられる
    if (SOCL_SocketIsTCP(socket))
    {
        (void)SOCLi_SendCommandPacket(&socket->send_pipe->h, NULL);
        --SOCLi_Resource.numDescTcp;
    }
    else
    {
        --SOCLi_Resource.numDescUdp;
    }

    // 制御スレッドにコマンドを投げ、コールバック関数内部で CPS_Close を
    // 起動する．コマンドの送信に関しては BLOCK で行なうが CLOSE 処理自体の終了は待たない
    command = SOCLi_CreateCommandPacket(SOCLi_CloseCallBack, socket, SOCL_FLAGBLOCK_BLOCK);
    command->h.response = NULL; // Close 処理終了通知は必要なし
    (void)SOCLi_SendCommandPacketToCtrlPipe(socket, command);

    // 一度目のクローズに 0 を返す．
    return SOCL_ESUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_CloseCallBack

  Description:  Close 処理のコールバック
  
  Arguments:    arg  コマンドブロックへのポインタ
  
  Returns:      SOCLi_ExecCommand* に渡される値(BLOCKモードのとき)
 *---------------------------------------------------------------------------*/
static int SOCLi_CloseCallBack(void* arg)
{
    SOCLiCommandClose*  cpacket = (SOCLiCommandClose*)arg;
    SOCLSocket*         socket = (SOCLSocket*)cpacket->h.socket;
    OSIntrMode          enable;

    if (SOCL_SocketIsTCP(socket))
    {
        // send pipe 内のデータが送信されるまで待つ
        OS_JoinThread(&socket->send_pipe->h.thread);

        // ソケットのクローズ処理
        CPS_TcpShutdown();
        CPS_TcpClose();
        CPS_SocRelease();
    }

    CPS_SocUnRegister();

    socket->state &= ~(SOCL_STATUS_CONNECTING | SOCL_STATUS_CONNECTED);

    // このスレッドに対して終了通知を行なう．この時点ではキューが空いているのでデッドロックは起こらない
    (void)SOCLi_SendCommandPacket(SOCL_SocketIsUDPSend(socket) ? &socket->send_pipe->h : &socket->recv_pipe->h, NULL);

    // スレッドを廃棄リストに載せる
    enable = OS_DisableInterrupts();
    SOCLi_SocketUnregister(socket);
    SOCLi_SocketRegisterTrash(socket);
    (void)OS_RestoreInterrupts(enable);

    socket->state |= SOCL_STATUS_WAIT_RELEASE;

    return SOCL_ESUCCESS;   // 成功
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_CleanupSocket

  Description:  ソケット用の領域を開放する
                以下の領域を順に開放していく
  
                領域
                --------------------------------------------------------
                送信用スレッドのバッファ
                受信用スレッドのバッファ
                CPS  ソケット用のバッファ
                SOCL ソケット領域

  Arguments:    socket  ソケットパラメータ
  
  Returns:      なし
 *---------------------------------------------------------------------------*/
void SOCLi_CleanupSocket(SOCLSocket* socket)
{
    OSIntrMode  enable;

    if (socket)
    {
        socket->state = 0;

        // 各パーツを順に開放する
        if (SOCL_SocketIsTCP(socket))
        {
            SOCLi_FreeCommandPipe(&socket->send_pipe->h);
            SOCLi_FreeCommandPipe(&socket->recv_pipe->h);
        }
        else if (SOCL_SocketIsUDP(socket))
        {
            // UDP 受信データを開放しておく
            SOCLiSocketUdpData*     udpdata = socket->recv_pipe->udpdata.out;
            SOCLiSocketUdpData*     udpdata_next;

            while (udpdata)
            {
                udpdata_next = udpdata->next;
                SOCLi_Free(udpdata);
                udpdata = udpdata_next;
            }

            socket->recv_pipe->udpdata.size = 0;
            socket->recv_pipe->udpdata.in = NULL;
            socket->recv_pipe->udpdata.out = NULL;
            OS_WakeupThread(&socket->recv_pipe->udpdata.waiting);

            SOCLi_FreeCommandPipe(&socket->recv_pipe->h);
        }
        else if (SOCL_SocketIsUDPSend(socket))
        {
            SOCLi_FreeCommandPipe(&socket->send_pipe->h);
        }

        enable = OS_DisableInterrupts();
        SOCLi_SocketUnregister(socket);
        SOCLi_SocketUnregisterTrash(socket);

#ifdef SDK_DEBUG
        MI_CpuFill8(socket, 0xaa, sizeof(*socket));
#endif

        SOCLi_Free(socket);
        (void)OS_RestoreInterrupts(enable);
    }

    return;
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_FreeCommandPipe

  Description:  ソケットのコマンドパイプ領域を開放する

  Arguments:    pipe  コマンドパイプ
  
  Returns:      なし
 *---------------------------------------------------------------------------*/

//
// SOCKET THREAD

//
static void SOCLi_FreeCommandPipe(SOCLiSocketCommandPipe* pipe)
{
    SOCLiCommandPacket*     command;
    OSIntrMode  enable;

    if (pipe == NULL)
        return;

    SDK_ASSERT(OS_GetCurrentThread() != &pipe->thread);

    //
    // ソケットスレッドの終了をまつ
    // 既に終了しているときは何もせず直ぐに返ってくる
    //
    OS_JoinThread(&pipe->thread);

    //
    // ソケットスレッドの処理待ちになっている他のスレッドを起動させる
    //     - コマンド処理待ちのスレッドへ SOCL_ECANCELED メッセージを送信
    //     - コマンドパケットを回収
    // この関連処理全てをまとめてアトミックにしている．
    // 内部のメッセージ関連処理全てが NOBLOCK でないとハングする可能性
    // があるので注意．
    //

    // スレッド切り替えを止める
    enable = OS_DisableInterrupts();
    (void)OS_DisableScheduler();
    
    while (OS_ReceiveMessage(&pipe->queue, (void*) &command, OS_MESSAGE_NOBLOCK))
    {
        if (command)
        {
            if (command->h.response)
            {
                (void)OS_SendMessage(command->h.response, (void*)SOCL_ECANCELED, OS_MESSAGE_NOBLOCK);
            }

            SOCLi_FreeCommandPacket(command);   // NOBLOCK
        }
    }
    
    // 上記のメッセージ送信処理によって起動した他の優先度の高い
    // スレッドに実行権を委譲する
    (void)OS_EnableScheduler();
    OS_RescheduleThread();
    (void)OS_RestoreInterrupts(enable);
    
    return;
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_TrashSocket

  Description:  廃棄リストに登録されているソケットの領域を開放する
                CPS ライブラリから定期的に呼び出される
  
  Arguments:    なし
  
  Returns:      なし
 *---------------------------------------------------------------------------*/
void SOCLi_TrashSocket(void)
{
    OSIntrMode  enable = OS_DisableInterrupts();

    while (SOCLiSocketListTrash != NULL)
    {
        SOCLi_CleanupSocket(SOCLiSocketListTrash);
    }
    (void)OS_RestoreInterrupts(enable);
}
