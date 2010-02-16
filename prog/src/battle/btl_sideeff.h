//=============================================================================================
/**
 * @file  btl_sideeff.h
 * @brief ポケモンWB バトル -システム-  サイドエフェクト種類等
 * @author  taya
 *
 * @date  2008.07.02  作成
 */
//=============================================================================================
#pragma once

/**
 *  サイドエフェクト種類
 */
typedef enum {

  BTL_SIDEEFF_START=0,

  BTL_SIDEEFF_REFRECTOR=BTL_SIDEEFF_START,      ///< 物理攻撃を半減
  BTL_SIDEEFF_HIKARINOKABE,   ///< 特殊攻撃を半減
  BTL_SIDEEFF_SINPINOMAMORI,  ///< ポケ系状態異常にならない
  BTL_SIDEEFF_SIROIKIRI,      ///< ランクダウン効果を受けない
  BTL_SIDEEFF_OIKAZE,         ///< すばやさ２倍
  BTL_SIDEEFF_OMAJINAI,       ///< 攻撃が急所に当たらない
  BTL_SIDEEFF_MAKIBISI,       ///< 入れ替えて出てきたポケモンにダメージ（３段階）
  BTL_SIDEEFF_DOKUBISI,       ///< 入れ替えて出てきたポケモンに毒（２段階）
  BTL_SIDEEFF_STEALTHROCK,    ///< 入れ替えて出てきたポケモンにダメージ（相性計算あり）
  BTL_SIDEEFF_WIDEGUARD,      ///< ヒット範囲「敵全体」「自分以外」の攻撃ワザを防ぐ
  BTL_SIDEEFF_FASTGUARD,      ///< 先制プライオリティ1以上のポケモン対象ワザを防ぐ
  BTL_SIDEEFF_RAINBOW,        ///< 合体ワザ効果・虹（追加効果の発生率が倍になる）
  BTL_SIDEEFF_BURNING,        ///< 合体ワザ効果・火の海（炎タイプ以外に毎ターン1/8ダメージ）
  BTL_SIDEEFF_MOOR,           ///< 合体ワザ効果・湿原（素早さ 1/4）

  BTL_SIDEEFF_MAX,
  BTL_SIDEEFF_NULL = BTL_SIDEEFF_MAX,

  BTL_SIDEEFF_BITFLG_BYTES = 1 + (BTL_SIDEEFF_MAX/8) + ((BTL_SIDEEFF_MAX%8)!=0),

}BtlSideEffect;



