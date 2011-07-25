//============================================================================================
/**
 * @file    fmission_save.c
 * @brief   フェスミッションセーブデータ ローカルヘッダ
 * @author　Miyuki Iwasawa
 * @date    11.07.22
 */
//============================================================================================

#pragma once

#define FMISSION_STATUS_MAX (50)

typedef struct _FMISSION_STATUS{
  u16 high_score; ///<ハイスコア(14bit)
  u8  level;      ///<クリアしているミッションレベル(2bit)
  u8  new_f;      ///<新着フラグ(1bit)
}FMISSION_STATUS;

struct _FMISSION_SAVE{
  FMISSION_STATUS status[FMISSION_STATUS_MAX];
  u32 open_num;   ///<フェスミッション開催回数
  u32 entry_num;  ///<フェスミッション参加回数
  u32 clear_num;  ///<フェスミッション達成回数
};


