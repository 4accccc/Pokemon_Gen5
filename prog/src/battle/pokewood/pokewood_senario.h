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

#define EVENT_TALK_TURN_MAX     (  7 )  ///< イベント中の会話の最大数
#define EVENT_REACTION_MAX      (  4 )  ///< 選択肢リアクションの最大数
#define ENEMY_POKE_MAX          (  6 )  ///< 敵トレーナーの最大数
#define SELECT_MSG_NUM          (  9 )  ///< 選択時状況メッセージの最大種類
#define SITUATION_MSG_NUM       (  6 )  ///< シチュエーションメッセージの最大種類
#define EVENT_DATA_MAX          (  7 )  ///< イベントデータの最大数
#define POKEWOOD_TALK_MSG_MAX   ( 20 )  ///< ターン毎会話の最大数
#define POKEWOOD_NORMAL_VFX_MAX (  3 )  ///< ターン開始時に背景チェンジを行う最大回数
#define POKEWOOD_CHOICE_STR_MAX ( 20*2 )///< 選択肢文字列の最大長


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
  s16 etc1,etc2;      ///< パディング
  
  s16 bg_change[POKEWOOD_NORMAL_VFX_MAX][2];  ///< 背景チェンジターン
  s16 etc3,etc4;
  
  POKEWOOD_ENEMY_POKE enemy_poke[ENEMY_POKE_MAX]; ///< 敵トレーナーのポケモンの最大数
  s16 select_msg[2][SELECT_MSG_NUM];              ///< メッセージ選択時の会話ID(自分・敵）
  s16 situation_msg[2][SITUATION_MSG_NUM];        ///< 状況別会話ID(自分・敵）
  s16 turn_msg[2][POKEWOOD_TALK_MSG_MAX];         ///< ターン毎の会話ID(自分・敵）
  POKEWOOD_EVENT_DATA event[EVENT_DATA_MAX];      ///< イベントデータ
}POKEWOOD_SENARIO;

extern POKEWOOD_SENARIO *BTL_MAIN_GetPokewoodSenario( BTL_MAIN_MODULE *wk );


#endif
