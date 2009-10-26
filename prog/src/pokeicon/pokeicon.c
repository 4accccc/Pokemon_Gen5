//==============================================================================
/**
 * @file	pokeicon.c
 * @brief	ポケモンアイコン
 * @author	matsuda
 * @date	2008.11.25(火)
 */
//==============================================================================
#include <gflib.h>
#include "arc_def.h"
#include "poke_tool/poke_personal.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "pokeicon/pokeicon.h"
#include "poke_icon.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	POKEICON_GetPAL_TAMAGO			( 1 )	// タマゴのパレット番号
#define	POKEICON_GetPAL_TAMAGO_MNF		( 2 )	// マナフィのタマゴのパレット番号

//フォルムによってパレットが変わるポケモンのIconPalAtrテーブル参照番号
#define	POKEICON_GetTAMAGO		( 494 )		// タマゴ
#define	POKEICON_GetTAMAGO_MNF	( 495 )		// マナフィのタマゴ
#define	POKEICON_GetDEOKISISU	( 496 )		// デオキシス
#define	POKEICON_GetANNOON		( 499 )		// アンノーン
#define	POKEICON_GetMINOMUTTI	( 527 )		// ミノムッチ
#define	POKEICON_GetMINOMESU	( 529 )		// ミノメス
#define	POKEICON_GetSIIUSI		( 531 )		// シーウシ
#define	POKEICON_GetSIIDORUGO	( 532 )		// シードルゴ
#define	POKEICON_GetGIRATHINA	( 533 )		// ギラティナ
#define	POKEICON_GetSHEIMI		( 534 )		// シェイミ
#define	POKEICON_GetROTOMU		( 535 )		// ロトム
#define	POKEICON_GetPOWARUN		( 540 )		// ポワルン
#define	POKEICON_GetTHERIMU		( 543 )		// チェリム


//==============================================================================
//	データ
//==============================================================================
#include "pokeicon.dat"


// ポケモンアイコンがないので、アルセウス以降のポケモンは？？？を表示します
static u32 TestMonsConv( u32 mons )
{
	if( mons > MONSNO_ARUSEUSU ){
		mons = 0;
	}
	return mons;
}


//--------------------------------------------------------------------------------------------
/**
 * キャラのアーカイブインデックス取得 ( POKEMON_PASO_PARAM 版 )
 *
 * @param	ppp			POKEMON_PASO_PARAM
 *
 * @return	アーカイブインデックス
 *
 */
//--------------------------------------------------------------------------------------------
u32 POKEICON_GetCgxArcIndex( const POKEMON_PASO_PARAM* ppp )
{
	u32  monsno;
	u32  arcIndex;
	BOOL fastMode;
	u32  form_no;
	u32  egg;

	fastMode = PPP_FastModeOn((POKEMON_PASO_PARAM*)ppp);
	monsno = PPP_Get(ppp, ID_PARA_monsno, NULL );
	egg = PPP_Get(ppp, ID_PARA_tamago_flag, NULL );
	form_no = POKEICON_GetCgxForm(ppp);

	arcIndex = POKEICON_GetCgxArcIndexByMonsNumber( monsno, form_no, egg );
	PPP_FastModeOff((POKEMON_PASO_PARAM*)ppp, fastMode);
	return arcIndex;
}

//--------------------------------------------------------------------------------------------
/**
 * キャラのアーカイブインデックス取得
 *
 * @param	mons		ポケモン番号
 * @param	form_no		フォルム番号
 * @param	egg			タマゴフラグ(TRUE=タマゴ)
 *
 * @return	アーカイブインデックス
 *
 *	form_noはデオキシスやアンノーンに使用
 */
