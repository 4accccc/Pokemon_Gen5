//============================================================================================
/**
 * @file  poke_tool.h
 * @bfief ポケモンパラメータツール郡（WB改定版）
 * @author  HisashiSogabe
 * @date  08.11.12
 */
//============================================================================================
#pragma once

#ifndef __ASM_NO_DEF_
#include <heapsys.h>

#include "savedata\mystatus.h"
#include "poke_tool\poketype.h"
#include "poke_tool\poke_personal.h"
#include "waza_tool\wazadata.h"

#endif __ASM_NO_DEF_

// 性別
#define PTL_SEX_MALE    (0)   ///<オス
#define PTL_SEX_FEMALE  (1)   ///<メス
#define PTL_SEX_UNKNOWN (2)   ///<性別なし

#define PTL_LEVEL_MAX         (100)   //レベルMAX
#define PTL_WAZA_MAX          (4)     ///<1体のポケモンがもてる技の最大値
#define PTL_WAZAPP_COUNT_MAX  (3)     //pp_countMAX値

#define PTL_FRIEND_MAX      (255)   //なつき度MAX値
#define PTL_STYLE_MAX       (255)   //かっこよさMAX値
#define PTL_BEAUTIFUL_MAX   (255)   //うつくしさMAX値
#define PTL_CUTE_MAX        (255)   //かわいさMAX値
#define PTL_CLEVER_MAX      (255)   //かしこさMAX値
#define PTL_STRONG_MAX      (255)   //たくましさMAX値
#define PTL_FUR_MAX         (255)   //毛艶MAX値

#define PTL_FORM_NONE      ( 0 )   //別フォルムなし

//能力値
#define PTL_ABILITY_HP      (0) ///<体力
#define PTL_ABILITY_ATK     (1) ///<攻撃力
#define PTL_ABILITY_DEF     (2) ///<防御力
#define PTL_ABILITY_SPATK   (3) ///<特攻
#define PTL_ABILITY_SPDEF   (4) ///<特防
#define PTL_ABILITY_AGI     (5) ///<素早さ
#define PTL_ABILITY_MAX     (6)
//#define ABILITY_HIT     (6) ///<命中率
//#define ABILITY_AVOID   (7) ///<回避率

//努力値に関する定義
#define PARA_EXP_TOTAL_MAX  ( 510 )   //6個のパラメータの合計値のMAX
#define PARA_EXP_ITEM_MAX ( 100 )   //アイテムで上昇できる限界値
#define PARA_EXP_MAX    ( 255 )   //6個のパラメータのMAX

// マーキングに関する定義
#define POKEPARA_MARKING_ELEMS_MAX  ( 6 ) ///< マーキングに使用する記号の総数

//PokeItemSetのrange引数定義
#define ITEM_RANGE_NORMAL   ( 0 ) //通常確率
#define ITEM_RANGE_HUKUGAN    ( 1 ) //特性ふくがん効果確率

//ポケモンの性格定義
#if 0
#define POKE_PERSONALITY_GANBARIYA    ( 0 )   //がんばりや
#define POKE_PERSONALITY_SAMISHIGARI  ( 1 )   //さみしがり
#define POKE_PERSONALITY_YUUKAN     ( 2 )   //ゆうかん
#define POKE_PERSONALITY_IJIPPARI   ( 3 )   //いじっぱり
#define POKE_PERSONALITY_YANTYA     ( 4 )   //やんちゃ
#define POKE_PERSONALITY_ZUBUTOI    ( 5 )   //ずぶとい
#define POKE_PERSONALITY_SUNAO      ( 6 )   //すなお
#define POKE_PERSONALITY_NONKI      ( 7 )   //のんき
#define POKE_PERSONALITY_WANPAKU    ( 8 )   //わんぱく
#define POKE_PERSONALITY_NOUTENKI   ( 9 )   //のうてんき
#define POKE_PERSONALITY_OKUBYOU    ( 10 )  //おくびょう
#define POKE_PERSONALITY_SEKKATI    ( 11 )  //せっかち
#define POKE_PERSONALITY_MAJIME     ( 12 )  //まじめ
#define POKE_PERSONALITY_YOUKI      ( 13 )  //ようき
#define POKE_PERSONALITY_MUJYAKI    ( 14 )  //むじゃき
#define POKE_PERSONALITY_HIKAEME    ( 15 )  //ひかえめ
#define POKE_PERSONALITY_OTTORI     ( 16 )  //おっとり
#define POKE_PERSONALITY_REISEI     ( 17 )  //れいせい
#define POKE_PERSONALITY_TEREYA     ( 18 )  //てれや
#define POKE_PERSONALITY_UKKARIYA   ( 19 )  //うっかりや
#define POKE_PERSONALITY_ODAYAKA    ( 20 )  //おだやか
#define POKE_PERSONALITY_OTONASHII    ( 21 )  //おとなしい
#define POKE_PERSONALITY_NAMAIKI    ( 22 )  //なまいき
#define POKE_PERSONALITY_SINTYOU    ( 23 )  //しんちょう
#define POKE_PERSONALITY_KIMAGURE   ( 24 )  //きまぐれ
#else
#define PTL_SEIKAKU_GANBARIYA    ( 0 )   ///< がんばりや
#define PTL_SEIKAKU_SAMISHIGARI  ( 1 )   //さみしがり
#define PTL_SEIKAKU_YUUKAN       ( 2 )   //ゆうかん
#define PTL_SEIKAKU_IJIPPARI     ( 3 )   //いじっぱり
#define PTL_SEIKAKU_YANTYA       ( 4 )   //やんちゃ
#define PTL_SEIKAKU_ZUBUTOI      ( 5 )   //ずぶとい
#define PTL_SEIKAKU_SUNAO        ( 6 )   //すなお
#define PTL_SEIKAKU_NONKI        ( 7 )   //のんき
#define PTL_SEIKAKU_WANPAKU      ( 8 )   //わんぱく
#define PTL_SEIKAKU_NOUTENKI     ( 9 )   //のうてんき
#define PTL_SEIKAKU_OKUBYOU      ( 10 )  //おくびょう
#define PTL_SEIKAKU_SEKKATI      ( 11 )  //せっかち
#define PTL_SEIKAKU_MAJIME       ( 12 )  //まじめ
#define PTL_SEIKAKU_YOUKI        ( 13 )  //ようき
#define PTL_SEIKAKU_MUJYAKI      ( 14 )  //むじゃき
#define PTL_SEIKAKU_HIKAEME      ( 15 )  //ひかえめ
#define PTL_SEIKAKU_OTTORI       ( 16 )  //おっとり
#define PTL_SEIKAKU_REISEI       ( 17 )  //れいせい
#define PTL_SEIKAKU_TEREYA       ( 18 )  //てれや
#define PTL_SEIKAKU_UKKARIYA     ( 19 )  //うっかりや
#define PTL_SEIKAKU_ODAYAKA      ( 20 )  //おだやか
#define PTL_SEIKAKU_OTONASHII    ( 21 )  //おとなしい
#define PTL_SEIKAKU_NAMAIKI      ( 22 )  //なまいき
#define PTL_SEIKAKU_SINTYOU      ( 23 )  //しんちょう
#define PTL_SEIKAKU_KIMAGURE     ( 24 )  //きまぐれ
#define PTL_SEIKAKU_MAX          ( 25 )
#endif

