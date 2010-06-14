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



//--------------------------------------------------------------
/**
 * @brief   暗号化
 * @param   rec   
 */
//--------------------------------------------------------------
void SymbolSave_Local_Encode(SYMBOL_SAVE_WORK *pSymbol)
{
  GF_ASSERT(pSymbol->crc.coded_number==0);

  pSymbol->crc.coded_number = OS_GetVBlankCount() | (OS_GetVBlankCount() << 8);
  if(pSymbol->crc.coded_number==0){
    pSymbol->crc.coded_number = 1;
  }
  pSymbol->crc.crc16ccitt_hash = GFL_STD_CODED_CheckSum(pSymbol, sizeof(SYMBOL_SAVE_WORK) - sizeof(RECORD_CRC));
  GFL_STD_CODED_Coded(pSymbol, sizeof(SYMBOL_SAVE_WORK) - sizeof(RECORD_CRC), 
    pSymbol->crc.crc16ccitt_hash + (pSymbol->crc.coded_number << 16));
}

//--------------------------------------------------------------
/**
 * @brief   復号化
 * @param   rec   
 */
//--------------------------------------------------------------
void SymbolSave_Local_Decode(SYMBOL_SAVE_WORK *pSymbol)
{
  GF_ASSERT(pSymbol->crc.coded_number!=0);

  GFL_STD_CODED_Decoded(pSymbol, sizeof(SYMBOL_SAVE_WORK) - sizeof(RECORD_CRC), 
    pSymbol->crc.crc16ccitt_hash + (pSymbol->crc.coded_number << 16));

  pSymbol->crc.coded_number = 0;
}


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
  SymbolSave_Local_Encode(symbol_save);
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
 * 配置NoからSYMBOL_ZONE_TYPEを取得します
 *
 * @param   no		配置No
 *
 * @retval  SYMBOL_ZONE_TYPE		
 */
