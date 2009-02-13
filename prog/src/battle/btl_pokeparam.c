//=============================================================================================
/**
 * @file	btl_pokeparam.c
 * @brief	ポケモンWB バトルシステム  参加ポケモン戦闘用データ
 * @author	taya
 *
 * @date	2008.10.08	作成
 */
//=============================================================================================
#include <gflib.h>
#include "poke_tool/poke_tool.h"

#include "btl_common.h"
#include "btl_calc.h"
#include "btl_pokeparam.h"


/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
	RANK_STATUS_MIN = 0,
	RANK_STATUS_MAX = 13,
	RANK_STATUS_DEF = 6,

	RANK_CRITICAL_MIN = 0,
	RANK_CRITICAL_MAX = 4,
	RANK_CRITICAL_DEF = 0,
};



//--------------------------------------------------------------
/**
 *	基本パラメータ  --バトル中、不変な要素--
 */
//--------------------------------------------------------------
typedef struct {

	u16	monsno;			///< ポケモンナンバー

	u8	hpMax;			///< 最大HP
	u8	attack;			///< こうげき
	u8	defence;		///< ぼうぎょ
	u8	sp_attack;		///< とくこう
	u8	sp_defence;		///< とくぼう
	u8	agility;		///< すばやさ

	u8  level;			///< レベル
	u8  type1;
	u8  type2;

}BPP_BASE_PARAM;

//--------------------------------------------------------------
/**
 *	ランク効果
 */
//--------------------------------------------------------------
typedef struct {

	s8	attack;		///< こうげき
	s8	defence;	///< ぼうぎょ
	s8	sp_attack;	///< とくこう
	s8	sp_defence;	///< とくぼう
	s8	agility;	///< すばやさ
	s8	hit;		///< 命中率
	s8	avoid;		///< 回避率
	s8	critical;	///< クリティカル率

}BPP_VARIABLE_PARAM;

//--------------------------------------------------------------
/**
 *	実際に計算に使われるパラメータ値（初期値×ランク効果）
 */
//--------------------------------------------------------------
typedef struct {

	u16	attack;			///< こうげき
	u16	defence;		///< ぼうぎょ
	u16	sp_attack;		///< とくこう
	u16	sp_defence;		///< とくぼう
	u16	agility;		///< すばやさ
	u16	dmy;

}BPP_REAL_PARAM;

//--------------------------------------------------------------
/**
 *	ワザ
 */
//--------------------------------------------------------------
typedef struct {
	u16  number;
	u8   pp;
	u8   ppMax;
}BPP_WAZA;


struct _BTL_POKEPARAM {

	BPP_BASE_PARAM		baseParam;
	BPP_VARIABLE_PARAM	varyParam;
	BPP_REAL_PARAM		realParam;
	BPP_WAZA			waza[ PTL_WAZA_MAX ];

	u16  item;
	u16  tokusei;
	u16  hp;

	u8	wazaCnt;
	u8	myID;

	const POKEMON_PARAM*	ppSrc;
};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void Effrank_Init( BPP_VARIABLE_PARAM* rank );
static void update_RealParam( BTL_POKEPARAM* pp, BppValueID rankType );



//=============================================================================================
/**
 * バトル用ポケモンパラメータ生成
 *
 * @param   pp			ゲーム内汎用ポケモンパラメータ
 * @param   pokeID		バトル参加ポケID（バトルに参加しているポケモンに割り振る一意の数値）
 * @param   heapID		
 *
 * @retval  BTL_POKEPARAM*		
 */