//=========================================================================
//  ポケモンコンディションビット
//=========================================================================

#define PTL_CONDITION_NEMURI            ( 0x00000007 )        //眠り
#define PTL_CONDITION_DOKU              ( 0x00000008 )        //毒
#define PTL_CONDITION_YAKEDO            ( 0x00000010 )        //やけど
#define PTL_CONDITION_KOORI             ( 0x00000020 )        //こおり
#define PTL_CONDITION_MAHI              ( 0x00000040 )        //まひ
#define PTL_CONDITION_DOKUDOKU          ( 0x00000080 )        //どくどく
#define PTL_CONDITION_DOKUDOKU_CNT      ( 0x00000f00 )        //どくどくカウンタ

#define PTL_CONDITION_DOKUALL           ( 0x00000f88 )        //どく全部

#define PTL_CONDITION_NEMURI_OFF        ( 0x00000007 ^ 0xffffffff )   //眠り
#define PTL_CONDITION_DOKU_OFF          ( 0x00000008 ^ 0xffffffff )   //毒
#define PTL_CONDITION_YAKEDO_OFF        ( 0x00000010 ^ 0xffffffff )   //やけど
#define PTL_CONDITION_KOORI_OFF         ( 0x00000020 ^ 0xffffffff )   //こおり
#define PTL_CONDITION_MAHI_OFF          ( 0x00000040 ^ 0xffffffff )   //まひ
#define PTL_CONDITION_DOKUDOKU_CNT_OFF  ( 0x00000f00 ^ 0xffffffff )   //どくどくカウンタ
#define PTL_CONDITION_POISON_OFF        ( ( CONDITION_DOKU | CONDITION_DOKUDOKU | CONDITION_DOKUDOKU_CNT ) ^ 0xffffffff )

#define PTL_DOKUDOKU_COUNT              ( 0x00000100 )

#define PTL_CONDITION_BAD               ( 0x000000ff )

//=========================================================================
//  ポケモンのフォルムナンバー定義
//=========================================================================
#if 0
//ポワルン
#define FORMNO_POWARUN_NORMAL   (0)   ///<ノーマルポワルン
#define FORMNO_POWARUN_SUN      (1)   ///<太陽ポワルン
#define FORMNO_POWARUN_RAIN     (2)   ///<雨ポワルン
#define FORMNO_POWARUN_SNOW     (3)   ///<あられポワルン

#define FORMNO_POWARUN_MAX      (3)   ///<ポワルンの別フォルムMAX

//ミノムッチ
#define FORMNO_MINOMUTTI_KUSA   (0)   ///<くさみの
#define FORMNO_MINOMUTTI_SUNA   (1)   ///<すなみの
#define FORMNO_MINOMUTTI_TETSU    (2)   ///<てつみの

#define FORMNO_MINOMUTTI_MAX    (2)   ///<ミノムッチの別フォルムMAX

//チェリシュ
#define FORMNO_THERISYU_NORMAL    (0)   ///<つぼみチェリシュ
#define FORMNO_THERISYU_SUN     (1)   ///<咲いてるチェリシュ

#define FORMNO_THERISYU_MAX     (1)   ///<チェリシュの別フォルムMAX

//シーウシ・シードルゴ
#define FORMNO_SII_WEST       (0)   ///<西シーウシ・シードルゴ
#define FORMNO_SII_EAST       (1)   ///<東シーウシ・シードルゴ

#define FORMNO_SII_MAX        (1)   ///<シーウシ・シードルゴの別フォルムMAX

//シェイミ
#define FORMNO_SHEIMI_NORMAL    (0)   ///<シェイミノーマル
#define FORMNO_SHEIMI_FLOWER    (1)   ///<シェイミ花咲き

#define FORMNO_SHEIMI_MAX     (1)   ///<シェイミの別フォルムMAX

