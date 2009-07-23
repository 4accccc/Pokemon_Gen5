//==============================================================================
/**
 * @file    union_types.h
 * @brief   ユニオンルームで共通で使用する定義類
 * @author  matsuda
 * @date    2009.07.02(木)
 */
//==============================================================================
#pragma once

#include "buflen.h"
#include "app/trainer_card.h"


//==============================================================================
//  定数定義
//==============================================================================
///一度に接続できる最大人数
#define UNION_CONNECT_PLAYER_NUM      (5) //レコードコーナーが最大5人なので

///ビーコンデータが有効なものである事を示す数値
#define UNION_BEACON_VALID        (0x7a)

///実行中のゲームカテゴリー
typedef enum{ //※MenuMemberMax, UNION_STATUS_TRAINERCARD以降のテーブルと並びを同じにしておくこと！
  UNION_PLAY_CATEGORY_UNION,          ///<ユニオンルーム
  UNION_PLAY_CATEGORY_TALK,           ///<会話中
  UNION_PLAY_CATEGORY_TRAINERCARD,    ///<トレーナーカード
  UNION_PLAY_CATEGORY_PICTURE,        ///<お絵かき
  UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50,         ///<コロシアム
  UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE,       ///<コロシアム
  UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD,   ///<コロシアム
  UNION_PLAY_CATEGORY_COLOSSEUM_MULTI,      ///<コロシアム
  UNION_PLAY_CATEGORY_TRADE,          ///<ポケモン交換
  UNION_PLAY_CATEGORY_GURUGURU,       ///<ぐるぐる交換
  UNION_PLAY_CATEGORY_RECORD,         ///<レコードコーナー
  
  UNION_PLAY_CATEGORY_MAX,
}UNION_PLAY_CATEGORY;

///ユニオン：ステータス
enum{
  UNION_STATUS_NORMAL,      ///<通常状態(何もしていない)
  UNION_STATUS_ENTER,       ///<ユニオンルームへ進入
  UNION_STATUS_LEAVE,       ///<ユニオンルームから退出
  
  UNION_STATUS_CONNECT_REQ,     ///<接続リクエスト実行中
  UNION_STATUS_CONNECT_ANSWER,  ///<接続リクエストを受けた側が返信として接続しにいっている状態
  UNION_STATUS_TALK_PARENT,     ///<接続確立後の会話(親)
  UNION_STATUS_TALK_LIST_SEND_PARENT, ///<接続確立後、親の選んだ項目送信＆子の返事待ち
  UNION_STATUS_TALK_CHILD,      ///<接続確立後の会話(子)
  UNION_STATUS_TALK_BATTLE_PARENT,  ///<会話：対戦メニュー(親)
  UNION_STATUS_TALK_PLAYGAME_PARENT,  ///<既に遊んでいる相手(親)に話しかけた
  UNION_STATUS_TALK_PLAYGAME_CHILD,   ///<既に遊んでいる相手(子)に話しかけた
  
  //PLAY_CATEGORYと並びを同じにする必要がある　ここから============
  UNION_STATUS_TRAINERCARD, ///<トレーナーカード
  UNION_STATUS_PICTURE,     ///<お絵かき
  UNION_STATUS_BATTLE_1VS1_SINGLE_50,        ///<戦闘
  UNION_STATUS_BATTLE_1VS1_SINGLE_FREE,      ///<戦闘
  UNION_STATUS_BATTLE_1VS1_SINGLE_STANDARD,  ///<戦闘
  UNION_STATUS_BATTLE_MULTI,      ///<戦闘
  UNION_STATUS_TRADE,       ///<交換
  UNION_STATUS_GURUGURU,    ///<ぐるぐる交換
  UNION_STATUS_RECORD,      ///<レコードコーナー
  // ここまで======================
  
  UNION_STATUS_INTRUDE,     ///<乱入
  UNION_STATUS_SHUTDOWN,    ///<切断
  
