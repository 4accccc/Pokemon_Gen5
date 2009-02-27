//=============================================================================================
/**
 * @file	btl_common.h
 * @brief	ポケモンWB バトルシステム ローカル共通ヘッダ
 * @author	taya
 *
 * @date	2008.09.06	作成
 */
//=============================================================================================
#ifndef __BTL_COMMON_H__
#define __BTL_COMMON_H__

#ifdef TEST_IN_DOS
#include "ds_types.h"
#endif


#include <gflib.h>
#include "pm_define.h"
#include "waza_tool\wazadata.h"

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/


//--------------------------------------------------------------
/**
 *	定数群
 */
//--------------------------------------------------------------
enum {
	BTL_CLIENT_MAX = 4,				///< 稼働するクライアント数
	BTL_POSIDX_MAX = 3,				///< クライアントが管理する最大の戦闘位置数
	BTL_PARTY_MEMBER_MAX = 6,	///< パーティのメンバー最大数

// 状態異常処理関連
	BTL_NEMURI_TURN_MIN = 3,	///< 「ねむり」最小ターン数
	BTL_NEMURI_TURN_MAX = 8,	///< 「ねむり」最大ターン数
	BTL_NEMURI_TURN_RANGE = (BTL_NEMURI_TURN_MAX - BTL_NEMURI_TURN_MIN + 1),

	BTL_MAHI_EXE_RATIO = 25,	///< 「まひ」でしびれて動けない確率
	BTL_KORI_MELT_RATIO = 20,	///< 「こおり」が溶ける確率

	BTL_DOKU_SPLIT_DENOM = 8,					///< 「どく」で最大HPの何分の１減るか
	BTL_YAKEDO_SPLIT_DENOM = 8,				///< 「やけど」で最大HPの何分の１減るか
	BTL_YAKEDO_DAMAGE_RATIO = 50,			///< 「やけど」で物理ダメージを減じる率
	BTL_MOUDOKU_SPLIT_DENOM = 16,			///< もうどく時、最大HPの何分の１減るか（基本値=最小値）
	BTL_MOUDOKU_COUNT_MAX = BTL_MOUDOKU_SPLIT_DENOM-1,	///< もうどく時、ダメージ倍率をいくつまでカウントアップするか

};

//--------------------------------------------------------------
/**
 *	陣営を現す
 */
//--------------------------------------------------------------
typedef enum {

	BTL_SIDE_1ST,		///< スタンドアローンならプレイヤー側、通信対戦ならサーバ側
	BTL_SIDE_2ND,

	BTL_SIDE_MAX,

}BtlSide;

//--------------------------------------------------------------
/**
 *	ポケモン立ち位置を現す（画面上の見た目ではなく、概念上の位置）
 */
//--------------------------------------------------------------
typedef enum {

	BTL_POS_1ST_0,		///< スタンドアローンならプレイヤー側、通信対戦ならサーバ側
	BTL_POS_2ND_0,
	BTL_POS_1ST_1,
	BTL_POS_2ND_1,
	BTL_POS_1ST_2,
	BTL_POS_2ND_2,

	BTL_POS_MAX,

}BtlPokePos;

//--------------------------------------------------------------
/**
 *	考えるのが人間かAIか？
 */
//--------------------------------------------------------------
typedef enum {

	BTL_THINKER_UI = 0,
	BTL_THINKER_AI,

	BTL_THINKER_TYPE_MAX,
	BTL_THINKER_NONE = BTL_THINKER_TYPE_MAX,

}BtlThinkerType;

//--------------------------------------------------------------
/**
 *	「逃げる」を選択した時の反応タイプ
 */
//--------------------------------------------------------------
typedef enum {

	BTL_ESCAPE_MODE_OK,		///< すぐに「にげる」確定（野生戦など）
	BTL_ESCAPE_MODE_NG,		///< しょうぶのさいちゅうに背中を見せられない
	BTL_ESCAPE_MODE_CONFIRM,	///< にげると負けになるけどホントにいいの？と確認する

	BTL_ESCAPE_MODE_MAX,

}BtlEscapeMode;

//--------------------------------------------------------------
/**
 *	ダブルバトル時のチーム構成
 */
//--------------------------------------------------------------
typedef enum {
	BTL_DBLTEAM_ALONE_ALONE,	///< 個人vs個人
	BTL_DBLTEAM_ALONE_TAG,		///< 個人vsタッグ
	BTL_DBLTEAM_TAG_ALONE,		///< タッグvs個人
	BTL_DBLTEAM_TAG_TAG,		///< タッグvsタッグ
}BtlDblTeamFormation;




#endif
