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

#include "waza_tool/wazadata.h"

#include "btl_server_flow.h"
#include "btl_server.h"


//--------------------------------------------------------------
/**
*	イベントタイプ
*/
//--------------------------------------------------------------
typedef enum {
	BTL_EVENT_NULL = 0,

	BTL_EVENT_CHECK_CHANGE,						///< いれかえチェック
	BTL_EVENT_CHECK_ESCAPE,						///< にげるチェック
	BTL_EVENT_CALC_AGILITY,						///< すばやさ計算
	BTL_EVENT_CHECK_CONF,							///< 混乱チェック
	BTL_EVENT_CALC_CONF_DAMAGE,				///< 混乱ダメージ計算
	BTL_EVENT_WAZA_EXECUTE_POKESICK,	///< ワザ実行チェック（ポケモン系状態異常）
	BTL_EVENT_WAZA_EXECUTE,						///< ワザ実行チェック
	BTL_EVENT_WAZA_EXECUTE_FIX,				///< ワザ実行チェック確定
	BTL_EVENT_WAZA_PARAM,							///< ワザパラメータチェック
	BTL_EVENT_DECIDE_TARGET,					///< ワザ対象決定
	BTL_EVENT_NOEFFECT_TYPE_CHECK,		///< ワザ無効化（タイプによる）チェックハンドラ
	BTL_EVENT_NOEFFECT_CHECK_L1,			///< ワザ無効化チェック（lv1 = 必中には負ける）ハンドラ
	BTL_EVENT_NOEFFECT_CHECK_L2,			///< ワザ無効化チェック（lv2 = 必中にも勝つ）ハンドラ
	BTL_EVENT_DMG_TO_RECOVER,					///< ワザダメージ->回復化チェックハンドラ
	BTL_EVENT_WAZA_HIT_RANK,					///< 命中率・回避率の決定ハンドラ
	BTL_EVENT_WAZA_HIT_RATIO,					///< ワザの命中率補正ハンドラ
	BTL_EVENT_WAZA_HIT_COUNT,					///< ヒット回数決定（複数回ヒットワザのみ）
	BTL_EVENT_CRITICAL_CHECK,					///< クリティカルチェック
	BTL_EVENT_WAZA_POWER,							///< ワザ威力補正ハンドラ
	BTL_EVENT_ATTACKER_POWER_PREV,		///< 攻撃側の能力値（こうげきorとくこう）取得前ハンドラ
	BTL_EVENT_DEFENDER_GUARD_PREV,		///< 防御側の能力値（ぼうぎょorとくぼう）取得前ハンドラ
	BTL_EVENT_ATTACKER_POWER,					///< 攻撃側の能力値（こうげきorとくこう）補正ハンドラ
	BTL_EVENT_DEFENDER_GUARD,					///< 防御側の能力値（ぼうぎょorとくぼう）補正ハンドラ
	BTL_EVENT_DAMAGE_PROC1,						///< タイプ計算前ダメージ補正ハンドラ
	BTL_EVENT_ATTACKER_TYPE,					///< 攻撃側ポケタイプ
	BTL_EVENT_DEFENDER_TYPE,					///< 防御側ポケタイプ
	BTL_EVENT_TYPEMATCH_RATIO,				///< 攻撃側タイプ一致時倍率
	BTL_EVENT_WAZA_ATK_TYPE,					///< 攻撃側ポケタイプチェックハンドラ
	BTL_EVENT_WAZA_DEF_TYPE,					///< 防御側ポケタイプチェックハンドラ
	BTL_EVENT_WAZA_DMG_PROC2,					///< タイプ計算後ダメージ補正ハンドラ
	BTL_EVENT_WAZA_DMG_LAST,					///< 最終ダメージ補正ハンドラ
	BTL_EVENT_WAZA_DMG_AFTER,					///< 単体ダメージ処理後
	BTL_EVENT_WAZA_DMG_AFTER_ATONCE,	///< 一斉ダメージ処理後
	BTL_EVENT_DECREMENT_PP,						///< 使用ワザのPP値を減少

	BTL_EVENT_CALC_REACTION,					///< 反動計算ハンドラ
	BTL_EVENT_ADD_RANK_TARGET,				///< ワザを受けた側への追加効果
	BTL_EVENT_ADD_RANK_USER,					///< ワザを使った側への追加効果
	BTL_EVENT_CHECK_RANKEFF,					///< ランク増減効果の最終成否チェック

	BTL_EVENT_MEMBER_OUT,							///< 個別ポケ退場直前
	BTL_EVENT_MEMBER_IN,							///< 個別ポケ入場直後
	BTL_EVENT_MEMBER_COMP,						///< 全参加ポケ登場後
	BTL_EVENT_GET_RANKEFF_VALUE,			///< ワザによる能力ランク増減値チェック
	BTL_EVENT_BEFORE_RANKUP,					///< 能力上げる（前）
	BTL_EVENT_AFTER_RANKUP,						///< 能力上げた（後）
	BTL_EVENT_BEFORE_RANKDOWN,				///< 能力下げる（前）
	BTL_EVENT_AFTER_RANKDOWN,					///< 能力下げた（後）
	BTL_EVENT_ADD_SICK,								///< ワザの追加効果による状態異常の発生チェック
	BTL_EVENT_MAKE_POKESICK,					///< ポケモン系の状態異常にする
	BTL_EVENT_MAKE_WAZASICK,					///< ワザ系の状態異常にする
	BTL_EVENT_SICK_DAMAGE,						///< 状態異常によるダメージ計算
	BTL_EVENT_SHRINK_CHECK,						///< ひるみ計算
	BTL_EVENT_SHRINK_FIX,							///< ひるみ確定後
	BTL_EVENT_ICHIGEKI_CHECK,					///< 一撃必殺チェック
	BTL_EVENT_NOT_WAZA_DAMAGE,				///< ワザ以外のダメージチェック

	BTL_EVENT_TURNCHECK_BEGIN,				///< ターンチェック（先頭）
	BTL_EVENT_TURNCHECK_END,					///< ターンチェック（終端）
	BTL_EVENT_WEATHER_CHANGE,					///< 天候の変化（直前）
	BTL_EVENT_WEATHER_CHANGE_AFTER,		///< 天候の変化（直後）
	BTL_EVENT_CALC_WEATHER_DAMAGE,		///< 天候ダメージ計算
	BTL_EVENT_SIMPLE_DAMAGE_ENABLE,		///< ワザ以外ダメージ有効判定
	BTL_EVENT_SIMPLE_DAMAGE_REACTION,	///< ワザ以外ダメージ後の反応チェック
	BTL_EVENT_SKILL_SWAP,							///< とくせい入れ替え
	BTL_EVENT_CHECK_PUSHOUT,					///< ふきとばし系ワザチェック
	BTL_EVENT_CALC_DRAIN_VOLUME,			///< ドレイン系ワザ回復量計算

}BtlEventType;


