//============================================================================================
/**
 * @file	poke_tool.c
 * @bfief	ポケモンパラメータツール群（WB改定版）
 * @author	HisashiSogabe
 * @date	08.11.12
 */
//============================================================================================
#include    <gflib.h>

#include    "poke_tool/poke_tool.h"
#include    "poke_tool/monsno_def.h"
#include    "poke_tool/tokusyu_def.h"
#include    "waza_tool/waza_tool.h"

#include    "poke_tool_def.h"
#include    "poke_personal_local.h"

#include    "arc_def.h"


//============================================================================================
/**
 * プロトタイプ宣言
 */
//============================================================================================

//ポケモンパラメータ操作関数系
void	PokeParaInit( POKEMON_PARAM *pp );
void	PokePasoParaInit( POKEMON_PASO_PARAM *ppp );

int				PokemonParam_GetWorkSize( void );
POKEMON_PARAM	*PokemonParam_AllocWork( HEAPID heapID );

BOOL	PokeParaFastModeOn( POKEMON_PARAM *pp );
BOOL	PokeParaFastModeOff( POKEMON_PARAM *pp, BOOL flag );
BOOL	PokePasoParaFastModeOn( POKEMON_PASO_PARAM *ppp );
BOOL	PokePasoParaFastModeOff( POKEMON_PASO_PARAM *ppp, BOOL flag );

void	PokeParaSet( POKEMON_PARAM *pp, int mons_no, int level, int pow, int rndflag, u32 rnd, int idflag, u32 id );
void	PokePasoParaSet( POKEMON_PASO_PARAM *ppp, int mons_no, int level, int pow, int rndflag, u32 rnd, int idflag, u32 id );

void	PokeParaCalc( POKEMON_PARAM *pp );
void	PokeParaCalcLevelUp( POKEMON_PARAM *pp );

void	PokeParaPut( POKEMON_PARAM *pp, int id, const void *buf);
void	PokePasoParaPut( POKEMON_PASO_PARAM *ppp, int id, const void *buf);
void	PokeParaAdd( POKEMON_PARAM *pp, int id, int value);
void	PokePasoParaAdd( POKEMON_PASO_PARAM *ppp, int id, int value);
BOOL	PokeRareCheck( u32 id, u32 rnd );
u8		PokeSexGet( POKEMON_PARAM *pp );
u8		PokePasoSexGet( POKEMON_PASO_PARAM *ppp );
u8		PokePersonal_SexGet( POKEMON_PERSONAL_DATA* personalData, u16 monsno, u32 rnd );
void	PokeWazaOboe( POKEMON_PARAM *pp );
void	PokePasoWazaOboe( POKEMON_PASO_PARAM *ppp );
u16		PokeWazaSet( POKEMON_PARAM *pp, u16 wazano );
u16		PokePasoWazaSet( POKEMON_PASO_PARAM *ppp, u16 wazano );
void	PokeWazaOboeOshidashi( POKEMON_PARAM *pp, u16 wazano );
void	PokePasoWazaOboeOshidashi( POKEMON_PASO_PARAM *ppp, u16 wazano );
void	PokeWazaSetPos( POKEMON_PARAM *pp, u16 wazano, u8 pos );
void	PokePasoWazaSetPos( POKEMON_PASO_PARAM *ppp, u16 wazano, u8 pos );
u32		PokeParaLevelCalc( POKEMON_PARAM *pp );
u32		PokePasoLevelCalc( POKEMON_PASO_PARAM *ppp );
u32		PokePersonal_LevelCalc( POKEMON_PERSONAL_DATA* personalData, u32 exp );
u32		PokeParaLevelExpGet(POKEMON_PARAM *pp);
u8		PokeSeikakuGet( POKEMON_PARAM *pp );
u8		PokePasoSeikakuGet( POKEMON_PASO_PARAM *ppp );

static	BOOL	PokeParaDecodedAct( POKEMON_PARAM *pp );
static	BOOL	PokePasoParaDecodedAct( POKEMON_PASO_PARAM *ppp );
static	void	PokeParaCodedAct( POKEMON_PARAM *pp );
static	void	PokePasoParaCodedAct( POKEMON_PASO_PARAM *ppp );

static	u32		PokeParaGetAct( POKEMON_PARAM *pp, int id, void *buf);
static	u32		PokePasoParaGetAct( POKEMON_PASO_PARAM *ppp, int id, void *buf);
static	void	PokeParaPutAct( POKEMON_PARAM *pp, int id, const void *buf);
static	void	PokePasoParaPutAct( POKEMON_PASO_PARAM *ppp, int id, const void *buf);
static	void	PokeParaAddAct( POKEMON_PARAM *pp, int id, int value);
static	void	PokePasoParaAddAct( POKEMON_PASO_PARAM *ppp, int id, int value);

static	void	ParaAddCalc( u8 *data, int value, int max );
static	void	PokeParaCoded( void *data, u32 size, u32 code );
static	u16		CodeRand( u32 *code );
static	u16		PokeParaMakeCheckSum( void *data, u32 size );
static	u16		PokeChrAbiCalc( u8 chr, u16 para, u8 cond );
static	void	load_grow_table( int para, u32 *GrowTable );
static	u32		PokeGrowParaGet( int para, int level );
static	void	*PokeParaAdrsGet( POKEMON_PASO_PARAM *ppp, u32 rnd, u8 id );

#define	PokeParaDecoded( data, size, code )		PokeParaCoded( data, size, code );

//ポケモンパーソナル操作関数系
u32	PokePersonalParaGet( u16 mons_no, u16 form_no, int para );

//============================================================================================
/**
 *	スタティック変数
 */
//============================================================================================
static	POKEMON_PERSONAL_DATA	PersonalDataWork;	//ポケモン１体分のパーソナルデータ構造体
static	u32 GrowTable[ GROW_TBL_SIZE ];				//成長曲線テーブル




//=============================================================================================
/**
 * システム初期化（プログラム起動後に１度だけ呼び出す）
 *
 * @param   heapID		システム初期化用ヒープID
 *
 */
//=============================================================================================
void POKETOOL_InitSystem( HEAPID heapID )
{
	POKE_PERSONAL_InitSystem( heapID );
}





//============================================================================================
/**
 * ポケモンパラメータ操作関数系
 */
//============================================================================================
//============================================================================================
/**
 *	ポケモンパラメータ構造体を初期化する
 *
 * @param[in]	pp	初期化するポケモンパラメータ構造体のポインタ
 */
