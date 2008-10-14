/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     poketool.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*! @file
	@brief	ポケモンデータを取り扱うためのライブラリ
	
	@author	kitayama(kitayama_shigetoshi@nintendo.co.jp)
	
	@version 0.03 (2006/09/13)
		@li	B_TOWER_POKEMON::form_no を追加しました。
		@li PokeTool_GetPokeParam() でフォームナンバーを取得できるようにしました。
			ただし、モンスターナンバー493のアルセウスだけは取得できません。
			（現状ではアサートになりますので、アルセウスに対してフォームナンバーを取得
			しないでください。）
		@li	PokeTool_IsRare(), PokeTool_BtIsRare(), PokeTool_HasMail(), PokeTool_GetMailLang()
			を追加しました。
		@li PokeTool_GetDefaultName(), PokeTool_SetNickNameDefault() を追加しました。
			ただし、これらは現状UNIX環境では正常に動作しません。
	@version 0.01 (2006/07/20)
		@li 新規リリースしました。
*/

#ifndef POKETOOL_H_
#define POKETOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dpw_typedef.h"

/*-----------------------------------------------------------------------*
					型・定数宣言
 *-----------------------------------------------------------------------*/

/*======== ポケモンのソースからコピーしてきた、ポケモンのパラメータを表す型 ========*/

typedef u16		STRCODE;
typedef	u16		PMS_WORD;

typedef struct _CUSTOM_BALL_SEAL	CB_SEAL;	// シールのデータ
typedef struct _CUSTOM_BALL_CORE	CB_CORE;	// カスタムボールのデータ

typedef struct pokemon_paso_param1 POKEMON_PASO_PARAM1;
typedef struct pokemon_paso_param2 POKEMON_PASO_PARAM2;
typedef struct pokemon_paso_param3 POKEMON_PASO_PARAM3;
typedef struct pokemon_paso_param4 POKEMON_PASO_PARAM4;

// ボックスポケモン分の構造体
typedef struct pokemon_paso_param POKEMON_PASO_PARAM;

//ボックスポケモン分以外の構造体
typedef struct pokemon_calc_param POKEMON_CALC_PARAM;

//! 手持ちポケモンの構造体
typedef struct pokemon_param POKEMON_PARAM;

//! バトルタワーで使用されるポケモンのデータ
typedef struct _B_TOWER_POKEMON	B_TOWER_POKEMON;

typedef struct _MAIL_DATA MAIL_DATA;

#define	EOM_SIZE			1		//!< 終了コードの長さ
#define	EOM_			0xffff		//!< 終了コード

#define	WAZA_TEMOTI_MAX		(4)		//!< 1体のポケモンがもてる技の最大値
#define MONS_NAME_SIZE		10		//!< ポケモン名の長さ(バッファサイズ EOM_含まず)
#define PERSON_NAME_SIZE	7		//!< 人物の名前の長さ（自分も含む）

#define MAILDAT_ICONMAX		(3)		//!< ポケモンアイコン要素数
#define PMS_WORD_MAX		(2)		//!< 文中に含まれる単語の最大数
#define MAILDAT_MSGMAX		(3)		//!< 簡易文要素数

#define CB_DEF_SEAL_MAX		(8)		//!< 貼れる枚数

#define POKEMON_NONE_NAME	L"NONE"	//!< ポケモンデフォルト名がL""だった場合の名前

