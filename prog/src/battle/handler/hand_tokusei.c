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
#include "poke_tool/monsno_def.h"

#include "../btl_common.h"
#include "../btl_calc.h"
#include "../btl_field.h"
#include "../btl_event_factor.h"

#include "hand_tokusei.h"

#include "tokusei/hand_tokusei_common.h"


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void handler_Iromagane( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_HardRock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kasoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_hpborder_powerup( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType );
static void handler_Mouka( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Gekiryu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Sinryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MusinoSirase( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Konjou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tousousin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Technician( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_FusiginaUroko( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SkillLink( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Surudoime( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tanjun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_ReafGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Donkan( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Amefurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Hideri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Sunaokosi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Yukifurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_weather_change( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather );
static void handler_AirLock_Appear( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_AirLock_ChangeWeather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );





//=============================================================================================
/**
 * ポケモンの「とくせい」ハンドラをシステムに追加
 *
 * @param   pp		
 *
 * @retval  BTL_EVENT_FACTOR*		
 */
//=============================================================================================
BTL_EVENT_FACTOR*  BTL_HANDLER_TOKUSEI_Add( const BTL_POKEPARAM* pp )
{
	static const struct {
		PokeTokusei				tokusei;
		pTokuseiEventAddFunc	func;
	}funcTbl[] = {
		{ POKETOKUSEI_IKAKU,					HAND_TOK_ADD_Ikaku },
		{ POKETOKUSEI_KURIABODY,			HAND_TOK_ADD_ClearBody },
		{ POKETOKUSEI_SIROIKEMURI,		HAND_TOK_ADD_ClearBody },	// しろいけむり=クリアボディと等価
		{ POKETOKUSEI_SEISINRYOKU,		HAND_TOK_ADD_Seisinryoku },
		{ POKETOKUSEI_FUKUTUNOKOKORO,	HAND_TOK_ADD_Fukutsuno },
		{ POKETOKUSEI_ATUISIBOU,			HAND_TOK_ADD_AtuiSibou },
		{ POKETOKUSEI_KAIRIKIBASAMI,	HAND_TOK_ADD_KairikiBasami },
		{ POKETOKUSEI_TIKARAMOTI,			HAND_TOK_ADD_Tikaramoti },
		{ POKETOKUSEI_YOGAPAWAA,			HAND_TOK_ADD_Tikaramoti },	// ヨガパワー = ちからもちと等価
		{ POKETOKUSEI_IROMEGANE,			HAND_TOK_ADD_Iromegane },
		{ POKETOKUSEI_KASOKU,					HAND_TOK_ADD_Kasoku },
		{ POKETOKUSEI_MOUKA,					HAND_TOK_ADD_Mouka },
		{ POKETOKUSEI_GEKIRYUU,				HAND_TOK_ADD_Gekiryu },
		{ POKETOKUSEI_SINRYOKU,				HAND_TOK_ADD_Sinryoku },
		{ POKETOKUSEI_MUSINOSIRASE,		HAND_TOK_ADD_MusinoSirase },
		{ POKETOKUSEI_KONJOU,					HAND_TOK_ADD_Konjou },
		{ POKETOKUSEI_SUKIRURINKU,		HAND_TOK_ADD_SkillLink },
		{ POKETOKUSEI_SURUDOIME,			HAND_TOK_ADD_Surudoime },
		{ POKETOKUSEI_TANJUN,					HAND_TOK_ADD_Tanjun },
		{ POKETOKUSEI_HAADOROKKU,			HAND_TOK_ADD_HardRock },
		{ POKETOKUSEI_FUSIGINAUROKO,	HAND_TOK_ADD_FusiginaUroko },
		{ POKETOKUSEI_TOUSOUSIN,			HAND_TOK_ADD_Tousousin },
		{ POKETOKUSEI_RIIFUGAADO,			HAND_TOK_ADD_ReafGuard },
		{ POKETOKUSEI_AMEFURASI,			HAND_TOK_ADD_Amefurasi },
		{ POKETOKUSEI_HIDERI,					HAND_TOK_ADD_Hideri },
		{ POKETOKUSEI_SUNAOKOSI,			HAND_TOK_ADD_Sunaokosi },
		{ POKETOKUSEI_YUKIFURASI,			HAND_TOK_ADD_Yukifurasi },
		{ POKETOKUSEI_EAROKKU,				HAND_TOK_ADD_AirLock },
		{ POKETOKUSEI_NOOTENKI,				HAND_TOK_ADD_AirLock },	// ノーてんき = エアロックと等価
		{ POKETOKUSEI_TEKUNISYAN,			HAND_TOK_ADD_Technician },
		{ POKETOKUSEI_DONKAN,					HAND_TOK_ADD_Donkan },
	};

	int i;
	u16 tokusei = BTL_POKEPARAM_GetValue( pp, BPP_TOKUSEI );

	for(i=0; i<NELEMS(funcTbl); i++)
	{
		if( funcTbl[i].tokusei == tokusei )
		{
			u16 agi = BTL_POKEPARAM_GetValue( pp, BPP_AGILITY );
			u8 pokeID = BTL_POKEPARAM_GetID( pp );

			BTL_Printf("ポケモン[%d]の とくせい(%d)ハンドラを追加\n", pokeID, tokusei);
			return funcTbl[i].func( agi, pokeID );
		}
	}

	BTL_Printf("とくせい(%d)はハンドラが用意されていない\n", tokusei);
	return NULL;
}

//=============================================================================================
/**
 * ポケモンの「とくせい」ハンドラをシステムから削除
 *
 * @param   pp		
 *
 */
//=============================================================================================
void BTL_HANDLER_TOKUSEI_Remove( const BTL_POKEPARAM* pp )
{
	BTL_EVENT_FACTOR* factor;
	u8 pokeID = BTL_POKEPARAM_GetID( pp );

	while( (factor = BTL_EVENT_SeekFactor(BTL_EVENT_FACTOR_TOKUSEI, pokeID)) != NULL )
	{
		BTL_Printf("ポケモン[%d]の とくせいハンドラを除去\n", pokeID);
		BTL_EVENT_RemoveFactor( factor );
	}
}

//------------------------------------------------------------------------------
/**
 *	とくせい「いろめがね」
 */
//------------------------------------------------------------------------------
// ダメージ計算最終段階のハンドラ
static void handler_Iromagane( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// 攻撃側が自分で
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
	{
		// 相性イマイチの時
		BtlTypeAffAbout aff = BTL_CALC_TypeAffAbout( BTL_EVENTVAR_GetValue(BTL_EVAR_TYPEAFF) );
		if( aff == BTL_TYPEAFF_ABOUT_DISADVANTAGE )
		{
			// ダメージ２倍
			u32 dmg = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
			dmg *= 2;
			BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, dmg );
			BTL_Printf("ポケ[%d]の いろめがね でダメージ２倍\n", pokeID);
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Iromegane( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_WAZA_DMG_PROC2, handler_Iromagane },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「ハードロック」
 */
//------------------------------------------------------------------------------
// ダメージ計算最終段階のハンドラ
static void handler_HardRock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// 防御側が自分で
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
	{
		// こうかバツグンの時
		BtlTypeAffAbout aff = BTL_CALC_TypeAffAbout( BTL_EVENTVAR_GetValue(BTL_EVAR_TYPEAFF) );
		if( aff == BTL_TYPEAFF_ABOUT_ADVANTAGE )
		{
			// ダメージ75％
			u32 dmg = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
			dmg = (dmg*75) / 100;
			BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, dmg );
			BTL_Printf("ポケ[%d]の ハードロック でダメージ75％\n", pokeID);
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_HardRock( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_WAZA_DMG_PROC2, handler_HardRock },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「かそく」
 */
//------------------------------------------------------------------------------
// ターンチェックのハンドラ
static void handler_Kasoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	BtlPokePos myPos = BTL_SVFLOW_CheckExistFrontPokeID( flowWk, pokeID );

	BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
	BTL_SERVER_RECEPT_RankUpEffect( flowWk, myPos, BPP_AGILITY, 1 );
	BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kasoku( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_TURNCHECK, handler_Kasoku },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}


//--------------------------------------------------------------------------
/**
 * HPが1/3の時に、特定タイプのワザ威力を上げるとくせいの共通処理
 *
 * @param   flowWk		
 * @param   pokeID		
 * @param   wazaType		
 *
 */
//--------------------------------------------------------------------------
static void common_hpborder_powerup( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType )
{
	// 攻撃側が自分で
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
	{
		// HP 1/3 以下で
		const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
		if( BTL_POKEPARAM_GetHPBorder(bpp) <= BPP_HPBORDER_YELLOW )
		{
			// 使うのが指定タイプワザなら
			if( WAZADATA_GetType( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZAID) ) == wazaType )
			{
				// 威力２倍
				u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_POWER );
				pow *= 2;
				BTL_EVENTVAR_SetValue( BTL_EVAR_POWER, pow );
			}
		}
	}
}
//------------------------------------------------------------------------------
/**
 *	とくせい「もうか」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Mouka( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_hpborder_powerup( flowWk, pokeID, POKETYPE_FIRE );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Mouka( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_ATTACKER_POWER, handler_Mouka },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「げきりゅう」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Gekiryu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_hpborder_powerup( flowWk, pokeID, POKETYPE_WATER );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Gekiryu( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_ATTACKER_POWER, handler_Gekiryu },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「しんりょく」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Sinryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_hpborder_powerup( flowWk, pokeID, POKETYPE_KUSA );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sinryoku( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_ATTACKER_POWER, handler_Sinryoku },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「むしのしらせ」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_MusinoSirase( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_hpborder_powerup( flowWk, pokeID, POKETYPE_MUSHI );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_MusinoSirase( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_ATTACKER_POWER, handler_MusinoSirase },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「こんじょう」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Konjou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// 攻撃側が自分で
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
	{
		// 状態異常で
		const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
		if( BTL_POKEPARAM_GetPokeSick(bpp) != POKESICK_NULL )
		{
			WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
			// ダメージタイプが物理の時
			if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
			{
				// 威力２倍
				u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_POWER );
				pow *= 2;
				BTL_EVENTVAR_SetValue( BTL_EVAR_POWER, pow );
				BTL_Printf("ポケ[%d]の こんじょう で威力２倍\n", pokeID);
			}
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Konjou( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_ATTACKER_POWER, handler_Konjou },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「とうそうしん」
 */
//------------------------------------------------------------------------------
// ワザ威力決定のハンドラ
static void handler_Tousousin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// 攻撃側が自分
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
	{
		const BTL_POKEPARAM* myParam = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
		const BTL_POKEPARAM* targetParam = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk,
					BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) );

		u8 mySex = BTL_POKEPARAM_GetValue( myParam, BPP_SEX );
		u8 targetSex = BTL_POKEPARAM_GetValue( targetParam, BPP_SEX );

		// 互いに性別不明じゃない場合
		if( (mySex!=PTL_SEX_UNKNOWN) && (targetSex!=PTL_SEX_UNKNOWN) )
		{
			u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER );
			if( mySex == targetSex ){
				pow = (pow * FX32_CONST(1.25f)) >> FX32_SHIFT;
			}else{
				pow = (pow * FX32_CONST(0.75f)) >> FX32_SHIFT;
			}
			BTL_EVENTVAR_SetValue( BTL_EVAR_WAZA_POWER, pow );
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tousousin( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_WAZA_POWER, handler_Tousousin },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「テクニシャン」
 */
//------------------------------------------------------------------------------
// ワザ威力決定のハンドラ
static void handler_Technician( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// 攻撃側が自分
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
	{
		// 素の威力60以下なワザなら1.5倍にする
		WazaID  waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
		if( WAZADATA_GetPower(waza) <= 60 )
		{
			u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER );
			pow = (pow * FX32_CONST(1.5f)) >> FX32_SHIFT;
			BTL_EVENTVAR_SetValue( BTL_EVAR_WAZA_POWER, pow );
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Technician( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_WAZA_POWER, handler_Technician },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「ふしぎなうろこ」
 */
//------------------------------------------------------------------------------
///< 防御能力決定のハンドラ
static void handler_FusiginaUroko( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// 攻撃側が自分で
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
	{
		// ミロカロスで状態異常で
		const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
		if( (BTL_POKEPARAM_GetPokeSick(bpp) != POKESICK_NULL)
		&&	(BTL_POKEPARAM_GetMonsNo(bpp) == MONSNO_MIROKAROSU)
		){
			WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
			// ダメージタイプが特殊の時
			if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_SPECIAL )
			{
				// 防御２倍
				u32 guard = BTL_EVENTVAR_GetValue( BTL_EVAR_GUARD );
				guard *= 2;
				BTL_EVENTVAR_SetValue( BTL_EVAR_GUARD, guard );
				BTL_Printf("ポケ[%d]の ふしぎなまもり で防御２倍\n", pokeID);
			}
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_FusiginaUroko( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_DEFENDER_GUARD, handler_FusiginaUroko },	///< 防御能力決定のハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「スキルリンク」
 */
//------------------------------------------------------------------------------
// 攻撃回数決定のハンドラ
static void handler_SkillLink( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// 攻撃側が自分の時
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
	{
		// ヒット回数を最大に
		u16 max = BTL_EVENTVAR_GetValue( BTL_EVAR_HITCOUNT_MAX );
		BTL_EVENTVAR_SetValue( BTL_EVAR_HITCOUNT, max );
		BTL_Printf("POKE[%d]の スキルリンク で、ヒット回数を最大の%dにした\n", max);
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_SkillLink( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_WAZA_HIT_COUNT, handler_SkillLink },	// 攻撃回数決定のハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「するどいめ」
 */
//------------------------------------------------------------------------------
// ランクダウン直前処理のハンドラ
static void handler_Surudoime( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
	&&	(BTL_EVENTVAR_GetValue(BTL_EVAR_STATUS_TYPE) ==  WAZA_RANKEFF_HIT)
	){
		BTL_EVENTVAR_SetValue( BTL_EVAR_FAIL_FLAG, TRUE );
		BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
		BTL_SERVER_RECTPT_SetMessage( flowWk, BTL_STRID_SET_RankdownFail_HIT, pokeID );
		BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Surudoime( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_BEFORE_RANKDOWN, handler_Surudoime },	// ランクダウン直前処理のハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「たんじゅん」
 */
//------------------------------------------------------------------------------
// ランクダウン直前＆ランクアップ直前のハンドラ
static void handler_Tanjun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		u8 volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
		volume *= 2;
		BTL_EVENTVAR_SetValue( BTL_EVAR_VOLUME, volume );
		BTL_Printf("ポケ[%d]の たんじゅん により効果倍増\n");
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tanjun( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_BEFORE_RANKDOWN, handler_Tanjun },	// ランクダウン直前処理のハンドラ
		{ BTL_EVENT_BEFORE_RANKUP, handler_Tanjun },		// ランクアップ直前処理のハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「リーフガード」
 */
//------------------------------------------------------------------------------
// 状態異常くらう前のハンドラ
static void handler_ReafGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// くらう側が自分
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
	{
		// 天候が晴れ
		if( BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE )
		{
			// 状態異常の種類がポケモン系
			if( BTL_EVENTVAR_GetValue(BTL_EVAR_SICKID) < POKESICK_MAX )
			{
				BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, POKESICK_NULL );
				if( BTL_EVENTVAR_GetValue(BTL_EVAR_ALMOST_FLAG) )
				{
					BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
					BTL_SERVER_RECTPT_SetMessage( flowWk, BTL_STRID_SET_NoEffect, pokeID );
					BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
				}
			}
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_ReafGuard( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_ADD_SICK, handler_ReafGuard },	// 状態異常くらう前のハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「どんかん」
 */
//------------------------------------------------------------------------------
// 状態異常くらう前のハンドラ
static void handler_Donkan( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// くらう側が自分
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
	{
		// メロメロ
		if( BTL_EVENTVAR_GetValue(BTL_EVAR_SICKID) == WAZASICK_MEROMERO )
		{
			BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, WAZASICK_MEROMERO );
			if( BTL_EVENTVAR_GetValue(BTL_EVAR_ALMOST_FLAG) )
			{
				BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
				BTL_SERVER_RECTPT_SetMessage( flowWk, BTL_STRID_SET_NoEffect, pokeID );
				BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
			}
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Donkan( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_ADD_SICK, handler_Donkan },	// 状態異常くらう前のハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「あめふらし」
 */
//------------------------------------------------------------------------------
// 自分が出場ハンドラ
static void handler_Amefurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_weather_change( flowWk, pokeID, BTL_WEATHER_RAIN );
	BTL_EVENT_RemoveFactor( myHandle );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Amefurasi( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_MEMBER_IN, handler_Amefurasi },	// 自分が出場ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「ひでり」
 */
//------------------------------------------------------------------------------
// 自分が出場ハンドラ
static void handler_Hideri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_weather_change( flowWk, pokeID, BTL_WEATHER_SHINE );
	BTL_EVENT_RemoveFactor( myHandle );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hideri( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_MEMBER_IN, handler_Hideri },	// 自分が出場ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「すなおこし」
 */
//------------------------------------------------------------------------------
// 自分が出場ハンドラ
static void handler_Sunaokosi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_weather_change( flowWk, pokeID, BTL_WEATHER_SAND );
	BTL_EVENT_RemoveFactor( myHandle );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sunaokosi( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_MEMBER_IN, handler_Sunaokosi },	// 自分が出場ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「ゆきふらし」
 */
//------------------------------------------------------------------------------
// 自分が出場ハンドラ
static void handler_Yukifurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_weather_change( flowWk, pokeID, BTL_WEATHER_SNOW );
	BTL_EVENT_RemoveFactor( myHandle );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yukifurasi( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_MEMBER_IN, handler_Yukifurasi },	// 自分が出場ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//-------------------------------------------
/**
 *	天候変化とくせいの共通処理
 */
//-------------------------------------------
static void common_weather_change( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
		BTL_SVFLOW_RECEPT_ChangeWeather( flowWk, weather );
		BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
	}
}

//------------------------------------------------------------------------------
/**
 *	とくせい「エアロック」&「ノーてんき」
 */
//------------------------------------------------------------------------------
// 自分が出場ハンドラ
static void handler_AirLock_Appear( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		BTL_Printf("ポケ[%d]のエアロック！ 自分出場ハンドラ\n", pokeID);
		if( BTL_FIELD_GetWeather() != BTL_WEATHER_NONE )
		{
			BTL_Printf("  ... 天候をフラットにするよ\n");
			BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
			BTL_SVFLOW_RECEPT_ChangeWeather( flowWk, BTL_WEATHER_NONE );
			BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
		}
	}
}
// 天候変化ハンドラ
static void handler_AirLock_ChangeWeather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	BTL_Printf("ポケ[%d]のエアロック！ 天候変化ハンドラ\n", pokeID);
	BTL_EVENTVAR_SetValue( BTL_EVAR_WEATHER, BTL_WEATHER_NONE );

	BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
	BTL_SERVER_RECTPT_StdMessage( flowWk, BTL_STRID_STD_WeatherLocked );
	BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_AirLock( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_MEMBER_IN,			handler_AirLock_Appear },					// 自分が出場ハンドラ
		{ BTL_EVENT_WEATHER_CHANGE, handler_AirLock_ChangeWeather },	// 天候変化ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}


