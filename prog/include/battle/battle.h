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

#ifdef TEST_IN_DOS
#include "ds_types.h"
#endif

#include <net.h>
#include <procsys.h>

#include "poke_tool/pokeparty.h"
#include "tr_tool/trno_def.h"
#include "savedata/mystatus.h"
#include "savedata/myitem_savedata.h"

//--------------------------------------------------------------
/**
 *  駆動タイプ
 */
//--------------------------------------------------------------
typedef enum {

  BTL_ENGINE_ALONE,         ///< スタンドアロン
  BTL_ENGINE_COMM_PARENT,   ///< 通信（親）
  BTL_ENGINE_COMM_CHILD,    ///< 通信（子）

}BtlEngineType;

//--------------------------------------------------------------
/**
 *  対戦ルール
 */
//--------------------------------------------------------------
typedef enum {

  BTL_RULE_SINGLE,    ///< シングル
  BTL_RULE_DOUBLE,    ///< ダブル
  BTL_RULE_TRIPLE,    ///< トリプル

}BtlRule;

//--------------------------------------------------------------
/**
 *  対戦相手
 */
//--------------------------------------------------------------
typedef enum {

  BTL_COMPETITOR_WILD,    ///< 野生
  BTL_COMPETITOR_TRAINER,   ///< ゲーム内トレーナー
  BTL_COMPETITOR_COMM,    ///< 通信対戦

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
  BTL_WEATHER_MIST,     ///< きり

  BTL_WEATHER_MAX,

}BtlWeather;

//--------------------------------------------------------------
/**
 *  地形（@@@ まだ暫定的なものです。不明な場合、GRASSを指定してください）
 */
//--------------------------------------------------------------
typedef enum {
  BTL_LANDFORM_GRASS,   ///< 草むら
  BTL_LANDFORM_SAND,    ///< 砂地
  BTL_LANDFORM_SEA,     ///< 海
  BTL_LANDFORM_RIVER,   ///< 川
  BTL_LANDFORM_SNOW,    ///< 雪原
  BTL_LANDFORM_CAVE,    ///< 洞窟
  BTL_LANDFORM_ROCK,    ///< 岩場
  BTL_LANDFORM_ROOM,    ///< 室内

  BTL_LANDFORM_MAX,
}BtlLandForm;


//-----------------------------------------------------------------------------------
/**
 *  地形
 */
//-----------------------------------------------------------------------------------
typedef struct {

  BtlEngineType   engine;
  BtlCompetitor   competitor;
  BtlRule         rule;
  BtlLandForm     landForm;

  GFL_NETHANDLE*  netHandle;
  BtlCommMode     commMode;
  u8              commPos;    ///< 通信対戦なら自分の立ち位置（非通信時は無視）
  u8              netID;      ///< NetID
  u8              multiMode;  ///< ダブルの時、１だとマルチバトル。

  POKEPARTY*      partyPlayer;  ///< プレイヤーのパーティ
  POKEPARTY*      partyPartner; ///< 2vs2時の味方AI（不要ならnull）
  POKEPARTY*      partyEnemy1;  ///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
  POKEPARTY*      partyEnemy2;  ///< 2vs2時の２番目敵AI用（不要ならnull）

  const MYSTATUS*   statusPlayer; ///< プレイヤーのステータス
  MYITEM*           itemData;     ///< アイテムデータ
  TrainerID         trID;         ///<対戦相手トレーナーID（7/31ROMでトレーナーエンカウントを実現するための暫定）

  u16       musicDefault;   ///< デフォルト時のBGMナンバー
  u16       musicPinch;     ///< ピンチ時のBGMナンバー

}BATTLE_SETUP_PARAM;


//--------------------------------------------------------------
/**
 *  Proc Data
 */
//--------------------------------------------------------------
extern const GFL_PROC_DATA BtlProcData;


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