//! ポケモンパラメータデータ取得のためのインデックス
typedef enum {
	ID_PARA_personal_rnd=0,						//!< 個性乱数
	ID_PARA_pp_fast_mode,						//
	ID_PARA_ppp_fast_mode,						//
	ID_PARA_checksum,							//!< チェックサム

	ID_PARA_monsno,								//!< モンスターナンバー
	ID_PARA_item,								//!< 所持アイテムナンバー
	ID_PARA_id_no,								//!< IDNo
	ID_PARA_exp,								//!< 経験値
	ID_PARA_friend,								//!< なつき度
	ID_PARA_speabino,							//!< 特殊能力
	ID_PARA_mark,								//!< ポケモンにつけるマーク（ボックス）
	ID_PARA_country_code,						//!< 国コード
	ID_PARA_hp_exp,								//!< HP努力値
	ID_PARA_pow_exp,							//!< 攻撃力努力値
	ID_PARA_def_exp,							//!< 防御力努力値
	ID_PARA_agi_exp,							//!< 素早さ努力値
	ID_PARA_spepow_exp,							//!< 特攻努力値
	ID_PARA_spedef_exp,							//!< 特防努力値
	ID_PARA_style,								//!< かっこよさ
	ID_PARA_beautiful,							//!< うつくしさ
	ID_PARA_cute,								//!< かわいさ
	ID_PARA_clever,								//!< かしこさ
	ID_PARA_strong,								//!< たくましさ
	ID_PARA_fur,								//!< 毛艶
	ID_PARA_sinou_champ_ribbon,					//!< シンオウチャンプリボン
	ID_PARA_sinou_battle_tower_ttwin_first,		//!< シンオウバトルタワータワータイクーン勝利1回目
	ID_PARA_sinou_battle_tower_ttwin_second,	//!< シンオウバトルタワータワータイクーン勝利2回目
	ID_PARA_sinou_battle_tower_2vs2_win50,		//!< シンオウバトルタワータワーダブル50連勝
	ID_PARA_sinou_battle_tower_aimulti_win50,	//!< シンオウバトルタワータワーAIマルチ50連勝
	ID_PARA_sinou_battle_tower_siomulti_win50,	//!< シンオウバトルタワータワー通信マルチ50連勝
	ID_PARA_sinou_battle_tower_wifi_rank5,		//!< シンオウバトルタワーWifiランク５入り
	ID_PARA_sinou_syakki_ribbon,				//!< シンオウしゃっきリボン
	ID_PARA_sinou_dokki_ribbon,					//!< シンオウどっきリボン
	ID_PARA_sinou_syonbo_ribbon,				//!< シンオウしょんぼリボン
	ID_PARA_sinou_ukka_ribbon,					//!< シンオウうっかリボン
	ID_PARA_sinou_sukki_ribbon,					//!< シンオウすっきリボン
	ID_PARA_sinou_gussu_ribbon,					//!< シンオウぐっすリボン
	ID_PARA_sinou_nikko_ribbon,					//!< シンオウにっこリボン
	ID_PARA_sinou_gorgeous_ribbon,				//!< シンオウゴージャスリボン
	ID_PARA_sinou_royal_ribbon,					//!< シンオウロイヤルリボン
	ID_PARA_sinou_gorgeousroyal_ribbon,			//!< シンオウゴージャスロイヤルリボン
	ID_PARA_sinou_ashiato_ribbon,				//!< シンオウあしあとリボン
	ID_PARA_sinou_record_ribbon,				//!< シンオウレコードリボン
	ID_PARA_sinou_history_ribbon,				//!< シンオウヒストリーリボン
	ID_PARA_sinou_legend_ribbon,				//!< シンオウレジェンドリボン
	ID_PARA_sinou_red_ribbon,					//!< シンオウレッドリボン
	ID_PARA_sinou_green_ribbon,					//!< シンオウグリーンリボン
	ID_PARA_sinou_blue_ribbon,					//!< シンオウブルーリボン
	ID_PARA_sinou_festival_ribbon,				//!< シンオウフェスティバルリボン
	ID_PARA_sinou_carnival_ribbon,				//!< シンオウカーニバルリボン
	ID_PARA_sinou_classic_ribbon,				//!< シンオウクラシックリボン
	ID_PARA_sinou_premiere_ribbon,				//!< シンオウプレミアリボン
	ID_PARA_sinou_amari_ribbon,					//!< あまり

	ID_PARA_waza1,								//!< 所持技1
	ID_PARA_waza2,								//!< 所持技2
	ID_PARA_waza3,								//!< 所持技3
	ID_PARA_waza4,								//!< 所持技4
	ID_PARA_pp1,								//!< 所持技PP1
	ID_PARA_pp2,								//!< 所持技PP2
	ID_PARA_pp3,								//!< 所持技PP3
	ID_PARA_pp4,								//!< 所持技PP4
	ID_PARA_pp_count1,							//!< 所持技PP_COUNT1
	ID_PARA_pp_count2,							//!< 所持技PP_COUNT2
	ID_PARA_pp_count3,							//!< 所持技PP_COUNT3
	ID_PARA_pp_count4,							//!< 所持技PP_COUNT4
	ID_PARA_pp_max1,							//!< 所持技PPMAX1
	ID_PARA_pp_max2,							//!< 所持技PPMAX2
	ID_PARA_pp_max3,							//!< 所持技PPMAX3
	ID_PARA_pp_max4,							//!< 所持技PPMAX4
	ID_PARA_hp_rnd,								//!< HP乱数
	ID_PARA_pow_rnd,							//!< 攻撃力乱数
	ID_PARA_def_rnd,							//!< 防御力乱数
	ID_PARA_agi_rnd,							//!< 素早さ乱数
	ID_PARA_spepow_rnd,							//!< 特攻乱数
	ID_PARA_spedef_rnd,							//!< 特防乱数
	ID_PARA_tamago_flag,						//!< タマゴフラグ
	ID_PARA_nickname_flag,						//!< ニックネームをつけたかどうかフラグ
	ID_PARA_stylemedal_normal,					//!< かっこよさ勲章(ノーマル)AGBコンテスト
	ID_PARA_stylemedal_super,					//!< かっこよさ勲章(スーパー)AGBコンテスト
	ID_PARA_stylemedal_hyper,					//!< かっこよさ勲章(ハイパー)AGBコンテスト
	ID_PARA_stylemedal_master,					//!< かっこよさ勲章(マスター)AGBコンテスト
	ID_PARA_beautifulmedal_normal,				//!< うつくしさ勲章(ノーマル)AGBコンテスト
	ID_PARA_beautifulmedal_super,				//!< うつくしさ勲章(スーパー)AGBコンテスト
	ID_PARA_beautifulmedal_hyper,				//!< うつくしさ勲章(ハイパー)AGBコンテスト
	ID_PARA_beautifulmedal_master,				//!< うつくしさ勲章(マスター)AGBコンテスト
	ID_PARA_cutemedal_normal,					//!< かわいさ勲章(ノーマル)AGBコンテスト
	ID_PARA_cutemedal_super,					//!< かわいさ勲章(スーパー)AGBコンテスト
	ID_PARA_cutemedal_hyper,					//!< かわいさ勲章(ハイパー)AGBコンテスト
	ID_PARA_cutemedal_master,					//!< かわいさ勲章(マスター)AGBコンテスト
	ID_PARA_clevermedal_normal,					//!< かしこさ勲章(ノーマル)AGBコンテスト
	ID_PARA_clevermedal_super,					//!< かしこさ勲章(スーパー)AGBコンテスト
	ID_PARA_clevermedal_hyper,					//!< かしこさ勲章(ハイパー)AGBコンテスト
	ID_PARA_clevermedal_master,					//!< かしこさ勲章(マスター)AGBコンテスト
	ID_PARA_strongmedal_normal,					//!< たくましさ勲章(ノーマル)AGBコンテスト
	ID_PARA_strongmedal_super,					//!< たくましさ勲章(スーパー)AGBコンテスト
	ID_PARA_strongmedal_hyper,					//!< たくましさ勲章(ハイパー)AGBコンテスト
	ID_PARA_strongmedal_master,					//!< たくましさ勲章(マスター)AGBコンテスト
	ID_PARA_champ_ribbon,						//!< チャンプリボン
	ID_PARA_winning_ribbon,						//!< ウィニングリボン
	ID_PARA_victory_ribbon,						//!< ビクトリーリボン
	ID_PARA_bromide_ribbon,						//!< ブロマイドリボン
	ID_PARA_ganba_ribbon,						//!< がんばリボン
	ID_PARA_marine_ribbon,						//!< マリンリボン
	ID_PARA_land_ribbon,						//!< ランドリボン
	ID_PARA_sky_ribbon,							//!< スカイリボン
	ID_PARA_country_ribbon,						//!< カントリーリボン
	ID_PARA_national_ribbon,					//!< ナショナルリボン
	ID_PARA_earth_ribbon,						//!< アースリボン
	ID_PARA_world_ribbon,						//!< ワールドリボン
	ID_PARA_event_get_flag,						//!< イベントで配布されたことを示すフラグ
	ID_PARA_sex,								//!< 性別
	ID_PARA_form_no,							//!< 形状ナンバー（アンノーン、デオキシス、ミノメスなど用）
	ID_PARA_dummy_p2_1,							//!< あまり
	ID_PARA_dummy_p2_2,							//!< あまり
	ID_PARA_dummy_p2_3,							//!< あまり

	ID_PARA_nickname,							//!< ニックネーム
	ID_PARA_nickname_buf,						//!< ニックネーム（STRBUF使用）
	ID_PARA_nickname_buf_flag,					//!< ニックネーム（STRBUF使用,nickname_flagもオンにする）
	ID_PARA_pref_code,							//!< 都道府県コード
	ID_PARA_get_cassette,						//!< 捕まえたカセット（カラーバージョン）
	ID_PARA_trial_stylemedal_normal,			//!< かっこよさ勲章(ノーマル)トライアル
	ID_PARA_trial_stylemedal_super,				//!< かっこよさ勲章(スーパー)トライアル
	ID_PARA_trial_stylemedal_hyper,				//!< かっこよさ勲章(ハイパー)トライアル
	ID_PARA_trial_stylemedal_master,			//!< かっこよさ勲章(マスター)トライアル
	ID_PARA_trial_beautifulmedal_normal,		//!< うつくしさ勲章(ノーマル)トライアル
	ID_PARA_trial_beautifulmedal_super,			//!< うつくしさ勲章(スーパー)トライアル
	ID_PARA_trial_beautifulmedal_hyper,			//!< うつくしさ勲章(ハイパー)トライアル
	ID_PARA_trial_beautifulmedal_master,		//!< うつくしさ勲章(マスター)トライアル
	ID_PARA_trial_cutemedal_normal,				//!< かわいさ勲章(ノーマル)トライアル
	ID_PARA_trial_cutemedal_super,				//!< かわいさ勲章(スーパー)トライアル
	ID_PARA_trial_cutemedal_hyper,				//!< かわいさ勲章(ハイパー)トライアル
	ID_PARA_trial_cutemedal_master,				//!< かわいさ勲章(マスター)トライアル
	ID_PARA_trial_clevermedal_normal,			//!< かしこさ勲章(ノーマル)トライアル
	ID_PARA_trial_clevermedal_super,			//!< かしこさ勲章(スーパー)トライアル
	ID_PARA_trial_clevermedal_hyper,			//!< かしこさ勲章(ハイパー)トライアル
	ID_PARA_trial_clevermedal_master,			//!< かしこさ勲章(マスター)トライアル
	ID_PARA_trial_strongmedal_normal,			//!< たくましさ勲章(ノーマル)トライアル
	ID_PARA_trial_strongmedal_super,			//!< たくましさ勲章(スーパー)トライアル
	ID_PARA_trial_strongmedal_hyper,			//!< たくましさ勲章(ハイパー)トライアル
	ID_PARA_trial_strongmedal_master,			//!< たくましさ勲章(マスター)トライアル
	ID_PARA_amari_ribbon,						//!< 余りリボン

	ID_PARA_oyaname,							//!< 親の名前
	ID_PARA_oyaname_buf,						//!< 親の名前（STRBUF使用）
	ID_PARA_get_year,							//!< 捕まえた年
	ID_PARA_get_month,							//!< 捕まえた月
	ID_PARA_get_day,							//!< 捕まえた日
	ID_PARA_birth_year,							//!< 生まれた年
	ID_PARA_birth_month,						//!< 生まれた月
	ID_PARA_birth_day,							//!< 生まれた日
	ID_PARA_get_place,							//!< 捕まえた場所
	ID_PARA_birth_place,						//!< 生まれた場所
	ID_PARA_pokerus,							//!< ポケルス
	ID_PARA_get_ball,							//!< 捕まえたボール
	ID_PARA_get_level,							//!< 捕まえたレベル
	ID_PARA_oyasex,								//!< 親の性別
	ID_PARA_get_ground_id,						//!< 捕まえた場所の地形アトリビュート（ミノッチ用）
	ID_PARA_dummy_p4_1,							//!< あまり

	ID_PARA_condition,							//!< コンディション
	ID_PARA_level,								//!< レベル
	ID_PARA_cb_id,								//!< カスタムボールID
	ID_PARA_hp,									//!< HP
	ID_PARA_hpmax,								//!< HPMAX
	ID_PARA_pow,								//!< 攻撃力
	ID_PARA_def,								//!< 防御力
	ID_PARA_agi,								//!< 素早さ
	ID_PARA_spepow,								//!< 特攻
	ID_PARA_spedef,								//!< 特防
	ID_PARA_mail_data,							//!< メールデータ
	ID_PARA_cb_core,							//!< カスタムボールデータ
	
	ID_PARA_poke_exist,							//!< ポケモン存在フラグ
	ID_PARA_tamago_exist,						//!< タマゴ存在フラグ

	ID_PARA_monsno_egg,							//!< タマゴかどうかも同時にチェック

	ID_PARA_power_rnd,							//!< パワー乱数をu32で扱う
	ID_PARA_nidoran_nickname,					//!< モンスターナンバーがニドランの時にnickname_flagが立っているかチェック

	ID_PARA_type1,								//!< ポケモンのタイプ１を取得（アウス、マルチタイプチェックもあり）
	ID_PARA_type2,								//!< ポケモンのタイプ２を取得（アウス、マルチタイプチェックもあり）

	ID_PARA_default_name,						//!< ポケモンのデフォルト名

	ID_PARA_end									// パラメータを追加するときは、ここから上に追加
} PokeToolParamID;

