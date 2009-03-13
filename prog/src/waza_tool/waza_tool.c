//============================================================================================
/**
 * @file	waza_tool.c
 * @bfief	技パラメータツール群（WB改訂版）
 * @author	HisashiSogabe
 * @date	08.11.13
 */
//============================================================================================
#include	<gflib.h>

#include	"arc_def.h"

#include	"waza_tool/waza_tool.h"
#include	"waza_tool/wazano_def.h"
#include	"waza_tool/wazadata.h"

#include	"waza_tool_def.h"


typedef struct{
	u16	monsno;
	s16 hp;

	u16	hpmax;
	u8	eqp;
	u8	atc;

	u32 condition;

	u8	speabino;
	u8	sex;
	u8	type1;
	u8	type2;

}WAZA_DAMAGE_CALC_PARAM;

//============================================================================================
/**
 * プロトタイプ宣言
 */
//============================================================================================
void		WT_WazaTableDataGet( void *buffer );
ARCHANDLE	*WT_WazaDataParaHandleOpen( HEAPID heapID );
void		WT_WazaDataParaHandleClose( ARCHANDLE *handle );
u32			WT_WazaDataParaGet( int waza_no, int id );
u32			WT_WazaDataParaGetHandle( ARCHANDLE *handle, int waza_no, int id );
u8			WT_PPMaxGet( u16 wazano, u8 maxupcnt );
u32			WT_WazaDataParaGetAct( WAZA_TABLE_DATA *wtd, int id );

static	void	WT_WazaDataGet( int waza_no, WAZA_TABLE_DATA *wtd );

//============================================================================================
/**
 *	技データテーブルデータをAllocしたメモリに展開
 *
 * @param[in]	heapID	データを展開するメモリヒープID
 */
//============================================================================================
void	WT_WazaTableDataGet( void *buffer )
{
	GFL_ARC_LoadDataOfs( buffer, ARCID_WAZA_TBL, 0, 0, sizeof( WAZA_TABLE_DATA ) * ( WAZANO_MAX + 1 ) );
}

//============================================================================================
/**
 *	技データテーブルファイルハンドルオープン
 *
 * @param[in]	heapID	ハンドルをAllocするヒープID
 */
//============================================================================================
ARCHANDLE	*WT_WazaDataParaHandleOpen( HEAPID heapID )
{
	return GFL_ARC_OpenDataHandle( ARCID_WAZA_TBL, heapID );
}

//============================================================================================
/**
 *	技データテーブルファイルハンドルクローズ
 *
 * @param[in]	handle	クローズするHandle
 */
//============================================================================================
void	WT_WazaDataParaHandleClose( ARCHANDLE *handle )
{
	GFL_ARC_CloseDataHandle( handle );
}

//============================================================================================
/**
 *	技データテーブルから値を取得
 *
 * @param[in]	waza_no		取得したい技ナンバー
 * @param[in]	id			取得したい値の種類
 */
//============================================================================================
u32	WT_WazaDataParaGet( int waza_no, int id )
{
	WAZA_TABLE_DATA wtd;

	WT_WazaDataGet( waza_no, &wtd );

	return	WT_WazaDataParaGetAct( &wtd, id );
}

//============================================================================================
/**
 *	技データテーブルから値を取得（ファイルハンドルを使用する）
 *
 * @param[in]	waza_no		取得したい技ナンバー
 * @param[in]	id			取得したい値の種類
 */
//============================================================================================
u32	WT_WazaDataParaGetHandle( ARCHANDLE *handle, int waza_no, int id )
{
	WAZA_TABLE_DATA wtd;

	GFL_ARC_LoadDataByHandle( handle, waza_no, &wtd );

	return	WT_WazaDataParaGetAct( &wtd, id );
}

//============================================================================================
/**
 *	PPMaxを取得
 *
 * @param[in]	wazano		取得する技ナンバー
 * @param[in]	maxupcnt	マックスアップを使用した回数
 *
 * @retval	PPMAX
 */
//============================================================================================
u8	WT_PPMaxGet( u16 wazano, u8 maxupcnt )
{
	u8	ppmax;

	if( maxupcnt > 3 ){
		maxupcnt = 3;
	}

	ppmax = WT_WazaDataParaGet( wazano, ID_WTD_pp );
	ppmax += ( ppmax * 20 * maxupcnt ) / 100;

	return	ppmax;
}

//============================================================================================
/**
 *	技テーブル構造体データから値を取得
 *
 * @param[in]	wtd		技テーブル構造体データへのポインタ
 * @param[in]	id		取得したい技データのIndex
 */
