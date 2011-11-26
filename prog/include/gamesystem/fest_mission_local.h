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

/*
 * FEST_BEACONデータ中で、同一ミッションかどうかの判定に用いるパラメータ群
 * ぴったり4byteである必要がある
 */
union _FEST_BEACON_ID{
  struct{
    u32  m_id:7;           ///<ミッションNo(127) FEST_MISSION_ID
    u32  m_level:3;        ///<ミッションレベル(0-3) FMISSION_LEVEL
    u32  base_param:10;    ///<汎用パラメータ 基礎値(1023)
    u32  ex_param:10;      ///<汎用パラメータ 拡張値(1023)
    u32  distribution_f:1; ///<配信ミッションフラグ
    u32  version_idx:1;    ///<開催バージョンIndex(FMISSION_VERSION_WHITE / FMISSION_VERSION_BLACK )
  };
  u32 id_param;
};

///typedef宣言はfest_mission.h内(構造体サイズが20byteを越えてはいけない)
struct _FEST_BEACON{
  //絶対必要
  FEST_BEACON_ID;        ///<ビーコンIdentityパラメータ(メンバを直接参照したいので敢えて名無し)

  union{
#if 0
    struct{
      u32  act_param:10;     ///<アクションパラメータ
      u32  end_type:1;       ///<ミッション終了条件タイプ FMISSION_END_TYPE
      u32  pay:7;            ///<報酬の基礎値(127)
      u32  goal:14;          ///<達成目標(max9999)

      u32  pay_bonus:2;      ///<報酬の計算式タイプ(0-2) FMISSION_PAY_BONUS
      u32  time_bonus:4;     ///<スコアアップ時のタイムボーナス FMISSION_TBONUS
      u32  item:10;          ///<報酬のアイテムNo(1023)
      u32  sp_item:10;       ///<スペシャル報酬のアイテムNo(1023)
      u32  sp_pay:6;         ///<スペシャル報酬の基礎値(max50)
  
      u32  time:12;          ///<ミッションの制限時間(sec max=FMISSION_TIME_MAX=3599)
      u32  :20;               

      u32  dummy;            ///<パディング
    };
#else
    struct{
      u32  item:10;          ///<報酬のアイテムNo(1023)
      u32  pay:7;            ///<報酬の基礎値(127)
      u32  goal:14;          ///<達成目標(max9999)
      u32  end_type:1;       ///<ミッション終了条件タイプ FMISSION_END_TYPE
      
      u32  pattern:18;       ///<出現パターン18bit FMISSION_POINT_PATTERN_BIT_NUM*FMISSION_POINT_PAT_MAX(3bit*6パターン)
      u32  entry_flag_id:12; ///<参加条件フラグID
      u32  pay_bonus:2;      ///<報酬の計算式タイプ(0-2) FMISSION_PAY_BONUS
      
      u32  act_param:10;     ///<アクションパラメータ
      u32  time:12;          ///<ミッションの制限時間(sec max=FMISSION_TIME_MAX=3599)
      u32  sp_item:10;       ///<スペシャル報酬のアイテムNo(1023)

      u16  sp_pay:6;         ///<スペシャル報酬の基礎値(max50)
      u16  unit:3;           ///<表示単位 FMISSION_UNIT_ID
      u16  time_bonus:2;     ///<スコアアップ時のタイムボーナス FMISSION_TBONUS
      u16  dummy:5;

      u8   m_title_id;       ///<タイトルメッセージID
      u8   m_exp_id;         ///<説明文メッセージID
    };
#endif
    STRCODE fword[FMISSION_FWORD_MESSAGE_LEN];  ///<フリーワードビーコン用(_EOM無し8文字)
  };
};

///typedef宣言はfest_mission.h内(FEST_BEACON構造体サイズが20byteを越えてはいけない)
struct _FEST_MISSION{
  FEST_BEACON beacon;
  FMISSION_ARC_COMMON arc;
  u16 my_score;   //マイスコア max=FMISSION_SCORE_MAX=9999(14bitでOK)
  u8  fword_sid;  //フリーワード用ショートカットID NAMEIN_SHORTCUT_RET(2bitあればOK)
  u32 key;        //マジックキー
  s32 end_life;   //ミッション終了時の残り時間(秒)
  s64 start_rtc;  //ミッション開始時RTC(日付+時刻の秒変換値)
};

#define FMISSION_GOAL_NULL  (0x3FFF)  //ミッション目標無効値
#define FMISSION_SCORE_MAX  (9999)    //ミッションスコア最大値
#define FMISSION_TIME_MAX   (((59*60)+59)) //ミッション制限時間Max(59分59秒)
#define FMISSION_RTC_NULL   (0xFFFFFFFF) //ミッションスタート時RTC無効値
