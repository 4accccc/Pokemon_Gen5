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

//↓FMISSION_ID_MAX分あればいいが、構造体サイズを252byteにし、いざという時のパディング領域を確保するため多めに取っている
#define FMISSION_STATUS_MAX (60) //(FMISSION_ID_MAX)

#define FMISSION_SAVE_HIGH_SCORE_MAX  ( FMISSION_SCORE_MAX )
#define FMISSION_SAVE_OPEN_NUM_MAX ( 9999 )
#define FMISSION_SAVE_ENTRY_NUM_MAX ( 9999 )
#define FMISSION_SAVE_CLEAR_NUM_MAX ( 9999 )
#define FMISSION_SAVE_TOP_SCORE_NUM_MAX ( 9999 )

typedef struct _FMISSION_STATUS{
  u32 high_score:14;///<参加者合計のハイスコア記録(ミッションタイプによって、スコアの場合と時間の場合がある)
  u32 my_record:14; ///<自分のハイスコア記録
  u32 level:3;      ///<クリアしているミッションレベル(max3bit)
  u32 new_f:1;      ///<新着フラグ(max1bit)
}FMISSION_STATUS;

struct _FMISSION_SAVE{
  FMISSION_STATUS status[FMISSION_STATUS_MAX];
  u16 open_num;   ///<フェスミッション開催回数
  u16 entry_num;  ///<フェスミッション参加回数
  u16 clear_num;  ///<フェスミッション達成回数
  u16 top_score_num;	///<トップスコアを取った回数
  u8  exp[FMISSION_SV_EXP_IDX_MAX];     ///<経験値white&black
  u8  max_join_num;  ///<最大参加者数  
  u8  dummy;  //パディング
};


