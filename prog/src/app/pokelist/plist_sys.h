//======================================================================
/**
 * @file	plist_sys.c
 * @brief	ポケモンリスト メイン処理
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PLIST
 * モジュール名：PLIST_UTIL
 */
//======================================================================
#pragma once

#include "plist_local_def.h"

extern const BOOL PLIST_InitPokeList( PLIST_WORK *work );
extern const BOOL PLIST_TermPokeList( PLIST_WORK *work );
extern const BOOL PLIST_UpdatePokeList( PLIST_WORK *work );

//--------------------------------------------------------------
//	指定技箇所にフィールド技があるか？
//  戻り値はPL_RET_***** 無いときは０
//--------------------------------------------------------------
extern u32 PLIST_UTIL_CheckFieldWaza( const POKEMON_PARAM *pp , const u8 wazaPos );

