//=============================================================================================
/**
 * @file  hand_side.h
 * @brief ポケモンWB バトルシステム イベントファクター [サイドエフェクト]
 * @author  taya
 *
 * @date  2009.06.18  作成
 */
//=============================================================================================
#pragma once

#include "..\btl_pokeparam.h"

/**
 *  サイドエフェクト種類
 */
typedef enum {

  BTL_SIDEEFF_NULL = 0,
  BTL_SIDEEFF_REFRECTOR,      ///< 物理攻撃を半減
  BTL_SIDEEFF_HIKARINOKABE,   ///< 特殊攻撃を半減
  BTL_SIDEEFF_SINPINOMAMORI,  ///< ポケ系状態異常にならない
  BTL_SIDEEFF_SIROIKIRI,      ///< ランクダウン効果を受けない
  BTL_SIDEEFF_MAKIBISI,       ///< 入れ替えて出てきたポケモンにダメージ
  BTL_SIDEEFF_DOKUBISI,       ///< 入れ替えて出てきたポケモンに毒
  BTL_SIDEEFF_MIZUASOBI,      ///< 炎ワザ威力減
  BTL_SIDEEFF_DOROASOBI,      ///< 電気ワザ威力減


  BTL_SIDEEFF_MAX,

}BtlSideEffect;



extern void BTL_HANDLER_SIDE_InitSystem( void );
extern BTL_EVENT_FACTOR*  BTL_HANDLER_SIDE_Add( const BTL_POKEPARAM* pp, BtlSideEffect sideEffect, BPP_SICK_CONT contParam );


