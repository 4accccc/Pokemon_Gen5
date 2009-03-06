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
	WAZADATA_CATEGORY_ICHIGEKI,
	WAZADATA_CATEGORY_BIND,
	WAZADATA_CATEGORY_GUARD,
	WAZADATA_CATEGORY_FIELD_EFFECT,
	WAZADATA_CATEGORY_SIDE_EFFECT,
	WAZADATA_CATEGORY_OTHERS,

	WAZADATA_CATEGORY_MAX,

}WazaCategory;


typedef enum {

	WAZADATA_DMG_PHYSIC,	///< 物理ダメージ
	WAZADATA_DMG_SPECIAL,	///< 特殊ダメージ
	WAZADATA_DMG_NONE,

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

	WAZASICK_MOUDOKU = POKESICK_MAX,
	WAZASICK_KONRAN,
	WAZASICK_MEROMERO,
	WAZASICK_AKUMU,
	WAZASICK_TYOUHATSU,
	WAZASICK_SASIOSAE,
	WAZASICK_KAIHUKUHUUJI,
	WAZASICK_KANASIBARI,

	WAZASICK_MAX,
	WAZASICK_NULL = POKESICK_NULL,

}WazaSick;


//------------------------------------------------------------------------------
/**
 *	ワザによって発生する追加効果（攻撃等のランクアップ・ダウン効果）
 */
//------------------------------------------------------------------------------
typedef enum {

	WAZA_RANKEFF_NULL = 0,
	WAZA_RANKEFF_ORIGIN = 1,

	WAZA_RANKEFF_ATTACK = WAZA_RANKEFF_ORIGIN,
	WAZA_RANKEFF_DEFENCE,
	WAZA_RANKEFF_SP_ATTACK,
	WAZA_RANKEFF_SP_DEFENCE,
	WAZA_RANKEFF_AGILITY,
	WAZA_RANKEFF_HIT,
	WAZA_RANKEFF_AVOID,

	WAZA_RANKEFF_MAX,

}WazaRankEffect;


typedef enum {

	WAZA_TARGET_SINGLE,				///< 自分以外の１体（選択）
	WAZA_TARGET_ENEMY2,				///< 敵側２体
	WAZA_TARGET_OTHER_ALL,		///< 自分以外全部

	WAZA_TARGET_ONLY_USER,		///< 自分１体のみ
	WAZA_TARGET_SINGLE_FRIEND,///< 自分を含む味方１体（選択）
	WAZA_TARGET_OTHER_FRIEND,	///< 自分以外の味方１体
	WAZA_TARGET_SINGLE_ENEMY,	///< 敵１体（選択）
	WAZA_TARGET_RANDOM,				///< 敵ランダム

	WAZA_TARGET_TEAM_USER,		///< 自分側陣地
	WAZA_TARGET_TEAM_ENEMY,		///< 敵側陣地
	WAZA_TARGET_FIELD,				///< 場に効く（天候系など）

	WAZA_TARGET_UNKNOWN,			///< ゆびをふるなど、相手のワザに応じるタイプ

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

// 必中フラグ。つばめがえしなどがTRUEを返す。
extern BOOL WAZADATA_IsAlwaysHit( WazaID id );

// ダメージワザかどうか
extern BOOL WAZADATA_IsDamage( WazaID id );

// ダメージタイプ
extern WazaDamageType  WAZADATA_GetDamageType( WazaID id );

// クリティカルランク
extern u8 WAZADATA_GetCriticalRank( WazaID id );

// 最大ヒット回数
extern u8 WAZADATA_GetMaxHitCount( WazaID id );

// 効果範囲
extern WazaTarget WAZADATA_GetTarget( WazaID id );

// 状態異常効果を取得
extern PokeSick WAZADATA_GetSick( WazaID id );

// 同上。こちらは追加効果の確率が返る。
extern u32 WAZADATA_GetSickPer( WazaID id );


/*-----------------------------------------------------------------


使用できるか？
必ず失敗するか？
威力は？
ダメージは？
当たったか？
回復は？
追加効果は？


-----------------------------------------------------------------*/

#ifdef PM_DEBUG
extern void WAZADATA_PrintDebug( void );
#else
#define WAZADATA_PrintDebug()	/* */
#endif


#endif
