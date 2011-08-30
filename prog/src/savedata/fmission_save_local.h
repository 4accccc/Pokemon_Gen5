//============================================================================================
/**
 * @file    fmission_save.c
 * @brief   フェスミッションセーブデータ ローカルヘッダ
 * @author　Miyuki Iwasawa
 * @date    11.07.22
 */
//============================================================================================

#pragma once

#define FMISSION_STATUS_MAX (64)

typedef struct _FMISSION_STATUS{
  u8  level:4;      ///<クリアしているミッションレベル(max3bit)
  u8  new_f:4;      ///<新着フラグ(max1bit)
}FMISSION_STATUS;

struct _FMISSION_SAVE{
  u16 high_score[FMISSION_STATUS_MAX]; ///<ハイスコア(14bit)
  FMISSION_STATUS status[FMISSION_STATUS_MAX];
  u32 open_num;   ///<フェスミッション開催回数
  u32 entry_num;  ///<フェスミッション参加回数
  u32 clear_num;  ///<フェスミッション達成回数
  u16 exp[FMISSION_SV_EXP_IDX_MAX];     ///<経験値white&black
};