//デオキシス
#define FORMNO_DEOKISISU_NORMAL   (0)   ///<ノーマルフォルム
#define FORMNO_DEOKISISU_ATTACK   (1)   ///<アタックフォルム
#define FORMNO_DEOKISISU_DEFENCE  (2)   ///<ディフェンスフォルム
#define FORMNO_DEOKISISU_SPEED    (3)   ///<スピードフォルム

#define FORMNO_DEOKISISU_MAX    (3)   ///<デオキシスの別フォルムMAX

// ロトムのフォルム定義
#define FORMNO_ROTOMU_NORMAL  (0)     // ノーマルフォルム
#define FORMNO_ROTOMU_HOT   (1)     // 電子レンジフォルム
#define FORMNO_ROTOMU_WASH    (2)     // 洗濯機フォルム
#define FORMNO_ROTOMU_COLD    (3)     // 冷蔵庫フォルム
#define FORMNO_ROTOMU_FAN   (4)     // 扇風機フォルム
#define FORMNO_ROTOMU_CUT   (5)     // 草刈り機フォルム

#define FORMNO_ROTOMU_MAX   (5)     // ロトムの別フォルムMAX

// ロトムのフォルム定義(BIT指定)
#define FORMNO_ROTOMU_BIT_NORMAL    (1 << FORMNO_ROTOMU_NORMAL)
#define FORMNO_ROTOMU_BIT_HOT     (1 << FORMNO_ROTOMU_HOT)
#define FORMNO_ROTOMU_BIT_WASH      (1 << FORMNO_ROTOMU_WASH)
#define FORMNO_ROTOMU_BIT_COLD      (1 << FORMNO_ROTOMU_COLD)
#define FORMNO_ROTOMU_BIT_FAN     (1 << FORMNO_ROTOMU_FAN)
#define FORMNO_ROTOMU_BIT_CUT     (1 << FORMNO_ROTOMU_CUT)

// ギラティナフォルム定義
#define FORMNO_GIRATINA_NORMAL  (0)     // ノーマル
#define FORMNO_GIRATINA_ORIGIN  (1)     // オリジン

#define FORMNO_GIRATINA_MAX   (1)
#endif

#ifndef __ASM_NO_DEF_
//なつき計算
enum{
  FRIEND_LEVELUP = 0,   //レベルアップ
  FRIEND_PARAEXP_ITEM,  //努力値アイテム
  FRIEND_BATTLE_ITEM,   //戦闘中アイテム
  FRIEND_BOSS_BATTLE,   //ボス戦闘
  FRIEND_WAZA_MACHINE,  //技マシン使用
  FRIEND_TSUREARUKI,    //連れ歩き
  FRIEND_HINSHI,      //瀕死
  FRIEND_DOKU_HINSHI,   //毒による瀕死
  FRIEND_LEVEL30_HINSHI,  //レベル差30以上の瀕死
  FRIEND_CONTEST_VICTORY, //コンテスト優勝
};

///<ボックスポケモン分の構造体
typedef struct pokemon_paso_param POKEMON_PASO_PARAM;
typedef struct pokemon_paso_param1 POKEMON_PASO_PARAM1;
typedef struct pokemon_paso_param2 POKEMON_PASO_PARAM2;
typedef struct pokemon_paso_param3 POKEMON_PASO_PARAM3;
typedef struct pokemon_paso_param4 POKEMON_PASO_PARAM4;

typedef struct pokemon_para_data11 POKEMON_PARA_DATA11;
typedef struct pokemon_para_data12 POKEMON_PARA_DATA12;
typedef struct pokemon_para_data13 POKEMON_PARA_DATA13;
typedef struct pokemon_para_data14 POKEMON_PARA_DATA14;
typedef struct pokemon_para_data15 POKEMON_PARA_DATA15;
typedef struct pokemon_para_data16 POKEMON_PARA_DATA16;
typedef struct pokemon_para_data21 POKEMON_PARA_DATA21;
typedef struct pokemon_para_data22 POKEMON_PARA_DATA22;
typedef struct pokemon_para_data23 POKEMON_PARA_DATA23;
typedef struct pokemon_para_data24 POKEMON_PARA_DATA24;
typedef struct pokemon_para_data25 POKEMON_PARA_DATA25;
typedef struct pokemon_para_data26 POKEMON_PARA_DATA26;
typedef struct pokemon_para_data31 POKEMON_PARA_DATA31;
typedef struct pokemon_para_data32 POKEMON_PARA_DATA32;
typedef struct pokemon_para_data33 POKEMON_PARA_DATA33;
typedef struct pokemon_para_data34 POKEMON_PARA_DATA34;
typedef struct pokemon_para_data35 POKEMON_PARA_DATA35;
typedef struct pokemon_para_data36 POKEMON_PARA_DATA36;
typedef struct pokemon_para_data41 POKEMON_PARA_DATA41;
typedef struct pokemon_para_data42 POKEMON_PARA_DATA42;
typedef struct pokemon_para_data43 POKEMON_PARA_DATA43;
typedef struct pokemon_para_data44 POKEMON_PARA_DATA44;
typedef struct pokemon_para_data45 POKEMON_PARA_DATA45;
typedef struct pokemon_para_data46 POKEMON_PARA_DATA46;

///<ボックスポケモン分以外の構造体
typedef struct pokemon_calc_param POKEMON_CALC_PARAM;

///<手持ちポケモンの構造体
typedef struct pokemon_param POKEMON_PARAM;

