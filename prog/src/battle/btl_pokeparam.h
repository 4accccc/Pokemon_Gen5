//=============================================================================================
/**
 * @file	btl_pokeparam.h
 * @brief	ポケモンWB バトルシステム  参加ポケモン戦闘用データ
 * @author	taya
 *
 * @date	2008.10.08	作成
 */
//=============================================================================================
#ifndef __BTL_POKEPARAM_H__
#define __BTL_POKEPARAM_H__

#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_personal.h"
#include "waza_tool/wazadata.h"

#include "btl_util.h"

//--------------------------------------------------------------
/**
 *	ハンドラ型定義
 */
//--------------------------------------------------------------
typedef struct _BTL_POKEPARAM	BTL_POKEPARAM;



//--------------------------------------------------------------
/**
 *	取得できる値
 */
//--------------------------------------------------------------
typedef enum {

	BPP_ATTACK,
	BPP_DEFENCE,
	BPP_SP_ATTACK,
	BPP_SP_DEFENCE,
	BPP_AGILITY,
	BPP_HIT_RATIO,
	BPP_AVOID_RATIO,

	//--- ここまでランクアップ／ダウン効果の引数としても用いる ---
	BPP_CRITICAL_RATIO,

	BPP_HP,
	BPP_MAX_HP,
	BPP_LEVEL,
	BPP_TOKUSEI,

	BPP_RANKTYPE_MAX = BPP_AVOID_RATIO+1,

}BppValueID;


extern BTL_POKEPARAM*  BTL_POKEPARAM_Create( const POKEMON_PARAM* pp, u8 id, HEAPID heapID );
extern void BTL_POKEPARAM_Delete( BTL_POKEPARAM* bpp );

extern u8 BTL_POKEPARAM_GetID( const BTL_POKEPARAM* pp );
extern const POKEMON_PARAM* BTL_POKEPARAM_GetSrcData( const BTL_POKEPARAM* bpp );

extern u16 BTL_POKEPARAM_GetMonsNo( const BTL_POKEPARAM* pp );
extern u8 BTL_POKEPARAM_GetWazaCount( const BTL_POKEPARAM* pp );
extern WazaID BTL_POKEPARAM_GetWazaNumber( const BTL_POKEPARAM* pp, u8 idx );
extern WazaID BTL_POKEPARAM_GetWazaParticular( const BTL_POKEPARAM* pp, u8 idx, u8* PP, u8* PPMax );

extern PokeTypePair BTL_POKEPARAM_GetPokeType( const BTL_POKEPARAM* pp );
extern BOOL BTL_POKEPARAM_IsMatchType( const BTL_POKEPARAM* pp, PokeType type );

extern int BTL_POKEPARAM_GetValue( const BTL_POKEPARAM* pp, BppValueID vid );
extern int BTL_POKEPARAM_GetValueCriticalHit( const BTL_POKEPARAM* pp, BppValueID vid );

extern BOOL BTL_POKEPARAM_IsDead( const BTL_POKEPARAM* pp );
extern BOOL BTL_POKEPARAM_CheckSick( const BTL_POKEPARAM* pp, WazaSick sickType );

extern PokeSick BTL_POKEPARAM_GetPokeSick( const BTL_POKEPARAM* pp );
extern int BTL_POKEPARAM_CalcSickDamage( const BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_SetPokeSick( BTL_POKEPARAM* pp, PokeSick sick, u8 turn );
extern BOOL BTL_POKEPARAM_Nemuri_CheckWake( BTL_POKEPARAM* pp );
extern void BTL_POKEPARAM_WazaSick_TurnCheck( BTL_POKEPARAM* pp );


//-------------------------
extern BOOL BTL_POKEPARAM_RankUp( BTL_POKEPARAM* pp, BppValueID rankType, u8 volume );
extern BOOL BTL_POKEPARAM_RankDown( BTL_POKEPARAM* pp, BppValueID rankType, u8 volume );
extern void BTL_POKEPARAM_HpMinus( BTL_POKEPARAM* pp, u16 value );
extern void BTL_POKEPARAM_HpPlus( BTL_POKEPARAM* pp, u16 value );
extern void BTL_POKEPARAM_PPMinus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value );
extern void BTL_POKEPARAM_PPPlus( BTL_POKEPARAM* pp, u8 wazaIdx, u8 value );

#endif
