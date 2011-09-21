//============================================================================================
/**
 * @file  c_gear_data_local.h
 * @brief Cギア用セーブデータ構造体定義
 * @date  2011.08.24
 */
//============================================================================================
#pragma once
#include "savedata/c_gear_data.h"

#define CGEAR_OLD_PANEL_WIDTH_HEIGHT   (9*4)

//----------------------------------------------------------
/**
 * @brief  構造体定義
 */
//----------------------------------------------------------

struct _CGEAR_SAVEDATA {
	u8  dummy[CGEAR_OLD_PANEL_WIDTH_HEIGHT];  //CGear-WBで必要としていた領域。パディング領域として取っておく
  u8  panel_pos[CGEAR_PANEL_MAX];
  u8  ring_col[CGEAR_RING_NUM];
  u8  CGearPowerON;
  u8  CGearON;
  u8  gearType;
  
  u16 CGearPictCRC;
  u8  CGearPictureON;
};