///<ポケモンパラメータデータ取得のためのインデックス
enum{
  ID_PARA_personal_rnd = 0,         //個性乱数
  ID_PARA_pp_fast_mode,             //
  ID_PARA_ppp_fast_mode,            //
  ID_PARA_fusei_tamago_flag,        //ダメタマゴフラグ
  ID_PARA_checksum,                 //チェックサム

  ID_PARA_monsno,                   //モンスターナンバー
  ID_PARA_item,                     //所持アイテムナンバー
  ID_PARA_id_no,                    //IDNo
  ID_PARA_exp,                      //経験値
  ID_PARA_friend,                   //なつき度
  ID_PARA_speabino,                 //とくせい
  ID_PARA_mark,                     //ポケモンにつけるマーク（ボックス）
  ID_PARA_country_code,             //国コード
  ID_PARA_hp_exp,                   //HP努力値
  ID_PARA_pow_exp,                  //攻撃力努力値
  ID_PARA_def_exp,                  //防御力努力値
  ID_PARA_agi_exp,                  //素早さ努力値
  ID_PARA_spepow_exp,               //特攻努力値
  ID_PARA_spedef_exp,               //特防努力値
  ID_PARA_style,                    //かっこよさ
  ID_PARA_beautiful,                //うつくしさ
  ID_PARA_cute,                     //かわいさ
  ID_PARA_clever,                   //かしこさ
  ID_PARA_strong,                   //たくましさ
  ID_PARA_fur,                      //毛艶
  ID_PARA_sinou_champ_ribbon,       //シンオウチャンプリボン
  ID_PARA_sinou_battle_tower_ttwin_first,     //シンオウバトルタワータワータイクーン勝利1回目
  ID_PARA_sinou_battle_tower_ttwin_second,    //シンオウバトルタワータワータイクーン勝利2回目
  ID_PARA_sinou_battle_tower_2vs2_win50,      //シンオウバトルタワータワーダブル50連勝
  ID_PARA_sinou_battle_tower_aimulti_win50,   //シンオウバトルタワータワーAIマルチ50連勝
  ID_PARA_sinou_battle_tower_siomulti_win50,  //シンオウバトルタワータワー通信マルチ50連勝
  ID_PARA_sinou_battle_tower_wifi_rank5,      //シンオウバトルタワーWifiランク５入り
  ID_PARA_sinou_syakki_ribbon,        //シンオウしゃっきリボン
  ID_PARA_sinou_dokki_ribbon,         //シンオウどっきリボン
  ID_PARA_sinou_syonbo_ribbon,        //シンオウしょんぼリボン
  ID_PARA_sinou_ukka_ribbon,          //シンオウうっかリボン
  ID_PARA_sinou_sukki_ribbon,         //シンオウすっきリボン
  ID_PARA_sinou_gussu_ribbon,         //シンオウぐっすリボン
  ID_PARA_sinou_nikko_ribbon,         //シンオウにっこリボン
  ID_PARA_sinou_gorgeous_ribbon,      //シンオウゴージャスリボン
  ID_PARA_sinou_royal_ribbon,         //シンオウロイヤルリボン
  ID_PARA_sinou_gorgeousroyal_ribbon, //シンオウゴージャスロイヤルリボン
  ID_PARA_sinou_ashiato_ribbon,       //シンオウあしあとリボン
  ID_PARA_sinou_record_ribbon,        //シンオウレコードリボン
  ID_PARA_sinou_history_ribbon,       //シンオウヒストリーリボン
  ID_PARA_sinou_legend_ribbon,        //シンオウレジェンドリボン
  ID_PARA_sinou_red_ribbon,           //シンオウレッドリボン
  ID_PARA_sinou_green_ribbon,         //シンオウグリーンリボン
  ID_PARA_sinou_blue_ribbon,          //シンオウブルーリボン
  ID_PARA_sinou_festival_ribbon,      //シンオウフェスティバルリボン
  ID_PARA_sinou_carnival_ribbon,      //シンオウカーニバルリボン
  ID_PARA_sinou_classic_ribbon,       //シンオウクラシックリボン
  ID_PARA_sinou_premiere_ribbon,      //シンオウプレミアリボン
  ID_PARA_sinou_amari_ribbon,         //あまり

  ID_PARA_waza1,                //所持技1
  ID_PARA_waza2,                //所持技2
  ID_PARA_waza3,                //所持技3
  ID_PARA_waza4,                //所持技4
  ID_PARA_pp1,                //所持技PP1
  ID_PARA_pp2,                //所持技PP2
  ID_PARA_pp3,                //所持技PP3
  ID_PARA_pp4,                //所持技PP4
  ID_PARA_pp_count1,              //所持技PP_COUNT1
  ID_PARA_pp_count2,              //所持技PP_COUNT2
  ID_PARA_pp_count3,              //所持技PP_COUNT3
  ID_PARA_pp_count4,              //所持技PP_COUNT4
  ID_PARA_pp_max1,              //所持技PPMAX1
  ID_PARA_pp_max2,              //所持技PPMAX2
  ID_PARA_pp_max3,              //所持技PPMAX3
  ID_PARA_pp_max4,              //所持技PPMAX4
  ID_PARA_hp_rnd,               //HP乱数
  ID_PARA_pow_rnd,              //攻撃力乱数
  ID_PARA_def_rnd,              //防御力乱数
  ID_PARA_agi_rnd,              //素早さ乱数
  ID_PARA_spepow_rnd,             //特攻乱数
  ID_PARA_spedef_rnd,             //特防乱数
  ID_PARA_tamago_flag,            //タマゴフラグ

