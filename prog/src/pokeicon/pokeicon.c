//==============================================================================
/**
 * @file	pokeicon.c
 * @brief	ポケモンアイコン
 * @author	matsuda changed by soga（シャチでのフォルム違いとオスメス書き分けに対応）
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

//#define GENDER_VER

//============================================================================================
//	定数定義
//============================================================================================

enum{ 
  POKEICON_M_NCGR = 0,
  POKEICON_F_NCGR,

  POKEICON_FILE_MAX,
};

//==============================================================================
//	データ
//==============================================================================
#include "pokeicon.dat"

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
	u32  sex;
	u32  egg;

	fastMode  = PPP_FastModeOn((POKEMON_PASO_PARAM*)ppp);
	monsno    = PPP_Get(ppp, ID_PARA_monsno, NULL );
	form_no   = PPP_Get( (POKEMON_PASO_PARAM*)ppp, ID_PARA_form_no, NULL );
	sex       = PPP_Get( (POKEMON_PASO_PARAM*)ppp, ID_PARA_sex, NULL );
	egg       = PPP_Get(ppp, ID_PARA_tamago_flag, NULL );

#ifdef GENDER_VER
	arcIndex = POKEICON_GetCgxArcIndexByMonsNumber( monsno, form_no, sex, egg );
#else
	arcIndex = POKEICON_GetCgxArcIndexByMonsNumber( monsno, form_no, egg );
#endif
	PPP_FastModeOff((POKEMON_PASO_PARAM*)ppp, fastMode);
	return arcIndex;
}

//--------------------------------------------------------------------------------------------
/**
 * キャラのアーカイブインデックス取得
 *
 * @param	mons_no	ポケモン番号
 * @param	form_no	フォルム番号
 * @param	sex		  性別
 * @param	egg			タマゴフラグ(TRUE=タマゴ)
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------------------------------------
#ifdef GENDER_VER
u32 POKEICON_GetCgxArcIndexByMonsNumber( u32 mons_no, u32 form_no, u32 sex, BOOL egg )
#else
u32 POKEICON_GetCgxArcIndexByMonsNumber( u32 mons_no, u32 form_no, BOOL egg )
#endif
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
	u32 file_start;

  file_start	= NARC_poke_icon_poke_icon_000_m_NCGR + POKEICON_FILE_MAX * mons_no;

  //タマゴチェック
  if( egg )
  { 
    if( mons_no == MONSNO_MANAFI )
    { 
      form_no = 1;
    }
    else
    { 
      form_no = 0;
    }
    file_start = POKEICON_FILE_MAX * ( MONSNO_TAMAGO + form_no );
  }
  //別フォルム処理
  else if( form_no )
  { 
    int gra_index = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_form_gra_index );
    int pltt_only = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_pltt_only );
    int form_max = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_form_max );
    if( form_no >= form_max )
    { 
      form_no = 0;
    }
    if( pltt_only )
    { 
#if 0
      //@todo アイコンの方のアルセウスをどうするか決まってないので現状は変化なしにする
      if( pltt_only_offset )
      { 
        *pltt_only_offset = POKEICON_FILE_MAX * ( MONSNO_MAX + OTHER_FORM_MAX + 1 ) + 13 + POKEGRA_PLTT_ONLY_MAX * ( gra_index + form_no - 1 ) + rare;
      }
#else
      file_start = NARC_poke_icon_poke_icon_000_m_NCGR + POKEICON_FILE_MAX * ( MONSNO_MAX + 1 ) + POKEICON_FILE_MAX * ( gra_index + form_no - 1 );
#endif
    }
    else
    { 
      file_start = NARC_poke_icon_poke_icon_000_m_NCGR + POKEICON_FILE_MAX * ( MONSNO_MAX + 1 ) + POKEICON_FILE_MAX * ( gra_index + form_no - 1 );
    }
  }

#ifdef GENDER_VER
  //性別のチェック
  switch( sex ){
  case PTL_SEX_MALE:
    break;
  case PTL_SEX_FEMALE:
    //オスメス書き分けしているかチェックする（サイズが０なら書き分けなし）
    sex = ( GFL_ARC_GetDataSize( ARCID_POKEICON, file_start + POKEICON_F_NCGR ) == 0 ) ? PTL_SEX_MALE : PTL_SEX_FEMALE;
    break;
  case PTL_SEX_UNKNOWN:
    //性別なしは、オス扱いにする
    sex = PTL_SEX_MALE;
    break;
  default:
    //ありえない性別
    GF_ASSERT(0);
    break;
  }

	return ( file_start + sex );
#else
	return ( file_start );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * パレット番号取得
 *
 * @param	mons_no		ポケモン番号
 * @param	form_no		フォルム番号
 * @param	sex		    性別
 * @param	egg			  タマゴフラグ(TRUE:タマゴ)
 *
 * @return	パレット番号
 */
//--------------------------------------------------------------------------------------------
#ifdef GENDER_VER
const u8 POKEICON_GetPalNum( u32 mons_no, u32 form_no, u32 sex, BOOL egg )
#else
const u8 POKEICON_GetPalNum( u32 mons_no, u32 form_no, BOOL egg )
#endif
{
  //タマゴチェック
  if( egg )
  { 
    if( mons_no == MONSNO_MANAFI )
    { 
      form_no = 1;
    }
    else
    { 
      form_no = 0;
    }
    mons_no = MONSNO_TAMAGO + form_no;
  }
  //別フォルム処理
  else if( form_no )
  { 
    int gra_index = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_form_gra_index );
    int pltt_only = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_pltt_only );
    int form_max = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_form_max );
    if( form_no >= form_max )
    { 
      form_no = 0;
    }
    if( pltt_only )
    { 
#if 0
      //@todo アイコンの方のアルセウスをどうするか決まってないので現状は変化なしにする
      if( pltt_only_offset )
      { 
        *pltt_only_offset = POKEICON_FILE_MAX * ( MONSNO_MAX + OTHER_FORM_MAX + 1 ) + 13 + POKEGRA_PLTT_ONLY_MAX * ( gra_index + form_no - 1 ) + rare;
      }
#else
      mons_no = POKEICON_FILE_MAX * ( MONSNO_MAX + 1 ) + POKEICON_FILE_MAX * ( gra_index + form_no - 1 );
#endif
    }
    else
    { 
      mons_no = POKEICON_FILE_MAX * ( MONSNO_MAX + 1 ) + POKEICON_FILE_MAX * ( gra_index + form_no - 1 );
    }
  }
#ifdef GENDER_VER
  if( sex )
  { 
	  return ( IconPalAtr[ mons_no ] & 0xf0 ) >> 4;
  }
#endif
	return IconPalAtr[ mons_no ] & 0x0f;
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
	u32  sex;
	u32  egg;

	fast = PPP_FastModeOn( (POKEMON_PASO_PARAM *)ppp );

	mons = PPP_Get( (POKEMON_PASO_PARAM*)ppp, ID_PARA_monsno, NULL );
	form = PPP_Get( (POKEMON_PASO_PARAM*)ppp, ID_PARA_form_no, NULL );
	sex  = PPP_Get( (POKEMON_PASO_PARAM*)ppp, ID_PARA_sex, NULL );
	egg  = PPP_Get( (POKEMON_PASO_PARAM*)ppp, ID_PARA_tamago_flag, NULL );

	PPP_FastModeOff( (POKEMON_PASO_PARAM *)ppp, fast );

#ifdef GENDER_VER
	return POKEICON_GetPalNum( mons, form, sex, egg );
#else
	return POKEICON_GetPalNum( mons, form, egg );
#endif
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
