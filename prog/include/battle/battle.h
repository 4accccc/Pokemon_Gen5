//=============================================================================================
/**
 * @file  battle.h
 * @brief ポケモンWB バトルシステム 外部システムとのインターフェイス
 * @author  taya
 *
 * @date  2008.09.22  作成
 */
//=============================================================================================
#ifndef __BATTLE_H__
#define __BATTLE_H__

#include <gflib.h>
#include <net.h>
#include <procsys.h>

#include "system/timezone.h"
#include "system/pms_data.h"
#include "poke_tool/pokeparty.h"
#include "tr_tool/trno_def.h"
#include "savedata/zukan_savedata.h"
#include "savedata/mystatus.h"
#include "savedata/myitem_savedata.h"
#include "savedata/config.h"
#include "gamesystem/comm_player_support.h"
#include "buflen.h"

#include "battle_bg_def.h"  //zonetableコンバータから参照させたいので定義を分離しました by iwasawa

//--------------------------------------------------------------
/**
 *  対戦ルール
 */
//--------------------------------------------------------------
typedef enum {

  BTL_RULE_SINGLE,    ///< シングル
  BTL_RULE_DOUBLE,    ///< ダブル
  BTL_RULE_TRIPLE,    ///< トリプル
  BTL_RULE_ROTATION,  ///< ローテーション

}BtlRule;

//--------------------------------------------------------------
/**
 *  対戦相手
 */
//--------------------------------------------------------------
typedef enum {

  BTL_COMPETITOR_WILD,    ///< 野生
  BTL_COMPETITOR_TRAINER, ///< ゲーム内トレーナー
  BTL_COMPETITOR_SUBWAY,  ///< バトルサブウェイトレーナー
  BTL_COMPETITOR_COMM,    ///< 通信対戦

  BTL_COMPETITOR_MAX,

}BtlCompetitor;

//--------------------------------------------------------------
/**
 *  通信タイプ
 */
//--------------------------------------------------------------
typedef enum {

  BTL_COMM_NONE,  ///< 通信しない
  BTL_COMM_DS,    ///< DS無線通信
  BTL_COMM_WIFI,  ///< Wi-Fi通信

}BtlCommMode;


//--------------------------------------------------------------
/**
 *  天候
 */
//--------------------------------------------------------------
typedef enum {

  BTL_WEATHER_NONE = 0,   ///< 天候なし

  BTL_WEATHER_SHINE,    ///< はれ
  BTL_WEATHER_RAIN,     ///< あめ
  BTL_WEATHER_SNOW,     ///< あられ
  BTL_WEATHER_SAND,     ///< すなあらし

  BTL_WEATHER_MAX,

}BtlWeather;

//--------------------------------------------------------------
/**
 *  勝敗コード
 */
//--------------------------------------------------------------
typedef enum {

  BTL_RESULT_LOSE,        ///< 負けた
  BTL_RESULT_WIN,         ///< 勝った
  BTL_RESULT_DRAW,        ///< ひきわけ
  BTL_RESULT_RUN,         ///< 逃げた
  BTL_RESULT_RUN_ENEMY,   ///< 相手が逃げた（野生のみ）
  BTL_RESULT_CAPTURE,     ///< 捕まえた（野生のみ）

  BTL_RESULT_MAX,

}BtlResult;

typedef enum{
  BTL_CLIENT_PLAYER,
  BTL_CLIENT_ENEMY1,
  BTL_CLIENT_PARTNER,
  BTL_CLIENT_ENEMY2,
  BTL_CLIENT_NUM,
}BTL_CLIENT_ID;

//-----------------------------------------------------------------------------------
/**
 * フィールドの状態から決定されるバトルシチュエーションデータ
 */
//-----------------------------------------------------------------------------------
typedef struct {

  BtlBgType   bgType;
  BtlBgAttr   bgAttr;
  BtlWeather  weather;

  TIMEZONE    timeZone;
  u8          season;
}BTL_FIELD_SITUATION;

//-----------------------------------------------------------------------------------
/**
 * トレーナーデータ構造体
 */
//-----------------------------------------------------------------------------------
enum {
  BSP_TRAINERDATA_ITEM_MAX = 4,
};

