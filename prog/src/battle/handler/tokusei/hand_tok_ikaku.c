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


BTL_EVENT_FACTOR*  HAND_TOK_ADD_Ikaku( u16 pri, u8 pokeID )
{
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

// BTL_EVENT_MEMBER_COMP:戦闘メンバーが出そろった直後のハンドラ
static void handler_MemberComp( BTL_SERVER* server, u8 pokeID, int* work )
{
	// work[0] を発動フラグとして使用
	if( work[0] == 0 )
	{
		BtlPokePos myPos = BTL_SERVER_CheckExistFrontPokeID( server, pokeID );

		BTL_SERVER_RECEPT_TokuseiWinIn( server, myPos );
		BTL_SERVER_RECEPT_RankDownEffect( server, EXPOS_MAKE(BTL_EXPOS_ENEMY_ALL, myPos), BPP_ATTACK, 1 );
		BTL_SERVER_RECEPT_TokuseiWinOut( server, myPos );

		work[0] = 1;
	}
}

