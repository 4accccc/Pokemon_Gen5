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
static void handler_Suisui( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Youryokuso( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Hayaasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tidoriasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Harikiri_HitRatio( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Harikiri_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Iromegane( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_HardRock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kasoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tekiouryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Mouka( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Gekiryu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Sinryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MusinoSirase( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_hpborder_powerup( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType );
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
static void handler_IceBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_AmeukeZara( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_weather_recover( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather );
static void handler_SunPower_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SunPower_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Rinpun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_TennoMegumi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_UruoiBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Dappi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_PoisonHeal( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );





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
		{ POKETOKUSEI_FIRUTAA,				HAND_TOK_ADD_HardRock },	// フィルター = ハードロックと等価
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
		{ POKETOKUSEI_URUOIBODY,			HAND_TOK_ADD_UruoiBody },
		{ POKETOKUSEI_POIZUNHIIRU,		HAND_TOK_ADD_PoisonHeal },
		{ POKETOKUSEI_AISUBODY,				HAND_TOK_ADD_IcoBody },
		{ POKETOKUSEI_AMEUKEZARA,			HAND_TOK_ADD_AmeukeZara },
		{ POKETOKUSEI_RINPUN,					HAND_TOK_ADD_Rinpun },
		{ POKETOKUSEI_TEKIOURYOKU,		HAND_TOK_ADD_Tekiouryoku },
		{ POKETOKUSEI_TENNOMEGUMI,		HAND_TOK_ADD_TennoMegumi },
		{ POKETOKUSEI_SANPAWAA,				HAND_TOK_ADD_SunPower },
		{ POKETOKUSEI_SUISUI,					HAND_TOK_ADD_Suisui },
		{ POKETOKUSEI_YOURYOKUSO,			HAND_TOK_ADD_Youryokuso },
		{ POKETOKUSEI_DAPPI,					HAND_TOK_ADD_Dappi },
		{ POKETOKUSEI_TIDORIASI,			HAND_TOK_ADD_Tidoriasi },
		{ POKETOKUSEI_HAYAASI,				HAND_TOK_ADD_Hayaasi },
		{ POKETOKUSEI_HARIKIRI,				HAND_TOK_ADD_Harikiri },

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
 *	とくせい「すいすい」
 */
//------------------------------------------------------------------------------
// すばやさ計算ハンドラ
static void handler_Suisui( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		// 雨のとき、すばやさ２倍
		if( BTL_FIELD_GetWeather() == BTL_WEATHER_RAIN )
		{
			u16 agi = BTL_EVENTVAR_GetValue( BTL_EVAR_AGILITY ) * 2;
			BTL_EVENTVAR_SetValue( BTL_EVAR_AGILITY, agi );
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Suisui( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_CALC_AGILITY, handler_Suisui },		// すばやさ計算ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「ようりょくそ」
 */
//------------------------------------------------------------------------------
// すばやさ計算ハンドラ
static void handler_Youryokuso( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		// はれの時、すばやさ２倍
		if( BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE )
		{
			u16 agi = BTL_EVENTVAR_GetValue( BTL_EVAR_AGILITY ) * 2;
			BTL_EVENTVAR_SetValue( BTL_EVAR_AGILITY, agi );
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Youryokuso( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_CALC_AGILITY, handler_Youryokuso },		// すばやさ計算ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「はやあし」
 */
//------------------------------------------------------------------------------
// すばやさ計算ハンドラ
static void handler_Hayaasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		// 状態異常の時、すばやさ1.5倍＆マヒによる減衰を無視する
		const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
		if( BTL_POKEPARAM_GetPokeSick(bpp) != POKESICK_NULL )
		{
			u32 agi = BTL_EVENTVAR_GetValue( BTL_EVAR_AGILITY );
			agi = (agi * FX32_CONST(1.5f)) >> FX32_SHIFT;
			BTL_EVENTVAR_SetValue( BTL_EVAR_AGILITY, agi );
			BTL_EVENTVAR_SetValue( BTL_EVAR_GEN_FLAG, FALSE );
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hayaasi( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_CALC_AGILITY, handler_Hayaasi },		// すばやさ計算ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「ちどりあし」
 */
//------------------------------------------------------------------------------
// 命中率計算ハンドラ
static void handler_Tidoriasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
	{
		const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
		if( BTL_POKEPARAM_CheckSick(bpp, WAZASICK_KONRAN) )
		{
			u32 hitRatio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
			hitRatio = BTL_CALC_MulRatio( hitRatio, BTL_CALC_TOK_CHIDORI_HITRATIO );
			BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, hitRatio );
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tidoriasi( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_CALC_AGILITY, handler_Tidoriasi },		// すばやさ計算ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「はりきり」
 */
//------------------------------------------------------------------------------
// 命中率計算ハンドラ
static void handler_Harikiri_HitRatio( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
	{
		// 物理ワザは命中率減少
		WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
		if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
		{
			u32 hit_ratio = BTL_EVENTVAR_GetValue( BTL_EVAR_RATIO );
			hit_ratio = BTL_CALC_MulRatio( hit_ratio, BTL_CALC_TOK_HARIKIRI_HITRATIO );
			BTL_EVENTVAR_SetValue( BTL_EVAR_RATIO, hit_ratio );
		}
	}
}
// 攻撃力計算ハンドラ
static void handler_Harikiri_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
	{
		// 物理ワザは威力増加
		WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
		if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
		{
			u32 power = BTL_EVENTVAR_GetValue( BTL_EVAR_POWER );
			power = BTL_CALC_MulRatio( power, BTL_CALC_TOK_HARIKIRI_POWRATIO );
			BTL_EVENTVAR_SetValue( BTL_EVAR_POWER, power );
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Harikiri( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_WAZA_HIT_RATIO,	handler_Harikiri_HitRatio },	// 命中率計算ハンドラ
		{ BTL_EVENT_ATTACKER_POWER,	handler_Harikiri_AtkPower },	// 攻撃力計算ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「いろめがね」
 */
//------------------------------------------------------------------------------
// ダメージ計算最終段階のハンドラ
static void handler_Iromegane( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
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
		{ BTL_EVENT_WAZA_DMG_PROC2, handler_Iromegane },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「ハードロック」＆「フィルター」
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
		{ BTL_EVENT_TURNCHECK_END, handler_Kasoku },
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「てきおうりょく」
 */
//------------------------------------------------------------------------------
// タイプ一致補正率計算ハンドラ
static void handler_Tekiouryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		if( BTL_EVENTVAR_GetValue(BTL_EVAR_GEN_FLAG) )
		{
			BTL_EVENTVAR_SetValue(BTL_EVAR_RATIO, FX32_CONST(2));
			BTL_Printf("ポケ[%d]の てきおうりょく でタイプ一致補正率２倍に\n", pokeID);
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tekiouryoku( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_TYPEMATCH_RATIO, handler_Tekiouryoku },	// タイプ一致補正率計算ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
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
	// 防御側が自分で
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
		{ BTL_EVENT_MAKE_SICK, handler_ReafGuard },	// 状態異常くらう前のハンドラ
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
		{ BTL_EVENT_MAKE_SICK, handler_Donkan },	// 状態異常くらう前のハンドラ
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
//------------------------------------------------------------------------------
/**
 *	とくせい「アイスボディ」
 */
//------------------------------------------------------------------------------
// 天候ダメージ計算ハンドラ
static void handler_IceBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_weather_recover( flowWk, pokeID, BTL_WEATHER_SNOW );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_IcoBody( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_CALC_WEATHER_DAMAGE, handler_IceBody },	// 天候ダメージ計算ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *	とくせい「あめうけざら」
 */
//------------------------------------------------------------------------------
// 天候ダメージ計算ハンドラ
static void handler_AmeukeZara( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	common_weather_recover( flowWk, pokeID, BTL_WEATHER_RAIN );
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_AmeukeZara( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_CALC_WEATHER_DAMAGE, handler_AmeukeZara },	// 天候ダメージ計算ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

/**
 *	天候に応じてHP回復するとくせいの共通処理
 */
static void common_weather_recover( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		if( BTL_EVENTVAR_GetValue(BTL_EVAR_WEATHER) == weather )
		{
			const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
			int recoverHP = BTL_POKEPARAM_GetValue( bpp, BPP_MAX_HP ) / 8;
			if( recoverHP == 0 ){ recoverHP = 1; }
			recoverHP *= -1;
			BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, recoverHP );
		}
		BTL_SVFLOW_RECEPT_ChangeWeather( flowWk, weather );
	}
}

//------------------------------------------------------------------------------
/**
 *	とくせい「サンパワー」
 */
//------------------------------------------------------------------------------
// 天候ダメージ計算ハンドラ
static void handler_SunPower_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
	{
		// 晴れの時、MAXHPの 1/8 減る
		if( BTL_EVENTVAR_GetValue(BTL_EVAR_WEATHER) == BTL_WEATHER_SHINE )
		{
			const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
			int damage = BTL_POKEPARAM_GetValue( bpp, BPP_MAX_HP ) / 8;
			if( damage == 0 ){ damage = 1; }
			BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, damage );
		}
	}
}
// 攻撃力決定ハンドラ
static void handler_SunPower_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	// 攻撃側が自分で
	if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
	{
		// 天候が晴れで
		if( BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE )
		{
			WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
			// ダメージタイプが特殊の時
			if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_SPECIAL )
			{
				// 威力1.5倍
				u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_POWER );
				pow = (pow * FX32_CONST(1.5f)) >> FX32_SHIFT;
				BTL_EVENTVAR_SetValue( BTL_EVAR_POWER, pow );
				BTL_Printf("ポケ[%d]の サンパワー で威力1.5倍\n", pokeID);
			}
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_SunPower( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_CALC_WEATHER_DAMAGE, handler_SunPower_Weather },// 天候ダメージ計算ハンドラ
		{ BTL_EVENT_ATTACKER_POWER,	 handler_SunPower_AtkPower },		// 攻撃力決定ハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「りんぷん」
 */
//------------------------------------------------------------------------------
// 追加効果（ひるみ、状態異常，ランク効果共通）ハンドラ
static void handler_Rinpun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) )
	{
		BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, 0 );
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Rinpun( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_ADD_SICK,				handler_Rinpun },	// 追加効果（状態異常）チェックハンドラ
		{ BTL_EVENT_ADD_RANK_TARGET,handler_Rinpun },	// 追加効果（ランク効果）チェックハンドラ
		{ BTL_EVENT_SHRINK_CHECK,		handler_Rinpun },	// ひるみチェックハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「てんのめぐみ」
 */
//------------------------------------------------------------------------------
// 追加効果（ひるみ、状態異常，ランク効果共通）ハンドラ
static void handler_TennoMegumi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) )
	{
		u16 per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER ) * 2;
		BTL_EVENTVAR_SetValue( BTL_EVAR_ADD_PER, per );
		BTL_Printf("ポケ[%d]の てんのめぐみ で追加発生率２倍=%d%%\n", pokeID, per);
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_TennoMegumi( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_ADD_SICK,				handler_TennoMegumi },	// 追加効果（状態異常）チェックハンドラ
		{ BTL_EVENT_ADD_RANK_TARGET,handler_TennoMegumi },	// 追加効果（ランク効果）チェックハンドラ
		{ BTL_EVENT_SHRINK_CHECK,		handler_TennoMegumi },	// ひるみチェックハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}


//------------------------------------------------------------------------------
/**
 *	とくせい「うるおいボディ」
 */
//------------------------------------------------------------------------------
// 状態異常ダメージハンドラ
static void handler_UruoiBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) )
	{
		if( BTL_FIELD_GetWeather() == BTL_WEATHER_RAIN )
		{
			WazaSick sick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
			if( sick < POKESICK_MAX )
			{
				BTL_EVENTVAR_SetValue( BTL_EVAR_SICKID, POKESICK_NULL );
				BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
				BTL_SVFLOW_RECEPT_CurePokeSick( flowWk, pokeID );
				BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
			}
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_UruoiBody( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_SICK_DAMAGE,			handler_UruoiBody },	// 状態異常ダメージハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「だっぴ」
 */
//------------------------------------------------------------------------------
// ターンチェック開始ハンドラ
static void handler_Dappi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
	if( BTL_POKEPARAM_GetPokeSick(bpp) != POKESICK_NULL )
	{
		if( GFL_STD_MtRand(100) < 33 )
		{
			BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
			BTL_SVFLOW_RECEPT_CurePokeSick( flowWk, pokeID );
			BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_Dappi( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_TURNCHECK_BEGIN,			handler_Dappi },	// ターンチェック開始
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *	とくせい「ポイズンヒール」
 */
//------------------------------------------------------------------------------
// 状態異常ダメージハンドラ
static void handler_PoisonHeal( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
	if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) )
	{
		WazaSick sick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
		if( (sick == WAZASICK_DOKU) || (sick == WAZASICK_MOUDOKU) )
		{
			const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
			int recoverHP = BTL_POKEPARAM_GetValue( bpp, BPP_MAX_HP ) / 8;
			if( recoverHP == 0 )
			{
				recoverHP = 1;
			}
			BTL_EVENTVAR_SetValue( BTL_EVAR_DAMAGE, 0 );
			BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
			BTL_SERVER_RECEPT_HP_Add( flowWk, pokeID, recoverHP );
			BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
		}
	}
}
BTL_EVENT_FACTOR*  HAND_TOK_ADD_PoisonHeal( u16 pri, u8 pokeID )
{
	static const BtlEventHandlerTable HandlerTable[] = {
		{ BTL_EVENT_SICK_DAMAGE,			handler_PoisonHeal },	// 状態異常ダメージハンドラ
		{ BTL_EVENT_NULL, NULL },
	};
	return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, pri, pokeID, HandlerTable );
}


