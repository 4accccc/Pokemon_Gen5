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
#include "app/app_menu_common.h"
#include "poke_tool/monsno_def.h"

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
//-------------------------------------
///	キャラデータ読み込み  
//  （内部でパッチールのぶち生成もおこなっています
//  　ただしキャラデータは１Dマッピングの8x8,4x8,8x4,4x4順にならんでいるので、
//    2Dキャラとして使いたい場合、pokegra.hの加工関数を使ってください）
//=====================================
extern void* POKE2DGRA_LoadCharacterPPP( NNSG2dCharacterData **ncg_data, const POKEMON_PASO_PARAM* ppp, int dir, HEAPID heapID );
extern void* POKE2DGRA_LoadCharacter( NNSG2dCharacterData **ncg_data, int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 personal_rnd, HEAPID heapID );
//-------------------------------------
///	BG関係
//=====================================
//モンスター番号等から読む場合
extern void POKE2DGRA_BG_TransResource( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 frm, u32 personal_rnd, u32 chr_offs, u32 plt_offs, HEAPID heapID );
extern GFL_ARCUTIL_TRANSINFO POKE2DGRA_BG_TransResourceByAreaMan( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 frm, u32 plt_offs, HEAPID heapID );
//PPPから読む場合
extern void POKE2DGRA_BG_TransResourcePPP( const POKEMON_PASO_PARAM* ppp, int dir, u32 frm, u32 chr_offs, u32 plt_offs, HEAPID heapID );
extern GFL_ARCUTIL_TRANSINFO POKE2DGRA_BG_TransResourceByAreaManPPP( const POKEMON_PASO_PARAM* ppp, int dir, u32 frm, u32 plt_offs, HEAPID heapID );
//スクリーン貼り付け
extern void POKE2DGRA_BG_WriteScreen( u32 frm, u32 chr_offs, u32 pal_offs, u16 x, u16 y );
//-------------------------------------
///	OBJ関係
//=====================================
//ポケグラのアークハンドル
extern ARCHANDLE *POKE2DGRA_OpenHandle( HEAPID heapID );
//モンスター番号等から読む場合
extern u32 POKE2DGRA_OBJ_PLTT_Register( ARCHANDLE *p_handle, int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, CLSYS_DRAW_TYPE vramType, u16 byteOffs, HEAPID heapID );
extern u32 POKE2DGRA_OBJ_CGR_Register( ARCHANDLE *p_handle, int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 personal_rnd, CLSYS_DRAW_TYPE vramType, HEAPID heapID );
extern u32 POKE2DGRA_OBJ_CELLANM_Register( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, APP_COMMON_MAPPING mapping, CLSYS_DRAW_TYPE vramType, HEAPID heapID );
//PPPから読む場合
extern u32 POKE2DGRA_OBJ_PLTT_RegisterPPP( ARCHANDLE *p_handle, const POKEMON_PASO_PARAM* ppp, int dir, CLSYS_DRAW_TYPE vramType, u16 byteOffs, HEAPID heapID );
extern u32 POKE2DGRA_OBJ_CGR_RegisterPPP( ARCHANDLE *p_handle, const POKEMON_PASO_PARAM* ppp, int dir, CLSYS_DRAW_TYPE vramType, HEAPID heapID );
extern u32 POKE2DGRA_OBJ_CELLANM_RegisterPPP( const POKEMON_PASO_PARAM* ppp, int dir, APP_COMMON_MAPPING mapping, CLSYS_DRAW_TYPE vramType, HEAPID heapID );