//=============================================================================================
BTL_POKEPARAM*  BTL_POKEPARAM_Create( const POKEMON_PARAM* pp, u8 pokeID, HEAPID heapID )
{
	BTL_POKEPARAM* bpp = GFL_HEAP_AllocMemory( heapID, sizeof(BTL_POKEPARAM) );
	int i;
	u16 monsno;

	// 基本パラメタ初期化

	monsno = PP_Get( pp, ID_PARA_monsno, 0 );

	bpp->baseParam.monsno = monsno;
	bpp->baseParam.level = PP_Get( pp, ID_PARA_level, 0 );
	bpp->baseParam.hpMax = PP_Get( pp, ID_PARA_hpmax, 0 );
	bpp->baseParam.attack = PP_Get( pp, ID_PARA_pow, 0 );
	bpp->baseParam.defence = PP_Get( pp, ID_PARA_def, 0 );
	bpp->baseParam.sp_attack = PP_Get( pp, ID_PARA_spepow, 0 );
	bpp->baseParam.sp_defence = PP_Get( pp, ID_PARA_spedef, 0 );
	bpp->baseParam.agility = PP_Get( pp, ID_PARA_agi, 0 );
	bpp->baseParam.type1 = PP_Get( pp, ID_PARA_type1, 0 );
	bpp->baseParam.type2 = PP_Get( pp, ID_PARA_type2, 0 );

	// ランク効果初期化
	Effrank_Init( &bpp->varyParam );

	// 実パラメータ初期化
	bpp->realParam.attack = bpp->baseParam.attack;
	bpp->realParam.defence = bpp->baseParam.defence;
	bpp->realParam.sp_attack = bpp->baseParam.sp_attack;
	bpp->realParam.sp_defence = bpp->baseParam.sp_defence;
	bpp->realParam.agility = bpp->baseParam.agility;

	// 所有ワザデータ初期化
	bpp->wazaCnt = 0;
	for(i=0; i<PTL_WAZA_MAX; i++)
	{
		bpp->waza[i].number = PP_Get( pp, ID_PARA_waza1+i, 0 );
		if( bpp->waza[i].number )
		{
			bpp->waza[i].pp = PP_Get( pp, ID_PARA_pp1+i, 0 );
			bpp->waza[i].ppMax = PP_Get( pp, ID_PARA_pp_max1+i, 0 );
			bpp->wazaCnt++;
		}
		else
		{
			bpp->waza[i].pp = 0;
			bpp->waza[i].ppMax = 0;
		}
	}


	bpp->item = 0;
	bpp->tokusei = PP_Get( pp, ID_PARA_speabino, 0 );
	bpp->hp = PP_Get( pp, ID_PARA_hp, 0 );
	bpp->myID = pokeID;

	bpp->ppSrc = pp;

	return bpp;
}

//=============================================================================================
/**
 * バトル用ポケモンパラメータ削除
 *
 * @param   bpp		
 *
 */
//=============================================================================================
void BTL_POKEPARAM_Delete( BTL_POKEPARAM* bpp )
{
	GFL_HEAP_FreeMemory( bpp );
}


//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

static void Effrank_Init( BPP_VARIABLE_PARAM* rank )
{
	rank->attack = RANK_STATUS_DEF;
	rank->defence = RANK_STATUS_DEF;
	rank->sp_attack = RANK_STATUS_DEF;
	rank->sp_defence = RANK_STATUS_DEF;
	rank->agility = RANK_STATUS_DEF;
	rank->hit = RANK_STATUS_DEF;
	rank->avoid = RANK_STATUS_DEF;
	rank->critical = RANK_CRITICAL_DEF;
}




//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

u8 BTL_POKEPARAM_GetID( const BTL_POKEPARAM* pp )
{
	return pp->myID;
}

u16 BTL_POKEPARAM_GetMonsNo( const BTL_POKEPARAM* pp )
{
	return pp->baseParam.monsno;
}

u8 BTL_POKEPARAM_GetWazaCount( const BTL_POKEPARAM* pp )
{
	return pp->wazaCnt;
}

WazaID BTL_POKEPARAM_GetWazaNumber( const BTL_POKEPARAM* pp, u8 idx )
{
	GF_ASSERT(idx < pp->wazaCnt);
	return pp->waza[idx].number;
}

WazaID BTL_POKEPARAM_GetWazaParticular( const BTL_POKEPARAM* pp, u8 idx, u8* PP, u8* PPMax )
{
	GF_ASSERT(idx < pp->wazaCnt);
	*PP = pp->waza[idx].pp;
	*PPMax = pp->waza[idx].ppMax;

	return pp->waza[idx].number;
}


PokeTypePair BTL_POKEPARAM_GetPokeType( const BTL_POKEPARAM* pp )
{
	return PokeTypePair_Make( pp->baseParam.type1, pp->baseParam.type2 );
}

BOOL BTL_POKEPARAM_IsMatchType( const BTL_POKEPARAM* pp, PokeType type )
{
	PokeTypePair pair = PokeTypePair_Make( pp->baseParam.type1, pp->baseParam.type2 );
	return PokeTypePair_IsMatch( pair, type );
}

const POKEMON_PARAM* BTL_POKEPARAM_GetSrcData( const BTL_POKEPARAM* bpp )
{
	return bpp->ppSrc;
}


//=============================================================================================
/**
 * 各種パラメータ取得
 *
 * @param   pp		
 * @param   vid		
 *
 * @retval  int		
 */
