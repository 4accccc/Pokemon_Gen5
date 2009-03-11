//=============================================================================================
/**
 * @file	hand_tok_fukutsuno.c
 * @brief	ポケモンWB バトルシステム	イベントファクター[とくせい]：『ふくつのこころ』
 * @author	taya
 *
 * @date	2009.03.11	作成
 */
//=============================================================================================
#include "hand_tokusei_common.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void handler_ShrinkFix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWork, u8 pokeID, int* work );


//--------------------------------------------------------------
/**
 * ハンドラテーブル
*/
//--------------------------------------------------------------
static const BtlEventHandlerTable HandlerTable[] = {
	{ BTL_EVENT_SHRINK_FIX, handler_ShrinkFix },
	{ BTL_EVENT_NULL, NULL },
};
//BTL_EVENT_RemoveFactor

BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fukutsuno( u16 pri, u8 pokeID )
{
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}


// BTL_EVENT_SHRINK_FIX:ひるみ確定
static void handler_ShrinkFix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		BtlPokePos myPos = BTL_SVFLOW_CheckExistFrontPokeID( flowWk, pokeID );
		BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, myPos );
		BTL_SERVER_RECEPT_RankUpEffect( flowWk, myPos, BPP_AGILITY, 1 );
		BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, myPos );
	}
}