  UNION_STATUS_COLOSSEUM_MEMBER_WAIT,   ///<コロシアム：メンバー集合待ち
  UNION_STATUS_COLOSSEUM_NORMAL,        ///<コロシアム：フリー移動
  UNION_STATUS_COLOSSEUM_STANDPOSITION, ///<コロシアム：立ち位置にたった
  UNION_STATUS_COLOSSEUM_STANDING_BACK, ///<コロシアム：立ち位置から後退
  UNION_STATUS_COLOSSEUM_POKELIST,      ///<コロシアム：ポケモンリスト呼び出し
  
  UNION_STATUS_CHAT,        ///<チャット編集中
  
  UNION_STATUS_MAX,
};

///ユニオン：アピール番号
enum{
  UNION_APPEAL_NULL,        ///<アピール無し
  
  UNION_APPEAL_BATTLE,      ///<戦闘
  UNION_APPEAL_TRADE,       ///<交換
  UNION_APPEAL_RECORD,      ///<レコードコーナー
  UNION_APPEAL_PICTURE,     ///<お絵かき
  UNION_APPEAL_GURUGURU,    ///<ぐるぐる交換
};

///ビーコン：バトル：戦闘モード
enum{
  UNION_BATTLE_MODE_SINGLE,     ///<1vs1 シングルバトル
  UNION_BATTLE_MODE_DOUBLE,     ///<1vs1 ダブルバトル
  UNION_BATTLE_MODE_TRIPLE,     ///<1vs1 トリプルバトル
  UNION_BATTLE_MODE_ROTATION,   ///<1vs1 ローテーションバトル
  
  UNION_BATTLE_MODE_MULTI,      ///<2vs2 マルチバトル
};

///ユニオンビーコン：受信状況
enum{
  UNION_BEACON_RECEIVE_NULL,    ///<ビーコン変化なし
  UNION_BEACON_RECEIVE_NEW,     ///<新規ビーコン
  UNION_BEACON_RECEIVE_UPDATE,  ///<ビーコン更新
};

///ユニオンルームのキャラクタ寿命(フレーム単位)
#define UNION_CHAR_LIFE         (30 * 15)  //フィールドの為、1/30

///同期用のタイミングコマンド番号
enum{
  UNION_TIMING_SHUTDOWN = 1,            ///<切断前の同期取り
  
  UNION_TIMING_TRAINERCARD_PARAM,       ///<トレーナーカードの情報交換前
  UNION_TIMING_TRAINERCARD_PROC_BEFORE, ///<トレーナーカード画面呼び出し前
  UNION_TIMING_TRAINERCARD_PROC_AFTER,  ///<トレーナーカード画面終了後
  
  UNION_TIMING_COLOSSEUM_PROC_BEFORE,   ///<コロシアム遷移前の同期取り
  UNION_TIMING_COLOSSEUM_MEMBER_ENTRY_AFTER,  ///<コロシアム：メンバー集まった後の同期取り
  UNION_TIMING_COLOSSEUM_ADD_CMD_TBL_AFTER,   ///<コロシアム：通信テーブルを追加後
  UNION_TIMING_COLOSSEUM_PROC_AFTER,    ///<コロシアム終了後の同期取り
  
  UNION_TIMING_BATTLE_ADD_CMD_TBL_AFTER,  ///<戦闘：通信テーブルを追加後
};

///サブPROC呼び出しID     ※SubProc_PlayCategoryTblと並びを同じにしておくこと！
typedef enum{
  UNION_SUBPROC_ID_NULL,              ///<サブPROC無し
  UNION_SUBPROC_ID_TRAINERCARD,       ///<トレーナーカード
  UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_50,    ///<コロシアム遷移
  UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_FREE,    ///<コロシアム遷移
  UNION_SUBPROC_ID_COLOSSEUM_WARP_1VS1_SINGLE_STANDARD,    ///<コロシアム遷移
  UNION_SUBPROC_ID_COLOSSEUM_WARP_MULTI,    ///<コロシアム遷移(マルチ部屋)
  UNION_SUBPROC_ID_POKELIST,                ///<ポケモンリスト呼び出し
  UNION_SUBPROC_ID_BATTLE,                  ///<戦闘遷移
  
  UNION_SUBPROC_ID_MAX,
}UNION_SUBPROC_ID;


