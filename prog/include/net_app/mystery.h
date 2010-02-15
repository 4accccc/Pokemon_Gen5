//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mystery.h
 *	@brief  ふしぎなおくりものプロセス
 *	@author	Toru=Nagihashi
 *	@date		2009.12.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>
#include "gamesystem/game_data.h"
#include "savedata/mystery_data.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
FS_EXTERN_OVERLAY(mystery);

//=============================================================================
/**
 *					引数
*/
//=============================================================================
typedef struct
{ 
  int dummy;
}MYSTERY_PARAM;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern const GFL_PROC_DATA MysteryGiftProcData;

//--------------------------------------------------------------
/**
 * @brief  不思議データからのポケモン作成
 * @param  GIFT_PACK_DATA   セーブデータに格納されてる不思議データ
 * @param  HEAPID                 POKEMON_PARAMをつくるHEAPID
 * @param  GAMEDATA 
 * @param  POKEMON_PARAM  不正ならNULL  終ったら開放してください
 */
//--------------------------------------------------------------
extern POKEMON_PARAM* MYSTERY_CreatePokemon(const GIFT_PACK_DATA* pGift, HEAPID heapID, GAMEDATA* pGameData);

//--------------------------------------------------------------
/**
 * @brief  不思議データからのアイテム作成
 * @param  GIFT_PACK_DATA   セーブデータに格納されてる不思議データ
 * @retval itemsym.hのアイテム番号 不正の場合 ITEM_DUMMY_DATA
 */
//--------------------------------------------------------------
extern int MYSTERY_CreateItem(const GIFT_PACK_DATA* pGift);

//--------------------------------------------------------------
/**
 * @brief  不思議データからのGPower作成
 * @param  GIFT_PACK_DATA   セーブデータに格納されてる不思議データ
 * @retval @todo 未定
 */
//--------------------------------------------------------------
extern int MYSTERY_CreateGPower(const GIFT_PACK_DATA* pGift);