//============================================================================================
void	PokeParaInit( POKEMON_PARAM *pp )
{
	MI_CpuClearFast( pp, sizeof( POKEMON_PARAM ) );
//パラメータを暗号化
	PokeParaCoded( &pp->ppp.paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, pp->ppp.checksum );
	PokeParaCoded( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体を初期化する
 *
 * @param[in]	ppp	初期化するボックスポケモンパラメータ構造体のポインタ
 */
//============================================================================================
void	PokePasoParaInit( POKEMON_PASO_PARAM *ppp )
{
	MI_CpuClearFast( ppp, sizeof( POKEMON_PASO_PARAM ) );
//個性乱数セットしたところで暗号化
	PokeParaCoded( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
}

//============================================================================================
/**
 *	POKEMON_PARAM構造体のサイズを返す
 *
 * @retval	POKEMON_PARAM構造体のサイズ
 */
//============================================================================================
int		PokemonParam_GetWorkSize( void )
{
	return sizeof( POKEMON_PARAM );
}

//============================================================================================
/**
 *	POKEMON_PARAM構造体のワークを確保、初期化してポインタを返す
 *
 *	@param[in]	heapID	メモリを確保するヒープID
 *
 * @retval	POKEMON_PARAM構造体へのポインタ
 */
//============================================================================================
POKEMON_PARAM	*PokemonParam_AllocWork( HEAPID heapID )
{
	POKEMON_PARAM	*pp;

	pp = GFL_HEAP_AllocMemory( heapID, sizeof( POKEMON_PARAM ) );
	PokeParaInit( pp );

	return pp;
}

//============================================================================================
/**
 *	ポケモンパラメータ構造体へのアクセスを高速化モードへ
 *
 * @param[in]	pp	ポケモンパラメータ構造体へのポインタ
 *
 * @retval	FALSE:すでに高速化モードになっている　TRUE:高速化モードへ移行
 */
//============================================================================================
BOOL	PokeParaFastModeOn( POKEMON_PARAM *pp )
{
	BOOL	ret;

	ret = FALSE;

	if( pp->ppp.pp_fast_mode == 0 ){
		ret = TRUE;
		GF_ASSERT_MSG( pp->ppp.ppp_fast_mode == 0, "pppが高速化モードになっています\n" );
		pp->ppp.pp_fast_mode = 1;
		pp->ppp.ppp_fast_mode = 1;
		PokeParaDecoded( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
		PokeParaDecoded( &pp->ppp.paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, pp->ppp.checksum );
	}

	return ret;
}

//============================================================================================
/**
 *	ポケモンパラメータ構造体へのアクセスを通常モードへ
 *
 * @param[in]	pp	ポケモンパラメータ構造体へのポインタ
 * @param[in]	flag	モード移行可能か判断するためのフラグ（FALSE:移行不可、TRUE:移行する）
 *
 * @retval	FALSE:すでに通常モードになっている　TRUE:通常モードへ移行
 */
//============================================================================================
BOOL	PokeParaFastModeOff( POKEMON_PARAM *pp, BOOL flag )
{
	BOOL	ret;

	ret = FALSE;

	if( ( pp->ppp.pp_fast_mode == 1 ) && ( flag == TRUE ) ){
		ret = TRUE;
		pp->ppp.pp_fast_mode = 0;
		pp->ppp.ppp_fast_mode = 0;

		PokeParaCoded( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
		pp->ppp.checksum = PokeParaMakeCheckSum( &pp->ppp.paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
		PokeParaCoded( &pp->ppp.paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, pp->ppp.checksum );
	}

	return ret;
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体へのアクセスを高速化モードへ
 *
 * @param[in]	ppp	ボックスポケモンパラメータ構造体へのポインタ
 *
 * @retval	FALSE:すでに高速化モードになっている　TRUE:高速化モードへ移行
 */
//============================================================================================
BOOL	PokePasoParaFastModeOn( POKEMON_PASO_PARAM *ppp )
{
	BOOL	ret;

	ret = FALSE;

	if( ppp->ppp_fast_mode == 0 ){
		ret = TRUE;
		ppp->ppp_fast_mode = 1;
		PokeParaDecoded( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
	}

	return ret;
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体へのアクセスを通常モードへ
 *
 * @param[in]	ppp		ボックスポケモンパラメータ構造体へのポインタ
 * @param[in]	flag	モード移行可能か判断するためのフラグ（FALSE:移行不可、TRUE:移行する）
 *
 * @retval	FALSE:すでに通常モードになっている　TRUE:通常モードへ移行
 */
//============================================================================================
BOOL	PokePasoParaFastModeOff( POKEMON_PASO_PARAM *ppp, BOOL flag )
{
	BOOL	ret;

	ret = FALSE;

	if( ( ppp->ppp_fast_mode == 1 ) && ( flag == TRUE ) ){
		ret = TRUE;
		ppp->ppp_fast_mode = 0;

		ppp->checksum = PokeParaMakeCheckSum( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
		PokeParaCoded( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
	}

	return ret;
}

//============================================================================================
/**
 *	ポケモンパラメータ構造体にポケモンパラメータを生成
 *
 * @param[in]	pp		パラメータを生成するポケモンパラメータ構造体のポインタ
 * @param[in]	mons_no	パラメータを生成するポケモンナンバー
 * @param[in]	level	生成するレベル
 * @param[in]	pow		POW_RND以外:セットするパワー乱数　POW_RND:パワー乱数をランダムでセット
 * @param[in]	rndflag	個性乱数セットフラグ（RND_NO_SET:個性乱数をランダムでセット RND_SET:引数rndの値をセット
 * @param[in]	rnd		rndflagがRND_SETの時にセットされる値
 * @param[in]	idflag	IDNoセットフラグ（ID_NO_SET:プレイヤーのIDをセット ID_SET:引数idの値をセット
 *										　ID_NO_RARE:個性乱数との組み合わせでレアポケモンにならないように値を調整してセット）
 * @param[in]	id		idflagがID_SETの時にセットされる値
 */
//============================================================================================
void	PokeParaSet( POKEMON_PARAM *pp, int mons_no, int level, int pow, int rndflag, u32 rnd, int idflag, u32 id )
{
	u32			i;
//メールとカスタムボール対応がまだです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning MAIL_DATA and CB_CORE nothing
#endif
//	MAIL_DATA	*mail_data;
//	CB_CORE		cb_core;

	PokeParaInit( pp );

	PokePasoParaSet( ( POKEMON_PASO_PARAM * )&pp->ppp, mons_no, level, pow, rndflag, rnd, idflag, id );

//個性乱数がきまったら、PCPを再暗号化
	PokeParaCoded( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), 0 );
	PokeParaCoded( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );

//レベルセット
	PokeParaPut( pp, ID_PARA_level, (u8 *)&level );

#if 0
//メールデータ
	mail_data = MailData_CreateWork( HEAPID_BASE_SYSTEM );
	PokeParaPut( pp, ID_PARA_mail_data, mail_data );
	GFL_HEAP_FreeMemory( mail_data );

//カスタムボールID
	i = 0;
	PokeParaPut( pp, ID_PARA_cb_id, (u8 *)&i );

//カスタムボール
	MI_CpuClearFast( &cb_core, sizeof( CB_CORE ) );
	PokeParaPut( pp, ID_PARA_cb_core, ( CB_CORE * )&cb_core );							
#endif

	PokeParaCalc( pp );
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体にポケモンパラメータを生成
 *
 * @param[in]	ppp		パラメータを生成するボックスポケモンパラメータ構造体のポインタ
 * @param[in]	mons_no	パラメータを生成するポケモンナンバー
 * @param[in]	level	生成するレベル
 * @param[in]	pow		POW_RND以外:セットするパワー乱数　POW_RND:パワー乱数をランダムでセット
 * @param[in]	rndflag	個性乱数セットフラグ（RND_NO_SET:個性乱数をランダムでセット RND_SET:引数rndの値をセット
 * @param[in]	rnd		rndflagがRND_SETの時にセットされる値
 * @param[in]	idflag	IDNoセットフラグ（ID_NO_SET:プレイヤーのIDをセット ID_SET:引数idの値をセット
 *										　ID_NO_RARE:個性乱数との組み合わせでレアポケモンにならないように値を調整してセット）
 * @param[in]	id		idflagがID_SETの時にセットされる値
 */
//============================================================================================
void PokePasoParaSet( POKEMON_PASO_PARAM *ppp, int mons_no, int level, int pow, int rndflag, u32 rnd, int idflag, u32 id )
{
	BOOL	flag;
	u32		i,j;
	POKEMON_PERSONAL_DATA* ppd;
	u16		sum;

	PokePasoParaInit( ppp );

	flag = PokePasoParaFastModeOn( ppp );

// パーソナルデータをロードしておく
	POKE_PERSONAL_LoadData( mons_no, FORM_NO_NONE, &PersonalDataWork );
	ppd = &PersonalDataWork;

//個性乱数セット
	if( !rndflag ){
		rnd = __GFL_STD_MtRand();
	}
	PokePasoParaPut( ppp, ID_PARA_personal_rnd, (u8 *)&rnd );

//IDナンバーセット
	if( idflag == ID_NO_RARE ){
		do{
			id = __GFL_STD_MtRand();
		}while( PokeRareCheck( id, rnd ) == TRUE );
	}
	else if( idflag != ID_SET ){
		id = 0;
	}
	PokePasoParaPut( ppp, ID_PARA_id_no, (u8 *) & id );

#ifdef DEBUG_ONLY_FOR_sogabe
#warning CasetteLanguage Nothing
#endif

//国コード
//	PokePasoParaPut( ppp, ID_PARA_country_code, ( u8 * )&CasetteLanguage );

//ポケモンナンバーセット
	PokePasoParaPut( ppp, ID_PARA_monsno, (u8 *)&mons_no );

//ニックネームセット
	PokePasoParaPut( ppp, ID_PARA_default_name, NULL );

//経験値セット
	i = POKETOOL_GetMinExp( mons_no, FORM_NO_NONE, level );
	PokePasoParaPut( ppp, ID_PARA_exp, (u8 *)&i );

//友好値セット
	i = POKE_PERSONAL_GetParam( ppd, POKE_PER_ID_friend );
	PokePasoParaPut( ppp, ID_PARA_friend, ( u8 * )&i );

//捕獲データセット
	PokePasoParaPut( ppp, ID_PARA_get_level, ( u8 * )&level );

#ifdef DEBUG_ONLY_FOR_sogabe
#warning CasetteVersion Nothing
#endif

//	PokePasoParaPut( ppp, ID_PARA_get_cassette, ( u8 * )&CasetteVersion );
	i = ITEM_MONSUTAABOORU;	//デフォルトはモンスターボールにしておく	
	PokePasoParaPut( ppp, ID_PARA_get_ball, ( u8 * )&i );

//パワー乱数セット
	if( pow < POW_RND ){
		PokePasoParaPut( ppp, ID_PARA_hp_rnd,		( u8 * )&pow );
		PokePasoParaPut( ppp, ID_PARA_pow_rnd,		( u8 * )&pow );
		PokePasoParaPut( ppp, ID_PARA_def_rnd,		( u8 * )&pow );
		PokePasoParaPut( ppp, ID_PARA_agi_rnd,		( u8 * )&pow );
		PokePasoParaPut( ppp, ID_PARA_spepow_rnd,	( u8 * )&pow );
		PokePasoParaPut( ppp, ID_PARA_spedef_rnd,	( u8 * )&pow );
	}
	else{
		i = __GFL_STD_MtRand();
		j = ( i & ( 0x0000001f <<  0 ) ) >>  0;
		PokePasoParaPut( ppp, ID_PARA_hp_rnd,		( u8 * )&j );
		j = ( i & ( 0x0000001f <<  5 ) ) >>  5;
		PokePasoParaPut( ppp, ID_PARA_pow_rnd,		( u8 * )&j );
		j = ( i & ( 0x0000001f << 10 ) ) >> 10;
		PokePasoParaPut( ppp, ID_PARA_def_rnd,		( u8 * )&j );
		j = ( i & ( 0x0000001f << 15 ) ) >> 15;
		PokePasoParaPut( ppp, ID_PARA_agi_rnd,		( u8 * )&j );
		j = ( i & ( 0x0000001f << 20 ) ) >> 20;
		PokePasoParaPut( ppp, ID_PARA_spepow_rnd,	( u8 * )&j );
		j = ( i & ( 0x0000001f << 25 ) ) >> 25;
		PokePasoParaPut( ppp, ID_PARA_spedef_rnd,	( u8 * )&j );
	}

//特殊能力セット
	i = POKE_PERSONAL_GetParam( ppd, POKE_PER_ID_speabi1 );
	j = POKE_PERSONAL_GetParam( ppd, POKE_PER_ID_speabi2 );
	if( j != 0 ){
		if( rnd & 1 ){
			PokePasoParaPut( ppp, ID_PARA_speabino, ( u8 * )&j );
		}
		else{
			PokePasoParaPut( ppp, ID_PARA_speabino, ( u8 * )&i );
		}
	}
	else{
		PokePasoParaPut( ppp, ID_PARA_speabino, ( u8 * )&i );
	}

//性別セット
	i = PokePasoSexGet( ppp );
	PokePasoParaPut( ppp, ID_PARA_sex, ( u8 * )&i );

//技セット
	PokePasoWazaOboe( ppp );

	PokePasoParaFastModeOff( ppp, flag );
}

//============================================================================================
/**
 *	ポケモンパラメータ計算ルーチン
 *
 * @param[in]	pp	計算するポケモンパラメータ構造体のポインタ
 */
//============================================================================================
void	PokeParaCalc( POKEMON_PARAM *pp )
{
	int		level;
	BOOL	flag;

	flag = PokeParaFastModeOn( pp );

//レベルセット
	level = PokeParaLevelCalc( pp );
	PokeParaPut( pp, ID_PARA_level, ( u8 * )&level );

	PokeParaCalcLevelUp( pp );

	PokeParaFastModeOff( pp, flag );
}

//============================================================================================
/**
 *	ポケモンパラメータ計算ルーチン（レベルアップ時専用）
 *
 * @param[in]	pp	計算するポケモンパラメータ構造体のポインタ
 */
//============================================================================================
void	PokeParaCalcLevelUp( POKEMON_PARAM *pp )
{
	int	oldhpmax, hp, hpmax;
	int	pow, def, agi, spepow, spedef;
	int	hp_rnd, pow_rnd, def_rnd, agi_rnd, spepow_rnd, spedef_rnd;
	int	hp_exp, pow_exp, def_exp, agi_exp, spepow_exp, spedef_exp;
	int	monsno;
	int	level;
	int	form_no;
	int	speabi1,speabi2,rnd;
	BOOL	flag;
	POKEMON_PERSONAL_DATA* ppd;

	flag = PokeParaFastModeOn( pp );

	level =			PokeParaGet( pp, ID_PARA_level,			0);
	oldhpmax =		PokeParaGet( pp, ID_PARA_hpmax,			0);
	hp =			PokeParaGet( pp, ID_PARA_hp,			0);
	hp_rnd =		PokeParaGet( pp, ID_PARA_hp_rnd,		0);
	hp_exp =		PokeParaGet( pp, ID_PARA_hp_exp,		0);
	pow_rnd =		PokeParaGet( pp, ID_PARA_pow_rnd,		0);
	pow_exp =		PokeParaGet( pp, ID_PARA_pow_exp,		0);
	def_rnd =		PokeParaGet( pp, ID_PARA_def_rnd,		0);
	def_exp =		PokeParaGet( pp, ID_PARA_def_exp,		0);
	agi_rnd =		PokeParaGet( pp, ID_PARA_agi_rnd,		0);
	agi_exp =		PokeParaGet( pp, ID_PARA_agi_exp,		0);
	spepow_rnd =	PokeParaGet( pp, ID_PARA_spepow_rnd,	0);
	spepow_exp =	PokeParaGet( pp, ID_PARA_spepow_exp,	0);
	spedef_rnd =	PokeParaGet( pp, ID_PARA_spedef_rnd,	0);
	spedef_exp =	PokeParaGet( pp, ID_PARA_spedef_exp,	0);
	form_no =		PokeParaGet( pp, ID_PARA_form_no,		0);

	monsno = PokeParaGet( pp, ID_PARA_monsno, 0 );

	POKE_PERSONAL_LoadData( monsno, form_no, &PersonalDataWork );
	ppd = &PersonalDataWork;

	if( monsno == MONSNO_NUKENIN ){
		hpmax = 1;
	}
	else{
		hpmax = ( ( 2 * ppd->basic_hp + hp_rnd + hp_exp / 4 ) * level / 100 + level + 10 );
	}

	PokeParaPut( pp, ID_PARA_hpmax, ( u8 * )&hpmax );

	pow = ( ( 2 * ppd->basic_pow + pow_rnd + pow_exp / 4 ) * level / 100 + 5 );
	pow = PokeChrAbiCalc( PokeSeikakuGet( pp ), pow, ABILITY_POW );
	PokeParaPut( pp, ID_PARA_pow, ( u8 * )&pow );

	def = ( ( 2 * ppd->basic_def + def_rnd + def_exp / 4 ) * level / 100 + 5 );
	def = PokeChrAbiCalc( PokeSeikakuGet( pp ), def, ABILITY_DEF );
	PokeParaPut( pp, ID_PARA_def, ( u8 * )&def );

	agi = ( ( 2 * ppd->basic_agi + agi_rnd + agi_exp / 4 ) * level / 100 + 5 );
	agi = PokeChrAbiCalc( PokeSeikakuGet( pp ), agi, ABILITY_AGI );
	PokeParaPut( pp, ID_PARA_agi, ( u8 * )&agi );

	spepow = ( ( 2 * ppd->basic_spepow + spepow_rnd + spepow_exp / 4 ) * level / 100 + 5 );
	spepow = PokeChrAbiCalc( PokeSeikakuGet( pp ), spepow, ABILITY_SPEPOW );
	PokeParaPut( pp, ID_PARA_spepow, ( u8 * )&spepow );

	spedef = ( ( 2 * ppd->basic_spedef + spedef_rnd + spedef_exp / 4 ) * level / 100 + 5 );
	spedef = PokeChrAbiCalc( PokeSeikakuGet( pp ), spedef, ABILITY_SPEDEF );
	PokeParaPut( pp, ID_PARA_spedef, ( u8 * )&spedef );
	
	if( ( hp == 0 ) && ( oldhpmax != 0 ) ){
		;
	}
	else{
		if( monsno == MONSNO_NUKENIN ){
			hp = 1;
		}
		else if( hp == 0 ){
			hp = hpmax;
		}
		else{
			hp += ( hpmax - oldhpmax );
		}
	}

	if( hp ){
		PokeParaPut( pp, ID_PARA_hp, ( u8 * )&hp );
	}

	PokeParaFastModeOff( pp, flag );
}

//============================================================================================
/**
 *	ポケモンパラメータ構造体から任意で値を取得（暗号／復号／チェックサムチェックあり）
 *
 * @param[in]	pp	取得するポケモンパラメータ構造体のポインタ
 * @param[in]	id	取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]	buf	取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return		取得したデータ
 */
//============================================================================================
u32	PokeParaGet( const POKEMON_PARAM *pp, int id, void *buf )
{
	u32	ret;

	PokeParaDecodedAct( (POKEMON_PARAM*)pp );

	ret = PokeParaGetAct( (POKEMON_PARAM*)pp, id, buf );

	PokeParaCodedAct( (POKEMON_PARAM*)pp );

	return ret;
}

//============================================================================================
/**
 *	ポケモンパラメータ構造体に任意で値を格納（暗号／復号／チェックサムチェックあり）
 *
 * @param[in]	pp	格納したいポケモンパラメータ構造体のポインタ
 * @param[in]	id	格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	buf	格納したいデータのポインタ
 */
//============================================================================================
void	PokeParaPut( POKEMON_PARAM *pp, int id, const void *buf )
{
	if( PokeParaDecodedAct( pp ) == TRUE ){
		PokeParaPutAct( pp, id, buf );
	}

	PokeParaCodedAct( pp );
}

//============================================================================================
/**
 *	ポケモンパラメータ構造体に任意で値を加算（暗号／復号／チェックサムチェックあり）
 *
 * @param[in]	pp		加算したいポケモンパラメータ構造体のポインタ
 * @param[in]	id		加算したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	value	加算したいデータのポインタ
 */
//============================================================================================
void	PokeParaAdd( POKEMON_PARAM *pp, int id, int value )
{
	if( PokeParaDecodedAct( pp ) == TRUE ){
		PokeParaAddAct( pp, id, value );
	}

	PokeParaCodedAct( pp );
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体から任意で値を取得（暗号／復号／チェックサムチェックあり）
 *
 * @param[in]	pp	取得するボックスポケモンパラメータ構造体のポインタ
 * @param[in]	id	取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]	buf	取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return		取得したデータ
 */
//============================================================================================
u32	PokePasoParaGet( const POKEMON_PASO_PARAM *ppp, int id, void *buf )
{
	u32	ret;

	PokePasoParaDecodedAct( (POKEMON_PASO_PARAM*)ppp );

	ret = PokePasoParaGetAct( (POKEMON_PASO_PARAM*)ppp, id, buf );

	PokePasoParaCodedAct( (POKEMON_PASO_PARAM*)ppp );

	return ret;
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体に任意で値を格納（暗号／復号／チェックサムチェックあり）
 *
 * @param[in]	ppp	格納したいボックスポケモンパラメータ構造体のポインタ
 * @param[in]	id	格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	buf	格納したいデータのポインタ
 */
//============================================================================================
void	PokePasoParaPut( POKEMON_PASO_PARAM *ppp, int id, const void *buf )
{
	if( PokePasoParaDecodedAct( ppp ) == TRUE ){
		PokePasoParaPutAct( ppp, id, buf );
	}

	PokePasoParaCodedAct( ppp );
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体に任意で値を加算（暗号／復号／チェックサムチェックあり）
 *
 * @param[in]	ppp		加算したいボックスポケモンパラメータ構造体のポインタ
 * @param[in]	id		加算したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	value	加算したいデータのポインタ
 */
//============================================================================================
void	PokePasoParaAdd( POKEMON_PASO_PARAM *ppp, int id, int value )
{
	if( PokePasoParaDecodedAct( ppp ) == TRUE ){
		PokePasoParaAddAct( ppp, id, value );
	}

	PokePasoParaCodedAct( ppp );
}

//==============================================================================
/**
 * @brief   ＩＤと個性乱数からレアかどうかを判定する
 *
 * @param   id		ID
 * @param   rnd		個性乱数
 *
 * @retval  FALSE:レアじゃない	TRUE:レア
 */
//==============================================================================
BOOL	PokeRareCheck( u32 id, u32 rnd )
{
	return ( ( ( ( id & 0xffff0000 ) >> 16 ) ^ ( id & 0xffff ) ^ ( ( rnd & 0xffff0000 ) >> 16 ) ^ ( rnd & 0xffff ) ) < 8 );
}

//============================================================================================
/**
 *	ポケモンパラメータからポケモンの性別を取得
 *
 * @param[in]	pp		ポケモンパラメータ構造体
 *
 * @retval	PTL_SEX_MALE:♂　PTL_SEX_FEMALE:♀　PTL_SEX_UNK:性別不明
 */
//============================================================================================
u8	PokeSexGet( POKEMON_PARAM *pp )
{
	return ( PokePasoSexGet( ( POKEMON_PASO_PARAM * )&pp->ppp ) );
}
//============================================================================================
/**
 *	ボックスポケモンパラメータからポケモンの性別を取得
 *
 * @param[in]	pp		ポケモンパラメータ構造体
 *
 * @retval	PTL_SEX_MALE:♂　PTL_SEX_FEMALE:♀　PTL_SEX_UNK:性別不明
 */
//============================================================================================
u8	PokePasoSexGet( POKEMON_PASO_PARAM *ppp )
{
	u16	mons_no;
	int	form_no;
	u32	rnd;
	int	flag;

	flag = PokePasoParaFastModeOn( ppp );
	mons_no = PokePasoParaGet( ppp, ID_PARA_monsno, 0 );
	form_no = PokePasoParaGet( ppp, ID_PARA_form_no, 0 );
	rnd = PokePasoParaGet( ppp, ID_PARA_personal_rnd, 0 );
	PokePasoParaFastModeOff( ppp, flag );

	return	POKETOOL_GetSex( mons_no, form_no, rnd );
}
//============================================================================================
/**
 *	ポケモンの性別を取得（内部でパーソナルデータをロードする）
 *
 * @param[in]	mons_no	性別を取得するポケモンナンバー
 * @param[in]	form_no	性別を取得するポケモンのフォルムナンバー
 * @param[in]	rnd		性別を取得するポケモンの個性乱数
 *
 * @retval	PTL_SEX_MALE:♂　PTL_SEX_FEMALE:♀　PTL_SEX_UNK:性別不明
 */
//============================================================================================
u8 POKETOOL_GetSex( u16 mons_no, u16 form_no, u32 personal_rnd )
{
	u8 sex_param;

	POKE_PERSONAL_LoadData( mons_no, form_no, &PersonalDataWork );
	sex_param = POKE_PERSONAL_GetParam( &PersonalDataWork, POKE_PER_ID_sex );

// 性別固定のケース
	switch( sex_param ){
	case POKEPER_SEX_MALE:
		return PTL_SEX_MALE;
	case POKEPER_SEX_FEMALE:
		return PTL_SEX_FEMALE;
	case POKEPER_SEX_UNKNOWN:
		return PTL_SEX_UNKNOWN;
	}

// 個性乱数により性別が決まるケース
	if( sex_param > ( personal_rnd & 0xff ) )
	{
		return PTL_SEX_FEMALE;
	}
	else
	{
		return PTL_SEX_MALE;
	}
}



//============================================================================================
/**
 *	モンスターナンバーとレベルから技をセットする。
 *
 * @param[in]	pp	セットするポケモンデータ構造体のポインタ
 */
//============================================================================================
void	PokeWazaOboe( POKEMON_PARAM *pp )
{
	PokePasoWazaOboe( &pp->ppp );
}

//============================================================================================
/**
 *	モンスターナンバーとレベルから技をセットする。
 *
 * @param[in]	ppp	セットするポケモンデータ構造体のポインタ
 */
//============================================================================================
void	PokePasoWazaOboe( POKEMON_PASO_PARAM *ppp )
{
	static	u16	wot[ LEVELUPWAZA_OBOE_MAX ];	//技覚えテーブル

	BOOL	flag;
	int	i;
	u16	mons_no;
	u16	waza_no;
	int	form_no;
	u16	ret;
	u8	level;

	flag = PokePasoParaFastModeOn( ppp );

	mons_no = PokePasoParaGet( ppp, ID_PARA_monsno, 0 );
	form_no = PokePasoParaGet( ppp, ID_PARA_form_no, 0 );
	level = PokePasoLevelCalc( ppp );
	POKE_PERSONAL_LoadWazaOboeTable( mons_no, form_no, wot );

	i = 0;
	while( wot[ i ] != 0xffff ){
		if( ( wot[ i ] & 0xfe00 ) <= ( level << 9 ) ){
			waza_no = wot[ i ] & 0x1ff;
			ret = PokePasoWazaSet( ppp, waza_no );
			if( ret == NO_WAZA_SET ){
				PokePasoWazaOboeOshidashi( ppp, waza_no );
			}
		}
		else{
			break;
		}
		i++;
	}

	PokePasoParaFastModeOff( ppp, flag );
}

//============================================================================================
/**
 *	あいている場所に技を追加する。あいていない場合はその旨を返す。
 *
 *	@param[in]	pp		セットするポケモンパラメータ構造体のポインタ
 *	@param[in]	wazano	セットする技ナンバー
 *
 *	@retvl	wazano			正常終了
 *			SAME_WAZA_SET	すでに覚えている技と同じ技を覚えようとした
 *			NO_WAZA_SET		場所が空いていない
 */
//============================================================================================
u16	PokeWazaSet( POKEMON_PARAM *pp, u16 wazano )
{
	return PokePasoWazaSet( &pp->ppp, wazano );
}

//============================================================================================
/**
 *	あいている場所に技を追加する。あいていない場合はその旨を返す。
 *
 *	@param[in]	ppp		セットするポケモンパラメータ構造体のポインタ
 *	@param[in]	wazano	セットする技ナンバー
 *
 *	@retvl	wazano			正常終了
 *			SAME_WAZA_SET	すでに覚えている技と同じ技を覚えようとした
 *			NO_WAZA_SET		場所が空いていない
 */
//============================================================================================
u16	PokePasoWazaSet( POKEMON_PASO_PARAM *ppp, u16 wazano )
{
	int	i;
	u8	pp;
	u16	waza;
	u16	ret;
	BOOL	flag;

	ret = NO_WAZA_SET;

	flag = PokePasoParaFastModeOn( ppp );

	for( i = 0 ; i < 4 ; i++ ){
		if( ( waza = PokePasoParaGet( ppp, ID_PARA_waza1 + i, NULL ) ) == 0 ){
			PokePasoWazaSetPos( ppp, wazano, i );
			ret = wazano;
			break;
		}
		else{
			//同じ技を覚えちゃだめ
			if( waza == wazano ){
				ret = SAME_WAZA_SET;
				break;
			}
		}
	}
	
	PokePasoParaFastModeOff( ppp, flag );

	return	ret;
}

//============================================================================================
/**
 *	押し出し式に技を覚える。
 *
 * @param[in]	pp		セットするポケモンパラメータ構造体のポインタ
 * @param[in]	wazano	覚える技
 */
//============================================================================================
void	PokeWazaOboeOshidashi( POKEMON_PARAM *pp, u16 wazano )
{
	PokePasoWazaOboeOshidashi( &pp->ppp, wazano );
}

//============================================================================================
/**
 *	押し出し式に技を覚える。
 *
 * @param[in]	ppp		セットするポケモンパラメータ構造体のポインタ
 * @param[in]	wazano	覚える技
 */
//============================================================================================
void	PokePasoWazaOboeOshidashi( POKEMON_PASO_PARAM *ppp, u16 wazano )
{
	int	i;
	u16	waza[4];
	u8	pp[4];
	u8	ppcnt[4];
	BOOL	flag;

	flag = PokePasoParaFastModeOn( ppp );

	for( i = 0 ; i < 3 ; i++ ){
		waza[i] =	PokePasoParaGet( ppp, ID_PARA_waza2 + i,	 NULL );
		pp[i] =		PokePasoParaGet( ppp, ID_PARA_pp2 + i,		 NULL );
		ppcnt[i] =	PokePasoParaGet( ppp, ID_PARA_pp_count2 + i, NULL );
	}

	waza[3] = wazano;
	pp[3] = WT_WazaDataParaGet( wazano, ID_WTD_pp );
	ppcnt[3] = 0;

	for( i=0 ; i < 4 ; i++ ){
		PokePasoParaPut( ppp, ID_PARA_waza1 + i,	 ( u8 * )&waza[i] );
		PokePasoParaPut( ppp, ID_PARA_pp1 + i,		 ( u8 * )&pp[i] );
		PokePasoParaPut( ppp, ID_PARA_pp_count1 + i, ( u8 * )&ppcnt[i] );
	}

	PokePasoParaFastModeOff( ppp, flag );
}

//============================================================================================
/**
 *	場所を指定して技をセットする。
 *
 * @param[in]	pp		セットするポケモンパラメータ構造体のポインタ
 * @param[in]	wazano	セットする技ナンバー
 * @param[in]	pos		技をセットする場所
 */
//============================================================================================
void	PokeWazaSetPos( POKEMON_PARAM *pp, u16 wazano, u8 pos )
{
	PokePasoWazaSetPos( &pp->ppp, wazano, pos );
}

//============================================================================================
/**
 *	場所を指定して技をセットする。
 *
 * @param[in]	ppp		セットするポケモンパラメータ構造体のポインタ
 * @param[in]	wazano	セットする技ナンバー
 * @param[in]	pos		技をセットする場所
 */
//============================================================================================
void	PokePasoWazaSetPos( POKEMON_PASO_PARAM *ppp, u16 wazano, u8 pos )
{
	u8	pp;
	u8	pp_count;

	PokePasoParaPut( ppp, ID_PARA_waza1 + pos, ( u8 * )&wazano );
	pp_count = 0;
	PokePasoParaPut( ppp, ID_PARA_pp_count1 + pos, &pp_count );
	pp = WT_PPMaxGet( wazano, 0 );
	PokePasoParaPut( ppp, ID_PARA_pp1 + pos, ( u8 * )&pp );
}

//============================================================================================
/**
 *	ポケモンデータからポケモンのレベルを取得
 *
 * @param[in]	pp	取得したいポケモンパラメータ構造体のポインタ
 *
 * @return	取得したレベル
 */
//============================================================================================
u32	PokeParaLevelCalc( POKEMON_PARAM *pp )
{
	return PokePasoLevelCalc( &pp->ppp );
}
//============================================================================================
/**
 *	ボックスポケモンデータからポケモンのレベルを取得
 *
 * @param[in]	ppp	取得したいボックスポケモンパラメータ構造体のポインタ
 *
 * @return	取得したレベル
 */
//============================================================================================
u32	PokePasoLevelCalc( POKEMON_PASO_PARAM *ppp )
{
	int	mons_no;
	int form_no;
	u32	exp;
	BOOL	flag;

	flag = PokePasoParaFastModeOn( ppp );

	mons_no = PokePasoParaGet( ppp, ID_PARA_monsno, 0 );
	form_no = PokePasoParaGet( ppp, ID_PARA_form_no, 0 );
	exp = PokePasoParaGet( ppp, ID_PARA_exp, 0 );

	PokePasoParaFastModeOff( ppp, flag );

	return POKETOOL_CalcLevel( mons_no, form_no, exp );

}
//============================================================================================
/**
 *	ポケモンナンバー、経験値からポケモンのレベルを計算
 *	（内部でパーソナルデータをロードする）
 *
 * @param[in]	mons_no	取得したいポケモンナンバー
 * @param[in]	form_no	取得したいポケモンのフォルムナンバー
 * @param[in]	exp		取得したいポケモンの経験値
 *
 * @return	取得したレベル
 */
//============================================================================================
u32	POKETOOL_CalcLevel( u16 mons_no, u16 form_no, u32 exp )
{
	u16 growType, level;

	POKE_PERSONAL_LoadData( mons_no, form_no, &PersonalDataWork );
	growType = POKE_PERSONAL_GetParam( &PersonalDataWork, POKE_PER_ID_grow );
	load_grow_table( growType, &GrowTable[0] );

	for( level = 1 ; level < 101 ; level++ ){
		if( GrowTable[ level ] > exp ) break;
	}

	return level - 1;
}
//============================================================================================
/**
 *	ポケモンナンバーとレベルから、そのレベルになるための最小経験値を取得
 *
 * @param[in]	mons_no		取得するモンスターナンバー
 * @param[in]	form_no		取得するモンスターのフォルムナンバー
 * @param[in]	level		取得するレベル
 *
 * @retval	u32		指定レベルに達するための最小経験値
 */
//============================================================================================
u32	POKETOOL_GetMinExp( u16 mons_no, u16 form_no, u16 level )
{
	return	PokeGrowParaGet( PokePersonalParaGet(mons_no, form_no, POKE_PER_ID_grow), level );
}



//============================================================================================
/**
 *	ポケモンパラメータ構造体から経験値データを取得
 *
 * @param[in]	pp	ポケモンパラメータ構造体のポインタ
 */
//============================================================================================
u32	PokeParaLevelExpGet( POKEMON_PARAM *pp )
{
	return POKETOOL_GetMinExp( PokeParaGet( pp, ID_PARA_monsno, NULL ), 
							PokeParaGet( pp, ID_PARA_form_no, NULL ),
							PokeParaGet( pp, ID_PARA_level, NULL ) );
}
//============================================================================================
/**
 *	ポケモンの性格を取得（引数がPOKEMON_PARAM)
 *
 * @param[in]	pp	取得したいポケモンパラメータ構造体のポインタ
 *
 * @return	取得した性格
 */
//============================================================================================
u8	PokeSeikakuGet( POKEMON_PARAM *pp )
{
	return	PokePasoSeikakuGet( &pp->ppp );
}
//============================================================================================
/**
 *	ポケモンの性格を取得（引数がPOKEMON_PASO_PARAM)
 *
 * @param[in]	ppp	取得したいポケモンパラメータ構造体のポインタ
 *
 * @return	取得した性格
 */
//============================================================================================
u8	PokePasoSeikakuGet( POKEMON_PASO_PARAM *ppp )
{
	BOOL	flag;
	u32		rnd;

	flag = PokePasoParaFastModeOn( ppp );
	rnd = PokePasoParaGet( ppp, ID_PARA_personal_rnd, 0 );
	PokePasoParaFastModeOff( ppp, flag );

	return POKETOOL_GetSeikaku( rnd );
}
//============================================================================================
/**
 *	ポケモンの性格を取得
 *
 *	性格は、個性乱数を25で割った余りから算出される
 *
 * @param[in]	rnd	取得したい個性乱数
 *
 * @return	取得した性格
 */
//============================================================================================
u8 POKETOOL_GetSeikaku( u32 personal_rnd )
{
	return ( u8 )( personal_rnd % 25 );
}








//============================================================================================
/**
 *	ポケモンパラメータ構造体を復号してチェックサムをチェックして不正ならダメタマゴにする
 *
 * @param[in]	pp		復号するポケモンパラメータ構造体のポインタ
 *
 * @return		FALSE:不正データ　TRUE:正常データ
 */
//============================================================================================
static	BOOL	PokeParaDecodedAct( POKEMON_PARAM *pp )
{
	BOOL	ret = TRUE;
	if( pp->ppp.pp_fast_mode == 0 ){
		PokeParaDecoded( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
		ret = PokePasoParaDecodedAct( &pp->ppp );
	}
	return ret;
}
//============================================================================================
/**
 *	ポケモンパラメータ構造体を復号してチェックサムをチェックして不正ならダメタマゴにする
 *
 * @param[in]	ppp		復号するボックスポケモンパラメータ構造体のポインタ
 *
 * @return		FALSE:不正データ　TRUE:正常データ
 */
//============================================================================================
static	BOOL	PokePasoParaDecodedAct( POKEMON_PASO_PARAM *ppp )
{
	BOOL	ret = TRUE;
	u16		sum;

	if( ppp->ppp_fast_mode == 0 ){
		PokeParaDecoded( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
		sum = PokeParaMakeCheckSum( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
		if( sum != ppp->checksum ){
			GF_ASSERT_MSG( ( sum == ppp->checksum ), "checksum Crash!\n" );
			ppp->fusei_tamago_flag = 1;
			ret = FALSE;
		}
	}
	return ret;
}
//============================================================================================
/**
 *	ポケモンパラメータ構造体を暗号化する
 *
 * @param[in]	ppp		復号するボックスポケモンパラメータ構造体のポインタ
 *
 * @retval	なし
 */
//============================================================================================
static	void	PokeParaCodedAct( POKEMON_PARAM *pp )
{
	if( pp->ppp.pp_fast_mode == 0 ){
		PokeParaCoded( &pp->pcp, sizeof( POKEMON_CALC_PARAM ), pp->ppp.personal_rnd );
		PokePasoParaCodedAct( &pp->ppp );
	}
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体を暗号化する
 *
 * @param[in]	ppp		復号するボックスポケモンパラメータ構造体のポインタ
 *
 * @retval	なし
 */
//============================================================================================
static	void	PokePasoParaCodedAct( POKEMON_PASO_PARAM *ppp )
{
	if( ppp->ppp_fast_mode == 0 ){
		ppp->checksum = PokeParaMakeCheckSum( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
		PokeParaCoded( ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX, ppp->checksum );
	}
}

//============================================================================================
/**
 *	ポケモンパラメータ構造体から任意で値を取得
 *
 * @param[in]	pp	取得するポケモンパラメータ構造体のポインタ
 * @param[in]	id	取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]	buf	取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return		取得したデータ
 */
//============================================================================================
static	u32	PokeParaGetAct( POKEMON_PARAM *pp, int id, void *buf )
{
	u32	ret = 0;

//メールとカスタムボール対応がまだです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning MAIL_DATA and CB_CORE nothing
#endif
	switch( id ){
	case ID_PARA_condition:
		ret = pp->pcp.condition;
		break;
	case ID_PARA_level:
		ret = pp->pcp.level;
		break;
	case ID_PARA_cb_id:
		ret = pp->pcp.cb_id;
		break;
	case ID_PARA_hp:
		ret = pp->pcp.hp;
		break;
	case ID_PARA_hpmax:
		ret = pp->pcp.hpmax;
		break;
	case ID_PARA_pow:
		ret = pp->pcp.pow;
		break;
	case ID_PARA_def:
		ret = pp->pcp.def;
		break;
	case ID_PARA_agi:
		ret = pp->pcp.agi;
		break;
	case ID_PARA_spepow:
		ret = pp->pcp.spepow;
		break;
	case ID_PARA_spedef:
		ret = pp->pcp.spedef;
		break;
	case ID_PARA_mail_data:
//		MailData_Copy( &pp->pcp.mail_data, ( MAIL_DATA * )buf );
		ret = TRUE;
		break;
	case ID_PARA_cb_core:
//		CB_Tool_CoreData_Copy( &pp->pcp.cb_core, ( CB_CORE * )buf );
		ret = TRUE;
		break;
	default:
		ret = PokePasoParaGetAct( ( POKEMON_PASO_PARAM * )&pp->ppp, id, buf );
		break;
	}
	return	ret;
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体から任意で値を取得
 *
 * @param[in]	pp	取得するボックスポケモンパラメータ構造体のポインタ
 * @param[in]	id	取得したいデータのインデックス（poke_tool.hに定義）
 * @param[out]	buf	取得したいデータが配列の時に格納先のアドレスを指定
 *
 * @return		取得したデータ
 */
//============================================================================================
static	u32	PokePasoParaGetAct( POKEMON_PASO_PARAM *ppp, int id, void *buf )
{
	u32	ret = 0;
	u64	bit;
	POKEMON_PASO_PARAM1	*ppp1;
	POKEMON_PASO_PARAM2	*ppp2;
	POKEMON_PASO_PARAM3	*ppp3;
	POKEMON_PASO_PARAM4	*ppp4;

	ppp1 = ( POKEMON_PASO_PARAM1 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA1 );
	ppp2 = ( POKEMON_PASO_PARAM2 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA2 );
	ppp3 = ( POKEMON_PASO_PARAM3 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA3 );
	ppp4 = ( POKEMON_PASO_PARAM4 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA4 );

	switch( id ){
		default:
			ret = 0;
			break;
//PARAM
		case ID_PARA_personal_rnd:
			ret = ppp->personal_rnd;
			break;
		case ID_PARA_pp_fast_mode:
			ret = ppp->pp_fast_mode;
			break;
		case ID_PARA_ppp_fast_mode:
			ret = ppp->ppp_fast_mode;
			break;
		case ID_PARA_fusei_tamago_flag:
			ret = ppp->fusei_tamago_flag;
			break;
		case ID_PARA_checksum:
			ret = ppp->checksum;
			break;
		case ID_PARA_poke_exist:
			ret = ( ppp1->monsno != 0 );
			break;
		case ID_PARA_tamago_exist:
			if( ppp->fusei_tamago_flag ){
				ret = ppp->fusei_tamago_flag;
			}
			else{
				ret = ppp2->tamago_flag;
			}
			break;
		case ID_PARA_monsno_egg:
			ret = ppp1->monsno;
			if( ret == 0){
				break;
			}
			else if( ( ppp2->tamago_flag ) || ( ppp->fusei_tamago_flag ) ){
				ret = MONSNO_TAMAGO;
			}
			break;
		case ID_PARA_level:
			ret = POKETOOL_CalcLevel( ppp1->monsno, ppp2->form_no, ppp1->exp );
			break;
//PARAM1
		case ID_PARA_monsno:
			if( ppp->fusei_tamago_flag ){
				ret = MONSNO_TAMAGO;
			}
			else{
				ret = ppp1->monsno;
			}
			break;
		case ID_PARA_item:
			ret = ppp1->item;
			break;
		case ID_PARA_id_no:
			ret = ppp1->id_no;
			break;
		case ID_PARA_exp:
			ret = ppp1->exp;
			break;
		case ID_PARA_friend:
			ret = ppp1->friend;
			break;
		case ID_PARA_speabino:
			ret = ppp1->speabino;
			break;
		case ID_PARA_mark:
			ret = ppp1->mark;
			break;
		case ID_PARA_country_code:
			ret = ppp1->country_code;
			break;
		case ID_PARA_hp_exp:
			ret = ppp1->hp_exp;
			break;
		case ID_PARA_pow_exp:
			ret = ppp1->pow_exp;
			break;
		case ID_PARA_def_exp:
			ret = ppp1->def_exp;
			break;
		case ID_PARA_agi_exp:
			ret = ppp1->agi_exp;
			break;
		case ID_PARA_spepow_exp:
			ret = ppp1->spepow_exp;
			break;
		case ID_PARA_spedef_exp:
			ret = ppp1->spedef_exp;
			break;
		case ID_PARA_style:
			ret = ppp1->style;
			break;
		case ID_PARA_beautiful:
			ret = ppp1->beautiful;
			break;
		case ID_PARA_cute:
			ret = ppp1->cute;
			break;
		case ID_PARA_clever:
			ret = ppp1->clever;
			break;
		case ID_PARA_strong:
			ret = ppp1->strong;
			break;
		case ID_PARA_fur:
			ret = ppp1->fur;
			break;
		case ID_PARA_sinou_champ_ribbon:				//シンオウチャンプリボン
		case ID_PARA_sinou_battle_tower_ttwin_first:	//シンオウバトルタワータワータイクーン勝利1回目
		case ID_PARA_sinou_battle_tower_ttwin_second:	//シンオウバトルタワータワータイクーン勝利2回目
		case ID_PARA_sinou_battle_tower_2vs2_win50:		//シンオウバトルタワータワーダブル50連勝
		case ID_PARA_sinou_battle_tower_aimulti_win50:	//シンオウバトルタワータワーAIマルチ50連勝
		case ID_PARA_sinou_battle_tower_siomulti_win50:	//シンオウバトルタワータワー通信マルチ50連勝
		case ID_PARA_sinou_battle_tower_wifi_rank5:		//シンオウバトルタワーWifiランク５入り
		case ID_PARA_sinou_syakki_ribbon:				//シンオウしゃっきリボン
		case ID_PARA_sinou_dokki_ribbon:				//シンオウどっきリボン
		case ID_PARA_sinou_syonbo_ribbon:				//シンオウしょんぼリボン
		case ID_PARA_sinou_ukka_ribbon:					//シンオウうっかリボン
		case ID_PARA_sinou_sukki_ribbon:				//シンオウすっきリボン
		case ID_PARA_sinou_gussu_ribbon:				//シンオウぐっすリボン
		case ID_PARA_sinou_nikko_ribbon:				//シンオウにっこリボン
		case ID_PARA_sinou_gorgeous_ribbon:				//シンオウゴージャスリボン
		case ID_PARA_sinou_royal_ribbon:				//シンオウロイヤルリボン
		case ID_PARA_sinou_gorgeousroyal_ribbon:		//シンオウゴージャスロイヤルリボン
		case ID_PARA_sinou_ashiato_ribbon:				//シンオウあしあとリボン
		case ID_PARA_sinou_record_ribbon:				//シンオウレコードリボン
		case ID_PARA_sinou_history_ribbon:				//シンオウヒストリーリボン
		case ID_PARA_sinou_legend_ribbon:				//シンオウレジェンドリボン
		case ID_PARA_sinou_red_ribbon:					//シンオウレッドリボン
		case ID_PARA_sinou_green_ribbon:				//シンオウグリーンリボン
		case ID_PARA_sinou_blue_ribbon:					//シンオウブルーリボン
		case ID_PARA_sinou_festival_ribbon:				//シンオウフェスティバルリボン
		case ID_PARA_sinou_carnival_ribbon:				//シンオウカーニバルリボン
		case ID_PARA_sinou_classic_ribbon:				//シンオウクラシックリボン
		case ID_PARA_sinou_premiere_ribbon:				//シンオウプレミアリボン
		case ID_PARA_sinou_amari_ribbon:				//あまり
			bit = 1;
			ret = ( ( ppp1->sinou_ribbon & ( bit << id - ID_PARA_sinou_champ_ribbon ) ) != 0 );
			break;
//PARAM2
		case ID_PARA_waza1:
		case ID_PARA_waza2:
		case ID_PARA_waza3:
		case ID_PARA_waza4:
			ret = ppp2->waza[ id-ID_PARA_waza1 ];
			break;
		case ID_PARA_pp1:
		case ID_PARA_pp2:
		case ID_PARA_pp3:
		case ID_PARA_pp4:
			ret = ppp2->pp[ id-ID_PARA_pp1 ];
			break;
		case ID_PARA_pp_count1:
		case ID_PARA_pp_count2:
		case ID_PARA_pp_count3:
		case ID_PARA_pp_count4:
			ret = ppp2->pp_count[ id - ID_PARA_pp_count1 ];
			break;
		case ID_PARA_pp_max1:
		case ID_PARA_pp_max2:
		case ID_PARA_pp_max3:
		case ID_PARA_pp_max4:
			ret = WT_PPMaxGet( ppp2->waza[ id-ID_PARA_pp_max1 ], ppp2->pp_count[ id - ID_PARA_pp_max1 ] );
			break;
		case ID_PARA_hp_rnd:
			ret = ppp2->hp_rnd;
			break;
		case ID_PARA_pow_rnd:
			ret = ppp2->pow_rnd;
			break;
		case ID_PARA_def_rnd:
			ret = ppp2->def_rnd;
			break;
		case ID_PARA_agi_rnd:
			ret = ppp2->agi_rnd;
			break;
		case ID_PARA_spepow_rnd:
			ret = ppp2->spepow_rnd;
			break;
		case ID_PARA_spedef_rnd:
			ret = ppp2->spedef_rnd;
			break;
		case ID_PARA_tamago_flag:
			if( ppp->fusei_tamago_flag ){
				ret = ppp->fusei_tamago_flag;
			}
			else{
				ret = ppp2->tamago_flag;
			}
			break;
		case ID_PARA_nickname_flag:
			ret = ppp2->nickname_flag;
			break;
		case ID_PARA_stylemedal_normal:
		case ID_PARA_stylemedal_super:
		case ID_PARA_stylemedal_hyper:
		case ID_PARA_stylemedal_master:
		case ID_PARA_beautifulmedal_normal:
		case ID_PARA_beautifulmedal_super:
		case ID_PARA_beautifulmedal_hyper:
		case ID_PARA_beautifulmedal_master:
		case ID_PARA_cutemedal_normal:
		case ID_PARA_cutemedal_super:
		case ID_PARA_cutemedal_hyper:
		case ID_PARA_cutemedal_master:
		case ID_PARA_clevermedal_normal:
		case ID_PARA_clevermedal_super:
		case ID_PARA_clevermedal_hyper:
		case ID_PARA_clevermedal_master:
		case ID_PARA_strongmedal_normal:
		case ID_PARA_strongmedal_super:
		case ID_PARA_strongmedal_hyper:
		case ID_PARA_strongmedal_master:
		case ID_PARA_champ_ribbon:
		case ID_PARA_winning_ribbon:
		case ID_PARA_victory_ribbon:
		case ID_PARA_bromide_ribbon:
		case ID_PARA_ganba_ribbon:
		case ID_PARA_marine_ribbon:
		case ID_PARA_land_ribbon:
		case ID_PARA_sky_ribbon:
		case ID_PARA_country_ribbon:
		case ID_PARA_national_ribbon:
		case ID_PARA_earth_ribbon:
		case ID_PARA_world_ribbon:
			bit = 1;
			ret = ( ( ppp2->old_ribbon & ( bit << id - ID_PARA_stylemedal_normal ) ) !=0 );
			break;
		case ID_PARA_event_get_flag:
			ret = ppp2->event_get_flag;
			break;
		case ID_PARA_sex:
			//必ずパラメータから計算して返すようする
			ret = POKETOOL_GetSex( ppp1->monsno, ppp2->form_no, ppp->personal_rnd );
			//再計算したものを代入しておく
			ppp2->sex = ret;
			//チェックサムを再計算
			ppp->checksum = PokeParaMakeCheckSum( &ppp->paradata, sizeof( POKEMON_PASO_PARAM1 ) * POKE_PARA_BLOCK_MAX );
			break;
		case ID_PARA_form_no:
			ret = ppp2->form_no;
			break;
		case ID_PARA_dummy_p2_1:
			ret = ppp2->dummy_p2_1;
			break;
		case ID_PARA_dummy_p2_2:
			ret = ppp2->dummy_p2_2;
			break;
//PARAM3
		case ID_PARA_nickname:
			if( ppp->fusei_tamago_flag ){
//ダメタマゴ処理がないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning DAMETAMAGO Name Nothing
#endif
//				MSGDAT_MonsNameGet( MONSNO_DAMETAMAGO, HEAPID_BASE_SYSTEM, buf );
			}
			else{
				{
					u16	*buf16 = buf;

					for( ret = 0 ; ret < MONS_NAME_SIZE ; ret++ ){
						buf16[ ret ] = ppp3->nickname[ ret ];
					}
					buf16[ret] = EOM_;
				}
			}
			break;
		case ID_PARA_nickname_buf_flag:
			ret = ppp2->nickname_flag;
		case ID_PARA_nickname_buf:
//STRBUF処理がないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning STRBUF Nothing
#endif
			if( ppp->fusei_tamago_flag ){
//				STRBUF*  default_monsname = MSGDAT_UTIL_GetMonsName( MONSNO_DAMETAMAGO, HEAPID_BASE_SYSTEM );
//				STRBUF_Copy( ( STRBUF * )buf, default_monsname );
//				STRBUF_Delete( default_monsname );
			}
			else{
//				STRBUF_SetStringCode( ( STRBUF * )buf, ppp3->nickname );
			}
			break;

		case ID_PARA_pref_code:
			ret = ppp3->pref_code;
			break;
		case ID_PARA_get_cassette:
			ret = ppp3->get_cassette;
			break;
		case ID_PARA_trial_stylemedal_normal:
		case ID_PARA_trial_stylemedal_super:
		case ID_PARA_trial_stylemedal_hyper:
		case ID_PARA_trial_stylemedal_master:
		case ID_PARA_trial_beautifulmedal_normal:
		case ID_PARA_trial_beautifulmedal_super:
		case ID_PARA_trial_beautifulmedal_hyper:
		case ID_PARA_trial_beautifulmedal_master:
		case ID_PARA_trial_cutemedal_normal:
		case ID_PARA_trial_cutemedal_super:
		case ID_PARA_trial_cutemedal_hyper:
		case ID_PARA_trial_cutemedal_master:
		case ID_PARA_trial_clevermedal_normal:
		case ID_PARA_trial_clevermedal_super:
		case ID_PARA_trial_clevermedal_hyper:
		case ID_PARA_trial_clevermedal_master:
		case ID_PARA_trial_strongmedal_normal:
		case ID_PARA_trial_strongmedal_super:
		case ID_PARA_trial_strongmedal_hyper:
		case ID_PARA_trial_strongmedal_master:
		case ID_PARA_amari_ribbon:
			bit = 1;
			ret = ( ( ppp3->new_ribbon & ( bit << id - ID_PARA_trial_stylemedal_normal ) ) != 0 );
			break;
//PARAM4
		case ID_PARA_oyaname:
		{
			u16 *buf16 = buf;
			for( ret = 0 ; ret < PERSON_NAME_SIZE ; ret++ ){
				buf16[ ret ] = ppp4->oyaname[ ret ];
			}
			buf16[ ret ] = EOM_;
			break;
		}

		case ID_PARA_oyaname_buf:
//STRBUF処理がないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning STRBUF Nothing
#endif
//			STRBUF_SetStringCode( ( STRBUF * )buf, ppp4->oyaname );
			break;

		case ID_PARA_get_year:							//捕まえた年
			ret = ppp4->get_year;
			break;
		case ID_PARA_get_month:							//捕まえた月
			ret = ppp4->get_month;
			break;
		case ID_PARA_get_day:							//捕まえた日
			ret = ppp4->get_day;
			break;
		case ID_PARA_birth_year:						//生まれた年
			ret = ppp4->birth_year;
			break;
		case ID_PARA_birth_month:						//生まれた月
			ret = ppp4->birth_month;
			break;
		case ID_PARA_birth_day:							//生まれた日
			ret = ppp4->birth_day;
			break;
		case ID_PARA_get_place:							//捕まえた場所
		case ID_PARA_new_get_place:						//捕まえた場所
			//旧領域の値が「とおいばしょ」で、新領域の値が 0 以外なら、新領域の値を返す 
			if( ( ppp4->get_place == ID_TOOIBASYO ) && ( ppp2->new_get_place ) ){
				ret = ppp2->new_get_place;
			}
			//前項以外の場合、旧領域の値を返す
			else{
				ret = ppp4->get_place;
			}
			break;
		case ID_PARA_birth_place:						//生まれた場所
		case ID_PARA_new_birth_place:					//生まれた場所
			//旧領域の値が「とおいばしょ」で、新領域の値が 0 以外なら、新領域の値を返す 
			if( ( ppp4->birth_place == ID_TOOIBASYO ) && ( ppp2->new_birth_place ) ){
				ret = ppp2->new_birth_place;
			}
			//前項以外の場合、旧領域の値を返す
			else{
				ret = ppp4->birth_place;
			}
			break;
			break;
		case ID_PARA_pokerus:							//ポケルス
			ret = ppp4->pokerus;
			break;
		case ID_PARA_get_ball:							//捕まえたボール
			ret = ppp4->get_ball;	
			break;
		case ID_PARA_get_level:							//捕まえたレベル
			ret = ppp4->get_level;
			break;
		case ID_PARA_oyasex:							//親の性別
			ret = ppp4->oyasex;	
			break;
		case ID_PARA_get_ground_id:						//捕まえた場所の地形アトリビュート（ミノッチ用）
			ret = ppp4->get_ground_id;
			break;
		case ID_PARA_dummy_p4_1:						//あまり
			ret = ppp4->dummy_p4_1;
			break;

		case ID_PARA_power_rnd:
			ret = ( ppp2->hp_rnd		<<  0 )|
				  ( ppp2->pow_rnd		<<  5 )|
				  ( ppp2->def_rnd		<< 10 )|
				  ( ppp2->agi_rnd		<< 15 )|
				  ( ppp2->spepow_rnd	<< 20 )|
				  ( ppp2->spedef_rnd	<< 25 );
			break;
		//モンスターナンバーがニドランの時にnickname_flagが立っているかチェック
		case ID_PARA_nidoran_nickname:
			if( ( ( ppp1->monsno == MONSNO_NIDORAN_F ) || ( ppp1->monsno == MONSNO_NIDORAN_M ) ) &&
				  ( ppp2->nickname_flag == 0 ) ){
				ret = FALSE;
			}
			else{
				ret = TRUE;
			}
			break;
		case ID_PARA_type1:
		case ID_PARA_type2:
			if( ( ppp1->monsno == MONSNO_AUSU ) && ( ppp1->speabino == TOKUSYU_MARUTITAIPU ) ){
//アイテム処理系がないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning ITEM_PRM_EQUIP Nothing
#endif
//				ret = AusuTypeGet( ItemParamGet( ppp1->item, ITEM_PRM_EQUIP, HEAPID_BASE_SYSTEM ) );
			}
			else{
				ret = PokePersonalParaGet( ppp1->monsno, ppp2->form_no, POKE_PER_ID_type1 + ( id - ID_PARA_type1 ) );
			}
			break;
		case ID_PARA_default_name:						//ポケモンのデフォルト名
//MSGDAT処理がないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning MSGDAT Nothing
#endif
//			MSGDAT_MonsNameGet( ppp1->monsno, HEAPID_BASE_SYSTEM, buf );
			break;
	}

	return	ret;
}

//============================================================================================
/**
 *	ポケモンパラメータ構造体に任意で値を格納
 *
 * @param[in]	pp	格納したいポケモンパラメータ構造体のポインタ
 * @param[in]	id	格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	buf	格納したいデータのポインタ
 */
//============================================================================================
static	void	PokeParaPutAct( POKEMON_PARAM *pp, int id, const void *buf )
{
	u32	*buf32 = (u32 *)buf;
	u16	*buf16 = (u16 *)buf;
	u8	*buf8 = (u8 *)buf;

	switch( id ){
	case ID_PARA_condition:
		pp->pcp.condition = buf32[0];
		break;
	case ID_PARA_level:
		pp->pcp.level = buf8[0];
		break;
	case ID_PARA_cb_id:
		pp->pcp.cb_id = buf8[0];	
		break;
	case ID_PARA_hp:
		pp->pcp.hp = buf16[0];
		break;
	case ID_PARA_hpmax:
		pp->pcp.hpmax = buf16[0];
		break;
	case ID_PARA_pow:
		pp->pcp.pow = buf16[0];
		break;
	case ID_PARA_def:
		pp->pcp.def = buf16[0];
		break;
	case ID_PARA_agi:
		pp->pcp.agi = buf16[0];
		break;
	case ID_PARA_spepow:
		pp->pcp.spepow = buf16[0];
		break;
	case ID_PARA_spedef:
		pp->pcp.spedef = buf16[0];
		break;
	case ID_PARA_mail_data:
//メール処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning MAIL_DATA Nothing
#endif
//		MailData_Copy( ( MAIL_DATA * )buf, &pp->pcp.mail_data );
		break;
	case ID_PARA_cb_core:
//カスタムボール処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning CB_CORE Nothing
#endif
//		CB_Tool_CoreData_Copy( ( CB_CORE * )buf, &pp->pcp.cb_core );
		break;
	default:
		PokePasoParaPutAct( ( POKEMON_PASO_PARAM * )&pp->ppp, id, buf );
		break;
	}
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体に任意で値を格納
 *
 * @param[in]	ppp	格納したいボックスポケモンパラメータ構造体のポインタ
 * @param[in]	id	格納したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	buf	格納したいデータのポインタ
 */
//============================================================================================
static	void	PokePasoParaPutAct( POKEMON_PASO_PARAM *ppp, int id, const void *buf )
{
	int	i;
	u64	bit;
	u16	sum;
	u32	*buf32 = ( u32 * )buf;
	u16	*buf16 = ( u16 * )buf;
	u8	*buf8 = ( u8 * )buf;
	POKEMON_PASO_PARAM1	*ppp1;
	POKEMON_PASO_PARAM2	*ppp2;
	POKEMON_PASO_PARAM3	*ppp3;
	POKEMON_PASO_PARAM4	*ppp4;

	ppp1 = ( POKEMON_PASO_PARAM1 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA1 );
	ppp2 = ( POKEMON_PASO_PARAM2 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA2 );
	ppp3 = ( POKEMON_PASO_PARAM3 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA3 );
	ppp4 = ( POKEMON_PASO_PARAM4 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA4 );

	switch( id ){
//PARAM
		case ID_PARA_personal_rnd:
			ppp->personal_rnd = buf32[0];
			break;
		case ID_PARA_pp_fast_mode:
			GF_ASSERT_MSG( ( 0 ), "fast_modeへ不正な書き込み\n" );
			ppp->pp_fast_mode = buf8[0];
			break;
		case ID_PARA_ppp_fast_mode:
			GF_ASSERT_MSG( ( 0 ), "fast_modeへ不正な書き込み\n" );
			ppp->ppp_fast_mode = buf8[0];
			break;
		case ID_PARA_fusei_tamago_flag:
			ppp->fusei_tamago_flag = buf8[0];
			break;
		case ID_PARA_checksum:
			ppp->checksum = buf16[0];
			break;
//PARAM1
		case ID_PARA_monsno:
			ppp1->monsno = buf16[0];
			break;
		case ID_PARA_item:
			ppp1->item = buf16[0];
			break;
		case ID_PARA_id_no:
			ppp1->id_no = buf32[0];
			break;
		case ID_PARA_exp:
			ppp1->exp = buf32[0];
			break;
		case ID_PARA_friend:
			ppp1->friend = buf8[0];
			break;
		case ID_PARA_speabino:
			ppp1->speabino = buf8[0];
			break;
		case ID_PARA_mark:
			ppp1->mark = buf8[0];		
			break;
		case ID_PARA_country_code:
			ppp1->country_code = buf8[0];		
			break;
		case ID_PARA_hp_exp:
			ppp1->hp_exp = buf8[0];
			break;
		case ID_PARA_pow_exp:
			ppp1->pow_exp = buf8[0];
			break;
		case ID_PARA_def_exp:
			ppp1->def_exp = buf8[0];
			break;
		case ID_PARA_agi_exp:
			ppp1->agi_exp = buf8[0];
			break;
		case ID_PARA_spepow_exp:
			ppp1->spepow_exp = buf8[0];
			break;
		case ID_PARA_spedef_exp:
			ppp1->spedef_exp = buf8[0];
			break;
		case ID_PARA_style:
			ppp1->style = buf8[0];
			break;
		case ID_PARA_beautiful:
			ppp1->beautiful = buf8[0];
			break;
		case ID_PARA_cute:
			ppp1->cute = buf8[0];
			break;
		case ID_PARA_clever:
			ppp1->clever = buf8[0];
			break;
		case ID_PARA_strong:
			ppp1->strong = buf8[0];	
			break;
		case ID_PARA_fur:
			ppp1->fur = buf8[0];		
			break;
		case ID_PARA_sinou_champ_ribbon:				//シンオウチャンプリボン
		case ID_PARA_sinou_battle_tower_ttwin_first:	//シンオウバトルタワータワータイクーン勝利1回目
		case ID_PARA_sinou_battle_tower_ttwin_second:	//シンオウバトルタワータワータイクーン勝利2回目
		case ID_PARA_sinou_battle_tower_2vs2_win50:		//シンオウバトルタワータワーダブル50連勝
		case ID_PARA_sinou_battle_tower_aimulti_win50:	//シンオウバトルタワータワーAIマルチ50連勝
		case ID_PARA_sinou_battle_tower_siomulti_win50:	//シンオウバトルタワータワー通信マルチ50連勝
		case ID_PARA_sinou_battle_tower_wifi_rank5:		//シンオウバトルタワーWifiランク５入り
		case ID_PARA_sinou_syakki_ribbon:				//シンオウしゃっきリボン
		case ID_PARA_sinou_dokki_ribbon:				//シンオウどっきリボン
		case ID_PARA_sinou_syonbo_ribbon:				//シンオウしょんぼリボン
		case ID_PARA_sinou_ukka_ribbon:					//シンオウうっかリボン
		case ID_PARA_sinou_sukki_ribbon:				//シンオウすっきリボン
		case ID_PARA_sinou_gussu_ribbon:				//シンオウぐっすリボン
		case ID_PARA_sinou_nikko_ribbon:				//シンオウにっこリボン
		case ID_PARA_sinou_gorgeous_ribbon:				//シンオウゴージャスリボン
		case ID_PARA_sinou_royal_ribbon:				//シンオウロイヤルリボン
		case ID_PARA_sinou_gorgeousroyal_ribbon:		//シンオウゴージャスロイヤルリボン
		case ID_PARA_sinou_ashiato_ribbon:				//シンオウあしあとリボン
		case ID_PARA_sinou_record_ribbon:				//シンオウレコードリボン
		case ID_PARA_sinou_history_ribbon:				//シンオウヒストリーリボン
		case ID_PARA_sinou_legend_ribbon:				//シンオウレジェンドリボン
		case ID_PARA_sinou_red_ribbon:					//シンオウレッドリボン
		case ID_PARA_sinou_green_ribbon:				//シンオウグリーンリボン
		case ID_PARA_sinou_blue_ribbon:					//シンオウブルーリボン
		case ID_PARA_sinou_festival_ribbon:				//シンオウフェスティバルリボン
		case ID_PARA_sinou_carnival_ribbon:				//シンオウカーニバルリボン
		case ID_PARA_sinou_classic_ribbon:				//シンオウクラシックリボン
		case ID_PARA_sinou_premiere_ribbon:				//シンオウプレミアリボン
		case ID_PARA_sinou_amari_ribbon:				//あまり
			bit = 1 << ( id - ID_PARA_sinou_champ_ribbon );
			if( buf8[0] ){
				ppp1->sinou_ribbon |= bit;
			}
			else{
				ppp1->sinou_ribbon &= ( bit ^ 0xffffffff );
			}
			break;
//PARAM2
		case ID_PARA_waza1:
		case ID_PARA_waza2:
		case ID_PARA_waza3:
		case ID_PARA_waza4:
			ppp2->waza[ id - ID_PARA_waza1 ] = buf16[0];
			break;
		case ID_PARA_pp1:
		case ID_PARA_pp2:
		case ID_PARA_pp3:
		case ID_PARA_pp4:
			ppp2->pp[ id - ID_PARA_pp1 ] = buf8[0];
			break;
		case ID_PARA_pp_count1:
		case ID_PARA_pp_count2:
		case ID_PARA_pp_count3:
		case ID_PARA_pp_count4:
			ppp2->pp_count[ id - ID_PARA_pp_count1 ] = buf8[0];
			break;
		case ID_PARA_pp_max1:
		case ID_PARA_pp_max2:
		case ID_PARA_pp_max3:
		case ID_PARA_pp_max4:
			OS_Printf("不正な書き込み\n");
			break;
		case ID_PARA_hp_rnd:
			ppp2->hp_rnd = buf8[0];
			break;
		case ID_PARA_pow_rnd:
			ppp2->pow_rnd = buf8[0];
			break;
		case ID_PARA_def_rnd:
			ppp2->def_rnd = buf8[0];
			break;
		case ID_PARA_agi_rnd:
			ppp2->agi_rnd = buf8[0];
			break;
		case ID_PARA_spepow_rnd:
			ppp2->spepow_rnd = buf8[0];
			break;
		case ID_PARA_spedef_rnd:
			ppp2->spedef_rnd = buf8[0];
			break;
		case ID_PARA_tamago_flag:
			ppp2->tamago_flag = buf8[0];
			break;
		case ID_PARA_nickname_flag:
			ppp2->nickname_flag = buf8[0];
			break;
		case ID_PARA_stylemedal_normal:
		case ID_PARA_stylemedal_super:
		case ID_PARA_stylemedal_hyper:
		case ID_PARA_stylemedal_master:
		case ID_PARA_beautifulmedal_normal:
		case ID_PARA_beautifulmedal_super:
		case ID_PARA_beautifulmedal_hyper:
		case ID_PARA_beautifulmedal_master:
		case ID_PARA_cutemedal_normal:
		case ID_PARA_cutemedal_super:
		case ID_PARA_cutemedal_hyper:
		case ID_PARA_cutemedal_master:
		case ID_PARA_clevermedal_normal:
		case ID_PARA_clevermedal_super:
		case ID_PARA_clevermedal_hyper:
		case ID_PARA_clevermedal_master:
		case ID_PARA_strongmedal_normal:
		case ID_PARA_strongmedal_super:
		case ID_PARA_strongmedal_hyper:
		case ID_PARA_strongmedal_master:
		case ID_PARA_champ_ribbon:
		case ID_PARA_winning_ribbon:
		case ID_PARA_victory_ribbon:
		case ID_PARA_bromide_ribbon:
		case ID_PARA_ganba_ribbon:
		case ID_PARA_marine_ribbon:
		case ID_PARA_land_ribbon:
		case ID_PARA_sky_ribbon:
		case ID_PARA_country_ribbon:
		case ID_PARA_national_ribbon:
		case ID_PARA_earth_ribbon:
		case ID_PARA_world_ribbon:
			bit = 1 << ( id - ID_PARA_stylemedal_normal );
			if( buf8[0] ){
				ppp2->old_ribbon |= bit;
			}
			else{
				ppp2->old_ribbon &= ( bit ^ 0xffffffff );
			}
			break;
		case ID_PARA_event_get_flag:
			ppp2->event_get_flag = buf8[0];
			break;
		case ID_PARA_sex:
			//ppp2->sex=buf8[0];
			//必ずパラメータから計算して代入する
			ppp2->sex = POKETOOL_GetSex( ppp1->monsno, ppp2->form_no, ppp->personal_rnd );
			GF_ASSERT_MSG( ( buf8[0] == ppp2->sex ), "Disagreement personal_rnd <> ID_PARA_sex\n");
			break;
		case ID_PARA_form_no:
			ppp2->form_no = buf8[0];
			break;
		case ID_PARA_dummy_p2_1:
			ppp2->dummy_p2_1 = buf8[0];
			break;
		case ID_PARA_dummy_p2_2:
			ppp2->dummy_p2_2 = buf16[0];
			break;
//PARAM3
		case ID_PARA_nickname_code_flag:
			{
//メッセージ処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning MSGDAT Nothing
#endif
//				STRCODE	def_name[ MONS_NAME_SIZE + EOM_SIZE ];

				//デフォルト名と比較して、デフォルト名だったら、ニックネームフラグを落とす
				//海外版からきたポケモンは、ニックネームの変更が不可なので、国コードを見る必要はなし
//				MSGDAT_MonsNameGet( ppp1->monsno, HEAPID_BASE_SYSTEM, &def_name[0] );
//				ppp2->nickname_flag = PM_strcmp( def_name, &buf16[0] );
			}
		case ID_PARA_nickname:
			for( i = 0 ; i < NELEMS( ppp3->nickname ) ; i++ ){
				ppp3->nickname[i] = buf16[i];
			}
			break;
		case ID_PARA_nickname_buf_flag:
			{
//メッセージ処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning MSGDAT Nothing
#endif
//				STRCODE	def_name[ MONS_NAME_SIZE + EOM_SIZE ];
//				STRCODE	buf_name[ MONS_NAME_SIZE + EOM_SIZE ];

				//デフォルト名と比較して、デフォルト名だったら、ニックネームフラグを落とす
				//海外版からきたポケモンは、ニックネームの変更が不可なので、国コードを見る必要はなし
//				MSGDAT_MonsNameGet( ppp1->monsno, HEAPID_BASE_SYSTEM, &def_name[0] );
//				STRBUF_GetStringCode( ( STRBUF * )buf, &buf_name[0], NELEMS( buf_name ) );
//				ppp2->nickname_flag = PM_strcmp( def_name, buf_name );
			}
		case ID_PARA_nickname_buf:
//メッセージ処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning STRBUF Nothing
#endif
//			STRBUF_GetStringCode( ( STRBUF * )buf, ppp3->nickname, NELEMS( ppp3->nickname ) );
			break;
		case ID_PARA_pref_code:
			ppp3->pref_code = buf8[0];
			break;
		case ID_PARA_get_cassette:
			ppp3->get_cassette = buf8[0];
			break;
		case ID_PARA_trial_stylemedal_normal:
		case ID_PARA_trial_stylemedal_super:
		case ID_PARA_trial_stylemedal_hyper:
		case ID_PARA_trial_stylemedal_master:
		case ID_PARA_trial_beautifulmedal_normal:
		case ID_PARA_trial_beautifulmedal_super:
		case ID_PARA_trial_beautifulmedal_hyper:
		case ID_PARA_trial_beautifulmedal_master:
		case ID_PARA_trial_cutemedal_normal:
		case ID_PARA_trial_cutemedal_super:
		case ID_PARA_trial_cutemedal_hyper:
		case ID_PARA_trial_cutemedal_master:
		case ID_PARA_trial_clevermedal_normal:
		case ID_PARA_trial_clevermedal_super:
		case ID_PARA_trial_clevermedal_hyper:
		case ID_PARA_trial_clevermedal_master:
		case ID_PARA_trial_strongmedal_normal:
		case ID_PARA_trial_strongmedal_super:
		case ID_PARA_trial_strongmedal_hyper:
		case ID_PARA_trial_strongmedal_master:
		case ID_PARA_amari_ribbon:
			bit = 1 << ( id - ID_PARA_trial_stylemedal_normal );
			if( buf8[0] ){
				ppp3->new_ribbon |= bit;
			}
			else{
				ppp3->new_ribbon &= ( bit ^ 0xffffffffffffffff );
			}
			break;
//PARAM4
		case ID_PARA_oyaname:
			for( i = 0 ; i < NELEMS( ppp4->oyaname ) ; i++ )
				ppp4->oyaname[i] = buf16[i];
			break;
		case ID_PARA_oyaname_buf:
//メッセージ処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning STRBUF Nothing
#endif
//			STRBUF_GetStringCode( ( STRBUF * )buf, ppp4->oyaname, NELEMS( ppp4->oyaname ) );
			break;

		case ID_PARA_get_year:							//捕まえた年
			ppp4->get_year = buf8[0];
			break;
		case ID_PARA_get_month:							//捕まえた月
			ppp4->get_month = buf8[0];
			break;
		case ID_PARA_get_day:							//捕まえた日
			ppp4->get_day = buf8[0];
			break;
		case ID_PARA_birth_year:						//生まれた年
			ppp4->birth_year = buf8[0];
			break;
		case ID_PARA_birth_month:						//生まれた月
			ppp4->birth_month = buf8[0];
			break;
		case ID_PARA_birth_day:							//生まれた日
			ppp4->birth_day = buf8[0];
			break;
		case ID_PARA_get_place:							//捕まえた場所
		case ID_PARA_new_get_place:						//捕まえた場所
//処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning PlaceName_RangeCheckDP Nothing
#endif
#if 0
			//DPにもあった場所なら、新領域と旧領域にその地名IDを書き込む
			if( buf16[0] == 0 || PlaceName_RangeCheckDP( buf16[0] ) == TRUE ){
				//get_placeが0かどうかでタマゴで孵ったか、生体で取得したか判定しているので
				//get_placeの0セットはありにする
				ppp4->get_place = buf16[0];				//捕まえた場所
				ppp2->new_get_place = buf16[0];			//捕まえた場所
			}
			//DPになかった場所なら、新領域にその地名ID、旧領域に「とおいばしょ」を書き込む
			else{
				ppp4->get_place = ID_TOOIBASYO;			//捕まえた場所
				ppp2->new_get_place = buf16[0];			//捕まえた場所
			}
#endif
			break;
		case ID_PARA_birth_place:						//生まれた場所
		case ID_PARA_new_birth_place:					//生まれた場所
//処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning PlaceName_RangeCheckDP Nothing
#endif
#if 0
			//DPにもあった場所なら、新領域と旧領域にその地名IDを書き込む
			if( buf16[0] == 0 || PlaceName_RangeCheckDP( buf16[0] ) == TRUE ){
				//範囲外マップで捕まえたポケモンは「なぞのばしょ」と表示する為、
				//birth_placeの0セットもありにした。
				ppp4->birth_place = buf16[0];			//生まれた場所
				ppp2->new_birth_place = buf16[0];		//生まれた場所
			}
			//DPになかった場所なら、新領域にその地名ID、旧領域に「とおいばしょ」を書き込む
			else{
				ppp4->birth_place = ID_TOOIBASYO;		//生まれた場所
				ppp2->new_birth_place = buf16[0];		//生まれた場所
			}
#endif
			break;
		case ID_PARA_pokerus:							//ポケルス
			ppp4->pokerus = buf8[0];
			break;
		case ID_PARA_get_ball:							//捕まえたボール
			ppp4->get_ball = buf8[0];
			break;
		case ID_PARA_get_level:							//捕まえたレベル
			ppp4->get_level = buf8[0];
			break;
		case ID_PARA_oyasex:							//親の性別
			ppp4->oyasex = buf8[0];
			break;
		case ID_PARA_get_ground_id:						//捕まえた場所の地形アトリビュート（ミノッチ用）
			ppp4->get_ground_id = buf8[0];
			break;
		case ID_PARA_dummy_p4_1:						//あまり
			ppp4->dummy_p4_1 = buf16[0];
			break;

		case ID_PARA_power_rnd:
			ppp2->hp_rnd	 = ( buf32[0] >>  0 ) & 0x0000001f ;
			ppp2->pow_rnd	 = ( buf32[0] >>  5 ) & 0x0000001f ;
			ppp2->def_rnd	 = ( buf32[0] >> 10 ) & 0x0000001f ;
			ppp2->agi_rnd	 = ( buf32[0] >> 15 ) & 0x0000001f ;
			ppp2->spepow_rnd = ( buf32[0] >> 20 ) & 0x0000001f ;
			ppp2->spedef_rnd = ( buf32[0] >> 25 ) & 0x0000001f ;
			break;
		//モンスターナンバーがニドランの時にnickname_flagが立っているかチェック
		case ID_PARA_nidoran_nickname:
		case ID_PARA_type1:
		case ID_PARA_type2:
			OS_Printf("不正な書き込み\n");
			break;
		case ID_PARA_default_name:						//ポケモンのデフォルト名
			{
//メッセージ処理ないです
#ifdef DEBUG_ONLY_FOR_sogabe
#warning STRBUF Nothing
#endif
//				STRBUF	*name_buf;

//				name_buf = MSGDAT_UTIL_GetMonsName( ppp1->monsno, HEAPID_BASE_SYSTEM );
//				STRBUF_GetStringCode( name_buf, ppp3->nickname, NELEMS( ppp3->nickname ) );
//				STRBUF_Delete( name_buf );
			}
			break;
	}
}

//============================================================================================
/**
 *	ポケモンパラメータ構造体に任意で値を加算
 *
 * @param[in]	pp		加算したいポケモンパラメータ構造体のポインタ
 * @param[in]	id		加算したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	value	加算したいデータのポインタ
 */
//============================================================================================
static	void	PokeParaAddAct( POKEMON_PARAM *pp, int id, int value )
{
	switch( id ){
		case ID_PARA_hp:
			if( ( pp->pcp.hp + value ) > pp->pcp.hpmax ){
				pp->pcp.hp = pp->pcp.hpmax;
			}
			else if( ( pp->pcp.hp + value ) < 0 ){
				pp->pcp.hp = 0;
			}
			else{
				pp->pcp.hp += value;
			}
			break;
		case ID_PARA_condition:
		case ID_PARA_level:
		case ID_PARA_cb_id:
		case ID_PARA_hpmax:
		case ID_PARA_pow:
		case ID_PARA_def:
		case ID_PARA_agi:
		case ID_PARA_spepow:
		case ID_PARA_spedef:
		case ID_PARA_mail_data:
			GF_ASSERT_MSG( (0), "Addできないパラメータです\n" );
			break;
		default:
			PokePasoParaAddAct( ( POKEMON_PASO_PARAM * )&pp->ppp, id, value );
			break;
	}
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体に任意で値を加算
 *
 * @param[in]	ppp		加算したいボックスポケモンパラメータ構造体のポインタ
 * @param[in]	id		加算したいデータのインデックス（poke_tool.hに定義）
 * @param[in]	value	加算したいデータのポインタ
 */
//============================================================================================
static	void	PokePasoParaAddAct( POKEMON_PASO_PARAM *ppp, int id, int value )
{
	int	i;
	u16	sum;
	POKEMON_PASO_PARAM1	*ppp1;
	POKEMON_PASO_PARAM2	*ppp2;
	POKEMON_PASO_PARAM3	*ppp3;
	POKEMON_PASO_PARAM4	*ppp4;

	ppp1 = ( POKEMON_PASO_PARAM1 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA1 );
	ppp2 = ( POKEMON_PASO_PARAM2 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA2 );
	ppp3 = ( POKEMON_PASO_PARAM3 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA3 );
	ppp4 = ( POKEMON_PASO_PARAM4 * )PokeParaAdrsGet( ppp, ppp->personal_rnd, ID_POKEPARA4 );

	switch( id ){
		case ID_PARA_exp:
			{
				u32 max_exp = POKETOOL_GetMinExp( ppp1->monsno, ppp2->form_no, POKE_LEVEL_MAX );
				if( ( ppp1->exp + value ) > max_exp )
				{
					ppp1->exp = max_exp;
				}
				else
				{
					ppp1->exp += value;
				}
			}
			break;
		case ID_PARA_friend:
			ParaAddCalc( &ppp1->friend, value, FRIEND_MAX );
			break;
		case ID_PARA_hp_exp:
		case ID_PARA_pow_exp:
		case ID_PARA_def_exp:
		case ID_PARA_agi_exp:
		case ID_PARA_spepow_exp:
		case ID_PARA_spedef_exp:
			GF_ASSERT_MSG( ( 0 ), "不正な加算：努力値は、合計値の上限が決まっているので、個別の加算はサポートしません\n");
			break;
		case ID_PARA_style:
			ParaAddCalc( &ppp1->style, value, STYLE_MAX );
			break;
		case ID_PARA_beautiful:
			ParaAddCalc( &ppp1->beautiful, value, BEAUTIFUL_MAX );
			break;
		case ID_PARA_cute:
			ParaAddCalc( &ppp1->cute, value, CUTE_MAX );
			break;
		case ID_PARA_clever:
			ParaAddCalc( &ppp1->clever, value, CLEVER_MAX );
			break;
		case ID_PARA_strong:
			ParaAddCalc( &ppp1->strong, value, STRONG_MAX );
			break;
		case ID_PARA_fur:
			ParaAddCalc( &ppp1->fur, value, FUR_MAX );
			break;
		case ID_PARA_pp1:
		case ID_PARA_pp2:
		case ID_PARA_pp3:
		case ID_PARA_pp4:
			ParaAddCalc( &ppp2->pp[ id - ID_PARA_pp1 ],
						 value,
						 WT_PPMaxGet( ppp2->waza[ id - ID_PARA_pp1 ], ppp2->pp_count[ id - ID_PARA_pp1 ] ) );
			break;
		case ID_PARA_pp_count1:
		case ID_PARA_pp_count2:
		case ID_PARA_pp_count3:
		case ID_PARA_pp_count4:
			ParaAddCalc( &ppp2->pp_count[ id - ID_PARA_pp_count1 ], value, PP_COUNT_MAX );
			break;
		case ID_PARA_pp_max1:
		case ID_PARA_pp_max2:
		case ID_PARA_pp_max3:
		case ID_PARA_pp_max4:
		case ID_PARA_hp_rnd:
		case ID_PARA_pow_rnd:
		case ID_PARA_def_rnd:
		case ID_PARA_agi_rnd:
		case ID_PARA_spepow_rnd:
		case ID_PARA_spedef_rnd:
		case ID_PARA_personal_rnd:
		case ID_PARA_pp_fast_mode:
		case ID_PARA_ppp_fast_mode:
		case ID_PARA_checksum:
		case ID_PARA_monsno:
		case ID_PARA_item:
		case ID_PARA_id_no:
		case ID_PARA_speabino:
		case ID_PARA_mark:
		case ID_PARA_country_code:
		case ID_PARA_sinou_champ_ribbon:				//シンオウチャンプリボン
		case ID_PARA_sinou_battle_tower_ttwin_first:	//シンオウバトルタワータワータイクーン勝利1回目
		case ID_PARA_sinou_battle_tower_ttwin_second:	//シンオウバトルタワータワータイクーン勝利2回目
		case ID_PARA_sinou_battle_tower_2vs2_win50:		//シンオウバトルタワータワーダブル50連勝
		case ID_PARA_sinou_battle_tower_aimulti_win50:	//シンオウバトルタワータワーAIマルチ50連勝
		case ID_PARA_sinou_battle_tower_siomulti_win50:	//シンオウバトルタワータワー通信マルチ50連勝
		case ID_PARA_sinou_battle_tower_wifi_rank5:		//シンオウバトルタワーWifiランク５入り
		case ID_PARA_sinou_syakki_ribbon:				//シンオウしゃっきリボン
		case ID_PARA_sinou_dokki_ribbon:				//シンオウどっきリボン
		case ID_PARA_sinou_syonbo_ribbon:				//シンオウしょんぼリボン
		case ID_PARA_sinou_ukka_ribbon:					//シンオウうっかリボン
		case ID_PARA_sinou_sukki_ribbon:				//シンオウすっきリボン
		case ID_PARA_sinou_gussu_ribbon:				//シンオウぐっすリボン
		case ID_PARA_sinou_nikko_ribbon:				//シンオウにっこリボン
		case ID_PARA_sinou_gorgeous_ribbon:				//シンオウゴージャスリボン
		case ID_PARA_sinou_royal_ribbon:				//シンオウロイヤルリボン
		case ID_PARA_sinou_gorgeousroyal_ribbon:		//シンオウゴージャスロイヤルリボン
		case ID_PARA_sinou_ashiato_ribbon:				//シンオウあしあとリボン
		case ID_PARA_sinou_record_ribbon:				//シンオウレコードリボン
		case ID_PARA_sinou_history_ribbon:				//シンオウヒストリーリボン
		case ID_PARA_sinou_legend_ribbon:				//シンオウレジェンドリボン
		case ID_PARA_sinou_red_ribbon:					//シンオウレッドリボン
		case ID_PARA_sinou_green_ribbon:				//シンオウグリーンリボン
		case ID_PARA_sinou_blue_ribbon:					//シンオウブルーリボン
		case ID_PARA_sinou_festival_ribbon:				//シンオウフェスティバルリボン
		case ID_PARA_sinou_carnival_ribbon:				//シンオウカーニバルリボン
		case ID_PARA_sinou_classic_ribbon:				//シンオウクラシックリボン
		case ID_PARA_sinou_premiere_ribbon:				//シンオウプレミアリボン
		case ID_PARA_sinou_amari_ribbon:				//あまり
		case ID_PARA_waza1:
		case ID_PARA_waza2:
		case ID_PARA_waza3:
		case ID_PARA_waza4:
		case ID_PARA_tamago_flag:
		case ID_PARA_nickname_flag:
		case ID_PARA_stylemedal_normal:
		case ID_PARA_stylemedal_super:
		case ID_PARA_stylemedal_hyper:
		case ID_PARA_stylemedal_master:
		case ID_PARA_beautifulmedal_normal:
		case ID_PARA_beautifulmedal_super:
		case ID_PARA_beautifulmedal_hyper:
		case ID_PARA_beautifulmedal_master:
		case ID_PARA_cutemedal_normal:
		case ID_PARA_cutemedal_super:
		case ID_PARA_cutemedal_hyper:
		case ID_PARA_cutemedal_master:
		case ID_PARA_clevermedal_normal:
		case ID_PARA_clevermedal_super:
		case ID_PARA_clevermedal_hyper:
		case ID_PARA_clevermedal_master:
		case ID_PARA_strongmedal_normal:
		case ID_PARA_strongmedal_super:
		case ID_PARA_strongmedal_hyper:
		case ID_PARA_strongmedal_master:
		case ID_PARA_champ_ribbon:
		case ID_PARA_winning_ribbon:
		case ID_PARA_victory_ribbon:
		case ID_PARA_bromide_ribbon:
		case ID_PARA_ganba_ribbon:
		case ID_PARA_marine_ribbon:
		case ID_PARA_land_ribbon:
		case ID_PARA_sky_ribbon:
		case ID_PARA_country_ribbon:
		case ID_PARA_national_ribbon:
		case ID_PARA_earth_ribbon:
		case ID_PARA_world_ribbon:
		case ID_PARA_event_get_flag:
		case ID_PARA_sex:
		case ID_PARA_form_no:
		case ID_PARA_dummy_p2_1:
		case ID_PARA_dummy_p2_2:
		case ID_PARA_nickname:
		case ID_PARA_nickname_buf:
		case ID_PARA_nickname_buf_flag:
		case ID_PARA_pref_code:
		case ID_PARA_get_cassette:
		case ID_PARA_trial_stylemedal_normal:
		case ID_PARA_trial_stylemedal_super:
		case ID_PARA_trial_stylemedal_hyper:
		case ID_PARA_trial_stylemedal_master:
		case ID_PARA_trial_beautifulmedal_normal:
		case ID_PARA_trial_beautifulmedal_super:
		case ID_PARA_trial_beautifulmedal_hyper:
		case ID_PARA_trial_beautifulmedal_master:
		case ID_PARA_trial_cutemedal_normal:
		case ID_PARA_trial_cutemedal_super:
		case ID_PARA_trial_cutemedal_hyper:
		case ID_PARA_trial_cutemedal_master:
		case ID_PARA_trial_clevermedal_normal:
		case ID_PARA_trial_clevermedal_super:
		case ID_PARA_trial_clevermedal_hyper:
		case ID_PARA_trial_clevermedal_master:
		case ID_PARA_trial_strongmedal_normal:
		case ID_PARA_trial_strongmedal_super:
		case ID_PARA_trial_strongmedal_hyper:
		case ID_PARA_trial_strongmedal_master:
		case ID_PARA_amari_ribbon:
		case ID_PARA_oyaname:
		case ID_PARA_oyaname_buf:
		case ID_PARA_get_year:							//捕まえた年
		case ID_PARA_get_month:							//捕まえた月
		case ID_PARA_get_day:							//捕まえた日
		case ID_PARA_birth_year:						//生まれた年
		case ID_PARA_birth_month:						//生まれた月
		case ID_PARA_birth_day:							//生まれた日
		case ID_PARA_get_place:							//捕まえた場所
		case ID_PARA_birth_place:						//生まれた場所
		case ID_PARA_pokerus:							//ポケルス
		case ID_PARA_get_ball:							//捕まえたボール
		case ID_PARA_get_level:							//捕まえたレベル
		case ID_PARA_oyasex:							//親の性別
		case ID_PARA_get_ground_id:						//捕まえた場所の地形アトリビュート（ミノッチ用）
		case ID_PARA_dummy_p4_1:						//あまり
		case ID_PARA_power_rnd:
		case ID_PARA_nidoran_nickname:
		case ID_PARA_type1:
		case ID_PARA_type2:
		case ID_PARA_default_name:						//ポケモンのデフォルト名
		default:
			GF_ASSERT_MSG( ( 0 ), "不正な加算\n" );
			break;
	}
}

//============================================================================================
/**
 *	上限下限も考慮して計算
 *
 * @param[in]	data	加算したいワークへのポインタ
 * @param[in]	value	加算したいデータ
 * @param[in]	value	加算するときの上限値
 */
//============================================================================================
static	void	ParaAddCalc( u8 *data, int value, int max )
{
	int	work=*data;

	if( ( work + value ) > max ){
		work = max;
	}
	if( ( work + value ) < 0 ){
		work = 0;
	}
	else{
		work += value;
	}

	*data = ( u8 )work;
}

//============================================================================================
/**
 *	暗号処理
 *
 * @param[in]	data	暗号化するデータのポインタ
 * @param[in]	size	暗号化するデータのサイズ
 * @param[in]	code	暗号化キーの初期値
 */
//============================================================================================
static	void	PokeParaCoded( void *data, u32 size, u32 code )
{
	int	i;
	u16	*data_p = ( u16 * )data;

	//暗号は、乱数暗号キーでマスク
	for( i = 0 ; i < size / 2 ; i++ ){
		data_p[i] ^= CodeRand( &code );
	}
}

//============================================================================================
/**
 *	復号処理
 *
 * @param[in]	data	復号するデータのポインタ
 * @param[in]	size	復号するデータのサイズ
 * @param[in]	code	暗号化キーの初期値
 */
//============================================================================================
//static	void	PokeParaDecoded( void *data, u32 size, u32 code )
//{
//	PokeParaCoded( data, size, code );
//}

//============================================================================================
/**
 *	乱数暗号キー生成ルーチン
 *
 * @param[in,out]	code	暗号キー格納ワークのポインタ
 *
 * @return	暗号キー格納ワークの上位2バイトを暗号キーとして返す
 */
//============================================================================================
static	u16	CodeRand( u32 *code )
{
    code[0] = code[0] *1103515245L + 24691;
    return ( u16 )( code[0] / 65536L ) ;
}

//============================================================================================
/**
 *	ポケモンパラメータのチェックサムを生成
 *
 * @param[in]	data	ポケモンパラメータ構造体のポインタ
 * @param[in]	size	チェックサムを生成するデータのサイズ
 *
 * @return	生成したチェックサム
 */
//============================================================================================
static	u16	PokeParaMakeCheckSum( void *data, u32 size )
{
	int	i;
	u16	*data_p = ( u16 * )data;
	u16	sum = 0;

	for( i = 0 ; i < size / 2 ; i++ ){
		sum += data_p[i];
	}

	return sum;
}

//============================================================================================
/**
 *	性格によるパラメータ変化値テーブル
 */
//============================================================================================
static	const	s8	SeikakuAbiTbl[][5]={
//		pow	def	agi	spepow	spedef
	{	 0,	 0,	 0,	 0,		 0},		//がんばりや
	{	 1,	-1,	 0,	 0,		 0},		//さみしがり
	{	 1,	 0,	-1,	 0,		 0},		//ゆうかん
	{	 1,	 0,	 0,	-1,		 0},		//いじっぱり
	{	 1,	 0,	 0,	 0,		-1},		//やんちゃ
	{	-1,	 1,	 0,	 0,		 0},		//ずぶとい
	{	 0,	 0,	 0,	 0,		 0},		//すなお
	{	 0,	 1,	-1,	 0,		 0},		//のんき
	{	 0,	 1,	 0,	-1,		 0},		//わんぱく
	{	 0,	 1,	 0,	 0,		-1},		//のうてんき
	{	-1,	 0,	 1,	 0,		 0},		//おくびょう
	{	 0,	-1,	 1,	 0,		 0},		//せっかち
	{	 0,	 0,	 0,	 0,		 0},		//まじめ
	{	 0,	 0,	 1,	-1,		 0},		//ようき
	{	 0,	 0,	 1,	 0,		-1},		//むじゃき
	{	-1,	 0,	 0,	 1,		 0},		//ひかえめ
	{	 0,	-1,	 0,	 1,		 0},		//おっとり
	{	 0,	 0,	-1,	 1,		 0},		//れいせい
	{	 0,	 0,	 0,	 0,		 0},		//てれや
	{	 0,	 0,	 0,	 1,		-1},		//うっかりや
	{	-1,	 0,	 0,	 0,		 1},		//おだやか
	{	 0,	-1,	 0,	 0,		 1},		//おとなしい
	{	 0,	 0,	-1,	 0,		 1},		//なまいき
	{	 0,	 0,	 0,	-1,		 1},		//しんちょう
	{	 0,	 0,	 0,	 0,		 0},		//きまぐれ
};

//============================================================================================
/**
 *	性格によるパラメータ変化計算ルーチン
 *
 * @param[in]	chr		計算する性格
 * @param[in]	para	計算するパラメータ値
 * @param[in]	cond	取り出すパラメータ変化値テーブルのインデックス（poke_tool.hに定義）
 *
 * @return	計算結果
 */
//============================================================================================
static	u16	PokeChrAbiCalc( u8 chr, u16 para, u8 cond)
{
	u16	ret;

	if( ( cond < ABILITY_POW ) || ( cond > ABILITY_SPEDEF ) ) return para;

	switch( SeikakuAbiTbl[ chr ][ cond - 1 ]){
		case 1:
			ret = para * 110;
			ret /= 100;
			break;
		case -1:
			ret = para * 90;
			ret /= 100;
			break;
		default:
			ret = para;
			break;
	}
	return	ret;
}

//============================================================================================
/**
 *	ポケモン成長テーブルデータを取得
 *
 * @param[in]	para		取得する成長テーブルのインデックス（0～7）
 * @param[out]	GrowTable	取得した成長テーブルの格納先
 */
//============================================================================================
static	void	load_grow_table( int para, u32 *GrowTable )
{
	GF_ASSERT_MSG( para < 8, "load_grow_table:TableIndexOver!\n" );
	GFL_ARC_LoadData( GrowTable, ARCID_GROW_TBL, para );
}

//============================================================================================
/**
 *	ポケモン成長テーブルデータから任意で経験値データを取得
 *
 * @param[in]	para		取得する成長テーブルのインデックス（0～7）
 * @param[in]	level		取得する成長テーブルのレベル（0～100）
 *
 * @return	取得した経験値データ
 */
//============================================================================================
static	u32	PokeGrowParaGet( int para, int level )
{
	GF_ASSERT_MSG( para < 8, "PokeGrowParaGet:TableIndexOver!\n" );
	GF_ASSERT_MSG( level <= 101, "PokeGrowParaGet:Level Over!\n" );

	load_grow_table( para, &GrowTable[0] );

	return GrowTable[ level ];
}

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体アドレス取得用テーブル値
 */
//============================================================================================

#define	POS1	( sizeof( POKEMON_PASO_PARAM1 ) * 0 )
#define	POS2	( sizeof( POKEMON_PASO_PARAM1 ) * 1 )
#define	POS3	( sizeof( POKEMON_PASO_PARAM1 ) * 2 )
#define	POS4	( sizeof( POKEMON_PASO_PARAM1 ) * 3 )

//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体内のメンバのアドレスを取得
 *
 * @param[in]	ppp	取得したいボックスポケモンパラメータ構造体のポインタ
 * @param[in]	rnd	構造体入れ替えのキー
 * @param[in]	id	取り出したいメンバのインデックス（poke_tool.hに定義）
 *
 * @return	取得したアドレス
 */
//============================================================================================
static	void	*PokeParaAdrsGet( POKEMON_PASO_PARAM *ppp, u32 rnd, u8 id )
{
	void	*ret;
	static	const u8 PokeParaAdrsTbl[32][4] =
		{
			{ POS1, POS2, POS3, POS4 },
			{ POS1, POS2, POS4, POS3 },
			{ POS1, POS3, POS2, POS4 },
			{ POS1, POS4, POS2, POS3 },
			{ POS1, POS3, POS4, POS2 },
			{ POS1, POS4, POS3, POS2 },
			{ POS2, POS1, POS3, POS4 },
			{ POS2, POS1, POS4, POS3 },
			{ POS3, POS1, POS2, POS4 },
			{ POS4, POS1, POS2, POS3 },
			{ POS3, POS1, POS4, POS2 },
			{ POS4, POS1, POS3, POS2 },
			{ POS2, POS3, POS1, POS4 },
			{ POS2, POS4, POS1, POS3 },
			{ POS3, POS2, POS1, POS4 },
			{ POS4, POS2, POS1, POS3 },
			{ POS3, POS4, POS1, POS2 },
			{ POS4, POS3, POS1, POS2 },
			{ POS2, POS3, POS4, POS1 },
			{ POS2, POS4, POS3, POS1 },
			{ POS3, POS2, POS4, POS1 },
			{ POS4, POS2, POS3, POS1 },
			{ POS3, POS4, POS2, POS1 },
			{ POS4, POS3, POS2, POS1 },
			{ POS1, POS2, POS3, POS4 },
			{ POS1, POS2, POS4, POS3 },
			{ POS1, POS3, POS2, POS4 },
			{ POS1, POS4, POS2, POS3 },
			{ POS1, POS3, POS4, POS2 },
			{ POS1, POS4, POS3, POS2 },
			{ POS2, POS1, POS3, POS4 },
			{ POS2, POS1, POS4, POS3 },
		};

	rnd = ( rnd & 0x0003e000 ) >> 13;

	GF_ASSERT_MSG( rnd <= ID_POKEPARADATA62, "PokeParaAdrsGet:RND Index Over!" );
	GF_ASSERT_MSG( id <= ID_POKEPARA4, "PokeParaAdrsGet:ID Index Over!" );

	ret = &ppp->paradata[ PokeParaAdrsTbl[ rnd ][ id ] ];

	return	ret;
}

//============================================================================================
/**
 * ポケモンパーソナル操作関数系
 */
//============================================================================================
//============================================================================================
/**
 *	ポケモンパーソナル構造体データから任意でデータを取得
 *  （一発でデータを取り出したいときに使用してください、
 *    パーソナルデータをたくさん取り出したいときはOpen,Get,Closeした方が軽いです）
 *  
 * @param[in]	mons_no	取得したいポケモンナンバー
 * @param[in]	form_no	取得したいポケモンのフォルムナンバー
 * @param[in]	para	取得したデータのインデックス（poke_tool.hに定義）
 *
 * @return	取得したデータ
 */
//============================================================================================
u32	PokePersonalParaGet( u16 mons_no, u16 form_no, int para )
{
	u32	ret;

	POKE_PERSONAL_LoadData( mons_no, form_no, &PersonalDataWork );
	ret = POKE_PERSONAL_GetParam( &PersonalDataWork, para );

	return ret;
}

