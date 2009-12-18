
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
  DREAM_WORLD_SERVER_ERROR_ETC,       // その他サーバがエラーの場合 これ以上の値が来ても同様にエラーにする
  DREAM_WORLD_SERVER_ERROR_MAX
} DREAM_WORLD_SERVER_ERROR_TYPE;   /// サーバから返ってくるサーバの状態

typedef enum{
  DREAM_WORLD_SERVER_PLAY_NONE,        // まだ遊んでいません
  DREAM_WORLD_SERVER_PLAY_END,	         // ドリームワールドで遊びました
  DREAM_WORLD_SERVER_PLAY_MAX
} DREAM_WORLD_SERVER_PLAY_TYPE;   /// サーバから返ってくる遊びに関しての情報



/// 状態確認構造体
struct DREAM_WORLD_SERVER_STATUS_DATA{
  DREAM_WORLD_SERVER_ERROR_TYPE type;   /// DREAM_WORLD_SERVER_ERROR_TYPE
  DREAM_WORLD_SERVER_PLAY_TYPE PlayStatus; //DREAM_WORLD_SERVER_PLAY_TYPE
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
typedef struct {
  u8 pokemonList[DREAM_WORLD_SERVER_POKMEONLIST_MAX/8];  ///< 寝かせてよいポケモンBITLIST
} DREAM_WORLD_SERVER_POKEMONLIST_DATA;



typedef enum{
  DREAM_WORLD_MATCHUP_NONE,     // 登録していない状態
  DREAM_WORLD_MATCHUP_SIGNUP,	  // web登録した。DS->SAKEの接続はまだ
  DREAM_WORLD_MATCHUP_ENTRY,	  // 参加中
  DREAM_WORLD_MATCHUP_RETIRE,		 // 途中で解除
} DREAM_WORLD_MATCHUP_TYPE;   /// 世界対戦の進行状況


/// 世界対戦確認用データ構造体
struct DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA{
  u32 WifiMatchUpID;   ///< 世界対戦の開催ID
  u8 GPFEntryFlg;      ///< GPFからエントリーしたフラグ
  u8 WifiMatchUpState; ///< 世界対戦の進行状況
};


/// 世界対戦書き込みデータ構造体
struct DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA{
  u8 WifiMatchUpState; ///< 世界対戦の進行状況
  u8 padding;
};


