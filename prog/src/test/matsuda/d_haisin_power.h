//==============================================================================
/**
 * @file    d_haisin_power.h
 * @brief   
 * @author  matsuda
 * @date    2010.08.18(水)
 */
//==============================================================================
#pragma once

#include "field/gpower_id.h"


//==============================================================================
//  定数定義
//==============================================================================
///一度にセットできる配信パワー数
#define HAISIN_POWER_MAX    (8)

enum{
  HAISIN_RETURN_MODE_NAMEIN,
  HAISIN_RETURN_MODE_SELFMSG,
  HAISIN_RETURN_MODE_START,
};


//==============================================================================
//  構造体定義
//==============================================================================
///配信パワー設定データ
typedef struct{
  GPOWER_ID g_power_id; //GパワーID
  s32 time;       //秒
  s32 odds;       //確立
}HAISIN_POWER;

///配信パワー設定データグループ
typedef struct{
  HAISIN_POWER hp[HAISIN_POWER_MAX];
  u32 data_num;   //hpに入っているデータ数

  s8 start_time_hour;
  s8 start_time_min;
  s8 end_time_hour;
  s8 end_time_min;
  s32 beacon_space_time;  ///<ビーコン配信間隔
  int movemode;    ///< 動作モード 0ランダム 1じゅんばん
}HAISIN_POWER_GROUP;

typedef struct{
  HAISIN_POWER_GROUP powergroup;
  STRBUF *name;           ///<配信マシン名
  STRBUF *selfmsg;        ///<一言メッセージ
  s32 union_index;        ///<見た目
  int return_mode;
  BOOL bsavedata;   ///< セーブされていたかどうか
}HAISIN_CONTROL_WORK;


//--------------------------------------------------------------
//  セーブ
//--------------------------------------------------------------
///セーブデータ
typedef struct{
  u32 magic_key;    //HAISIN_SAVE_MAGICKEY

  HAISIN_POWER_GROUP powergroup;
  STRCODE name[SAVELEN_PLAYER_NAME + EOM_SIZE];
  STRCODE selfmsg[8 + EOM_SIZE];
  s32 union_index;
  s8 start_time_hour;
  s8 start_time_min;
  s8 end_time_hour;
  s8 end_time_min;
  s32 beacon_space_time;
}HAISIN_SAVEDATA;

///セーブデータ有効を示すマジックキー
#define HAISIN_SAVE_MAGICKEY    (0x3235aef)
