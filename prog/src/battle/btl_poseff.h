//=============================================================================================
/**
 * @file  btl_poseff.h
 * @brief ポケモンWB バトル -システム - 位置エフェクト種類等 -
 * @author  taya
 *
 * @date  2008.07.21  作成
 */
//=============================================================================================
#pragma once

enum {
  BTL_POSEFF_PARAM_MAX = 4,
};

/**
 *  位置エフェクト種類
 */
typedef enum {

  BTL_POSEFF_NEGAIGOTO,
  BTL_POSEFF_MIKADUKINOMAI,
  BTL_POSEFF_IYASINONEGAI,
  BTL_POSEFF_DELAY_ATTACK,
  BTL_POSEFF_BATONTOUCH,

  BTL_POSEFF_MAX,
  BTL_POSEFF_NULL = BTL_POSEFF_MAX,

}BtlPosEffect;

