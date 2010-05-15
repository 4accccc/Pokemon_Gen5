//============================================================================================
/**
 * @file	  wifi_negotiation_debug.c
 * @brief	  WIFIネゴシエーション用ともだち履歴データ デバッグ
 * @author	k.ohno
 * @date	  2009.10.16
 */
//============================================================================================

#if PM_DEBUG
#include "gflib.h"

#include "savedata/save_tbl.h"
#include "savedata/wifi_negotiation.h"
#include "savedata/mystatus_local.h"
#include "system/pms_data.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "buflen.h"


#include "wifi_negotiation_local.h"



//--------------------------------------------------------------------------------------------
/**
 * @brief   ともだち削除
 * @param   pSV       WIFI_NEGOTIATION_SAVEDATAポインタ
 * @param   pMyStatus MYSTATUS
 */
//--------------------------------------------------------------------------------------------
void WIFINEGOSV_DEBUG_DeleteFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,int index)
{
  int x=0;
  int i, j, count;
  MYSTATUS* pMyStatus;

  count = WIFI_NEGOTIATION_SV_GetCount(pSV);

  j = count - 1 - index;
  if(j<0){
    j+=WIFI_NEGOTIATION_DATAMAX;
  }
  j = j % WIFI_NEGOTIATION_DATAMAX;

  for(x = j; x < (WIFI_NEGOTIATION_DATAMAX-1); x++){
    MyStatus_Copy(&pSV->aMyStatus[x+1], &pSV->aMyStatus[x]);
  }
  GFL_STD_MemClear(&pSV->aMyStatus[WIFI_NEGOTIATION_DATAMAX-1],  MyStatus_GetWorkSize());
  pSV->count = WIFI_NEGOTIATION_SV_GetFriendNum(pSV);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   ダミー追加
 * @param   数
 */
//--------------------------------------------------------------------------------------------
void WIFINEGOSV_DEBUG_AddFriend(WIFI_NEGOTIATION_SAVEDATA* pSV,u32 addnum)
{
  int i;
  
  for(i = 0; i < addnum;i++){
    MYSTATUS* pMyStatus = MyStatus_AllocWork(GFL_HEAPID_APP);
    MyStatus_SetProfileID(pMyStatus,1+i);
    MyStatus_SetID(pMyStatus, GFUser_GetPublicRand(50000) );
    MyStatus_SetTrainerView(pMyStatus, i%16);
    MyStatus_SetMyNationArea(pMyStatus, 1+i,1);
    WIFI_NEGOTIATION_SV_SetFriend(pSV,pMyStatus);
    GFL_HEAP_FreeMemory(pMyStatus);
  }
}


#endif
