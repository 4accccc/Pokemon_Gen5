/*
 *  @file   fest_mission_local.h
 *  @brief  フェスタミッションパラメータ構造体定義
 *  @author Miyuki Iwasawa
 *  @date   11.02.04 
 *  game_beaconxxx.xxx などと共有するために、最小限のリテラル定義や構造体のみ抜き出した
 */

#pragma once

#include "fmission_data_def.h"

typedef enum{
 FMISSION_WSET_DEFAULT,   ///<デフォルト
 FMISSION_WSET_MONSNAME,  ///<ポケモン種族名
 FMISSION_WSET_ITEMNAME,  ///<アイテム名
 FMISSION_WSET_EVENT,     ///<イベント
 FMISSION_WSET_MAX,
}FMISSION_WSET;

///typedef宣言はfest_mission.h内(構造体サイズが20byteを越えてはいけない)
struct _FEST_MISSION{
#if 1
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
#else
  u16 type:4;     ///<ミッションタイプ
  u16 time:12;    ///<ミッション制限時間(秒 4095秒まで)
  u16 param;      ///<汎用パラメータ
  
  u32 target:10;    ///<達成目標
  u32 item:10;      ///<報酬のアイテムNo 
  u32 item_num:8;   ///<報酬数の基礎値
  u32 calc_type:3;  ///<報酬計算式タイプ
  u32 end_type:1;   ///<終了条件タイプ
  
  u32 bonus:4;      ///<スコアボーナス
  u32 life:18;      ///<ミッション寿命(frame)
  u32 my_score:10;  ///<スコア

  u32 key;          ///<マジックキー

  u16 dummy1;
  u16 dummy2;
//  u16 act_no;       ///<アクションNo
//  u16 act_prm;      ///<アクションパラメータ
#endif
};

#define FMISSION_GOAL_NULL  (0x3FFF)  //ミッション目標無効値
#define FMISSION_SCORE_MAX  (999)     //ミッションスコア最大値
#define FMISSION_LIFE_MAX   (0x3FFFF) //ミッションライフMax

