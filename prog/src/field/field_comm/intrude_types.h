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
#include "field/intrude_common.h"
#include "field/field_comm/bingo_types.h"
#include "savedata/intrude_save.h"  //OCCUPY_INFO
#include "field/field_comm/mission_types.h"
#include "field/field_wfbc_data.h"


//==============================================================================
//  定数定義
//==============================================================================
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
  INTRUDE_COMM_STATUS_RESTART,      ///<通信再起動中
  INTRUDE_COMM_STATUS_EXIT_START,   ///<終了処理開始
  INTRUDE_COMM_STATUS_EXIT,         ///<終了処理完了
  INTRUDE_COMM_STATUS_ERROR,        ///<エラー
};

///アクションステータス(プレイヤーが現在何を行っているか)
typedef enum{
  INTRUDE_ACTION_FIELD,                   ///<フィールド
  INTRUDE_ACTION_TALK,                    ///<話中
  INTRUDE_ACTION_BATTLE,                  ///<戦闘中
  INTRUDE_ACTION_BINGO_BATTLE,            ///<ビンゴバトル中
  INTRUDE_ACTION_BINGO_BATTLE_INTRUSION,  ///<ビンゴバトル乱入参加
  INTRUDE_ACTION_EFFECT,                  ///<演出中
}INTRUDE_ACTION;

///会話タイプ
typedef enum{
  INTRUDE_TALK_TYPE_NORMAL,         ///<通常会話
  INTRUDE_TALK_TYPE_MISSION,        ///<ミッション用の会話
}INTRUDE_TALK_TYPE;

///会話ステータス
typedef enum{
  INTRUDE_TALK_STATUS_NULL,         ///<初期値(何もしていない状態)
  INTRUDE_TALK_STATUS_OK,           ///<会話OK
  INTRUDE_TALK_STATUS_NG,           ///<会話NG
  INTRUDE_TALK_STATUS_CANCEL,       ///<会話キャンセル
  INTRUDE_TALK_STATUS_BATTLE,       ///<対戦
  INTRUDE_TALK_STATUS_ITEM,         ///<アイテム渡す
  INTRUDE_TALK_STATUS_CARD,         ///<トレーナーカード見せて
}INTRUDE_TALK_STATUS;

///同期取り番号
enum{
    INTRUDE_TIMING_EXIT = 20,
    INTRUDE_TIMING_BATTLE_COMMAND_ADD_BEFORE,   ///<通信対戦のコマンドADD前
    INTRUDE_TIMING_BATTLE_COMMAND_ADD_AFTER,    ///<通信対戦のコマンドADD後
};

///石版タイプ
enum{
  MONOLITH_TYPE_WHITE,
  MONOLITH_TYPE_BLACK,
  
  MONOLITH_TYPE_MAX,
};

///ミッションが発動していない時のミッション番号
#define MISSION_NO_NULL     (0xff)

///巨大データ受信バッファサイズ
#define INTRUDE_HUGE_RECEIVE_BUF_SIZE   (600)


//==============================================================================
//  型定義
//==============================================================================
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
  u8 action_status; ///<実行中のアクション(INTRUDE_ACTION_???)
  u16 disguise_no;   ///<変装番号
  u8 padding[2];
}INTRUDE_STATUS;

///プロフィールデータ(送信のみに使用。受信は各々のバッファに分かれる)
typedef struct{
  MYSTATUS mystatus;
  OCCUPY_INFO occupy;
  INTRUDE_STATUS status;
}INTRUDE_PROFILE;

///話しかける最初のデータ
typedef struct{
  u8 talk_type;               ///<INTRUDE_TALK_TYPE
  u8 padding[3];
  union{  //talk_typeによって変化する内容
    MISSION_DATA mdata;
  };
}INTRUDE_TALK_FIRST_ATTACK;

///会話用ワーク
typedef struct{
  u8 talk_netid;              ///<話しかけた相手のNetID
  u8 answer_talk_netid;       ///<話しかけられている相手のNetID
  u8 talk_status;             ///<INTRUDE_TALK_STATUS_???
  u8 answer_talk_status;      ///<話しかけられている相手のINTRUDE_TALK_STATUS_???
}INTRUDE_TALK;

///侵入システムワーク
typedef struct _INTRUDE_COMM_SYS{
  GAME_COMM_SYS_PTR game_comm;
  COMM_PLAYER_SYS_PTR cps;                          ///<通信プレイヤー制御ワークへのポインタ
  
  u8 huge_receive_buf[INTRUDE_HUGE_RECEIVE_BUF_SIZE];  ///<巨大データ受信バッファ

  GBS_BEACON send_beacon;
  INTRUDE_STATUS intrude_status_mine;               ///<自分の現在情報
  INTRUDE_STATUS intrude_status[FIELD_COMM_MEMBER_MAX]; ///<全員の現在情報
  COMM_PLAYER_PACKAGE backup_player_pack[FIELD_COMM_MEMBER_MAX];  ///<Backup座標データ(座標変換をしていない)
  INTRUDE_TALK_FIRST_ATTACK recv_talk_first_attack;
  INTRUDE_TALK   talk;
  INTRUDE_PROFILE send_profile;   ///<自分プロフィール送信バッファ(受信はgamedata内なので不要)
  
  MISSION_SYSTEM mission;     ///<ミッションシステム
  BINGO_SYSTEM bingo;         ///<ビンゴシステムワーク
  
  WFBC_COMM_DATA wfbc_comm_data;  ///<WFBC通信ワーク
  
//  BOOL comm_act_vanish[FIELD_COMM_MEMBER_MAX];   ///<TRUE:非表示
  u8 invalid_netid;           ///<侵入先ROMのnet_id
  u8 exit_recv;               ///<TRUE:終了フラグ
  u8 recv_profile;            ///<プロフィール受信フラグ(bit管理)
  u8 mission_no;              ///<ミッション番号

//  FIELD_COMM_MENU *commMenu_;
  u8 comm_status;
  u8 profile_req;             ///<TRUE:プロフィール要求リクエストを受けている
  u8 profile_req_wait;        ///<プロフィール再要求するまでのウェイト
  u8 send_status;             ///<TRUE:自分の現在情報送信リクエスト
  
  u8 answer_talk_ng_bit;      ///<話しかけられたが、対応できない場合のNG返事を返す(bit管理)
  u8 connect_map_count;       ///<連結したマップの数
  u8 member_num;              ///<参加人数(親機から受信)
  u8 member_send_req;         ///<TRUE:参加人数の送信を行う
  
  u8 warp_town_tblno;         ///<ワープ先のテーブル番号
  u8 area_occupy_update;      ///<TRUE:侵入しているエリアの占拠情報を受信した(下画面やり取り)
  u8 send_occupy;             ///<TRUE:自分の占拠情報を送信リクエスト
  u8 recv_target_timing_no;   ///<相手指定タイミングコマンドの同期番号の受信バッファ
  
  u16 other_player_timeout;   ///<自分一人になった場合、通信終了へ遷移するまでのタイムアウト
  u8 recv_target_timing_netid;  ///<相手指定タイミングコマンドの送信者のNetID
  u8 wfbc_req;                ///<WFBCパラメータ要求フラグ(bit管理)
  
  u8 wfbc_recv;               ///<TRUE:WFBCパラメータを受信した
  u8 new_mission_recv;        ///<TRUE:新規にミッションを受信した
  u8 palace_in;               ///<TRUE:パレス島に訪れた
  u8 warp_player_netid;       ///<ワープ先のプレイヤーNetID
}INTRUDE_COMM_SYS;

