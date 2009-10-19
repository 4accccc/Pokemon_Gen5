//======================================================================
/**
 * @file  gym_anti_sv.h
 * @brief  アンチジムセーブデータ
 * @author  Saito
 */
//======================================================================
#pragma once

#define ANTI_SW_NUM_MAX  (9)
#define ANTI_DOOR_NUM_MAX  (3)
//アンチジムセーブワーク
typedef struct GYM_ANTI_SV_WORK_tag
{
  u8 Door[ANTI_DOOR_NUM_MAX];
  u8 PushSwIdx;
}GYM_ANTI_SV_WORK;