//============================================================================================
u32	WT_WazaDataParaGetAct( WAZA_TABLE_DATA *wtd, int id )
{
	u32	ret;

	switch( id ){
	case ID_WTD_battleeffect:
		ret = wtd->battleeffect;
		break;
	case ID_WTD_kind:
		ret = wtd->kind;
		break;
	case ID_WTD_damage:
		ret = wtd->damage;
		break;
	case ID_WTD_wazatype:
		ret = wtd->wazatype;
		break;
	case ID_WTD_hitprobability:
		ret = wtd->hitprobability;
		break;
	case ID_WTD_pp:
		ret = wtd->pp;
		break;
	case ID_WTD_addeffect:
		ret = wtd->addeffect;
		break;
	case ID_WTD_attackrange:
		ret = wtd->attackrange;
		break;
	case ID_WTD_attackpri:
		ret = wtd->attackpri;
		break;
	case ID_WTD_flag:
		ret = wtd->flag;
		break;
	case ID_WTD_ap_no:				//コンテスト用パラメータ　アピールNo
		ret = wtd->condata.ap_no;
		break;
	case ID_WTD_contype:			//コンテスト用パラメータ　コンテスト技タイプ
		ret = wtd->condata.contype;
		break;
	}
	return ret;
}

//============================================================================================
/**
 *							外部公開しない関数郡
 */
//============================================================================================
//============================================================================================
/**
 *	技テーブル構造体データを取得
 *
 * @param[in]	waza_no		取得したい技ナンバー
 * @param[out]	wtd			取得した技データの格納先を指定
 */
//============================================================================================
static	void	WT_WazaDataGet( int waza_no, WAZA_TABLE_DATA *wtd )
{
	GFL_ARC_LoadData( wtd, ARCID_WAZA_TBL, waza_no );
}



//=============================================================================================
/**
 * @file	wazadata.c
 * @brief	ポケモンWB  ワザデータアクセサ
 * @author	taya
 *
 * @date	2008.10.14	作成
 */
//=============================================================================================

#include "waza_tool/wazadata.h"


enum {
	HITRATIO_MUST = 101,
};

//------------------------------------------------------------------------------
/**
 * 定数宣言
 */
//------------------------------------------------------------------------------
enum {
	WSEQ_SINMPLESICK_NEMURI = 1,
	WSEQ_SINMPLESICK_DOKU = 66,
	WSEQ_SINMPLESICK_MAHI = 67,
	WSEQ_SINMPLESICK_MOUDOKU = 33,
	WSEQ_DAMAGESICK_DOKU = 2,
	WSEQ_DAMAGESICK_YAKEDO = 4,
	WSEQ_DAMAGESICK_KOORI = 5,
	WSEQ_DAMAGESICK_MAHI = 6,
	WSEQ_SIMPLEEFF_ATK = 10,
	WSEQ_SIMPLEEFF_DEF = 11,
	WSEQ_SIMPLEEFF_SPATK = 13,
	WSEQ_SIMPLEEFF_AVOID = 16,
	WSEQ_SHRINK = 31,
	WSEQ_MUST_SHRINK = 158,

};

// @@@ 旧バージョンのワザデータシーケンスを、新バージョンのデータテーブルとして扱うための
// 一時的な変換用テーブル
typedef struct {
	u16 wseq;
	u16 category;

	WazaSick  sick;
	struct {
		WazaRankEffect	type;
		s8							value;
	}rankEff[2];

	u8 shrinkFlg;

}SEQ_PARAM;


