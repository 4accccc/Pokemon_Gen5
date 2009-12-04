/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - libraries
  File:     socl_bind.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-08#$
  $Rev: 1183 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitroWiFi/socl.h>

static int  SOCLi_BindCallBack(void* arg);
static int  SOCLi_ExecBindCommand(SOCLSocket* socket);

/*---------------------------------------------------------------------------*
  Name:         SOCL_Bind

  Description:  ソケットのローカルポート側の設定(バインド)を行なう
                CPS 関数では接続先のポートとアドレスも同時に設定して
                接続するのでここではソケット構造体内部に設定値を保存
                するのみ．

  Arguments:    s		ソケット
                localport	ローカルのポート番号

  Returns:      正or 0: 成功
                負    : エラー
                  現状サポートしているエラー値は以下
                    - :
                  エラー値は追加される場合があるため、他の負の値も全て汎用
                  エラーとして扱うこと．
 *---------------------------------------------------------------------------*/
int SOCL_Bind(int s, u16 local_port)
{
    SOCLSocket*     socket = (SOCLSocket*)s;

    if (SOCL_SocketIsInvalid(socket))
    {
        return SOCL_EINVAL;     // 規定外の値
    }

    if (!SOCL_SocketIsCreated(socket))
    {
        return SOCL_ENETRESET;  // 初期化されていない
    }

    if (SOCL_SocketIsConnecting(socket))
    {
        return SOCL_EALREADY;   // 接続処理を開始している
    }

    socket->local_port = local_port;

    return SOCL_SocketIsUDP(socket) ? SOCLi_ExecBindCommand(socket) : 0;
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_Connect

  Description:  ソケットのリモートポート側の設定(バインド)を行なう
                CPS 関数では接続先のポートとアドレスも同時に設定して
                接続するのでここでは SOC_Bind でソケット構造体内部に保存
                された設定値を使用する．

  Arguments:    s		ソケット
                remoteport	リモートのポート番号
                remoteIP	リモートの IP アドレス

  Returns:      正or 0: 成功
                負    : エラー
                現状サポートしているエラー値は以下
                  SOC_EINPROGRESS  : 接続がまだ完了していません。
                  SOC_EINVAL       : 無効な処理。
                  SOC_EISCONN      : ソケットがすでに接続されています。
                  SOC_EMFILE       : ソケット記述子をこれ以上作れません。
                  SOC_ENETRESET    : ソケットが初期化されていません。
                  SOC_ENOBUFS      : リソース不足。
                  SOC_ETIMEDOUT    : TCPのコネクトタイムアウト。

                エラー値は追加される場合があるため、他の負の値も全て汎用
                エラーとして扱うこと．
 *---------------------------------------------------------------------------*/
int SOCLiResultCodeInConnecting = SOCL_EINPROGRESS; //または SOCL_EALREADY;
int SOCL_Connect(int s, u16 remote_port, SOCLInAddr remote_ip)
{
    SOCLSocket*     socket = (SOCLSocket*)s;
    int result;

    if (SOCL_SocketIsInvalid(socket) || SOCL_SocketIsClosing(socket))
    {
        return SOCL_EINVAL;                 // 規定外の値あるいは切断中
    }

    if (!SOCL_SocketIsCreated(socket))
    {
        return SOCL_ENETRESET;              // 初期化されていない
    }

    // TCP の場合
    if (SOCL_SocketIsTCP(socket))
    {
        if (SOCL_AddrIsClassD(remote_ip) || SOCL_AddrIsBroadcast(remote_ip))
        {
            return SOCL_EINVAL;                 // Class D, ブロードキャストで TCP は使えない
        }
        
        if (SOCL_SocketIsConnected(socket)) // すでに接続されているか
        {
            return SOCL_SocketIsBlock(socket) ? SOCL_EISCONN : 0;   // 既に接続している
        }

        if (SOCL_SocketIsConnecting(socket))
        {
            if (SOCL_SocketIsError(socket))
            {
                return socket->result;

                // SOCL_ETIMEDOUT : 接続タイムアウト
            }
            else
            {
                // 既に接続開始中．
                // リターン値を幾つにするかは、互換性の問題．
                // アプリケーションのよって SOCL_EINPROGRESS を期待しているものと
                // SOCL_EALREADY を期待しているものとがあるらしい．-> サポート情報
                // ユーザが変更可能にするためにグローバル変数にしておく
                return SOCLiResultCodeInConnecting;
            }
        }

        socket->remote_port = remote_port;
        socket->remote_ip = remote_ip;

        result = SOCLi_ExecBindCommand(socket);

        if ( SOCL_SocketIsBlock( socket))
        {
            return result;//成功かSOCL_EMFILE
        }
        else
        {
            return  ( 0 <= result) ? SOCL_EINPROGRESS : result;//SOCL_EINPROGRESSかSOCL_EMFILEかSOCL_ENOBUF
        }
    }
    else
    {
        // UDPの場合は既に接続しててもよい(bind 後の connectに対応。覚えるだけ)
        socket->remote_port = remote_port;
        socket->remote_ip = remote_ip;
        return 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_ExecBindCommand

  Description:  ソケットのリモートポート側の設定(バインド)を行なう
                CPS 関数では接続先のポートとアドレスも同時に設定して
                接続するのでここでは SOC_Bind でソケット構造体内部に保存
                された設定値を使用する．

  Arguments:    s		ソケット
                remoteport	リモートのポート番号
                remoteIP	リモートの IP アドレス

  Returns:      正or 0: 成功
                負    : エラー
                現状サポートしているエラー値は以下
                  SOCL_EMFILE    : ソケットコマンドパケットを確保できなかった
                エラー値は追加される場合があるため、他の負の値も全て汎用
                エラーとして扱うこと．
 *---------------------------------------------------------------------------*/
static int SOCLi_ExecBindCommand(SOCLSocket* socket)
{
    SOCLiCommandPacket*     command;
    s32 result;

    // recv_pipe にコマンドを投げ、コールバック関数内部で CPS_Bind を
    // 起動する．ソケットタイプが NOBLOCK になっているならコマンドを確保
    // できない場合にエラーとなる
    command = SOCLi_CreateCommandPacket(SOCLi_BindCallBack, socket, socket->flag_block);
    if (NULL == command)
    {
        return SOCL_EMFILE;
    }

    // bind 用のパラメータ設定
    command->bind.local_port = socket->local_port;
    command->bind.remote_port = socket->remote_port;
    command->bind.remote_ip = socket->remote_ip;
    socket->state |= SOCL_STATUS_CONNECTING;

    // コマンド実行
    result = SOCLi_ExecCommandPacketInRecvPipe(socket, command);

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_BindCallBack

  Description:  Bind/Connect 処理のコールバック

  Arguments:    arg  コマンドブロックへのポインタ

  Returns:      SOCLi_ExecCommand* に渡される値(BLOCKモードのとき)
 *---------------------------------------------------------------------------*/
static int SOCLi_BindCallBack(void* arg)
{
    SOCLiCommandBind*   cpacket = (SOCLiCommandBind*)arg;
    SOCLSocket*         socket = cpacket->h.socket;
    SOCLiSocketRecvCommandPipe*     recv_pipe;
    u32 retcode = 0;

    SDK_ASSERT(socket);
    recv_pipe = socket->recv_pipe;
    SDK_ASSERT(recv_pipe);

    OS_LockMutex(&recv_pipe->h.in_use); // BLOCKED
    {
#ifdef SDK_MY_DEBUG
        OS_TPrintf("CPS_SocBind(\n");
        OS_TPrintf("   local  port=%d,\n", cpacket->local_port);
        OS_TPrintf("   remote_port=%d,\n", cpacket->remote_port);
        OS_TPrintf("   remote_ip  =%3d. %3d. %3d. %3d\n", CPS_CV_IPv4(cpacket->remote_ip));
#endif
        CPS_SocBind(cpacket->local_port, cpacket->remote_port, cpacket->remote_ip);
        recv_pipe->consumed = 0;

        // TCP ならコネクト処理を行なう．
        // 失敗したらタイムアウトエラー値を返す
        if (cpacket->h.flag_mode == SOCL_FLAGMODE_TCP || cpacket->h.flag_mode == SOCL_FLAGMODE_SSL)
        {
#ifdef SDK_MY_DEBUG
            OS_TPrintf("CPS_TcpConnect\n");
#endif
            retcode = CPS_TcpConnect();
#ifdef SDK_MY_DEBUG
            OS_TPrintf("CPS_TcpConnect.retcode=%d\n", retcode);
#endif
        }
    }

    OS_UnlockMutex(&recv_pipe->h.in_use);

    if (retcode)
    {
        socket->state |= SOCL_STATUS_ERROR;
        return SOCL_ETIMEDOUT;
    }

    socket->state |= SOCL_STATUS_CONNECTED;
    return SOCL_ESUCCESS;               // 成功
}
