//============================================================================================
/**
 * @file	  dreamworld_data.c
 * @brief	  ポケモンドリームワールドに必要なデータ
 * @author	k.ohno
 * @date	  2009.05.14
 */
//============================================================================================

#include <gflib.h>

#include "dreamworld_data_local.h"



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
POKEMON_PARAM* DREAMWORLD_SV_GetSleepPokemon(DREAMWORLD_SAVEDATA* pSV)
{
  return &pSV->pp;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   寝てるポケモンをセットする
            データの検査はここではしてないので必ず入れる前にする事
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @param	  POKEMON_PASO_PARAM*
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetSleepPokemon(DREAMWORLD_SAVEDATA* pSV,POKEMON_PARAM* pp)
{
  GFL_STD_MemCopy(pp,&pSV->pp,POKETOOL_GetWorkSize());
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   アカウント登録を一回でも起動したかどうか
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @param	  SigninしていたらTRUE
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetSignin(DREAMWORLD_SAVEDATA* pSV,BOOL bFlg)
{
  pSV->signin = bFlg;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   アカウント登録を一回でも起動したかどうか
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @return	ポケモンセット時はTRUE
 */
//--------------------------------------------------------------------------------------------
BOOL DREAMWORLD_SV_GetSignin(DREAMWORLD_SAVEDATA* pSV)
{
  return pSV->signin;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   寝てるポケモンをセットしたらフラグをON
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @param	  ポケモンセット時はTRUE
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetSleepPokemonFlg(DREAMWORLD_SAVEDATA* pSV,BOOL bFlg)
{
  pSV->pokemonIn = bFlg;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   寝てるポケモンをセットしたかどうか確認
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @return	ポケモンセット時はTRUE
 */
//--------------------------------------------------------------------------------------------
BOOL DREAMWORLD_SV_GetSleepPokemonFlg(DREAMWORLD_SAVEDATA* pSV)
{
  return pSV->pokemonIn;
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




