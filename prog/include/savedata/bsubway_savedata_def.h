//======================================================================
/**
 * @file	bsubway_savedata_def.h
 * @bfief	バトルサブウェイ　セーブデータ関連定数定義
 * @author iwasawa
 * @date	07.05.28
 * @note プラチナより移植 kagaya
 */
//======================================================================
#ifndef	__SUBWAY_SAVEDATA_DEF_H__
#define __SUBWAY_SAVEDATA_DEF_H__

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
/// プレイデータ取得ID
//--------------------------------------------------------------
typedef enum
{
 BSWAY_PSD_playmode,
 BSWAY_PSD_round,
 BSWAY_PSD_rec_down,
 BSWAY_PSD_rec_turn,
 BSWAY_PSD_rec_damage,
 BSWAY_PSD_pokeno,
 BSWAY_PSD_pare_poke,
 BSWAY_PSD_pare_itemfix,
 BSWAY_PSD_trainer,
 BSWAY_PSD_partner,
 BSWAY_PSD_rnd_seed,
}BSWAY_PSD_ID;

//--------------------------------------------------------------
/// スコアデータ操作モード
//--------------------------------------------------------------
typedef enum
{
 BSWAY_DATA_get,		///<データ取得
 BSWAY_DATA_set,		///<セット
 BSWAY_DATA_reset,	///<リセット
 BSWAY_DATA_inc,		///<1++
 BSWAY_DATA_dec,		///<1--
 BSWAY_DATA_add,		///<add
 BSWAY_DATA_sub,		///<sub
}BSWAY_DATA_SETID;

//--------------------------------------------------------------
/// スコアデータフラグ アクセスID
//--------------------------------------------------------------
typedef enum
{
 BSWAY_SFLAG_SILVER_GET,
 BSWAY_SFLAG_GOLD_GET,
 BSWAY_SFLAG_SILVER_READY,
 BSWAY_SFLAG_GOLD_READY,
 BSWAY_SFLAG_WIFI_LOSE_F,
 BSWAY_SFLAG_WIFI_UPLOAD,
 BSWAY_SFLAG_WIFI_POKE_DATA,
 BSWAY_SFLAG_SINGLE_POKE_DATA,
 BSWAY_SFLAG_SINGLE_RECORD,
 BSWAY_SFLAG_DOUBLE_RECORD,
 BSWAY_SFLAG_MULTI_RECORD,
 BSWAY_SFLAG_CMULTI_RECORD,
 BSWAY_SFLAG_WIFI_RECORD,
 BSWAY_SFLAG_COPPER_GET,
 BSWAY_SFLAG_COPPER_READY,
 BSWAY_SFLAG_WIFI_MULTI_RECORD,		//プラチナ追加
 BSWAY_SFLAG_MAX,					//最大数
}BSWAY_SFLAG_ID;

///プレイヤーメッセージデータ　アクセスID
typedef enum
{
 BSWAY_MSG_PLAYER_READY,
 BSWAY_MSG_PLAYER_WIN,
 BSWAY_MSG_PLAYER_LOSE,
 BSWAY_MSG_LEADER,
}BSWAY_PLAYER_MSG_ID;

///スコアデータ　ポケモンデータアクセスモード
typedef enum
{
 BSWAY_SCORE_POKE_SINGLE,
 BSWAY_SCORE_POKE_WIFI,
}BSWAY_SCORE_POKE_DATA;

///選択する手持ちポケモン数のMAX
#define BSUBWAY_STOCK_MEMBER_MAX	(4)
///保存しておく対戦トレーナーナンバーの数
#define BSUBWAY_STOCK_TRAINER_MAX	(14)
///保存しておくAIマルチペアのポケモンパラメータ数
#define BSUBWAY_STOCK_PAREPOKE_MAX	(2)
///WiFi DLプレイヤーデータ数
#define BSUBWAY_STOCK_WIFI_PLAYER_MAX	(7)
///WiFi DLリーダーデータ数
#define BSUBWAY_STOCK_WIFI_LEADER_MAX	(30)
///ルームデータのDLフラグエリアバッファ長
#define BSUBWAY_ROOM_DATA_FLAGS_LEN	(250)

//kari pt_save.h
#define	WAZA_TEMOTI_MAX		(4)		///<1体のポケモンがもてる技の最大値

#endif //__SUBWAY_SAVEDATA_DEF_H__
