//======================================================================
/**
 * @file  gym_ground_sv.h
 * @brief  地面ジムセーブデータ
 * @author  Saito
 */
//======================================================================
#pragma once

#define LIFT_NUM_MAX (6)

//地面ジムセーブワーク
typedef struct GYM_GROUND_SV_WORK_tag
{
  BOOL LiftMoved[LIFT_NUM_MAX]; //リフトが基準位置から動いたか　FALSEで基準位置
  BOOL WallOpen;
}GYM_GROUND_SV_WORK;