  ID_PARA_stylemedal_normal,          //かっこよさ勲章(ノーマル)AGBコンテスト
  ID_PARA_stylemedal_super,         //かっこよさ勲章(スーパー)AGBコンテスト
  ID_PARA_stylemedal_hyper,         //かっこよさ勲章(ハイパー)AGBコンテスト
  ID_PARA_stylemedal_master,          //かっこよさ勲章(マスター)AGBコンテスト
  ID_PARA_beautifulmedal_normal,        //うつくしさ勲章(ノーマル)AGBコンテスト
  ID_PARA_beautifulmedal_super,       //うつくしさ勲章(スーパー)AGBコンテスト
  ID_PARA_beautifulmedal_hyper,       //うつくしさ勲章(ハイパー)AGBコンテスト
  ID_PARA_beautifulmedal_master,        //うつくしさ勲章(マスター)AGBコンテスト
  ID_PARA_cutemedal_normal,         //かわいさ勲章(ノーマル)AGBコンテスト
  ID_PARA_cutemedal_super,          //かわいさ勲章(スーパー)AGBコンテスト
  ID_PARA_cutemedal_hyper,          //かわいさ勲章(ハイパー)AGBコンテスト
  ID_PARA_cutemedal_master,         //かわいさ勲章(マスター)AGBコンテスト
  ID_PARA_clevermedal_normal,         //かしこさ勲章(ノーマル)AGBコンテスト
  ID_PARA_clevermedal_super,          //かしこさ勲章(スーパー)AGBコンテスト
  ID_PARA_clevermedal_hyper,          //かしこさ勲章(ハイパー)AGBコンテスト
  ID_PARA_clevermedal_master,         //かしこさ勲章(マスター)AGBコンテスト
  ID_PARA_strongmedal_normal,         //たくましさ勲章(ノーマル)AGBコンテスト
  ID_PARA_strongmedal_super,          //たくましさ勲章(スーパー)AGBコンテスト
  ID_PARA_strongmedal_hyper,          //たくましさ勲章(ハイパー)AGBコンテスト
  ID_PARA_strongmedal_master,         //たくましさ勲章(マスター)AGBコンテスト
  ID_PARA_champ_ribbon,           //チャンプリボン
  ID_PARA_winning_ribbon,           //ウィニングリボン
  ID_PARA_victory_ribbon,           //ビクトリーリボン
  ID_PARA_bromide_ribbon,           //ブロマイドリボン
  ID_PARA_ganba_ribbon,           //がんばリボン
  ID_PARA_marine_ribbon,            //マリンリボン
  ID_PARA_land_ribbon,            //ランドリボン
  ID_PARA_sky_ribbon,             //スカイリボン
  ID_PARA_country_ribbon,           //カントリーリボン
  ID_PARA_national_ribbon,          //ナショナルリボン
  ID_PARA_earth_ribbon,           //アースリボン
  ID_PARA_world_ribbon,           //ワールドリボン
  ID_PARA_event_get_flag,           //イベントで配布されたことを示すフラグ
  ID_PARA_sex,                //性別
  ID_PARA_form_no,              //形状ナンバー（アンノーン、デオキシス、ミノメスなど用）
  ID_PARA_seikaku,             //性格
  ID_PARA_dummy_p2_2,             //あまり
  ID_PARA_new_get_place,            //1eh 捕まえた場所（なぞの場所対応用）
  ID_PARA_new_birth_place,          //20h 生まれた場所（なぞの場所対応用）

  ID_PARA_nickname,             //ニックネーム（STRBUF使用）
  ID_PARA_nickname_raw,           //ニックネーム（STRCODE配列使用 ※許可制）
  ID_PARA_nickname_flag,            //ニックネームをつけたかどうかフラグ

  ID_PARA_pref_code,              //都道府県コード
  ID_PARA_get_cassette,           //捕まえたカセット（カラーバージョン）
  ID_PARA_trial_stylemedal_normal,      //かっこよさ勲章(ノーマル)トライアル
  ID_PARA_trial_stylemedal_super,       //かっこよさ勲章(スーパー)トライアル
  ID_PARA_trial_stylemedal_hyper,       //かっこよさ勲章(ハイパー)トライアル
  ID_PARA_trial_stylemedal_master,      //かっこよさ勲章(マスター)トライアル
  ID_PARA_trial_beautifulmedal_normal,    //うつくしさ勲章(ノーマル)トライアル
  ID_PARA_trial_beautifulmedal_super,     //うつくしさ勲章(スーパー)トライアル
  ID_PARA_trial_beautifulmedal_hyper,     //うつくしさ勲章(ハイパー)トライアル
  ID_PARA_trial_beautifulmedal_master,    //うつくしさ勲章(マスター)トライアル
  ID_PARA_trial_cutemedal_normal,       //かわいさ勲章(ノーマル)トライアル
  ID_PARA_trial_cutemedal_super,        //かわいさ勲章(スーパー)トライアル
  ID_PARA_trial_cutemedal_hyper,        //かわいさ勲章(ハイパー)トライアル
  ID_PARA_trial_cutemedal_master,       //かわいさ勲章(マスター)トライアル
  ID_PARA_trial_clevermedal_normal,     //かしこさ勲章(ノーマル)トライアル
  ID_PARA_trial_clevermedal_super,      //かしこさ勲章(スーパー)トライアル
  ID_PARA_trial_clevermedal_hyper,      //かしこさ勲章(ハイパー)トライアル
  ID_PARA_trial_clevermedal_master,     //かしこさ勲章(マスター)トライアル
  ID_PARA_trial_strongmedal_normal,     //たくましさ勲章(ノーマル)トライアル
  ID_PARA_trial_strongmedal_super,      //たくましさ勲章(スーパー)トライアル
  ID_PARA_trial_strongmedal_hyper,      //たくましさ勲章(ハイパー)トライアル
  ID_PARA_trial_strongmedal_master,     //たくましさ勲章(マスター)トライアル
  ID_PARA_amari_ribbon,           //余りリボン

