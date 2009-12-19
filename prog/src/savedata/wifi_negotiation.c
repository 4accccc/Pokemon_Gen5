//============================================================================================
/**
 * @file	  wifi_negotiation.c
 * @brief	  WIFIネゴシエーション用ともだち履歴データ
 * @author	k.ohno
 * @date	  2009.10.16
 */
//============================================================================================

#include "gflib.h"

#include "savedata/save_tbl.h"
#include "savedata/wifi_negotiation.h"

//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _WIFI_NEGOTIATION_SAVEDATA{
  s32 aFriendData[WIFI_NEGOTIATION_DATAMAX];
  u32 count;
};


//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体サイズを得る
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------
int WIFI_NEGOTIATION_SV_GetWorkSize(void)
{
	return sizeof(WIFI_NEGOTIATION_SAVEDATA);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体を得る
 * @param   HEAPID
 * @return	構造体
 */
//--------------------------------------------------------------------------------------------

WIFI_NEGOTIATION_SAVEDATA* WIFI_NEGOTIATION_SV_AllocWork(HEAPID heapID)
{
	return GFL_HEAP_AllocClearMemory(heapID, WIFI_NEGOTIATION_SV_GetWorkSize());
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   中身を初期化する
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------

void WIFI_NEGOTIATION_SV_Init(WIFI_NEGOTIATION_SAVEDATA* pSV)
{
	GFL_STD_MemClear(pSV, WIFI_NEGOTIATION_SV_GetWorkSize());
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   Wi-Fiネゴシエーション用ともだちデータをループ保存
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   profileID  ProfileID
 * @return	type  パネルタイプ CGEAR_PANELTYPE_ENUM
 */
//--------------------------------------------------------------------------------------------
void WIFI_NEGOTIATION_SV_SetFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,s32 profileID)
{
  if(pSV->count >= WIFI_NEGOTIATION_DATAMAX){
    pSV->count=0;
  }
  pSV->aFriendData[ pSV->count ] = profileID;
  pSV->count++;  //繰り返し保存
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   Wi-Fiネゴシエーション用ともだちデータを引き出す
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   index インデックス WIFI_NEGOTIATION_DATAMAXまで
 * @return	profileID
 */
//--------------------------------------------------------------------------------------------
s32 WIFI_NEGOTIATION_SV_GetFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,u32 index)
{
  GF_ASSERT(index < WIFI_NEGOTIATION_DATAMAX);
  if(index >= WIFI_NEGOTIATION_DATAMAX){
    return pSV->aFriendData[0];
  }
  return pSV->aFriendData[ index ];
}

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
WIFI_NEGOTIATION_SAVEDATA* WIFI_NEGOTIATION_SV_GetSaveData(SAVE_CONTROL_WORK* pSave)
{
	WIFI_NEGOTIATION_SAVEDATA* pData;
	pData = SaveControl_DataPtrGet(pSave, GMDATA_ID_WIFI_NEGOTIATION);
	return pData;

}

