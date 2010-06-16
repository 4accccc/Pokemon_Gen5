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
#include "savedata/intrude_save.h"  //OCCUPY_INFO
#include "field/field_comm/mission_types.h"
#include "field/field_wfbc_data.h"
#include "field/intrude_symbol.h"


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
  INTRUDE_TALK_TYPE_NORMAL,               ///<通常会話
  
  INTRUDE_TALK_TYPE_MISSION_N_to_M,       ///<共通：傍観者＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_N_to_T,       ///<共通：傍観者＞ターゲット
  
  //ミッション会話のオフセット (※ミッション以外の会話はこれより前に配置する事!!)
  // ※ミッション会話以降はEventCommFuncTalkTbl, EventCommFuncTalkedTblと並びを一致させること！！
  INTRUDE_TALK_TYPE_MISSION_OFFSET_START,
  INTRUDE_TALK_TYPE_MISSION_OFFSET_M_to_T = INTRUDE_TALK_TYPE_MISSION_OFFSET_START,    ///<ミッション実施者＞ターゲット
  INTRUDE_TALK_TYPE_MISSION_OFFSET_M_to_M,    ///<ミッション実施者＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_OFFSET_M_to_N,    ///<ミッション実施者＞傍観者
  INTRUDE_TALK_TYPE_MISSION_OFFSET_T_to_M,    ///<ターゲット＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_OFFSET_T_to_N,    ///<ターゲット＞傍観者

  INTRUDE_TALK_TYPE_MISSION_VICTORY_START,
  INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_T = INTRUDE_TALK_TYPE_MISSION_VICTORY_START, ///<ミッション実施者＞ターゲット
  INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_M, ///<ミッション実施者＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_VICTORY_M_to_N, ///<ミッション実施者＞傍観者
  INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_M, ///<ターゲット＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_VICTORY_T_to_N, ///<ターゲット＞傍観者

  INTRUDE_TALK_TYPE_MISSION_SKILL_START,
  INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_T = INTRUDE_TALK_TYPE_MISSION_SKILL_START, ///<ミッション実施者＞ターゲット
  INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_M, ///<ミッション実施者＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_SKILL_M_to_N, ///<ミッション実施者＞傍観者
  INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_M, ///<ターゲット＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_SKILL_T_to_N, ///<ターゲット＞傍観者

  INTRUDE_TALK_TYPE_MISSION_BASIC_START,
  INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_T = INTRUDE_TALK_TYPE_MISSION_BASIC_START, ///<ミッション実施者＞ターゲット
  INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_M, ///<ミッション実施者＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_BASIC_M_to_N, ///<ミッション実施者＞傍観者
  INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_M, ///<ターゲット＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_BASIC_T_to_N, ///<ターゲット＞傍観者

  INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_START,
  INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_T = INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_START, ///<ミッション実施者＞ターゲット
  INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_M, ///<ミッション実施者＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_M_to_N, ///<ミッション実施者＞傍観者
  INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_M, ///<ターゲット＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_ATTRIBUTE_T_to_N, ///<ターゲット＞傍観者

  INTRUDE_TALK_TYPE_MISSION_ITEM_START,
  INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_T = INTRUDE_TALK_TYPE_MISSION_ITEM_START, ///<ミッション実施者＞ターゲット
  INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_M, ///<ミッション実施者＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_ITEM_M_to_N, ///<ミッション実施者＞傍観者
  INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_M, ///<ターゲット＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_ITEM_T_to_N, ///<ターゲット＞傍観者

  INTRUDE_TALK_TYPE_MISSION_PERSONALITY_START,
  INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_T = INTRUDE_TALK_TYPE_MISSION_PERSONALITY_START, ///<ミッション実施者＞ターゲット
  INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_M, ///<ミッション実施者＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_PERSONALITY_M_to_N, ///<ミッション実施者＞傍観者
  INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_M, ///<ターゲット＞ミッション実施者
  INTRUDE_TALK_TYPE_MISSION_PERSONALITY_T_to_N, ///<ターゲット＞傍観者
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
  INTRUDE_TALK_STATUS_SHOP_OK,      ///<売買成立
  INTRUDE_TALK_STATUS_SHOP_NG,      ///<売買不成立
  INTRUDE_TALK_STATUS_SHOP_SHOT_OF_MONEY,      ///<お金が足りない
  INTRUDE_TALK_STATUS_SHOP_ITEM_FULL,          ///<アイテムがいっぱい
}INTRUDE_TALK_STATUS;

