/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - libraries
  File:     socl_misc.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-06#$
  $Rev: 1087 $
  $Author: matsuoka_masayoshi $
 *---------------------------------------------------------------------------*/
#include <nitroWiFi/soc.h>

struct errortable
{
    int code;
    const char*     name;
};

static struct errortable    errtable[] =
{
#include "errtable.h"
};

SOCLResource SOCLi_Resource;

/*---------------------------------------------------------------------------*
  Name:         SOCL_GetErrName

  Description:  SOCL_EXXXX のエラーコード名を取得する

  Arguments:    エラーコード
  
  Returns:      エラーコードの Enum 名
 *---------------------------------------------------------------------------*/
const char* SOCL_GetErrName(int errcode)
{
    int i;

    for (i = 0; i < sizeof(errtable) / sizeof(errtable[0]); i++)
    {
        if (errtable[i].code == errcode)
        {
            return errtable[i].name;
        }
    }

    return "";
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_ROUNDUP4/SOCLi_RoundUp4
  
  Description:  4 の倍数に切り上げる
                コードサイズを減らすためにマクロと関数の2つを用意している
                size が定数の場合はマクロの方を使いコンパイラの最適化を期
                待する方がよい
  
  Arguments:    size     切り上げたい値
  
  Returns:      size を 4 に切り上げた数
 *---------------------------------------------------------------------------*/
u32 SOCLi_RoundUp4(u32 size)
{
    return SOCLi_ROUNDUP4(size);
}

void SOCL_GetResource(SOCLResource* resource)
{
    *resource = SOCLi_Resource;
}

void SOCLi_InitResource(void)
{
    MI_CpuClear8(&SOCLi_Resource, sizeof(SOCLi_Resource));
}

/*---------------------------------------------------------------------------*
  Name:         SOCLi_CheckNConfig
  
  Description:  ホストIPアドレス、サブネットマスク、ゲートウェイアドレス
                DNSサーバアドレスに不正がないか確認する。

  Arguments:    なし(CPSのグローバル変数で定義されるアドレスをチェックする)
  
  Returns:      1：不正はない
                0：不正があった
 *---------------------------------------------------------------------------*/

static int SOCLi_IsIPv4ValidDNSAddr( u32 netmask, u32 dns, u32 addr )
{
    return  (
                ( 0xffffffff != dns )                       /* ディレクティッドブロードキャスト*/
            &&  ( 0 != dns )                                /* 0 */
            &&  ( addr != dns )                             /* ホストアドレスと同じ */
            &&  ( ((addr^dns)&netmask) || (dns&~netmask) )  /* ネットワークアドレス */
            &&  ( ((addr^dns)&netmask) || ((dns&~netmask)^~netmask))/* リミテッドブロードキャスト */
            &&  ( dns&0xff000000^0x7f000000)                /* ループバック 127.*.*.* */
            &&  ( dns&0xf0000000^0xe0000000));              /* マルチキャストアドレス */
}

static int SOCLi_IsIPv4ValidNodeAddr( u32 netmask, u32 addr )
{
    return (
//              ( netmask == 0xffffffff )  ||               /* 単一ホスト */
                (   (addr&~netmask^~netmask)                /* ブロードキャスト */
//              &&  ( netmask != 0xfffffffe )               /* Ponit to Point */ 
                &&  (addr&~netmask)                         /* ネットワークそのもの */
                &&  (addr&0xff000000^0x7f000000)            /* ループバック 127.*.*.* */
                &&  (addr&0xf0000000^0xe0000000))           /* マルチキャストアドレス */
           );
}

int SOCLi_CheckNConfig( void )
{
	if( ( 0x00000000 == CPSNetMask) || (( ~CPSNetMask & ( ~CPSNetMask + 1 ) ) != 0 ) )
	{
		return 0;
	}

	if( ! SOCLi_IsIPv4ValidNodeAddr( CPSNetMask, CPSMyIp ) )
	{
		return 0;
	}

	if (    ( 0 != CPSGatewayIp )
        &&  ( ! SOCLi_IsIPv4ValidNodeAddr( CPSNetMask, CPSGatewayIp )
        ||  ( ( CPSNetMask != 0xffffffff ) && ( ( CPSMyIp & CPSNetMask ) != ( CPSGatewayIp & CPSNetMask ) ) )
        ||  ( CPSMyIp == CPSGatewayIp ) ) )
	{
		CPSGatewayIp = 0;
	}

    if( !SOCLi_IsIPv4ValidDNSAddr( CPSNetMask, CPSDnsIp[0], CPSMyIp ) )
    {
        CPSDnsIp[0] = 0;
    }

    if( !SOCLi_IsIPv4ValidDNSAddr( CPSNetMask, CPSDnsIp[1], CPSMyIp ) )
    {
        CPSDnsIp[1] = 0;
    }
	return 1;
}
