//=============================================================================================
/**
 * @file  btl_server_flow_sub.h
 * @brief ポケモンWB バトルシステム サーバコマンド生成処理（サブルーチン群）
 * @author  taya
 *
 * @date  2010.05.28  作成
 */
//=============================================================================================
#pragma once

extern u8 BTL_SVFSUB_RegisterTargets( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos targetPos,
  const SVFL_WAZAPARAM* wazaParam, BTL_POKESET* rec );

