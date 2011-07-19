//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wb_save_convert.h
 *	@brief  WBセーブのコンバート処理
 *	@author	Toru=Nagihashi
 *	@date		2011.07.15
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "buflen.h"
#include "pm_define.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	フラグ類
//=====================================
typedef enum
{
  WB_SAVE_CONV_FLAG_GAME_CLEAR,
  WB_SAVE_CONV_FLAG_FIRST_POKE_GET,
  WB_SAVE_CONV_FLAG_CHAMPION_WIN,
  WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_01,
  WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_02,
  WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_01,
  WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_02,
  WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_03,
  WB_SAVE_CONV_FLAG_BADGE_01,
  WB_SAVE_CONV_FLAG_BADGE_02,
  WB_SAVE_CONV_FLAG_BADGE_03,
  WB_SAVE_CONV_FLAG_BADGE_04,
  WB_SAVE_CONV_FLAG_BADGE_05,
  WB_SAVE_CONV_FLAG_BADGE_06,
  WB_SAVE_CONV_FLAG_BADGE_07,
  WB_SAVE_CONV_FLAG_BADGE_08,
  WB_SAVE_CONV_FLAG_MAX
} WB_SAVE_CONV_FLAG;

//-------------------------------------
///	ワーク類
//=====================================
typedef enum
{
  WB_SAVE_CONV_WORK_N_BATTLE_RAIMON,
  WB_SAVE_CONV_WORK_MAX
} WB_SAVE_CONV_WORK;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	WBセーブオーナーデータ
//=====================================
typedef struct
{
  u32     trainerID;
  STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];
  u8      sex;
  u8      version;
  u8      dummy[2];
}WB_SAVE_CONV_OWNER;

//----------------------------------------------------------
/**
 *  殿堂入りポケモン個体データ（60 bytes）
 */
//----------------------------------------------------------
typedef struct {
  u16 monsno;       ///< ポケモンナンバー（０ならデータ全体が無効）
  u8  level;        ///< レベル
  u8  formNumber:6; ///< フォルム
  u8  sex:2;        ///< 性別
  u32 personalRandom;   ///< 個体乱数
  u32 idNumber;     ///< ID
  STRCODE nickname[ MONS_NAME_SIZE+EOM_SIZE ]; ///< ニックネーム
  STRCODE oyaname[ PERSON_NAME_SIZE+EOM_SIZE ];   ///< おや名
  u16 waza[ 4 ];      ///< 技ナンバー
}WB_SAVE_CONV_DENDOU_POKEMON_DATA;

//-------------------------------------
///	WBコンバートデータ
//=====================================
typedef struct
{
  u8  flag[ MATH_ROUNDUP(WB_SAVE_CONV_FLAG_MAX,8) / 8 ];
  u16 work[WB_SAVE_CONV_WORK_MAX];

  u8  musical_item[256/8];
  u16 musical_sum_point;
  u16 musical_entry_num;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  pokemon[ TEMOTI_POKEMAX ];

} WB_SAVE_CONV_DATA;

//-------------------------------------
///	全データ
//=====================================
typedef struct
{
  WB_SAVE_CONV_OWNER  owner;
  WB_SAVE_CONV_DATA   data;
} WB_SAVE_CONV_ALL_DATA;



//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern WB_SAVE_CONV_OWNER * WB_SAVE_CONV_AllocConvertOwner( const void *cp_data_adrs, HEAPID heapID );
extern WB_SAVE_CONV_DATA * WB_SAVE_CONV_AllocConvertData( const void *cp_data_adrs, HEAPID heapID );

extern WB_SAVE_CONV_ALL_DATA * WB_SAVE_CONV_AllocConvertAllData( const void *cp_data_adrs, HEAPID heapID );
