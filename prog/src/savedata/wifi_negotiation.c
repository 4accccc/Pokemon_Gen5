//============================================================================================
/**
 * @file	  wifi_negotiation.c
 * @brief	  WIFIネゴシエーション用ともだち履歴データ
            国連と共通
 * @author	k.ohno
 * @date	  2009.10.16
 */
//============================================================================================

#include "gflib.h"

#include "savedata/save_tbl.h"
#include "savedata/wifi_negotiation.h"
#include "buflen.h"

//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _WIFI_NEGOTIATION_SAVEDATA{
  s32 aFriendData[WIFI_NEGOTIATION_DATAMAX];
  STRCODE name[WIFI_NEGOTIATION_DATAMAX][PERSON_NAME_SIZE + EOM_SIZE];		// 16
  s32 dummyUnitedNations[WIFI_NEGOTIATION_DATAMAX];  //@todo国連用仮確保
  s32 dummyUnitedNations2[WIFI_NEGOTIATION_DATAMAX];  //@todo国連用仮確保
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
  if(profileID==0){
    return;
  }
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


//--------------------------------------------------------------------------------------------
/**
 * @brief   渡したプロファイルIDに一致する物があるかどうかさがす
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   profile 検査profileID
 * @return	一致したらTRUE
 */
//--------------------------------------------------------------------------------------------
BOOL WIFI_NEGOTIATION_SV_IsCheckFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,s32 profile)
{
  int i;

  for(i=0;i<WIFI_NEGOTIATION_DATAMAX;i++){
    if(pSV->aFriendData[ i ] == profile){
      return TRUE;
    }
  }
  return FALSE;
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

