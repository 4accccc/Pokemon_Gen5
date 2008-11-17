//=============================================================================================
/**
 * @file	btl_event.h
 * @brief	ポケモンWB バトルシステム	イベント＆ハンドラ処理
 * @author	taya
 *
 * @date	2008.10.23	作成
 */
//=============================================================================================
#ifndef __BTL_EVENT_H__
#define __BTL_EVENT_H__


#include "btl_server.h"

//--------------------------------------------------------------
/**
*	イベントタイプ
*/
//--------------------------------------------------------------
typedef enum {
	BTL_EVENT_NULL = 0,

	BTL_EVENT_CHECK_CONF,			///< 混乱チェック
	BTL_EVENT_CALC_CONF_DAMAGE,		///< 混乱ダメージ計算
	BTL_EVENT_WAZA_EXECUTE,			///< ワザ実行チェック

	BTL_EVENT_WAZA_HIT_RATIO,		///< ワザの命中率補正ハンドラ
	BTL_EVENT_WAZA_HIT_COUNT,		///< ヒット回数決定（複数回ヒットワザのみ）
	BTL_EVENT_CRITICAL_RANK,		///< クリティカル率補正ハンドラ
	BTL_EVENT_WAZA_POWER,			///< ワザ威力補正ハンドラ
	BTL_EVENT_ATTACKER_POWER,		///< 攻撃側の能力値（こうげきorとくこう）補正ハンドラ
	BTL_EVENT_DEFENDER_GUARD,		///< 防御側の能力値（ぼうぎょorとくぼう）補正ハンドラ
	BTL_EVENT_DAMAGE_DENOM,			///< 防御分母補正ハンドラ
	BTL_EVENT_DAMAGE_PROC1,			///< タイプ計算前ダメージ補正ハンドラ
	BTL_EVENT_ATTACKER_TYPE,		///< 攻撃側ポケタイプ
	BTL_EVENT_DEFENDER_TYPE,		///< 防御側ポケタイプ
	BTL_EVENT_TYPEMATCH_RATIO,		///< 攻撃側タイプ一致時倍率
	BTL_EVENT_WAZA_TYPE,			///< ワザタイプチェックハンドラ
	BTL_EVENT_WAZA_ATK_TYPE,		///< 攻撃側ポケタイプチェックハンドラ
	BTL_EVENT_WAZA_DEF_TYPE,		///< 防御側ポケタイプチェックハンドラ
	BTL_EVENT_WAZA_DMG_PROC2,		///< タイプ計算後ダメージ補正ハンドラ
	BTL_EVENT_WAZA_DMG_LAST,		///< 最終ダメージ補正ハンドラ
	BTL_EVENT_WAZA_DMG_AFTER,		///< ダメージ処理後

	BTL_EVENT_WAZA_REFRECT,			///< 反動計算ハンドラ
	BTL_EVENT_WAZA_ADD_TARGET,		///< ワザを受けた側への追加効果
	BTL_EVENT_WAZA_ADD_USER,		///< ワザを使った側への追加効果

	BTL_EVENT_MEMBER_OUT,			///< 個別ポケ退場直前
	BTL_EVENT_MEMBER_IN,			///< 個別ポケ入場直後
	BTL_EVENT_MEMBER_COMP,			///< 全参加ポケ登場後
	BTL_EVENT_BEFORE_RANKDOWN,		///< 能力下げる（前）
	BTL_EVENT_AFTER_RANKDOWN,		///< 能力下げる（後）

}BtlEventType;


enum {
	BTL_EVARG_COMMON_CLIENT_ID = 0,
	BTL_EVARG_COMMON_END,

	BTL_EVARG_RANKDOWN_STATUS_TYPE = BTL_EVARG_COMMON_END,
	BTL_EVARG_RANKDOWN_VOLUME,
	BTL_EVARG_RANKDOWN_FAIL_FLAG,


	BTL_EVARG_MAX = 16,
};



//--------------------------------------------------------------
/**
*	イベントファクタータイプ
*/
//--------------------------------------------------------------
typedef enum {

	BTL_EVENT_FACTOR_WAZA,
	BTL_EVENT_FACTOR_TOKUSEI,
	BTL_EVENT_FACTOR_ITEM,

}BtlEventFactor;


extern void BTL_EVENT_InitSystem( void );
extern void BTL_EVENT_CallHandlers( BTL_SERVER* server, BtlEventType type );


#endif
