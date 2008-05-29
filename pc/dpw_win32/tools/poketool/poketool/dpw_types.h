/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     dpw_types.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*! @file
	@brief	DPWライブラリで使用される型の定義
	
	@author	kitayama(kitayama_shigetoshi@nintendo.co.jp)
	
	@version 0.01 (2006/07/20)
		@li 新規リリースしました。
*/

#ifndef DPW_TYPES_H_
#define DPW_TYPES_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "dpw_typedef.h"
#include "poketool.h"

/*-----------------------------------------------------------------------*
					世界通信交換で使用される型・定義
 *-----------------------------------------------------------------------*/

//! 名前のサイズ
#define DPW_TR_NAME_SIZE 8

//! 日時構造体
typedef struct {
	u16 year;	//!< 年
	u8 month;	//!< 月
	u8 day;		//!< 日
	u8 hour;	//!< 時
	u8 minutes;	//!< 分
	u8 sec;		//!< 秒
	u8 padding;	// パディング
} Dpw_Tr_Date;

//! ポケモンの性別
typedef enum {
	DPW_TR_GENDER_MALE = 1,		//!< オス
	DPW_TR_GENDER_FEMALE = 2,	//!< メス
	DPW_TR_GENDER_NONE = 3		//!< 性別なし。検索時に性別を問わないようにするためには、この値を指定してください。
} Dpw_Tr_PokemonGender;

//! ポケモンデータのサーバー検索用構造体
typedef struct {
	s16 characterNo;	//!< キャラクタ番号
	s8 gender;			//!< 性別。 Dpw_Tr_PokemonGender の値になる。
	s8 level;			//!< レベル。
} Dpw_Tr_PokemonDataSimple;

//! ポケモンデータのサーバー検索条件構造体
typedef struct {
	s16 characterNo;	//!< キャラクタ番号
	s8 gender;			//!< 性別。 Dpw_Tr_PokemonGender の値になる。
	s8 level_min;		//!< レベルの下限。0は指定なし、1～100はそのレベル以上のポケモンという意味になる。
	s8 level_max;		//!< レベルの上限。0は指定なし、1～100はそのレベル以下のポケモンという意味になる。
	s8 padding;			// パディング
} Dpw_Tr_PokemonSearchData;

//! ポケモンのデータ。
typedef struct {
	POKEMON_PARAM data;		//!< ポケモンのデータ
} Dpw_Tr_PokemonData;

//! Dpw_Trライブラリでサーバーとやり取りされるメインデータ構造体
typedef struct {
	Dpw_Tr_PokemonData postData;			//!< 「預ける」ポケモンの詳細データ
	Dpw_Tr_PokemonDataSimple postSimple;	//!< 「預ける」ポケモンの簡略データ（サーバー検索用）
	Dpw_Tr_PokemonSearchData wantSimple;	//!< 「ほしい」ポケモンの簡略データ（サーバー検索用）
	u8 gender;								//!< 主人公の性別
	u8 padding;								// パディング
	Dpw_Tr_Date postDate;					//!< 預けた日時（サーバーにてセット）
	Dpw_Tr_Date tradeDate;					//!< 交換成立日時（サーバーにてセット）
	s32 id;									//!< このデータのID（サーバーにてセット）
	u16 name[DPW_TR_NAME_SIZE];				//!< 主人公名
	u16 trainerID;							//!< トレーナーのID
	u8 countryCode;							//!< 住んでいる国コード
	u8 localCode;							//!< 住んでいる地方コード
	u8 trainerType;							//!< トレーナータイプ
	s8 isTrade;								//!< 交換済みフラグ（サーバーにてセット）
	u8 versionCode;							//!< バージョンコード
	u8 langCode;							//!< 言語コード
} Dpw_Tr_Data;

/*-----------------------------------------------------------------------*
					バトルタワーで使用される型・定義
 *-----------------------------------------------------------------------*/

//! ポケモンのデータ。
typedef struct {
	B_TOWER_POKEMON data;		//!< ポケモンのデータ
} Dpw_Bt_PokemonData;

//! リーダーデータ構造体
typedef struct {
	u16 playerName[8];		//!< プレイヤー名
	u8 versionCode;			//!< バージョンコード
	u8 langCode;			//!< 言語コード
	u8 countryCode;			//!< 住んでいる国コード
	u8 localCode;			//!< 住んでいる地方コード
	u8 playerId[4];			//!< プレイヤーID
	s8 leaderMessage[8];	//!< リーダーメッセージ
	union{
		struct{
			u8 ngname_f	:1;	//!< NGネームフラグ
			u8 gender	:1;	//!< プレイヤーの性別フラグ
			u8			:6;	// あまり6bit
		};
		u8	  flags;
	};
	u8 padding;				// パディング
} Dpw_Bt_Leader;

//! プレイヤーデータ構造体
typedef struct {
	Dpw_Bt_PokemonData pokemon[3];	//!< ポケモンデータ
	u16 playerName[8];		//!< プレイヤー名
	u8 versionCode;			//!< バージョンコード
	u8 langCode;			//!< 言語コード
	u8 countryCode;			//!< 住んでいる国コード
	u8 localCode;			//!< 住んでいる地方コード
	u8 playerId[4];			//!< プレイヤーID
	s8 leaderMessage[8];	//!< リーダーメッセージ
	union{
		struct{
			u8 ngname_f	:1;	//!< NGネームフラグ
			u8 gender	:1;	//!< プレイヤーの性別フラグ
			u8			:6;	// あまり6bit
		};
		u8	  flags;
	};
	u8 trainerType;			//!< トレーナータイプ
	s8 message[24];			//!< メッセージ
	u16 result;				//!< 成績
} Dpw_Bt_Player;

//! ルームデータ構造体
typedef struct {
	Dpw_Bt_Player player[7];	//!< プレイヤーデータ
	Dpw_Bt_Leader leader[30];	//!< リーダーデータ
} Dpw_Bt_Room;


#ifdef __cplusplus
}
#endif

#endif /* DPW_TYPES_H_ */
