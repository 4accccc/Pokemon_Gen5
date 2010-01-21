//============================================================================================
/**
 * @file	  dreamworld_data_local.h
 * @brief	  ポケモンドリームワールドに必要なデータ
 * @author	k.ohno
 * @date	  2009.05.14
 */
//============================================================================================
#pragma once

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
  POKEMON_PARAM pp;   ///< 眠るポケモン
  u16 itemID[DREAM_WORLD_DATA_MAX_ITEMBOX];  ///持ち帰ったアイテム
  u8 itemNum[DREAM_WORLD_DATA_MAX_ITEMBOX];
	DREAM_WORLD_FURNITUREDATA furnitureID[DREAM_WORLD_DATA_MAX_FURNITURE];  //9	配信家具
  u8 pokemoStatus;        //8	送信したポケモン						1byte			送信したポケモンの状態を受け取る
  u8 pokemonIn;  ///ppにポケモンが入っているかどうか
  u8 signin; //アカウントをすでに登録したかどうか
};

