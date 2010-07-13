//=============================================================================
/**
 * @file	net_dev.c
 * @bfief	通信デバイス選択
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	08/12/19
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "net/network_define.h"
#include "net/net_dev.h"

// 通信GFLIBは通信デバイスを持っていません
// それはGFLIBに持ってしまうと、余計にライブラリサイズを増やしてしまう為です
// このファイルはオーバーレイしないでください。
// また、この関数はGFL_NET_Initで勝手に呼ばれますので、必ず宣言してください。


//対応する通信は1 しない場合0
#ifndef MULTI_BOOT_MAKE

#define SUPPORT_WIFI_  (1)   // WIFI通信
#define SUPPORT_WIRELESS_  (1)   // ワイヤレス通信
#define SUPPORT_IRC_  (1)   // IRC通信
#define SUPPORT_OVERLAY_ (1)  //オーバーレイを行う場合

#else

#define SUPPORT_WIFI_  (0)   // WIFI通信
#define SUPPORT_WIRELESS_  (1)   // ワイヤレス通信
#define SUPPORT_IRC_  (0)   // IRC通信
#define SUPPORT_OVERLAY_ (0)  //オーバーレイを行う場合

#endif //MULTI_BOOT_MAKE


extern GFLNetDevTable *NET_GetIrcDeviceTable(void);
extern GFLNetDevTable *NET_GetWirelessDeviceTable(void);
extern GFLNetDevTable *NET_GetWifiDeviceTable(void);

//SRCS_OVERLAY_NOTWIFI

FS_EXTERN_OVERLAY(notwifi);
FS_EXTERN_OVERLAY(intrude_system);
FS_EXTERN_OVERLAY(dev_wifilib);
FS_EXTERN_OVERLAY(dev_wifi);
FS_EXTERN_OVERLAY(dev_wireless);
FS_EXTERN_OVERLAY(dev_irc);
FS_EXTERN_OVERLAY(wifilobby_common);
FS_EXTERN_OVERLAY(dpw_common);


#ifdef PM_DEBUG
#include "net_alone_test.h"
//net_whpipe.cでしか使用しないが、オーバーレイされるsrcの為、値が消えてしまう
//消されないように常駐であるこのファイルに定義だけ行う
u8 DebugAloneTest = _DEBUG_ALONETEST;
//こっちはwih.c
u8 WH_DebugFixedChannel = 0;
#endif


//------------------------------------------------------------------------------
/**
 * @brief   ワイヤレスデバイスをロードする
 * @param   deviceNo  デバイス番号
 * @retval  GFLNetDevTable*
 */
//------------------------------------------------------------------------------
GFLNetDevTable* NET_DeviceLoad(int deviceNo)
{
  switch(deviceNo){
#if SUPPORT_WIFI_	
	case GFL_NET_TYPE_WIFI_GTS:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( notwifi ) );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( intrude_system ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wifilib ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wifi ) );
		GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common) );
    return NET_GetWifiDeviceTable();
  case GFL_NET_TYPE_WIFI:
#if SUPPORT_OVERLAY_
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( notwifi ) );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( intrude_system ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wifilib ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wifi ) );
#endif
    return NET_GetWifiDeviceTable();
#endif
#if SUPPORT_WIFI_
  case GFL_NET_TYPE_WIFI_LOBBY:
#if SUPPORT_OVERLAY_
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( notwifi ) );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( intrude_system ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wifilib ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wifi ) );
    //        GFL_OVERLAY_Load( FS_OVERLAY_ID( wifilobby_common ) );
#endif
    return NET_GetWifiDeviceTable();
#endif
#if SUPPORT_WIRELESS_
  case GFL_NET_TYPE_WIRELESS:
  case GFL_NET_TYPE_WIRELESS_SCANONLY:
#if SUPPORT_OVERLAY_
    GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_wireless ) );
#endif
    return NET_GetWirelessDeviceTable();
#endif
#if SUPPORT_IRC_
  case GFL_NET_TYPE_IRC:
  case GFL_NET_TYPE_IRC_WIRELESS:
#if SUPPORT_OVERLAY_
    GFL_OVERLAY_Load( FS_OVERLAY_ID( dev_irc ) );
