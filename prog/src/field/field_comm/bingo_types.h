//==============================================================================
/**
 * @file    bingo_types.h
 * @brief   ビンゴ共通ヘッダ
 * @author  matsuda
 * @date    2009.10.10(土)
 */
//==============================================================================
#pragma once

#include "field/intrude_common.h"


//==============================================================================
//  定数定義
//==============================================================================
///ビンゴバトル乱入最大人数(自分含む)
#define BINGO_BATTLE_PLAYER_MAX       (4)

///乱入リクエストの返事(intrusion_player_answerに代入される値)
typedef enum{
  BINGO_INTRUSION_ANSWER_WAIT,    ///<返事保留
  BINGO_INTRUSION_ANSWER_OK,      ///<OK
  BINGO_INTRUSION_ANSWER_NG,      ///<NG
}BINGO_INTRUSION_ANSWER;

//==============================================================================
//  構造体定義
//==============================================================================
///ビンゴバトルPROCに渡すParentWork
typedef struct{
  //--- 最初からセットされているデータ
  
  //--- ビンゴ通信システム側から随時変更されるデータ
  u8 intrusion_player[FIELD_COMM_MEMBER_MAX]; ///<ビンゴ乱入希望(希望順にnetIDが入る)
                                                ///<希望無しの場合はINTRUDE_NETID_NULL
  
  //--- ビンゴバトルPROC側から通信システム側へ伝えるデータ
  u8 intrusion_player_answer[FIELD_COMM_MEMBER_MAX]; ///<乱入希望の返事(BINGO_INTRUSION_ANSWER_???)
                                                     ///<intrusion_playerと同じく希望順の並びです
}BINGO_PARENT_WORK;

///ビンゴカード
typedef struct{
  u8 work[32];
}BINGO_CARD;

///侵入システムから代入させるデータ
typedef struct{
  BINGO_CARD  card;
  u8 intrusion_answer;      ///<乱入要求の返事(BINGO_INTRUSION_ANSWER_???)
  u8 padding[3];
}BINGO_RECV_WORK;

///侵入システムへ伝えるデータ
typedef struct{
  u8 card_req;          ///<TRUE:カードください要求
  u8 intrusion_netid;   ///<乱入希望(乱入先のプレイヤーのnetID)
  u8 padding[2];
}BINGO_SEND_WORK;

///ビンゴバトル乱入用パラメータ
typedef struct{
  u8 occ;               ///<TRUE:乱入用パラメータのデータ有効
  u8 dummy[127];
}BINGO_INTRUSION_PARAM;

///ビンゴシステム構造体
typedef struct{
  BINGO_PARENT_WORK parent;
  BINGO_RECV_WORK recv;
  BINGO_SEND_WORK send;
  BINGO_INTRUSION_PARAM intrusion_param;    ///<ビンゴバトル乱入用パラメータ
  u8 intrusion_netid_bit;               ///<乱入希望者のNetID(bit管理) ：返事が確定した時点でクリア
  u8 intrusion_answer_ok_netid_bit;     ///<乱入希望者への返事(乱入OK) ：返事を返したらクリアします
  u8 intrusion_answer_ng_netid_bit;     ///<乱入希望者への返事(乱入NG) ：返事を返したらクリアします
  u8 intrusion_recv_answer;             ///<乱入要求の返事受信バッファ
  u8 param_req_bit;                     ///<ビンゴバトル乱入パラメータ希望者のNetID(bit管理)
}BINGO_SYSTEM;

