
#ifndef __BTL_CALC_H__
#define __BTL_CALC_H__

#include "btl_util.h"

enum {
	BTL_CALC_BASERANK_DEFAULT = 0,

	BTL_CALC_HITRATIO_MID = 6,
	BTL_CALC_HITRATIO_MIN = 0,
	BTL_CALC_HITRATIO_MAX = 12,

	BTL_CALC_CRITICAL_MIN = 0,
	BTL_CALC_CRITICAL_MAX = 4,


	BTL_CALC_CONF_PER = 30,		///< 混乱時の自爆確率

};

/**
 *	タイプ相性
 */
typedef enum {

	BTL_TYPEAFF_0 = 0,	///< 無効
	BTL_TYPEAFF_25,
	BTL_TYPEAFF_50,
	BTL_TYPEAFF_100,
	BTL_TYPEAFF_200,
	BTL_TYPEAFF_400,

}BtlTypeAff;

/**
 *	タイプ相性（こうかなし・ふつう・ばつぐん・もうひとつの４段階）
 */
typedef enum {

	BTL_TYPEAFF_ABOUT_NONE = 0,
	BTL_TYPEAFF_ABOUT_NORMAL,
	BTL_TYPEAFF_ABOUT_ADVANTAGE,
	BTL_TYPEAFF_ABOUT_DISADVANTAGE,

}BtlTypeAffAbout;

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


extern u16 BTL_CALC_StatusRank( u16 defaultVal, u8 rank );
extern u8 BTL_CALC_HitPer( u8 defPer, u8 rank );
extern BOOL BTL_CALC_CheckCritical( u8 rank );

extern BtlTypeAff BTL_CALC_TypeAff( PokeType wazaType, PokeTypePair defenderType );
extern u32 BTL_CALC_AffDamage( u32 rawDamage, BtlTypeAff aff );

extern u8 BTL_CALC_HitCountMax( u8 numHitMax );

#endif

