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
#include "../btl_tables.h"

typedef struct {
  u8  pokeCnt;
  u8  pokeID[ BTL_POS_MAX ];
}HANDWORK_POKEID;


/**
 *  共有ツール関数群
 */
extern BOOL HandCommon_CheckTargetPokeID( u8 pokeID );
extern BOOL HandCommon_IsPokeOrderLast( BTL_SVFLOW_WORK* flowWk, u8 pokeID );
extern BOOL HandCommon_CheckForbitItemPokeCombination( u16 monsno, u16 itemID );
extern BOOL HandCommon_CheckCantChangeItemPoke( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern BOOL HandCommon_CheckCantStealPoke( BTL_SVFLOW_WORK* flowWk, u8 attackPokeID, u8 targetPokeID );
extern void HandCommon_MagicCoat_CheckSideEffWaza( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
extern void HandCommon_MagicCoat_Wait( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
extern void HandCommon_MagicCoat_Reaction( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
extern void HandCommon_MulWazaBasePower( u32 ratio );

