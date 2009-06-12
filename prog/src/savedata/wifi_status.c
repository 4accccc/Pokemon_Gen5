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

#define _POKEMON_NUM   (6)
//トータル189バイト送信できるが多く送ると重くなる

struct _WIFI_STATUS{
  u16 pokemonType[_POKEMON_NUM];                    //12
  u16 hasItemType[_POKEMON_NUM];                   //24
	MYSTATUS aMyStatus;   // MYSTATUS                          56
	u8 VChatMac[6];       // VChatをしたい人のMacアドレス      62
	u8 WifiMode;         // WIFIでのゲーム等の状態             63
	u8 VChatStatus;       // VChatの状態                       64
	u8 Active;          // VChatの状態                         65
  u8 nation;           //  66
  u8 area;             //  67
	u8 dummy[128-67];  // 未来用 今は0   128
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

MYSTATUS* WIFI_STATUS_GetMyStatus(WIFI_STATUS* pStatus)
{
	return &pStatus->aMyStatus;
}


//----------------------------------------------------------
/**
 * @brief	  MYSTATUSポインタを返す
 * @return	,,
 */
//----------------------------------------------------------

extern void WIFI_STATUS_SetMyStatus(WIFI_STATUS* pStatus, const MYSTATUS* pMy)
{
	MyStatus_Copy(pMy,&pStatus->aMyStatus);
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
 * @brief	  WifiModeを返す
 * @return	,,
 */
//----------------------------------------------------------

void WIFI_STATUS_SetWifiMode( WIFI_STATUS* pStatus, u8 mode)
{
	 pStatus->WifiMode = mode;
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
 * @brief	  ボイスチャット許可状態かどうかを設定
 * @return	,,
 */
//----------------------------------------------------------

void WIFI_STATUS_SetVChatStatus(WIFI_STATUS* pStatus, u8 vct)
{
	pStatus->VChatStatus= vct;
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


void WIFI_STATUS_SetMonsNo(WIFI_STATUS* pStatus,int index,u16 no)
{
	pStatus->pokemonType[index] = no;
}

void WIFI_STATUS_SetItemNo(WIFI_STATUS* pStatus,int index,u16 no)
{
	pStatus->hasItemType[index] = no;
}

u16 WIFI_STATUS_GetMonsNo(WIFI_STATUS* pStatus,int index)
{
	return pStatus->pokemonType[index];
}

u16 WIFI_STATUS_GetItemNo(WIFI_STATUS* pStatus,int index)
{
	return pStatus->hasItemType[index];
}


void WIFI_STATUS_SetMyNation(WIFI_STATUS* pStatus,u8 no)
{
	pStatus->nation = no;
}

void WIFI_STATUS_SetMyArea(WIFI_STATUS* pStatus,u8 no)
{
	pStatus->area = no;
}