enum COUNTRY_CODE{          // 言語コード詳細
	COUNTRY_CODE_JP = 1,    // 1, 日本語
	COUNTRY_CODE_US,        // 2, 英語
	COUNTRY_CODE_FR,        // 3, フランス語
	COUNTRY_CODE_IT,        // 4, イタリア語
	COUNTRY_CODE_DE,        // 5, ドイツ語
	COUNTRY_CODE_RESERVE,   // 6, 未使用
	COUNTRY_CODE_SP,        // 7, スペイン語
    COUNTRY_CODE_KR,        // 8, 韓国語
    COUNTRY_CODE_MAX = 8	//!< 国コードの個数
};

enum GENDER
{
    GENDER_MALE = 0,
    GENDER_FEMALE = 1,
    GENDER_NONE = 2
};

struct pokemon_paso_param1
{
	u16	monsno;								//02h	モンスターナンバー
	u16 item;								//04h	所持アイテムナンバー
	u32	id_no;								//08h	IDNo
	u32	exp;								//0ch	経験値
	u8	_friend;							//0dh	なつき度
	u8	speabino;							//0eh	特殊能力
	u8	mark;								//0fh	ポケモンにつけるマーク（ボックス）
	u8	country_code;						//10h	国コード
	u8	hp_exp;								//11h	HP努力値
	u8	pow_exp;							//12h	攻撃力努力値
	u8	def_exp;							//13h	防御力努力値
	u8	agi_exp;							//14h	素早さ努力値
	u8	spepow_exp;							//15h	特攻努力値
	u8	spedef_exp;							//16h	特防努力値
	u8	style;								//17h	かっこよさ
	u8	beautiful;							//18h	うつくしさ
	u8	cute;								//19h	かわいさ
	u8	clever;								//1ah	かしこさ
	u8	strong;								//1bh	たくましさ
	u8	fur;								//1ch	毛艶
	u32	sinou_ribbon;						//20h	シンオウ系リボン（ビットフィールドでアクセスするとプログラムメモリを消費するので、ビットシフトに変更）
//以下内訳
#if 0
	u32	sinou_champ_ribbon					:1;	//シンオウチャンプリボン
	u32	sinou_battle_tower_ttwin_first		:1;	//シンオウバトルタワータワータイクーン勝利1回目
	u32	sinou_battle_tower_ttwin_second		:1;	//シンオウバトルタワータワータイクーン勝利1回目
	u32	sinou_battle_tower_2vs2_win50		:1;	//シンオウバトルタワータワーダブル50連勝
	u32	sinou_battle_tower_aimulti_win50	:1;	//シンオウバトルタワータワーAIマルチ50連勝
	u32	sinou_battle_tower_siomulti_win50	:1;	//シンオウバトルタワータワー通信マルチ50連勝
	u32	sinou_battle_tower_wifi_rank5		:1;	//シンオウバトルタワーWifiランク５入り
	u32	sinou_syakki_ribbon					:1;	//シンオウしゃっきリボン
	u32	sinou_dokki_ribbon					:1;	//シンオウどっきリボン
	u32	sinou_syonbo_ribbon					:1;	//シンオウしょんぼリボン

