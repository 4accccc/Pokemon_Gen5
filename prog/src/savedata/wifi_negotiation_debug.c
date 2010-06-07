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
  int size = WIFI_NEGOTIATION_DATAMAX-index;
  MYSTATUS* pMyStatus;

  size--;
  if(size > 0){
    STD_MoveMemory(&pSV->aMyStatus[index], &pSV->aMyStatus[index+1],MyStatus_GetWorkSize()*size);
  }
  GFL_STD_MemClear(&pSV->aMyStatus[WIFI_NEGOTIATION_DATAMAX-1],  MyStatus_GetWorkSize());
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

  WIFI_NEGOTIATION_SV_Init(pSV);
  for(i = 0; i < addnum;i++){
    MYSTATUS* pMyStatus = MyStatus_AllocWork(GFL_HEAPID_APP);
    MyStatus_SetProfileID(pMyStatus,1+i);
    MyStatus_SetID(pMyStatus, i+1 );
    MyStatus_SetTrainerView(pMyStatus, i%16);
    MyStatus_SetMyNationArea(pMyStatus, 1+i,1);
    WIFI_NEGOTIATION_SV_SetFriend(pSV,pMyStatus);
    GFL_HEAP_FreeMemory(pMyStatus);
  }
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   デバッグプリント
 * @param   数
 */
//--------------------------------------------------------------------------------------------
void WIFINEGOSV_DEBUG_PrintFriend(WIFI_NEGOTIATION_SAVEDATA* pSV)
{
  int i;
  
  for(i = 0; i < 100;i++){
    int no = WIFI_NEGOTIATION_SV_GetFriend(pSV,i);
    OS_TPrintf("LISTNO %d  ID %d\n",i,no);
  }
}


#endif