//=============================================================================================
int BTL_POKEPARAM_GetValue( const BTL_POKEPARAM* pp, BppValueID vid )
{
	switch( vid ){
	case BPP_ATTACK:		return pp->realParam.attack;
	case BPP_DEFENCE:		return pp->realParam.defence;
	case BPP_SP_ATTACK:		return pp->realParam.sp_attack;
	case BPP_SP_DEFENCE:	return pp->realParam.sp_defence;
	case BPP_AGILITY:		return pp->realParam.agility;

	case BPP_HIT_RATIO:			return pp->varyParam.hit;
	case BPP_AVOID_RATIO:		return pp->varyParam.avoid;
	case BPP_CRITICAL_RATIO:	return pp->varyParam.critical;

	case BPP_LEVEL:			return pp->baseParam.level;
	case BPP_HP:			return pp->hp;
	case BPP_MAX_HP:		return pp->baseParam.hpMax;

	case BPP_TOKUSEI:		return pp->tokusei;

	default:
		GF_ASSERT(0);
		return 0;
	};
}

//=============================================================================================
/**
 * クリティカルヒット時のパラメータ取得
 *
 * @param   pp		
 * @param   vid		
 *
 * @retval  int
 */
//=============================================================================================
int BTL_POKEPARAM_GetValueCriticalHit( const BTL_POKEPARAM* pp, BppValueID vid )
{
	switch( vid ){
	case BPP_ATTACK:	return (pp->varyParam.attack < 0)? pp->baseParam.attack : pp->realParam.attack;
	case BPP_SP_ATTACK:	return (pp->varyParam.sp_attack < 0)? pp->baseParam.sp_attack : pp->realParam.sp_attack;
	case BPP_DEFENCE:	return (pp->varyParam.defence > 0)? pp->baseParam.defence : pp->realParam.defence;
	case BPP_SP_DEFENCE:return (pp->varyParam.sp_defence > 0)? pp->baseParam.sp_defence : pp->realParam.sp_defence;

	default:
		return BTL_POKEPARAM_GetValue( pp, vid );
	}
}
//=============================================================================================
/**
 * ひんし状態かチェック
 *
 * @param   pp				
 *
 * @retval  BOOL			
 */
//=============================================================================================
BOOL BTL_POKEPARAM_IsDead( const BTL_POKEPARAM* pp )
{
	return BTL_POKEPARAM_GetValue( pp, BPP_HP ) == 0;
}
//=============================================================================================
/**
 * 特定の状態異常にかかっているかチェック
 *
 * @param   pp				
 * @param   sickType		
 *
 * @retval  BOOL			
 */
//=============================================================================================
BOOL BTL_POKEPARAM_CheckSick( const BTL_POKEPARAM* pp, WazaSick sickType )
{
	return FALSE;
}



//-----------------------------

//=============================================================================================
/**
 * ランクアップ効果
 *
 * @param   pp			
 * @param   rankType	
 * @param   volume		
 *
 * @retval  BOOL		ランクが上がった場合TRUE／もう上がらない場合FALSE
 */
//=============================================================================================
BOOL BTL_POKEPARAM_RankUp( BTL_POKEPARAM* pp, BppValueID rankType, u8 volume )
{
	s8 *ptr;
	s8 max = RANK_STATUS_MAX;

	switch( rankType ){
	case BPP_ATTACK:		ptr = &pp->varyParam.attack; break;
	case BPP_DEFENCE:		ptr = &pp->varyParam.defence; break;
	case BPP_SP_ATTACK:		ptr = &pp->varyParam.sp_attack; break;
	case BPP_SP_DEFENCE:	ptr = &pp->varyParam.sp_defence; break;
	case BPP_AGILITY:		ptr = &pp->varyParam.agility; break;
	case BPP_HIT_RATIO:		ptr = &pp->varyParam.hit; break;
	case BPP_AVOID_RATIO:	ptr = &pp->varyParam.avoid; break;
	case BPP_CRITICAL_RATIO:
		ptr = &pp->varyParam.critical;
		max = RANK_CRITICAL_MAX;
		break;
	default:
		GF_ASSERT(0);
		return FALSE;
	}

	if( *ptr < max )
	{
		*ptr += volume;
		if( *ptr > max )
		{
			*ptr = max;
		}
		update_RealParam( pp, rankType );
		return TRUE;
	}

	return FALSE;
}

//=============================================================================================
/**
 * ランクダウン効果
 *
 * @param   pp			
 * @param   rankType	
 * @param   volume		
 *
 * @retval  BOOL		ランクが下がった場合TRUE／もう下がらない場合FALSE
 */