	u32	sinou_ukka_ribbon					:1;	//シンオウうっかリボン
	u32	sinou_sukki_ribbon					:1;	//シンオウすっきリボン
	u32	sinou_gussu_ribbon					:1;	//シンオウぐっすリボン
	u32	sinou_nikko_ribbon					:1;	//シンオウにっこリボン
	u32	sinou_gorgeous_ribbon				:1;	//シンオウゴージャスリボン
	u32	sinou_royal_ribbon					:1;	//シンオウロイヤルリボン
	u32	sinou_gorgeousroyal_ribbon			:1;	//シンオウゴージャスロイヤルリボン
	u32	sinou_ashiato_ribbon				:1;	//シンオウあしあとリボン
	u32	sinou_record_ribbon					:1;	//シンオウレコードリボン
	u32	sinou_history_ribbon				:1;	//シンオウヒストリーリボン

	u32	sinou_legend_ribbon					:1;	//シンオウレジェンドリボン
	u32	sinou_red_ribbon					:1;	//シンオウレッドリボン
	u32	sinou_green_ribbon					:1;	//シンオウグリーンリボン
	u32	sinou_blue_ribbon					:1;	//シンオウブルーリボン
	u32	sinou_festival_ribbon				:1;	//シンオウフェスティバルリボン
	u32	sinou_carnival_ribbon				:1;	//シンオウカーニバルリボン
	u32	sinou_classic_ribbon				:1;	//シンオウクラシックリボン
	u32	sinou_premiere_ribbon				:1;	//シンオウプレミアリボン

