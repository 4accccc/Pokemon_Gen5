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


//==============================================================================
//  定数定義
//==============================================================================
///コロシアムに参加出来る最大人数
#define COLOSSEUM_MEMBER_MAX    (4)

///立ち位置についていない(COLOSSEUM_PARENT_WORK.stand_positionの値)
#define COLOSSEUM_STANDING_POSITION_NULL    (0xff)


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
  u8 padding[3];
}COLOSSEUM_BASIC_STATUS;

///親だけが持つシステムデータ
typedef struct{
  u8 stand_position[COLOSSEUM_MEMBER_MAX];  ///<各メンバーがどの立ち位置にいるか
  u8 answer_stand_position[COLOSSEUM_MEMBER_MAX];   ///<立ち位置確認を送ってきたメンバーへの返事
}COLOSSEUM_PARENT_WORK;

///自分のローカルデータ
typedef struct{
  u8 stand_position;          ///<自分の立ち位置
  u8 answer_stand_position;   ///<立ち位置使用許可の返事受信バッファ
}COLOSSEUM_MINE_WORK;

///受信バッファ
typedef struct{
  COMM_PLAYER_PACKAGE comm_player_pack;     ///<通信相手の座標
  u8 comm_player_pack_update;               ///<TRUE:更新あり
}COLOSSEUM_RECV_BUF;

///コロシアムシステムワーク
typedef struct _COLOSSEUM_SYSTEM{
  COMM_PLAYER_SYS_PTR cps;                  ///<通信プレイヤー制御システム
  COMM_PLAYER_PACKAGE send_mine_package;    ///<自分座標送信バッファ

  COLOSSEUM_BASIC_STATUS basic_status[COLOSSEUM_MEMBER_MAX];  ///<各プレイヤーの基本情報
  BOOL comm_ready;                          ///<TRUE:基本情報の交換が済んだので自由に通信してOK
  
  TR_CARD_DATA *tr_card[COLOSSEUM_MEMBER_MAX];  ///<通信相手のトレーナーカード情報
  u8 tr_card_occ[COLOSSEUM_MEMBER_MAX];     ///<TRUE:トレーナーカードデータ有効
  
  COLOSSEUM_PARENT_WORK parentsys;          ///<親データ
  COLOSSEUM_MINE_WORK mine;                 ///<自分データ
  COLOSSEUM_RECV_BUF recvbuf[COLOSSEUM_MEMBER_MAX]; ///<受信バッファ
}COLOSSEUM_SYSTEM;


