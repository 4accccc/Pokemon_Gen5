//=============================================================================================
/**
 * @file	poke_personal.c
 * @brief	ポケモンパーソナルデータアクセス関数群
 * @author	taya
 *
 * @date	2008.11.06	作成
 */
//=============================================================================================
#include <gflib.h>

#include "poke_tool/poke_tool.h"
#include "poke_personal_local.h"

#include "arc_def.h"


/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
	DEOKISISU_OTHER_FORM_PERSONAL = 	(496-1),	// デオキシスの別フォルムパーソナルの開始ナンバー
	MINOMESU_OTHER_FORM_PERSONAL = 		(499-1),	// ミノメスの別フォルムパーソナルの開始ナンバー
	GIRATINA_OTHER_FORM_PERSONAL = 		(501-1),	// ギラティナの別フォルムパーソナルの開始ナンバー
	SHEIMI_OTHER_FORM_PERSONAL = 		(502-1),	// シェイミの別フォルムパーソナルの開始ナンバー
	ROTOMU_OTHER_FORM_PERSONAL = 		(503-1),	// ロトムの別フォルムパーソナルの開始ナンバー
};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static u16 get_personalID( u16 mons_no, u16 form_no );
extern void POKE_PERSONAL_InitSystem( HEAPID heapID );
extern void POKE_PERSONAL_GetData( u16 mons_no, u16 form_no, POKEMON_PERSONAL_DATA* ppd );

/*--------------------------------------------------------------------------*/
/* Globals                                                                  */
/*--------------------------------------------------------------------------*/
static ARCHANDLE*  ArcHandle = NULL;


//=============================================================================================
/**
 * システム初期化（プログラム起動後に１度だけ呼び出す）
 *
 * @param   heapID		システム初期化用ヒープID（アーカイブハンドル用に数十バイト必要）
 *
 */
//=============================================================================================
void POKE_PERSONAL_InitSystem( HEAPID heapID )
{
	if( ArcHandle == NULL )
	{
		ArcHandle = GFL_ARC_OpenDataHandle( ARCID_PERSONAL, heapID );
	}
}




//=============================================================================================
/**
 * パーソナルデータをメモリ上にロード（poke_toolグループ内）
 *
 * @param   monsno		モンスターナンバー
 * @param   form_no		フォルムナンバー
 * @param   ppd			[out] データ取得用構造体アドレス
 *
 */
//=============================================================================================
void POKE_PERSONAL_LoadData( u16 mons_no, u16 form_no, POKEMON_PERSONAL_DATA* ppd )
{
	u32 data_ofs;
	u16 personalID = get_personalID( mons_no, form_no );

	data_ofs = GFL_ARC_GetDataOfsByHandle( ArcHandle, personalID );
	GFL_ARC_SeekDataByHandle( ArcHandle, data_ofs );
	GFL_ARC_LoadDataByHandleContinue( ArcHandle, sizeof(POKEMON_PERSONAL_DATA), ppd );
}

//=============================================================================================
/**
 * わざおぼえテーブル取得（poke_toolグループ内）
 *
 * @param   mons_no		モンスターナンバー
 * @param   form_no		フォルムナンバー
 * @param   dst			[out] わざ覚えテーブル取得用バッファアドレス
 *
 */
//=============================================================================================
void POKE_PERSONAL_LoadWazaOboeTable( u16 mons_no, u16 form_no, u16* dst )
{
	u16 personalID = get_personalID( mons_no, form_no );
	GFL_ARC_LoadData( dst, ARCID_WOTBL, personalID );
}



//=============================================================================================-
//=============================================================================================-

//=============================================================================================
/**
 * パーソナルデータハンドルオープン
 *
 * @param   mons_no		モンスターナンバー
 * @param   form_no		フォルムナンバー
 * @param   heapID		ハンドル生成用ヒープID
 *
 * @retval  POKEMON_PERSONAL_DATA*		パーソナルデータハンドル
 */
//=============================================================================================
POKEMON_PERSONAL_DATA*  POKE_PERSONAL_OpenHandle( u16 mons_no, u16 form_no, HEAPID heapID )
{
	POKEMON_PERSONAL_DATA* ppd = GFL_HEAP_AllocMemory( heapID, sizeof(POKEMON_PERSONAL_DATA) );
	POKE_PERSONAL_LoadData( mons_no, form_no, ppd );
	return ppd;
}

//=============================================================================================
/**
 * パーソナルデータハンドルクローズ
 *
 * @param   handle		パーソナルデータハンドル
 *
 */
//=============================================================================================
void POKE_PERSONAL_CloseHandle( POKEMON_PERSONAL_DATA* handle )
{
	GFL_HEAP_FreeMemory( handle );
}

//==============================================================================
/**
 * パーソナルデータからパラメータ取得
 *
 * @param   ppd			パーソナルデータハンドル
 * @param   paramID		取得したいパラメータID
 *
 * @retval  u32			パラメータ
 */
