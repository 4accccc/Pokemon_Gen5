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
  DWCAccFriendData aFriendData[WIFI_NEGOTIATION_DATAMAX];
  int count;
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
 * @param   DWCAccFriendData  フレンドデータのポインタ
 * @return	type  パネルタイプ CGEAR_PANELTYPE_ENUM
 */
//--------------------------------------------------------------------------------------------
void WIFI_NEGOTIATION_SV_SetFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,DWCAccFriendData* pFriendData)
{
  GFL_STD_MemCopy(pFriendData, &pSV->aFriendData[ pSV->count ], sizeof(DWCAccFriendData));
  pSV->count++;
  if(pSV->count >= WIFI_NEGOTIATION_DATAMAX){
    pSV->count=0;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   Wi-Fiネゴシエーション用ともだちデータを引き出す
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   index インデックス WIFI_NEGOTIATION_DATAMAXまで
 * @return	none
 */
//--------------------------------------------------------------------------------------------
DWCAccFriendData* WIFI_NEGOTIATION_SV_GetFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,int index)
{
  GF_ASSERT(index < WIFI_NEGOTIATION_DATAMAX);
  if(index >= WIFI_NEGOTIATION_DATAMAX){
    return &pSV->aFriendData[0];
  }
  return &pSV->aFriendData[ index ];
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