	u32	sinou_amari_ribbon					:4;	//20h	あまり
#endif
};
	
struct pokemon_paso_param2
{
	u16	waza[WAZA_TEMOTI_MAX];				//08h	所持技
	u8	pp[WAZA_TEMOTI_MAX];				//0ch	所持技PP
	u8	pp_count[WAZA_TEMOTI_MAX];			//10h	所持技PP_COUNT
	u32	hp_rnd			:5;					//		HP乱数
	u32	pow_rnd			:5;					//		攻撃力乱数
	u32	def_rnd			:5;					//		防御力乱数
	u32	agi_rnd			:5;					//		素早さ乱数
	u32	spepow_rnd		:5;					//		特攻乱数
	u32	spedef_rnd		:5;					//		特防乱数
	u32	tamago_flag		:1;					//		タマゴフラグ（0:タマゴじゃない　1:タマゴだよ）
	u32	nickname_flag	:1;					//14h	ニックネームをつけたかどうかフラグ（0:つけていない　1:つけた）

	u32	old_ribbon;							//18h	過去作のリボン系（ビットフィールドでアクセスするとプログラムメモリを消費するので、ビットシフトに変更）
//以下内訳
#if 0
	u32	stylemedal_normal		:1;			//	かっこよさ勲章(ノーマル)(AGBコンテスト)
	u32	stylemedal_super		:1;			//	かっこよさ勲章(スーパー)(AGBコンテスト)
	u32	stylemedal_hyper		:1;			//	かっこよさ勲章(ハイパー)(AGBコンテスト)
	u32	stylemedal_master		:1;			//	かっこよさ勲章(マスター)(AGBコンテスト)
	u32	beautifulmedal_normal	:1;			//	うつくしさ勲章(ノーマル)(AGBコンテスト)
	u32	beautifulmedal_super	:1;			//	うつくしさ勲章(スーパー)(AGBコンテスト)
	u32	beautifulmedal_hyper	:1;			//	うつくしさ勲章(ハイパー)(AGBコンテスト)
	u32	beautifulmedal_master	:1;			//	うつくしさ勲章(マスター)(AGBコンテスト)
	u32	cutemedal_normal		:1;			//	かわいさ勲章(ノーマル)(AGBコンテスト)
	u32	cutemedal_super			:1;			//	かわいさ勲章(スーパー)(AGBコンテスト)
	u32	cutemedal_hyper			:1;			//	かわいさ勲章(ハイパー)(AGBコンテスト)
	u32	cutemedal_master		:1;			//	かわいさ勲章(マスター)(AGBコンテスト)
	u32	clevermedal_normal		:1;			//	かしこさ勲章(ノーマル)(AGBコンテスト)
	u32	clevermedal_super		:1;			//	かしこさ勲章(スーパー)(AGBコンテスト)
	u32	clevermedal_hyper		:1;			//	かしこさ勲章(ハイパー)(AGBコンテスト)
	u32	clevermedal_master		:1;			//	かしこさ勲章(マスター)(AGBコンテスト)
	u32	strongmedal_normal		:1;			//	たくましさ勲章(ノーマル)(AGBコンテスト)
	u32	strongmedal_super		:1;			//	たくましさ勲章(スーパー)(AGBコンテスト)
	u32	strongmedal_hyper		:1;			//	たくましさ勲章(ハイパー)(AGBコンテスト)
	u32	strongmedal_master		:1;			//	たくましさ勲章(マスター)(AGBコンテスト)

