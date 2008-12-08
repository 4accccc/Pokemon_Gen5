/*---------------------------------------------------------------------------*
  Project:  NET Initialize demo
  File:	 netconfig.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: netconfig.h,v $
  Revision 1.12  2007/09/07 02:13:31  seiki_masashi
  REXDEMOSafeSOFinish 関数を分離

  Revision 1.11  2007/09/07 00:41:38  seiki_masashi
  ネットワーク終了処理の安全性を向上

  Revision 1.10  2007/06/23 12:21:28  seiki_masashi
  Added REXDEMOCreateMEM1Heap, and so on.

  Revision 1.9  2007/05/28 08:37:00  hirose_kazuki
  Added REXDEMODestroyHeapForSocket().

  Revision 1.8  2007/01/29 07:52:19  terui
  SO ライブラリの主要関数非公開化に伴い主要関数を用いる関数宣言を netfunc.h に移動

  Revision 1.7  2006/09/25 07:06:13  seiki_masashi
  configId 0 の場合は NAND 上のネットワーク設定を使用するように変更
  peerName をコマンドライン引数からも取得するように変更

  Revision 1.6  2006/08/29 07:19:20  adachi_hiroaki
  プレフィックス変更、そのほか整理

  Revision 1.5  2006/08/25 10:22:54  adachi_hiroaki
  REXDEMOWaitForInterfaceUp() の名前と仕様を変更

  Revision 1.4  2006/08/25 09:33:01  adachi_hiroaki
  各種ブロック関数に中断フラグを追加

  Revision 1.3  2006/08/25 02:14:48  adachi_hiroaki
  ユーティリティ関数群を追加

  Revision 1.2  2006/08/21 10:57:43  adachi_hiroaki
  NCD の準備待ち処理を追加

  Revision 1.1  2006/08/10 12:09:06  adachi_hiroaki
  ヘッダファイルの位置を変更

  Revision 1.1  2006/08/09 08:46:58  adachi_hiroaki
  ネットワーク共通設定コードを暫定追加

  Revision 1.1  2006/08/08 10:47:28  adachi_hiroaki
  ネットワーク初期化のデモを tests/so から移動


  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef __DEMOS_SHARED_NETCONFIG_H__
#define __DEMOS_SHARED_NETCONFIG_H__

#include <revolution/types.h>
#include <revolution/ncd.h>
#include <revolution/so.h>

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================*/

#include "rexdemo/heap.h"

#define REXDEMO_OVERRIDE_NETCONFIG

#define REXDEMO_NETWORK_TERMINATION_RETRY_LIMIT 120
#define REXDEMO_NETWORK_TERMINATION_RETRY_WAIT  500 // milliseconds


BOOL	REXDEMOStartNetwork( BOOL override );
BOOL	REXDEMOEndNetwork( void );

BOOL	REXDEMOSafeSOFinish( void );


void	REXDEMOParseArgument( int argc, const char* argv[] );

s32	 REXDEMOOverrideNetConfigAuto( void );
s32	 REXDEMOOverrideNetConfig( const NCDIfConfig* pIfConfig, const NCDIpConfig* pIpConfig );
void	REXDEMOWaitForNCDReady( void );
s32	 REXDEMOWaitForInterfaceReady( u32 msecTimeout );
s32	 REXDEMOWaitForInterfaceReadyEx( u32 msecTimeout, BOOL wantUp, volatile BOOL* flagCancel );

void	REXDEMOSetNetConfigId( u8 configId );
u8	  REXDEMOGetNetConfigId( void );
void	REXDEMOSetPeerName( const char* peerName );
const char* REXDEMOGetPeerName( void );

BOOL	REXDEMOCreateIfConfig( NCDIfConfig* pIfConfig, u8 configIdSrc );
BOOL	REXDEMOCreateIpConfig( NCDIpConfig* pIpConfig, u8 configIdSrc );


/*---------------------------------------------------------------------------*/
static inline void
REXDEMOSetInAddr( u8* addr, u8 para0, u8 para1, u8 para2, u8 para3 )
{
	addr[ 0 ]   = para0;
	addr[ 1 ]   = para1;
	addr[ 2 ]   = para2;
	addr[ 3 ]   = para3;
}

/*===========================================================================*/
#ifdef __cplusplus
}
#endif
#endif  /* __DEMOS_SHARED_NETCONFIG_H__ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
