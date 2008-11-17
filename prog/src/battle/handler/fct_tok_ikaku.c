//=============================================================================================
/**
 * @file	fct_tok_ikaku.c
 * @brief	ポケモンWB バトルシステム	イベントファクター[とくせい]：『いかく』
 * @author	taya
 *
 * @date	2008.11.11	作成
 */
//=============================================================================================

#include "fct_tokusei_common.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void handler_MemberComp( BTL_SERVER* server, u8 pokeID, int* work );



//--------------------------------------------------------------
/**
 * ハンドラテーブル
*/
//--------------------------------------------------------------
static const BtlEventHandlerTable HandlerTable[] = {
	{ BTL_EVENT_MEMBER_COMP, handler_MemberComp },
	{ BTL_EVENT_NULL, NULL },
};


BTL_EVENT_FACTOR*  FCT_TOK_ADD_Ikaku( u16 pri, u8 pokeID )
{
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}


static void handler_MemberComp( BTL_SERVER* server, u8 pokeID, int* work )
{
	if( work[0] == 0 )
	{
		u8 clientID = BTL_SERVER_RECEPT_PokeIDtoClientID( server, pokeID );

		BTL_Printf("いかくだよん\n");

		BTL_SERVER_RECEPT_TokuseiWinIn( server, clientID );
		BTL_SERVER_RECEPT_RankDownEffect( server, EXID_MAKE(BTL_EXCLIENT_ENEMY_ALL, clientID), BPP_ATTACK, 1 );
		BTL_SERVER_RECEPT_TokuseiWinOut( server, clientID );

		work[0] = 1;
	}
}

