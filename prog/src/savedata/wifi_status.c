//============================================================================================
/**
 * @file	  wifi_status.c
 * @brief	  WIFIに流れる情報の管理
            セーブするデータではないが mystatus構造体の内包のためここにある
 * @author	k.ohno
 * @date	  2009.6.6
 */
//============================================================================================

#include "gflib.h"
#include "savedata/wifi_status.h"
#include "mystatus_local.h"


struct _WIFI_STATUS{
	MYSTATUS aMyStatus;   // MYSTATUS                          32
	u8 VChatMac[6];       // VChatをしたい人のMacアドレス      38
	u8 WifiMode;         // WIFIでのゲーム等の状態             39
	u8 VChatStatus;       // VChatの状態                       40
	u8 Active;          // VChatの状態                         41
	u8 dummy[64-41];  // 未来用 今は0
};

//----------------------------------------------------------
/**
 * @brief	  構造体サイズを返す
 * @return	int		サイズ（バイト単位）
 */
//----------------------------------------------------------

int WIFI_STATUS_GetSize(void)
{
	return sizeof(WIFI_STATUS);

}
//----------------------------------------------------------
/**
 * @brief	  MYSTATUSポインタを返す
 * @return	,,
 */
//----------------------------------------------------------

const MYSTATUS* WIFI_STATUS_GetMyStatus(const WIFI_STATUS* pStatus)
{
	return &pStatus->aMyStatus;
}

//----------------------------------------------------------
/**
 * @brief	  WifiModeを返す
 * @return	,,
 */
//----------------------------------------------------------

u8 WIFI_STATUS_GetWifiMode(const WIFI_STATUS* pStatus)
{
	return pStatus->WifiMode;
}

//----------------------------------------------------------
/**
 * @brief	  ボイスチャット許可状態かどうかを返す
 * @return	,,
 */
//----------------------------------------------------------

u8 WIFI_STATUS_GetVChatStatus(const WIFI_STATUS* pStatus)
{
	return pStatus->VChatStatus;
}

//----------------------------------------------------------
/**
 * @brief	  ゲーム中で他の人を受け入れられないかどうかを返す
 * @return	,,
 */
//----------------------------------------------------------

u8 WIFI_STATUS_GetActive(const WIFI_STATUS* pStatus)
{
	return pStatus->Active;
}

//----------------------------------------------------------
/**
 * @brief	  この人とVCHATしたいというMACアドレスが一致してるかどうかを返す
 * @param   比較されるWIFI_STATUS
 * @param   比較するMACアドレス
 * @return	,,
 */
//----------------------------------------------------------

BOOL WIFI_STATUS_IsVChatMac(const WIFI_STATUS* pStatus, const u8* SearchMacAddress)
{
	if(0==GFL_STD_MemComp(SearchMacAddress,pStatus->VChatMac, 6))
	{
		return TRUE;
	}
	return FALSE;
}


