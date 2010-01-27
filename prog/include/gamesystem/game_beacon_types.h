//==============================================================================
/**
 * @file    game_beacon_types.h
 * @brief   すれ違いのビーコン情報
 * @author  matsuda
 * @date    2010.01.19(火)
 */
//==============================================================================
#pragma once

#include "gamesystem/game_beacon.h"


//==============================================================================
//  定数定義
//==============================================================================
///システムログ件数
#define GAMEBEACON_SYSTEM_LOG_MAX       (30)

///お礼メッセージの文字数(EOM込み)
#define GAMEBEACON_THANKYOU_MESSAGE_LEN   (11)
///自己紹介メッセージの文字数(!! EOM除く !!)
#define GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN   (10)


//==============================================================================
//  構造体定義
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   詳細パラメータ
 *
 * フィールドビーコンに混ぜる為、外部公開しています。
 * 上記以外での直接参照は控えてください
 */
//--------------------------------------------------------------
typedef struct{
  u8 details_no;                 ///<詳細No(GAMEBEACON_DETAILS_NO_xxx)
  u8 padding;
  union{
    u16 battle_trainer;          ///<戦闘相手のトレーナーコード
    u16 battle_monsno;           ///<戦闘相手のポケモンNo
  };
}GAMEBEACON_INFO_DETAILS;

//--------------------------------------------------------------
/**
 * @brief   行動パラメータ
 *
 * フィールドビーコンに混ぜる為、外部公開しています。
 * 上記以外での直接参照は控えてください
 */
//--------------------------------------------------------------
typedef struct{
  u16 action_no;                            ///<行動No
  u16 padding;
  
  //action_noによってセットされるものが変わる
  union{
    u8 work[24];
    u16 tr_no;                              ///<トレーナー番号
    u16 itemno;                             ///<アイテム番号
    u32 target_trainer_id;                  ///<対象相手のトレーナーID
    STRCODE nickname[BUFLEN_POKEMON_NAME];  ///<ポケモン名(ニックネーム)
    STRCODE thankyou_message[GAMEBEACON_THANKYOU_MESSAGE_LEN]; ///お礼メッセージ
  };
}GAMEBEACON_INFO_ACTION;

//==================================================================
/**
 * @brief   送受信されるビーコンパラメータ
 *
 * フィールドビーコンに混ぜる為、外部公開しています。
 * 上記以外での直接参照は控えてください
 */
//==================================================================
struct _GAMEBEACON_INFO{
  u8 version_bit;                          ///<受信可能なPM_VERSIONをbit指定
  u8 send_counter;                         ///<送信No(同じデータの無視判定に使用)
  u8 nation;                               ///<国コード
  u8 area;                                 ///<地域コード
  
  u32 zone_id:16;                          ///<現在地
  u32 g_power_id:8;                        ///<発動しているGパワーID(GPOWER_ID_xxx)
  u32 pm_version:7;
  u32 sex:1;                               ///<性別

  u32 suretigai_count:17;                  ///<すれ違い人数
  u32 language:5;                          ///<PM_LANG
  u32 trainer_view:3;                      ///<ユニオンルームでの見た目
  u32           :7;

  u32 favorite_color:15;                   ///<本体情報の色(GXRgb)
  u32 thanks_recv_count:17;                ///<お礼を受けた回数
  
  STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];         ///<トレーナー名
  u32 trainer_id;                          ///<トレーナーID
  
  STRCODE self_introduction[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN]; ///<自己紹介メッセージ
  
  GAMEBEACON_INFO_DETAILS details;         ///<詳細情報
  GAMEBEACON_INFO_ACTION action;           ///<行動パラメータ
};


//==================================================================
/**
 * @brief   送受信されるビーコンパラメータのログテーブル
 *
 * フィールドビーコンに混ぜる為、外部公開しています。
 * 上記以外での直接参照は控えてください
 */
//==================================================================
struct _GAMEBEACON_INFO_TBL{
  struct _GAMEBEACON_INFO info[GAMEBEACON_INFO_TBL_MAX];
  u16 time[GAMEBEACON_INFO_TBL_MAX];    ///<受信日時(上位8bit：時(0～23)、下位8bit：分(0～59))
  
  u8  ring_top;   //リングバッファ制御用 リストのトップindexを保持
  u8  entry_num;  //リング登録数
};
