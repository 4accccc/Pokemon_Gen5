//=============================================================================================
/**
 * @file	wazadata.c
 * @brief	ポケモンWB  ワザデータアクセサ
 * @author	taya
 *
 * @date	2008.10.14	作成
 */
//=============================================================================================

#ifdef TEST_IN_DOS
#include <stdlib.h>
#endif

#include "waza_tool/wazadata.h"


enum {
	HITRATIO_MUST = 101,
};


typedef struct {
	u16  id;

	u8   category;
	u8   type;

	u8   dmgType;
	u8   hitRatio;
	u8   power;
	u8   numMaxHit;

	u8   sick1;
	u8   sick1per;

	u8   influ1;
	u8   influ1per;
	s8   influ1volume;

}WAZA_DATA;

static const WAZA_DATA WazaDataTbl[] = {
	{
		WAZANO_NAMINORI,
		WAZADATA_CATEGORY_SIMPLE_DAMAGE, POKETYPE_WATER,
		WAZADATA_DMG_SPECIAL, 100, 80, 1,
		WAZASICK_NULL, 0,
		WAZA_INFLU_NULL, 0, 0,
	},{
		WAZANO_KOORINOKIBA,
		WAZADATA_CATEGORY_SIMPLE_DAMAGE, POKETYPE_KOORI,
		WAZADATA_DMG_PHYSIC, 100, 65, 1,
		WAZASICK_KOORI, 20,
		WAZA_INFLU_NULL, 0, 0,
	},{
		WAZANO_AMAERU,
		WAZADATA_CATEGORY_SIMPLE_EFFECT, POKETYPE_NORMAL,
		WAZADATA_DMG_NONE, 0, 0, 1,
		WAZASICK_NULL, 0,
		WAZA_INFLU_ATTACK, 0, -2,
	},{
		WAZANO_KAGEBUNSIN,
		WAZADATA_CATEGORY_SIMPLE_EFFECT, POKETYPE_NORMAL,
		WAZADATA_DMG_NONE, 0, 0, 1,
		WAZASICK_NULL, 0,
		WAZA_INFLU_AVOID, 0, 1,
	},{
		WAZANO_HIKARINOKABE,
		WAZADATA_CATEGORY_SIMPLE_EFFECT, POKETYPE_SP,
		WAZADATA_DMG_NONE, 0, 0, 1,
		WAZASICK_NULL, 0,
		WAZA_INFLU_AVOID, 0, 1,
	},{
		0,
		WAZADATA_CATEGORY_SIMPLE_DAMAGE, POKETYPE_WATER,
		WAZADATA_DMG_SPECIAL, 100, 80, 1,
		WAZASICK_NULL, 0,
		WAZA_INFLU_NULL, 0, 0,
	},
};


static const WAZA_DATA* getData( WazaID id )
{
	int i;
	for(i=0; WazaDataTbl[i].id != 0; i++)
	{
		if( WazaDataTbl[i].id == id )
		{
			break;
		}
	}

	return &WazaDataTbl[i];
}



s8   WAZADATA_GetPriority( WazaID id )
{
	int ret;

	srand( id );

	ret = WAZAPRI_MIN + (rand() % WAZAPRI_RANGE);

	srand( time(NULL) );

	return ret;
}


WazaCategory  WAZADATA_GetCategory( WazaID id )
{
	const WAZA_DATA* dat = getData( id );
	return dat->category;
}

PokeType WAZADATA_GetType( WazaID id )
{
	const WAZA_DATA* dat = getData( id );
	return dat->type;
}

u16 WAZADATA_GetPower( WazaID id )
{
	const WAZA_DATA* dat = getData( id );
	return dat->power;
}

WazaDamageType WAZADATA_GetDamageType( WazaID id )
{
	const WAZA_DATA* dat = getData( id );
	return dat->dmgType;
}
// 命中率
u8 WAZADATA_GetHitRatio( WazaID id )
{
	const WAZA_DATA* dat = getData( id );
	return dat->hitRatio;
}

// 必中フラグ。つばめがえしなどがTRUEを返す。
BOOL WAZADATA_IsAleadyHit( WazaID id )
{
	const WAZA_DATA* dat = getData( id );
	return dat->hitRatio == HITRATIO_MUST;
}
// ダメージワザかどうか
BOOL WAZADATA_IsDamage( WazaID id )
{
	const WAZA_DATA* dat = getData( id );
	return dat->dmgType != WAZADATA_DMG_NONE;
}

// クリティカルランク
u8 WAZADATA_GetCriticalRank( WazaID id )
{
	const WAZA_DATA* dat = getData( id );
	return dat->dmgType != WAZADATA_DMG_NONE;
}

// 最高ヒット回数
u8 WAZADATA_GetMaxHitCount( WazaID id )
{
	const WAZA_DATA* dat = getData( id );
	return dat->numMaxHit;
}

#if 0
BOOL      WAZADATA_IsDamage( WazaID id );
WazaDamageType  WAZADATA_GetDamageType( WazaID id );
u16  WAZADATA_GetPower( WazaID id );
u16  WAZADATA_GetHitRatio( WazaID id );


// 複数回連続攻撃ワザの最大回数。
extern u16  WAZADATA_GetContiniusCount( WazaID id );

// きゅうしょに当たるランク
extern u16  WAZADATA_GetCriticalRatio( WazaID id );

// 通常攻撃ワザであり、追加効果を含む場合は追加効果値が返る。
// 状態異常のみを起こすワザ（例えば催眠術など）は、WAZASICK_NULL が返る。
extern WazaSick  WAZADATA_GetAdditionalSick( WazaID id );

// 同上。こちらは追加効果の確率が返る。
extern u8  WAZADATA_GetAdditionalSickRatio( WazaID id );

#endif
