//==============================================================================
/**
 * @file    symbol_save_notwifi.c
 * @brief   シンボルエンカウント用セーブデータ：notwifiオーバーレイに配置
 * @author  matsuda
 * @date    2010.03.06(土)
 *
 * WiFiでは絶対に使用しないが、それ以外ではどのタイミングであっても使用する可能性のあるもの
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/symbol_save.h"
#include "savedata/symbol_save_notwifi.h"
#include "symbol_save_local.h"



//==================================================================
/**
 * マップレベル(64x64)を取得
 * @param   symbol_save		
 * @retval  SYMBOL_MAP_LEVEL_LARGE		
 */
//==================================================================
SYMBOL_MAP_LEVEL_LARGE SymbolSave_GetMapLevelLarge(SYMBOL_SAVE_WORK *symbol_save)
{
  return symbol_save->map_level_large;
}

//==================================================================
/**
 * マップレベル(32x32)を取得
 * @param   symbol_save		
 * @retval  SYMBOL_MAP_LEVEL_SMALL		
 */
//==================================================================
SYMBOL_MAP_LEVEL_SMALL SymbolSave_GetMapLevelSmall(SYMBOL_SAVE_WORK *symbol_save)
{
  return symbol_save->map_level_small;
}

//--------------------------------------------------------------
/**
 * シンボルポケモンのデータを配列単位で取得する
 *
 * @param   symbol_save		
 * @param   dest_array		代入先
 * @param   array_num		  dest_arrayの配列要素数
 * @param   no		        コピー開始する配置No
 * @param   copy_num		  コピー数
 *
 * @retval  dest_arrayにコピーされたデータにいくつ有効データがあるか
 */
//--------------------------------------------------------------
static u8 SymbolSave_GetArraySymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_POKEMON *dest_array, int array_num, int no, int copy_num)
{
  int i;
  
  GF_ASSERT(array_num <= copy_num);
  GFL_STD_MemClear(dest_array, sizeof(struct _SYMBOL_POKEMON) * array_num);
  GFL_STD_MemCopy(&symbol_save->symbol_poke[no], dest_array, sizeof(struct _SYMBOL_POKEMON) * copy_num);
  for(i = 0; i < copy_num; i++){
    if(dest_array[i].monsno == 0){
      break;
    }
  }
  return i;
}

//==================================================================
/**
 * キープゾーン(64x64)のシンボルポケモンデータを一括で取得
 *
 * @param   symbol_save		
 * @param   dest_array		代入先
 * @param   array_num		  dest_arrayの配列要素数
 * @param   occ_num       dest_arrayにコピーされたデータにいくつ有効データがあるか
 *
 * @param   先頭の配置No
 */
//==================================================================
u16 SymbolSave_GetKeepLargeSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_POKEMON *dest_array, int array_num, u8 *occ_num)
{
  *occ_num = SymbolSave_GetArraySymbolPokemon(symbol_save, dest_array, array_num, 
    SYMBOL_NO_START_KEEP_LARGE, SYMBOL_NO_END_KEEP_LARGE - SYMBOL_NO_START_KEEP_LARGE);
  return SYMBOL_NO_START_KEEP_LARGE;
}

//==================================================================
/**
 * フリーゾーン(32x32)のシンボルポケモンデータを一括で取得
 *
 * @param   symbol_save		
 * @param   dest_array		代入先
 * @param   array_num		  dest_arrayの配列要素数
 * @param   occ_num       dest_arrayにコピーされたデータにいくつ有効データがあるか
 *
 * @param   先頭の配置No
 */
//==================================================================
u16 SymbolSave_GetKeepSmallSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_POKEMON *dest_array, int array_num, u8 *occ_num)
{
  *occ_num = SymbolSave_GetArraySymbolPokemon(symbol_save, dest_array, array_num, 
    SYMBOL_NO_START_KEEP_SMALL, SYMBOL_NO_END_KEEP_SMALL - SYMBOL_NO_START_KEEP_SMALL);
  return SYMBOL_NO_START_KEEP_SMALL;
}

//==================================================================
/**
 * キープゾーン(64x64)のシンボルポケモンデータを一括で取得
 *
 * @param   symbol_save		
 * @param   dest_array		代入先
 * @param   array_num		  dest_arrayの配列要素数
 * @param   map_no		    マップ番号
 * @param   occ_num       dest_arrayにコピーされたデータにいくつ有効データがあるか
 *
 * @param   先頭の配置No
 */
//==================================================================
u16 SymbolSave_GetFreeLargeSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_POKEMON *dest_array, int array_num, int map_no, u8 *occ_num)
{
  u16 no = SYMBOL_NO_START_FREE_LARGE + SYMBOL_MAP_STOCK_LARGE * map_no;
  GF_ASSERT(no < SYMBOL_NO_END_FREE_LARGE);
  *occ_num = SymbolSave_GetArraySymbolPokemon(
    symbol_save, dest_array, array_num, no, SYMBOL_MAP_STOCK_LARGE);
  return no;
}

//==================================================================
/**
 * フリーゾーン(32x32)のシンボルポケモンデータを一括で取得
 *
 * @param   symbol_save		
 * @param   dest_array		代入先
 * @param   array_num		  dest_arrayの配列要素数
 * @param   map_no		    マップ番号
 * @param   occ_num       dest_arrayにコピーされたデータにいくつ有効データがあるか
 *
 * @param   先頭の配置No
 */
//==================================================================
u16 SymbolSave_GetFreeSmallSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_POKEMON *dest_array, int array_num, int map_no, u8 *occ_num)
{
  u16 no = SYMBOL_NO_START_FREE_SMALL + SYMBOL_MAP_STOCK_SMALL * map_no;
  GF_ASSERT(no < SYMBOL_NO_END_FREE_SMALL);
  *occ_num = SymbolSave_GetArraySymbolPokemon(
    symbol_save, dest_array, array_num, no, SYMBOL_MAP_STOCK_SMALL);
  return no;
}
