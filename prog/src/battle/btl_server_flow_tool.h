//=============================================================================================
/**
 * @file  btl_server_flow_tool.h
 * @brief ポケモンWB バトルシステム サーバコマンド生成処理モジュールの一部関数宣言をAI、ハンドラ等から参照できるようにする
 * @author  taya
 *
 * @date  2010.03.17  作成
 */
//=============================================================================================

#pragma once

#include "btl_common.h"
#include "btl_sideeff.h"
#include "btl_server_flow_def.h"

extern BOOL BTL_SVFTOOL_IsExistSideEffect( BTL_SVFLOW_WORK* wk, BtlPokePos pos, BtlSideEffect sideEffect );

