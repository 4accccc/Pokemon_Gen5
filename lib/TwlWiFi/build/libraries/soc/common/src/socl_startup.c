/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - libraries
  File:     socl_startup.c

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
#include "wcm_cpsif.h"

//---------------------------------------------------------------------------*
//
//  SOCL の Config 値へのポインタの保存
//  SOCL 関数の呼び出し前にここに値が設定する必要がある
//
CPSConfig       SOCLiCPSConfig;
const SOCLConfig*   SOCLiConfigPtr = NULL;
int         SOCLiUDPSendSocket = 0;
int         SOCLiDhcpState = 0;
SOCLInAddr  SOCLiRequestedIP = 0;
u32         SOCLiYieldWait = 0;
u32         SOCLiDefaultLanBufferSize = SOCL_LAN_BUFFER_SIZE_DEFAULT;

//---------------------------------------------------------------------------*
static void SOCLi_StartupCPS(void);
static int  SOCLi_StartupSOCL(void);
static void SOCLi_SetMyIP(void);

#include <nitroWiFi/version.h>
#include <nitro/version_begin.h>
#define MODULE  "WiFi"SDK_WIFI_VERSION_STRING
static char id_string[] = SDK_MIDDLEWARE_STRING("NINTENDO", MODULE);
#include <nitro/version_end.h>

/*---------------------------------------------------------------------------*
  Name:         SOCL_Startup

  Description:  ライブラリを初期化します。
                関数を呼び出す前に SOCLConfig パラメータをライブラリ仕様に
                合わせて初期化してください.

  Arguments:    socl_config     構造体SOCLConfigへのポインタ
                socl_hostip     IP アドレスの設定値
                                NULL なら DHCP で取得する

  Returns:      0  なら正常終了
                -1 ならメモリ不足
 *---------------------------------------------------------------------------*/
int SOCL_Startup(const SOCLConfig* socl_config)
{
    int result;
    SDK_USING_MIDDLEWARE(id_string);
    SDK_ASSERT(socl_config);

    if (SOCLiConfigPtr)
    {
        return 0;
    }

    SOCLi_InitResource();
    SOCLiConfigPtr = socl_config;
    SOCLi_StartupCPS();                     // CPS     サービス起動
    result = SOCLi_StartupSOCL();           // SOCL    サービス起動
    if (result < 0)
    {
        SOCLi_CleanupCPS();
        SOCLiConfigPtr = NULL;
    }
    return result;
}

static int SOCLi_StartupSOCL(void)
{
    int r;

    // Command サービス起動
    r = SOCLi_StartupCommandPacketQueue((int)SOCLiConfigPtr->cmd_packet_max);

    // UDP Send ソケットの作成
    if (r >= 0)
    {
        SOCLiUDPSendSocket = SOCL_UdpSendSocket();
    }

    return r;
}

