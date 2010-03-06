//==============================================================================
/**
 * @file    symbol_save_notwifi.h
 * @brief   シンボルエンカウント用セーブデータ：notwifiオーバーレイに配置
 * @author  matsuda
 * @date    2010.03.06(土)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern SYMBOL_MAP_LEVEL_LARGE SymbolSave_GetMapLevelLarge(SYMBOL_SAVE_WORK *symbol_save);
extern SYMBOL_MAP_LEVEL_SMALL SymbolSave_GetMapLevelSmall(SYMBOL_SAVE_WORK *symbol_save);

extern u16 SymbolSave_GetKeepLargeSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_POKEMON *dest_array, int array_num, u8 *occ_num);
extern u16 SymbolSave_GetKeepSmallSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_POKEMON *dest_array, int array_num, u8 *occ_num);
extern u16 SymbolSave_GetFreeLargeSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_POKEMON *dest_array, int array_num, int map_no, u8 *occ_num);
extern u16 SymbolSave_GetFreeSmallSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_POKEMON *dest_array, int array_num, int map_no, u8 *occ_num);


