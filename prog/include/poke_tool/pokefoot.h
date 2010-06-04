//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		pokefoot.h
 *	@brief		ポケモン足跡グラフィックデータ取得関数
 *	@author		tomoya takahashi
 *	@data		2006.01.31
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __POKEFOOT_H__
#define __POKEFOOT_H__

#undef GLOBAL
#ifdef	__POKEFOOT_H_GLOBAL
#define	GLOBAL	/* */
#else
#define	GLOBAL	extern
#endif

#include "pokefoot.naix"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
#define POKEFOOT_MONS_NO_OLD_MAX  (493)  // <=493までが前世代のデータ

#define POKEFOOT_ARC_PLTT         NARC_pokefoot_poke_foot_NCLR      //( 0 )		// パレットインデックス
#define POKEFOOT_ARC_CELL         NARC_pokefoot_poke_foot_000_NCER  //( 2 )		// セルインデックス(mons_noが0<= <=POKEFOOT_MONS_NO_OLD_MAX)
#define POKEFOOT_ARC_CELLANM      NARC_pokefoot_poke_foot_000_NANR  //( 1 )		// セルアニメインデックス
#define POKEFOOT_ARC_CELL_NEW     NARC_pokefoot_poke_foot_600_NCER  //( 4 )		// セルインデックス(mons_noがPOKEFOOT_MONS_NO_OLD_MAX< )
#define POKEFOOT_ARC_CELLANM_NEW  NARC_pokefoot_poke_foot_600_NANR  //( 3 )		// セルアニメインデックス
#define POKEFOOT_ARC_CHAR_DMMY    NARC_pokefoot_poke_foot_000_NCGR  //( 5 )		// poke_foot_000
#define POKEFOOT_ARC_CHAR_START   NARC_pokefoot_poke_foot_001_NCGR  //( 6	)		// キャラクタ開始インデックス

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	モンスターナンバーから足跡キャラクタデータ取得
 *
 *	@param	mons_no		モンスターナンバー
 *	@param	heap		ヒープ
 *
 *	@return	ファイルデータ	破棄は各自
 * [allocType]
 *		ALLOC_TOP		ヒープ先頭から確保
 *		ALLOC_BOTTOM	ヒープ後方から確保
 */
//-----------------------------------------------------------------------------
GLOBAL void* PokeFootCharPtrGet( int mons_no, HEAPID heap );
//----------------------------------------------------------------------------
/**
 *	@brief	足跡パレットデータ取得
 *
 *	@param	heap	ヒープ
 *
 *	@return	ファイルデータ	破棄は各自
 * [allocType]
 *		ALLOC_TOP		ヒープ先頭から確保
 *		ALLOC_BOTTOM	ヒープ後方から確保
 */
//-----------------------------------------------------------------------------
GLOBAL void* PokeFootPlttPtrGet( HEAPID heap );
//----------------------------------------------------------------------------
/**
 *	@brief	足跡セルデータ取得
 *
 *	@param	heap	ヒープ
 *
 *	@return	ファイルデータ	破棄は各自
 * [allocType]
 *		ALLOC_TOP		ヒープ先頭から確保
 *		ALLOC_BOTTOM	ヒープ後方から確保
 */
//-----------------------------------------------------------------------------
//GLOBAL void* PokeFootCellPtrGet( HEAPID heap );
GLOBAL void* PokeFootCellPtrGet( int mons_no, HEAPID heap );
//----------------------------------------------------------------------------
/**
 *	@brief	足跡セルアニメデータ取得
 *
 *	@param	heap	ヒープ
 *
 *	@return	ファイルデータ	破棄は各自
 * [allocType]
 *		ALLOC_TOP		ヒープ先頭から確保
 *		ALLOC_BOTTOM	ヒープ後方から確保
 */
//-----------------------------------------------------------------------------
//GLOBAL void* PokeFootCellAnmPtrGet( HEAPID heap );
GLOBAL void* PokeFootCellAnmPtrGet( int mons_no, HEAPID heap );

// アーカイブファイルIDX　データIDX
// キャラクタ　セル　セルアニメデータは圧縮されています
GLOBAL int PokeFootArcFileGet( void );
GLOBAL int PokeFootCharDataIdxGet( int mons_no );
GLOBAL int PokeFootPlttDataIdxGet( void );
//GLOBAL int PokeFootCellDataIdxGet( void );
GLOBAL int PokeFootCellDataIdxGet( int mons_no );
//GLOBAL int PokeFootCellAnmDataIdxGet( void );
GLOBAL int PokeFootCellAnmDataIdxGet( int mons_no );


#undef	GLOBAL
#endif		// __POKEFOOT_H__