//==============================================================================
//  構造体定義
//==============================================================================
///UNION_SYSTEM構造体の不定形ポインタ型
typedef struct _UNION_SYSTEM * UNION_SYSTEM_PTR;


///接続メンバーの情報
typedef struct{
  u8 mac_address[6];    ///<一緒に遊んでいる相手のMacAddress
  u8 trainer_view;      ///<一緒に遊んでいる相手の見た目
  u8 sex:1;             ///<一緒に遊んでいる相手の性別
  u8 occ:1;             ///<TRUE：データ有効
  u8    :6;
}UNION_MEMBER;

///接続メンバーの集合体
typedef struct{
  UNION_MEMBER member[UNION_CONNECT_PLAYER_NUM];
}UNION_PARTY;

//--------------------------------------------------------------
//  キャラクタ
//--------------------------------------------------------------
///キャラクタ制御ワーク
typedef struct{
  struct _UNION_BEACON_PC *parent_pc; ///<親PCへのポインタ
  u8 trainer_view;            ///<トレーナータイプ(ユニオンルーム内での見た目)
  u8 sex;                     ///<性別
  u8 child_no;                ///<子番号(親:0　子:1～)
  u8 occ;                     ///<TRUE:データ有効　FALSE:無効

  u8 event_status;            ///<イベントステータス(BPC_EVENT_STATUS_???)
  u8 next_event_status;       ///<次に実行するイベントステータス(BPC_EVENT_STATUS_???)
  u8 func_proc;               ///<動作プロセスNo(BPC_SUBPROC_???)
  u8 func_seq;                ///<動作プロセスシーケンス
}UNION_CHARACTER;

///一人の親が管理するキャラクタ制御ワークのグループ
typedef struct{
  UNION_CHARACTER *character[UNION_CONNECT_PLAYER_NUM];
}UNION_CHARA_GROUP;

//--------------------------------------------------------------
//  ビーコン
//--------------------------------------------------------------
///ビーコン：バトル
typedef struct{
  u8 battle_mode;           ///<戦闘モード(UNION_BATTLE_MODE_???)
  u8 regulation;            ///<レギュレーション番号　※check 構造体丸ごとの方がいいかも
  u8 padding[2];
}UNION_BEACON_BATTLE;

///ビーコン：チャット
typedef struct{
  u8 chat_no;
  u8 padding[3];
  u16 word[2];
}UNION_BEACON_CHAT;

///ユニオンで送受信するビーコンデータ
typedef struct{
  u8 connect_mac_address[6];  ///<接続したい人へのMacAddress
  u8 connect_num;             ///<現在の接続人数
  u8 padding;
  
  u8 pm_version;              ///<PM_VERSION
  u8 language;                ///<PM_LANG
  u8 union_status;            ///<プレイヤーの状況(UNION_STATUS_???)
  u8 appeal_no;               ///<アピール番号(UNION_APPEAL_???)
  
  STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];  ///<名前
  
  u8 data_valid;              ///<UNION_BEACON_VALID:このビーコンデータは有効なものである
  u8 play_category;           ///<実行中のゲームカテゴリー
  u8 trainer_view;            ///<トレーナータイプ(ユニオンルーム内での見た目)
  u8 sex;                     ///<性別
  
  UNION_PARTY party;          ///<接続相手の情報
  
  //play_categoryの内容によってワークの中身が変化
  union{
    UNION_BEACON_CHAT chat;
    UNION_BEACON_BATTLE battle;
  };

  u8 reserve[16];             ///<将来の為の予約
}UNION_BEACON;

