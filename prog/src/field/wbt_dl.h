//============================================================================================
/**
 * @file  wbt_dl.h
 * @brief
 * @author  tamada GAME FREAK inc.
 * @date  2011.02.15
 *
 * @todo  ダウンロードデータのサイズ、項目確認（肩書の長さなど）
 */
//============================================================================================
#pragma once

#include "wbt.h"

#include "battle/battle.h"
#include "poke_tool/poketype_def.h" //POKETYPE_MAX
#include "waza_tool/wazano_def.h" //WAZANO_MAX
#include "item/itemsym.h"   //ITEM_DATA_MAX


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  WBT_DL_POKENUMBER_MAX = MATH_ROUNDUP( MONSNO_END, 8 ) / 8,
  WBT_DL_POKEWAZA_MAX = MATH_ROUNDUP( WAZANO_MAX, 8 ) / 8,
  WBT_DL_POKEITEM_MAX = MATH_ROUNDUP( ITEM_DATA_MAX, 8 ) / 8,
  WBT_DL_SHOOTERITEM_MAX = MATH_ROUNDUP( SHOOTER_ITEM_MAX, 8 ) / 8,
};
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  BtlRule rule;
  ///ポケモン制限フラグ（ずかんIDと1対1)
  u8 poke_id_mask[ WBT_DL_POKENUMBER_MAX ];
  ///手持ちに伝説ポケモン2体まで許可するか？
  BOOL limit_legend_poke;
  ///わざ制限フラグ（わざIDと1対1)
  u8 waza_id_mask[ WBT_DL_POKEWAZA_MAX ];
  ///手持ちに同じポケモンを許可するか？
  BOOL limit_same_poke;
  ///所持どうぐ制限（どうぐIDと1対1）
  u8 item_id_mask[ WBT_DL_POKEITEM_MAX ];

  u8 limit_level;
  u8 limit_level_type;
  u8 limit_type;
  BOOL limit_shooter_battle;
  u8 limit_shooter_item_mask[ WBT_DL_SHOOTERITEM_MAX ];
  u8 need_poke_number;
  u8 need_poke_form_number;
  PokeSick start_status;
  BtlWeather weather;
  u8 limit_player_type;
}WBT_DL_RULE;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  u16 monsno;
  u8 form_no;
  STRCODE nickname[MONS_NAME_SIZE+EOM_SIZE];
  u8 rare_flag  :1;
  u8 sex        :2;
  u16 waza_id[PTL_WAZA_MAX];
  u8 tokusei;
  u8 seikaku;
  u16 item_no;

	u32	hp_rnd			:5;					//		HP乱数
	u32	pow_rnd			:5;					//		攻撃力乱数
	u32	def_rnd			:5;					//		防御力乱数
	u32	agi_rnd			:5;					//		素早さ乱数
	u32	spepow_rnd		:5;					//		特攻乱数
	u32	spedef_rnd		:5;					//		特防乱数
  u32 pad         :2;

	u8	hp_exp;								//11h	HP努力値
	u8	pow_exp;							//12h	攻撃力努力値
	u8	def_exp;							//13h	防御力努力値
	u8	agi_exp;							//14h	素早さ努力値
	u8	spepow_exp;							//15h	特攻努力値
	u8	spedef_exp;							//16h	特防努力値

  u8  friend_rate;        ///<なつき度

}WBT_DL_POKEMON;

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  WBT_DL_MSG_LEN = 36 + 1,
  WBT_DL_TRAINER_TITLE_LEN = 10 + 1,

  WBT_DL_TOURNAMENT_NAME_LEN = 20 + 1,

  WBT_DL_TOURNAMENT_REG_EXPLAIN_LEN = (18+1)*3*2,
  WBT_DL_TOURNAMENT_EXPLAIN_LEN = (18+1)*2*2,

  WBT_DL_CHAR_MAX = 7,
};

//------------------------------------------------------------------
/**
 * @brief WBT DownLoad キャラクターデータ
 */
//------------------------------------------------------------------
typedef struct {
  STRCODE name[ SAVELEN_TRAINER_NAME ];
  STRCODE title[ WBT_DL_TRAINER_TITLE_LEN ];
  u16 obj_id;
  WBT_DL_POKEMON pokemon[ TEMOTI_POKEMAX ];
  u8 priority_bit;
  u8 type;
  u16 ai_bit; 
  STRCODE msg_before[ WBT_DL_MSG_LEN ];
  STRCODE msg_after[ WBT_DL_MSG_LEN ];
  STRCODE msg_win[ WBT_DL_MSG_LEN ];
  STRCODE msg_lose[ WBT_DL_MSG_LEN ];
  u16 bgm_id[3];
  u32 check_sum;
}WBT_DL_CHARACTER;

//------------------------------------------------------------------
/**
 * @brief WBT DownLoad トーナメントデータ
 */
//------------------------------------------------------------------
typedef struct {
  WBT_DL_RULE regulation;
  WBT_DL_CHARACTER character[ WBT_DL_CHAR_MAX ];

  //トーナメント名
  STRCODE tournament_name[ WBT_DL_TOURNAMENT_NAME_LEN ];
  //トーナメントの内容説明
  STRCODE contents_explain[ WBT_DL_TOURNAMENT_EXPLAIN_LEN ];
  //トーナメントにおいてのレギュレーション説明
  STRCODE regulation_explain[ WBT_DL_TOURNAMENT_REG_EXPLAIN_LEN ];
  ///配信開始年月日
  u8 year, month, day;
  ///受信可能なカセットバージョン
  u32 cassette_version;
  ///言語ＩＤ
  u8 lang_id;
  ///トーナメントＩＤ
  u16 tournament_id;
}WBT_DL_DATA;