typedef struct {
  u32     tr_id;
  u32     tr_type;          //トレーナー分類
  u32     ai_bit;
  u16     use_item[BSP_TRAINERDATA_ITEM_MAX];      //使用道具

  STRBUF*   name; //トレーナー名

  //バトルサブウェイ他 Wifi-DLトレーナーとの対戦時にのみ必要
  PMS_DATA  win_word;   //戦闘終了時勝利メッセージ
  PMS_DATA  lose_word;  //戦闘終了時負けメッセージ

}BSP_TRAINER_DATA;

//--------------------------------------------------------------
/**
 *  デバッグフラグ
 */
//--------------------------------------------------------------
typedef enum {

  BTL_DEBUGFLAG_MUST_TUIKA = 0, ///< 追加効果の発生率を100％にする
  BTL_DEBUGFLAG_MUST_TOKUSEI,   ///< 一部とくせいの効果発生率を100％にする
  BTL_DEBUGFLAG_MUST_ITEM,      ///< 一部アイテムの効果発生率を100％にする
  BTL_DEBUGFLAG_MUST_CRITICAL,  ///< 打撃ワザのクリティカル発生率を100％にする
  BTL_DEBUGFLAG_HP_CONST,       ///< HP減らない
  BTL_DEBUGFLAG_PP_CONST,       ///< PP減らない
  BTL_DEBUGFLAG_DMG_RAND_OFF,   ///< ダメージのランダム補正を行わない

  BTL_DEBUGFLAG_MAX,

}BtlDebugFlag;


//-----------------------------------------------------------------------------------
/**
 *  バトルセットアップパラメータ
 */
//-----------------------------------------------------------------------------------
typedef struct {

  //バトルルール
  BtlCompetitor   competitor;
  BtlRule         rule;

  //フィールドの状態から決定されるバトルシチュエーションデータ
  BTL_FIELD_SITUATION   fieldSituation;
  u16             musicDefault;   ///< デフォルト時のBGMナンバー
  u16             musicPinch;     ///< ピンチ時のBGMナンバー

  //通信データ
  GFL_NETHANDLE*  netHandle;
  BtlCommMode     commMode;
  u8              commPos;    ///< 通信対戦なら自分の立ち位置（非通信時は常に0を指定）
  u8              multiMode;  ///< ダブルの時、１だとマルチバトル。
  u8              fRecordPlay;///< TRUEだと録画データ再生モード

  // [io] パーティデータ・対戦相手ステータス
  //（通信バトル後、録画データ生成のために操作プレイヤー以外のデータが格納される）
  POKEPARTY*        party[ BTL_CLIENT_NUM ];
  MYSTATUS*         playerStatus[ BTL_CLIENT_NUM ];

  // トレーナーデータ
  BSP_TRAINER_DATA*  tr_data[BTL_CLIENT_NUM];

  // セーブデータ系
  const CONFIG*     configData;   ///< コンフィグデータ
  MYITEM*           itemData;     ///< アイテムデータ
  BAG_CURSOR*       bagCursor;    ///< バッグカーソルデータ
  ZUKAN_SAVEDATA*   zukanData;    ///< 図鑑データ

  // ゲームデータ系
  COMM_PLAYER_SUPPORT*  commSupport;  ///< 通信プレイヤーサポート

  // デバッグ系
  u16         DebugFlagBit;          ///< 各種デバッグフラグ


  //----- 以下、バトルの結果格納パラメータ ----
  BtlResult   result;           ///< 勝敗結果
  u16         getMoney;         ///< 手に入れたお小遣い
  u8          capturedPokeIdx;  ///< 捕獲したポケモンのメンバー内インデックス（partyEnemy1 内）

  u8*         recBuffer;        ///< 録画データ生成先
  u32         recDataSize;      ///< 録画データサイズ
  GFL_STD_RandContext  recRandContext;    ///< 録画データ用乱数コンテキスト

}BATTLE_SETUP_PARAM;


//--------------------------------------------------------------
/**
 *  Proc Data
 */
//--------------------------------------------------------------
extern const GFL_PROC_DATA BtlProcData;

// オーバーレイモジュール宣言
FS_EXTERN_OVERLAY(battle);

//--------------------------------------------------------------
/**
 *  NetFunc Table
 */
//--------------------------------------------------------------
extern const NetRecvFuncTable BtlRecvFuncTable[];
enum {
  BTL_NETFUNCTBL_ELEMS = 6,
};


#endif