///強制切断後のメッセージID
typedef enum{
  MISSION_FORCEWARP_MSGID_NULL,       ///<指定なし
  MISSION_FORCEWARP_MSGID_BATTLE_NG,  ///<対戦を断られた
  MISSION_FORCEWARP_MSGID_SHOP_NG,    ///<買い物を断られた
}MISSION_FORCEWARP_MSGID;

///同期取り番号
enum{
    INTRUDE_TIMING_EXIT = 20,
    INTRUDE_TIMING_BATTLE_COMMAND_ADD_BEFORE,   ///<通信対戦のコマンドADD前
    INTRUDE_TIMING_BATTLE_COMMAND_ADD_AFTER,    ///<通信対戦のコマンドADD後
};

///バトルミッション同期タイミングbit
enum{
  MISSION_BATTLE_TIMING_BIT_FIRST = 1 << 0,
  MISSION_BATTLE_TIMING_BIT_SECOND = 1 << 1,
  
  MISSION_BATTLE_TIMING_BIT_ALL = MISSION_BATTLE_TIMING_BIT_FIRST | MISSION_BATTLE_TIMING_BIT_SECOND,
};

///石版タイプ
enum{
  MONOLITH_TYPE_WHITE,
  MONOLITH_TYPE_BLACK,
  
  MONOLITH_TYPE_MAX,
};

///ミッションが発動していない時のミッション番号
#define MISSION_NO_NULL     (0xff)

///近くにいるビーコンの人物を表示する最大数
#define INTRUDE_BCON_PLAYER_PRINT_SEARCH_MAX   (2)
///近くにいるビーコンの人物情報をリセットする時間
#define INTRUDE_BCON_PLAYER_PRINT_LIFE        (30 * 5)

///ダブルバッファ用のバッファNo定義
typedef enum{
  INTSEND_BUFF_SIDE_A,
  INTSEND_BUFF_SIDE_B,
  
  INTSEND_BUFF_SIDE_MAX,
}INTSEND_BUFF_SIDE;

///話しかけが発生していない時のtalk_randコード
#define TALK_RAND_NULL      (0)


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
  
  u16 zone_id;       ///<ゾーンID
  u16 palace_area:3; ///<パレスエリア
  u16 symbol_mapid:6;
  u16 action_status:6; ///<実行中のアクション(INTRUDE_ACTION_???)
  u16 detect_fold:1;  ///<TRUE:蓋を閉じている
  
  u16 disguise_no;    ///<変装番号
  u8 disguise_type:4; ///<変装タイプ(TALK_TYPE_xxx)
  u8 disguise_sex:1;  ///<変装中の性別
  u8 mission_entry:1; ///<TRUE:ミッションに参加している
  u8 season:2;        ///<四季(PMSEASON)
  u8 pm_version:6;    ///<PM_VERSION
  u8 tutorial:1;      ///<TRUE:チュートリアル中
  u8 force_vanish:1;  ///<TRUE:強制的に非表示
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
  u8 talk_rand;               ///<会話ランダム値
  u8 padding[2];
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
  u8 talk_rand;
  u8 now_talk_rand;
  u8 padding[2];
}INTRUDE_TALK;

///会話の返事データ
typedef struct{
  u8 talk_status;             ///<INTRUDE_TALK_STATUS_???
  u8 talk_rand;
  u8 padding[2];
}INTRUDE_TALK_ANSWER;

