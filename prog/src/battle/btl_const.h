//=============================================================================================
/**
 * @file  btl_sick.h
 * @brief ポケモンWB バトルシステム 主にサーバ計算用途の定数群
 * @author  taya
 *
 * @date  2009.07.29  作成
 */
//=============================================================================================
#pragma once

enum {
  BTL_CALC_BASERANK_DEFAULT = 0,

  BTL_CALC_HITRATIO_MID = 6,
  BTL_CALC_HITRATIO_MIN = 0,
  BTL_CALC_HITRATIO_MAX = 12,

  BTL_CALC_CRITICAL_MIN = 0,
  BTL_CALC_CRITICAL_MAX = 4,

// ダメージ計算関連
  BTL_CALC_DMG_TARGET_RATIO_PLURAL = FX32_CONST(0.75f),     ///< 複数体が対象ワザのダメージ補正率
  BTL_CALC_DMG_TARGET_RATIO_NONE   = FX32_CONST(1),         ///< 対象によるダメージ補正なし
  BTL_CALC_DMG_WEATHER_RATIO_ADVANTAGE = FX32_CONST(1.5f),    ///< ワザタイプ＆天候の組み合わせで有利な場合の補正率
  BTL_CALC_DMG_WEATHER_RATIO_DISADVANTAGE = FX32_CONST(0.5f), ///< ワザタイプ＆天候の組み合わせで不利な場合の補正率
  BTL_CALC_DMG_WEATHER_RATIO_NONE = FX32_CONST(1),            ///< ワザタイプ＆天候の組み合わせによる補正なし


// 状態異常処理関連
  BTL_NEMURI_TURN_MIN = 2,  ///< 「ねむり」最小ターン数
  BTL_NEMURI_TURN_MAX = 4,  ///< 「ねむり」最大ターン数
  BTL_NEMURI_TURN_RANGE = (BTL_NEMURI_TURN_MAX - BTL_NEMURI_TURN_MIN + 1),

  BTL_MAHI_EXE_PER = 25,        ///< 「まひ」でしびれて動けない確率
  BTL_MAHI_AGILITY_RATIO = 25,  ///< 「まひ」時のすばやさ減衰率
  BTL_KORI_MELT_PER = 20,       ///< 「こおり」が溶ける確率
  BTL_MEROMERO_EXE_PER = 50,    ///< 「メロメロ」で動けない確率

  BTL_DOKU_SPLIT_DENOM = 8,         ///< 「どく」で最大HPの何分の１減るか
  BTL_MOUDOKU_INC_MAX = 15,         ///< 「どくどく」でダメージ量が増加するターン数最大
  BTL_YAKEDO_SPLIT_DENOM = 8,       ///< 「やけど」で最大HPの何分の１減るか
  BTL_YAKEDO_DAMAGE_RATIO = 50,     ///< 「やけど」で物理ダメージを減じる率
  BTL_MOUDOKU_SPLIT_DENOM = 16,     ///< もうどく時、最大HPの何分の１減るか（基本値=最小値）
  BTL_MOUDOKU_COUNT_MAX = BTL_MOUDOKU_SPLIT_DENOM-1,  ///< もうどく時、ダメージ倍率をいくつまでカウントアップするか

  BTL_BIND_TURN_MAX = 6,        ///< まきつき系の継続ターン数最大値


  BTL_CONF_EXE_RATIO = 30,    ///< 混乱時の自爆確率
  BTL_CONF_TURN_MIN = 2,
  BTL_CONF_TURN_MAX = 5,
  BTL_CONF_TURN_RANGE = (BTL_CONF_TURN_MAX - BTL_CONF_TURN_MIN)+1,

  BTL_KANASIBARI_EFFECTIVE_TURN = 4,  ///< 「かなしばり」の実効ターン数

  BTL_CALC_SICK_TURN_PERMANENT = 0xff,  ///< 継続ターン数を設定しない（永続する）場合の指定値

