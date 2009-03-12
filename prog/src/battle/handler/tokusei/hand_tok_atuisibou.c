//=============================================================================================
/**
 * @file	hand_tok_atuisibou.c
 * @brief	ポケモンWB バトルシステム	イベントファクター[とくせい]：『あついしぼう』
 * @author	taya
 *
 * @date	2009.03.12	作成
 */
//=============================================================================================
#include "hand_tokusei_common.h"

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void handler_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );


//--------------------------------------------------------------
/**
 * ハンドラテーブル
*/
//--------------------------------------------------------------
static const BtlEventHandlerTable HandlerTable[] = {
	{ BTL_EVENT_ATTACKER_POWER, handler_AtkPower },
	{ BTL_EVENT_NULL, NULL },
};
//BTL_EVENT_RemoveFactor

BTL_EVENT_FACTOR*  HAND_TOK_ADD_AtuiSibou( u16 pri, u8 pokeID )
{
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}


// BTL_EVENT_ATTACKER_POWER: 攻撃側の攻撃値決定
static void handler_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// ダメージ対象が自分
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
	{
		WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
		PokeType  type = WAZADATA_GetType( waza );
		// ワザタイプが氷か炎
		if( (type == POKETYPE_KOORI) || (type == POKETYPE_FIRE) )
		{
				u32 power = BTL_EVENTVAR_GetValue( BTL_EVAR_POWER );
				power /= 2;
				BTL_EVENTVAR_SetValue( BTL_EVAR_POWER, power );
				BTL_Printf("pokeID:%d とくせい[あついしぼう]により、攻撃威力半減\n", pokeID);
		}
	}
}



