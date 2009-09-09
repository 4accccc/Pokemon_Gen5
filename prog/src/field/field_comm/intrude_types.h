//==============================================================================
/**
 * @file    intrude_types.h
 * @brief   侵入：共通ヘッダ
 * @author  matsuda
 * @date    2009.09.03(木)
 */
//==============================================================================
#pragma once

#include "field\game_beacon_search.h"
#include "net_app/union/comm_player.h"

//==============================================================================
//  定数定義
//==============================================================================
///侵入：通信最大接続人数
#define FIELD_COMM_MEMBER_MAX (4)

///通信ステータス
enum{
  INTRUDE_COMM_STATUS_NULL,         ///<何も動作していない
  INTRUDE_COMM_STATUS_INIT_START,   ///<初期化開始
  INTRUDE_COMM_STATUS_INIT,         ///<初期化完了
  INTRUDE_COMM_STATUS_BOOT_PARENT,  ///<親として起動
  INTRUDE_COMM_STATUS_BOOT_CHILD,   ///<子として起動
  INTRUDE_COMM_STATUS_HARD_CONNECT, ///<ハードは接続した(ネゴシエーションはまだ)
  INTRUDE_COMM_STATUS_BASIC_SEND,   ///<基本情報送信中
  INTRUDE_COMM_STATUS_UPDATE,       ///<メイン更新実行中
  INTRUDE_COMM_STATUS_EXIT_START,   ///<終了処理開始
  INTRUDE_COMM_STATUS_EXIT,         ///<終了処理完了
  INTRUDE_COMM_STATUS_ERROR,        ///<エラー
};

///同期取り番号
enum{
    INTRUDE_TIMING_EXIT = 20,
};

//==============================================================================
//  型定義
//==============================================================================
///_INTRUDE_COMM_SYS構造体の不定形ポインタ型
typedef struct _INTRUDE_COMM_SYS * INTRUDE_COMM_SYS_PTR;

///パレスシステムワーク構造体への不定形ポインタ
typedef struct _PALACE_SYS_WORK * PALACE_SYS_PTR;

//==============================================================================
//  構造体定義
//==============================================================================
///侵入ステータス(自分の現在情報)
typedef struct{
  COMM_PLAYER_PACKAGE player_pack;    ///<プレイヤー座標データパッケージ
  u16 zone_id;      ///<ゾーンID
  u8 palace_area;   ///<パレスエリア
  u8 mission_no;    ///<ミッション番号
}INTRUDE_STATUS;

///侵入システムワーク
typedef struct _INTRUDE_COMM_SYS{
  GAME_COMM_SYS_PTR game_comm;
  COMM_PLAYER_SYS_PTR cps;                          ///<通信プレイヤー制御ワークへのポインタ
  
  GBS_BEACON send_beacon;
  INTRUDE_STATUS intrude_status_mine;               ///<自分の現在情報
  INTRUDE_STATUS intrude_status[FIELD_COMM_MEMBER_MAX]; ///<全員の現在情報
  
//  BOOL comm_act_vanish[FIELD_COMM_MEMBER_MAX];   ///<TRUE:非表示
  u8 invalid_netid;           ///<侵入先ROMのnet_id
  u8 exit_recv;               ///<TRUE:終了フラグ
  u8 recv_profile;            ///<プロフィール受信フラグ(bit管理)
  u8 mission_no;              ///<ミッション番号

  PALACE_SYS_PTR palace;    ///<パレスシステムワークへのポインタ
//  FIELD_COMM_MENU *commMenu_;
  u8 comm_status;
  u8 profile_req;             ///<TRUE:プロフィール要求リクエストを受けている
  u8 profile_req_wait;        ///<プロフィール再要求するまでのウェイト
  u8 send_status;             ///<TRUE:自分の現在情報送信リクエスト
  
  u8 recv_status;             ///<一度でも受信した事がある侵入ステータスを把握(bit管理)
  u8 padding[3];
}INTRUDE_COMM_SYS;