//--------------------------------------------------------------------------------------------
u32 POKEICON_GetCgxArcIndexByMonsNumber( u32 mons, u32 form_no, BOOL egg )
{
#ifdef PM_DEBUG
#if 0
	{
		if(GX_GetOBJVRamModeChar() != GX_OBJVRAMMODE_CHAR_1D_128K){
			GF_ASSERT(0 && "非対応のマッピングモードです");
			/*---------------
			ポケモンアイコンのCGXは1D128Kで作られている為、このままキャラクタを転送すると
			マッピングモードが変更されてしまいます。
			128kマッピングモード以外でも使用したい場合は
			NNS_G2dGetUnpackedCharacterDataでアンパックした後に
			pCharData->mapingTypeのマッピングタイプを変更してから
			NNS_G2dLoadImage1DMappingで転送するようにしてください
			もしくは32k,64k用のポケモンアイコンCGXデータもアーカイブ内に持つ事も選択肢の一つです
			-----------------*/
		}
	}
#endif
#endif	//PM_DEBUG

	if( egg == TRUE ){
		if( mons == MONSNO_MANAFI ){
			return NARC_poke_icon_poke_icon_mnf_NCGR;
		}else{
			return NARC_poke_icon_poke_icon_tam_NCGR;
		}
	}

	mons = TestMonsConv( mons );

//	form_no = PokeFuseiFormNoCheck(mons, form_no);

	if( form_no != 0 ){
		if( mons == MONSNO_DEOKISISU ){
			return ( NARC_poke_icon_poke_icon_d01_NCGR + form_no - 1 );
		}
		if( mons == MONSNO_ANNOON ){
			return ( NARC_poke_icon_poke_icon_u02_NCGR + form_no - 1 );
		}
		if( mons == MONSNO_MINOMUTTI ){
			return ( NARC_poke_icon_poke_icon_455_01_NCGR + form_no - 1 );
		}
		if( mons == MONSNO_MINOMADAMU ){
			return ( NARC_poke_icon_poke_icon_457_01_NCGR + form_no - 1 );
		}
		if( mons == MONSNO_KARANAKUSI ){
			return ( NARC_poke_icon_poke_icon_458_01_NCGR + form_no - 1 );
		}
		if( mons == MONSNO_TORITODON ){
			return ( NARC_poke_icon_poke_icon_459_01_NCGR + form_no - 1 );
		}
		if( mons == MONSNO_GIRATHINA ){
			return ( NARC_poke_icon_poke_icon_509_01_NCGR + form_no - 1 );
		}
		if( mons == MONSNO_SHEIMI ){
			return ( NARC_poke_icon_poke_icon_516_01_NCGR + form_no - 1 );
		}
		if( mons == MONSNO_ROTOMU ){
			return ( NARC_poke_icon_poke_icon_519_01_NCGR + form_no - 1 );
		}
		// ポワルン（HG/SSから追加）
		if( mons == MONSNO_POWARUN ){
			return ( NARC_poke_icon_poke_icon_351_rain_NCGR + form_no - 1 );
		}
		// チェリム（HG/SSから追加）
		if( mons == MONSNO_THERIMU ){
			return ( NARC_poke_icon_poke_icon_483_sun_NCGR + form_no - 1 );
		}
	}

	if( mons > MONSNO_END ){ mons = 0; }

	return ( NARC_poke_icon_poke_icon_000_NCGR + mons );
}

//------------------------------------------------------------------
/**
 * キャラの拡張パターンナンバーを取得
 *
 * @param   ppp		
 *
 * @retval  u16		格調パターンナンバー
 */
