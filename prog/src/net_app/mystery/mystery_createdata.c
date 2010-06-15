//============================================================================================
/**
 * @file    mystery_createdata.c
 * @brief   不思議通信データからゲーム用データを作る関数

             この関数はNOTWIFIにおいてありますが、
             通信中でも使いたいためmystery_inline.hへ移しました。nagihashi

 * @author  k.ohno
 * @date    10.01.27
 */
//============================================================================================
#include "mystery_inline.h"
#include "net_app/mystery.h"
//--------------------------------------------------------------
/**
 * @brief  不思議データからのポケモン作成
 * @param  GIFT_PACK_DATA   セーブデータに格納されてる不思議データ
 * @param  HEAPID                 POKEMON_PARAMをつくるHEAPID
 * @param  GAMEDATA 
 * @retval POKEMON_PARAM  失敗したらNULL 終ったら開放してください
 */
//--------------------------------------------------------------
POKEMON_PARAM* MYSTERY_CreatePokemon(const GIFT_PACK_DATA* pPack, HEAPID heapID, GAMEDATA* pGameData)
{
  return Mystery_CreatePokemon( pPack, heapID, pGameData);
}

//--------------------------------------------------------------
/**
 * @brief  不思議データからのアイテム作成
 * @param  GIFT_PACK_DATA   セーブデータに格納されてる不思議データ
 * @retval itemsym.hのアイテム番号 不正の場合 ITEM_DUMMY_DATA
 */
//--------------------------------------------------------------
int MYSTERY_CreateItem(const GIFT_PACK_DATA* pPack)
{
  return Mystery_CreateItem(pPack);
}

//--------------------------------------------------------------
/**
 * @brief  不思議データからのGPower作成
 * @param  GIFT_PACK_DATA   セーブデータに格納されてる不思議データ
 * @retval GPowerID
 */
//--------------------------------------------------------------
int MYSTERY_CreateGPower(const GIFT_PACK_DATA* pPack)
{
  return Mystery_CreateGPower(pPack);
}
