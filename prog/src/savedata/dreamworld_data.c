//============================================================================================
/**
 * @file	  dreamworld_data.c
 * @brief	  ポケモンドリームワールドに必要なデータ
 * @author	k.ohno
 * @date	  2009.05.14
 */
//============================================================================================

#include <gflib.h>

#include "savedata/save_tbl.h"
#include "savedata/save_control.h"	//SAVE_CONTROL_WORK参照のため
#include "savedata/dreamworld_data.h"


//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _DREAMWORLD_SAVEDATA {
  u8 recv_flag[DREAM_WORLD_DATA_MAX_EVENT / 8];		//256 * 8 = 2048 bit
  u16 categoryType;    //カテゴリID						2byte			夢の結果データのカテゴリ（イベントなのか、アイテムなのか、エンカウントなのか）
  DREAM_WORLD_TREAT_DATA treat;  //ご褒美データ 
  u8 pokemoStatus;        //8	送信したポケモン						1byte			送信したポケモンの状態を受け取る
	DREAM_WORLD_FURNITUREDATA furnitureID[DREAM_WORLD_DATA_MAX_FURNITURE];  //9	配信家具
};




//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体サイズを得る
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------
int DREAMWORLD_SV_GetWorkSize(void)
{
	return sizeof(DREAMWORLD_SAVEDATA);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体を得る
 * @return	構造体
 */
//--------------------------------------------------------------------------------------------

DREAMWORLD_SAVEDATA* DREAMWORLD_SV_AllocWork(HEAPID heapID)
{
	return GFL_HEAP_AllocClearMemory(heapID, DREAMWORLD_SV_GetWorkSize());
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   中身を初期化する
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------

void DREAMWORLD_SV_Init(DREAMWORLD_SAVEDATA* pSV)
{
	GFL_STD_MemClear(pSV, DREAMWORLD_SV_GetWorkSize());
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   一回受け取ったかどうか
 * @param	  pSV		セーブデータへのポインタ
 * @param   num     通し番号
 * @return	受け取ったらTRUE
 */
//--------------------------------------------------------------------------------------------
BOOL DREAMWORLD_SV_IsEventRecvFlag(DREAMWORLD_SAVEDATA* pSV,int num)
{
  GF_ASSERT(num < DREAM_WORLD_DATA_MAX_EVENT);
  if(num > DREAM_WORLD_DATA_MAX_EVENT){
    return TRUE;
  }
  return (pSV->recv_flag[num / 8] & (1 << (num & 7)));
}

//------------------------------------------------------------------
/**
 * @brief	指定のイベントもらったよフラグを立てる
 * @param	pSV		セーブデータへのポインタ
 * @param	num		イベント番号
 * @return	NONE
 */
//------------------------------------------------------------------
void DREAMWORLD_SV_SetEventRecvFlag(DREAMWORLD_SAVEDATA* pSV, int num)
{
  GF_ASSERT(num < DREAM_WORLD_DATA_MAX_EVENT);
  if(num > DREAM_WORLD_DATA_MAX_EVENT){
    return;
  }
  pSV->recv_flag[num / 8] |= (1 << (num & 7));
}


//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
DREAMWORLD_SAVEDATA* DREAMWORLD_SV_GetDreamWorldSaveData(SAVE_CONTROL_WORK* pSave)
{
	DREAMWORLD_SAVEDATA* pData;
	pData = SaveControl_DataPtrGet(pSave, GMDATA_ID_DREAMWORLD);
	return pData;

}




