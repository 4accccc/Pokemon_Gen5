//============================================================================================
/**
 * @file  wbt_download.h
 * @brief
 * @author  tamada GAME FREAK inc.
 * @date  2011.02.15
 *
 * @todo  ダウンロードデータのサイズ、項目確認（肩書の長さなど）
 */
//============================================================================================
#pragma once

#include "field/wbt.h"

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
  //WBT_DL_POKEITEM_MAX = MATH_ROUNDUP( ITEM_DATA_MAX, 8 ) / 8,
  WBT_DL_POKEITEM_MAX = (608/8),
  WBT_DL_POKEWAZA_MAX = MATH_ROUNDUP( WAZANO_MAX, 8 ) / 8,

  DUMMY_SHOOTER_ITEM_BIT_TBL_MAX = 7,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  u8  dummy_cupNo;
  u8  dummy_ruleNo;

  u8 num_lo;    ///<参加数下限(1-6)
  u8 num_hi;    ///<参加数上限(1-6)

  u8 level;         ///<参加レベル(1-100、0なら無制限）
  u8 level_range;   ///<REGULATION_LEVEL_RANGE_TYPE レベル範囲
  u16 level_total;  ///<レベル合計制限（0ならば制限無し）

  u8 both_pokemon;  ///<同じポケモンを許可するか
  u8 both_item;     ///<同じアイテムを許可するか

  ///ポケモン制限フラグ（ずかんIDと1対1)
  u8 veto_poke_bit[ WBT_DL_POKENUMBER_MAX ];

  ///所持どうぐ制限（どうぐIDと1対1）
  u8 veto_item_bit[ WBT_DL_POKEITEM_MAX ];

  u16 dummy_must_poke;
  u8  dummy_must_poke_form;

  u8  dummy_veto_shooter_item[ DUMMY_SHOOTER_ITEM_BIT_TBL_MAX ];

  u8  dummy_time_vs;
  u8  dummy_time_command;
  u8 nick_name;     ///<ニックネーム表示
  u8  dummy_camera;
  u8  dummy;
  u8  dummy_show_poke;
  u8  dummy_show_poke_time;
  u8 btl_rule;      ///<BtlRule
  u8  dummy_btl_count; 

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
typedef enum {
  WBT_DL_MATCH_TRAINER_RANDOM = 0,  ///<ランダム（ROMデータから）
  WBT_DL_MATCH_TRAINER_DATA,        ///<DLデータ指定
  WBT_DL_MATCH_TRAINER_ROM,         ///<ROMデータ指定
}WBT_DL_MATCH_TRAINER;

typedef struct {
  u8 type;  ///<WBT_DL_MATCH_TRAINER
  u8 pri;
  u16 id;
}WBT_DL_MATCH;

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  WBT_DL_MSG_LEN = 36 * 2 + 1,
  WBT_DL_TRAINER_TITLE_LEN = 10 + 1,

  WBT_DL_TOURNAMENT_NAME_LEN = 20 + 1,

  WBT_DL_TOURNAMENT_REG_EXPLAIN_LEN = (36+1) * 3,
  WBT_DL_TOURNAMENT_EXPLAIN_LEN = (36+1) * 3,

  WBT_DL_CHAR_MAX = 4,
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
  STRCODE msg_before[ WBT_DL_MSG_LEN ];   ///<対戦前MSG
  STRCODE msg_win[ WBT_DL_MSG_LEN ];      ///<相手の負けMSG
  STRCODE msg_lose[ WBT_DL_MSG_LEN ];     ///<相手の勝ちMSG
  STRCODE msg_announce[ WBT_DL_MSG_LEN ]; ///<決勝戦　前口上
  u8  msg_before_id;
  u8  msg_after_win_id;
  u8  msg_after_lose_id;
  u8  msg_win_id;
  u8  msg_lose_id;
  u8  msg_announce_id;
}WBT_DL_CHARACTER;

//------------------------------------------------------------------
/**
 * @brief WBT DownLoad トーナメントデータ
 */
//------------------------------------------------------------------
typedef struct {
  ///配信開始年月日
  u8 year, month, day;

  ///言語ＩＤ 日=1、英=2、仏=3、伊=4、独=5、西=7、韓=8
  u8 lang_id;

  ///受信可能なカセットバージョン
  u32 cassette_version;

  ///トーナメントＩＤ
  u16 tournament_id;

  ///トーナメント名
  STRCODE tournament_name[ WBT_DL_TOURNAMENT_NAME_LEN ];

  ///トーナメントの内容説明
  STRCODE contents_explain[ WBT_DL_TOURNAMENT_EXPLAIN_LEN ];

  ///トーナメントにおいてのレギュレーション説明
  STRCODE regulation_explain[ WBT_DL_TOURNAMENT_REG_EXPLAIN_LEN ];

  ///レギュレーション指定
  WBT_DL_RULE regulation;
  //REGULATION regulation;

  ///ダウンロードトレーナーデータ
  WBT_DL_CHARACTER character[ WBT_DL_CHAR_MAX ];

  ///組み合わせデータ
  WBT_DL_MATCH match[ WBT_TRAINER_NUM - 1 ];

  u32 crc;
}WBT_DL_DATA;


//============================================================================================
//
//  WiFiダウンロードデータ定義
//
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
enum {
  WBT_DL_TOTAL_NUM = 12,
};

//------------------------------------------------------------------
/**
 * @brief ダウンロードデータのかたまり
 */
//------------------------------------------------------------------
typedef struct {
  WBT_DL_DATA data[ WBT_DL_TOTAL_NUM ];
  u32 crc;
}WBT_DL_TOTAL_DATA;





