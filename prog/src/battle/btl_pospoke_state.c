//=============================================================================================
/**
 * @file  btl_pospoke_state.c
 * @brief ポケモンWB バトルシステム  場にいるポケモンの生存確認用構造体と処理ルーチン
 * @author  taya
 *
 * @date  2009.07.27  作成
 */
//=============================================================================================

#include "btl_common.h"
#include "btl_pokeparam.h"
#include "btl_pospoke_state.h"


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void set_pos_state( BTL_POSPOKE_WORK* wk, const BTL_POKE_CONTAINER* pokeCon, BtlPokePos pos );



void BTL_POSPOKE_InitWork( BTL_POSPOKE_WORK* wk, const BTL_POKE_CONTAINER* pokeCon, BtlRule rule )
{
  u32 i;

  for(i=0; i<NELEMS(wk->state); ++i){
    wk->state[i] = POSPOKE_EMPTY;
  }

  switch( rule ){
  case BTL_RULE_TRIPLE:
    set_pos_state( wk, pokeCon, BTL_POS_1ST_2 );
    set_pos_state( wk, pokeCon, BTL_POS_2ND_2 );
    /* fallthru */
  case BTL_RULE_DOUBLE:
    set_pos_state( wk, pokeCon, BTL_POS_1ST_1 );
    set_pos_state( wk, pokeCon, BTL_POS_2ND_1 );
    /* fallthru */
  case BTL_RULE_SINGLE:
    set_pos_state( wk, pokeCon, BTL_POS_1ST_0 );
    set_pos_state( wk, pokeCon, BTL_POS_2ND_0 );
    break;
  }
}
void BTL_POSPOKE_Update( BTL_POSPOKE_WORK* wk, const BTL_POKE_CONTAINER* pokeCon )
{
  u32 i;
  for(i=0; i<NELEMS(wk->state); ++i){
    if( wk->state[i] != POSPOKE_EMPTY ){
      set_pos_state( wk, pokeCon, i );
    }
  }
}
static void set_pos_state( BTL_POSPOKE_WORK* wk, const BTL_POKE_CONTAINER* pokeCon, BtlPokePos pos )
{
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( pokeCon, pos );
  if( (bpp != NULL)
  &&  (!BPP_IsDead(bpp) )
  ){
    wk->state[ pos ] = POSPOKE_EXIST;
  }
  else{
    wk->state[ pos ] = POSPOKE_DEAD;
  }
}

void BTL_POSPOKE_Compair( const BTL_POSPOKE_WORK* wk_before, const BTL_POSPOKE_WORK* wk_after, BTL_POSPOKE_COMPAIR_RESULT* result )
{
  u32 i, cnt;
  for(i=0, cnt=0; i<NELEMS(wk_before->state); ++i)
  {
    if( (wk_before->state[i] == POSPOKE_EXIST)
    &&  (wk_after->state[i] == POSPOKE_DEAD)
    ){
      result->pos[ cnt++ ] = i;
    }
  }
  result->count = cnt;
}
void BTL_POSPOKE_CopyWork( const BTL_POSPOKE_WORK* src, BTL_POSPOKE_WORK* dst )
{
  GFL_STD_MemCopy( src, dst, sizeof(BTL_POSPOKE_WORK) );
}