//==================================================================
static SYMBOL_ZONE_TYPE _GetZoneTypeFromNumber(u32 no)
{
  SYMBOL_ZONE_TYPE zone_type;
  SYMBOL_ZONE_TYPE zone_typeret = SYMBOL_ZONE_TYPE_FREE_SMALL;

  for(zone_type = 0; zone_type < NELEMS(SymbolZoneTypeDataNo); zone_type++){
    if(no < SymbolZoneTypeDataNo[zone_type].end){
      zone_typeret=zone_type;
      break;
    }
  }
  GF_ASSERT_MSG(zone_type < NELEMS(SymbolZoneTypeDataNo), "zone_type = %d", zone_type);
  return zone_typeret;
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
static u32 _CheckSpace(const SYMBOL_SAVE_WORK *symbol_save, SYMBOL_ZONE_TYPE zone_type)
{
  u32 i, start,end;
  u32 retcode = SYMBOL_SPACE_NONE;
  const SYMBOL_POKEMON *spoke;
  
  GF_ASSERT( zone_type < SYMBOL_ZONE_TYPE_KEEP_ALL );

  start = SymbolZoneTypeDataNo[zone_type].start;
  end = SymbolZoneTypeDataNo[zone_type].end;
  
  spoke = &symbol_save->symbol_poke[start];
  for(i = start; i < end; i++){
    if(spoke->monsno == 0){
      retcode = i;
      break;
    }
    spoke++;
  }
  return retcode;
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
static void _DataShift(SYMBOL_SAVE_WORK *symbol_save, u32 no)
{
  SYMBOL_ZONE_TYPE zone_type;
  u32 start, end;

  zone_type = _GetZoneTypeFromNumber(no);
  start = no;
  end = SymbolZoneTypeDataNo[zone_type].end;

  if ( start < end ) {
    GFL_STD_MemCopy(&symbol_save->symbol_poke[start + 1], &symbol_save->symbol_poke[start], 
      sizeof(struct _SYMBOL_POKEMON) * (end - (start + 1)));
  }
  symbol_save->symbol_poke[end - 1].monsno = 0;

}



//==================================================================
/**
 * @brief シンボルポケモンデータを取得
 * @param   symbol_save
 * @param   no		配置No
 * @return  SYMBOL_POKEMON  シンボルポケモンデータへのポインタ
 */
//==================================================================

BOOL SymbolSave_GetSymbolPokemon(SYMBOL_SAVE_WORK *symbol_save, u32 no, SYMBOL_POKEMON* pSymbol)
{
  BOOL bRet;
  GF_ASSERT( no < SYMBOL_POKE_MAX );

  if(no >= SYMBOL_POKE_MAX){
    return FALSE;
  }
  SymbolSave_Local_Decode(symbol_save);
  GFL_STD_MemCopy( &symbol_save->symbol_poke[ no ],pSymbol, sizeof(SYMBOL_POKEMON));
  SymbolSave_Local_Encode(symbol_save);
  return TRUE;
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
u32 SymbolSave_CheckSpace( SYMBOL_SAVE_WORK *symbol_save, SYMBOL_ZONE_TYPE zone_type)
{
  u32 retcode;
  
  GF_ASSERT( zone_type < SYMBOL_ZONE_TYPE_KEEP_ALL );

  SymbolSave_Local_Decode(symbol_save);
  retcode = _CheckSpace(symbol_save, zone_type);
  SymbolSave_Local_Encode(symbol_save);
  return retcode;
}

//==================================================================
/**
 * 配置NoからSYMBOL_ZONE_TYPEを取得します
 *
 * @param   no		配置No
 *
 * @retval  SYMBOL_ZONE_TYPE		
 */
//==================================================================
SYMBOL_ZONE_TYPE SYMBOLZONE_GetZoneTypeFromNumber(u32 no)
{
  SYMBOL_ZONE_TYPE zone_type;

  zone_type = _GetZoneTypeFromNumber(no);
  return zone_type;
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

  SymbolSave_Local_Decode(symbol_save);
  _DataShift(symbol_save, no);
  SymbolSave_Local_Encode(symbol_save);

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
 * @param   move_type     動作タイプ指定
 * @param   zone_type     SYMBOL_ZONE_TYPE
 *
 * 空きが無い場合は先頭のデータをところてん式に追い出して最後尾にセットします
 */
//==================================================================
void SymbolSave_SetFreeZone(SYMBOL_SAVE_WORK *symbol_save,
    u16 monsno, u16 wazano, u8 sex, u8 form_no, u8 move_type, SYMBOL_ZONE_TYPE zone_type)
{
  u32 no, start, end;
  SYMBOL_POKEMON *spoke;

  GF_ASSERT( zone_type == SYMBOL_ZONE_TYPE_FREE_LARGE || zone_type == SYMBOL_ZONE_TYPE_FREE_SMALL );

  SymbolSave_Local_Decode(symbol_save);
  
  start = SymbolZoneTypeDataNo[zone_type].start;
  end = SymbolZoneTypeDataNo[zone_type].end;
  
  no = _CheckSpace(symbol_save, zone_type);
  if(no == SYMBOL_SPACE_NONE){
    _DataShift(symbol_save, start);
    spoke = &symbol_save->symbol_poke[end - 1];
  }
  else{
    spoke = &symbol_save->symbol_poke[no];
  }
  spoke->monsno = monsno;
  spoke->wazano = wazano;
  spoke->sex = sex;
  spoke->form_no = form_no;
  spoke->move_type = move_type;

  //マップの拡張チェック
  if(zone_type == SYMBOL_ZONE_TYPE_FREE_LARGE){
    if(symbol_save->map_level_large < SYMBOL_MAP_LEVEL_LARGE_MAX){
      symbol_save->map_level_large++;
    }
  }
  else if(zone_type == SYMBOL_ZONE_TYPE_FREE_SMALL)
  {
    if (no != SYMBOL_SPACE_NONE && no >= SYMBOL_NO_START_FREE_SMALL + SMALL_MAP_EXPAND_NUM * 2)
    {
      symbol_save->map_level_small = (no - SYMBOL_NO_START_FREE_SMALL) / SMALL_MAP_EXPAND_NUM - 1;
      if(symbol_save->map_level_small > SYMBOL_MAP_LEVEL_SMALL_MAX)
      {
        symbol_save->map_level_small = SYMBOL_MAP_LEVEL_SMALL_MAX;  //一応
        GF_ASSERT_MSG(0, "no=%d", no - SYMBOL_NO_START_FREE_SMALL);
      }
    }
  }
  SymbolSave_Local_Encode(symbol_save);
}

//==================================================================
/**
 * シンボル不正チェック：指定したシンボルポケモンがフラッシュに記録されているか調べる
 *
 * @param   ctrl		
 * @param   spoke		
 * @param   heap_id		
 * @param   pp_mode   TRUE:PokemonParamから逆引きして判定に使用出来る値だけを参照して不正チェック
 *
 * @retval  int		    
 *
 * 直接フラッシュからロードしてきて、比較を行います
 * シンボルポケモンの新規追加はPDWで行われる為、ゲーム内で登場しているシンボルポケモンのデータは
 * 必ずフラッシュ上に存在しているはず、という前提のチェックです
 */
//==================================================================
int SymbolSave_CheckFlashLoad(SAVE_CONTROL_WORK *ctrl, const SYMBOL_POKEMON *spoke, HEAPID heap_id, BOOL pp_mode)
{
  BOOL load_ret, result = FALSE;
  SYMBOL_SAVE_WORK *load_symbol;
  int i, side;
  enum{
    _SIDE_A,
    _SIDE_B,
    _SIDE_NUM,
  };
  
  load_symbol = GFL_HEAP_AllocClearMemory(heap_id, sizeof(SYMBOL_SAVE_WORK));
  for(side = 0; side < _SIDE_NUM; side++){
    load_ret = SaveControl_PageFlashLoad(
      ctrl, GMDATA_ID_SYMBOL, side, load_symbol, sizeof(SYMBOL_SAVE_WORK));
    if(load_ret == TRUE){
      SymbolSave_Local_Decode(load_symbol);
      for(i = 0; i < SYMBOL_POKE_MAX; i++){
        if(pp_mode == TRUE){
          if(load_symbol->symbol_poke[i].monsno == spoke->monsno
              && load_symbol->symbol_poke[i].form_no == spoke->form_no){
            result = TRUE;
            break;
          }
        }
        else{
          if(GFL_STD_MemComp(&load_symbol->symbol_poke[i], spoke, sizeof(SYMBOL_POKEMON)) == 0){
            result = TRUE;
            break;
          }
        }
      }
    }
    if(result == TRUE){
      break;
    }
  }
  GFL_HEAP_FreeMemory(load_symbol);
  
  return result;
}
