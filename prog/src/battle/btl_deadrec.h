//=============================================================================================
/**
 * @file  btl_deadrec.h
 * @brief ポケモンWB 死亡ポケ記録
 * @author  taya
 *
 * @date  2009.11.23  作成
 */
//=============================================================================================
#pragma once

#include "btl_common.h"

enum {
  BTL_DEADREC_TURN_MAX = 4,   ///< 記録ターン数
};

/**
 *  ポケ死亡レコード
 */
typedef struct {

  struct {
    u8 cnt;
    u8 fDead[ BTL_POKEID_MAX ];          ///< 死亡フラグ（ポケモンID毎）
    u8 fExpChecked[ BTL_POKEID_MAX ];    ///< 経験値チェック処理済みフラグ（ポケモンID毎）
  }record[BTL_DEADREC_TURN_MAX];

}BTL_DEADREC;


extern void BTL_DEADREC_Init( BTL_DEADREC* wk );
extern void BTL_DEADREC_StartTurn( BTL_DEADREC* wk );
extern void BTL_DEADREC_Add( BTL_DEADREC* wk, u8 pokeID );

extern u8 BTL_DEADREC_GetCount( const BTL_DEADREC* wk, u8 turn );
extern u8 BTL_DEADREC_GetPokeID( const BTL_DEADREC* wk, u8 turn, u8 idx );
extern BOOL BTL_DEADREC_GetExpCheckedFlag( const BTL_DEADREC* wk, u8 turn, u8 idx );
extern void BTL_DEADREC_SetExpCheckedFlag( BTL_DEADREC* wk, u8 turn, u8 idx );

