//=============================================================================================
/**
 * @file    pokewood_senario.h
 * @brief   ポケウッドシナリオヘッダファイル
 * @author  mori
 *
 * @date  2011.03.28  作成
 */
//=============================================================================================
#ifndef __POKEWOOD_SENARIO_H__
#define __POKEWOOD_SENARIO_H__

#include "battle/pokewood.h"

#define EVENT_TALK_TURN_MAX     (  7 )  ///< イベント中の会話の最大数
#define EVENT_REACTION_MAX      (  4 )  ///< 選択肢リアクションの最大数
#define ENEMY_POKE_MAX          (  6 )  ///< 敵トレーナーの最大数
#define SELECT_MSG_NUM          (  9 )  ///< 選択時状況メッセージの最大種類
#define SITUATION_MSG_NUM       (  6 )  ///< シチュエーションメッセージの最大種類
#define EVENT_DATA_MAX          (  7 )  ///< イベントデータの最大数
#define POKEWOOD_TALK_MSG_MAX   ( 20 )  ///< ターン毎会話の最大数
#define POKEWOOD_NORMAL_VFX_MAX (  3 )  ///< ターン開始時に背景チェンジを行う最大回数
#define POKEWOOD_CHOICE_STR_MAX ( 20*2 )///< 選択肢文字列の最大長
#define POKEWOOD_SENARIO_HEADER_SIZE  ( 18 ) ///< 台本画面が参照するシナリオデータの最小バイト数

// 立ち位置AAとBBの状態(PWS_はPOKEWOOD_STAGEの頭文字)
#define PWS_A_POKE    ( 0 ) ///< AAお盆の上にポケモン
#define PWS_A_TRAINER ( 1 ) ///< AAお盆の上にトレーナー
#define PWS_A_NONE    ( 2 ) ///< AAお盆の上に何もない

#define PWS_B_POKE    ( PWS_A_POKE<<4 )     ///< BBお盆の上にポケモン
#define PWS_B_TRAINER ( PWS_A_TRAINER<<4 )  ///< BBお盆の上にトレーナー
#define PWS_B_NONE    ( PWS_A_NONE<<4 )     ///< BBお盆の上に何もない


// シナリオデータから指定されるお盆状態(AAとBBが同時に指定されている）
enum{
  PW_STAGE_BOTH_POKE           =(PWS_B_POKE|PWS_A_POKE),       ///< 0x00 両方ポケモン
  PW_STAGE_A_POKEMON_B_TRAINER =(PWS_B_TRAINER|PWS_A_POKE),    ///< 0x10 敵がトレーナー自分はポケモン
  PW_STAGE_A_TRAINER_B_TRAINER =(PWS_B_TRAINER|PWS_A_TRAINER), ///< 0x11 両方トレーナー

  PW_STAGE_A_TRAINER_B_POKEMON =(PWS_B_POKE|PWS_A_TRAINER),    ///< 0x01 敵がポケモン自分がトレーナー

  PW_STAGE_A_TRAINER_B_NONE    =(PWS_B_NONE|PWS_A_TRAINER),    ///< 0x21 敵側いない自分側トレーナー
  PW_STAGE_A_NONE_B_TRAINER    =(PWS_B_TRAINER|PWS_A_NONE),    ///< 0x12 自分側いない敵側トレーナー
  PW_STAGE_A_NONE_B_NONE       =(PWS_B_NONE|PWS_A_NONE),       ///< 0x22 両方いない
  
  
  // いつかポケモンから無人状態も要望されるか？
  POKEWOOD_STAGE_COND_MAX,
};

// 勝利条件の数値定義
enum{
  PW_PROV_EVERY_TURN=0,     ///< 毎ターン倒す
  PW_PROV_ALL_DEAD,         ///< ターン内に相手を全滅
  PW_PROV_USE_WAZA_DEAD,    ///< 特定の技を使用して全滅
  PW_PROV_TARGET_DEAD1,     ///< 特定の相手を倒す１
  PW_PROV_TARGET_DEAD2,     ///< 特定の相手を倒す２
  PW_PROV_NOT_TARGET_DEAD,  ///< 特定の相手以外を倒す
  PW_PROV_SURVIVE1,         ///< 生き残る１
  PW_PROV_SURVIVE2,         ///< 生き残る２
  PW_PROV_SUCIDE,           ///< 自滅する

};

// ポケウッド敵トレーナー所持ポケモン情報
typedef struct {
  s16 monsno; ///< 開発NO
  s16 sex;    ///< 性別（poke_tool.h参照
  s16 level;  ///< レベル
  s16 etc;    ///< パディング
} POKEWOOD_ENEMY_POKE;

typedef struct{
  s16 my_message;         ///< GMMメッセージID
  s16 enemy_message;      ///< GMMメッセージID
  s16 select_enemy_waza;  ///< 選択した内容によって敵が使用する技がこの番号に固定される
  s16 visual_effect;      ///< 背景チェンジが発生する場合はNOが。無い場合は-1
}POKEWOOD_EVENT_REACTION_DATA;

// ポケウッドイベントデータ
typedef struct {
  s16 type;               ///< イベントタイプ（現在は未使用）
  s16 start_turn;         ///< イベント開始ターン
  s16 flag_provision;     ///< 分岐条件値（ポケウッド内で保持するワークとの比較に使用する値）
  s16 padding;            ///< パディング
  s16 talk_msg[2][EVENT_TALK_TURN_MAX];                      ///< メッセージIDが自分７つ、相手７つまで格納。
  s16 stage_cast[EVENT_TALK_TURN_MAX];                       ///< お盆の上にいるのは役者か？ポケモンか？
  POKEWOOD_EVENT_REACTION_DATA reaction[EVENT_REACTION_MAX]; ///< 選択肢対応データが格納される
}POKEWOOD_EVENT_DATA;

// ポケウッドシナリオデータ
typedef struct{
  s16 senario_no;     ///< シナリオNO
  s16 battle_mode;    ///< シングル・ダブルバトルどちらか
  s16 reguration;     ///< レギュレーション（制限なし・フラット
  s16 pokenum_limit;  ///< 出せるポケモン最大数
  s16 item_ok;        ///< 道具使用OKかどうか
  s16 turn_max;       ///< ターン最大数
  s16 success_event;  ///< 撮影条件イベント
  s16 etc1;           ///< パディング
  
  s16 win_provision;  ///< 勝利条件パターン
  s16 poke_provision[2];   ///< 勝利条件にポケモン指定がある時に参照する（0:撮影 1:上映）
  s16 waza_provision; ///< 勝利条件に技指定がある時に参照する
  s16 trtype[4];      ///< 敵トレーナータイプ（0:男で撮影 1:男で上映 2:女で撮影 3:女で上映）
  s16 mytype[2];      ///< 自分トレーナータイプ（ 0:撮影 1:上映）
  s16 bg_change[POKEWOOD_NORMAL_VFX_MAX][2];  ///< 背景チェンジターン
  
  POKEWOOD_ENEMY_POKE enemy_poke[ENEMY_POKE_MAX]; ///< 敵トレーナーのポケモンの最大数
  s16 select_msg[2][SELECT_MSG_NUM];              ///< メッセージ選択時の会話ID(自分・敵）
  s16 situation_msg[2][SITUATION_MSG_NUM];        ///< 状況別会話ID(自分・敵）
  s16 turn_msg[2][POKEWOOD_TALK_MSG_MAX];         ///< ターン毎の会話ID(自分・敵）
  POKEWOOD_EVENT_DATA event[EVENT_DATA_MAX];      ///< イベントデータ
}POKEWOOD_SENARIO;



#endif
