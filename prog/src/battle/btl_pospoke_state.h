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

  POSPOKE_EMPTY = 0, ///< 管理外（シングル戦で２体目の位置などは管理外になる）
  POSPOKE_EXIST,           ///< 生きているポケモンが存在する
  POSPOKE_DEAD,            ///<

}BtlPosPokeState;


/**
 * 状態保持構造体
 */
typedef struct {
  BtlPosPokeState  state[ BTL_POS_MAX ];
}BTL_POSPOKE_WORK;

/**
 * 状態比較結果構造体
 */
typedef struct {
  u32          count;
  BtlPokePos   pos[ BTL_POS_MAX ];
}BTL_POSPOKE_COMPAIR_RESULT;


extern void BTL_POSPOKE_InitWork( BTL_POSPOKE_WORK* wk, const BTL_POKE_CONTAINER* pokeCon, BtlRule rule );
extern void BTL_POSPOKE_Update( BTL_POSPOKE_WORK* wk, const BTL_POKE_CONTAINER* pokeCon );
extern void BTL_POSPOKE_Compair( const BTL_POSPOKE_WORK* wk_before, const BTL_POSPOKE_WORK* wk_after, BTL_POSPOKE_COMPAIR_RESULT* result );
extern void BTL_POSPOKE_CopyWork( const BTL_POSPOKE_WORK* src, BTL_POSPOKE_WORK* dst );


