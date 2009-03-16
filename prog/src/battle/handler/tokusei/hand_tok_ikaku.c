//=============================================================================================
/**
 * @file	hand_tok_ikaku.c
 * @brief	ポケモンWB バトルシステム	イベントファクター[とくせい]：『いかく』
 * @author	taya
 *
 * @date	2008.11.11	作成
 */
//=============================================================================================

#include "hand_tokusei_common.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void handler_MemberComp( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWork, u8 pokeID, int* work );



//--------------------------------------------------------------
/**
 * ハンドラテーブル
*/
//--------------------------------------------------------------
static const BtlEventHandlerTable HandlerTable[] = {
	{ BTL_EVENT_MEMBER_IN, handler_MemberComp },
	{ BTL_EVENT_NULL, NULL },
};
//BTL_EVENT_RemoveFactor

BTL_EVENT_FACTOR*  HAND_TOK_ADD_Ikaku( u16 pri, u8 pokeID )
{
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}


// ポケモンが出場した時のハンドラ
static void handler_MemberComp( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		BtlPokePos myPos = BTL_SVFLOW_CheckExistFrontPokeID( flowWk, pokeID );

		BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
		BTL_SERVER_RECEPT_RankDownEffect( flowWk, EXPOS_MAKE(BTL_EXPOS_ENEMY_ALL, myPos), BPP_ATTACK, 1 );
		BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );

		BTL_EVENT_RemoveFactor( myHandle );
	}
}

