//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		poke2dgra.h
 *	@brief	ポケモンOBJ,BGグラフィック
 *	@author	Toru=Nagihashi
 *	@date		2009.10.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "poke_tool/poke_tool.h"
#include "system/pokegra.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	キャラサイズ
//=====================================
#define POKE2DGRA_POKEMON_CHARA_WIDTH		(12)	//ポケモン絵のキャラ幅
#define POKE2DGRA_POKEMON_CHARA_HEIGHT	(12)	//ポケモン絵のキャラ高さ

#define POKE2DGRA_POKEMON_CHARA_SIZE		(POKE2DGRA_POKEMON_CHARA_WIDTH*POKE2DGRA_POKEMON_CHARA_HEIGHT*GFL_BG_1CHRDATASIZ)	//ポケモンのキャラサイズ


//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
extern void* POKE2DGRA_LoadCharacter( NNSG2dCharacterData **ncg_data, const POKEMON_PASO_PARAM* ppp, int dir, HEAPID heapID );
extern void* POKE2DGRA_OBJ_LoadCharacter( NNSG2dCharacterData **ncg_data, const POKEMON_PASO_PARAM* ppp, int dir, HEAPID heapID );

//-------------------------------------
///	BG関係
//=====================================
extern void POKE2DGRA_BG_TransResource( const POKEMON_PASO_PARAM* ppp, int dir, u32 frm, u32 chr_offs, u32 plt_offs, HEAPID heapID );
extern GFL_ARCUTIL_TRANSINFO POKE2DGRA_BG_TransResourceByAreaMan( const POKEMON_PASO_PARAM* ppp, int dir, u32 frm, u32 plt_offs, HEAPID heapID );
extern void POKE2DGRA_BG_WriteScreen( u32 frm, u32 chr_offs, u32 pal_offs, u16 x, u16 y );

//-------------------------------------
///	OBJ関係
//=====================================
extern ARCHANDLE *POKE2DGRA_OpenHandle( HEAPID heapID );
extern u32 POKE2DGRA_OBJ_PLTT_Register( ARCHANDLE *p_handle, const POKEMON_PASO_PARAM* ppp, int dir, CLSYS_DRAW_TYPE vramType, u16 byteOffs, HEAPID heapID );
extern u32 POKE2DGRA_OBJ_CGR_Register( ARCHANDLE *p_handle, const POKEMON_PASO_PARAM* ppp, int dir, CLSYS_DRAW_TYPE vramType, HEAPID heapID );
extern u32 POKE2DGRA_OBJ_CELLANM_Register( const POKEMON_PASO_PARAM* ppp, int dir, APP_COMMON_MAPPING mapping, CLSYS_DRAW_TYPE vramType, HEAPID heapID );
