/*---------------------------------------------------------------------------*
  Project:  RevoEX - demos - share
  File:	 init.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include <revolution/os.h>
#include <revolution/ncd.h>
#include <revolution/so.h>
#include <revolution/net.h>
#include <string.h>
#include <stdlib.h>
#include "rexdemo/netconfig.h"
#include "rexdemo/graphic.h"

/*---------------------------------------------------------------------------*
	変数定義
 *---------------------------------------------------------------------------*/

static u8 NetConfigId = 0;
static char PeerName[256];

/*---------------------------------------------------------------------------*
	内部関数プロトタイプ宣言
 *---------------------------------------------------------------------------*/

BOOL
REXDEMOStartNetwork(BOOL override)
{
	s32 result = 0;
	
	/* NAND に保存されているネットワーク設定を上書きするか？ */
	if (override)
	{
		/* 引数解析結果に従ってネットワーク設定を上書きする(デバッグ専用) */
		result = REXDEMOOverrideNetConfigAuto();
		if (result < NCD_RESULT_SUCCESS)
		{
			REXDEMOError("REXDEMOOverrideNetConfigAuto() failed.(%d)", result);
			return FALSE;
		}
	}

	/* ソケットライブラリに与えるヒープを作成する */
	if (!REXDEMOCreateHeapForSocket( REXDEMO_SOCKET_HEAPSIZE_DEFAULT ))
	{
		REXDEMOError("REXDEMOCreateHeapForSocket() failed.");
		return FALSE;
	}
	/* REXDEMO 内のアロケータを指定してソケットライブラリを初期化する */
	result = SOInit( REXDEMOGetSOLibConfig() );
	if (result == SO_EALREADY)
	{
		REXDEMOError("SOInit() already done. (%d)", result);
	}
	else if (result < SO_SUCCESS)
	{
		REXDEMOError("SOInit() failed.(%d)", result);
		return FALSE;
	}
	
	/* ソケットライブラリのネットワーク機能を開始する */
	result = SOStartup();
	if (result == SO_EALREADY)
	{
		REXDEMOError("SOStartup() already done. (%d)", result);
	}
	else if (result < SO_SUCCESS)
	{
		REXDEMOError("SOStartup() failed.(%d)", result);
		REXDEMOError("Network Error Code is %d", -NETGetStartupErrorCode(result));
		return FALSE;
	}
	
	/* SOStartup は DHCP の解決を待つようになりました */
	
	return TRUE;
}

BOOL
REXDEMOSafeSOFinish( void )
{
	s32 iRetry;
	s32 result;
	BOOL succeeded = TRUE;

	for ( iRetry = REXDEMO_NETWORK_TERMINATION_RETRY_LIMIT; iRetry > 0; iRetry-- )
	{
		result = SOFinish();
		if ( result == SO_EBUSY || result == SO_EAGAIN || result == SO_EINPROGRESS )
		{
			s32 r;
			REXDEMOError("SOFinish() failed.(%d)\n", result);

			/* SOFinish を呼び出せないステートにいるので、SOCleanup を試みる */
			REXDEMOReport( "Try SOCleanup()...\n" );
			r = SOCleanup();
			if ( r < SO_SUCCESS )
			{
				REXDEMOError("SOCleanup() failed.(%d)\n", r);
			}
			/* 他のプロセスのネットワーク関連のリソースの開放を待つ */
			OSSleepMilliseconds( REXDEMO_NETWORK_TERMINATION_RETRY_WAIT );
		}
		else
		{
			break;
		}
	}
	if (result == SO_EALREADY)
	{
		/* SOFinish がすでに呼び出されているか、そもそも SOInit が呼び出されていない */
		/* SOFinish 呼び出し後と同じ状態ではあるので、返り値は TRUE */
		REXDEMOError("SOFinish() already done. (%d)", result);
	}
	else if (result < SO_SUCCESS)
	{
		/* 発生しないはずの想定外の SOFinish のエラー */
		/* 異常系処理が必要 */
		REXDEMOError("SOFinish() failed.(%d)", result);
		succeeded = FALSE;
	}

	if ( iRetry <= 0 )
	{
		/* 再試行上限回数に引っかかった */
		/* ネットワークの終了処理が適切に行えていないので、異常系処理が必要 */
		REXDEMOError("Too many network termination retries; give up to call SOFinish().\n");
		succeeded = FALSE;
	}

	return succeeded;
}

BOOL
REXDEMOEndNetwork( void )
{
	s32 result = 0;
	BOOL succeeded = TRUE;
	
	/* ソケットライブラリのネットワーク機能を停止する */
	result = SOCleanup();
	if (result == SO_EALREADY)
	{
		REXDEMOError("SOCleanup() already done. (%d)", result);
	}
	else if (result < SO_SUCCESS)
	{
		REXDEMOError("SOCleanup() failed.(%d)", result);
		succeeded = FALSE;
	}
	
	/* ソケットライブラリのリソースを開放する */
	if (REXDEMOSafeSOFinish() == FALSE)
	{
		succeeded = FALSE;
	}
	
	/* ソケットライブラリに与えるヒープを解放 */
	REXDEMODestroyHeapForSocket();
	
	return succeeded;
}

