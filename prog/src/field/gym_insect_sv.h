//======================================================================
/**
 * @file  gym_insect_sv.h
 * @brief  虫ジムセーブデータ
 * @author  Saito
 */
//======================================================================
#pragma once

#define INSECT_SW_MAX (8)
#define INSECT_PL_MAX (10)
#define INSECT_WALL_MAX (7)

typedef enum
{
  WALL_ST_NORMAL,
  WALL_ST_WEAKNESS,
}GYM_INSECT_WALL_ST;

//虫ジムセーブワーク
typedef struct GYM_INSECT_SV_WORK_tag
{
  BOOL Sw[INSECT_SW_MAX];    //スイッチ状況     8
  BOOL Pl[INSECT_PL_MAX];     //ポール状況      10
  GYM_INSECT_WALL_ST WallSt[INSECT_WALL_MAX]; //ウォールバンド状態  7
}GYM_INSECT_SV_WORK;

