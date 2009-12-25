//======================================================================
/**
 * @file	app_menu_common.c
 * @brief	共通素材関係ソース
 * @author	ariizumi
 * @data	09/07/14
 *
 * モジュール名：APP_COMMON
 */
//======================================================================
#include <gflib.h>

#include "arc_def.h"
#include "app_menu_common.naix"

#include "app/app_menu_common.h"

//マルチブート用きり分け(MB時のみ読み込む
#ifdef MULTI_BOOT_MAKE  //DL子機時処理
#include "app_menu_common_dl.naix"
#endif //MULTI_BOOT_MAKE


///技タイプ毎のパレットオフセット番号
ALIGN4 static const u8 PokeTypePlttOffset[] = {
	0,	//POKETYPE_NORMAL
	0,	//POKETYPE_BATTLE
	1,	//POKETYPE_HIKOU
	1,	//POKETYPE_POISON
	0,	//POKETYPE_JIMEN
	0,	//POKETYPE_IWA
	2,	//POKETYPE_MUSHI
	1,	//POKETYPE_GHOST
	0,	//POKETYPE_METAL
	0,	//POKETYPE_FIRE
	1,	//POKETYPE_WATER
	2,	//POKETYPE_KUSA
	0,	//POKETYPE_ELECTRIC
	1,	//POKETYPE_SP
	1,	//POKETYPE_KOORI
	2,	//POKETYPE_DRAGON
	0,	//POKETYPE_AKU
	0,	//ICONTYPE_STYLE,			///<格好よさ
	1,	//ICONTYPE_BEAUTIFUL,		///<美しさ
	1,	//ICONTYPE_CUTE,			///<可愛さ
	2,	//ICONTYPE_INTELLI,		///<賢さ
	0,	//ICONTYPE_STRONG,		///<逞しさ
};

///分類毎のパレットオフセット番号
ALIGN4 static const u8 WazaKindPlttOffset[] = {
  0,    //KIND_HENNKA
  0,    //KIND_BUTSURI
  1,    //KIND_TOKUSYU
};

//バーボタン用パレットオフセット
ALIGN4 static const u8 BarIconPlttOffset[]	= {	
	0,//  APP_COMMON_BARICON_EXIT,
	0,//  APP_COMMON_BARICON_RETURN,
	1,//  APP_COMMON_BARICON_CURSOR_DOWN,
	1,//  APP_COMMON_BARICON_CURSOR_UP,
	1,//  APP_COMMON_BARICON_CURSOR_LEFT,
	1,//  APP_COMMON_BARICON_CURSOR_RIGHT,
	1,//  APP_COMMON_BARICON_CHECK_ON,
	1,//  APP_COMMON_BARICON_CHECK_OFF,
  
};

//--------------------------------------------------------------
//	ARCIDの取得
//--------------------------------------------------------------
const u32 APP_COMMON_GetArcId(void)
{
  return ARCID_APP_MENU_COMMON;
}

//--------------------------------------------------------------
//	タイプ系素材取得
//--------------------------------------------------------------
const u32 APP_COMMON_GetPokeTypePltArcIdx( void )
{
  return NARC_app_menu_common_p_st_type_NCLR;
}

const u8  APP_COMMON_GetPokeTypePltOffset( const PokeType type )
{
  return PokeTypePlttOffset[type];
}

const u32 APP_COMMON_GetPokeTypeCharArcIdx( const PokeType type )
{
  return NARC_app_menu_common_p_st_type_normal_NCGR + type;
}

const u32 APP_COMMON_GetPokeTypeCellArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_p_st_type_32k_NCER + mapping;
}

const u32 APP_COMMON_GetPokeTypeAnimeArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_p_st_type_32k_NANR + mapping;
}

//--------------------------------------------------------------
//技分類系素材取得(パレット・セル・アニメはタイプと共通
//--------------------------------------------------------------
const u8  APP_COMMON_GetWazaKindPltOffset( const PokeType type )
{
  return WazaKindPlttOffset[type];
}

const u32 APP_COMMON_GetWazaKindCharArcIdx( const PokeType type )
{
  return NARC_app_menu_common_p_st_bunrui_henka_NCGR + type;
}

//--------------------------------------------------------------
//下部バーボタン系素材取得
//--------------------------------------------------------------
const u32 APP_COMMON_GetBarIconPltArcIdx( void )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_bar_button_NCLR;
#else                    //DL子機時処理
	return NARC_app_menu_common_dl_bar_button_NCLR;
