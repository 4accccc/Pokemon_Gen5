//=============================================================================================
/**
 * @file	fct_tokusei.c
 * @brief	ポケモンWB バトルシステム	イベントファクター[とくせい]追加処理
 * @author	taya
 *
 * @date	2008.11.11	作成
 */
//=============================================================================================


#include "../btl_common.h"
#include "../btl_event_factor.h"
#include "../poketype.h"

#include "fct_tokusei.h"

#include "fct_tokusei_common.h"


BTL_EVENT_FACTOR*  BTL_FCT_TOKUSEI_Add( const BTL_POKEPARAM* pp )
{
	static const struct {
		PokeTokusei				tokusei;
		pTokuseiEventAddFunc	func;
	}funcTbl[] = {
		{ POKETOKUSEI_IKAKU,		FCT_TOK_ADD_Ikaku },
		{ POKETOKUSEI_KURIABODY,	FCT_TOK_ADD_ClearBody },
	};

	int i;
	u16 tokusei = BTL_POKEPARAM_GetValue( pp, BPP_TOKUSEI );

	for(i=0; i<NELEMS(funcTbl); i++)
	{
		if( funcTbl[i].tokusei == tokusei )
		{
			u16 agi = BTL_POKEPARAM_GetValue( pp, BPP_AGILITY );
			u8 pokeID = BTL_POKEPARAM_GetID( pp );

			return funcTbl[i].func( agi, pokeID );
		}
	}

	BTL_Printf("とくせい(%d)は未実装\n", tokusei);

//	BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, agi, pokeID, 
}