static const SEQ_PARAM* getSeqParam( WazaID waza )
{
	static const SEQ_PARAM SeqParamTable[] = {
		{
			WSEQ_SINMPLESICK_NEMURI, WAZADATA_CATEGORY_SIMPLE_SICK, POKESICK_NEMURI, 
			{ { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_SINMPLESICK_DOKU, WAZADATA_CATEGORY_SIMPLE_SICK, POKESICK_DOKU, 
			{ { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_SINMPLESICK_MAHI, WAZADATA_CATEGORY_SIMPLE_SICK, POKESICK_MAHI, 
			{ { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_DAMAGESICK_DOKU, WAZADATA_CATEGORY_DAMAGE_SICK, POKESICK_DOKU, 
			{ { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_DAMAGESICK_YAKEDO, WAZADATA_CATEGORY_DAMAGE_SICK, POKESICK_YAKEDO, 
			{ { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_DAMAGESICK_KOORI, WAZADATA_CATEGORY_DAMAGE_SICK, POKESICK_KOORI, 
			{ { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_DAMAGESICK_MAHI, WAZADATA_CATEGORY_DAMAGE_SICK, POKESICK_MAHI, 
			{ { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_SIMPLEEFF_ATK, WAZADATA_CATEGORY_SIMPLE_EFFECT, POKESICK_NULL, 
			{ { WAZA_RANKEFF_ATTACK, 1 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_SIMPLEEFF_DEF, WAZADATA_CATEGORY_SIMPLE_EFFECT, POKESICK_NULL, 
			{ { WAZA_RANKEFF_DEFENCE, 1 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_SIMPLEEFF_SPATK, WAZADATA_CATEGORY_SIMPLE_EFFECT, POKESICK_NULL, 
			{ { WAZA_RANKEFF_SP_ATTACK, 1 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
			WSEQ_SIMPLEEFF_AVOID, WAZADATA_CATEGORY_SIMPLE_EFFECT, POKESICK_NULL, 
			{ { WAZA_RANKEFF_AVOID, 1 }, { WAZA_RANKEFF_NULL, 0 } },
			FALSE,
		},{
				WSEQ_SHRINK, WAZADATA_CATEGORY_SIMPLE_DAMAGE, POKESICK_NULL, 
			{ { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
			TRUE,
		},{
				WSEQ_MUST_SHRINK, WAZADATA_CATEGORY_SIMPLE_DAMAGE, POKESICK_NULL, 
			{ { WAZA_RANKEFF_NULL, 0 }, { WAZA_RANKEFF_NULL, 0 } },
			TRUE,
		},
	};
	u16 seq = WT_WazaDataParaGet( waza, ID_WTD_battleeffect );
	u16 i;

	for(i=0; i<NELEMS(SeqParamTable); ++i)
	{
		if( SeqParamTable[i].wseq == seq )
		{
			return &SeqParamTable[i];
		}
	}
	return NULL;
}

// ワザ優先度
s8   WAZADATA_GetPriority( WazaID id )
{
	return WT_WazaDataParaGet( id, ID_WTD_attackpri );
}
// ワザカテゴリ
WazaCategory  WAZADATA_GetCategory( WazaID id )
{
	const SEQ_PARAM* seqparam = getSeqParam( id );

	if( seqparam )
	{
		return seqparam->category;
	}

	if( WAZADATA_IsDamage(id) )
	{
		return WAZADATA_CATEGORY_SIMPLE_DAMAGE;
	}
	else
	{
		return WAZADATA_CATEGORY_OTHERS;
	}
}

PokeType WAZADATA_GetType( WazaID id )
{
	return WT_WazaDataParaGet( id, ID_WTD_wazatype );
}

u16 WAZADATA_GetPower( WazaID id )
{
	return WT_WazaDataParaGet( id, ID_WTD_damage );
}

WazaDamageType WAZADATA_GetDamageType( WazaID id )
{
	return WT_WazaDataParaGet( id, ID_WTD_kind );
}

// 命中率
u8 WAZADATA_GetHitRatio( WazaID id )
{
	u8 ret = WT_WazaDataParaGet( id, ID_WTD_hitprobability );
	TAYA_Printf("[WAZATL] GetHitRatio : waza=%d, ratio=%d\n", id, ret);
	if( ret == 0 )
	{
		ret = 100;
	}
	return ret;
}

// 必中フラグ。つばめがえしなどがTRUEを返す。
BOOL WAZADATA_IsAlwaysHit( WazaID id )
{
	return WAZADATA_GetHitRatio(id) == HITRATIO_MUST;
}

// ダメージワザかどうか
BOOL WAZADATA_IsDamage( WazaID id )
{
	return WAZADATA_GetPower(id) != 0;
}

// クリティカルランク
u8 WAZADATA_GetCriticalRank( WazaID id )
{
	return 0;
}

// 最高ヒット回数
u8 WAZADATA_GetMaxHitCount( WazaID id )
{
	return 1;
}

// 効果範囲
WazaTarget WAZADATA_GetTarget( WazaID id )
{
	enum {
		OLD_RANGE_NORMAL		= 0,
		OLD_RANGE_NONE			= 1,
		OLD_RANGE_RANDOM		= 2,
		OLD_RANGE_ENEMY2		= 4,
		OLD_RANGE_OTHER_ALL	= 8,
		OLD_RANGE_USER			= 16,
		OLD_RANGE_FRIEND_ALL = 32,
		OLD_RANGE_WEATHER		= 64,
		OLD_RANGE_MAKIBISI	= 128,
		OLD_RANGE_TEDASUKE	= 256,
		OLD_RANGE_TUBO			= 512,
		OLD_RANGE_SAKIDORI	= 1024,
	};

	u32 old_range = WT_WazaDataParaGet( id, ID_WTD_attackrange );

	switch( old_range ){
	case OLD_RANGE_NORMAL:		return WAZA_TARGET_SINGLE;
	case OLD_RANGE_NONE:			return WAZA_TARGET_UNKNOWN;
	case OLD_RANGE_ENEMY2:		return WAZA_TARGET_ENEMY2;
	case OLD_RANGE_OTHER_ALL:	return WAZA_TARGET_OTHER_ALL;
	case OLD_RANGE_USER:			return WAZA_TARGET_ONLY_USER;
	case OLD_RANGE_FRIEND_ALL:return WAZA_TARGET_TEAM_USER;
	case OLD_RANGE_RANDOM:		return WAZA_TARGET_RANDOM;
	case OLD_RANGE_WEATHER:		return WAZA_TARGET_FIELD;
	case OLD_RANGE_MAKIBISI:	return WAZA_TARGET_TEAM_ENEMY;
	case OLD_RANGE_TEDASUKE:	return WAZA_TARGET_OTHER_FRIEND;
	case OLD_RANGE_TUBO:			return WAZA_TARGET_SINGLE_FRIEND;
	case OLD_RANGE_SAKIDORI:	return WAZA_TARGET_SINGLE_ENEMY;
	}

	return WAZA_TARGET_SINGLE;///< 自分以外の１体（選択）
}

// 追加効果の状態異常を返す
PokeSick WAZADATA_GetSick( WazaID id )
{
	const SEQ_PARAM* seq = getSeqParam( id );
	if( seq ){
		return seq->sick;
	}
	return POKESICK_NULL;
}

//=============================================================================================
/**
 * 追加の状態異常発生率を返す
 *
 * @param   id		ワザID
 *
 * @retval  u32		発生率（パーセンテージ = 0～100）
 */
//=============================================================================================
u32 WAZADATA_GetSickPer( WazaID id )
{
	// @@@ 今はてきとうに
	return WT_WazaDataParaGet( id, ID_WTD_addeffect );
}

//=============================================================================================
/**
 * 追加の「ひるみ」率を返す
 *
 * @param   id		ワザID
 *
 * @retval  u32		発生率（パーセンテージ = 0～100）
 */
//=============================================================================================
u32 WAZADATA_GetShrinkPer( WazaID id )
{
	const SEQ_PARAM* seq = getSeqParam( id );
	if( seq && seq->shrinkFlg ){
		return WT_WazaDataParaGet( id, ID_WTD_addeffect );
	}
	return 0;
}


#ifdef PM_DEBUG
void WAZADATA_PrintDebug( void )
{
	static const struct {
		const char* name;
		u16 id;
	}tbl[] = {
		{ "みずでっぽう（通常）",			WAZANO_MIZUDEPPOU },
		{ "ねっぷう（相手２体）",			WAZANO_NEPPUU },
		{ "なみのり（３体）",					WAZANO_NAMINORI },
		{ "かげぶんしん（自分）",			WAZANO_KAGEBUNSIN },
		{ "くろいきり（場）",					WAZANO_KUROIKIRI },
		{ "げきりん（ランダム）",			WAZANO_KUROIKIRI },
		{ "あまごい（場）",						WAZANO_AMAGOI },
		{ "あばれる（ランダム）",			WAZANO_ABARERU },
		{ "アロマセラピー（味方２体）",WAZANO_AROMASERAPII },
		{ "まきびし（まきびし）",				WAZANO_MAKIBISI },
		{ "てだすけ（てだすけ）",				WAZANO_TEDASUKE },
		{ "つぼつく（つぼつく）",				WAZANO_TUBOWOTUKU },
		{ "さきどり（さきどり）",				WAZANO_SAKIDORI },
		{ "オウムがえし（なし）",		WAZANO_OUMUGAESI },
	};
	u32 i, range;

	OS_TPrintf("***** ワザ効果範囲値チェック *****\n");
	for(i=0; i<NELEMS(tbl); ++i)
	{
		range = WT_WazaDataParaGet( tbl[i].id, ID_WTD_attackrange );
		OS_TPrintf("%s\t=%d\n", tbl[i].name, range);
		
	}
}

#endif
