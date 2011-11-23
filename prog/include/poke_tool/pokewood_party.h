//============================================================================================
/**
 * @file  pokewood_party.h
 * @bfief ポケウッド用ポケモンデータ作成用定義ヘッダ
 * @author  AkitoMori (田中さんの定義をヘッダに分離）
 * @date  09.09.22
 */
//============================================================================================
#pragma once


//======================================================================
//  define
//======================================================================
// pokewood
enum{
  PKWDS_REC_POKE = 0,   // 撮影用ポケモン
  PKWDS_PLAY_POKE,      // 上映用ポケモン
  PKWDS_LEVEL,          // レベル
  PKWDS_FORM,           // フォルム
  PKWDS_SEX,            // 性別
  PKWDS_MIND,           // 性格
  PKWDS_STATE,          // 特性
  PKWDS_GOODS,          // 道具
  PKWDS_SKILL_1,        // 技1
  PKWDS_SKILL_2,        // 技2
  PKWDS_SKILL_3,        // 技3
  PKWDS_SKILL_4,        // 技4
  PKWDS_HP_FIX,         // HP固体値
  PKWDS_HP_EFF,         // HP努力値
  PKWDS_ATK_FIX,        // 攻撃固体値
  PKWDS_ATK_EFF,        // 攻撃努力値
  PKWDS_DEF_FIX,        // 防御固体値
  PKWDS_DEF_EFF,        // 防御努力値
  PKWDS_SPD_FIX,        // 早さ固体値
  PKWDS_SPD_EFF,        // 早さ努力値
  PKWDS_SPATK_FIX,      // 特攻固体値
  PKWDS_SPATK_EFF,      // 特攻努力値
  PKWDS_SPDEF_FIX,      // 特防固体値
  PKWDS_SPDEF_EFF,      // 特防努力値
  //
  PKWDS_PARAM_MAX,
};

// ----------------------------------------
// ポケウッド用ポケモンデータ（cast用）
// ----------------------------------------

typedef struct{
  s16 rec_monsno, show_monsno;  // 撮影用ポケモン/上映用ポケモン
  s16 level;                    // レベル
  s16 form;                     // フォルム
  s16 sex;                      // 性別
  s16 seikaku;                  // 性格
  s16 tokusei;                  // 特性
  s16 item;                     // 道具
  s16 waza[4];                  // 技４つ
  s16 hp_rnd;                   // HP固体値
  s16 hp_eff;                   // HP努力値
  s16 pow_rnd;                  // 攻撃固体値
  s16 pow_eff;                  // 攻撃努力値
  s16 def_rnd;                  // 防御固体値
  s16 def_eff;                  // 防御努力値
  s16 spd_rnd;                  // 早さ固体値
  s16 spd_eff;                  // 早さ努力値
  s16 spepow_rnd;                // 特攻固体値
  s16 spepow_eff;                // 特攻努力値
  s16 spedef_rnd;                // 特防固体値
  s16 spedef_eff;                // 特防努力値
} POKEWOOD_POKE_DATA;

// ------------------------------------------------------------
// コンバータから出力されるポケウッド用データ全体
// ------------------------------------------------------------
typedef struct{
  s16                party_num[2];  // [0]=自分ポケモン数、[1]=敵ポケモン数
  POKEWOOD_POKE_DATA data[12];      // 格納数によって変化するため、必ず11まで参照できるとは限らない 
                                    // 0～party_num[0]-1まで:自分ポケモン(レンタル時）
                                    // party_num[0]-1～party_num[1]-1:敵トレーナーポケモン
} POKEWOOD_PARTIES_DATA;

