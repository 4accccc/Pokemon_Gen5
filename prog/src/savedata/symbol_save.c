//==============================================================================
/**
 * @file    symbol_save.c
 * @brief   シンボルエンカウント用セーブデータ
 * @author  matsuda
 * @date    2010.01.06(水)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/symbol_save.h"
#include "symbol_save_local.h"


SDK_COMPILER_ASSERT(SYMBOL_POKE_MAX == SYMBOL_NO_END_FREE_SMALL);


//==============================================================================
//  定数定義
//==============================================================================
///マップ(32x32)が1レベルで所有するストック数
#define SMALL_MAP_EXPAND_NUM      (SYMBOL_MAP_STOCK_SMALL * 3)
///マップ(64x64)が1レベルで所有するストック数
#define LARGE_MAP_EXPAND_NUM      (SYMBOL_MAP_STOCK_LARGE * 3)


//==============================================================================
//  データ
//==============================================================================
///SYMBOL_ZONE_TYPE毎の配置Noの開始位置と終端位置
const SYMBOL_ZONE_TYPE_DATA_NO SymbolZoneTypeDataNo[] = {
  {SYMBOL_NO_START_KEEP_LARGE, SYMBOL_NO_END_KEEP_LARGE},   //SYMBOL_ZONE_TYPE_KEEP_LARGE
  {SYMBOL_NO_START_KEEP_SMALL, SYMBOL_NO_END_KEEP_SMALL},   //SYMBOL_ZONE_TYPE_KEEP_SMALL
  {SYMBOL_NO_START_FREE_LARGE, SYMBOL_NO_END_FREE_LARGE},   //SYMBOL_ZONE_TYPE_FREE_LARGE
  {SYMBOL_NO_START_FREE_SMALL, SYMBOL_NO_END_FREE_SMALL},   //SYMBOL_ZONE_TYPE_FREE_SMALL
};



//==============================================================================
//
//  
//
//==============================================================================
//------------------------------------------------------------------
/**
 * セーブデータサイズを返す
 *
 * @retval  int		
 */
//------------------------------------------------------------------
u32 SymbolSave_GetWorkSize( void )
{
	return sizeof(SYMBOL_SAVE_WORK);
}

//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec		
 */
//--------------------------------------------------------------
void SymbolSave_WorkInit(void *work)
{
  SYMBOL_SAVE_WORK *symbol_save = work;
  
  GFL_STD_MemClear(symbol_save, sizeof(SYMBOL_SAVE_WORK));
}


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * @brief   SYMBOL_SAVE_WORKデータ取得
 * @param   pSave		セーブデータポインタ
 * @return  SYMBOL_SAVE_WORKデータ
 */
//==================================================================
SYMBOL_SAVE_WORK* SymbolSave_GetSymbolData(SAVE_CONTROL_WORK* pSave)
{
	SYMBOL_SAVE_WORK* pData;
	pData = SaveControl_DataPtrGet(pSave, GMDATA_ID_SYMBOL);
	return pData;
}



//==================================================================
/**
 * フリーゾーンに空きがあるか調べる
 *
 * @param   symbol_save		
 * @param   zone_type		  SYMBOL_ZONE_TYPE
 *
 * @retval  u32		        空きがあった場合：配置No
 * @retval  u32           空きが無い場合：SYMBOL_SPACE_NONE
 */
//==================================================================
u32 SymbolSave_CheckFreeZoneSpace(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_ZONE_TYPE zone_type)
{
  u32 i, start,end;
  SYMBOL_POKEMON *spoke;
  
  start = SymbolZoneTypeDataNo[zone_type].start;
  end = SymbolZoneTypeDataNo[zone_type].end;
  
  spoke = &symbol_save->symbol_poke[start];
  for(i = start; i < end; i++){
    if(spoke->monsno == 0){
      return i;
    }
    spoke++;
  }
  return SYMBOL_SPACE_NONE;
}

