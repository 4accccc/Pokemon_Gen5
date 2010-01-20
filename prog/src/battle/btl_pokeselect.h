//=============================================================================================
/**
 * @file  btl_pokeselect.h
 * @brief ポケモンWB バトルシステム ポケモン選択画面用パラメータ等の定義
 * @author  taya
 *
 * @date  2008.09.25  作成
 */
//=============================================================================================
#ifndef __BTL_POKESELECT_H__
#define __BTL_POKESELECT_H__

#include "btl_common.h"
#include "btl_main_def.h"

#include "app\b_plist.h"

//------------------------------
/**
 *  選択できない理由
 */
//------------------------------
typedef enum {
  BTL_POKESEL_CANT_NONE=0,    ///< 選択できます

  BTL_POKESEL_CANT_DEAD,      ///< 死んでるから
  BTL_POKESEL_CANT_FIGHTING,  ///< 戦ってるから
  BTL_POKESEL_CANT_SELECTED,  ///< 同じターンでもう選ばれたから

}BtlPokeselReason;

//------------------------------------------------------
/**
 *  ポケモン選択画面用パラメータ
 */
//------------------------------------------------------
typedef struct {

  const BTL_PARTY*  party;        ///< パーティデータ
  u8                numSelect;    ///< 選択するポケモン数
  u8                prohibit[ BTL_PARTY_MEMBER_MAX ];
  u8                bplMode;      ///< バトルリスト画面モード

}BTL_POKESELECT_PARAM;

//------------------------------------------------------
/**
 *  結果格納用構造体
 */
//------------------------------------------------------
typedef struct {

  u8      selIdx[ BTL_POSIDX_MAX ]; ///< 選択したポケモンの並び順（現在の並び順で先頭を0と数える）
  u8      cnt;                      ///< 選択した数
  u8      max;                      ///< 選択する数

}BTL_POKESELECT_RESULT;



extern void BTL_POKESELECT_PARAM_Init( BTL_POKESELECT_PARAM* param, const BTL_PARTY* party, u8 numSelect, u8 aliveOnly );
extern void BTL_POKESELECT_PARAM_SetProhibitFighting( BTL_POKESELECT_PARAM* param, u8 numCover );
extern void BTL_POKESELECT_PARAM_SetProhibit( BTL_POKESELECT_PARAM* param, BtlPokeselReason reason, u8 idx );
extern u8 BTL_POKESELECT_PARAM_GetNumSelect( const BTL_POKESELECT_PARAM* param );

extern void BTL_POKESELECT_RESULT_Init( BTL_POKESELECT_RESULT *result, const BTL_POKESELECT_PARAM* param );
extern void BTL_POKESELECT_RESULT_Push( BTL_POKESELECT_RESULT *result, u8 idx );
extern void BTL_POKESELECT_RESULT_Pop( BTL_POKESELECT_RESULT *result );

extern BOOL BTL_POKESELECT_IsDone( const BTL_POKESELECT_RESULT* result );
extern u8 BTL_POKESELECT_RESULT_GetCount( const BTL_POKESELECT_RESULT* result );
extern u8 BTL_POKESELECT_RESULT_GetLast( const BTL_POKESELECT_RESULT* result );
extern u8 BTL_POKESELECT_RESULT_Get( const BTL_POKESELECT_RESULT* result, u8 idx );


#endif
