//=============================================================================================
/**
 * @file	hand_tokusei.c
 * @brief	ポケモンWB バトルシステム	イベントファクター[とくせい]追加処理
 * @author	taya
 *
 * @date	2008.11.11	作成
 */
//=============================================================================================

#include "poke_tool/poketype.h"

#include "../btl_common.h"
#include "../btl_event_factor.h"

#include "hand_tokusei.h"

#include "tokusei/hand_tokusei_common.h"




BTL_EVENT_FACTOR*  BTL_HANDLER_TOKUSEI_Add( const BTL_POKEPARAM* pp )
{
	static const struct {
		PokeTokusei				tokusei;
		pTokuseiEventAddFunc	func;
	}funcTbl[] = {
		{ POKETOKUSEI_IKAKU,		HAND_TOK_ADD_Ikaku },
		{ POKETOKUSEI_KURIABODY,	HAND_TOK_ADD_ClearBody },
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
	return NULL;

//	BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, agi, pokeID, 
}



