//============================================================================================
/**
 * @file	wifihistory_local.h
 * @brief	WiFi通信履歴データ構造定義ヘッダ
          国連セーブデータを追加  国関係としてまとめた方が都合が良い為
 * @date	2006.03.25
 */
//============================================================================================

#ifndef	__WIFIHISTORY_LOCAL_H__
#define	__WIFIHISTORY_LOCAL_H__

#include "savedata/wifihistory.h"
#include "savedata/mystatus_local.h"

//============================================================================================
//============================================================================================

enum {
	///国ごとに確保するデータサイズ（バイト単位）
	NATION_DATA_SIZE = WIFI_AREA_MAX * 2 / 8,
	//↑１データあたり2ビット必要でバイト単位に直すために8で割っている
};


enum {
  //国連のフロアのサイズ
  BILL_DATA_SIZE = (WIFI_COUNTRY_MAX+7) / 8,
  //国連にとっておく人の数
  UNITEDNATIONS_PEOPLE_MAX = 20,
};



typedef struct _UNITEDNATIONS_SAVE_tag{
  MYSTATUS aMyStatus;
  u16 recvPokemon;  //貰ったポケモン
  u16 sendPokemon;  //あげたポケモン
  u8 favorite;   //趣味
  u8 countryCount;  //交換した国の回数
  u8 nature:3;   //性格
  u8 bTalk:1;   //話したかどうか
  u8 valid:1;  //有効データか？
  u8 dummy1:3;
  u8 dummy2;
}_UNITEDNATIONS_SAVE;

//----------------------------------------------------------
/**
 * @brief	WiFi通信履歴データ用構造体定義
 */
//----------------------------------------------------------
struct _WIFI_HISTORY{
  /// 国連にとっておく人
  _UNITEDNATIONS_SAVE aUnitedPeople[UNITEDNATIONS_PEOPLE_MAX];
  u8 billopen[BILL_DATA_SIZE];  ///< あけたことがあるビルのフロア
	///日付
	u32	date;
	u8 world_flag;			///<全世界対象モードかどうかのフラグ
  u8 myCountryCount;  ///< 何カ国と交換したか回数
  u8 myFavorite;     ///自分の趣味
  u8 myNature;       ///自分の性格
	///履歴データ
	u8	data[(WIFI_NATION_MAX - 1) * NATION_DATA_SIZE];

};

#endif	/* __WIFIHISTORY_LOCAL_H__ */