enum {
	BTL_EVARG_COMMON_POKEID = 0,
	BTL_EVARG_COMMON_POKEPOS,
	BTL_EVARG_COMMON_END,

	BTL_EVARG_RANKDOWN_STATUS_TYPE = BTL_EVARG_COMMON_END,
	BTL_EVARG_RANKDOWN_VOLUME,
	BTL_EVARG_RANKDOWN_FAIL_FLAG,


	BTL_EVARG_MAX = 16,
};
//--------------------------------------------------------------
/**
*	イベント変数ラベル
*/
//--------------------------------------------------------------
typedef enum {
	BTL_EVAR_NULL = 0,
	BTL_EVAR_SYS_SEPARATE,
	//---ここから上はシステム予約領域-------------------------
	BTL_EVAR_POKEID,
	BTL_EVAR_POKEID_ATK,
	BTL_EVAR_POKEID_DEF,
	BTL_EVAR_DAMAGED_POKECNT,
	BTL_EVAR_POKEID_DAMAGED1,
	BTL_EVAR_POKEID_DAMAGED2,
	BTL_EVAR_POKEID_DAMAGED3,
	BTL_EVAR_POKEID_DAMAGED4,
	BTL_EVAR_POKEID_DAMAGED5,
	BTL_EVAR_POKEID_DAMAGED6,
	BTL_EVAR_WAZAID,
	BTL_EVAR_WAZA_TYPE,
	BTL_EVAR_USER_TYPE,
	BTL_EVAR_SICKID,
	BTL_EVAR_SICK_CONT,
	BTL_EVAR_STATUS_TYPE,
	BTL_EVAR_POKE_TYPE,
	BTL_EVAR_NOEFFECT_FLAG,
	BTL_EVAR_VOLUME,
	BTL_EVAR_GEN_FLAG,
	BTL_EVAR_FAIL_FLAG,
	BTL_EVAR_FAIL_CAUSE,
	BTL_EVAR_CONF_DMG,
	BTL_EVAR_TURN_COUNT,
	BTL_EVAR_ALMOST_FLAG,
	BTL_EVAR_ADD_PER,
	BTL_EVAR_HIT_RANK,
	BTL_EVAR_AVOID_RANK,
	BTL_EVAR_HITCOUNT_MAX,
	BTL_EVAR_HITCOUNT,
	BTL_EVAR_CRITICAL_RANK,
	BTL_EVAR_CRITICAL_FLAG,
	BTL_EVAR_ITEMUSE_FLAG,
	BTL_EVAR_AGILITY,
	BTL_EVAR_WAZA_POWER,
	BTL_EVAR_DAMAGE,
	BTL_EVAR_POWER,
	BTL_EVAR_GUARD,
	BTL_EVAR_RATIO,
	BTL_EVAR_TYPEAFF,
	BTL_EVAR_DAMAGE_TYPE,
	BTL_EVAR_WEATHER,
	BTL_EVAR_WORK_ADRS,

	BTL_EVAR_MAX,

}BtlEvVarLabel;


