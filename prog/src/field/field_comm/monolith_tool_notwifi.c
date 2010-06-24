//==============================================================================
/**
 * @file    monolith_tool_notwifi.c
 * @brief   not_wifiのオーバーレイに配置されるツール類
 * @author  matsuda
 * @date    2010.06.24(木)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "arc_def.h"
#include "monolith_common.h"
#include "monolith.naix"
#include "monolith_tool.h"
#include "field/intrude_common.h"
#include "intrude_work.h"
#include "msg/msg_monolith.h"
#include "field/monolith_tool_notwifi.h"


//==============================================================================
//  定数定義
//==============================================================================
///Gパワー：片方の条件を満たしていて、もう片方のレベル差分がこの値以内なら灰色
#define _POWER_GRAY_OFFSET    (3)



//--------------------------------------------------------------
/**
 * パワーの習得状況を調べる
 *
 * @param   setup		
 * @param   gpower_id		GパワーID
 *
 * @retval  MONO_USE_POWER		習得状況
 */
//--------------------------------------------------------------
MONO_USE_POWER MonolithToolEx_CheckUsePower(const POWER_CONV_DATA *powerdata, GPOWER_ID gpower_id, const OCCUPY_INFO *occupy, const u8 *gpower_distribution_bit)
{
  u32 distribution_bit = 0;
  
  //パレスパワー
  if(powerdata[gpower_id].level_w == POWER_LEVEL_PALACE){
    return MONO_USE_POWER_NONE; //パレスパワーは習得できない
  }
  
  //配布パワー
  if(powerdata[gpower_id].level_w == POWER_LEVEL_DISTRIBUTION){
    distribution_bit = gpower_distribution_bit[0] | (gpower_distribution_bit[1] << 8);
    if(distribution_bit & (1 << (gpower_id - GPOWER_ID_DISTRIBUTION_START))){
      return MONO_USE_POWER_OK;
    }
    return MONO_USE_POWER_NONE;
  }
  
  //普通のパワー
  if(powerdata[gpower_id].level_w <= occupy->white_level){
    if(powerdata[gpower_id].level_b <= occupy->black_level){
      return MONO_USE_POWER_OK;
    }
    if((s32)powerdata[gpower_id].level_b - occupy->black_level <= _POWER_GRAY_OFFSET){
      return MONO_USE_POWER_SOMEMORE;
    }
  }
  else if(powerdata[gpower_id].level_b <= occupy->black_level){
    if(powerdata[gpower_id].level_w <= occupy->white_level){
      return MONO_USE_POWER_OK;
    }
    if((s32)powerdata[gpower_id].level_w - occupy->white_level <= _POWER_GRAY_OFFSET){
      return MONO_USE_POWER_SOMEMORE;
    }
  }
  return MONO_USE_POWER_NONE;
}


//==================================================================
/**
 * 使用できるパワー数を調べる
 *
 * @param   powerdata		
 * @param   occupy		
 * @param   gpower_distribution_bit		
 *
 * @retval  u32		
 */
//==================================================================
u32 MonolithToolEx_CountUsePower(const POWER_CONV_DATA *powerdata, const OCCUPY_INFO *occupy, const u8 *gpower_distribution_bit)
{
  GPOWER_ID gpower_id;
  u32 count = 0;
  
  for(gpower_id = 0; gpower_id < GPOWER_ID_MAX; gpower_id++){
    if(MonolithToolEx_CheckUsePower(powerdata, gpower_id, occupy, gpower_distribution_bit) == MONO_USE_POWER_OK){
      count++;
    }
  }
  return count;
}

