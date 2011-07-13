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
#include "savedata/questionnaire_save.h"
#include "gamesystem/fest_mission.h"
#include "gamesystem/fest_mission_local.h"

//==============================================================================
//  定数定義
//==============================================================================
///システムログ件数
#define GAMEBEACON_SYSTEM_LOG_MAX       (30)

///お礼メッセージの文字数(EOM込み)
#define GAMEBEACON_THANKYOU_MESSAGE_LEN   (9)
///一言メッセージの文字数(EOM込み)
#define GAMEBEACON_FREEWORD_MESSAGE_LEN   (GAMEBEACON_THANKYOU_MESSAGE_LEN)
///自己紹介メッセージの文字数(!! EOM除く !!)
#define GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN   (8)


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
  u16 details_no:8;                 ///<詳細No(GAMEBEACON_DETAILS_NO_xxx)
  
  //簡易会話：GAMEBEACON_INFOに直接置きたいが、パディングの関係でここに配置
  //メンバ全てをu16にする事で2byteアライメントの座標に配置できるようにしている
  u16 sentence_type:3;
  u16 sentence_id:5;
  PMS_WORD word[PMS_WORD_MAX];
  
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
  union
  {
    u16 monsno;   ///<ポケモン番号 ※本来ならnormalの中に入っているものだが、action_noとunionの間に
                                 //アライメントが発生しているので、monsnoをアライメントに当てはめ、
                                 //最大サイズであるnormalを縮小し、共用体自体のアライメントも阻止
                                 //し、結果的に構造体全体のサイズを縮めている
    u16 resort_extend_type;
  };

  //action_noによってセットされるものが変わる
  union{  //最大20byteまで
    u16 tr_no;                              ///<トレーナー番号
    u16 itemno;                             ///<アイテム番号
    u16 wazano;                             ///<技番号
    u16 hour;                               ///<時
    u32 thankyou_count;                     ///<お礼を受けた回数
    u32 victory_count;                      ///<連勝数
    u8 trial_house_rank;                    ///<トライアルハウスのランク
    u16 gpower_id;                          ///<GパワーID
    struct{ //通常
      STRCODE nickname[MONS_NAME_SIZE];       ///<ポケモン名(ニックネーム) !! EOM除く !!
    }normal;
    struct{ //配布用
      u16 monsno;                           ///<ポケモン番号
      u16 itemno;                           ///<アイテム番号
      u16 gpower_id;                        ///<GパワーID
      u32 magic_key;                        ///<マジックキー
    }distribution;
    struct{
      STRCODE thankyou_message[GAMEBEACON_THANKYOU_MESSAGE_LEN]; ///お礼メッセージ
      u16 target_trainer_id;                ///<対象相手のトレーナーID(下位16ビット)
    }thanks;
    STRCODE freeword_message[GAMEBEACON_FREEWORD_MESSAGE_LEN]; ///一言メッセージ
    FEST_MISSION  fmission;
    u8 extend_buff[20];   ///<２０バイトのフリーエリア  リゾートで使用
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
  u8 nation;                               ///<国コード
  u8 area;                                 ///<地域コード
  u8 g_power_id:7;                         ///<発動しているGパワーID(GPOWER_ID_xxx)
  u8 sex:1;                                ///<性別       4
  
  u32 suretigai_count:17;                  ///<すれ違い人数
  u32 zone_id:10;                          ///<現在地
  u32 language:5;                          ///<PM_LANG  4
  
  u32 thanks_recv_count:17;                ///<お礼を受けた回数
  u32 send_counter:5;                      ///<送信No(同じデータの無視判定に使用)
  u32 townmap_root_zone_id:10;             ///<タウンマップでの座標テーブルIndex  4

  u32 pm_version:6;
  u32 trainer_view:3;                      ///<ユニオンルームでの見た目
  u32 research_team_rank:3;                ///<調査隊ランク(RESEARCH_TEAM_RANK)
  u32 favorite_color_index:4;              ///<本体情報の色(Index)
  u32 play_hour:10;                        ///<プレイ時間：時(MAX999)
  u32 play_min:6;                          ///<プレイ時間：分(MAX59)  4

  u16 trainer_id;                          ///<トレーナーID(下位16ビット)   2
  STRCODE name[PERSON_NAME_SIZE];          ///<トレーナー名(!! EOM除く !!)  14

  STRCODE self_introduction[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN]; ///<自己紹介メッセージ  16

  GAMEBEACON_INFO_ACTION action;           ///<行動パラメータ   24
  GAMEBEACON_INFO_DETAILS details;         ///<詳細情報         8
  QUESTIONNAIRE_ANSWER_WORK question_answer;  ///<アンケート回答    10

  //BWでのパディング領域2byteをスワンでメダルデータに使用する
  u8  medal_num;                            ///<獲得メダル数
  u8  push_medal;                           ///<イチオシメダルID  

  u32 dummy2;                               ///GAMEBEACON_INFOを領域末尾に含むビーコンのユーザー領域を定義している
                                            ///GBS_BEACON構造体(include/field/game_beacon_search.h)のサイズは104byteまでOK。
                                            ///BWでは100byteだったので、swanでGAMEBEACON_INFOのサイズを4byte足しても問題ないハズ
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
