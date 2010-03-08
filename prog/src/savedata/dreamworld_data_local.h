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
  GFDATE gsyncTime;  //最終アクセス時間
  u32 uploadCount;   //預けた回数  値を入れる。同じ物をダウンロードしたら値を反映しない
  POKEMON_PARAM pp;   ///< 眠るポケモン
  u16 itemID[DREAM_WORLD_DATA_MAX_ITEMBOX];  ///持ち帰ったアイテム
  u8 itemNum[DREAM_WORLD_DATA_MAX_ITEMBOX];
	DREAM_WORLD_FURNITUREDATA furnitureID[DREAM_WORLD_DATA_MAX_FURNITURE];  //9	配信家具
  u8 pokemoStatus;        //8	送信したポケモン						1byte			送信したポケモンの状態を受け取る
  u8 pokemonIn;  ///ppにポケモンが入っているかどうか
  u8 signin; //PDW登録済み
  u8 bAccount; //アカウント取得済み
  u8 furnitureNo     :7;  //選んだ家具のインデックス  無い場合 DREAM_WORLD_NOFANITURE
  u8 isSyncFurniture :1;  //家具をSyncして送ったか？(家具屋で話すと消える
  u8 musicalNo;      ///< webで選択した番号  無い場合 0xff
  u8 cgearNo;        ///< webで選択した番号  無い場合 0xff
  u8 zukanNo;        ///< webで選択した番号  無い場合 0xff
};

