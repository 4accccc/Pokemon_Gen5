//==============================================================================
/**
 * @file    colosseum_types.h
 * @brief   コロシアムで使用する定義類
 * @author  matsuda
 * @date    2009.07.17(金)
 */
//==============================================================================
#pragma once

#include "net_app/union/comm_player.h"
#include "app\trainer_card.h"
#include "field/field_msgbg.h"
#include "net_app/comm_entry_menu.h"


//==============================================================================
//  定数定義
//==============================================================================
///コロシアムに参加出来る最大人数
#define COLOSSEUM_MEMBER_MAX    (4)

///立ち位置についていない(COLOSSEUM_PARENT_WORK.stand_positionの値)
#define COLOSSEUM_STANDING_POSITION_NULL    (0xff)

///どのPOKEPARTYを使用するか
enum{
  COLOSSEUM_SELECT_PARTY_TEMOTI,    ///<手持ち
  COLOSSEUM_SELECT_PARTY_BOX,       ///<バトルボックス
  COLOSSEUM_SELECT_PARTY_CANCEL,    ///<やめる
};

//==============================================================================
//  構造体定義
//==============================================================================
///各プレイヤーの基本情報
typedef struct{
  STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];  ///<名前
  u32 id;               ///< ID(トレーナーカードにかかれているID)
  u8 mac_address[6];
  u8 sex;
  u8 trainer_view;
  u8 occ;               ///< TRUE:データ有効
  u8 force_entry;       ///< TRUE:強制エントリー
  u8 battle_server_version;    ///< バトルサーバーバージョン
  u8 padding;
}COLOSSEUM_BASIC_STATUS;

///親だけが持つシステムデータ
typedef struct{
  u8 stand_position[COLOSSEUM_MEMBER_MAX];  ///<各メンバーがどの立ち位置にいるか
  u8 answer_stand_position[COLOSSEUM_MEMBER_MAX];   ///<立ち位置確認を送ってきたメンバーへの返事
  u8 battle_ready[COLOSSEUM_MEMBER_MAX];    ///<各メンバーが戦闘準備できたか
  u8 battle_ready_cancel[COLOSSEUM_MEMBER_MAX];    ///<各メンバーから戦闘準備キャンセル要求
}COLOSSEUM_PARENT_WORK;

///自分のローカルデータ
typedef struct{
  u8 stand_position;          ///<自分の立ち位置
  u8 answer_stand_position;   ///<立ち位置使用許可の返事受信バッファ
  u8 entry_answer;            ///<エントリー結果
  u8 battle_ready;            ///<TRUE:戦闘準備完了
}COLOSSEUM_MINE_WORK;

///受信バッファ
typedef struct{
  COMM_PLAYER_PACKAGE comm_player_pack[COLOSSEUM_MEMBER_MAX];     ///<通信相手の座標
  u8 comm_player_pack_update[COLOSSEUM_MEMBER_MAX];               ///<TRUE:更新あり

  TR_CARD_DATA *tr_card[COLOSSEUM_MEMBER_MAX];         ///<通信相手のトレーナーカード情報
  u8 tr_card_occ[COLOSSEUM_MEMBER_MAX];                ///<TRUE:トレーナーカードデータ有効

  POKEPARTY *pokeparty[COLOSSEUM_MEMBER_MAX];          ///<通信相手のポケモンパーティ
  u8 pokeparty_occ[COLOSSEUM_MEMBER_MAX];              ///<TRUE:POKEPARTY有効
  
  u8 stand_position[COLOSSEUM_MEMBER_MAX];  ///<各メンバーがどの立ち位置にいるか
  u8 stand_position_occ;                    ///<TRUE:データ有効
  
  u8 pokelist_selected_num;                 ///<ポケモンリスト選択完了人数
  
  u8 leave[COLOSSEUM_MEMBER_MAX];           ///<TRUE:誰かが退出しようとしている
}COLOSSEUM_RECV_BUF;

///コロシアムシステムワーク
typedef struct _COLOSSEUM_SYSTEM{
  COMM_PLAYER_SYS_PTR cps;                  ///<通信プレイヤー制御システム
  COMM_PLAYER_PACKAGE send_mine_package;    ///<自分座標送信バッファ

  COLOSSEUM_BASIC_STATUS basic_status[COLOSSEUM_MEMBER_MAX];  ///<各プレイヤーの基本情報
  u8 comm_ready;                            ///<TRUE:基本情報の交換が済んだので自由に通信してOK
  u8 colosseum_leave;                       ///<TRUE:退出処理に入っている
  u8 entry_all_ready;                       ///<TRUE:全員のエントリーが完了
  u8 all_battle_ready;                      ///<TRUE:全員の戦闘準備完了
  
  COLOSSEUM_PARENT_WORK parentsys;          ///<親データ
  COLOSSEUM_MINE_WORK mine;                 ///<自分データ
  COLOSSEUM_RECV_BUF recvbuf;               ///<受信バッファ
  
  COMM_ENTRY_MENU_PTR entry_menu;           ///<参加募集メニュー管理
  
  u8 talk_obj_id;                           ///<話しかけ相手のOBJ_ID
  u8 select_pokeparty;                      ///<選択したPOKEPARTY(COLOSSEUM_SELECT_PARTY_???)
  u8 padding[2];
}COLOSSEUM_SYSTEM;


