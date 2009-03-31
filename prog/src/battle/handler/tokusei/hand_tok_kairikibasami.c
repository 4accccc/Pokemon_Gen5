//=============================================================================================
/**
 * @file	hand_tok_kairikibasami.c
 * @brief	ポケモンWB バトルシステム  イベントファクター[とくせい]：『かいりきばさみ』
 * @author	taya
 *
 * @date	2009.03.12	作成
 */
//=============================================================================================
#include "../../btl_string.h"
#include "hand_tokusei_common.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void handler_BeforeRankDown( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );



//--------------------------------------------------------------
/**
 * ハンドラテーブル
*/
//--------------------------------------------------------------
static const BtlEventHandlerTable HandlerTable[] = {
	{ BTL_EVENT_BEFORE_RANKDOWN, handler_BeforeRankDown },
	{ BTL_EVENT_NULL, NULL },
};


BTL_EVENT_FACTOR*  HAND_TOK_ADD_KairikiBasami( u16 pri, u16 tokID, u8 pokeID )
{
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}


static void handler_BeforeRankDown( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
	&&	(BTL_EVENTVAR_GetValue(BTL_EVAR_STATUS_TYPE) ==  WAZA_RANKEFF_ATTACK)
	){
		BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_FLAG, TRUE );
		BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
		BTL_SERVER_RECTPT_SetMessage( flowWk, BTL_STRID_SET_RankdownFail_ATK, pokeID );
		BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
	}
}

