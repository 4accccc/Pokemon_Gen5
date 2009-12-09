//======================================================================
/**
 * @file  gym_dragon_sv.h
 * @brief  ドラゴンジムセーブデータ
 * @author  Saito
 */
//======================================================================
#pragma once

#define DRAGON_NUM_MAX (3)

typedef enum {
  ARM_DIR_UP,
  ARM_DIR_DOWN,
  ARM_DIR_MAX
}ARM_DIR;

typedef enum {
  HEAD_DIR_UP,
  HEAD_DIR_DOWN,
  HEAD_DIR_LEFT,
  HEAD_DIR_RIGHT,
  HEAD_DIR_MAX,
}HEAD_DIR;

typedef enum {
  DRA_ARM_LEFT,
  DRA_ARM_RIGHT,
  DRA_ARM_MAX
}DRA_ARM;

typedef struct DRAGON_WORK_tag
{
  HEAD_DIR HeadDir;
  ARM_DIR ArmDir[DRA_ARM_MAX];
}DRAGON_WORK;

//ドラゴンジムセーブワーク
typedef struct GYM_DRAGON_SV_WORK_tag
{
  int dummy;
  DRAGON_WORK DraWk[DRAGON_NUM_MAX];
}GYM_DRAGON_SV_WORK;

