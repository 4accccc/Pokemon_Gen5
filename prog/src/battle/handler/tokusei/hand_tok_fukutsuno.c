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
static void handler_WazaExeFix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );


//--------------------------------------------------------------
/**
 * ハンドラテーブル
*/
//--------------------------------------------------------------
static const BtlEventHandlerTable HandlerTable[] = {
	{ BTL_EVENT_WAZA_EXECUTE_FAIL, handler_WazaExeFix },
	{ BTL_EVENT_NULL, NULL },
};

BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fukutsuno( u16 pri, u16 tokID, u8 pokeID )
{
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}


// BTL_EVENT_WAZA_EXECUTE_FAIL:ワザ出し失敗確定
static void handler_WazaExeFix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// ワザ失敗理由=ひるみ, 対象ポケ=自分で発動
	if( (BTL_EVENTVAR_GetValue(BTL_EVAR_FAIL_CAUSE) == SV_WAZAFAIL_SHRINK)
	&&	(BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
	){
		BTL_EVWK_WAZAEXE_FAIL* evwk = (BTL_EVWK_WAZAEXE_FAIL*) BTL_EVENTVAR_GetValue( BTL_EVAR_WORK_ADRS );

		evwk->flag_rankEffect = TRUE;
		evwk->rankType = BPP_AGILITY;
		evwk->rankVolume = 1;
		evwk->tokWinFlag = TRUE;
	}
}

