//============================================================================================
/**
 * @file  c_gear_data_local.h
 * @brief Cギア用セーブデータ構造体定義
 * @date  2011.08.24
 */
//============================================================================================
#pragma once
#include "savedata/c_gear_data.h"
//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _CGEAR_SAVEDATA {
	u8  type[C_GEAR_PANEL_WIDTH][C_GEAR_PANEL_HEIGHT];
  u8  panel_pos[CGEAR_PANEL_MAX];
  u8  ring_col[C_GEAR_RING_NUM];
  u8  CGearPowerON;
  u8  CGearON;
  u8  gearType;
  
  u16 CGearPictCRC;
  u8  CGearPictureON;
};

