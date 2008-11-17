//=============================================================================================
/**
 * @file	pokeparty.c
 * @brief	èüéËÇ…çÏÇ¡ÇΩPOKEPARTY
 * @author	taya
 *
 * @date	2008.10.02
 */
//=============================================================================================

#include "pokeparty.h"

struct _POKEPARTY {
	POKEMON_PARAM*	member[ TEMOTI_POKEMAX ];
	u8				memberCount;

};



POKEPARTY * PokeParty_Create( HEAPID heapID )
{
	POKEPARTY* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(POKEPARTY) );
	int i;
	u32 ppSize;

	ppSize = PP_GetWorkSize();

	for(i=0; i<NELEMS(wk->member); i++)
	{
		wk->member[i] = GFL_HEAP_AllocClearMemory( heapID, ppSize );
	}

	wk->memberCount = 0;
	return wk;
}

POKEMON_PARAM* PokeParty_GetMemberPointer(const POKEPARTY * party, u8 pos)
{
	GF_ASSERT( pos < party->memberCount );
	return party->member[pos];
}

BOOL PokeParty_Add( POKEPARTY * party, const POKEMON_PARAM* poke )
{
	if( party->memberCount < TEMOTI_POKEMAX )
	{
		GFL_STD_MemCopy32( poke, party->member[party->memberCount], PP_GetWorkSize() );
		party->memberCount++;
		return TRUE;
	}
	else
	{
		GF_ASSERT(0);
		return FALSE;
	}
}

int PokeParty_GetPokeCount( const POKEPARTY* party )
{
	return party->memberCount;
}

