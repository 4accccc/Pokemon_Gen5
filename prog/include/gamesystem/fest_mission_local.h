/*
 *  @file   fest_mission_local.h
 *  @brief  フェスタミッションパラメータ構造体定義
 *  @author Miyuki Iwasawa
 *  @date   11.02.04 
 *  game_beaconxxx.xxx などと共有するために、最小限のリテラル定義や構造体のみ抜き出した
 */

#pragma once

#if 0
typedef enum{
 FEST_MISSION_NULL,
 FEST_MISSION_WILDBTL_WIN,
 FEST_MISSION_ID_MAX,
 FEST_MISSION_ID_END = FEST_MISSION_ID_MAX-1,
}FEST_MISSION_ID;
#endif

/*
 *  増やすとき
 *  1, 定義を追加
 *  3, check src/field/event_debug_linkfest.c
 */
typedef enum{
  FMISSION_TYPE_NULL,
  FMISSION_TYPE_POKE_SEARCH,  ///<指定ポケモンを倒すか捕まえる
  FMISSION_TYPE_EVENT,        ///<指定イベントをクリアする
  FMISSION_TYPE_BALL_ITEM,    ///<ボールアイテムをゲットする
  FMISSION_TYPE_MAX,
  FMISSION_TYPE_END = FMISSION_TYPE_MAX-1,

  FMISSION_TYPE_EX_WITHOUT_PARAM = 0xFF,  ///<ログセット処理用の特殊コード。実際のミッションタイプとしては使いません
}FMISSION_TYPE_ID;

typedef enum{
 FMISSION_WSET_DEFAULT,   ///<デフォルト
 FMISSION_WSET_MONSNAME,  ///<ポケモン種族名
 FMISSION_WSET_ITEMNAME,  ///<アイテム名
 FMISSION_WSET_MAX,
}FMISSION_WSET;

///ミッション終了条件タイプ
typedef enum{
  FMISSION_ENDTYPE_TIME,  ///<時間終了まで継続
  FMISSION_ENDTYPE_CLEAR, ///<目標達成時点で終了
}FMISSION_ENDTYPE;

///typedef宣言はfest_mission.h内
struct _FEST_MISSION{
  u16 type:4;     ///<ミッションタイプ
  u16 time:12;    ///<ミッション制限時間(秒)
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

  u16 act_no;       ///<アクションNo
  u16 act_prm;      ///<アクションパラメータ
};

#define FMISSION_SCORE_MAX  (999) //ミッションスコア最大値
#define FMISSION_LIFE_MAX   (0x3FFFF) //ミッションライフMax

