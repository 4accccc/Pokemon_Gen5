//=============================================================================================
/**
 * @file  hand_pos.h
 * @brief ポケモンWB バトルシステム イベントファクター [位置エフェクト]
 * @author  taya
 *
 * @date  2009.07.11  作成
 */
//=============================================================================================
#pragma once

#include "..\btl_pokeparam.h"
#include "..\btl_server_flow.h"


extern BTL_EVENT_FACTOR*  BTL_HANDLER_POS_Add( BtlPosEffect effect, BtlPokePos pos, int param );