static void SOCLi_StartupCPS(void)
{
    //
    //  CPS 関数を使用するための設定パラメータ
    //
    CPSConfig*          cps_config = &SOCLiCPSConfig;
    const SOCLConfig*   socl_config = SOCLiConfigPtr;

    SDK_ASSERT(socl_config->alloc);
    SDK_ASSERT(socl_config->free);

    MI_CpuClear8(cps_config, sizeof(CPSConfig));
    cps_config->alloc = socl_config->alloc;
    cps_config->free = socl_config->free;
    cps_config->link_is_on = SOCL_LinkIsOn; // 物理リンクの状態を返す関数
    cps_config->random_seed = 0;            // 0 なら乱数の種を OS_Tick から取得
    cps_config->yield_wait = SOCLiYieldWait;    // CPS ライブラリの yield 時の sleep 時間

    // socl_config->lan_buffer_size が 0    なら default 値を設定
    // socl_config->lan_buffer      が NULL なら alloc で自力確保する
    if (socl_config->lan_buffer_size)
    {
        cps_config->lan_buflen = socl_config->lan_buffer_size;
    }
    else
    {
        cps_config->lan_buflen = SOCLiDefaultLanBufferSize;
    }

    if (socl_config->lan_buffer)
    {
        cps_config->lan_buf = socl_config->lan_buffer;
    }
    else
    {
        cps_config->lan_buf = SOCLi_Alloc(cps_config->lan_buflen);
    }

    // MTU/MSS の設定
    //  socl_config->mtu が 0 なら default 値を設定
    {
        s32 mtu, mss, rwin;

        mtu = socl_config->mtu ? socl_config->mtu : SOCL_MTU_SIZE_DEFAULT;
        mtu = MATH_CLAMP(mtu, SOCL_MTU_SIZE_MIN, SOCL_MTU_SIZE_MAX);
        rwin = socl_config->rwin ? socl_config->rwin : SOCL_RWIN_SIZE_DEFAULT;
        mss = SOCLi_MTUtoMSS(mtu);

        cps_config->mymss = (u32) mss;
        SOCLSocketParamTCP.buffer.rcvbuf_size = (u16) rwin;
        SOCLSocketParamTCP.buffer.rcvbuf_consume_min = (u16) (mss*2);
    }

    // HostIP の設定 hostip が 0 なら DHCP を使用する
    // DHCP が引けない場合、SOCLiDhcpState で判定できる
    CPSMyIp = 0x00000000;

    if (socl_config->use_dhcp)
    {
        SOCLiDhcpState = SOCL_DHCP_REQUEST;
        cps_config->mode = 0;
        cps_config->dhcp_callback = SOCLi_DhcpTimeout;
        cps_config->requested_ip = SOCLiRequestedIP;
    }
    else
    {
        SOCLiDhcpState = 0;
        cps_config->mode = CPS_DONOTUSE_DHCP;
        cps_config->dhcp_callback = SOCLi_SetMyIP;
    }

    // CPS スレッドの優先度
    CPS_SetThreadPriority(socl_config->cps_thread_prio ? socl_config->cps_thread_prio : (u32) SOCL_CPS_SOCKET_THREAD_PRIORITY);

    WCM_SetRecvDCFCallback(CPSi_RecvCallbackFunc);

    {   // 非同期 Cleanup に対する一時的な対策
        extern void CPS_SetScavengerCallback(void (*f) (void));
        CPS_SetScavengerCallback(SOCLi_TrashSocket);
    }
    CPS_SetCheckNConfigCallback(SOCLi_CheckNConfig);
#ifdef SDK_MY_DEBUG
    OS_TPrintf("CPS_Startup\n");
#endif
    CPS_Startup(cps_config);

    return;
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_SetMyIPinConfig

  Description:  SOCLConfig 内に登録されている IP アドレスフィールドの書き換え

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void SOCLi_SetMyIPinConfig(SOCLInAddr ip, SOCLInAddr mask, SOCLInAddr gateway)
{
    SOCLConfig*     config = (SOCLConfig*)SOCLiConfigPtr;

    if (config)
    {
        config->host_ip.my_ip = ip;
        config->host_ip.net_mask = mask;
        config->host_ip.gateway_ip = gateway;
    }

    return;
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_SetMyIP

  Description:  IP アドレスの手動設定

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void SOCLi_SetMyIP(void)
{
    const SOCLConfig*   config = SOCLiConfigPtr;

    SDK_ASSERT(config);
    CPSMyIp = SOCL2CPSInAddr(config->host_ip.my_ip);
    CPSNetMask = SOCL2CPSInAddr(config->host_ip.net_mask);
    CPSGatewayIp = SOCL2CPSInAddr(config->host_ip.gateway_ip);
    CPSDnsIp[0] = SOCL2CPSInAddr(config->host_ip.dns_ip[0]);
    CPSDnsIp[1] = SOCL2CPSInAddr(config->host_ip.dns_ip[1]);
    SOCLiDhcpState |= SOCL_DHCP_CALLBACK;
    return;
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_DhcpTimeout

  Description:  DHCP の Timeout のコールバック

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void SOCLi_DhcpTimeout(void)
{
    SOCLiDhcpState |= (SOCL_DHCP_CALLBACK | SOCL_DHCP_ERROR);
    return;
}

/*---------------------------------------------------------------------------*
  Name:         SOCL_LinkIsOn

  Description:  物理リンクの接続判定

  Arguments:    なし

  Returns:      接続しているなら TRUE
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL BOOL SOCL_LinkIsOn(void)
{
    return WCM_GetApMacAddress() != NULL;
}