///モノリスステータス(ミッションリスト以外でモノリス画面構築に必要なデータ)
typedef struct{
  u32 clear_mission_count;       ///<ミッションクリア数
  s64 palace_sojourn_time;       ///<パレス滞在時間
  u8 gpower_distribution_bit[INTRUDE_SAVE_DISTRIBUTION_BIT_WORK_MAX]; ///<Gパワー配布受信bit
  u8 occ;                        ///<TRUE:データ有効
  u8 padding;
}MONOLITH_STATUS;

///ミッション達成後の占拠結果
typedef struct{
  u8 add_white;               ///<白得点
  u8 add_black;               ///<黒得点
  u8 occ;                     ///<TRUE:データ有効
  u8 padding;
}INTRUDE_OCCUPY_RESULT;

///侵入システムワーク
typedef struct _INTRUDE_COMM_SYS{
  GAMESYS_WORK *gsys;
  GAME_COMM_SYS_PTR game_comm;
  COMM_PLAYER_SYS_PTR cps;                          ///<通信プレイヤー制御ワークへのポインタ
  
  GBS_BEACON send_beacon;
  INTRUDE_STATUS intrude_status_mine;               ///<自分の現在情報
  INTRUDE_STATUS intrude_status[FIELD_COMM_MEMBER_MAX]; ///<全員の現在情報
  COMM_PLAYER_PACKAGE backup_player_pack[FIELD_COMM_MEMBER_MAX];  ///<Backup座標データ(座標変換をしていない)
  INTRUDE_TALK_FIRST_ATTACK recv_talk_first_attack;
  INTRUDE_TALK   talk;
  INTRUDE_PROFILE my_profile;   ///<自分プロフィール(受信はgamedata内なので不要)
  
  MISSION_SYSTEM mission;     ///<ミッションシステム
  INTRUDE_OCCUPY_RESULT send_occupy_result;     ///<ミッション後の占拠結果
  INTRUDE_OCCUPY_RESULT recv_occupy_result;     ///<ミッション後の占拠結果受信バッファ
  
  WFBC_COMM_DATA wfbc_comm_data;  ///<WFBC通信ワーク
  
  //大き目のデータの送信バッファ
  INTRUDE_PROFILE huge_send_profile;       ///<送信バッファ：プロフィール
  MISSION_DATA huge_send_mission_data;     ///<送信バッファ：ミッションデータ
  MISSION_RESULT huge_send_mission_result; ///<送信バッファ：ミッション結果
  FIELD_WFBC_CORE huge_send_wfbc_core;     ///<送信バッファ：WFBC

  //巨大データ受信バッファ(一つ一つはそれほど大きくはないが、これらが同時に受信されると
  //GFLIBの受信バッファがオーバーしてしまうので)
  MISSION_CHOICE_LIST huge_recv_choicelist;  ///<巨大データ受信バッファ：ミッション選択リスト
  INTRUDE_PROFILE huge_recv_profile[FIELD_COMM_MEMBER_MAX];   ///<巨大データ受信バッファ：プロフィール
  MISSION_DATA huge_recv_mission_data[FIELD_COMM_MEMBER_MAX]; ///<巨大データ受信バッファ：ミッションデータ
  MISSION_RESULT huge_recv_mission_result; ///<巨大データ受信バッファ：ミッション結果
  
  INTRUDE_SYMBOL_WORK intrude_symbol;        ///<侵入先のシンボルワーク
  INTRUDE_SYMBOL_WORK intrude_send_symbol;   ///<送信バッファ：シンボルワーク
  SYMBOL_DATA_REQ req_symbol_data[FIELD_COMM_MEMBER_MAX]; ///<シンボルデータリクエスト
  SYMBOL_DATA_CHANGE send_symbol_change;     ///<送信バッファ：シンボルデータ変更
  INTRUDE_SECRET_ITEM_SAVE recv_secret_item;  ///<受信バッファ隠しアイテム
  u8 recv_secret_item_flag:1;                 ///<TRUE:隠しアイテムを受信した
  u8 recv_symbol_flag:1;            ///<TRUE:シンボルデータを受信した
  u8 recv_symbol_change_flag:1;     ///<TRUE:シンボルデータの変更通知を受け取った
  u8 huge_send_bufside_profile:1;   ///<INTSEND_BUFF_SIDE
  u8 talked_event_reserve:1;        ///<TRUE:話しかけられたイベント予約
  u8 palace_map_not_connect:1;      ///<TRUE:パレスマップ連結をしない
  u8        :2;
  
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
  
  u8 connect_map_count;       ///<連結したマップの数
  u8 member_num;              ///<参加人数(親機から受信)
  u8 member_send_req;         ///<TRUE:参加人数の送信を行う
  u8 padding;
  
  u8 warp_town_tblno;         ///<ワープ先のテーブル番号
  u8 area_occupy_update;      ///<TRUE:侵入しているエリアの占拠情報を受信した(下画面やり取り)
  u8 send_occupy;             ///<TRUE:自分の占拠情報を送信リクエスト
  u8 wfbc_req;                ///<WFBCパラメータ要求フラグ(bit管理)
  
  u16 other_player_timeout;   ///<自分一人になった場合、通信終了へ遷移するまでのタイムアウト
  u8 wfbc_recv;               ///<TRUE:WFBCパラメータを受信した
  u8 new_mission_recv;        ///<TRUE:新規にミッションを受信した
  
  s16 exit_wait;              ///<通信切断時のタイムアウトカウンタ
  u8 palace_in;               ///<TRUE:パレス島に訪れた
  u8 warp_player_netid;       ///<ワープ先のプレイヤーNetID
  
  MONOLITH_STATUS monolith_status;  ///<モノリスステータス

  u8 monost_req;              ///<モノリスステータス要求リクエスト(bit管理)
  u8 search_count;
  u8 search_print_life;       ///<
  u8 player_status_update;    ///<プレイヤーステータス更新情報(bit管理)
  
  STRBUF *search_child[INTRUDE_BCON_PLAYER_PRINT_SEARCH_MAX];
  u32 search_child_trainer_id[INTRUDE_BCON_PLAYER_PRINT_SEARCH_MAX];
  u8 search_child_sex[INTRUDE_BCON_PLAYER_PRINT_SEARCH_MAX];
  u8 search_child_lang[INTRUDE_BCON_PLAYER_PRINT_SEARCH_MAX];
  
  s8 member_fix;              ///<TRUE:乱入禁止
  s8 other_monolith_count;    ///<他人のモノリス画面を見ているプレイヤー人数
  u8 send_occupy_result_send_req; ///<占拠結果送信リクエスト
  u8 member_is_tutorial:1;    //自分以外の通信相手がチュートリアル中
  u8 error:1;                 ///<TRUE:エラー発生
  u8 live_comm_status:1;      ///<ライブ通信画面用フラグ
  u8 mission_start_event_ended:1; ///<TRUE:ミッション開始画面を表示した
  u8 mission_battle_timing_bit:2;     ///<バトルミッションタイミング受信bit(MISSION_BATTLE_TIMING_BIT)
  u8 timeout_stop:1;          ///<TRUE:タイムアウトによる切断を行わない
  u8 debug_time_unlimited:1;  ///<デバッグ用　TRUE:タイムアウトで切断をしない
  
  u32 mission_start_timeout:31;  ///<ミッション開始前までのタイムアウト時刻をカウント
  u32 mission_start_timeout_warning:1;  ///<「ミッション開始前のタイムアウトしそう」警告発生中
  
  u8 now_talk_rand;           ///<会話する時に投げ込むランダム値(既に終了している会話の判断で使用)
  u8 answer_talk_ng[FIELD_COMM_MEMBER_MAX];///<話しかけられたが、対応できない場合のNG返事を返す
  u8 send_talk_rand_disagreement[FIELD_COMM_MEMBER_MAX]; ///<会話返事NG(randが一致しなかった)
  u8 padding2;
}INTRUDE_COMM_SYS;


