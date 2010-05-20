//=============================================================================================
/**
 * @file  hand_common.h
 * @brief ポケモンWB バトルシステム イベントファクター 共通ヘッダ
 * @author  taya
 *
 * @date  2009.12.02  作成
 */
//=============================================================================================
#pragma once

#include "../btl_common.h"

typedef struct {
  u8  pokeCnt;
  u8  pokeID[ BTL_POS_MAX ];
}HANDWORK_POKEID;


/**
 *  共有ツール関数群
 */
extern BOOL HandCommon_CheckTargetPokeID( u8 pokeID );
extern BOOL HandCommon_IsPokeOrderLast( BTL_SVFLOW_WORK* flowWk, u8 pokeID );

