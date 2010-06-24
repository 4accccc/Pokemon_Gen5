//==============================================================================
/**
 * @file    monolith_tool_notwifi.h
 * @brief   not_wifiに配置されるツール類
 * @author  matsuda
 * @date    2010.06.24(木)
 */
//==============================================================================
#pragma once

#include "gamesystem/g_power.h"

//==============================================================================
//  定数定義
//==============================================================================
///Gパワー習得状況
typedef enum{
  MONO_USE_POWER_OK,        ///<習得している
  MONO_USE_POWER_SOMEMORE,  ///<あと少しで習得
  MONO_USE_POWER_NONE,      ///<習得していない
  
  MONO_USE_POWER_MAX,
}MONO_USE_POWER;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern MONO_USE_POWER MonolithToolEx_CheckUsePower(const POWER_CONV_DATA *powerdata, GPOWER_ID gpower_id, const OCCUPY_INFO *occupy, const u8 *gpower_distribution_bit);
extern u32 MonolithToolEx_CountUsePower(const POWER_CONV_DATA *powerdata, const OCCUPY_INFO *occupy, const u8 *gpower_distribution_bit);