  ID_PARA_oyaname,              //親の名前（STRBUF使用）
  ID_PARA_oyaname_raw,            //親の名前（STRCODE配列使用 ※許可制）
  ID_PARA_get_year,             //捕まえた年
  ID_PARA_get_month,              //捕まえた月
  ID_PARA_get_day,              //捕まえた日
  ID_PARA_birth_year,             //生まれた年
  ID_PARA_birth_month,            //生まれた月
  ID_PARA_birth_day,              //生まれた日
  ID_PARA_get_place,              //捕まえた場所
  ID_PARA_birth_place,            //生まれた場所
  ID_PARA_pokerus,              //ポケルス
  ID_PARA_get_ball,             //捕まえたボール
  ID_PARA_get_level,              //捕まえたレベル
  ID_PARA_oyasex,               //親の性別
  ID_PARA_get_ground_id,            //捕まえた場所の地形アトリビュート（ミノッチ用）
  ID_PARA_dummy_p4_1,             //あまり

  ID_PARA_condition,              //コンディション
  ID_PARA_level,                //レベル
  ID_PARA_cb_id,                //カスタムボールID
  ID_PARA_hp,                 //HP
  ID_PARA_hpmax,                //HPMAX
  ID_PARA_pow,                //攻撃力
  ID_PARA_def,                //防御力
  ID_PARA_agi,                //素早さ
  ID_PARA_spepow,               //特攻
  ID_PARA_spedef,               //特防
  ID_PARA_mail_data,              //メールデータ
  ID_PARA_cb_core,              //カスタムボールデータ

  ID_PARA_poke_exist,             //ポケモン存在フラグ
  ID_PARA_tamago_exist,           //タマゴ存在フラグ

  ID_PARA_monsno_egg,             //タマゴかどうかも同時にチェック

  ID_PARA_power_rnd,              //パワー乱数をu32で扱う
  ID_PARA_nidoran_nickname,         //モンスターナンバーがニドランの時にnickname_flagが立っているかチェック

  ID_PARA_type1,                //ポケモンのタイプ１を取得（アウス、マルチタイプチェックもあり）
  ID_PARA_type2,                //ポケモンのタイプ２を取得（アウス、マルチタイプチェックもあり）

  ID_PARA_end                 //パラメータを追加するときは、ここから上に追加
};


#define PTL_WAZASET_FAIL    (0xffff)  //技のセットができなかったときの返り値
#define PTL_WAZASET_SAME    (0xfffe)  //すでに覚えていた技のときの返り値

#define PTL_WAZAOBOE_NONE   (0x0000)  //技覚えで覚える技が無かったときの返り値
#define PTL_WAZAOBOE_FULL   (0x8000)  //技覚えで手持ちの技がいっぱいだったときに立てる

///<戦闘録画用ポケモンパラメータの構造体
typedef struct rec_pokepara REC_POKEPARA;

//ポケモンパラメータ取得系の定義
enum{
  ID_POKEPARADATA11=0,  //0
  ID_POKEPARADATA12,    //1
  ID_POKEPARADATA13,    //2
  ID_POKEPARADATA14,    //3
  ID_POKEPARADATA15,    //4
  ID_POKEPARADATA16,    //5
  ID_POKEPARADATA21,    //6
  ID_POKEPARADATA22,    //7
  ID_POKEPARADATA23,    //8
  ID_POKEPARADATA24,    //9
  ID_POKEPARADATA25,    //10
  ID_POKEPARADATA26,    //11
  ID_POKEPARADATA31,    //12
  ID_POKEPARADATA32,    //13
  ID_POKEPARADATA33,    //14
  ID_POKEPARADATA34,    //15
  ID_POKEPARADATA35,    //16
  ID_POKEPARADATA36,    //17
  ID_POKEPARADATA41,    //18
  ID_POKEPARADATA42,    //19
  ID_POKEPARADATA43,    //20
  ID_POKEPARADATA44,    //21
  ID_POKEPARADATA45,    //22
  ID_POKEPARADATA46,    //23

  ID_POKEPARADATA51,    //24
  ID_POKEPARADATA52,    //25
  ID_POKEPARADATA53,    //26
  ID_POKEPARADATA54,    //27
  ID_POKEPARADATA55,    //28
  ID_POKEPARADATA56,    //29

  ID_POKEPARADATA61,    //30
  ID_POKEPARADATA62,    //31
};

enum{
  ID_POKEPARA1=0,
  ID_POKEPARA2,
  ID_POKEPARA3,
  ID_POKEPARA4,
};

enum{
  UNK_A=0,
  UNK_B,
  UNK_C,
  UNK_D,
  UNK_E,
  UNK_F,
  UNK_G,
  UNK_H,
  UNK_I,
  UNK_J,
  UNK_K,
  UNK_L,
  UNK_M,
  UNK_N,
  UNK_O,
  UNK_P,
  UNK_Q,
  UNK_R,
  UNK_S,
  UNK_T,
  UNK_U,
  UNK_V,
  UNK_W,
  UNK_X,
  UNK_Y,
  UNK_Z,
  UNK_ENC,
  UNK_QUE,
  UNK_END
};

typedef enum
{
  PTL_TASTE_KARAI = 0,
  PTL_TASTE_SIBUI,
  PTL_TASTE_AMAI,
  PTL_TASTE_NIGAI,
  PTL_TASTE_SUPPAI,
  PTL_TASTE_MAX,
}PtlTaste;

