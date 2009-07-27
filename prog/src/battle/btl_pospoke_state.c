//=============================================================================================
/**
 * @file  btl_pospoke_state.c
 * @brief ポケモンWB バトルシステム  場にいるポケモンの生存確認用構造体と処理ルーチン
 * @author  taya
 *
 * @date  2009.07.27  作成
 */
//=============================================================================================


#include "btl_pospoke_state.h"


void BTL_POSPOKE_InitWork( BTL_POSPOKE_WORK* wk, const BTL_POKE_CONTAINER* pokeCon, BtlRule rule )
{

}
void BTL_POSPOKE_Compair( const BTL_POSPOKE_WORK* wk1, const BTL_POSPOKE_WORK* wk2, BTL_POSPOKE_COMPAIR_RESULT* result )
{

}
void BTL_POSPOKE_CopyWork( const BTL_POSPOKE_WORK* src, BTL_POSPOKE_WORK* dst )
{
  GFL_STD_MemCopy( src, dst, sizeof(BTL_POSPOKE_WORK) );
}


