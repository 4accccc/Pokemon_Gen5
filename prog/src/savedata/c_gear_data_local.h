//============================================================================================
/**
 * @file  c_gear_data_local.h
 * @brief Cギア用セーブデータ構造体定義
 * @date  2011.08.24
 */
//============================================================================================
#pragma once
#include "savedata/c_gear_data.h"

#define CGEAR_PADDING_BUF_NUM          (36)
//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _CGEAR_SAVEDATA {
  u8  custom_col[CGEAR_CUSTOM_COLOR_NUM];
  u8  CGearPowerON;
  u8  CGearON;
  u8  panel_pos[CGEAR_PANEL_MAX];
  u8  gearType;
  
  u16 CGearPictCRC;
  u8  CGearPictureON;
  u8  dummy;
	u8  dummy_buf[CGEAR_PADDING_BUF_NUM];  //CGear-WBで必要としていた領域。位置を変えてパディング領域として取っておく
};

