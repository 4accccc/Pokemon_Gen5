//============================================================================================
/**
 * @file    mystery_pokemoncreate.c
 * @bfief   不思議通信データからポケモンを作る関数
 * @author  k.ohno
 * @date    10.01.27
 */
//============================================================================================


#include <gflib.h>
#include "poke_tool/poke_tool.h"
#include "savedata/mystery_data.h"



POKEMON_PARAM* MYSTERY_PokemonCreate(const GIFT_PRESENT_POKEMON* pGift, HEAPID heapID)
{
  POKEMON_PARAM* pp;
  u16 level=pGift->level;

  
  if(pGift->mons_no > MONSNO_END){  //モンスター番号が存在しない物は作らない
    return NULL;
  }
  if(pGift->mons_no == 0){  //モンスター番号が存在しない物は作らない
    return NULL;
  }
  if(pGift->mons_no == 0){  //モンスター番号が存在しない物は作らない
    return NULL;
  }
  if(pGift->level > 100){  //レベル100より上は作らない
    return NULL;
  }
  
  if(pGift->level == 0){  //レベル０指定は乱数
    level = GFUser_GetPublicRand(99)+1;
  }

  
  pp = PP_Create( pGift->mons_no, level, u64 id, HEAPID heapID );

  

  PP_SetupEx

  

}



