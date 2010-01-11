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
#include "wifi_status.h"


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
 * @brief	  WifiStatusを返す
 * @return	,,
 */
//----------------------------------------------------------

u8 WIFI_STATUS_GetWifiStatus(const WIFI_STATUS* pStatus)
{
	return pStatus->status;
}

//----------------------------------------------------------
/**
 * @brief	  WifiStatusを返す
 * @return	,,
 */
//----------------------------------------------------------

void WIFI_STATUS_SetWifiStatus( WIFI_STATUS* pStatus, u8 mode)
{
	 pStatus->status = mode;
}

//----------------------------------------------------------
/**
 * @brief	  GameModeを返す
 * @return	,,
 */
//----------------------------------------------------------

u8 WIFI_STATUS_GetGameMode(const WIFI_STATUS* pStatus)
{
	return pStatus->GameMode;
}

//----------------------------------------------------------
/**
 * @brief	  GameModeを返す
 * @return	,,
 */
//----------------------------------------------------------

void WIFI_STATUS_SetGameMode( WIFI_STATUS* pStatus, u8 mode)
{
	 pStatus->GameMode = mode;
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
 * @param   自分のWIFI_STATUS
 * @param   比較するWIFI_STATUS
 * @return	,,
 */
//----------------------------------------------------------

BOOL WIFI_STATUS_IsVChatMac(const WIFI_STATUS* pMyStatus, const WIFI_STATUS* pFriendStatus)
{
	if(0==GFL_STD_MemComp(pMyStatus->MyMac,pFriendStatus->VChatMac, 6))
	{
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------
/**
 * @brief	  繋ぎたい人のMACADDRESS封入
 * @param   自分のWIFI_STATUS
 * @param   繋ぎたい人のWIFI_STATUS
 * @return	なし
 */
//----------------------------------------------------------

void WIFI_STATUS_SetVChatMac(WIFI_STATUS* pStatus, const WIFI_STATUS* pFriendStatus)
{
  GFL_STD_MemCopy(pFriendStatus->MyMac,pStatus->VChatMac, 6);
}

//----------------------------------------------------------
/**
 * @brief	  繋ぎたい人のMACADDRESS消去
 * @param   自分のWIFI_STATUS
 * @return	なし
 */
//----------------------------------------------------------

void WIFI_STATUS_ResetVChatMac(WIFI_STATUS* pStatus)
{
  u8 macnull[]={0,0,0,0,0,0};
  GFL_STD_MemCopy(macnull,pStatus->VChatMac, 6);
  OS_TPrintf("WIFI_STATUS_ResetVChatMac\n");
}

//----------------------------------------------------------
/**
 * @brief	  自分のMACADDRESS封入
 * @param   自分のWIFI_STATUS
 * @return	,,
 */
//----------------------------------------------------------

void WIFI_STATUS_SetMyMac(WIFI_STATUS* pStatus)
{
  OS_GetMacAddress(pStatus->MyMac);  
}




void WIFI_STATUS_SetMyNation(WIFI_STATUS* pStatus,u8 no)
{
	pStatus->nation = no;
}

void WIFI_STATUS_SetMyArea(WIFI_STATUS* pStatus,u8 no)
{
	pStatus->area = no;
}