	u32	champ_ribbon	:1;					//		チャンプリボン
	u32	winning_ribbon	:1;					//		ウィニングリボン
	u32	victory_ribbon	:1;					//		ビクトリーリボン
	u32	bromide_ribbon	:1;					//		ブロマイドリボン
	u32	ganba_ribbon	:1;					//		がんばリボン
	u32	marine_ribbon	:1;					//		マリンリボン
	u32	land_ribbon		:1;					//		ランドリボン
	u32	sky_ribbon		:1;					//		スカイリボン
	u32	country_ribbon	:1;					//		カントリーリボン
	u32	national_ribbon	:1;					//		ナショナルリボン
	u32	earth_ribbon	:1;					//		アースリボン
	u32	world_ribbon	:1;					//		ワールドリボン
#endif

	u8	event_get_flag	:1;					//		イベントで配布したことを示すフラグ
	u8	sex				:2;					//   	ポケモンの性別
	u8	form_no			:5;					//19h	形状ナンバー（アンノーン、デオキシス、ミノメスなど用）
	u8	dummy_p2_1;							//1ah	あまり
	u16	dummy_p2_2;							//1ch	あまり
	u32	dummy_p2_3;							//20h	あまり
};
	
struct pokemon_paso_param3
{
	STRCODE	nickname[MONS_NAME_SIZE+EOM_SIZE];	//16h	ニックネーム(MONS_NAME_SIZE=10)+(EOM_SIZE=1)=11
	u8	pref_code;								//18h	都道府県コード
	u8	get_cassette;							//		捕まえたカセットバージョン
	u64	new_ribbon;								//20h	新リボン系（ビットフィールドでアクセスするとプログラムメモリを消費するので、ビットシフトに変更）
//以下内訳
#if 0
	u32	trial_stylemedal_normal			:1;		//	かっこよさ勲章(ノーマル)(トライアル)
	u32	trial_stylemedal_super			:1;		//	かっこよさ勲章(スーパー)(トライアル)
	u32	trial_stylemedal_hyper			:1;		//	かっこよさ勲章(ハイパー)(トライアル)
	u32	trial_stylemedal_master			:1;		//	かっこよさ勲章(マスター)(トライアル)
	u32	trial_beautifulmedal_normal		:1;		//	うつくしさ勲章(ノーマル)(トライアル)
	u32	trial_beautifulmedal_super		:1;		//	うつくしさ勲章(スーパー)(トライアル)
	u32	trial_beautifulmedal_hyper		:1;		//	うつくしさ勲章(ハイパー)(トライアル)
	u32	trial_beautifulmedal_master		:1;		//	うつくしさ勲章(マスター)(トライアル)
	u32	trial_cutemedal_normal			:1;		//	かわいさ勲章(ノーマル)(トライアル)
	u32	trial_cutemedal_super			:1;		//	かわいさ勲章(スーパー)(トライアル)
	u32	trial_cutemedal_hyper			:1;		//	かわいさ勲章(ハイパー)(トライアル)
	u32	trial_cutemedal_master			:1;		//	かわいさ勲章(マスター)(トライアル)
	u32	trial_clevermedal_normal		:1;		//	かしこさ勲章(ノーマル)(トライアル)
	u32	trial_clevermedal_super			:1;		//	かしこさ勲章(スーパー)(トライアル)
	u32	trial_clevermedal_hyper			:1;		//	かしこさ勲章(ハイパー)(トライアル)
	u32	trial_clevermedal_master		:1;		//	かしこさ勲章(マスター)(トライアル)
	u32	trial_strongmedal_normal		:1;		//	たくましさ勲章(ノーマル)(トライアル)
	u32	trial_strongmedal_super			:1;		//	たくましさ勲章(スーパー)(トライアル)
	u32	trial_strongmedal_hyper			:1;		//	たくましさ勲章(ハイパー)(トライアル)
	u32	trial_strongmedal_master		:1;		//	たくましさ勲章(マスター)(トライアル)
	u32 amari_ribbon					:12;	//20h	あまり
	u32	amari;									//20h	あまり
#endif
};
	
