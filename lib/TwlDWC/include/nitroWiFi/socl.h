/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - include
  File:     socl.h

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
#ifndef NITROWIFI_SOCL_H_
#define NITROWIFI_SOCL_H_

#include <nitro/types.h>
#include <nitro/os.h>
#include <nitro/math.h>
#include <nitroWiFi/cps.h>
#include <nitroWiFi/ssl.h>
#include <nitroWiFi/soc_errcode.h>

#ifdef __cplusplus

extern "C" {
#endif

//---------------------------------------------------------------------------
//  SOCL ライブラリ設定
//---------------------------------------------------------------------------
//  汎用
typedef CPSInAddr   SOCLInAddr;
#define SOCL2CPSInAddr(socl)    ((CPSInAddr) (socl))

//  送信スループット最適化用スイッチ
#define SOCL_OPTIMIZE_TCP_SEND  1

//---------------------------------------------------------------------------

//  ソケット構造体

//---------------------------------------------------------------------------

// リンクバッファ
typedef struct SOCLiSocketRingBuffer
{
    CPSSocBuf       area;   // struct {u32 size; u8* data;}
    vu16        in;
    vu16        out;
    OSThreadQueue   waiting;

} SOCLiSocketRingBuffer;

// UDP データヘッダ
typedef struct SOCLiSocketUdpData
{
    struct SOCLiSocketUdpData*  next;
    u16         size;
    u16         remote_port;
    SOCLInAddr  remote_ip;

} SOCLiSocketUdpData;

// UDP データヘッダコントロールブロック
typedef struct SOCLiSocketUdpDataControl
{
    SOCLiSocketUdpData * volatile in;
    SOCLiSocketUdpData * volatile out;
    vu16        size;
    u16         size_max;
    OSThreadQueue   waiting;

} SOCLiSocketUdpDataControl;

// コマンドパイプ
typedef struct SOCLSocketCommandPipe
{
    OSMessageQueue  queue;  // Command キュー
    OSThread        thread; // Command 実行スレッド
    OSMutex         in_use; // Command 投入の排他調整用
} SOCLiSocketCommandPipe;

// 受信用コマンドパイプ
typedef struct SOCLiSocketRecvCommandPipe
{
    SOCLiSocketCommandPipe  h;
    vs32    consumed;       // 消費された受信バッファの量
    u16     consumed_min;   // 最小の consume
    s8      flag_noconsume; // CONSUME するかどうか
    u8      padding[1];
    SOCLiSocketUdpDataControl   udpdata;

    // consumed は u16 sndbuf.size をポイントするが、
    // consumed は計算過程で一時的にマイナスになる可能性があるので vu16 で
    // なく vs32 に consumed_min は固定値なので u16 にする
} SOCLiSocketRecvCommandPipe;

// 送信用コマンドパイプ
typedef struct SOCLiSocketSendCommandPipe
{
    SOCLiSocketCommandPipe  h;
    SOCLiSocketRingBuffer   buffer;     // struct {u32 size; u8* data;}
    struct SOCLSocket*      exe_socket; // Send 実行ソケット
} SOCLiSocketSendCommandPipe;

// ソケット
typedef struct SOCLSocket
{
    CPSSoc  cps_socket;         // 必ず先頭に置くこと(UDP 着信 CallBack で参照するため)
    SOCLiSocketRecvCommandPipe*     recv_pipe;
    SOCLiSocketSendCommandPipe*     send_pipe;
    vs32        result;         // コマンドハンドラの Return 値
    vs16        state;          // ソケットの状態
    s8          flag_block;     // OS_MESSAGE_NOBLOCK(0)/BLOCK(1)/BLOCK_NO_RESULT(3)
    s8          flag_mode;      // TCP(0)UDP(1)UDPSEND(2)ICMP(3)SSL(4)
    u16         local_port;
    u16         remote_port;
    SOCLInAddr  remote_ip;

    struct SOCLSocket*  next;   // 次のソケット-リストによる管理
} SOCLSocket;

// flag_block
#define SOCL_FLAGBLOCK_BLOCK    OS_MESSAGE_BLOCK        // 1
#define SOCL_FLAGBLOCK_NOBLOCK  OS_MESSAGE_NOBLOCK      // 0
#define SOCL_FLAGBLOCK_NORESULT (OS_MESSAGE_BLOCK | 2)  // コマンドの投入はブロックで行なうがその処理終了は待たない
#define SOCL_FLAGISBLOCK(x)     ((x) & OS_MESSAGE_BLOCK)

static inline BOOL SOCL_SocketIsBlock(SOCLSocket* socket)
{
    return(socket->flag_block == SOCL_FLAGBLOCK_BLOCK);
}

static inline BOOL SOCL_SocketIsNoBlock(SOCLSocket* socket)
{
    return(socket->flag_block == SOCL_FLAGBLOCK_NOBLOCK);
}

static inline void SOCL_SocketSetBlock(SOCLSocket* socket)
{
    socket->flag_block = SOCL_FLAGBLOCK_BLOCK;
}

static inline void SOCL_SocketSetNoBlock(SOCLSocket* socket)
{
    socket->flag_block = SOCL_FLAGBLOCK_NOBLOCK;
}

// flag_mode
#define SOCL_FLAGMODE_TCP       0
#define SOCL_FLAGMODE_UDP       1
#define SOCL_FLAGMODE_UDPSEND   2
#define SOCL_FLAGMODE_ICMP      3
#define SOCL_FLAGMODE_SSL       4

static inline BOOL SOCL_SocketIsTCP(SOCLSocket* socket)
{
    return(socket->flag_mode == SOCL_FLAGMODE_TCP || socket->flag_mode == SOCL_FLAGMODE_SSL);
}

static inline BOOL SOCL_SocketIsUDP(SOCLSocket* socket)
{
    return(socket->flag_mode == SOCL_FLAGMODE_UDP);
}

static inline BOOL SOCL_SocketIsUDPSend(SOCLSocket* socket)
{
    return(socket->flag_mode == SOCL_FLAGMODE_UDPSEND);
}

static inline BOOL SOCL_SocketIsICMP(SOCLSocket* socket)
{
    return(socket->flag_mode == SOCL_FLAGMODE_ICMP);
}

static inline BOOL SOCL_SocketIsSSL(SOCLSocket* socket)
{
    return(socket->flag_mode == SOCL_FLAGMODE_SSL);
}

// state
#define SOCL_STATUS_CREATE          0x0001
#define SOCL_STATUS_CONNECTING      0x0002
#define SOCL_STATUS_CONNECTED       0x0004
#define SOCL_STATUS_CLOSING         0x0008
#define SOCL_STATUS_WAIT_CLOSE      0x0010
#define SOCL_STATUS_WAIT_RELEASE    0x0020
#define SOCL_STATUS_ERROR           0x0040
#define SOCL_STATUS_RESET	    0x0080

static inline BOOL SOCL_SocketIsState(SOCLSocket* socket, u16 state)
{
    return(s32) socket->state & (s32) state;
}

static inline BOOL SOCL_SocketIsCreated(SOCLSocket* socket)
{
    return socket && SOCL_SocketIsState(socket, SOCL_STATUS_CREATE);
}

static inline BOOL SOCL_SocketIsConnecting(SOCLSocket* socket)
{
    return SOCL_SocketIsState(socket, SOCL_STATUS_CONNECTING);
}

static inline BOOL SOCL_SocketIsConnected(SOCLSocket* socket)
{
    return SOCL_SocketIsState(socket, SOCL_STATUS_CONNECTED);
}

static inline BOOL SOCL_SocketIsClosing(SOCLSocket* socket)
{
    return SOCL_SocketIsState(socket, SOCL_STATUS_CLOSING);
}

static inline BOOL SOCL_SocketIsWaitingClose(SOCLSocket* socket)
{
    return SOCL_SocketIsState(socket, SOCL_STATUS_WAIT_CLOSE);
}

static inline BOOL SOCL_SocketIsError(SOCLSocket* socket)
{
    return SOCL_SocketIsState(socket, SOCL_STATUS_ERROR);
}

static inline BOOL SOCL_SocketIsReset(SOCLSocket* socket)
{
    return SOCL_SocketIsState(socket, SOCL_STATUS_RESET);
}

static inline BOOL SOCL_AddrIsClassA(SOCLInAddr inAddr)
{
    return (inAddr & 0x80000000) == 0x00000000;
}

static inline BOOL SOCL_AddrIsClassB(SOCLInAddr inAddr)
{
    return (inAddr & 0xc0000000) == 0x80000000;
}

static inline BOOL SOCL_AddrIsClassC(SOCLInAddr inAddr)
{
    return (inAddr & 0xe0000000) == 0xc0000000;
}

static inline BOOL SOCL_AddrIsClassD(SOCLInAddr inAddr)
{
    return (inAddr & 0xf0000000) == 0xe0000000;
}

static inline BOOL SOCL_AddrIsClassE(SOCLInAddr inAddr)
{
    return (inAddr & 0xf8000000) == 0xf0000000;
}

static inline BOOL SOCL_AddrIsBroadcast(SOCLInAddr inAddr)
{
    return (CPSMyIp ^ inAddr) & CPSNetMask ? inAddr == 0xffffffff : (inAddr & ~CPSNetMask) == ~CPSNetMask;
}

// api flags
#define SOCL_MSG_PEEK       0x02
#define SOCL_MSG_DONTWAIT   0x04

//---------------------------------------------------------------------------
//  SSL 関連
//---------------------------------------------------------------------------
typedef CPSSslConnection    SOCLSslConnection;
typedef CPSCaInfo       SOCLCaInfo;
typedef int (*SOCLSslAuthHandler) (int result, SOCLSslConnection*con, int level);

//      Authentication error codes
#define SOCL_CERT_OUTOFDATE                 CPS_CERT_OUTOFDATE
#define SOCL_CERT_BADSERVER                 CPS_CERT_BADSERVER

#define SOCL_CERT_ERRMASK                   CPS_CERT_ERRMASK

#define SOCL_CERT_NOROOTCA                  CPS_CERT_NOROOTCA
#define SOCL_CERT_BADSIGNATURE              CPS_CERT_BADSIGNATURE
#define SOCL_CERT_UNKNOWN_SIGALGORITHM      CPS_CERT_UNKNOWN_SIGALGORITHM
#define SOCL_CERT_UNKNOWN_PUBKEYALGORITHM   CPS_CERT_UNKNOWN_PUBKEYALGORITHM

//---------------------------------------------------------------------------
//  ソケットスレッドのコマンドパケットの構造体
//---------------------------------------------------------------------------
typedef int (*SOCLiCommandHandler) (void*);             // コマンドのコールバック関数

//

// コマンド共通 (全てのコマンドパケットの先頭はこの形になる)

//
typedef struct SOCLiCommandHeader
{
    SOCLiCommandHandler handler;    // ソケットスレッドから呼び出す関数
    SOCLSocket*         socket;     // ここへのアクセスは状態フラグ変更のみ
    OSMessageQueue*     response;   // コールバック完了のレスポンス送信先
    s8  flag_mode;                  // TCP(0) UDP(1) UDPSEND(2) ICMP(3) SSL(4)
    s8  flag_block;                 // OS_MESSAGE_NOBLOCK(0) or BLOCK(1)
    u8  padding[2];

} SOCLiCommandHeader;

//

// コマンド: SOCL_Socket

//
typedef struct SOCLiCommandCreateSocket
{
    SOCLiCommandHeader  h;

} SOCLiCommandCreateSocket;

//

// コマンド: SOCL_Bind/SOCL_Connect

//
typedef struct SOCLiCommandBind
{
    SOCLiCommandHeader  h;
    u16         local_port;     // CPS_Bind パラメータ
    u16         remote_port;    // CPS_Bind パラメータ
    SOCLInAddr  remote_ip;      // CPS_Bind パラメータ
} SOCLiCommandBind;

//

// コマンド: SOCL_ListenAccept

//
typedef struct SOCLiCommandListenAccept
{
    SOCLiCommandHeader  h;
    u16 local_port; // CPS_Bind パラメータ
    u8  padding[2];
    volatile u16*           remote_port_ptr;    // CPS_SocWho パラメータ
    volatile SOCLInAddr*    remote_ip_ptr;      // CPS_SocWho パラメータ
} SOCLiCommandListenAccept;

//

// コマンド: SOCL_Read

//
typedef struct SOCLiCommandRead
{
    SOCLiCommandHeader  h;
    u8*         buffer;         // CPS_Read/Consume パラメータ
    s32         buffer_len;     // CPS_Read/Consume パラメータ
    u16*        remote_port;    // 受信相手 PORT 格納場所
    SOCLInAddr*     remote_ip;  // 受信相手 IP   格納場所
    s8  flag_noconsume;         // Consume を行なうか？
    u8  padding[3];
} SOCLiCommandRead;

//

// コマンド: SOCL_Consume

//
typedef struct SOCLiCommandConsume
{
    SOCLiCommandHeader  h;

} SOCLiCommandConsume;

//

// コマンド: SOCL_Write

//
typedef struct SOCLiCommandWrite
{
    SOCLiCommandHeader  h;
    u8*         buffer1;
    s32         buffer1_len;
    u8*         buffer2;
    s32         buffer2_len;
    u16         wrtbuf_after;
    u8          padding[2];
    u16         local_port;
    u16         remote_port;
    SOCLInAddr  remote_ip;

} SOCLiCommandWrite;

//

// コマンド: SOCL_Shutdown

//
typedef struct SOCLiCommandShutdown
{
    SOCLiCommandHeader  h;

} SOCLiCommandShutdown;

//

// コマンド: SOCL_Close

//
typedef struct SOCLiCommandClose
{
    SOCLiCommandHeader  h;

} SOCLiCommandClose;

//

// コマンド: SOCL_EnableSsl

//
typedef struct SOCLiCommandEnableSsl
{
    SOCLiCommandHeader  h;
    SOCLSslConnection*  connection;
} SOCLiCommandEnableSsl;

//
// どのコマンドパケットでも格納可能なサイズのパケット
// コマンドが追加されるたびにここにメンバーを追加すること
//
typedef union SOCLiCommandPacket
{
    SOCLiCommandHeader          h;
    SOCLiCommandCreateSocket    create_socket;
    SOCLiCommandBind        bind;
    SOCLiCommandListenAccept    listen_accept;
    SOCLiCommandRead        read;
    SOCLiCommandConsume         consume;
    SOCLiCommandWrite       write;
    SOCLiCommandShutdown        shutdown;
    SOCLiCommandClose       close;
    SOCLiCommandEnableSsl       enable_ssl;

} SOCLiCommandPacket;

//
// コマンドパケットキュー
//
// 同時にキューイングされるソケットコマンドパケットの最大数．
// これを取得するときにも BLOCK 処理が行なわれるため、数を超えた
// ときにも致命的にはならない．(NOBLOCK の時はエラーとなる)
//
#define SOCL_CMDPACKET_MAX  64

//---------------------------------------------------------------------------

//  ライブラリ初期化用パラメータ

//---------------------------------------------------------------------------
typedef struct
{
    BOOL    use_dhcp;           // if TRUE, use dhcp
    struct
    {
        SOCLInAddr  my_ip;
        SOCLInAddr  net_mask;
        SOCLInAddr  gateway_ip;
        SOCLInAddr  dns_ip[2];
    } host_ip;
    void*  (*alloc) (u32);
    void (*free) (void*);

    u32     cmd_packet_max;     // コマンドパケットの最大数
    u32     lan_buffer_size;    // 0 なら *alloc で自力確保
    void*   lan_buffer;         // 0 なら default(16384)設定

    // CPS スレッドの優先度
    // 0 なら SOCL_CPS_SOCKET_THREAD_PRIORITY
    s32     cps_thread_prio;

    // 通信バッファサイズ
    s32     mtu;                //default 1500
    s32     rwin;
} SOCLConfig;

#define SOCLi_MTUtoMSS(mtu)             ((mtu) - 40)

#define SOCL_LAN_BUFFER_SIZE_DEFAULT    16384
#define SOCL_MTU_SIZE_MAX               1500
#define SOCL_MTU_SIZE_MIN               576

#define SOCL_MTU_SIZE_DEFAULT           576
#define SOCL_RWIN_SIZE_DEFAULT          (SOCLi_MTUtoMSS(SOCL_MTU_SIZE_DEFAULT) * 8)

#define SOCL_DHCP_REQUEST               0x01
#define SOCL_DHCP_CALLBACK              0x02
#define SOCL_DHCP_ERROR                 SOCL_DHCP_CALLBACK

extern int          SOCLiDhcpState;
extern SOCLInAddr   SOCLiRequestedIP;
extern u32          SOCLiYieldWait;
extern u32          SOCLiDefaultLanBufferSize;

static inline void SOCL_SetRequestedIP(SOCLInAddr ip)
{
    SOCLiRequestedIP = ip;
}

static inline void SOCL_SetYieldWait(u32 wait)
{
    SOCLiYieldWait = wait;
}

static inline void SOCL_SetSslHandshakePriority(u32 prio)
{
    CPS_SetSslHandshakePriority(prio);
}

static inline u32 SOCL_GetSslHandshakePriority(void)
{
    return CPS_GetSslHandshakePriority();
}

static inline void SOCL_SetDefaultLanBufferSize(u32 size)
{
    SOCLiDefaultLanBufferSize = size;
}

//---------------------------------------------------------------------------

// ソケット構造体初期化用パラメータ (socl_const.c 参照)

//---------------------------------------------------------------------------
typedef struct SOCLSocketThreadParam
{
    u16 stack_size;
    u8  priority;
    u8  queue_max;

} SOCLSocketCommandPipeParam;

typedef struct SOCLSocketBufferParam
{
    u16 rcvbuf_size;
    u16 rcvbuf_consume_min;
    u16 sndbuf_size;
    u16 linbuf_size;
    u16 outbuf_size;
    u16 wrtbuf_size;                        // ライト用リングバッファ
    u16 udpbuf_size;                        // UDP 受信用バッファ
} SOCLSocketBufferParam;

typedef struct SOCLSocketParam
{
    s8  flag_mode;                          // TCP/UDP 設定 0:TCP 1:UDP 3:PING 4:SSL
    s8  flag_block;                         // BLOCK MODE   0:NOBLOCK 1:BLOCK
    SOCLSocketBufferParam       buffer;     // 送受信バッファ用設定
    SOCLSocketCommandPipeParam  recv_pipe;  // 受信ソケット処理スレッド用
    SOCLSocketCommandPipeParam  send_pipe;  // 送信ソケット処理スレッド用
} SOCLSocketParam;

extern SOCLSocketParam  SOCLSocketParamTCP;
extern SOCLSocketParam  SOCLSocketParamUDP;
extern SOCLSocketParam  SOCLSocketParamUDPSend;

#define SOCL_CPS_SOCKET_THREAD_PRIORITY 11  // cps thread prio

//--- TCP
#define SOCL_TCP_SOCKET_CPS_RCVBUF_SIZE (1460 * 2)  // cps.rcvbuf_size

// cps.rcvbuf を consume する最小の長さ(TCP の ACK 制御のため, UDPは1固定)
#define SOCL_TCP_SOCKET_CPS_RCVBUF_CONSUME_MIN  1460

#if SOCL_OPTIMIZE_TCP_SEND
#define SOCL_TCP_SOCKET_CPS_SNDBUF_SIZE     ((SOCLi_MTUtoMSS(SOCL_MTU_SIZE_MAX) * 2) + SOCL_TCP_SNDBUF_NOCOPY_OFFSET)   // cps.sndbuf_size
#define SOCL_TCP_SOCKET_SEND_WRTBUF_SIZE    ((SOCL_TCP_SOCKET_CPS_SNDBUF_SIZE - SOCL_TCP_SNDBUF_NOCOPY_OFFSET) * 2 + 1)     // wrtbuf_size
#else
#define SOCL_TCP_SOCKET_CPS_SNDBUF_SIZE     (SOCL_MTU_SIZE_MAX + 14)                    // cps.sndbuf_size
#define SOCL_TCP_SOCKET_SEND_WRTBUF_SIZE    (SOCL_TCP_SOCKET_CPS_SNDBUF_SIZE * 2 + 1)   // wrtbuf_size
#endif

#define SOCL_TCP_SOCKET_RECV_THREAD_PRIORITY    12      // thread.priority
#define SOCL_TCP_SOCKET_SEND_THREAD_PRIORITY    13      // thread.priority
#define SOCL_TCP_SOCKET_RECV_THREAD_STACK_SIZE  2048    // thread.stack_size
#define SOCL_TCP_SOCKET_SEND_THREAD_STACK_SIZE  2048    // thread.stack_size
#define SOCL_TCP_SOCKET_RECV_THREAD_QUEUE_MAX   32      // thread.queue_max
#define SOCL_TCP_SOCKET_SEND_THREAD_QUEUE_MAX   32      // thread.queue_max

//--- UDP
// SOCL_UDP_SOCKET_CPS_SNDBUF_SIZE は UDP で送ることの出来る最大データグラム長
// を制御する値となる．
#define SOCL_UDP_SOCKET_CPS_RCVBUF_SIZE         1472    // cps.rcvbuf_size
#define SOCL_UDP_SOCKET_CPS_SNDBUF_SIZE         1514    // cps.buffer_size
#define SOCL_UDP_SOCKET_SEND_WRTBUF_SIZE        (SOCL_UDP_SOCKET_CPS_SNDBUF_SIZE + 256 + 1) // wrtbuf_size
#define SOCL_UDP_SOCKET_RECV_UDPBUF_SIZE        2048    // udpbuf_size
#define SOCL_UDP_SOCKET_RECV_THREAD_PRIORITY    12      // thread.priority
#define SOCL_UDP_SOCKET_SEND_THREAD_PRIORITY    13      // thread.priority
#define SOCL_UDP_SOCKET_RECV_THREAD_STACK_SIZE  2048    // thread.stack_size
#define SOCL_UDP_SOCKET_SEND_THREAD_STACK_SIZE  2048    // thread.stack_size
#define SOCL_UDP_SOCKET_RECV_THREAD_QUEUE_MAX   32      // thread.queue_max
#define SOCL_UDP_SOCKET_SEND_THREAD_QUEUE_MAX   32      // thread.queue_max
#ifdef SDK_USE_CHAROUT
#define SOCL_TCP_SOCKET_CPS_LINBUF_SIZE 128             // cps.linbuf_size
#define SOCL_TCP_SOCKET_CPS_OUTBUF_SIZE 512             // cps.outbuf_size
#define SOCL_UDP_SOCKET_CPS_LINBUF_SIZE 128             // cps.linbuf_size
#define SOCL_UDP_SOCKET_CPS_OUTBUF_SIZE 512             // cps.outbuf_size
#else
#define SOCL_TCP_SOCKET_CPS_LINBUF_SIZE 0               // cps.linbuf_size
#define SOCL_TCP_SOCKET_CPS_OUTBUF_SIZE 0               // cps.outbuf_size
#define SOCL_UDP_SOCKET_CPS_LINBUF_SIZE 0               // cps.linbuf_size
#define SOCL_UDP_SOCKET_CPS_OUTBUF_SIZE 0               // cps.outbuf_size
#endif

#define SOCL_TCP_SNDBUF_NOCOPY_OFFSET   (14 + 20 + 20)
#define SOCL_UDP_SNDBUF_NOCOPY_OFFSET   (14 + 20 + 8)

//---------------------------------------------------------------------------
//  API プロトタイプ宣言
//---------------------------------------------------------------------------
int     SOCLi_StartupCommandPacketQueue(s32 cp_max_count);
int     SOCLi_CleanupCommandPacketQueue(void);
void    SOCLi_CommandPacketHandler(void* arg);

SOCLiCommandPacket*         SOCLi_AllocCommandPacket(s32 flag);
SOCLiCommandPacket*         SOCLi_CreateCommandPacket(SOCLiCommandHandler handler, SOCLSocket* socket, s32 flag);
SOCLiSocketCommandPipe*     SOCLi_GetCtrlPipe(SOCLSocket* socket);
void    SOCLi_FreeCommandPacket(SOCLiCommandPacket* command);
int     SOCLi_SendCommandPacket(SOCLiSocketCommandPipe* pipe, SOCLiCommandPacket* command);
int     SOCLi_SendCommandPacketToCtrlPipe(SOCLSocket* socket, SOCLiCommandPacket* command);
int     SOCLi_ExecCommandPacket(SOCLiSocketCommandPipe* pipe, SOCLiCommandPacket* command);
int     SOCLi_ExecCommandPacketInRecvPipe(SOCLSocket* socket, SOCLiCommandPacket* command);
int     SOCLi_ExecCommandPacketInSendPipe(SOCLSocket* socket, SOCLiCommandPacket* command);
int     SOCLi_ExecCommandPacketInCtrlPipe(SOCLSocket* socket, SOCLiCommandPacket* command);

#define SOCLi_ROUNDUP4(size)    MATH_ROUNDUP((size), 4)
u32         SOCLi_RoundUp4(u32 size);
int         SOCLi_CheckNConfig(void);
int         SOCL_Startup(const SOCLConfig* socl_config);
int         SOCL_Cleanup(void);
extern int  SOCLiUDPSendSocket;

void        SOCLi_SetMyIPinConfig(SOCLInAddr ip, SOCLInAddr mask, SOCLInAddr gateway);
void        SOCLi_SetMyIP(void);
void        SOCLi_DhcpTimeout(void);

int         SOCL_CreateSocket(const SOCLSocketParam* param);
int         SOCL_Bind(int s, u16 local_port);
int         SOCL_Connect(int s, u16 remote_port, SOCLInAddr remote_ip);
int         SOCL_Listen(int s, int backlog);
int         SOCL_Accept(int s, volatile u16* remote_port_ptr, volatile SOCLInAddr* remote_ip_ptr);
int         SOCL_ListenAccept(int s, volatile u16* remote_port_ptr, volatile SOCLInAddr* remote_ip_ptr);
int         SOCL_Read(int, void* , int, u16* , SOCLInAddr* );
int         SOCL_ReadFrom(int, void* , int, u16* , SOCLInAddr* , int);
int         SOCL_ReadNoBlock(int, void* , int, u16* , SOCLInAddr* );
int         SOCL_ReadBlock(int, void* , int, u16* , SOCLInAddr* );
int         SOCL_Write(int, const void* buffer, int buffer_len, u16 port, SOCLInAddr ip);
int         SOCL_WriteTo(int, const void* buffer, int buffer_len, u16 port, SOCLInAddr ip, int flags);
int         SOCL_WriteNoBlock(int s, const void* buffer, int buffer_len, u16 port, SOCLInAddr ip);
int         SOCL_WriteBlock(int s, const void* buffer, int buffer_len, u16 port, SOCLInAddr ip);
int         SOCL_Shutdown(int s);
int         SOCL_IsClosed(int s);
int         SOCL_Close(int s);
int         SOCL_CloseAll(void);
int         SOCL_CalmDown(void);
int         SOCL_GetRemote(int s, u16* port, SOCLInAddr* ip);
SOCLInAddr  SOCL_Resolve(const char* hostname);
SOCLInAddr  SOCL_InetAtoH(const char* hostname);
int         SOCL_GetResolver(SOCLInAddr* dns1, SOCLInAddr* dns2);
int         SOCL_SetResolver(const SOCLInAddr dns1, const SOCLInAddr dns2);
SOCLInAddr  SOCL_GetHostID(void);
BOOL        SOCL_LinkIsOn(void);
BOOL        SOCL_IsNetworkAvailable(void);
BOOL        SOCL_IsReleased ( int socket);

const char*     SOCL_GetErrName(int errcode);
int SOCL_GetStatus(int s);
int SOCL_EnableSsl(int s, SOCLSslConnection* con);

// ERROR-CODE BEGINS
#define SOCL_ESUCCESS       0
#define SOCL_ENOMEM         SOC_ENOMEM
#define SOCL_EMFILE         SOC_EMFILE
#define SOCL_ECONNRESET     SOC_ECONNRESET
#define SOCL_ECANCELED      SOC_ECANCELED
#define SOCL_EALREADY       SOC_EALREADY
#define SOCL_ENOTCONN       SOC_ENOTCONN
#define SOCL_ENETRESET      SOC_ENETRESET
#define SOCL_EINVAL         SOC_EINVAL
#define SOCL_ENOBUFS        SOC_ENOBUFS
#define SOCL_ETIMEDOUT      SOC_ETIMEDOUT
#define SOCL_EWOULDBLOCK    SOC_EWOULDBLOCK
#define SOCL_EAGAIN         SOC_EAGAIN
#define SOCL_EINPROGRESS    SOC_EINPROGRESS
#define SOCL_EISCONN        SOC_EISCONN
#define SOCL_EMSGSIZE       SOC_EMSGSIZE

// ERROR-CODE ENDS
#define SOCL_POLLRDNORM         SOC_POLLRDNORM
#define SOCL_POLLWRNORM         SOC_POLLWRNORM
#define SOCL_POLLERR            SOC_POLLERR
#define SOCL_POLLHUP            SOC_POLLHUP
#define SOCL_POLLNVAL           SOC_POLLNVAL

#define SOCL_POLLERRORSTATUS    (SOCL_POLLERR | SOCL_POLLHUP | SOCL_POLLNVAL)
#define SOCL_POLLREADSTATUS     (SOCL_POLLRDNORM | SOCL_POLLERRORSTATUS)
#define SOCL_POLLWRITESTATUS    (SOCL_POLLWRNORM | SOCL_POLLERRORSTATUS)

//---------------------------------------------------------------------------
// プライベート関数
SOCLSocket*     SOCLi_StartupSocket(const SOCLSocketParam* param);
void        SOCLi_CleanupSocket(SOCLSocket* socket);
void        SOCLi_TrashSocket(void);
s32     SOCLi_GetWriteBufferFreeSize(SOCLSocket* socket);
s32     SOCLi_GetReadBufferOccpiedSize(SOCLSocket* socket);
int     SOCLi_UdpRecvCallback(u8* data, u32 len, CPSSoc* soc);

void    SOCLi_SocketRegister(SOCLSocket* socket);
void    SOCLi_SocketUnregister(SOCLSocket* socket);
void    SOCLi_SocketRegisterTrash(SOCLSocket* socket);
void    SOCLi_SocketUnregisterTrash(SOCLSocket* socket);
int     SOCL_SocketIsInvalid(SOCLSocket* socket);
int     SOCL_SocketIsInTrash(SOCLSocket* socket);
void    SOCLi_CleanupCPS(void);
extern SOCLSocket*  SOCLiSocketList;
extern SOCLSocket*  SOCLiSocketListTrash;

/*---------------------------------------------------------------------------*
  Name:         SOCL_TcpSocket/SOCL_UdpSocket

  Description:  デフォルトパラメータを使ってソケットを作成する．
                inline にすることで、TCP/UDP の判定をコンパイルに
                おける最適化処理時に解決し、必要の無いパラメータテーブルを
                deadstrip にかけることを狙っている．
 *---------------------------------------------------------------------------*/
static inline int SOCL_TcpSocket(void)
{
    return SOCL_CreateSocket(&SOCLSocketParamTCP);
}

static inline int SOCL_UdpSocket(void)
{
    return SOCL_CreateSocket(&SOCLSocketParamUDP);
}

static inline int SOCL_UdpSendSocket(void)
{
    return SOCL_CreateSocket(&SOCLSocketParamUDPSend);
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_Alloc/SOCLi_Free

  Description:  SOCLStartup() で指定された Alloc/Free 関数を呼び出すため
 *---------------------------------------------------------------------------*/
extern CPSConfig        SOCLiCPSConfig;
extern const SOCLConfig*    SOCLiConfigPtr;

static inline void* SOCLi_Alloc(u32 size)
{
    SDK_POINTER_ASSERT(SOCLiConfigPtr);
    SDK_POINTER_ASSERT(SOCLiConfigPtr->alloc);
    return SOCLiConfigPtr->alloc(size);
}

static inline void SOCLi_Free(void* ptr)
{
    SDK_POINTER_ASSERT(SOCLiConfigPtr);
    SDK_POINTER_ASSERT(SOCLiConfigPtr->free);
    SOCLiConfigPtr->free(ptr);
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_GetHostPort

  Description:  自ホストの host port アドレスを取得する
                まだ取得できていななら 0 となる．
 *---------------------------------------------------------------------------*/
static inline u16 SOCL_GetHostPort(int s)
{
    SOCLSocket*     socket = (SOCLSocket*)s;
    SDK_POINTER_ASSERT(socket);
    return (u16) (s ? socket->local_port : 0);
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_SetBlock
  Name:         SOCL_SetNoBlock
  Name:         SOCL_IsBlock
  Name:         SOCL_IsNoBlock

  Description:  ソケットのタイプを取得/設定する
 *---------------------------------------------------------------------------*/
static inline void SOCL_SetBlock(int s)
{
    SOCL_SocketSetBlock((SOCLSocket*)s);
}

static inline void SOCL_SetNoBlock(int s)
{
    SOCL_SocketSetNoBlock((SOCLSocket*)s);
}

static inline BOOL SOCL_IsBlock(int s)
{
    return SOCL_SocketIsBlock((SOCLSocket*)s);
}

static inline BOOL SOCL_IsNoBlock(int s)
{
    return SOCL_SocketIsNoBlock((SOCLSocket*)s);
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_GetReadStatus
  Name:         SOCL_GetWriteStatus

  Description:  ソケットの R/W 状態を取得する
 *---------------------------------------------------------------------------*/
static inline int SOCL_GetReadStatus(int s)
{
    return(SOCL_GetStatus(s) & SOCL_POLLREADSTATUS);
}

static inline int SOCL_GetWriteStatus(int s)
{
    return(SOCL_GetStatus(s) & SOCL_POLLWRITESTATUS);
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_AddRandomSeed

  Description:  SSL で使用する乱数用の種を攪拌する
                OS_GetLowEntropyData 関数の返り値を定期的に与えるべきである
                SOCL の初期化前でも呼び出し可能.
 *---------------------------------------------------------------------------*/
static inline void SOCL_AddRandomSeed(void* seed, u32 len)
{
    CPS_SslAddRandomSeed(seed, len);
}

// SOCLi_IsInitialized
#define SOCLi_IsInitialized()   (NULL != SOCLiConfigPtr)

/*---------------------------------------------------------------------------*
  Name:         SOCLi_CheckCount
  Description:  特定の処理が起こった回数をカウントするためのカウンタ
 *---------------------------------------------------------------------------*/
enum
{
    SOCL_CHECKCOUNT_UDPRCVCB_1  = 0,
    SOCL_CHECKCOUNT_UDPRCVCB_2,
    SOCL_CHECKCOUNT_MAX
};
extern u32 SOCLi_CheckCount[SOCL_CHECKCOUNT_MAX];   // 本体は socl_read.c 内


typedef struct SOCLResource
{
    int numDescTcp;
    int numDescUdp;
} SOCLResource;

extern SOCLResource SOCLi_Resource;

void SOCL_GetResource(SOCLResource* resource);

void SOCLi_InitResource(void);

#ifdef __cplusplus

}
#endif

#endif // NITROWIFI_SOCL_H_
