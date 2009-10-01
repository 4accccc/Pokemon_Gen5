//=============================================================================================
/**
 * @file  btl_pospoke_state.h
 * @brief ポケモンWB バトルシステム  場にいるポケモンの生存確認用構造体と処理ルーチン
 * @author  taya
 *
 * @date  2009.07.27  作成
 */
//=============================================================================================
#pragma once

#include "btl_common.h"
#include "btl_main.h"

/**
 * 状態コード
 */
typedef enum {

  POSPOKE_EMPTY = 0,    ///< 管理外（シングル戦で２体目の位置などは管理外になる）
  POSPOKE_EXIST,        ///< 生きているポケモンが存在する
  POSPOKE_DEAD,         ///<

}BtlPosPokeState;


typedef struct {
  u8  fEnable;
  u8  clientID;
  u8  existPokeID;
}BTL_POSPOKE_STATE;

/**
 * 状態保持構造体
 */
typedef struct {
//  BtlPosPokeState  state[ BTL_POS_MAX ];
  BTL_POSPOKE_STATE  state[ BTL_POS_MAX ];
}BTL_POSPOKE_WORK;

/**
 * 状態比較結果構造体
 */
typedef struct {
  u32          count;
  BtlPokePos   pos[ BTL_POS_MAX ];
}BTL_POSPOKE_COMPAIR_RESULT;


extern void BTL_POSPOKE_InitWork( BTL_POSPOKE_WORK* wk, const BTL_MAIN_MODULE* mainModule, const BTL_POKE_CONTAINER* pokeCon, BtlRule rule );

extern void BTL_POSPOKE_PokeIn( BTL_POSPOKE_WORK* wk, BtlPokePos pos,  u8 pokeID );
extern void BTL_POSPOKE_PokeOut( BTL_POSPOKE_WORK* wk, u8 pokeID );
extern void BTL_POSPOKE_Swap( BTL_POSPOKE_WORK* wk, BtlPokePos pos1, BtlPokePos pos2 );
extern u8 BTL_POSPOKE_GetClientEmptyPos( const BTL_POSPOKE_WORK* wk, u8 clientID, u8* pos );
extern BOOL BTL_POSPOKE_IsExistPokemon( const BTL_POSPOKE_WORK* wk, u8 pokeID );


