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
#include "print/wordset.h"

extern const BOOL PLIST_InitPokeList( PLIST_WORK *work );
extern const BOOL PLIST_TermPokeList( PLIST_WORK *work );
extern const BOOL PLIST_UpdatePokeList( PLIST_WORK *work );

//--------------------------------------------------------------
//	指定技箇所にフィールド技があるか？
//  戻り値はPL_RET_***** 無いときは０
//--------------------------------------------------------------
extern u32 PLIST_UTIL_CheckFieldWaza( const POKEMON_PARAM *pp , const u8 wazaPos );
extern const BOOL PLIST_UTIL_IsBattleMenu( const PLIST_WORK *work );

//文字描画用
extern void PLIST_UTIL_DrawStrFunc( PLIST_WORK *work , GFL_BMPWIN *bmpWin , 
                                      const u16 strId , const u16 posX , const u16 posY , const u16 col );
extern void PLIST_UTIL_DrawStrFuncSys( PLIST_WORK *work , GFL_BMPWIN *bmpWin , 
                                      const u16 strId , const u16 posX , const u16 posY , const u16 col );
extern void PLIST_UTIL_DrawValueStrFunc( PLIST_WORK *work , GFL_BMPWIN *bmpWin , 
                                      WORDSET *wordSet, const u16 strId , const u16 posX , const u16 posY , const u16 col );
extern void PLIST_UTIL_DrawValueStrFuncSys( PLIST_WORK *work , GFL_BMPWIN *bmpWin , 
                                      WORDSET *wordSet, const u16 strId , const u16 posX , const u16 posY , const u16 col );