struct pokemon_paso_param4
{
	STRCODE	oyaname[7+EOM_SIZE];		//10h	親の名前(PERSON_NAME_SIZE=7)+(EOM_SIZE_=1)=8*2(STRCODE=u16)

	u8	get_year;						//11h	捕まえた年
	u8	get_month;						//12h	捕まえた月
	u8	get_day;						//13h	捕まえた日
	u8	birth_year;						//14h	生まれた年

	u8	birth_month;					//15h	生まれた月
	u8	birth_day;						//16h	生まれた日
	u16	get_place;						//18h	捕まえた場所

	u16	birth_place;					//1ah	生まれた場所
	u8	pokerus;						//1bh	ポケルス
	u8	get_ball;						//1ch	捕まえたボール

	u8	get_level		:7;				//1dh	捕まえたレベル
	u8	oyasex			:1;				//1dh	親の性別
	u8	get_ground_id;					//1eh	捕まえた場所の地形アトリビュート（ミノッチ用）
	u16	dummy_p4_1;						//20h	あまり
};

// ボックスポケモン分の構造体宣言
struct pokemon_paso_param
{
	u32	personal_rnd;					//04h	個性乱数
	u16	pp_fast_mode	:1;				//06h	暗号／復号／チェックサム生成を後回しにして、処理を高速化モード
	u16	ppp_fast_mode	:1;				//06h	暗号／復号／チェックサム生成を後回しにして、処理を高速化モード
	u16					:14;			//06h
	u16	checksum;						//08h	チェックサム

	u8	paradata[sizeof(POKEMON_PASO_PARAM1)+
				 sizeof(POKEMON_PASO_PARAM2)+
				 sizeof(POKEMON_PASO_PARAM3)+
				 sizeof(POKEMON_PASO_PARAM4)];	//88h
};

// メールポケモンアイコンデータ型
typedef union _MAIL_ICON{
	struct{
		u16	cgxID:12;	// ポケモンアイコンキャラクタCgxID
		u16	palID:4;	// ポケモンアイコンパレットID
	};
	u16	dat;
}MAIL_ICON;

typedef struct {
	u16				sentence_type;			// 文章タイプ
	u16				sentence_id;			// タイプ内ID
	PMS_WORD		word[PMS_WORD_MAX];		// 単語ID
}PMS_DATA;

typedef struct _MAIL_DATA{
	u32	writerID;	// トレーナーID 4
	u8	sex;		// 主人公の性別 1
	u8	region;		// 国コード 1
	u8	version;	// カセットバージョン 1
	u8	design;		// デザインナンバー 1
	STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];	// 16
	MAIL_ICON	icon[MAILDAT_ICONMAX+1];		// アイコンNO格納場所[](3+1<padding>) 6+2=8
	PMS_DATA	msg[MAILDAT_MSGMAX];			// 文章データ
}_MAIL_DATA;

struct _CUSTOM_BALL_SEAL {
	u8	seal_id;	// シール番号
	u8	x;			// x 座標
	u8	y;			// y 座標
};

struct _CUSTOM_BALL_CORE {

	CB_SEAL	cb_seal[ CB_DEF_SEAL_MAX ];

};

// ボックスポケモン分以外の構造体宣言
struct pokemon_calc_param
{
	u32			condition;			//04h	コンディション
	u8			level;				//05h	レベル
	u8			cb_id;				//06h	カスタムボールID
	u16			hp;					//08h	HP
	u16			hpmax;				//0ah	HPMAX
	u16			pow;				//0ch	攻撃力
	u16			def;				//0eh	防御力
	u16			agi;				//10h	素早さ
	u16			spepow;				//12h	特攻
	u16			spedef;				//14h	特防
	_MAIL_DATA	mail_data;			//3ch	ポケモンにもたせるメールデータ
	CB_CORE		cb_core;			//54h	カスタムボール
};

