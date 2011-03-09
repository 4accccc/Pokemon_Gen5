//======================================================================
/**
 * @file  gym_fly_sv.h
 * @brief  飛行ジムセーブデータ
 * @author  Saito
 */
//======================================================================
#pragma once

//飛行ジムセーブワーク
typedef struct GYM_FLY_SV_WORK_tag
{
  BOOL  time_flag;  // 時間計測開始フラグ
  u16   time;       // 時間計測

  u16   dummy;
}GYM_FLY_SV_WORK;

