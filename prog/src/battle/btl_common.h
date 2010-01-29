//=============================================================================================
/**
 * @file  btl_common.h
 * @brief ポケモンWB バトルシステム ローカル共通ヘッダ
 * @author  taya
 *
 * @date  2008.09.06  作成
 */
//=============================================================================================
#ifndef __BTL_COMMON_H__
#define __BTL_COMMON_H__


#include <gflib.h>
#include "pm_define.h"
#include "waza_tool\wazadata.h"

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/


//--------------------------------------------------------------
/**
 *  定数群
 */
//--------------------------------------------------------------
enum {
  BTL_CLIENT_MAX = 4,       ///< 稼働するクライアント数
  BTL_POSIDX_MAX = 3,       ///< クライアントが管理する最大の戦闘位置数
  BTL_PARTY_MEMBER_MAX = 6, ///< パーティのメンバー最大数
  BTL_FRONT_POKE_MAX = BTL_POSIDX_MAX*2,  ///< バトルに出ているポケモンの最大数
  BTL_POKEID_MAX = (BTL_CLIENT_MAX*BTL_PARTY_MEMBER_MAX), ///< ポケモンIDの最大値
  BTL_POKEID_NULL = 31,     ///< 無効が保証されるポケモンID（サーバコマンド生成の都合上、5bitに収まるように…）
  BTL_TURNCOUNT_MAX = 9999, ///< カウントするターン数の最大
  BTL_SHOOTER_ENERGY_MAX = 14,  ///< シューターの蓄積エネルギー最大値
  BTL_ROTATE_NUM = 3,       ///< ローテーション対象ポケ数

  BTL_CLIENTID_NULL = BTL_CLIENT_MAX,
  BTL_CLIENTID_COMM_SUPPORT = BTL_CLIENTID_NULL,
};

//--------------------------------------------------------------
/**
 *  陣営を現す
 */
//--------------------------------------------------------------
typedef enum {

  BTL_SIDE_1ST,   ///< スタンドアローンならプレイヤー側、通信対戦ならサーバ側
  BTL_SIDE_2ND,

  BTL_SIDE_MAX,

}BtlSide_tag;

typedef u8 BtlSide;

//--------------------------------------------------------------
/**
 *  ポケモン立ち位置を現す（画面上の見た目ではなく、概念上の位置）
 */
//--------------------------------------------------------------
typedef enum {

  BTL_POS_1ST_0,    ///< スタンドアローンならプレイヤー側、通信対戦ならサーバ側
  BTL_POS_2ND_0,
  BTL_POS_1ST_1,
  BTL_POS_2ND_1,
  BTL_POS_1ST_2,
  BTL_POS_2ND_2,

  BTL_POS_MAX,
  BTL_POS_NULL = BTL_POS_MAX,

}BtlPokePos_tag;

typedef u8  BtlPokePos;

//--------------------------------------------------------------
/**
 *  クライアントタイプ
 */
//--------------------------------------------------------------
typedef enum {

  BTL_CLIENT_TYPE_UI = 0,   ///< プレイヤー
  BTL_CLIENT_TYPE_AI,       ///< AI
  BTL_CLIENT_TYPE_REC,      ///< 録画再生

  BTL_CLIENT_TYPE_MAX,
  BTL_CLIENT_TYPE_NONE = BTL_CLIENT_TYPE_MAX,

}BtlClientType_tag;

typedef  u8  BtlClientType;

//--------------------------------------------------------------
/**
 *  「逃げる」を選択した時の反応タイプ
 */
//--------------------------------------------------------------
typedef enum {

  BTL_ESCAPE_MODE_OK,       ///< すぐに「にげる」確定（野生戦など）
  BTL_ESCAPE_MODE_NG,       ///< しょうぶのさいちゅうに背中を見せられない
  BTL_ESCAPE_MODE_CONFIRM,  ///< にげると負けになるけどホントにいいの？と確認する

  BTL_ESCAPE_MODE_MAX,

}BtlEscapeMode;

//--------------------------------------------------------------
/**
 *  ダブルバトル時のチーム構成
 */
//--------------------------------------------------------------
typedef enum {
  BTL_DBLTEAM_ALONE_ALONE,  ///< 個人vs個人
  BTL_DBLTEAM_ALONE_TAG,    ///< 個人vsタッグ
  BTL_DBLTEAM_TAG_ALONE,    ///< タッグvs個人
  BTL_DBLTEAM_TAG_TAG,      ///< タッグvsタッグ
}BtlDblTeamFormation;

//--------------------------------------------------------------
/**
 *  ローテーションバトルの回転方向
 */
//--------------------------------------------------------------
typedef enum {
  BTL_ROTATEDIR_NONE = 0,
  BTL_ROTATEDIR_STAY,
  BTL_ROTATEDIR_R,
  BTL_ROTATEDIR_L,
}BtlRotateDir_tag;

typedef u8 BtlRotateDir;

//--------------------------------------------------------------
/**
 *  はい／いいえ選択
 */
//--------------------------------------------------------------
typedef enum {
  BTL_YESNO_YES = 0,
  BTL_YESNO_NO,
}BtlYesNo;

//--------------------------------------------------------------
/**
 *  ポケモンひん死の時の処理タイプ
 */
//--------------------------------------------------------------
typedef enum {
  BTL_CHANGEMODE_IREKAE = 0,  ///<
  BTL_CHANGEMODE_KATINUKI,
}BtlChangeMode;



#endif
