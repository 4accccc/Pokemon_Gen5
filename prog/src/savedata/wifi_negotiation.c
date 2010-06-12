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

#define _COUNTMAX  (99999)  ///自分の交換ポケモン数MAX


#include "wifi_negotiation_local.h"


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
 * @brief   人物一致チェック
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   playerID
 * @return	一致したらTRUE
 */
//--------------------------------------------------------------------------------------------
static int _CheckFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,u32 playerID)
{
  int i;

  for(i=0;i<WIFI_NEGOTIATION_DATAMAX;i++){
    if(MyStatus_GetRomCode(&pSV->aMyStatus[ i ]) != 0){  //空かどうかの判定
      if(MyStatus_GetID(&pSV->aMyStatus[ i ]) == playerID){
        return i;
      }
    }
  }
  return -1;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   Wi-Fiネゴシエーション用ともだちデータを先頭に保存
 * @param   pSV       WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   pMyStatus MYSTATUS
 */
//--------------------------------------------------------------------------------------------
void WIFI_NEGOTIATION_SV_SetFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,const MYSTATUS* pMyStatus)
{
  u32 playerID = MyStatus_GetID(pMyStatus);
  u32 size = MyStatus_GetWorkSize();
  int no;

  if(playerID==0){
    return;
  }
  no = _CheckFriend(pSV,playerID);
  if(no != -1){ //一致している物はトレーナーVIEWだけ交換
    MyStatus_SetTrainerView(&pSV->aMyStatus[no], MyStatus_GetTrainerView(pMyStatus));
    return;
  }
  STD_MoveMemory(&pSV->aMyStatus[1],&pSV->aMyStatus[0],(WIFI_NEGOTIATION_DATAMAX-1)*size);
  GFL_STD_MemCopy(pMyStatus, &pSV->aMyStatus[0], size);
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
  return MyStatus_GetID(&pSV->aMyStatus[ i ]);
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
 * @brief   Wi-Fiネゴシエーション用MYSTATUSを引き出す
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   index インデックス WIFI_NEGOTIATION_DATAMAXまで
 * @return	profileID
 */
//--------------------------------------------------------------------------------------------
#if PM_DEBUG
MYSTATUS* WIFI_NEGOTIATION_SV_GetMyStatus_DEBUG(WIFI_NEGOTIATION_SAVEDATA* pSV,u32 index)
{
  int i = index;
  return &pSV->aMyStatus[ index ];
}
#endif


//--------------------------------------------------------------------------------------------
/**
 * @brief   渡したPlayerIDに一致する物があるかどうかさがす
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   playerID
 * @return	一致したらTRUE
 */
//--------------------------------------------------------------------------------------------
BOOL WIFI_NEGOTIATION_SV_IsCheckFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,u32 playerID)
{
  int i;

  if(-1==_CheckFriend(pSV, playerID)){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   Wi-Fiネゴシエーション用ともだちデータ有効数を返す
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   index インデックス WIFI_NEGOTIATION_DATAMAXまで
 * @return	人数
 */
//--------------------------------------------------------------------------------------------
s32 WIFI_NEGOTIATION_SV_GetFriendNum(WIFI_NEGOTIATION_SAVEDATA* pSV)
{
  int i;
  for(i=0;i<WIFI_NEGOTIATION_DATAMAX;i++){
    if(MyStatus_GetRomCode(&pSV->aMyStatus[ i ]) == 0){  //空かどうかの判定
      break;
    }
  }
  return i;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   交換回数を返す
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @return	回数
 */
//--------------------------------------------------------------------------------------------
u32 WIFI_NEGOTIATION_SV_GetChangeCount(WIFI_NEGOTIATION_SAVEDATA* pSV)
{
  return pSV->num;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   交換回数をインクリメント
 * @param   WIFI_NEGOTIATION_SAVEDATAポインタ
 * @return none
 */
//--------------------------------------------------------------------------------------------
void WIFI_NEGOTIATION_SV_AddChangeCount(WIFI_NEGOTIATION_SAVEDATA* pSV)
{
  pSV->num++;
  if(pSV->num > _COUNTMAX){
    pSV->num = _COUNTMAX;
  }
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

