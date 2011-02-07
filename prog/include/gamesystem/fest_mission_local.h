/*
 *  @file   fest_mission_def.h
 *  @brief  フェスタミッションパラメータ構造体定義
 *  @author Miyuki Iwasawa
 *  @date   11.02.04 
 *  game_beaconxxx.xxx などと共有するために、最小限のリテラル定義や構造体のみ抜き出した
 */

#pragma once

typedef enum{
 FEST_MISSION_NULL,
 FEST_MISSION_WILDBTL_WIN,
 FEST_MISSION_ID_MAX,
 FEST_MISSION_ID_END = FEST_MISSION_ID_MAX-1,
}FEST_MISSION_ID;

typedef enum{
  FMISSION_TYPE_WILDBTL_WIN,
  FMISSION_TYPE_MAX,
}FMISSION_TYPE_ID;

typedef enum{
 FMISSION_WSET_DEFAULT,   ///<デフォルト
 FMISSION_WSET_MONSNAME,  ///<ポケモン種族名
 FMISSION_WSET_ITEMNAME,  ///<アイテム名
 FMISSION_WSET_MAX,
}FMISSION_WSET;

typedef struct{
  u8  id;     ///<ミッションID
  u8  time;   ///<ミッション制限時間(分)
  u16 reward_item; ///<報酬のアイテム
  u16 reward_num; ///<報酬の数
  u16 target; ///<達成目標
  u16 param;  ///<汎用パラメータ
  u16 my_score; ///<スコア
  u32 key;  ///<マジックキー
  u32 life; ///<ミッションの寿命(frame)
}FEST_MISSION;