#endif
    return NET_GetIrcDeviceTable();
#endif
  default:
    //        GF_ASSERT_HEAVY(0);
    break;
  }
  return NULL;
}

//------------------------------------------------------------------------------
/**
 * @brief   ワイヤレスデバイスをアンロードする
 * @param   deviceNo  デバイス番号
 * @retval  none
 */
//------------------------------------------------------------------------------
void NET_DeviceUnload(int deviceNo)
{
  switch(deviceNo){
#if SUPPORT_WIFI_
	case GFL_NET_TYPE_WIFI_GTS:
#if SUPPORT_OVERLAY_
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( dpw_common ) );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_wifilib ) );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_wifi ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( notwifi ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( intrude_system ) );
#endif
		break;
  case GFL_NET_TYPE_WIFI:
#if SUPPORT_OVERLAY_
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_wifilib ) );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_wifi ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( notwifi ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( intrude_system ) );
#endif
    break;
#endif	//SUPPORT_WIFI_
#if SUPPORT_WIFI_
  case GFL_NET_TYPE_WIFI_LOBBY:
#if SUPPORT_OVERLAY_
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_wifilib ) );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_wifi ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( notwifi ) );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( intrude_system ) );
    //        GFL_OVERLAY_Unload( FS_OVERLAY_ID( wifilobby_common ) );
#endif
    break;
#endif
#if SUPPORT_WIRELESS_
  case GFL_NET_TYPE_WIRELESS:
  case GFL_NET_TYPE_WIRELESS_SCANONLY:
#if SUPPORT_OVERLAY_
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_wireless ) );
#endif
    break;
#endif
#if SUPPORT_IRC_
  case GFL_NET_TYPE_IRC:
  case GFL_NET_TYPE_IRC_WIRELESS:
#if SUPPORT_OVERLAY_
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( dev_irc ) );
#endif
    break;
#endif
  default:
    //        GF_ASSERT_HEAVY(0);
    break;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   通信メイン関数のラップ関数
 */
//------------------------------------------------------------------------------
#if 0  //こちらはパレスの調整が必要
static u8 bNotCall;        //通信を呼び出さない場合にTRUE
static u8 bFlip;  //DS通信の切り替えフラグ

void NET_DEV_Main(void)
{
  if( WB_NET_PALACE_SERVICEID==GFL_NET_GetGameServiceID() ){
    if(bNotCall==FALSE){
      if(bFlip==FALSE){
        bFlip =GFL_NET_Main();
      }
      else{
        bFlip=FALSE;
      }
    }
    else{
      bNotCall = FALSE;
    }
  }
  else{
    GFL_NET_Main();
  }
}


void NET_DEV_SetSkipNetMain(void)
{
  bNotCall = TRUE;
}

#else
//------------------------------------------------------------------------------
/**
 * @brief   通信メイン関数のラップ関数
 */
//------------------------------------------------------------------------------

static u8 bFlip=FALSE;

void NET_DEV_Main(void)
{
  if( WB_NET_PALACE_SERVICEID==GFL_NET_GetGameServiceID() ){
    if(bFlip==FALSE){
      bFlip =GFL_NET_Main();
    }
    else{
      bFlip=FALSE;
    }
  }
  else{
    GFL_NET_Main();
  }
}



#endif

//------------------------------------------------------------------------------
/**
 * @brief   赤外線系の通信かどうか（中では無線だったりする）
 */
//------------------------------------------------------------------------------

BOOL NET_DEV_IsIrcMode(void)
{
  switch(GFL_NET_GetGameServiceID()){
  case WB_NET_IRCTRADE:
  case WB_NET_IRC_BATTLE:
  case WB_NET_IRCBATTLE:
  case WB_NET_IRCCOMPATIBLE:
  case WB_NET_IRCAURA:
  case WB_NET_IRCRHYTHM:
  case WB_NET_IRCMENU:
  case WB_NET_IRCBATTLE_MULTI:
  case WB_NET_IRCBATTLE_MULTI_IRC:
  case WB_NET_IRC_REGULATION:
    return TRUE;
    break;
  }
  return FALSE;
}

