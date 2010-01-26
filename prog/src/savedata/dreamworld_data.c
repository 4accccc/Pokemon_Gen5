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
 * @brief   PDW登録ずみフラグを入れる
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @param	  PDW登録ずみ
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetSignin(DREAMWORLD_SAVEDATA* pSV,BOOL bFlg)
{
  pSV->signin = bFlg;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   PDW登録ずみかどうか
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @return	PDW登録ずみ時はTRUE
 */
//--------------------------------------------------------------------------------------------
BOOL DREAMWORLD_SV_GetSignin(DREAMWORLD_SAVEDATA* pSV)
{
  return pSV->signin;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   アカウント登録を一回でも起動したかどうか
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @param	  アカウント登録していたらTRUE
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_SetAccount(DREAMWORLD_SAVEDATA* pSV,BOOL bFlg)
{
  pSV->bAccount = bFlg;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   アカウント登録を一回でも起動したかどうか
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @return	アカウント登録時はTRUE
 */
//--------------------------------------------------------------------------------------------
BOOL DREAMWORLD_SV_GetAccount(DREAMWORLD_SAVEDATA* pSV)
{
  return pSV->bAccount;
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
 * @brief   貰ったお土産のアイテムNoを得る
 * @param	  pSV		DREAMWORLD_SAVEDATA
 * @param	  index インデックス
 * @return	アイテムNo
 */
//--------------------------------------------------------------------------------------------

u16 DREAMWORLD_SV_GetItem(DREAMWORLD_SAVEDATA* pSV,int index)
{
  if(index > DREAM_WORLD_DATA_MAX_ITEMBOX){
    return 0;
  }
  return pSV->itemID[index];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   アイテムの数を取得
 * @param	  pSV		DREAMWORLD_SAVEDATAポインタ
 * @param	  index		場所
 * @return	  num		 数
 */
//--------------------------------------------------------------------------------------------

u8 DREAMWORLD_SV_GetItemNum(DREAMWORLD_SAVEDATA* pSV, int index)
{
  if(index > DREAM_WORLD_DATA_MAX_ITEMBOX){
    return 0;
  }
  return pSV->itemNum[index];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   貰ったアイテムをいれる
 * @param	  pSV		DREAMWORLD_SAVEDATAポインタ
 * @param	  index		場所
 * @param	  itemNo		アイテム番号
 * @param	  num		 数
 */
//--------------------------------------------------------------------------------------------

void DREAMWORLD_SV_SetItem(DREAMWORLD_SAVEDATA* pSV,int index ,int itemNo, int num)
{
  if(index > DREAM_WORLD_DATA_MAX_ITEMBOX){
    return;
  }
  pSV->itemID[index] =itemNo;
  pSV->itemNum[index] =num;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   貰ったアイテム消す
 * @param	  pSV		DREAMWORLD_SAVEDATAポインタ
 * @param	  index		場所
 */
//--------------------------------------------------------------------------------------------
void DREAMWORLD_SV_DeleteItem(DREAMWORLD_SAVEDATA* pSV,int index)
{
  DREAMWORLD_SV_SetItem(pSV,index,0,0);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   貰ったアイテム全部消す
 * @param	  pSV		DREAMWORLD_SAVEDATAポインタ
 */
//--------------------------------------------------------------------------------------------

void DREAMWORLD_SV_ClearAllItem(DREAMWORLD_SAVEDATA* pSV)
{
  int i;

  for(i=0;i<DREAM_WORLD_DATA_MAX_ITEMBOX;i++){
    DREAMWORLD_SV_DeleteItem(pSV,i);
  }
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
DREAM_WORLD_SLEEP_TYPE DREAMWORLD_SV_GetPokemonStatus(DREAMWORLD_SAVEDATA* pSV)
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
void DREAMWORLD_SV_SetPokemonStatus(DREAMWORLD_SAVEDATA* pSV,DREAM_WORLD_SLEEP_TYPE status)
{
  GF_ASSERT(status < DREAM_WORLD_SLEEP_TYPE_MAX);
  if(!(status < DREAM_WORLD_SLEEP_TYPE_MAX)){
    return;
  }
  pSV->pokemoStatus = status;
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




