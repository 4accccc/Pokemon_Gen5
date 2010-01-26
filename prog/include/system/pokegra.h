//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		pokegra.h
 *	@brief	ポケモングラフィック素材の取得
 *	@author	Toru=Nagihashi
 *	@date		2009.10.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//ライブラリ
#include <gflib.h>
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define	POKEGRA_DIR_FRONT	( 0 )	//正面
#define	POKEGRA_DIR_BACK	( 1 )	//背面

//ポケモン一体を構成するMCSS用ファイルの構成
enum{
	POKEGRA_FRONT_M_NCGR = 0,
	POKEGRA_FRONT_F_NCGR,
	POKEGRA_FRONT_M_NCBR,
	POKEGRA_FRONT_F_NCBR,
	POKEGRA_FRONT_NCER,
	POKEGRA_FRONT_NANR,
	POKEGRA_FRONT_NMCR,
	POKEGRA_FRONT_NMAR,
	POKEGRA_FRONT_NCEC,
	POKEGRA_BACK_M_NCGR,
	POKEGRA_BACK_F_NCGR,
	POKEGRA_BACK_M_NCBR,
	POKEGRA_BACK_F_NCBR,
	POKEGRA_BACK_NCER,
	POKEGRA_BACK_NANR,
	POKEGRA_BACK_NMCR,
	POKEGRA_BACK_NMAR,
	POKEGRA_BACK_NCEC,
	POKEGRA_NORMAL_NCLR,
	POKEGRA_RARE_NCLR,

	POKEGRA_FILE_MAX,			//ポケモン一体を構成するMCSS用ファイルの総数

	POKEGRA_M_NCGR = 0,
	POKEGRA_F_NCGR,
	POKEGRA_M_NCBR,
	POKEGRA_F_NCBR,
	POKEGRA_NCER,
	POKEGRA_NANR,
	POKEGRA_NMCR,
	POKEGRA_NMAR,
	POKEGRA_NCEC,

  POKEGRA_MAX,
};

//-------------------------------------
///	リソース
//=====================================
#define POKEGRA_POKEMON_CHARA_WIDTH		(12)	//ポケモン絵のキャラ幅
#define POKEGRA_POKEMON_CHARA_HEIGHT	(12)	//ポケモン絵のキャラ高さ

#define POKEGRA_POKEMON_CHARA_SIZE		(POKEGRA_POKEMON_CHARA_WIDTH*POKEGRA_POKEMON_CHARA_HEIGHT*GFL_BG_1CHRDATASIZ)	//ポケモンのキャラサイズ

#define POKEGRA_POKEMON_PLT_NUM				(1)	//ポケモン1体分のパレット使用本数
#define POKEGRA_POKEMON_PLT_SIZE			(POKEGRA_POKEMON_PLT_NUM*0x20)	//ポケモン1体分のパレットサイズ

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern ARCID POKEGRA_GetArcID( void );
//NCGR
extern ARCDATID POKEGRA_GetCgrArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NCBR
extern ARCDATID POKEGRA_GetCbrArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NCLR
extern ARCDATID POKEGRA_GetPalArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NCER	-> マルチセル用のパーツです。
extern ARCDATID POKEGRA_GetCelArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NANR
extern ARCDATID POKEGRA_GetAnmArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NMCR
extern ARCDATID POKEGRA_GetMCelArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NMAR
extern ARCDATID POKEGRA_GetMAnmArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NCEC MCSS用ファイル
extern ARCDATID POKEGRA_GetNcecArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
