
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

	BTL_MAHI_EXE_RATIO = 25,			///< 「まひ」でしびれて動けない確率
	BTL_MAHI_AGILITY_RATIO = 25,	///< 「まひ」時のすばやさ減衰率
	BTL_KORI_MELT_RATIO = 20,			///< 「こおり」が溶ける確率

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

	// とくせい関連
	BTL_CALC_TOK_CHIDORI_HITRATIO = FX32_CONST(0.8f),			/// 「ちどりあし」の減衰命中率
	BTL_CALC_TOK_HARIKIRI_HITRATIO = FX32_CONST(0.8f),		/// 「はりきり」の命中率変化率
	BTL_CALC_TOK_FUKUGAN_HITRATIO = FX32_CONST(1.3f),			/// 「ふくがん」の命中変化率
	BTL_CALC_TOK_SUNAGAKURE_HITRATIO = FX32_CONST(0.8f),	/// 「すながくれ」の命中率変化率
	BTL_CALC_TOK_YUKIGAKURE_HITRATIO = FX32_CONST(0.8f),	/// 「ゆきがくれ」の命中変化率
	BTL_CALC_TOK_HARIKIRI_POWRATIO = FX32_CONST(1.5f),		/// 「はりきり」の攻撃力変化率
	BTL_CALC_TOK_TETUNOKOBUSI_POWRATIO = FX32_CONST(1.2f),/// 「てつのこぶし」の攻撃力変化率
	BTL_CALC_TOK_SUTEMI_POWRATIO = FX32_CONST(1.2f),			/// 「すてみ」の攻撃力変化率
	BTL_CALC_TOK_PLUS_POWRATIO = FX32_CONST(1.5f),				/// 「プラス」の攻撃力変化率
	BTL_CALC_TOK_MINUS_POWRATIO = FX32_CONST(1.5f),				/// 「マイナス」の攻撃力変化率
	BTL_CALC_TOK_FLOWERGIFT_POWRATIO = FX32_CONST(1.5f),	/// 「フラワーギフト」の攻撃力変化率
	BTL_CALC_TOK_FLOWERGIFT_GUARDRATIO = FX32_CONST(1.5f),/// 「フラワーギフト」の特防変化率
	BTL_CALC_TOK_MORAIBI_POWRATIO = FX32_CONST(1.5f),			/// 「もらいび」の攻撃力変化率
	BTL_CALC_TOK_TECKNICIAN_POWRATIO = FX32_CONST(1.5f),	/// 「テクニシャン」の攻撃力変化率
	BTL_CALC_TOK_HAYAASI_AGIRATIO = FX32_CONST(1.5f),			/// 「はやあし」のすばやさ変化率
	BTL_CALC_TOK_SLOWSTART_ENABLE_TURN = 5,								/// 「スロースタート」の適用ターン
	BTL_CALC_TOK_SLOWSTART_AGIRATIO = FX32_CONST(0.5f),		/// 「スロースタート」のすばやさ変化率
	BTL_CALC_TOK_SLOWSTART_ATKRATIO = FX32_CONST(0.5f),		/// 「スロースタート」のこうげき変化率
	BTL_CALC_TOK_FUSIGINAUROKO_GDRATIO = FX32_CONST(1.5f),	/// 「ふしぎなうろこ」のとくぼう変化率

	BTL_CALC_TOK_DOKUNOTOGE_PER   = 30,		///「どくのトゲ」発生確率
	BTL_CALC_TOK_HONONOKARADA_PER = 30,		///「ほのおのからだ」発生確率
	BTL_CALC_TOK_SEIDENKI_PER     = 30,		///「せいでんき」発生確率
	BTL_CALC_TOK_HOUSI_PER        = 30,		///「ほうし」発生確率
	BTL_CALC_TOK_MEROMEROBODY_PER = 30,		///「メロメロボディ」発生確率


};

static inline u32 BTL_CALC_MulRatio( u32 value, fx32 ratio )
{
	return (value * ratio) >> FX32_SHIFT;
}

static inline u32 BTL_CALC_IsOccurPer( u32 per )
{
	return (GFL_STD_MtRand(100) < per);
}

static inline int BTL_CALC_Roundup( int value, int min )
{
	if( value < min ){ value = min; }
	return value;
}

static inline u32 BTL_CALC_QuotMaxHP( const BTL_POKEPARAM* bpp, u32 denom )
{
	u32 ret = BTL_POKEPARAM_GetValue( bpp, BPP_MAX_HP ) / denom;
	if( ret == 0 ){ ret = 1; }
	return ret;
}

static inline u32 BTL_CALC_RandRange( u32 min, u32 max )
{
	if( min > max ){
		u32 tmp = min;
		min = max;
		max = tmp;
	}
	{
		u32 range = 1 + (max-min);
		return min + GFL_STD_MtRand( range );
	}
}

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
 * ワザデータの状態異常継続パラメータ値から、バトルで使う状態異常継続パラメータ値へ変換
 *
 * @param   wazaSickCont		
 * @param   attacker		ワザを使ったポケモン
 * @param   sickCont		[out]
 *
 */
//=============================================================================================
extern void BTL_CALC_WazaSickContToBppSickCont( WAZA_SICKCONT_PARAM wazaSickCont, const BTL_POKEPARAM* attacker, BPP_SICK_CONT* sickCont );

extern void BTL_CALC_MakeDefaultPokeSickCont( PokeSick sick, BPP_SICK_CONT* cont );
extern void BTL_CALC_MakeDefaultWazaSickCont( PokeSick sick, const BTL_POKEPARAM* attacker, BPP_SICK_CONT* cont );

//=============================================================================================
/**
 * 「トレース」によってコピーできないとくせい判定
 *
 * @param   tok		
 *
 * @retval  BOOL		コピーできない場合はTRUE
 */
//=============================================================================================
extern BOOL BTL_CALC_TOK_CheckCant_Trace( PokeTokusei tok );

//=============================================================================================
/**
 * 「スキルスワップ」によってコピーできないとくせい判定
 *
 * @param   tok		
 *
 * @retval  BOOL		コピーできない場合はTRUE
 */
//=============================================================================================
extern BOOL BTL_CALC_TOK_CheckCant_Swap( PokeTokusei tok );


#endif

