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


#include "btl_server_flow_local.h"

typedef enum {

  TRITEM_RESULT_NORMAL = 0, ///< 通常道具効果発生
  TRITEM_RESULT_ESCAPE,     ///< 強制的に逃げる

}TrItemResult;

extern u8 BTL_SVFSUB_RegisterTargets( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* attacker, BtlPokePos targetPos,
  const SVFL_WAZAPARAM* wazaParam, BTL_POKESET* rec );


/**
 *  レベルアップ計算
 */
extern void BTL_SVFSUB_CalcExp( BTL_SVFLOW_WORK* wk, BTL_PARTY* party, const BTL_POKEPARAM* deadPoke, CALC_EXP_WORK* result );

/**
 *  トレーナーアイテム使用処理ルート
 */
extern TrItemResult  BTL_SVFSUB_TrainerItemProc( BTL_SVFLOW_WORK* wk, BTL_POKEPARAM* bpp, u16 itemID, u8 actParam, u8 targetIdx );


/**
 *  勝敗判定
 */
extern BtlResult BTL_SVFSUB_CheckBattleResult( BTL_SVFLOW_WORK* wk );


/*====================================================================================================*/
/*                                                                                                    */
/*  相性記録ワーク                                                                                    */
/*                                                                                                    */
/*====================================================================================================*/
extern void BTL_DMGAFF_REC_Init( BTL_DMGAFF_REC* rec );
extern void BTL_DMGAFF_REC_Add( BTL_DMGAFF_REC* rec, u8 pokeID, BtlTypeAff aff );
extern BtlTypeAff BTL_DMGAFF_REC_Get( const BTL_DMGAFF_REC* rec, u8 pokeID );
extern BtlTypeAff BTL_DMGAFF_REC_GetIfEnable( const BTL_DMGAFF_REC* rec, u8 pokeID );
