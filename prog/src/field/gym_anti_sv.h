//======================================================================
/**
 * @file  gym_anti_sv.h
 * @brief  アンチジムセーブデータ
 * @author  Saito
 */
//======================================================================
#ifndef __GYM_ANTI_SV_H__

#define __GYM_ANTI_SV_H__

#define ANTI_SW_NUM_MAX  (3)
//アンチジムセーブワーク
typedef struct GYM_ANTI_SV_WORK_tag
{
  u8 Sw[ANTI_SW_NUM_MAX];
  u8 dummy;
}GYM_ANTI_SV_WORK;

#endif  //__GYM_ANTI_SV_H__
