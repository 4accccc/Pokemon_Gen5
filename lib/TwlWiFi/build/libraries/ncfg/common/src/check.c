/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NCFG - libraries
  File:     check.c

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
#include <nitroWiFi/ncfg.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部関数プロトタイプ
 *---------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// 機能 : アクセスポイント情報が正しいかチェック
// 引数 : info  - アクセスポイント接続情報
// 返値 : BOOL  - TRUE  : 正 / FALSE : 不正
//-----------------------------------------------------------------------------
BOOL NCFGi_CheckApInfo(const NCFGApInfo* info)
{
	// 設定方法
	if (info->setType == NCFG_SETTYPE_NOT     )  return  TRUE;
	if (info->setType >  NCFG_SETTYPE_RAKURAKU)  return  FALSE;
	// SSID
	if (! NCFGi_CheckSsid(info->ssid[0]))  return  FALSE;
	// IP アドレス
	if (! NCFGi_CheckIpInfo(info))  return  FALSE;

	return  TRUE;
}

//-----------------------------------------------------------------------------
// 機能 : 拡張アクセスポイント情報が正しいかチェック
// 引数 : info  - アクセスポイント接続情報
//        infoEx - 拡張アクセスポイント接続情報
// 返値 : BOOL  - TRUE  : 正 / FALSE : 不正
//-----------------------------------------------------------------------------
BOOL NCFGi_CheckApInfoEx(const NCFGApInfo* info, const NCFGApInfoEx* infoEx)
{
    // 設定方法
    if (info->setType == NCFG_SETTYPE_NOT)
    {
        return  TRUE;
    }
    if (info->setType <= NCFG_SETTYPE_RAKURAKU)
    {
        // 互換設定と同じ形式
        return NCFGi_CheckApInfo(info);
    }

//    if (info->setType != NCFG_SETTYPE_WPS_WEP &&
//          (info->setType < NCFG_SETTYPE_MANUAL_WPA
//         || NCFG_SETTYPE_WPS_WPA < info->setType))
//    {
//        return FALSE;
//    }
    // SSID
    if (! NCFGi_CheckSsid(info->ssid[0]))
    {
        return FALSE;
    }
    // IP アドレス
    if (! NCFGi_CheckIpInfo(info))
    {
        return FALSE;
    }

    // proxy
    if (infoEx->proxyMode > NCFG_PROXY_MODE_NORMAL)
    {
        return FALSE;
    }
    if (infoEx->proxyAuthType > NCFG_PROXY_AUTHTYPE_BASIC)
    {
        return FALSE;
    }

    return  TRUE;
}

static inline isNullAddress(const u8* addr)
{
    return (addr[0] == 0 && addr[1] == 0 && addr[2] == 0 && addr[3] == 0);
}

BOOL NCFGi_CheckIpInfo(const NCFGApInfo* info)
{
	u8   snm[4];

	// IP アドレス
	if (! isNullAddress(info->ip)) {
		// デフォルトゲートウェイ
		if (! NCFGi_CheckAddress(info->gateway))  return  FALSE;
		// サブネットマスク
		if (info->netmask > 32)  return  FALSE;
		// IP アドレス
		NCFGi_ConvMaskAddr(info->netmask, snm);
		if (! NCFGi_CheckIp(info->ip, snm))  return  FALSE;
	}
	// DNS
	if (! isNullAddress(info->dns[0])) {
		if (   (! NCFGi_CheckAddress(info->dns[0]))
			&& (! NCFGi_CheckAddress(info->dns[1])))
		{
			return  FALSE;
		}
	}
	return TRUE;
}

//*****************************************************************************
// 機能 : サブネットマスクを CIDR に変換
// 引数 : mask  - サブネットマスク (アドレス)
// 返値 : u8    - サブネットマスク (CIDR)
//*****************************************************************************
u8  NCFGi_ConvMaskCidr(const u8* mask)
{
	int  snm;
	int  i, j;

	for (i = 0, snm = 0; i < 4; ++ i) {
		for (j = 0; j < 8; ++ j) {
			if ((mask[i] >> j) & 1)  ++ snm;
		}
	}
	return  (u8)snm;
}

//*****************************************************************************
// 機能 : サブネットマスクをアドレスに変換
// 引数 : mask  - サブネットマスク (CIDR)
//        buf   - サブネットマスク (アドレス) 格納バッファ  <<-- 返値
// 返値 :
//*****************************************************************************
void  NCFGi_ConvMaskAddr(int mask, u8* buf)
{
	int  i;
	u32  snm = (0xFFFFFFFF >> mask) ^ 0xFFFFFFFF;

	for (i = 0; i < 4; ++ i) {
		buf[i] = (u8)(snm >> (24 - (8 * i)));
	}
}

//*****************************************************************************
// 機能 : SSID をチェック
// 引数 : ssid  - SSID
// 返値 : BOOL  - TRUE  : 正 / FALSE : 不正
//*****************************************************************************
BOOL  NCFGi_CheckSsid(const u8* ssid)
{
	int  i;

	for (i = 0; i < 0x20; ++ i) {
		if (ssid[i])  return  TRUE;
	}
	return  FALSE;
}

//*****************************************************************************
// 機能 : IP アドレスをチェック
// 引数 : ip    - アドレス
//        mask  - サブネットマスク
// 返値 : BOOL  - TRUE  : 正 / FALSE : 不正
//*****************************************************************************
BOOL  NCFGi_CheckIp(const u8* ip, const u8* mask)
{
	u32  ipa;
	u32  snm;

	if (! NCFGi_CheckAddress(ip))  return  FALSE;
	MI_CpuCopy8(ip  , &ipa, sizeof(u32));
	MI_CpuCopy8(mask, &snm, sizeof(u32));
	if ((ipa |   snm ) == ~1)  return  FALSE;
	if ((ipa & (~snm)) ==  0)  return  FALSE;
	return  TRUE;
}

//*****************************************************************************
// 機能 : アドレスをチェック
// 引数 : address  - アドレス
// 返値 : BOOL     - TRUE  : 正 / FALSE : 不正
//*****************************************************************************
BOOL  NCFGi_CheckAddress(const u8* address)
{
	if (address[0] == 127)  return  FALSE;
	if (address[0] <    1)  return  FALSE;
	if (address[0] >  223)  return  FALSE;
	return  TRUE;
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
