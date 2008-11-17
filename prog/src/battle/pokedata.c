//=============================================================================================
/**
 * @file	pokedata.c
 * @brief	勝手に作ったポケモンデータ操作モジュール
 * @author	taya
 *
 * @date	2008.10.02
 */
//=============================================================================================

#include "pokedata.h"
#include "personal.h"

struct _POKEMON_PARAM {
	u16  monsno;
	u8   lv;
	u8   wazaCount;

	u16  tokusei;

	u16  waza[ POKE_WAZA_MAX ];
};

u32 PP_GetWorkSize( void )
{
	return sizeof(POKEMON_PARAM);
}

POKEMON_PARAM* PP_Create( u16 monsno, u16 lv, const u16* wazaAry )
{
	POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( 0, sizeof(POKEMON_PARAM) );

	int i;

	pp->monsno = monsno;
	pp->lv = lv;
	pp->wazaCount=0;
	for(i=0; i<POKE_WAZA_MAX; i++)
	{
		if( wazaAry[i] != 0 )
		{
			pp->waza[pp->wazaCount++] = wazaAry[i];
		}
	}

	{
		u16 tok1 = PERSONAL_GetValue( monsno, PERSONAL_VALUE_TOKUSEI1 );
		u16 tok2 = PERSONAL_GetValue( monsno, PERSONAL_VALUE_TOKUSEI2 );

		if( tok2 == POKETOKUSEI_NULL )
		{
			pp->tokusei = tok1;
		}
		else
		{
			pp->tokusei = GFL_STD_MtRand(1)? tok1 : tok2;
		}
	}

	return pp;
}

void PP_Delete( POKEMON_PARAM* pp )
{
	GFL_HEAP_FreeMemory( pp );
}

u16 PP_GetMonsno( const POKEMON_PARAM* pp )
{
	return pp->monsno;
}

u16 PP_GetWazaCount( const POKEMON_PARAM* pp )
{
	return pp->wazaCount;
}

u16 PP_GetWazaNumber( const POKEMON_PARAM* pp, int idx )
{
	return pp->waza[ idx ];
}

u16 PP_GetTokusei( const POKEMON_PARAM* pp )
{
	return pp->tokusei;
}


