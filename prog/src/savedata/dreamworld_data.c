//============================================================================================
/**
 * @file	  dreamworld_data.c
 * @brief	  ポケモンドリームワールドに必要なデータ
 * @author	k.ohno
 * @date	  2009.05.14
 */
//============================================================================================

#include <gflib.h>

#include "poke_tool/poke_tool.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"	//SAVE_CONTROL_WORK参照のため
#include "savedata/dreamworld_data.h"
#include "poke_tool/poke_tool_def.h"


//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _DREAMWORLD_SAVEDATA {
  POKEMON_PASO_PARAM ppp;   ///< 眠るポケモン
  u8 recv_flag[DREAM_WORLD_DATA_MAX_EVENT / 8];		//256 * 8 = 2048 bit
  u16 categoryType;    //カテゴリID						2byte			夢の結果データのカテゴリ（イベントなのか、アイテムなのか、エンカウントなのか）
  DREAM_WORLD_TREAT_DATA treat;  //ご褒美データ 
	DREAM_WORLD_FURNITUREDATA furnitureID[DREAM_WORLD_DATA_MAX_FURNITURE];  //9	配信家具
  u8 pokemoStatus;        //8	送信したポケモン						1byte			送信したポケモンの状態を受け取る
  u8 padding;
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
 * @brief   DREAMWORLD_SAVEDATA構造体を得る
 * @return	DREAMWORLD_SAVEDATA構造体
 */
//--------------------------------------------------------------------------------------------

DREAMWORLD_SAVEDATA* DREAMWORLD_SV_AllocWork(HEAPID heapID)
{
	return GFL_HEAP_AllocClearMemory(heapID, DREAMWORLD_SV_GetWorkSize());
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   中身を初期化する
 * @param   DREAMWORLD_SAVEDATAのポインタ
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------

void DREAMWORLD_SV_Init(DREAMWORLD_SAVEDATA* pSV)
{
	GFL_STD_MemClear(pSV, DREAMWORLD_SV_GetWorkSize());
}



//--------------------------------------------------------------------------------------------
/**
 * @brief   寝てるポケモンを得る
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @return	POKEMON_PASO_PARAM
 */
//--------------------------------------------------------------------------------------------
POKEMON_PASO_PARAM* DREAMWORLD_SV_GetSleepPokemon(DREAMWORLD_SAVEDATA* pSV)
{
  return &pSV->ppp;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   寝てるポケモンをセットする
            データの検査はここではしてないので必ず入れる前にする事
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @param	  POKEMON_PASO_PARAM*
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetSleepPokemon(DREAMWORLD_SAVEDATA* pSV,POKEMON_PASO_PARAM* ppp)
{
  GFL_STD_MemCopy(ppp,&pSV->ppp,sizeof(POKETOOL_GetPPPWorkSize()));
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   貰ったお土産のカテゴリーを得る
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @return	DREAM_WORLD_RESULT_TYPE
 */
//--------------------------------------------------------------------------------------------
DREAM_WORLD_RESULT_TYPE DREAMWORLD_SV_GetCategoryType(DREAMWORLD_SAVEDATA* pSV)
{
  return pSV->categoryType;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   貰ったお土産のカテゴリーをセット
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @return	DREAM_WORLD_RESULT_TYPE
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetCategoryType(DREAMWORLD_SAVEDATA* pSV,DREAM_WORLD_RESULT_TYPE type)
{
  GF_ASSERT(type < DREAM_WORLD_RESULT_TYPE_MAX);
  if(!(type < DREAM_WORLD_RESULT_TYPE_MAX)){
    return;
  }
  pSV->categoryType = type;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   貰ったお土産をえる
 * @param	  pSV		DREAMWORLD_SAVEDATAポインタ
 * @return	DREAM_WORLD_TREAT_DATA*
 */
//--------------------------------------------------------------------------------------------
DREAM_WORLD_TREAT_DATA* DREAMWORLD_SV_GetTreatData(DREAMWORLD_SAVEDATA* pSV)
{
  return &pSV->treat;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   貰ったお土産を格納
            データの検査はここではしてないので必ず入れる前にする事
 * @param	  pSV		DREAMWORLD_SAVEDATAポインタ
 * @return	DREAM_WORLD_TREAT_DATA*
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetTreatData(DREAMWORLD_SAVEDATA* pSV,DREAM_WORLD_TREAT_DATA* pTreat)
{
  GFL_STD_MemCopy(pTreat,&pSV->treat,sizeof(DREAM_WORLD_TREAT_DATA));
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   家具データをえる
 * @param	  pSV		DREAMWORLD_SAVEDATAポインタ
 * @return	DREAM_WORLD_FURNITUREDATA*
 */
//--------------------------------------------------------------------------------------------
DREAM_WORLD_FURNITUREDATA* DREAMWORLD_SV_GetFurnitureData(DREAMWORLD_SAVEDATA* pSV,int index)
{
  GF_ASSERT(index < DREAM_WORLD_DATA_MAX_FURNITURE);
  if(!(index < DREAM_WORLD_DATA_MAX_FURNITURE)){
    return NULL;
  }
  return &pSV->furnitureID[index];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   家具データを格納する
 * @param	  pSV		DREAMWORLD_SAVEDATAポインタ
 * @return	DREAM_WORLD_FURNITUREDATA*
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetFurnitureData(DREAMWORLD_SAVEDATA* pSV,int index,DREAM_WORLD_FURNITUREDATA* pF)
{
  GF_ASSERT(index < DREAM_WORLD_DATA_MAX_FURNITURE);
  if(!(index < DREAM_WORLD_DATA_MAX_FURNITURE)){
    return;
  }
  GFL_STD_MemCopy(pF,&pSV->furnitureID[index],sizeof(DREAM_WORLD_FURNITUREDATA));
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   寝てるポケモンのステータスを得る
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @return	DREAM_WORLD_RESULT_TYPE
 */
//--------------------------------------------------------------------------------------------
DREAM_WORLD_RESULT_TYPE DREAMWORLD_SV_GetPokemonStatus(DREAMWORLD_SAVEDATA* pSV)
{
  return pSV->pokemoStatus;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   寝てるポケモンのステータスをセット
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @param	  status
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetPokemonStatus(DREAMWORLD_SAVEDATA* pSV,DREAM_WORLD_RESULT_TYPE status)
{
  GF_ASSERT(status < DREAM_WORLD_RESULT_TYPE_MAX);
  if(!(status < DREAM_WORLD_RESULT_TYPE_MAX)){
    return;
  }
  pSV->pokemoStatus = status;
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