//--------------------------------------------------------------
/**
 * 配置NoからSYMBOL_ZONE_TYPEを取得します
 *
 * @param   no		配置No
 *
 * @retval  SYMBOL_ZONE_TYPE		
 */
//--------------------------------------------------------------
static SYMBOL_ZONE_TYPE _Get_No_to_ZoneType(u32 no)
{
  SYMBOL_ZONE_TYPE zone_type;
  
  for(zone_type = 0; zone_type < NELEMS(SymbolZoneTypeDataNo); zone_type++){
    if(no < SymbolZoneTypeDataNo[zone_type].end){
      return zone_type;
    }
  }
  GF_ASSERT_MSG(0, "zone_type = %d", zone_type);
  return SYMBOL_ZONE_TYPE_FREE_SMALL;
}

//==================================================================
/**
 * 開始配置Noを指定してデータの前詰めを行います
 *
 * @param   symbol_save		シンボルセーブ領域へのポインタ
 * @param   no		        配置No
 * @param   zone_type		  SYMBOL_ZONE_TYPE
 *
 * 捕獲してデータを削除する時にもこの関数を使用します
 */
//==================================================================
void SymbolSave_DataShift(SYMBOL_SAVE_WORK *symbol_save, u32 no)
{
  SYMBOL_ZONE_TYPE zone_type;
  u32 start, end;
  
  zone_type = _Get_No_to_ZoneType(no);
  start = no;
  end = SymbolZoneTypeDataNo[zone_type].end;

  GFL_STD_MemCopy(&symbol_save->symbol_poke[start + 1], &symbol_save->symbol_poke[start], 
    sizeof(struct _SYMBOL_POKEMON) * (end - (start + 1)));
  symbol_save->symbol_poke[end - 1].monsno = 0;
}

//==================================================================
/**
 * シンボルポケモンをフリーゾーンに登録
 *
 * @param   symbol_save		シンボルセーブ領域へのポインタ
 * @param   monsno        ポケモン番号
 * @param   wazano		    技番号
 * @param   sex		        性別(PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN)
 * @param   form_no		    フォルム番号
 * @param   zone_type     SYMBOL_ZONE_TYPE
 *
 * 空きが無い場合は先頭のデータをところてん式に追い出して最後尾にセットします
 */
//==================================================================
void SymbolSave_SetFreeZone(SYMBOL_SAVE_WORK *symbol_save, u16 monsno, u16 wazano, u8 sex, u8 form_no, SYMBOL_ZONE_TYPE zone_type)
{
  u32 no, start, end;
  SYMBOL_POKEMON *spoke;

  start = SymbolZoneTypeDataNo[zone_type].start;
  end = SymbolZoneTypeDataNo[zone_type].end;
  
  no = SymbolSave_CheckFreeZoneSpace(symbol_save, zone_type);
  if(no == SYMBOL_SPACE_NONE){
    SymbolSave_DataShift(symbol_save, start);
    spoke = &symbol_save->symbol_poke[end - 1];
  }
  else{
    spoke = &symbol_save->symbol_poke[no];
  }
  spoke->monsno = monsno;
  spoke->wazano = wazano;
  spoke->sex = sex;
  spoke->form_no = form_no;

  //マップの拡張チェック
  if(zone_type == SYMBOL_ZONE_TYPE_FREE_LARGE){
    if(symbol_save->map_level_large < SYMBOL_MAP_LEVEL_LARGE_MAX){
      symbol_save->map_level_large++;
    }
  }
  else if(no != SYMBOL_SPACE_NONE){
    symbol_save->map_level_small = (no - SYMBOL_NO_START_FREE_SMALL) / SMALL_MAP_EXPAND_NUM;
    if(symbol_save->map_level_small > SYMBOL_MAP_LEVEL_SMALL_MAX){
      symbol_save->map_level_small = SYMBOL_MAP_LEVEL_SMALL_MAX;  //一応
      GF_ASSERT_MSG(0, "no=%d", no - SYMBOL_NO_START_FREE_SMALL);
    }
  }
}