  // 天候関連
  BTL_CALC_WEATHER_MIST_HITRATIO = FX32_CONST(75),    ///< 「きり」の時の命中率補正率
  BTL_WEATHER_TURN_DEFAULT = 5,         ///< ワザによる天候変化の継続ターン数デフォルト値
  BTL_WEATHER_TURN_PERMANENT = 0xff,    ///< 天候継続ターン無制限

  // とくせい関連
  BTL_CALC_TOK_CHIDORI_HITRATIO = FX32_CONST(0.8f),     /// 「ちどりあし」の減衰命中率
  BTL_CALC_TOK_HARIKIRI_HITRATIO = FX32_CONST(0.8f),    /// 「はりきり」の命中率変化率
  BTL_CALC_TOK_FUKUGAN_HITRATIO = FX32_CONST(1.3f),     /// 「ふくがん」の命中変化率
  BTL_CALC_TOK_SUNAGAKURE_HITRATIO = FX32_CONST(0.8f),  /// 「すながくれ」の命中率変化率
  BTL_CALC_TOK_YUKIGAKURE_HITRATIO = FX32_CONST(0.8f),  /// 「ゆきがくれ」の命中変化率
  BTL_CALC_TOK_HARIKIRI_POWRATIO = FX32_CONST(1.5f),    /// 「はりきり」の攻撃力変化率
  BTL_CALC_TOK_TETUNOKOBUSI_POWRATIO = FX32_CONST(1.2f),/// 「てつのこぶし」の攻撃力変化率
  BTL_CALC_TOK_SUTEMI_POWRATIO = FX32_CONST(1.2f),      /// 「すてみ」の攻撃力変化率
  BTL_CALC_TOK_PLUS_POWRATIO = FX32_CONST(1.5f),        /// 「プラス」の攻撃力変化率
  BTL_CALC_TOK_MINUS_POWRATIO = FX32_CONST(1.5f),       /// 「マイナス」の攻撃力変化率
  BTL_CALC_TOK_FLOWERGIFT_POWRATIO = FX32_CONST(1.5f),  /// 「フラワーギフト」の攻撃力変化率
  BTL_CALC_TOK_FLOWERGIFT_GUARDRATIO = FX32_CONST(1.5f),/// 「フラワーギフト」の特防変化率
  BTL_CALC_TOK_MORAIBI_POWRATIO = FX32_CONST(1.5f),     /// 「もらいび」の攻撃力変化率
  BTL_CALC_TOK_TECKNICIAN_POWRATIO = FX32_CONST(1.5f),  /// 「テクニシャン」の攻撃力変化率
  BTL_CALC_TOK_HAYAASI_AGIRATIO = FX32_CONST(1.5f),     /// 「はやあし」のすばやさ変化率
  BTL_CALC_TOK_SLOWSTART_ENABLE_TURN = 5,               /// 「スロースタート」の適用ターン
  BTL_CALC_TOK_SLOWSTART_AGIRATIO = FX32_CONST(0.5f),   /// 「スロースタート」のすばやさ変化率
  BTL_CALC_TOK_SLOWSTART_ATKRATIO = FX32_CONST(0.5f),   /// 「スロースタート」のこうげき変化率
  BTL_CALC_TOK_FUSIGINAUROKO_GDRATIO = FX32_CONST(1.5f),  /// 「ふしぎなうろこ」のとくぼう変化率

  BTL_CALC_TOK_DOKUNOTOGE_PER   = 30,   ///「どくのトゲ」発生確率
  BTL_CALC_TOK_HONONOKARADA_PER = 30,   ///「ほのおのからだ」発生確率
  BTL_CALC_TOK_SEIDENKI_PER     = 30,   ///「せいでんき」発生確率
  BTL_CALC_TOK_HOUSI_PER        = 30,   ///「ほうし」発生確率
  BTL_CALC_TOK_MEROMEROBODY_PER = 30,   ///「メロメロボディ」発生確率

  // 素早さ補正関連
  BTL_CALC_AGILITY_MAX = 10000,

  // 体重処理関連
  BTL_POKE_WEIGHT_MIN = 1,

  // フィールド・進化画面等への連絡関連
  BTL_STANDALONE_PLAYER_CLIENT_ID = 0,
  BTL_BONUS_MONEY_MAX = 99999,
};
