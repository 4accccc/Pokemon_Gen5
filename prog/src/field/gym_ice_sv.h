//======================================================================
/**
 * @file  gym_ice_sv.h
 * @brief  氷ジムセーブデータ
 * @author  Saito
 */
//======================================================================
#pragma once

#define WALL_NUM_MAX (3)

//氷ジムセーブワーク
typedef struct GYM_ICE_SV_WORK_tag
{
  BOOL WallMoved[WALL_NUM_MAX]; //基準位置から動いたか　FALSEで基準位置
}GYM_ICE_SV_WORK;

