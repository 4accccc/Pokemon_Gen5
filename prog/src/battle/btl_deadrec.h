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
 *  ポケ死亡レコード（ターン単位）
 */

typedef struct {
  u8 cnt;
  u8 fExpChecked[ BTL_POKEID_MAX ];    ///< 経験値チェック処理済みフラグ（ポケモンID毎）
  u8 deadPokeID[ BTL_POKEID_MAX ];
}BTL_DEADREC_UNIT;

/**
 *  ポケ死亡レコード（本体）
 */
typedef struct {


  BTL_DEADREC_UNIT  record[ BTL_DEADREC_TURN_MAX ];


}BTL_DEADREC;


extern void BTL_DEADREC_Init( BTL_DEADREC* wk );
extern void BTL_DEADREC_StartTurn( BTL_DEADREC* wk );
extern void BTL_DEADREC_Add( BTL_DEADREC* wk, u8 pokeID );

extern u8 BTL_DEADREC_GetCount( const BTL_DEADREC* wk, u8 turn );
extern u8 BTL_DEADREC_GetPokeID( const BTL_DEADREC* wk, u8 turn, u8 idx );
extern BOOL BTL_DEADREC_GetExpCheckedFlag( const BTL_DEADREC* wk, u8 turn, u8 idx );
extern void BTL_DEADREC_SetExpCheckedFlag( BTL_DEADREC* wk, u8 turn, u8 idx );
extern u8 BTL_DEADREC_GetLastDeadPokeID( const BTL_DEADREC* wk );