#define PTL_TASTE_NORMAL  ( 0 )
#define PTL_TASTE_LIKE    ( 1 )
#define PTL_TASTE_DISLIKE ( -1 )
typedef s8  PtlTasteJudge;

//-----------------------------------------------------------------------------
/**
 *  SetupEx系関数の引数
 */
//-----------------------------------------------------------------------------

/*---------------------------------------------*/
/* pow = 個体値                                */
/*---------------------------------------------*/

typedef  u32  PtlSetupPow;

// 個体値をランダムで決定
#define PTL_SETUP_POW_AUTO    (0xffffffff)

// 個体値を任意の値に設定するための変数パッキング処理 (HP，攻撃、防御、特攻、特防、素早さの順に指定）
static inline u32 PTL_SETUP_POW_PACK( u8 hp, u8 atk, u8 def, u8 sp_atk, u8 sp_def, u8 agi )
{
  return ( (hp&0x1f)|((atk&0x1f)<<5)|((def&0x1f)<<10)|((sp_atk&0x1f)<<15)|((sp_def&0x1f)<<20)|((agi&0x1f)<<25) );
}

// パッキングされた個体値から任意種類の値を取り出すアンパック処理（SetupEx系関数内部で使用）
static inline u8 PTL_SETUP_POW_UNPACK( u32 powrnd, u8 abilityType )
{
  return (powrnd >> (abilityType*5)) & 0x1f;
}

/*---------------------------------------------*/
/* ID                                          */
/*---------------------------------------------*/

// 以下の値以外が指定されれば、その指定された値を使う
#define PTL_SETUP_ID_AUTO   (0xffffffffffffffff)
#define PTL_SETUP_ID_NOT_RARE  (0xffffffff00000000)  ///< 必ずノーマルカラーになるように調整


/*---------------------------------------------*/
/* rnd = 個性乱数                              */
/*---------------------------------------------*/

// 以下の値以外が指定されれば、その指定された値を使う

#define PTL_SETUP_RND_AUTO    (0xffffffff00000000)  ///< 個性乱数を完全にランダムで決定
#define PTL_SETUP_RND_RARE    (0xffffffff00000001)  ///< 必ずレアカラーになるように調整





extern  void  POKETOOL_InitSystem( HEAPID heapID ); ///< システム初期化（プログラム起動後に１回だけ）
extern  u32   POKETOOL_GetWorkSize( void );     ///< POKEMON_PARAM１体分のサイズ取得
extern u32   POKETOOL_GetPPPWorkSize( void );  ///< POKEMON_PASO_PARAMのサイズ


//ポケモンパラメータ操作関数系


extern  POKEMON_PARAM*  PP_Create( u16 mons_no, u16 level, u64 id, HEAPID heapID ); ///<
extern  POKEMON_PARAM*  PP_CreateByPPP( const POKEMON_PASO_PARAM* ppp, HEAPID heapID );

// 動作高速化
// 複数回の Get, Put を行う前に FastModeOn しておくと、
// 暗号・複合処理が毎回行われなくなるので動作が軽くなります。
// 終わったら必ず FastModeOff すること！
extern  BOOL  PP_FastModeOn( POKEMON_PARAM *pp );
extern  BOOL  PP_FastModeOff( POKEMON_PARAM *pp, BOOL flag );
extern  BOOL  PPP_FastModeOn( POKEMON_PASO_PARAM *ppp );
extern  BOOL  PPP_FastModeOff( POKEMON_PASO_PARAM *ppp, BOOL flag );


extern  void  PP_Setup( POKEMON_PARAM *pp, u16 mons_no, u16 level, u64 ID );
extern  void  PP_SetupEx( POKEMON_PARAM *pp, u16 mons_no, u16 level, u64 ID, PtlSetupPow pow, u64 rnd );
extern  void  PPP_Setup( POKEMON_PASO_PARAM *ppp, u16 mons_no, u16 level, u64 ID );
extern  void  PPP_SetupEx( POKEMON_PASO_PARAM *ppp, u16 mons_no, u16 level, u64 id, PtlSetupPow pow, u64 rnd );

// パラメータ取得系
extern  u32   PP_Get( const POKEMON_PARAM *pp, int id, void *buf);
extern  u32   PPP_Get( const POKEMON_PASO_PARAM *ppp, int id, void *buf);
extern  u32   PP_CalcLevel( const POKEMON_PARAM *pp );
extern  u32   PPP_CalcLevel( const POKEMON_PASO_PARAM *ppp );
extern  u8    PP_GetSex( const POKEMON_PARAM *pp );
extern  u8    PPP_GetSex( const POKEMON_PASO_PARAM *ppp );
extern  u32   PP_GetMinExp( const POKEMON_PARAM *pp );
extern  u32   PPP_GetMinExp( const POKEMON_PASO_PARAM *ppp );
extern  u8    PP_GetSeikaku( const POKEMON_PARAM *pp );
extern  u8    PPP_GetSeikaku( const POKEMON_PASO_PARAM *ppp );
extern  BOOL  PP_CheckRare( const POKEMON_PARAM *pp );
extern  BOOL  PPP_CheckRare( const POKEMON_PASO_PARAM *pp );
extern  PokeSick PP_GetSick( const POKEMON_PARAM* pp );

// パラメータ設定系（汎用）
extern  void  PP_Put( POKEMON_PARAM *pp, int id, u32 arg );
extern  void  PPP_Put( POKEMON_PASO_PARAM *ppp, int id, u32 arg );
extern  void  PP_Add( POKEMON_PARAM *pp, int id, int value);
extern  void  PPP_Add( POKEMON_PASO_PARAM *ppp, int id, int value);

