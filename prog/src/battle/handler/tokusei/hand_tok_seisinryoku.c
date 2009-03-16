//=============================================================================================
/**
 * @file	hand_tok_seisinryoku.c
 * @brief	ポケモンWB バトルシステム	イベントファクター[とくせい]：『せいしんりょく』
 * @author	taya
 *
 * @date	2009.03.11	作成
 */
//=============================================================================================
#include "hand_tokusei_common.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void handler_ShrinkCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWork, u8 pokeID, int* work );


//--------------------------------------------------------------
/**
 * ハンドラテーブル
*/
//--------------------------------------------------------------
static const BtlEventHandlerTable HandlerTable[] = {
	{ BTL_EVENT_SHRINK_CHECK, handler_ShrinkCheck },
	{ BTL_EVENT_NULL, NULL },
};
//BTL_EVENT_RemoveFactor

BTL_EVENT_FACTOR*  HAND_TOK_ADD_Seisinryoku( u16 pri, u8 pokeID )
{
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}


// BTL_EVENT_SHRINK_CHECK:ひるみ発生チェック
static void handler_ShrinkCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		// 発生確率を0に書き換え
		BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, 0 );
	}
}