#endif //MULTI_BOOT_MAKE
}
const u8  APP_COMMON_GetBarIconPltOffset( const APP_COMMON_BARICON type )
{	
	GF_ASSERT( type < APP_COMMON_BARICON_MAX );
	return BarIconPlttOffset[ type ];
}
const u32 APP_COMMON_GetBarIconCharArcIdx()
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_bar_button_128k_NCGR;
#else                    //DL子機時処理
	return NARC_app_menu_common_dl_bar_button_128k_NCGR;
#endif //MULTI_BOOT_MAKE
}
const u32 APP_COMMON_GetBarIconCellArcIdx( const APP_COMMON_MAPPING mapping )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_bar_button_32k_NCER + mapping;
#else                    //DL子機時処理
	GF_ASSERT( mapping == APP_COMMON_MAPPING_128K );
	return NARC_app_menu_common_dl_bar_button_128k_NCER;
#endif //MULTI_BOOT_MAKE
}
const u32 APP_COMMON_GetBarIconAnimeArcIdx( const APP_COMMON_MAPPING mapping )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_bar_button_32k_NANR + mapping;
#else                    //DL子機時処理
	GF_ASSERT( mapping == APP_COMMON_MAPPING_128K );
	return NARC_app_menu_common_dl_bar_button_128k_NANR;
#endif //MULTI_BOOT_MAKE
}
//--------------------------------------------------------------
//下部バー系素材取得
//--------------------------------------------------------------
const u32 APP_COMMON_GetBarPltArcIdx( void )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_menu_bar_NCLR;
#else                    //DL子機時処理
	return NARC_app_menu_common_dl_menu_bar_NCLR;
#endif //MULTI_BOOT_MAKE
}
const u32 APP_COMMON_GetBarCharArcIdx( void )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_menu_bar_NCGR;
#else                    //DL子機時処理
	return NARC_app_menu_common_dl_menu_bar_NCGR;
#endif //MULTI_BOOT_MAKE
}
const u32 APP_COMMON_GetBarScrnArcIdx( void )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_menu_bar_NSCR;
#else                    //DL子機時処理
	return NARC_app_menu_common_dl_menu_bar_NSCR;
#endif //MULTI_BOOT_MAKE
}
const u32 APP_COMMON_GetBarScrn_512x256ArcIdx( void )
{	
	return NARC_app_menu_common_menu_bar_512x256_NSCR;
}

//--------------------------------------------------------------
// アイテムアイコン(ポケアイコン用)
//--------------------------------------------------------------

const u32 APP_COMMON_GetPokeItemIconPltArcIdx( void )
{	
	return NARC_app_menu_common_item_icon_NCLR;
}
const u32 APP_COMMON_GetPokeItemIconCharArcIdx( void )
{	
	return NARC_app_menu_common_item_icon_NCGR;
}
const u32 APP_COMMON_GetPokeItemIconCellArcIdx( const APP_COMMON_MAPPING mapping )
{	
	return NARC_app_menu_common_item_icon_32k_NCER + mapping;
}
const u32 APP_COMMON_GetPokeItemIconAnimeArcIdx( const APP_COMMON_MAPPING mapping )
{	
	return NARC_app_menu_common_item_icon_32k_NANR + mapping;
}

//--------------------------------------------------------------
//4 x 4 空データ取得（テクスチャ転送用）
//--------------------------------------------------------------
const u32 APP_COMMON_GetNull4x4PltArcIdx( void )
{	
	return NARC_app_menu_common_null_4x4_NCLR;
}
const u32 APP_COMMON_GetNull4x4CharArcIdx( void )
{	
	return NARC_app_menu_common_null_4x4_128k_NCGR;
}
const u32 APP_COMMON_GetNull4x4CellArcIdx( const APP_COMMON_MAPPING mapping )
{	
  return NARC_app_menu_common_null_4x4_32k_NCER + mapping;
}
const u32 APP_COMMON_GetNull4x4AnimeArcIdx( const APP_COMMON_MAPPING mapping )
{	
  return NARC_app_menu_common_null_4x4_32k_NANR + mapping;
}