//==============================================================================
u32 POKE_PERSONAL_GetParam( POKEMON_PERSONAL_DATA *ppd, PokePersonalParamID paramID )
{
	u32 ret;

	GF_ASSERT( ppd );

	switch( paramID ){
	case POKE_PER_ID_basic_hp:		//基本ＨＰ
		ret = ppd->basic_hp;
		break;
	case POKE_PER_ID_basic_pow:		//基本攻撃力
		ret = ppd->basic_pow;
		break;
	case POKE_PER_ID_basic_def:		//基本防御力
		ret = ppd->basic_def;
		break;
	case POKE_PER_ID_basic_agi:		//基本素早さ
		ret = ppd->basic_agi;
		break;
	case POKE_PER_ID_basic_spepow:	//基本特殊攻撃力
		ret = ppd->basic_spepow;
		break;
	case POKE_PER_ID_basic_spedef:	//基本特殊防御力
		ret = ppd->basic_spedef;
		break;
	case POKE_PER_ID_type1:			//属性１
		ret = ppd->type1;
		break;
	case POKE_PER_ID_type2:			//属性２
		ret = ppd->type2;
		break;
	case POKE_PER_ID_get_rate:		//捕獲率
		ret = ppd->get_rate;
		break;
	case POKE_PER_ID_give_exp:		//贈与経験値
		ret = ppd->give_exp;
		break;
	case POKE_PER_ID_pains_hp:		//贈与努力値ＨＰ
		ret = ppd->pains_hp;
		break;
	case POKE_PER_ID_pains_pow:		//贈与努力値攻撃力
		ret = ppd->pains_pow;
		break;
	case POKE_PER_ID_pains_def:		//贈与努力値防御力
		ret = ppd->pains_def;
		break;
	case POKE_PER_ID_pains_agi:		//贈与努力値素早さ
		ret = ppd->pains_agi;
		break;
	case POKE_PER_ID_pains_spepow:	//贈与努力値特殊攻撃力
		ret = ppd->pains_spepow;
		break;
	case POKE_PER_ID_pains_spedef:	//贈与努力値特殊防御力
		ret = ppd->pains_spedef;
		break;
	case POKE_PER_ID_item1:			//アイテム１
		ret = ppd->item1;
		break;
	case POKE_PER_ID_item2:			//アイテム２
		ret = ppd->item2;
		break;
	case POKE_PER_ID_sex:			//性別ベクトル
		ret = ppd->sex;
		break;
	case POKE_PER_ID_egg_birth:		//タマゴの孵化歩数
		ret = ppd->egg_birth;
		break;
	case POKE_PER_ID_friend:			//なつき度初期値
		ret = ppd->friend;
		break;
	case POKE_PER_ID_grow:			//成長曲線識別
		ret = ppd->grow;
		break;
	case POKE_PER_ID_egg_group1:		//こづくりグループ1
		ret = ppd->egg_group1;
		break;
	case POKE_PER_ID_egg_group2:		//こづくりグループ2
		ret = ppd->egg_group2;
		break;
	case POKE_PER_ID_speabi1:		//特殊能力１
		ret = ppd->speabi1;
		break;
	case POKE_PER_ID_speabi2:		//特殊能力２
		ret = ppd->speabi2;
		break;
	case POKE_PER_ID_escape:			//逃げる率
		ret = ppd->escape;
		break;
	case POKE_PER_ID_color:			//色（図鑑で使用）
		ret = ppd->color;
		break;
	case POKE_PER_ID_reverse:		//反転フラグ
		ret = ppd->reverse;
		break;
	case POKE_PER_ID_machine1:		//技マシンフラグ１
		ret = ppd->machine1;
		break;
	case POKE_PER_ID_machine2:		//技マシンフラグ２
		ret = ppd->machine2;
		break;
	case POKE_PER_ID_machine3:		//技マシンフラグ３
		ret = ppd->machine3;
		break;
	case POKE_PER_ID_machine4:		//技マシンフラグ４
		ret = ppd->machine4;
		break;
	default:
		GF_ASSERT(0);
		ret = 0;
		break;
	}
	return ret;
}







//--------------------------------------------------------------------------
/**
 * モンスターナンバー＆フォルムナンバーからパーソナルデータIDを取得
 *
 * @param   mons_no		ポケモンナンバー
 * @param   form_no		フォルムナンバー
 *
 * @retval  u16			パーソナルデータID
 */
//--------------------------------------------------------------------------
static u16 get_personalID( u16 mons_no, u16 form_no )
{
	//フォルムによって、パーソナルが変化するものをチェック
	switch( mons_no ){
	case MONSNO_DEOKISISU:
		if( ( form_no ) && ( form_no <= FORMNO_DEOKISISU_MAX ) ){
			mons_no = DEOKISISU_OTHER_FORM_PERSONAL + form_no;
		}
		break;
	case MONSNO_MINOMESU:
		if( ( form_no ) && ( form_no <= FORMNO_MINOMUTTI_MAX ) ){
			mons_no = MINOMESU_OTHER_FORM_PERSONAL + form_no;
		}
		break;
		
	case MONSNO_KIMAIRAN:
		if( ( form_no ) && ( form_no <= FORMNO_GIRATINA_MAX ) ){
			mons_no = GIRATINA_OTHER_FORM_PERSONAL + form_no;
		}
		break;
	case MONSNO_EURISU:
		if( ( form_no ) && ( form_no <= FORMNO_SHEIMI_MAX ) ){
			mons_no = SHEIMI_OTHER_FORM_PERSONAL + form_no;
		}
		break;
	case MONSNO_PURAZUMA:
		if( ( form_no ) && ( form_no <= FORMNO_ROTOMU_MAX ) ){
			mons_no = ROTOMU_OTHER_FORM_PERSONAL + form_no;
		}
		break;
	
	default:
		break;
	}

	return mons_no;

}