//! 手持ちポケモンの構造体宣言
struct	pokemon_param
{
	POKEMON_PASO_PARAM	ppp;		//!< ボックスポケモン分の構造体
	POKEMON_CALC_PARAM	pcp;		//!< ボックスポケモン分以外の構造体
};

//! タワー用ポケモンデータ型
struct _B_TOWER_POKEMON{
	union{
        struct{
            u16    mons_no    :11;    ///<モンスターナンバー
            u16    form_no    :5;    ///<モンスターフォームナンバー
        };
    };

    u16    item_no;    ///<装備道具

    u16    waza[WAZA_TEMOTI_MAX];        ///<所持技

    u32    id_no;                    ///<IDNo
    u32    personal_rnd;            ///<個性乱数

    union{
        struct{
        u32    hp_rnd        :5;        ///<HPパワー乱数
        u32    pow_rnd        :5;        ///<POWパワー乱数
        u32    def_rnd        :5;        ///<DEFパワー乱数
        u32    agi_rnd        :5;        ///<AGIパワー乱数
        u32    spepow_rnd    :5;        ///<SPEPOWパワー乱数
        u32    spedef_rnd    :5;        ///<SPEDEFパワー乱数
        u32    ngname_f    :1;        ///<NGネームフラグ
        u32                :1;        //1ch 余り
        };
        u32 power_rnd;
    };

    union{
        struct{
        u8    hp_exp;                ///<HP努力値
        u8    pow_exp;            ///<POW努力値
        u8    def_exp;            ///<DEF努力値
        u8    agi_exp;            ///<AGI努力値
        u8    spepow_exp;            ///<SPEPOW努力値
        u8    spedef_exp;            ///<SPEDEF努力値
        };
        u8 exp[6];
    };
    union{
        struct{
        u8    pp_count0:2;    ///<技1ポイントアップ
        u8    pp_count1:2;    ///<技2ポイントアップ
        u8    pp_count2:2;    ///<技3ポイントアップ
        u8    pp_count3:2;    ///<技4ポイントアップ
        };
        u8 pp_count;
    };

    u8    country_code;            ///<国コード

    u8    tokusei;                ///<特性
    u8    natuki;                ///<なつき度

    ///ニックネーム ((MONS_NAM22E_SIZE:10)+(EOM_SIZE:1))*(STRCODE:u16)=22
    STRCODE    nickname[MONS_NAME_SIZE+EOM_SIZE];
};

/*-----------------------------------------------------------------------*
					グローバル変数定義
 *-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*
					関数外部宣言
 *-----------------------------------------------------------------------*/

void PokeTool_EncodePokemonParam(const POKEMON_PARAM* src, POKEMON_PARAM* dst);
BOOL PokeTool_DecodePokemonParam(const POKEMON_PARAM* src, POKEMON_PARAM* dst);
u32 PokeTool_GetPokeParam(POKEMON_PARAM *pp, PokeToolParamID id, void *buf);
void PokeTool_SetPokeParam(POKEMON_PARAM *pp, PokeToolParamID id, void *buf);
BOOL PokeTool_GetNickNameFlag(POKEMON_PARAM *pp);
void PokeTool_SetNickNameFlag(POKEMON_PARAM *pp, BOOL flag);
BOOL PokeTool_GetNickName(POKEMON_PARAM *pp, u16* buf);
BOOL PokeTool_SetNickName(POKEMON_PARAM *pp, const u16* buf);
BOOL PokeTool_GetParentName(POKEMON_PARAM *pp, u16* buf);
BOOL PokeTool_SetParentName(POKEMON_PARAM *pp, const u16* buf);
BOOL PokeTool_GetMailName(POKEMON_PARAM *pp, u16* buf);
BOOL PokeTool_SetMailName(POKEMON_PARAM *pp, const u16* buf);
BOOL PokeTool_BtGetNickName(B_TOWER_POKEMON* btp, u16* buf);
BOOL PokeTool_BtSetNickName(B_TOWER_POKEMON* btp, const u16* buf);
const wchar_t* PokeTool_GetDefaultName(u32 monsno, u8 countryCode);
BOOL PokeTool_SetNickNameDefault(POKEMON_PARAM *pp);
BOOL PokeTool_IsRare(POKEMON_PARAM *pp);
BOOL PokeTool_BtIsRare(B_TOWER_POKEMON *btp);
BOOL PokeTool_HasMail(POKEMON_PARAM *pp);
u8 PokeTool_GetMailLang(POKEMON_PARAM *pp);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // POKETOOL_H_