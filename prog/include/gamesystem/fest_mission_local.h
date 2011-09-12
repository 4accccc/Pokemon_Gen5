/*
 *  @file   fest_mission_local.h
 *  @brief  フェスタミッションパラメータ構造体定義
 *  @author Miyuki Iwasawa
 *  @date   11.02.04 
 *  game_beaconxxx.xxx などと共有するために、最小限のリテラル定義や構造体のみ抜き出した
 */

#pragma once

#include "fest_mission_data.h"
//#include "fmission_data_def.h"

typedef enum{
 FMISSION_WSET_DEFAULT,   ///<デフォルト
 FMISSION_WSET_MONSNAME,  ///<ポケモン種族名
 FMISSION_WSET_ITEMNAME,  ///<アイテム名
 FMISSION_WSET_EVENT,     ///<イベント
 FMISSION_WSET_MAX,
}FMISSION_WSET;

///typedef宣言はfest_mission.h内(構造体サイズが20byteを越えてはいけない)
struct _FEST_BEACON{
  //絶対必要
  u32  key;              ///<マジックキー
  
  u32  life:18;          ///<ミッションの残り時間(frame)
  u32  my_score:14;      ///<マイスコア(max9999)

  u32  m_id:7;           ///<ミッションNo(127) FEST_MISSION_ID
  u32  m_level:3;        ///<ミッションレベル(0-3) FMISSION_LEVEL
  u32  end_type:1;       ///<ミッション終了条件タイプ FMISSION_END_TYPE
  u32  pay:7;            ///<報酬の基礎値(127)
  u32  goal:14;          ///<達成目標(max9999)

  u32  pay_bonus:2;      ///<報酬の計算式タイプ(0-2) FMISSION_END_TYPE
  u32  time_bonus:4;     ///<スコアアップ時のタイムボーナス FMISSION_TBONUS
  u32  item:10;          ///<報酬のアイテムNo(1023)
  u32  sp_item:10;       ///<スペシャル報酬のアイテムNo(1023)
  u32  sp_pay:6;         ///<スペシャル報酬の基礎値(max50)
  
  u32  act_param:10;     ///<アクションパラメータ
  u32  base_param:10;    ///<汎用パラメータ 基礎値(1023)
  u32  ex_param:10;      ///<汎用パラメータ 拡張値(1023)
  u32  distribution_f:1; ///<配信ミッションフラグ
  u32  version_idx:1;    ///<開催バージョンIndex(FMISSION_VERSION_WHITE / FMISSION_VERSION_BLACK )
};

///typedef宣言はfest_mission.h内(構造体サイズが20byteを越えてはいけない)
struct _FEST_MISSION{
#if 1
  FEST_BEACON beacon;
  FMISSION_ARC_COMMON arc;
#elif 0
  //絶対必要
  u32  key;              ///<マジックキー
  
  u32  life:18;          ///<ミッションの残り時間(frame)
  u32  my_score:14;      ///<マイスコア(max9999)

  u32  m_id:7;           ///<ミッションNo(127)
  u32  m_level:3;        ///<ミッションレベル(0-3)
  u32  end_type:1;       ///<ミッション終了条件タイプ
  u32  pay:7;            ///<報酬の基礎値(127)
  u32  goal:14;          ///<達成目標(max9999)

  u32  pay_bonus:2;      ///<報酬の計算式タイプ(0-2)
  u32  time_bonus:4;     ///<スコアアップ時のタイムボーナス
  u32  item:10;          ///<報酬のアイテムNo(1023)
  u32  sp_item:10;       ///<スペシャル報酬のアイテムNo(1023)
  u32  sp_pay:6;         ///<スペシャル報酬の基礎値(max50)
  
  u32  act_param:10;     ///<アクションパラメータ
  u32  param1:10;        ///<汎用パラメータ1(1023)
  u32  param2:10;        ///<汎用パラメータ2(1023)
  u32  distribution_f:1; ///<配信ミッションフラグ
  u32  version_f:1;      ///<開催バージョンフラグ(0ならW,1ならB)

  //
  u32  unit:3;           ///<単位系(0-4)
  u32  m_type:4;         ///<ミッションタイプ(0-5)
  //開催＆参加時に必要
//  u32 event_flag; ///<参加制限イベントフラグID
//  u32 host;       ///<開催制限
  //表示に必要
//  u32 location; ///<範囲

  //開催時だけあればOK
//  u32 m_time:12;       ///<ミッション制限時間(秒 4095秒まで)
#else
  u16 param;      ///<汎用パラメータ
  u16 item;       ///<報酬のアイテムNo

  u8  pay;        ///<報酬の基礎値
  u8  m_id;       ///<ミッションNo
  u8  m_level;    ///<ミッションレベル
  u8  m_type:4;   ///<ミッションタイプ
  u8  time_bonus:4;     ///<スコアアップ時のタイムボーナス

  u32 goal:14;          ///<達成目標(9999)
  u32 my_score:14;      ///<マイスコア(9999)
  u32 distribution_f:1; ///<配信ミッションフラグ
  u32 end_type:1;       ///<ミッション終了条件タイプ
  u32 pay_bonus:2;      ///<報酬の計算式タイプ

  u32 life:18;         ///<ミッションの残り時間(frame)
  u32 m_time:12;       ///<ミッション制限時間(秒 4095秒まで)
  u32 :2;
  u32 key;             ///<マジックキー
#endif
};

#define FMISSION_GOAL_NULL  (0x3FFF)  //ミッション目標無効値
#define FMISSION_SCORE_MAX  (999)     //ミッションスコア最大値
#define FMISSION_LIFE_MAX   (((59*60)+59)*60) //ミッションライフMax(59分59秒)

