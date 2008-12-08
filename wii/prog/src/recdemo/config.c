/*---------------------------------------------------------------------------*
  Project:  NET Initialize demo
  File:	 config.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: config.c,v $
  Revision 1.6  2007/08/01 09:36:29  kitase_hirotake
  TAB -> SPACE

  Revision 1.5  2007/01/29 07:54:31  terui
  REXDEMOCreatePeerAddress 関数の実体を peer.c に移動

  Revision 1.4  2006/09/25 07:06:13  seiki_masashi
  configId 0 の場合は NAND 上のネットワーク設定を使用するように変更
  peerName をコマンドライン引数からも取得するように変更

  Revision 1.3  2006/08/29 07:19:20  adachi_hiroaki
  プレフィックス変更、そのほか整理

  Revision 1.2  2006/08/24 00:43:18  adachi_hiroaki
  , が抜けていたのを修正

  Revision 1.1  2006/08/24 00:18:57  adachi_hiroaki
  公開用設定コードを追加


  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <revolution/ncd.h>
#include <revolution/so.h>
#include <string.h>		/* for memset() */
#include "rexdemo/netconfig.h"

/*---------------------------------------------------------------------------*/

BOOL
REXDEMOCreateIfConfig( NCDIfConfig* pIfConfig, u8 configId )
{
	(void)memset( pIfConfig, 0, sizeof( NCDIfConfig ) );
	switch( configId )
	{
	case 0:
		/* 本体の NAND 型フラッシュメモリに書かれたネットワーク設定を利用する */
		return FALSE;

	case 1:
		{
			pIfConfig->selectedMedia		= NCD_IF_SELECT_WIRED;
#ifdef  NCD_WIRED_LINKTYPE_SUPPORT
			pIfConfig->netif.wired.linkType = NCD_WIRED_LINKTYPE_AUTO;
#endif
			break;
		}
	case 2:
		{
#define	 SSID1   "TESTSSID"
			pIfConfig->selectedMedia	= NCD_IF_SELECT_WIRELESS;
			pIfConfig->netif.wireless.rateset	   = 0;	/* 自動 */
			pIfConfig->netif.wireless.retryLimit	= 0;	/* 自動 */
			pIfConfig->netif.wireless.configMethod  = NCD_CONFIG_METHOD_MANUAL;
			{
				NCDApConfig*	pApConfig   = &( pIfConfig->netif.wireless.config.manual );

				(void)strcpy( (char*)( pApConfig->ssid ), SSID1 );
				pApConfig->ssidLength   = (u16)strlen( SSID1 );
				pApConfig->privacy.mode = NCD_PRIVACY_MODE_WEP104;
				pApConfig->privacy.wep104.keyId = 0;
				(void)memcpy( pApConfig->privacy.wep104.key[0], "wepkey_WEPKEY", 13 );
			}
		}
		break;
	default:
		OSReport("Invalid configuration Id.(%d)", configId);
		OSHalt("Exiting...");
	}

	return TRUE;
}

/*---------------------------------------------------------------------------*/
BOOL
REXDEMOCreateIpConfig( NCDIpConfig* pIpConfig, u8 configId )
{
#define		MTU		1500

	(void)memset( pIpConfig, 0, sizeof( NCDIpConfig ) );
	switch( configId )
	{
	case 0:
		/* 本体の NAND 型フラッシュメモリに書かれたネットワーク設定を利用する */
		return FALSE;

	case 1:
		{
			pIpConfig->useDhcp  = TRUE;
		}
	case 2:
		{
			pIpConfig->useDhcp  = FALSE;
			pIpConfig->useProxy = FALSE;
			REXDEMOSetInAddr( pIpConfig->ip.addr,	 192, 168,   0, 101 );
			REXDEMOSetInAddr( pIpConfig->ip.netmask,  255, 255, 255,   0 );
			REXDEMOSetInAddr( pIpConfig->ip.gateway,  192, 168,   0,   1 );
			REXDEMOSetInAddr( pIpConfig->ip.dns1,	 192, 168,   0,   1 );
			REXDEMOSetInAddr( pIpConfig->ip.dns2,	   0,   0,   0,   0 );
			break;
		}
	default:
		OSReport("Invalid configuration Id.(%d)", configId);
		OSHalt("Exiting...");
	}

	pIpConfig->adjust.maxTransferUnit	   = MTU;
	pIpConfig->adjust.tcpRetransTimeout	 = 100;
	pIpConfig->adjust.dhcpRetransCount	  = 4;

	return TRUE;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