/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name		: REXDEMOSetNetConfigId
  Description : 
  Arguments   : configId	 - 
  Returns	 : None.
 *---------------------------------------------------------------------------*/
void
REXDEMOSetNetConfigId( u8 configId )
{
	NetConfigId = configId;
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOGetNetConfigId
  Description : 
  Arguments   : None.
  Returns	 : u8		   - 
 *---------------------------------------------------------------------------*/
u8
REXDEMOGetNetConfigId( void )
{
	return NetConfigId;
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOSetPeerName
  Description : 
  Arguments   : peerName - 
  Returns	 : None.
 *---------------------------------------------------------------------------*/
void
REXDEMOSetPeerName( const char* peerName )
{
	if( peerName != NULL )
	{
		(void)strncpy(PeerName, peerName, sizeof(PeerName)-1);
		PeerName[sizeof(PeerName)-1] = 0;
	}
	else
	{
		(void)memset(PeerName, 0, sizeof(PeerName));
	}
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOGetPeerName
  Description : 
  Arguments   : None.
  Returns	 : const u8*	- 
 *---------------------------------------------------------------------------*/
const char*
REXDEMOGetPeerName( void )
{
	return PeerName;
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOWaitForNCDReady
  Description : NCD が要求を受け付けられる状態になるまで待つ
				この関数で無限ループになる場合はファームウェアが不正である
  Arguments   : None.
  Returns	 : None.
 *---------------------------------------------------------------------------*/
void
REXDEMOWaitForNCDReady( void )
{
	while(NCDGetLinkStatus() == NCD_RESULT_INPROGRESS)
	{
		OSSleepMilliseconds(100);
	}
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOOverrideNetConfig
  Description : NCD のネットワーク設定を強制上書きする。
  Arguments   : pIfConfig	- 
				pIpConfig	- 
  Returns	 : s32		  - 
 *---------------------------------------------------------------------------*/
s32
REXDEMOOverrideNetConfig( const NCDIfConfig* pIfConfig, const NCDIpConfig* pIpConfig )
{
	s32 resultNcd;
	
	REXDEMOWaitForNCDReady();
	
	//////////////////////////////////////////////////////////////////////////
	// ※ 注意 ※
	// NCDSetIfConfig, NCDSetIpConfig はテスト用途の関数です。
	// 製品リリース時には、これらの関数を呼び出さず、
	// SOStartup をただ呼び出すだけにしてください。
	// この場合、本体に設定されたネットワーク設定が利用されます。
	// 開発機材のネットワーク設定は、システムメニューか、
	// RevoEX/RVL/bin/tools/ncdconfigtool.elf で行うことが可能です。
	//////////////////////////////////////////////////////////////////////////
	
	if( pIfConfig != NULL )
	{
		//OSReport("NCDSetIfConfig...\n");
		resultNcd = NCDSetIfConfig( pIfConfig );
		if( resultNcd != NCD_RESULT_SUCCESS )
		{
			OSReport( "NCDSetIfConfig failed (%d)\n", resultNcd );
			return resultNcd;
		}
	}
	
	if( pIpConfig != NULL )
	{
		//OSReport("NCDSetIpConfig...\n");
		resultNcd   = NCDSetIpConfig( pIpConfig );
		if( resultNcd != NCD_RESULT_SUCCESS )
		{
			OSReport( "NCDSetIpConfig failed (%d)\n", resultNcd );
			return resultNcd;
		}
	}
	
	//OSReport("REXDEMOOverrideNetConfig ok\n");
	return NCD_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOOverrideNetConfigAuto
  Description : NCD のネットワーク設定を config.c のネットワーク設定で
				強制上書きする。
				config.c の設定は REXDEMOSetNetConfigId() で切り替えが可能。
  Arguments   : None.
  Returns	 : s32		  - 
 *---------------------------------------------------------------------------*/
s32
REXDEMOOverrideNetConfigAuto( void )
{
	NCDIfConfig ifConfig;
	NCDIfConfig* pIfConfig;
	NCDIpConfig ipConfig;
	NCDIpConfig* pIpConfig;
	u8 configId;
	s32 result;

	configId = REXDEMOGetNetConfigId();
	pIfConfig = NULL;
	pIpConfig = NULL;
	if( REXDEMOCreateIfConfig( &ifConfig, configId ) )
	{
		pIfConfig = &ifConfig;
	}
	if( REXDEMOCreateIpConfig( &ipConfig, configId ) )
	{
		pIpConfig = &ipConfig;
	}

	result = REXDEMOOverrideNetConfig( pIfConfig, pIpConfig );
	if (result < NCD_RESULT_SUCCESS)
	{
		return result;
	}
	return result;
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOWaitForInterfaceReady
  Description : NCD がインターフェースを準備するまで待つ
  Arguments   : msecTimeout  - 
  Returns	 : s32		  - 
 *---------------------------------------------------------------------------*/
s32
REXDEMOWaitForInterfaceReady( u32 msecTimeout )
{
	return REXDEMOWaitForInterfaceReadyEx( msecTimeout, FALSE, NULL );
}

s32
REXDEMOWaitForInterfaceReadyEx( u32 msecTimeout, BOOL wantUp, volatile BOOL* flagCancel )
{
	u32 ms;
	s32 resultNcd;
	for ( ms = 0; msecTimeout == 0 || ms < msecTimeout; ms+=100 )
	{
		if (flagCancel && *flagCancel)
		{
			break;
		}
		switch( resultNcd = NCDGetLinkStatus() )
		{
		case NCD_RESULT_INPROGRESS:
		case NCD_LINKSTATUS_WORKING:
			/* ネットワーク I/F をまだ使用できない状態 */
			break;
		case NCD_LINKSTATUS_NONE:
			/* 一切の通信を許可しないよう設定されている */
			OSReport( "Not allowed to use Network functions\n" );
			return NCD_RESULT_FAILURE;

		case NCD_LINKSTATUS_WIRELESS_DOWN:
			if (wantUp)
			{
				break;
			}
		case NCD_LINKSTATUS_WIRED:
		case NCD_LINKSTATUS_WIRELESS_UP:
			return NCD_RESULT_SUCCESS;
		default:
			/* その他のエラーは通常返されない */
			OSReport( "NCDGetLinkStatus() failure (%d)\n", resultNcd );
			return resultNcd;
		}
		//OSReport("NCDGetLinkStatus() = %d\n", resultNcd);
		OSSleepTicks( OSMillisecondsToTicks( 100 ) );
	}
	return NCD_RESULT_UNDECIDED;
}

/*---------------------------------------------------------------------------*
  Name		: REXDEMOParseArgument
  Description : 
  Arguments   : argc		 - 
  Returns	 : None.
 *---------------------------------------------------------------------------*/
void
REXDEMOParseArgument( int argc, const char* argv[] )
{
	int curr_arg = 1;

	/* configId */
	if( curr_arg < argc )
	{
		BOOL f;
		int i;
		int id;
		const char* arg = argv[curr_arg];

		f = FALSE;
		id = 0;
		for( i = 0; arg[i] != 0 && i < 256; i++ )
		{
			if( '0' <= arg[i] && arg[i] <= '9' )
			{
				f = TRUE;
				id *= 10;
				id += arg[i] - '0';
				if( id >= 256 )
				{
					/* configId は 0～255 */
					f = FALSE;
					break;
				}
			}
			else
			{
				f = FALSE;
				break;
			}
		}

		if( f )
		{
			NetConfigId = (u8)id;
			curr_arg++;
		}
		else
		{
			NetConfigId = 0;
		}
	}

	/* peerName */
	if( curr_arg < argc )
	{
		REXDEMOSetPeerName(argv[curr_arg]);
	}
	else
	{
		REXDEMOSetPeerName(NULL);
	}
}

/*---------------------------------------------------------------------------*
  $Log: netconfig.c,v $
  Revision 1.10  2007/09/07 02:13:31  seiki_masashi
  REXDEMOSafeSOFinish 関数を分離

  Revision 1.9  2007/09/07 00:41:27  seiki_masashi
  ネットワーク終了処理の安全性を向上

  Revision 1.8  2007/06/04 09:40:54  hirose_kazuki
  SO の result が SO_EALREADY の場合分け処理を追加

  Revision 1.7  2007/06/04 05:29:05  seiki_masashi
  エラー発生時の処理の不具合を修正

  Revision 1.6  2007/05/28 08:37:49  hirose_kazuki
  Added REXDEMODestroyHeapForSocket() call.

  Revision 1.5  2007/01/29 07:55:24  terui
  REXDEMOCreatePeerAddressAuto 関数の実体を netfunc.c に移動
  REXDEMOGetPeerAddress 関数の実体を netfunc.c に移動

  Revision 1.4  2006/09/25 07:06:13  seiki_masashi
  configId 0 の場合は NAND 上のネットワーク設定を使用するように変更
  peerName をコマンドライン引数からも取得するように変更

  Revision 1.3  2006/09/25 02:07:07  seiki_masashi
  small fix

  Revision 1.2  2006/09/25 02:00:37  seiki_masashi
  SOStartup の仕様変更に追随
  NCDSetIfConfig に対する注意事項のコメントを追加

  Revision 1.1  2006/08/29 07:19:20  adachi_hiroaki
  プレフィックス変更、そのほか整理

 *---------------------------------------------------------------------------*/