//--------------------------------------------------------------
//状態異常アイコンデータ取得
//--------------------------------------------------------------
const u32 APP_COMMON_GetStatusIconPltArcIdx( void )
{
  return NARC_app_menu_common_p_st_ijou_NCLR;
}
const u32 APP_COMMON_GetStatusIconCharArcIdx( void )
{
  return NARC_app_menu_common_p_st_ijou_NCGR;
}
const u32 APP_COMMON_GetStatusIconCellArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_p_st_ijou_32k_NCER + mapping;
}
const u32 APP_COMMON_GetStatusIconAnimeArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_p_st_ijou_32k_NANR + mapping;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		状態異常アイコンのアニメ番号を取得
 *
 * @param		pp		POKEMON_PARAM
 *
 * @return	アニメ番号
 *
 * @li	ポケルスはチェックしてません
 */
//--------------------------------------------------------------------------------------------
u32 APP_COMMON_GetStatusIconAnime( POKEMON_PARAM * pp )
{
	// 瀕死
	if( PP_Get( pp, ID_PARA_hp, NULL ) == 0 ){
		return APP_COMMON_ST_ICON_HINSI;
	}else{
		u32	prm = PP_Get( pp, ID_PARA_condition, NULL );

		// まひ
		if( prm == POKESICK_MAHI ){
			return APP_COMMON_ST_ICON_MAHI;
    // ねむり
		}else if( prm == POKESICK_NEMURI ){
			return APP_COMMON_ST_ICON_NEMURI;
		// こおり
		}else if( prm == POKESICK_KOORI ){
			return APP_COMMON_ST_ICON_KOORI;
		// やけど
		}else if( prm == POKESICK_YAKEDO ){
			return APP_COMMON_ST_ICON_YAKEDO;
		// どく
		}else if( prm == POKESICK_DOKU ){
			return APP_COMMON_ST_ICON_DOKU;
		}

/*	prm = PokeSickなので修正 2009/12/25 by nakahiro
		// 毒
		if( ( prm & (PTL_CONDITION_DOKU|PTL_CONDITION_DOKUDOKU) ) != 0 ){
			return APP_COMMON_ST_ICON_DOKU;
		// 眠り
		}else if( ( prm & PTL_CONDITION_NEMURI ) != 0 ){
			return APP_COMMON_ST_ICON_NEMURI;
		// 火傷
		}else if( ( prm & PTL_CONDITION_YAKEDO ) != 0 ){
			return APP_COMMON_ST_ICON_YAKEDO;
		// 氷
		}else if( ( prm & PTL_CONDITION_KOORI ) != 0 ){
			return APP_COMMON_ST_ICON_KOORI;
		// 麻痺
		}else if( ( prm & PTL_CONDITION_MAHI ) != 0 ){
			return APP_COMMON_ST_ICON_MAHI;
		}
*/
		// なし
		return APP_COMMON_ST_ICON_NONE;
	}
}
//=============================================================================
/**
 *	ポケモン絵OBJのためのダミーセル
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	ダミーセルアーカイブインデックス
 *
 *	@param	const APP_COMMON_MAPPING mapping マッピングモード
 *
 *	@return	リソース番号
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetDummyCellArcIdx( const APP_COMMON_MAPPING mapping )
{	
	return NARC_app_menu_common_poke2dgra_dummy_32k_NCER + mapping;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ダミーアニメアーカイブインデックス取得
 *
 *	@param	const APP_COMMON_MAPPING mapping	マッピングモード
 *
 *	@return	リソース番号
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetDummyAnimeArcIdx( const APP_COMMON_MAPPING mapping )
{	
	return NARC_app_menu_common_poke2dgra_dummy_32k_NANR + mapping;
}
//=============================================================================
/**
 *	ボールアイコン
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	パレットリソース番号取得
 *
 *	@param	const BALL_ID ball					ボールID
 *
 *	@return	パレットアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetBallPltArcIdx( const BALL_ID ball )
{	
	GF_ASSERT_MSG( ball != BALLID_NULL, "ボールIDがNULLです\n" );
	return NARC_app_menu_common_ball00_NCLR + ball - 1;	//BALLIDは1オリジン
}
//----------------------------------------------------------------------------
/**
 *	@brief	キャラリソース番号取得
 *
 *	@param	const BALL_ID ball					ボールID
 *
 *	@return	キャラアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetBallCharArcIdx( const BALL_ID ball )
{	
	GF_ASSERT_MSG( ball != BALLID_NULL, "ボールIDがNULLです\n" );
	return NARC_app_menu_common_ball00_NCGR + ball - 1;	//BALLIDは１オリジン
}
//----------------------------------------------------------------------------
/**
 *	@brief	セルリソース番号取得
 *
 *	@param	const BALL_ID ball					ボールID
 *	@param	APP_COMMON_MAPPING mapping	マッピングモード
 *
 *	@return	セルアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetBallCellArcIdx( const BALL_ID ball, const APP_COMMON_MAPPING mapping )
{	
	GF_ASSERT_MSG( ball != BALLID_NULL, "ボールIDがNULLです\n" );
	return NARC_app_menu_common_ball_32k_NCER + mapping;
}
//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメリソース番号取得
 *
 *	@param	const BALL_ID ball					ボールID
 *	@param	APP_COMMON_MAPPING mapping	マッピングモード
 *
 *	@return	パレットアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetBallAnimeArcIdx( const BALL_ID ball, const APP_COMMON_MAPPING mapping )
{	
	GF_ASSERT_MSG( ball != BALLID_NULL, "ボールIDがNULLです\n" );
	return NARC_app_menu_common_ball_32k_NANR + mapping;
}
//=============================================================================
/**
 *	マーク
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	パレットリソース番号取得
 *
 *	@return	パレットアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetPokeMarkPltArcIdx( void )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_poke_mark_NCLR;
#else                    //DL子機時処理
	return NARC_app_menu_common_dl_poke_mark_NCLR;
#endif //MULTI_BOOT_MAKE
}
//----------------------------------------------------------------------------
/**
 *	@brief	キャラリソース番号取得
 *
 *	@param	APP_COMMON_MAPPING mapping	マッピングモード
 *
 *	@return	キャラアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetPokeMarkCharArcIdx( const APP_COMMON_MAPPING mapping )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_poke_mark_32k_NCGR + mapping;
#else                    //DL子機時処理
	GF_ASSERT( mapping == APP_COMMON_MAPPING_128K );
	return NARC_app_menu_common_dl_poke_mark_128k_NCGR;
#endif //MULTI_BOOT_MAKE
}
//----------------------------------------------------------------------------
/**
 *	@brief	セルリソース番号取得
 *
 *	@param	APP_COMMON_MAPPING mapping	マッピングモード
 *
 *	@return	セルアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetPokeMarkCellArcIdx( const APP_COMMON_MAPPING mapping )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_poke_mark_32k_NCER + mapping;
#else                    //DL子機時処理
	GF_ASSERT( mapping == APP_COMMON_MAPPING_128K );
	return NARC_app_menu_common_dl_poke_mark_128k_NCER;
#endif //MULTI_BOOT_MAKE
}
//----------------------------------------------------------------------------
/**
 *	@brief	セルアニメリソース番号取得
 *
 *	@param	APP_COMMON_MAPPING mapping	マッピングモード
 *
 *	@return	パレットアーカイブインデックス
 */
