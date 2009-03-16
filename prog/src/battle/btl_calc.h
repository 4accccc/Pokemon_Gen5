
#ifndef __BTL_CALC_H__
#define __BTL_CALC_H__

#include "battle\battle.h"
#include "btl_util.h"
#include "btl_pokeparam.h"

enum {
	BTL_CALC_BASERANK_DEFAULT = 0,

	BTL_CALC_HITRATIO_MID = 6,
	BTL_CALC_HITRATIO_MIN = 0,
	BTL_CALC_HITRATIO_MAX = 12,

	BTL_CALC_CRITICAL_MIN = 0,
	BTL_CALC_CRITICAL_MAX = 4,

// ダメージ計算関連
	BTL_CALC_DMG_TARGET_RATIO_PLURAL = FX32_CONST(0.75f),			///< 複数体が対象ワザのダメージ補正率
	BTL_CALC_DMG_TARGET_RATIO_NONE   = FX32_CONST(1),					///< 対象によるダメージ補正なし
	BTL_CALC_DMG_WEATHER_RATIO_ADVANTAGE = FX32_CONST(1.5f),		///< ワザタイプ＆天候の組み合わせで有利な場合の補正率
	BTL_CALC_DMG_WEATHER_RATIO_DISADVANTAGE = FX32_CONST(0.5f),	///< ワザタイプ＆天候の組み合わせで不利な場合の補正率
	BTL_CALC_DMG_WEATHER_RATIO_NONE = FX32_CONST(1),						///< ワザタイプ＆天候の組み合わせによる補正なし


// 状態異常処理関連
	BTL_NEMURI_TURN_MIN = 3,	///< 「ねむり」最小ターン数
	BTL_NEMURI_TURN_MAX = 8,	///< 「ねむり」最大ターン数
	BTL_NEMURI_TURN_RANGE = (BTL_NEMURI_TURN_MAX - BTL_NEMURI_TURN_MIN + 1),

	BTL_MAHI_EXE_RATIO = 25,	///< 「まひ」でしびれて動けない確率
	BTL_KORI_MELT_RATIO = 20,	///< 「こおり」が溶ける確率

	BTL_DOKU_SPLIT_DENOM = 8,					///< 「どく」で最大HPの何分の１減るか
	BTL_YAKEDO_SPLIT_DENOM = 8,				///< 「やけど」で最大HPの何分の１減るか
	BTL_YAKEDO_DAMAGE_RATIO = 50,			///< 「やけど」で物理ダメージを減じる率
	BTL_MOUDOKU_SPLIT_DENOM = 16,			///< もうどく時、最大HPの何分の１減るか（基本値=最小値）
	BTL_MOUDOKU_COUNT_MAX = BTL_MOUDOKU_SPLIT_DENOM-1,	///< もうどく時、ダメージ倍率をいくつまでカウントアップするか

	BTL_CONF_EXE_RATIO = 30,		///< 混乱時の自爆確率
	BTL_CONF_TURN_MIN = 2,
	BTL_CONF_TURN_MAX = 5,
	BTL_CONF_TURN_RANGE = (BTL_CONF_TURN_MAX - BTL_CONF_TURN_MIN)+1,

	BTL_CALC_SICK_TURN_PERMANENT = 0xff,	///< 継続ターン数を設定しない（永続する）場合の指定値

	// 天候関連
	BTL_CALC_WEATHER_MIST_HITRATIO = FX32_CONST(75),		///< 「きり」の時の命中率補正率
	BTL_WEATHER_TURN_DEFAULT = 5,					///< ワザによる天候変化の継続ターン数
	BTL_WEATHER_TURN_PERMANENT = 0xff,

};

//--------------------------------------------------------------------
/**
 *	タイプ相性（ 0, 25, 50, 100, 200, 400% の６段階）
 */
//--------------------------------------------------------------------
typedef enum {

	BTL_TYPEAFF_0 = 0,	///< 無効
	BTL_TYPEAFF_25,
	BTL_TYPEAFF_50,
	BTL_TYPEAFF_100,
	BTL_TYPEAFF_200,
	BTL_TYPEAFF_400,

}BtlTypeAff;
//--------------------------------------------------------------------
/**
 *	簡易タイプ相性（こうかなし・ふつう・ばつぐん・もうひとつの４段階）
 */
//--------------------------------------------------------------------
typedef enum {

	BTL_TYPEAFF_ABOUT_NONE = 0,
	BTL_TYPEAFF_ABOUT_NORMAL,
	BTL_TYPEAFF_ABOUT_ADVANTAGE,
	BTL_TYPEAFF_ABOUT_DISADVANTAGE,

}BtlTypeAffAbout;

//--------------------------------------------------------------------
/**
 *	タイプ相性 -> 簡易タイプ相性変換
 */
//--------------------------------------------------------------------
static inline BtlTypeAffAbout BTL_CALC_TypeAffAbout( BtlTypeAff aff )
{
	if( aff > BTL_TYPEAFF_100 )
	{
		return BTL_TYPEAFF_ABOUT_ADVANTAGE;
	}
	if( aff == BTL_TYPEAFF_100 )
	{
		return BTL_TYPEAFF_ABOUT_NORMAL;
	}
	if( aff != BTL_TYPEAFF_0 )
	{
		return BTL_TYPEAFF_ABOUT_DISADVANTAGE;
	}
	return BTL_TYPEAFF_ABOUT_NONE;
}

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
extern u16 BTL_CALC_StatusRank( u16 defaultVal, u8 rank );
extern u8 BTL_CALC_HitPer( u8 defPer, u8 rank );
extern BOOL BTL_CALC_CheckCritical( u8 rank );

extern BtlTypeAff BTL_CALC_TypeAff( PokeType wazaType, PokeTypePair defenderType );
extern u32 BTL_CALC_AffDamage( u32 rawDamage, BtlTypeAff aff );

extern u8 BTL_CALC_HitCountMax( u8 numHitMax );
extern u16 BTL_CALC_RecvWeatherDamage( const BTL_POKEPARAM* bpp, BtlWeather weather );


//=============================================================================================
/**
 * 状態異常の継続ターン数を決定する
 *
 * @param   sick		状態異常ID
 *
 * @retval  u8			継続ターン数（永続する異常の場合は0）
 */
//=============================================================================================
extern u8 BTL_CALC_DecideSickTurn( WazaSick sick );

#endif

