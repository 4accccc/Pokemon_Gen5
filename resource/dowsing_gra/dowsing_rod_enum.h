//============================================================================
/**
 *  @file   dowsing_rod_enum.h
 *  @brief  ロッドの状態のenum値を記したヘッダーファイル
 *  @author dowsing_area_make.pl
 *  @date   
 *  @note   dowsing_area_make.plで生成されました。
 *
 *  モジュール名：
 */
//============================================================================
#pragma once

typedef enum
{
  ROD_U    =  0,
  ROD_D    =  1,
  ROD_L    =  2,
  ROD_R    =  3,
  ROD_LU    =  4,
  ROD_LD    =  5,
  ROD_RU    =  6,
  ROD_RD    =  7,
  ROD_ABOVE    =  8,
  ROD_NONE    =  9,
  ROD_MAX    =  10,
}
ROD;

#define AREA_WIDTH    (32)
#define AREA_HEIGHT    (24)
#define AREA_ORIGIN_X    (15)
#define AREA_ORIGIN_Y    (11)
