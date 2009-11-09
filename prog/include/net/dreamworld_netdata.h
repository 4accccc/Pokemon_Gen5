
//============================================================================================
/**
 * @file	  dreamworld_netdata.h
 * @brief	  ポケモンドリームワールドに必要なデータ
 * @author	k.ohno
 * @date	  2009.11.08
 */
//============================================================================================
#pragma once

#include "savedata/dreamworld_data.h"

#define DREAM_WORLD_SERVER_POKMEONLIST_MAX (1024)  //寝るポケモンのリスト最大

typedef enum{
  DREAM_WORLD_SERVER_ERROR_NONE,        // 正常です
  DREAM_WORLD_SERVER_ERROR_SERVER_FULL,	// サーバーの容量オーバーです
	DREAM_WORLD_SERVER_ALREADY_EXISTS,	  // サーバーに既にデータがあります
	DREAM_WORLD_SERVER_ILLEGAL_DATA,		// アップロードされたデータが壊れています
	DREAM_WORLD_SERVER_CHEAT_DATA,		  // アップロードされたデータが不正です
	DREAM_WORLD_SERVER_NO_DATA,		      // サーバにデータがありません
  DREAM_WORLD_SERVER_END,             // サービスが終了している
  DREAM_WORLD_SERVER_ETC_ERROR,       // その他サーバがエラーの場合 これ以上の値が来ても同様にエラーにする
} DREAM_WORLD_SERVER_ERROR_TYPE;   /// サーバから返ってくる答え

/// 状態確認構造体
struct DREAM_WORLD_SERVER_STATUS_DATA{
  DREAM_WORLD_SERVER_ERROR_TYPE type;   /// DREAM_WORLD_SERVER_ERROR_TYPE
};



/// アカウント作成構造体
struct DREAM_WORLD_SERVER_CREATE_DATA{
  u8 GameSyncID[12];   /// GameSyncID
};



/// ダウンロードデータ構造体
struct DREAM_WORLD_SERVER_DOWNLOAD_DATA{
	u32 RomCodeBit;   ///< ROMコード	4byte			ROMのバージョン		pm_version.hのBIT管理 一致したら受け取る
	u32 PassCode;     ///<  特殊イベント用起動キーコード（簡単に起動できないように暗号化）
	u16 TreatType;    ///< 2byte		DREAM_WORLD_RESULT_TYPE	夢の結果データのカテゴリ
	DREAM_WORLD_TREAT_DATA TreatData;  ///< 2byte  DREAM_WORLD_TREAT_DATAのデータ
	DREAM_WORLD_FURNITUREDATA Furniture[DREAM_WORLD_DATA_MAX_FURNITURE];  ///<	26*5=130  配信家具
	u16 CountryBit;   ///<	国コード	2byte			言語バージョン		pm_version.hのBIT管理 一致したら受け取る
  u8 OnceBit;       ///<  一回だけかどうか 特殊イベントを1回しか受け取れないよう設定するフラグ
  u8 PokemonState;  ///< 寝ているポケモンの状態
};

/// ポケモンリストデータ構造体
struct DREAM_WORLD_SERVER_POKEMONLIST_DATA{
  u8 pokemonList[DREAM_WORLD_SERVER_POKMEONLIST_MAX/8];  ///< 寝かせてよいポケモンBITLIST
};
