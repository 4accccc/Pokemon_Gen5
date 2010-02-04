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
#include "savedata/mystatus_local.h"
#include "system/pms_data.h"
#include "buflen.h"

//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _WIFI_NEGOTIATION_SAVEDATA{
  MYSTATUS aMyStatus[WIFI_NEGOTIATION_DATAMAX];
  u16 date[WIFI_NEGOTIATION_DATAMAX];         //交換日時
  PMS_DATA message;   //簡易文 自己紹介文
  u16 count;   //データ循環の為のカウンタ
  u16 num;  //自分の交換ポケモン数
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
 * @param   pSV       WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   pMyStatus MYSTATUS
 */
//--------------------------------------------------------------------------------------------
void WIFI_NEGOTIATION_SV_SetFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,const MYSTATUS* pMyStatus)
{
  int profileID = MyStatus_GetProfileID(pMyStatus);

  if(profileID==0){
    return;
  }
  if(WIFI_NEGOTIATION_SV_IsCheckFriend(pSV,profileID)){
    return;
  }
  if(pSV->count >= WIFI_NEGOTIATION_DATAMAX){
    pSV->count = 0;
  }
  MyStatus_Copy(pMyStatus, &pSV->aMyStatus[pSV->count]);
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
  int i = index;
  GF_ASSERT(index < WIFI_NEGOTIATION_DATAMAX);

  if(index >= WIFI_NEGOTIATION_DATAMAX){
    i=0;
  }
  return MyStatus_GetProfileID(&pSV->aMyStatus[ i ]);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   Wi-Fiネゴシエーション用MYSTATUSを引き出す
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   index インデックス WIFI_NEGOTIATION_DATAMAXまで
 * @return	profileID
 */
//--------------------------------------------------------------------------------------------
MYSTATUS* WIFI_NEGOTIATION_SV_GetMyStatus(WIFI_NEGOTIATION_SAVEDATA* pSV,u32 index)
{
  int i = index;
  GF_ASSERT(index < WIFI_NEGOTIATION_DATAMAX);

  if(index >= WIFI_NEGOTIATION_DATAMAX){
    return NULL;
  }
  if(0==WIFI_NEGOTIATION_SV_GetFriend(pSV,index)){
    return NULL;
  }
  return &pSV->aMyStatus[ i ];
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
    if(MyStatus_GetProfileID(&pSV->aMyStatus[ i ]) == profile){
      return TRUE;
    }
  }
  return FALSE;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   保存番号を返す
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @return	順番
 */
//--------------------------------------------------------------------------------------------
int WIFI_NEGOTIATION_SV_GetCount(WIFI_NEGOTIATION_SAVEDATA* pSV)
{
  return pSV->count;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   渡したプロファイルIDに一致する物があるかどうかさがす
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   profile 検査profileID
 * @return	一致したらTRUE
 */
//--------------------------------------------------------------------------------------------
#if 0
void WIFI_NEGOTIATION_SV_GetDate(WIFI_NEGOTIATION_SAVEDATA* pSV,RTCDate * date)
{
//  RTCDate * date
  int i;
}
#endif



//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
WIFI_NEGOTIATION_SAVEDATA* WIFI_NEGOTIATION_SV_GetSaveData(SAVE_CONTROL_WORK* pSave)
{
	WIFI_NEGOTIATION_SAVEDATA* pData;
	pData = SaveControl_DataPtrGet(pSave, GMDATA_ID_WIFI_NEGOTIATION);
	return pData;

}