//--------------------------------------------------------------
/**
*	イベント変数スタック
*/
//--------------------------------------------------------------
extern void BTL_EVENTVAR_Push( void );
extern void BTL_EVENTVAR_Pop( void );
extern void BTL_EVENTVAR_SetValue( BtlEvVarLabel label, int value );
extern int BTL_EVENTVAR_GetValue( BtlEvVarLabel label );


//------------------------------------------------------------------------------------------
/**
*	イベントハンドラ用ワーク型＆定数
*/
//------------------------------------------------------------------------------------------

/**
 *	ワザダメージ->回復化チェックハンドラ用ワーク
 */
typedef struct {
	u8  pokeID;				///< 対象ポケモンID
	u8  tokFlag;			///< とくせいウインドウ表示
	u16 recoverHP;		///< 回復量

	WazaRankEffect	rankEffect;		///< 回復に加えランク効果が発生する場合に設定
	s16							rankVolume;		///< ランク効果増減値

}BTL_EVWK_DMG_TO_RECOVER;


/**
 *	ワザダメージ受けた後のハンドラ反応タイプ
 */
typedef enum {
	BTL_EV_AFTER_DAMAGED_REACTION_NONE,

	BTL_EV_AFTER_DAMAGED_REACTION_SICK,				///< 対象ポケモンを状態異常にする
	BTL_EV_AFTER_DAMAGED_REACTION_RANK,				///< 対象ポケモンのランク増減効果
	BTL_EV_AFTER_DAMAGED_REACTION_DAMAGE,			///< 対象ポケモンにダメージを与える
	BTL_EV_AFTER_DAMAGED_REACTION_POKETYPE,		///< 対象ポケモンのタイプ変更
	BTL_EV_AFTER_DAMAGED_REACTION_MAX,

}BtlEvAfterDamagedReactionType;

/**
 *	ワザダメージ受けた後のハンドラワーク
 */
typedef struct {

	BtlEvAfterDamagedReactionType		reaction;	///< 反応タイプ

	u8  damagedPokeID;		///< ダメージを受けたポケモンID
	u8	targetPokeID;			///< 効果対象ポケモンID
	u8  tokFlag;					///< とくせいウインドウ表示
	u8	itemUseFlag;			///< アイテム使用フラグ

	WazaSick				sick;				///< 対象ポケモンに与える状態異常効果
	BPP_SICK_CONT		sickCont;		///< 対象ポケモンに与える状態異常の継続パターン
	PokeType				pokeType;		///< 対象ポケモンを変化させるタイプ
	WazaRankEffect	rankType;		///< 対象ポケモンに与えるランク効果タイプ
	int							rankVolume;	///< 対象ポケモンに与えるランク効果値
	u16							damage;			///< 対象ポケモンに与えるダメージ量

}BTL_EVWK_DAMAGE_REACTION;


/**
 *	状態異常チェックハンドラの反応タイプ
 */
typedef enum {

	BTL_EV_SICK_REACTION_NONE = 0,
	BTL_EV_SICK_REACTION_DISCARD,	///< 効果を打ち消す
	BTL_EV_SICK_REACTION_REFRECT,	///< 相手にもうつす

}BtlEvSickReaction;

/**
 *	状態異常チェックハンドラ用ワーク
 */
typedef struct {

	BtlEvSickReaction		reaction;
	WazaSick						discardSickType;
	u8									fDiscardByTokusei;
	u8									discardPokeID;
	u8									fItemResponce;

}BTL_EVWK_ADDSICK;


/**
 *	ランク効果チェックハンドラ用ワーク
 */
typedef struct {

	u8							failFlag;						///< とにかく失敗
	u8							failTokuseiFlag;		///< 対象ポケモンのとくせいによる失敗
	WazaRankEffect	failSpecificType;		///< 特定の効果のみ失敗

}BTL_EVWK_CHECK_RANKEFF;




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
extern void BTL_EVENT_StartTurn( void );
extern void BTL_EVENT_CallHandlers( BTL_SVFLOW_WORK* server, BtlEventType type );


#endif