//-----------------------------------------------------------------------------
u32 APP_COMMON_GetPokeMarkAnimeArcIdx( const APP_COMMON_MAPPING mapping )
{	
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理
	return NARC_app_menu_common_poke_mark_32k_NANR + mapping;
#else                    //DL子機時処理
	GF_ASSERT( mapping == APP_COMMON_MAPPING_128K );
	return NARC_app_menu_common_dl_poke_mark_128k_NANR;
#endif //MULTI_BOOT_MAKE
}

//--------------------------------------------------------------
//HPバーデータ取得
//--------------------------------------------------------------
const u32 APP_COMMON_GetHPBarBasePltArcIdx( void )
{
  return NARC_app_menu_common_hp_bar_NCLR;
}
const u32 APP_COMMON_GetHPBarBaseCharArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_hp_dodai_32k_NCGR + mapping;
}
const u32 APP_COMMON_GetHPBarBaseCellArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_hp_dodai_32k_NCER + mapping;
}
const u32 APP_COMMON_GetHPBarBaseAnimeArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_hp_dodai_32k_NANR + mapping;
}


//--------------------------------------------------------------
//ポケルス
//--------------------------------------------------------------
u32 APP_COMMON_GetPokerusPltArcIdx( void )
{
  return NARC_app_menu_common_pokerus_NCLR;
}
u32 APP_COMMON_GetPokerusCharArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_pokerus_32k_NCGR + mapping;
}
u32 APP_COMMON_GetPokerusCellArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_pokerus_32k_NCER + mapping;
}
u32 APP_COMMON_GetPokerusAnimeArcIdx( const APP_COMMON_MAPPING mapping )
{
  return NARC_app_menu_common_pokerus_32k_NANR + mapping;
}

