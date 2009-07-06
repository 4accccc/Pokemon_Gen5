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


//==============================================================================
//  定数定義
//==============================================================================
///ビーコンデータが有効なものである事を示す数値
#define UNION_BEACON_VALID        (0x7a)

///ユニオン：ステータス
enum{
  UNION_STATUS_BUSY,        ///<何らかの処理中
  UNION_STATUS_NORMAL,      ///<通常状態
  
  UNION_STATUS_CONNECT_REQ,     ///<接続リクエスト実行中
  UNION_STATUS_CONNECT_ANSWER,  ///<接続リクエストを受けた側が返信として接続しにいっている状態
  
  UNION_STATUS_BATTLE,      ///<戦闘
  UNION_STATUS_TRADE,       ///<交換
  UNION_STATUS_RECORD,      ///<レコードコーナー
  UNION_STATUS_PICTURE,     ///<お絵かき
  UNION_STATUS_GURUGURU,    ///<ぐるぐる交換
  
  UNION_STATUS_CHAT,        ///<チャット編集中
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


//==============================================================================
//  構造体定義
//==============================================================================
///UNION_SYSTEM構造体の不定形ポインタ型
typedef struct _UNION_SYSTEM * UNION_SYSTEM_PTR;


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
  u8 mac_address[6];          ///<自分のMacAddress
  u8 connect_mac_address[6];  ///<接続したい人へのMacAddress

  u8 pm_version;              ///<PM_VERSION
  u8 language;                ///<PM_LANG
  u8 union_status;            ///<プレイヤーの状況(UNION_STATUS_???)
  u8 appeal_no;               ///<アピール番号(UNION_APPEAL_???)
  
  STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];  ///<名前
  
  u8 data_valid;              ///<UNION_BEACON_VALID:このビーコンデータは有効なものである
  u8 trainer_type;            ///<トレーナータイプ
  u8 padding[2];
  
  union{
    UNION_BEACON_CHAT chat;
    UNION_BEACON_BATTLE battle;
  };

  u8 reserve[16];             ///<将来の為の予約
}UNION_BEACON;

///受信したビーコンデータ
typedef struct{
  UNION_BEACON beacon;
  u8 new_data;                ///<TRUE:新規のビーコンデータ
  u8 padding[3];
}UNION_RECEIVE_BEACON;

//--------------------------------------------------------------
//  自分データ
//--------------------------------------------------------------
///ユニオンルーム内での自分の状況
typedef struct{
  u8 connect_mac_address[6];  ///<接続したい人へのMacAddress
  u8 union_status;                  ///<プレイヤーの状況(UNION_STATUS_???)
  u8 appeal_no;               ///<アピール番号(UNION_APPEAL_???)
}UNION_MY_SITUATION;

