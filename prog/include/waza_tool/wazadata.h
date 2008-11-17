//=============================================================================================
/**
 * @file	wazadata.h
 * @brief	ポケモンWB  ワザデータアクセサ
 * @author	taya
 *
 * @date	2008.10.14	作成
 */
//=============================================================================================
#ifndef __WAZADATA_H__
#define __WAZADATA_H__

#include "poke_tool/poketype.h"


typedef  unsigned short  WazaID;


//------------------------------------------------------------------------------
/**
 *	ワザのカテゴリ最上位分類
 */
//------------------------------------------------------------------------------
typedef enum {

	WAZADATA_CATEGORY_SIMPLE_DAMAGE,
	WAZADATA_CATEGORY_SIMPLE_SICK,
	WAZADATA_CATEGORY_SIMPLE_EFFECT,
	WAZADATA_CATEGORY_SIMPLE_RECOVER,
	WAZADATA_CATEGORY_DAMAGE_SICK,
	WAZADATA_CATEGORY_DAMAGE_EFFECT,
	WAZADATA_CATEGORY_DAMAGE_EFFECT_USER,
	WAZADATA_CATEGORY_DAMAGE_RECOVER,
	WAZADATA_CATEGORY_ICHIGEKI,
	WAZADATA_CATEGORY_BIND,
	WAZADATA_CATEGORY_GUARD,
	WAZADATA_CATEGORY_WEATHER,
	WAZADATA_CATEGORY_OTHERS,

}WazaCategory;


typedef enum {

	WAZADATA_DMG_NONE,
	WAZADATA_DMG_PHYSIC,	///< 物理ダメージ
	WAZADATA_DMG_SPECIAL,	///< 特殊ダメージ

}WazaDamageType;

//------------------------------------------------------------------------------
/**
 *	ワザによって引き起こされる状態異常。
 *	PokeSickをバトル用に拡張したもの。
 */
//------------------------------------------------------------------------------
typedef enum {

	WAZASICK_ORIGIN = POKESICK_ORIGIN,

	WAZASICK_DOKU = POKESICK_DOKU,
	WAZASICK_MAHI = POKESICK_MAHI,
	WAZASICK_NEMURI = POKESICK_NEMURI,
	WAZASICK_KOORI = POKESICK_KOORI,
	WAZASICK_YAKEDO = POKESICK_YAKEDO,

	WAZASICK_DOKUDOKU = POKESICK_MAX,
	WAZASICK_KONRAN,
	WAZASICK_MEROMERO,
	WAZASICK_AKUMU,
	WAZASICK_TYOUHATSU,
	WAZASICK_SASIOSAE,
	WAZASICK_KAIHUKUHUUJI,
	WAZASICK_KANASIBARI,

	WAZASICK_MAX,
	WAZASICK_NULL = WAZASICK_MAX,

}WazaSick;


//------------------------------------------------------------------------------
/**
 *	ワザによって発生する追加効果（攻撃等のランクアップ・ダウン効果）
 */
//------------------------------------------------------------------------------
typedef enum {

	WAZA_INFLU_NULL = 0,
	WAZA_INFLU_ORIGIN = 1,

	WAZA_INFLU_ATTACK = WAZA_INFLU_ORIGIN,
	WAZA_INFLU_DEFENCE,
	WAZA_INFLU_SP_ATTACK,
	WAZA_INFLU_SP_DEFENCE,
	WAZA_INFLU_AGILITY,
	WAZA_INFLU_HIT,
	WAZA_INFLU_AVOID,

	WAZA_INFLU_MAX,

}WazaInflu;


typedef enum {

	WAZA_TARGET_SINGLE,			///< 自分以外の１体選択
	WAZA_TARGET_ENEMY2,			///< 敵側２体
	WAZA_TARGET_WITHOUT_USER,	///< 自分以外全部

	WAZA_TARGET_ONLY_USER,		///< 自分１体
	WAZA_TARGET_TEAM_USER,		///< 自分側チーム
	WAZA_TARGET_TEAM_ENEMY,		///< 敵側チーム


//	WAZA_TARGET_DOUBLE,		///< 


}WazaTarget;


//------------------------------------------------------------------------------
/**
 *	ワザ優先度
 */
//------------------------------------------------------------------------------
typedef enum {

	WAZAPRI_MAX = 5,
	WAZAPRI_MIN = -7,

	WAZAPRI_RANGE = (WAZAPRI_MAX - WAZAPRI_MIN + 1),

}WazaPriority;



extern u16  WAZADATA_GetPower( WazaID id );
extern s8   WAZADATA_GetPriority( WazaID id );



// カテゴリ
extern WazaCategory  WAZADATA_GetCategory( WazaID id );

// タイプ
extern PokeType WAZADATA_GetType( WazaID id );

// 命中率
extern u8  WAZADATA_GetHitRatio( WazaID id );

// 複数回連続攻撃ワザの最大回数。
extern u16  WAZADATA_GetContiniusCount( WazaID id );

// きゅうしょに当たるランク
extern u16  WAZADATA_GetCriticalRatio( WazaID id );

// 通常攻撃ワザであり、追加効果を含む場合は追加効果値が返る。
// 状態異常のみを起こすワザ（例えば催眠術など）は、WAZASICK_NULL が返る。
extern WazaSick  WAZADATA_GetAdditionalSick( WazaID id );

// 同上。こちらは追加効果の確率が返る。
extern u8  WAZADATA_GetAdditionalSickRatio( WazaID id );

// 必中フラグ。つばめがえしなどがTRUEを返す。
extern BOOL WAZADATA_IsAleadyHit( WazaID id );

// ダメージワザかどうか
extern BOOL WAZADATA_IsDamage( WazaID id );

// ダメージタイプ
extern WazaDamageType  WAZADATA_GetDamageType( WazaID id );

// クリティカルランク
extern u8 WAZADATA_GetCriticalRank( WazaID id );

// 最大ヒット回数
extern u8 WAZADATA_GetMaxHitCount( WazaID id );


/*-----------------------------------------------------------------


使用できるか？
必ず失敗するか？
威力は？
ダメージは？
当たったか？
回復は？
追加効果は？


-----------------------------------------------------------------*/

#endif
