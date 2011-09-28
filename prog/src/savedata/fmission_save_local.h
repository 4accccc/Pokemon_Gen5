//============================================================================================
/**
 * @file    fmission_save.c
 * @brief   フェスミッションセーブデータ ローカルヘッダ
 * @author　Miyuki Iwasawa
 * @date    11.07.22
 */
//============================================================================================

#pragma once

#include "gamesystem/fest_mission_local.h"

#define FMISSION_STATUS_MAX (64)

#define FMISSION_SAVE_HIGH_SCORE_MAX  ( FMISSION_SCORE_MAX )
#define FMISSION_SAVE_OPEN_NUM_MAX ( 9999 )
#define FMISSION_SAVE_ENTRY_NUM_MAX ( 9999 )
#define FMISSION_SAVE_CLEAR_NUM_MAX ( 9999 )

typedef struct _FMISSION_STATUS{
  u8  level:4;      ///<クリアしているミッションレベル(max3bit)
  u8  new_f:4;      ///<新着フラグ(max1bit)
}FMISSION_STATUS;

struct _FMISSION_SAVE{
  u16 high_score[FMISSION_STATUS_MAX]; ///<ハイスコア(14bit)
  FMISSION_STATUS status[FMISSION_STATUS_MAX];
  u16 open_num;   ///<フェスミッション開催回数
  u16 entry_num;  ///<フェスミッション参加回数
  u16 clear_num;  ///<フェスミッション達成回数
  u8 exp[FMISSION_SV_EXP_IDX_MAX];     ///<経験値white&black
};