//------------------------------------------------------------------
u16 POKEICON_GetCgxForm( const POKEMON_PASO_PARAM* ppp )
{
	u32 monsno;

	monsno = PPP_Get( ppp, ID_PARA_monsno_egg, NULL );

	switch( monsno ){
	case MONSNO_ANNOON:
	case MONSNO_DEOKISISU:
	case MONSNO_MINOMUTTI:
	case MONSNO_MINOMADAMU:
	case MONSNO_KARANAKUSI:
	case MONSNO_TORITODON:
	case MONSNO_GIRATHINA:
	case MONSNO_SHEIMI:
	case MONSNO_ROTOMU:
	case MONSNO_POWARUN:	// ポワルン（HG/SSから追加）
	case MONSNO_THERIMU:	// チェリム（HG/SSから追加）
		return PPP_Get( (POKEMON_PASO_PARAM*)ppp, ID_PARA_form_no, NULL );

	default:
		return 0;
	}
	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * パレット番号取得
 *
 * @param	mons		ポケモン番号
 * @param	form		フォルム番号
 * @param	egg			タマゴフラグ(TRUE:タマゴ)
 *
 * @return	パレット番号
 */
//--------------------------------------------------------------------------------------------
const u8 POKEICON_GetPalNum( u32 mons, u32 form, BOOL egg )
{
	if( egg == 1 ){
		if( mons == MONSNO_MANAFI ){
			mons = POKEICON_GetTAMAGO_MNF;
		}else{
			mons = POKEICON_GetTAMAGO;
		}
	}else if( mons > MONSNO_END ){
		mons = 0;
	}else if( form != 0 ){
		if( mons == MONSNO_DEOKISISU ){
			mons = POKEICON_GetDEOKISISU + form - 1;
		}else if( mons == MONSNO_ANNOON ){
			mons = POKEICON_GetANNOON + form - 1;
		}else if( mons == MONSNO_MINOMUTTI ){
			mons = POKEICON_GetMINOMUTTI + form - 1;
		}else if( mons == MONSNO_MINOMADAMU ){
			mons = POKEICON_GetMINOMESU + form - 1;
		}else if( mons == MONSNO_KARANAKUSI ){
			mons = POKEICON_GetSIIUSI + form - 1;
		}else if( mons == MONSNO_TORITODON ){
			mons = POKEICON_GetSIIDORUGO + form - 1;
		}else if( mons == MONSNO_GIRATHINA ){
			mons = POKEICON_GetGIRATHINA + form - 1;
		}else if( mons == MONSNO_SHEIMI ){
			mons = POKEICON_GetSHEIMI + form - 1;
		}else if( mons == MONSNO_ROTOMU ){
			mons = POKEICON_GetROTOMU + form - 1;
		// ポワルン（HG/SSから追加）
		}else if( mons == MONSNO_POWARUN ){
			mons = POKEICON_GetPOWARUN + form - 1;
		// チェリム（HG/SSから追加）
		}else if( mons == MONSNO_THERIMU ){
			mons = POKEICON_GetTHERIMU + form - 1;
		}
	}else{
		mons = TestMonsConv( mons );
	}
	return IconPalAtr[mons];
}

//--------------------------------------------------------------------------------------------
/**
 * パレット番号取得（POKEMON_PASO_PARAM版）
 *
 * @param	ppp		POKEMON_PASO_PARAM
 *
 * @return	パレット番号
 */
//--------------------------------------------------------------------------------------------
u8 POKEICON_GetPalNumGetByPPP( const POKEMON_PASO_PARAM * ppp )
{
	BOOL fast;
	u32  mons;
	u32  form;
	u32  egg;

	fast = PPP_FastModeOn( (POKEMON_PASO_PARAM *)ppp );

	form = POKEICON_GetCgxForm( ppp );
	mons = PPP_Get( (POKEMON_PASO_PARAM*)ppp, ID_PARA_monsno, NULL );
	egg  = PPP_Get( (POKEMON_PASO_PARAM*)ppp, ID_PARA_tamago_flag, NULL );

	PPP_FastModeOff( (POKEMON_PASO_PARAM *)ppp, fast );

	return POKEICON_GetPalNum( mons, form, egg );
}

//--------------------------------------------------------------------------------------------
/**
 * パレットのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------------------------------------
u32 POKEICON_GetPalArcIndex(void)
{
	return NARC_poke_icon_poke_icon_NCLR;
}

//--------------------------------------------------------------------------------------------
/**
 * セルのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------------------------------------
u32 POKEICON_GetCellArcIndex(void)
{
	GXOBJVRamModeChar vrammode;
	
	vrammode = GX_GetOBJVRamModeChar();
	switch(vrammode){
	case GX_OBJVRAMMODE_CHAR_1D_32K:
		return NARC_poke_icon_poke_icon_32k_NCER;
	case GX_OBJVRAMMODE_CHAR_1D_64K:
		return NARC_poke_icon_poke_icon_64k_NCER;
	case GX_OBJVRAMMODE_CHAR_1D_128K:
		return NARC_poke_icon_poke_icon_128k_NCER;
	default:
		GF_ASSERT(0);	//非対応のマッピングモード
		return NARC_poke_icon_poke_icon_128k_NCER;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * セルのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------------------------------------
u32 POKEICON_GetCellSubArcIndex(void)
{
	GXOBJVRamModeChar vrammode;
	
	vrammode = GXS_GetOBJVRamModeChar();
	switch(vrammode){
	case GX_OBJVRAMMODE_CHAR_1D_32K:
		return NARC_poke_icon_poke_icon_32k_NCER;
	case GX_OBJVRAMMODE_CHAR_1D_64K:
		return NARC_poke_icon_poke_icon_64k_NCER;
	case GX_OBJVRAMMODE_CHAR_1D_128K:
		return NARC_poke_icon_poke_icon_128k_NCER;
	default:
		GF_ASSERT(0);	//非対応のマッピングモード
		return NARC_poke_icon_poke_icon_128k_NCER;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * セルアニメのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------------------------------------
u32 POKEICON_GetAnmArcIndex(void)
{
	GXOBJVRamModeChar vrammode;
	
	vrammode = GX_GetOBJVRamModeChar();
	switch(vrammode){
	case GX_OBJVRAMMODE_CHAR_1D_32K:
		return NARC_poke_icon_poke_icon_32k_NANR;
	case GX_OBJVRAMMODE_CHAR_1D_64K:
		return NARC_poke_icon_poke_icon_64k_NANR;
	case GX_OBJVRAMMODE_CHAR_1D_128K:
		return NARC_poke_icon_poke_icon_128k_NANR;
	default:
		GF_ASSERT(0);	//非対応のマッピングモード
		return NARC_poke_icon_poke_icon_128k_NANR;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * セルアニメのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------------------------------------
u32 POKEICON_GetAnmSubArcIndex(void)
{
	GXOBJVRamModeChar vrammode;
	
	vrammode = GXS_GetOBJVRamModeChar();
	switch(vrammode){
	case GX_OBJVRAMMODE_CHAR_1D_32K:
		return NARC_poke_icon_poke_icon_32k_NANR;
	case GX_OBJVRAMMODE_CHAR_1D_64K:
		return NARC_poke_icon_poke_icon_64k_NANR;
	case GX_OBJVRAMMODE_CHAR_1D_128K:
		return NARC_poke_icon_poke_icon_128k_NANR;
	default:
		GF_ASSERT(0);	//非対応のマッピングモード
		return NARC_poke_icon_poke_icon_128k_NANR;
	}
}