///受信したビーコンデータから作成されたPCパラメータ
typedef struct _UNION_BEACON_PC{
  UNION_BEACON beacon;
  u8 mac_address[6];          ///<送信相手のMacAddress
  u8 update_flag;             ///<ビーコンデータ受信状況(UNION_BEACON_???)
  u8 buffer_no;               ///<バッファ番号
  
  u16 life;                   ///<寿命(フレーム単位)　新しくビーコンを受信しなければ消える
  u8 seat;                    ///<接続相手の配置場所(bit管理 0bit目=一人目の席、1bit目=二人目の席)
  u8 my_seat_no;              ///<自分自身の席
  
  UNION_CHARA_GROUP chara_group;   ///<キャラクタグループ
  
  struct _UNION_BEACON_PC *link_parent_pc;   ///<リンクしているPC(親)へのポインタ
  u8 link_parent_index;       ///<リンクしているPCのindex
}UNION_BEACON_PC;

//--------------------------------------------------------------
//  自分データ
//--------------------------------------------------------------
///トレーナーカード情報
typedef struct{
  TRCARD_CALL_PARAM *card_param;  ///<カード画面呼び出しようのParentWork
  TR_CARD_DATA *my_card;      ///<自分のカード情報(送信バッファ)
  TR_CARD_DATA *target_card;  ///<相手のカード情報(受信バッファ)
  u8 target_card_receive;     ///<TRUE:相手のカードを受信した
  u8 padding[3];
}UNION_TRCARD;

///送受信で変更するパラメータ類(自機がフリー動作の状態になるたびに初期化される)
typedef struct{
  u8 force_exit;              ///<TRUE:通信相手から強制切断を受信
  u8 mainmenu_select;         ///<メインメニューでの選択結果
  u8 mainmenu_yesno_result;   ///<「はい(TRUE)」「いいえ(FALSE)」選択結果
  u8 submenu_select;          ///<メインメニュー後のサブメニューの選択結果
  UNION_TRCARD trcard;        ///<トレーナーカード情報
  UNION_PARTY party;          ///<一緒に遊んでいる相手のパラメータ
  u16 talk_obj_id;             ///<話しかけた相手のCharacterIndex
  u8 padding[2];
}UNION_MY_COMM;

///ユニオンルーム内での自分の状況
typedef struct{
  //↓送信ビーコンに含めるデータ
  u8 play_category;           ///<実行中のゲームカテゴリー(UNION_PLAY_CATEGORY_???)
  u8 union_status;            ///<プレイヤーの状況(UNION_STATUS_???)
  u8 appeal_no;               ///<アピール番号(UNION_APPEAL_???)
  u8 padding2;
  
  //↓構造体内の一部のデータのみを送信データに含める
  UNION_BEACON_PC *calling_pc; ///<接続して欲しい人のreceive_beaconへのポインタ
  UNION_MY_COMM mycomm;         ///<送受信で変更するパラメータ類
  
  //↓ここから下は通信では送らないデータ
  UNION_BEACON_PC *answer_pc;  ///<接続したい人のreceive_beaconへのポインタ
  UNION_BEACON_PC *connect_pc; ///<接続中の人のreceive_beaconへのポインタ
  s16 wait;
  s16 work;
  u8 before_union_status;     ///<前に実行していたプレイヤーの状況(UNION_STATUS_???)
  u8 next_union_status;       ///<次に実行するプレイヤーの状況(UNION_STATUS_???)
  u8 func_proc;
  u8 func_seq;
}UNION_MY_SITUATION;

//--------------------------------------------------------------
//  システム
//--------------------------------------------------------------
typedef struct{
  BOOL active;               ///<TRUE:サブPROC実行中
  UNION_SUBPROC_ID id;       ///<サブPROC呼び出しID
  void *parent_work;         ///<サブPROCに渡すParentWorkへのポインタ
  u8 seq;
  u8 padding[3];
}UNION_SUB_PROC;

