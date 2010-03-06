//==============================================================================
/**
 * @file    symbol_save_field.c
 * @brief   シンボルエンカウント用セーブデータ：fieldmapオーバーレイに配置
 * @author  matsuda
 * @date    2010.03.03(水)
 *
 * フィールドでしか使用しないシンボルエンカウントセーブアクセス群
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/symbol_save.h"
#include "savedata/symbol_save_notwifi.h"
#include "savedata/symbol_save_field.h"
#include "symbol_save_local.h"



//==================================================================
/**
 * キープゾーンに空きがあるか調べる
 *
 * @param   symbol_save		
 * @param   zone_type		  SYMBOL_ZONE_TYPE
 *
 * @retval  u32		        空きがあった場合：配置No
 * @retval  u32           空きが無い場合：SYMBOL_SPACE_NONE
 */
//==================================================================
u32 SymbolSave_Field_CheckKeepZoneSpace(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_ZONE_TYPE zone_type)
{
  u32 i, start,end;
  
  start = SymbolZoneTypeDataNo[zone_type].start;
  end = SymbolZoneTypeDataNo[zone_type].end;
  
  for(i = start; i < end; i++){
    if(symbol_save->symbol_poke[i].monsno == 0){
      return i;
    }
  }
  return SYMBOL_SPACE_NONE;
}

//==================================================================
/**
 * フリーゾーンにいるポケモンをキープゾーンに移動
 *
 * @param   symbol_save		
 * @param   now_no		    移動対象のポケモンの配置No
 */
//==================================================================
void SymbolSave_Field_Move_FreeToKeep(SYMBOL_SAVE_WORK *symbol_save, u32 now_no)
{
  u32 no, end;
  SYMBOL_ZONE_TYPE zone_type;
  
  if(now_no >= SYMBOL_NO_START_FREE_SMALL){
    zone_type = SYMBOL_ZONE_TYPE_KEEP_SMALL;
  }
  else if(now_no >= SYMBOL_NO_START_FREE_LARGE){
    zone_type = SYMBOL_ZONE_TYPE_KEEP_LARGE;
  }
  else{
    GF_ASSERT_MSG(0, "now_no = %d", now_no);
    return;
  }
  end = SymbolZoneTypeDataNo[zone_type].end;
  
  no = SymbolSave_Field_CheckKeepZoneSpace(symbol_save, zone_type);
  if(no == SYMBOL_SPACE_NONE){ //一応
    GF_ASSERT(no != SYMBOL_SPACE_NONE);
    return;
  }
  
  symbol_save->symbol_poke[no] = symbol_save->symbol_poke[now_no];
  
  //フリーゾーンを前詰め
  if(now_no + 1 < end){
    SymbolSave_DataShift(symbol_save, now_no);
  }
  symbol_save->symbol_poke[end - 1].monsno = 0;
}

//==================================================================
/**
 * キープゾーンにいるポケモンをフリーゾーンに移動
 *
 * @param   symbol_save		
 * @param   keep_no		      移動対象のポケモンの配置No
 */
//==================================================================
void SymbolSave_Field_Move_KeepToFree(SYMBOL_SAVE_WORK *symbol_save, u32 keep_no)
{
  u32 no, end;
  SYMBOL_ZONE_TYPE zone_type;
  SYMBOL_POKEMON *spoke = &symbol_save->symbol_poke[keep_no];
  
  if(keep_no >= SYMBOL_NO_START_FREE_LARGE){
    GF_ASSERT_MSG(0, "keep_no = %d", keep_no);
    return;
  }
  else if(keep_no >= SYMBOL_NO_START_KEEP_SMALL){
    zone_type = SYMBOL_ZONE_TYPE_FREE_SMALL;
  }
  else{
    zone_type = SYMBOL_ZONE_TYPE_FREE_LARGE;
  }
  end = SymbolZoneTypeDataNo[zone_type].end;
  
  //キープゾーンからフリーゾーンへ移動する時は必ずフリーの空きがある事前提
  //自分で自分のデータを潰さないようにする為
  GF_ASSERT(SymbolSave_CheckFreeZoneSpace(symbol_save, zone_type) != SYMBOL_SPACE_NONE);
  
  SymbolSave_SetFreeZone(symbol_save, 
    spoke->monsno, spoke->wazano, spoke->sex, spoke->form_no, zone_type);

  //キープゾーンを前詰め
  if(keep_no + 1 < end){
    SymbolSave_DataShift(symbol_save, keep_no);
  }
  symbol_save->symbol_poke[end - 1].monsno = 0;
}
