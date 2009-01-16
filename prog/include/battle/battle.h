//=============================================================================================
/**
 * @file	battle.h
 * @brief	ポケモンWB バトルシステム 外部システムとのインターフェイス
 * @author	taya
 *
 * @date	2008.09.22	作成
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

//--------------------------------------------------------------
/**
 *	駆動タイプ
 */
//--------------------------------------------------------------
typedef enum {

	BTL_ENGINE_ALONE,			///< スタンドアロン
	BTL_ENGINE_COMM_PARENT,		///< 通信（親）
	BTL_ENGINE_COMM_CHILD,		///< 通信（子）

}BtlEngineType;

//--------------------------------------------------------------
/**
 *	対戦ルール
 */
//--------------------------------------------------------------
typedef enum {

	BTL_RULE_SINGLE,		///< シングル
	BTL_RULE_DOUBLE,		///< ダブル
	BTL_RULE_TRIPLE,		///< トリプル

}BtlRule;

//--------------------------------------------------------------
/**
 *	対戦相手
 */
//--------------------------------------------------------------
typedef enum {

	BTL_COMPETITOR_WILD,		///< 野生
	BTL_COMPETITOR_TRAINER,		///< ゲーム内トレーナー
	BTL_COMPETITOR_COMM,		///< 通信対戦

}BtlCompetitor;

//--------------------------------------------------------------
/**
 *	通信タイプ
 */
//--------------------------------------------------------------
typedef enum {

	BTL_COMM_NONE,	///< 通信しない
	BTL_COMM_DS,	///< DS無線通信
	BTL_COMM_WIFI,	///< Wi-Fi通信

}BtlCommMode;


typedef struct {

	BtlEngineType		engine;
	BtlCompetitor		competitor;
	BtlRule				rule;

	GFL_NETHANDLE*		netHandle;
	BtlCommMode			commMode;
	u8							commPos;		///< 通信対戦なら自分の立ち位置（非通信時は無視）
	u8							netID;			///< NetID

	POKEPARTY*			partyPlayer;	///< プレイヤーのパーティ
	POKEPARTY*			partyPartner;	///< 2vs2時の味方AI（不要ならnull）
	POKEPARTY*			partyEnemy1;	///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
	POKEPARTY*			partyEnemy2;	///< 2vs2時の２番目敵AI用（不要ならnull）



}BATTLE_SETUP_PARAM;


//--------------------------------------------------------------
/**
 *	Proc Data
 */
//--------------------------------------------------------------
extern const GFL_PROC_DATA BtlProcData;


#endif