//=============================================================================================
BOOL BTL_POKEPARAM_RankDown( BTL_POKEPARAM* pp, BppValueID rankType, u8 volume )
{
	s8 *ptr;
	s8 min = RANK_STATUS_MIN;

	switch( rankType ){
	case BPP_ATTACK:		ptr = &pp->varyParam.attack; break;
	case BPP_DEFENCE:		ptr = &pp->varyParam.defence; break;
	case BPP_SP_ATTACK:		ptr = &pp->varyParam.sp_attack; break;
	case BPP_SP_DEFENCE:	ptr = &pp->varyParam.sp_defence; break;
	case BPP_AGILITY:		ptr = &pp->varyParam.agility; break;
	case BPP_HIT_RATIO:		ptr = &pp->varyParam.hit; break;
	case BPP_AVOID_RATIO:	ptr = &pp->varyParam.avoid; break;
	case BPP_CRITICAL_RATIO:
		ptr = &pp->varyParam.critical;
		min = RANK_CRITICAL_MIN;
		break;
	default:
		GF_ASSERT(0);
		return FALSE;
	}

	if( *ptr > min )
	{
		*ptr -= volume;
		if( *ptr < min )
		{
			*ptr = min;
		}
		update_RealParam( pp, rankType );
		return TRUE;
	}

	return FALSE;
}

//=============================================================================================
/**
 * HP値を減少
 *
 * @param   pp		
 * @param   value		
 *
 */
//=============================================================================================
void BTL_POKEPARAM_HpMinus( BTL_POKEPARAM* pp, u16 value )
{
	if( pp->hp > value )
	{
		pp->hp -= value;
	}
	else
	{
		pp->hp = 0;
	}
}
//=============================================================================================
/**
 * HP値を増加
 *
 * @param   pp		
 * @param   value		
 *
 */
//=============================================================================================
void BTL_POKEPARAM_HpPlus( BTL_POKEPARAM* pp, u16 value )
{
	pp->hp += value;
	if( pp->hp > pp->baseParam.hpMax )
	{
		pp->hp = pp->baseParam.hpMax;
	}
}
//=============================================================================================
/**
 * ワザPP値を減少
 *
 * @param   pp		
 * @param   wazaIdx		
 * @param   value		
 *
 */
//=============================================================================================
void BTL_POKEPARAM_PPMinus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value )
{
	GF_ASSERT(wazaIdx < pp->wazaCnt);

	if( pp->waza[wazaIdx].pp >= value )
	{
		pp->waza[wazaIdx].pp -= value;
	}
	else
	{
		pp->waza[wazaIdx].pp = 0;
	}
}
//=============================================================================================
/**
 * ワザPP値を増加
 *
 * @param   pp		
 * @param   wazaIdx		
 * @param   value		
 *
 */
//=============================================================================================
void BTL_POKEPARAM_PPPlus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value )
{
	GF_ASSERT(wazaIdx < pp->wazaCnt);

	pp->waza[wazaIdx].pp += value;
	if( pp->waza[wazaIdx].pp > pp->waza[wazaIdx].ppMax )
	{
		pp->waza[wazaIdx].pp = pp->waza[wazaIdx].ppMax;
	}
}


//--------------------------------------------------------------------------
/**
 * ランク効果とデフォルト値をかけあわせて実際値を算出しておく
 *
 * @param   pp		
 * @param   rankType		
 *
 */
//--------------------------------------------------------------------------
static void update_RealParam( BTL_POKEPARAM* pp, BppValueID rankType )
{
	switch( rankType ){
	case BPP_ATTACK:
		pp->realParam.attack = BTL_CALC_StatusRank(pp->baseParam.attack, pp->varyParam.attack);
		break;
	case BPP_DEFENCE:
		pp->realParam.defence = BTL_CALC_StatusRank(pp->baseParam.defence, pp->varyParam.defence);
		break;
	case BPP_SP_ATTACK:
		pp->realParam.defence = BTL_CALC_StatusRank(pp->baseParam.defence, pp->varyParam.defence);
		break;
	case BPP_SP_DEFENCE:
		pp->realParam.defence = BTL_CALC_StatusRank(pp->baseParam.defence, pp->varyParam.defence);
		break;
	case BPP_AGILITY:
		pp->realParam.defence = BTL_CALC_StatusRank(pp->baseParam.defence, pp->varyParam.defence);
		break;
	default:
		return;
	}

}