// パラメータ設定系（ワザ）
extern  u16   PP_SetWaza( POKEMON_PARAM *pp, u16 wazano );
extern  u16   PPP_SetWaza( POKEMON_PASO_PARAM *ppp, u16 wazano );
extern  void  PP_SetWazaDefault( POKEMON_PARAM *pp );
extern  void  PPP_SetWazaDefault( POKEMON_PASO_PARAM *ppp );
extern  void  PP_SetWazaPush( POKEMON_PARAM *pp, u16 wazano );
extern  void  PPP_SetWazaPush( POKEMON_PASO_PARAM *ppp, u16 wazano );
extern  void  PP_SetWazaPos( POKEMON_PARAM *pp, u16 wazano, u8 pos );
extern  void  PPP_SetWazaPos( POKEMON_PASO_PARAM *ppp, u16 wazano, u8 pos );
extern  void  PP_RecoverWazaPPAll( POKEMON_PARAM* pp );
extern  WazaID  PP_CheckWazaOboe( POKEMON_PARAM *pp, int* index, HEAPID  heapID );

// パラメータ設定系（その他）
extern  void  PP_SetSick( POKEMON_PARAM* pp, PokeSick sick );
extern  void  PP_SetSeikaku( POKEMON_PARAM *pp, u8 chr );
extern  void  PPP_SetSeikaku( POKEMON_PASO_PARAM *ppp, u8 chr );

// 親の一致チェック
extern BOOL PP_IsMatchOya( const POKEMON_PARAM* pp, const MYSTATUS* player );
extern BOOL PPP_IsMatchOya( const POKEMON_PASO_PARAM* ppp, const MYSTATUS* player );


// 領域初期化
extern  void  PP_Clear( POKEMON_PARAM *pp );
extern  void  PPP_Clear( POKEMON_PASO_PARAM *ppp );

// モンスターナンバー書き換え（進化等で使用）
extern void PP_ChangeMonsNo( POKEMON_PARAM* ppp, u16 monsno );
extern void PPP_ChangeMonsNo( POKEMON_PASO_PARAM* ppp, u16 monsno );

// フォルムナンバー書き換え
extern BOOL PP_ChangeFormNo( POKEMON_PARAM* ppp, u16 formno );
extern BOOL PPP_ChangeFormNo( POKEMON_PASO_PARAM* ppp, u16 formno );

// PPP部分からPP部分を再計算
extern  void  PP_Renew( POKEMON_PARAM *pp );

// PPPポインタを取得
extern  POKEMON_PASO_PARAM  *PP_GetPPPPointer( POKEMON_PARAM *pp );
extern  const POKEMON_PASO_PARAM *PP_GetPPPPointerConst( const POKEMON_PARAM *pp );

// 低レベル関数群
extern  u32   POKETOOL_CalcLevel( u16 mons_no, u16 form_no, u32 exp );
extern  u32   POKETOOL_GetMinExp( u16 mons_no, u16 form_no, u16 level );
extern  u8    POKETOOL_GetSex( u16 mons_no, u16 form_no, u32 personal_rnd );
extern  BOOL  POKETOOL_CheckRare( u32 id, u32 personal_rnd );
extern  u32   POKETOOL_GetPersonalParam( u16 mons_no, u16 form_no, PokePersonalParamID param );
extern  u32   POKETOOL_CalcPersonalRand( u16 mons_no, u16 form_no, u8 sex );

// めざめるパワー計算
extern PokeType POKETOOL_GetMezaPa_Type( const POKEMON_PARAM* pp );
extern u32 POKETOOL_GetMezaPa_Power( const POKEMON_PARAM* pp );

// 技マシンで技を覚えるかチェック
extern  BOOL  PP_CheckWazaMachine( const POKEMON_PARAM *pp, int machine_no );
extern  BOOL  PPP_CheckWazaMachine( const POKEMON_PASO_PARAM *ppp, int machine_no );
extern  BOOL  POKETOOL_CheckWazaMachine( u16 mons_no, u16 form_no, int machine_no );

//ポケモンの好みの味かチェック
extern  PtlTasteJudge PP_CheckDesiredTaste( const POKEMON_PARAM *pp, PtlTaste taste );
extern  PtlTasteJudge PPP_CheckDesiredTaste( const POKEMON_PASO_PARAM *ppp, PtlTaste taste );
extern  PtlTasteJudge POKETOOL_CheckDesiredTaste( u8 seikaku, PtlTaste taste );

// 戦闘録画用に外部公開
extern void  POKETOOL_encode_data( void *data, u32 size, u32 code );
#define POKETOOL_decord_data( data, size, code )   POKETOOL_encode_data( data, size, code );
extern void  *POKETOOL_ppp_get_param_block( POKEMON_PASO_PARAM *ppp, u32 rnd, u8 id );
extern u16 POKETOOL_make_checksum( const void *data, u32 size );

// 戦闘録画用
extern void POKETOOL_PokePara_to_RecPokePara(POKEMON_PARAM *pp, REC_POKEPARA *rec);
extern void POKETOOL_RecPokePara_to_PokePara(REC_POKEPARA *rec, POKEMON_PARAM *pp);

//レベル補正
extern void POKETOOL_MakeLevelRevise(POKEMON_PARAM *pp, u32 level);

//構造体コピー
extern  void POKETOOL_CopyPPtoPP( POKEMON_PARAM* pp_src, POKEMON_PARAM* pp_dst );

#endif __ASM_NO_DEF_

