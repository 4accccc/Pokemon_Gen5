//======================================================================
/**
 * @file  bsubway_savedata.h
 * バトルサブウェイ　セーブデータ関連
 * @author iwasawa
 * @date  07.05.28
 * @note プラチナより移植 kagaya
 */
//======================================================================
#ifndef __H_BSUBWAY_SAVEDATA_H__
#define __H_BSUBWAY_SAVEDATA_H__

#include "gamesystem/game_data.h"

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
/// 定数
//--------------------------------------------------------------
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
///WiFi 手持ちポケモン数
#define BSUBWAY_STOCK_WIFI_MEMBER_MAX	(3)

//kari pt_save.h
#define	WAZA_TEMOTI_MAX (4) ///<1体のポケモンがもてる技の最大値

//--------------------------------------------------------------
/// プレイデータ取得ID
//--------------------------------------------------------------
typedef enum
{
  BSWAY_PLAYDATA_ID_playmode,
  BSWAY_PLAYDATA_ID_round,
  BSWAY_PLAYDATA_ID_rec_down,
  BSWAY_PLAYDATA_ID_rec_turn,
  BSWAY_PLAYDATA_ID_rec_damage,
  BSWAY_PLAYDATA_ID_pokeno,
  BSWAY_PLAYDATA_ID_pare_poke,
  BSWAY_PLAYDATA_ID_pare_itemfix,
  BSWAY_PLAYDATA_ID_trainer,
  BSWAY_PLAYDATA_ID_partner,
  BSWAY_PLAYDATA_ID_use_battle_box,
  BSWAY_PLAYDATA_ID_MAX,
}BSWAY_PLAYDATA_ID;

//--------------------------------------------------------------
/// スコアデータ操作モード
//--------------------------------------------------------------
typedef enum
{
  BSWAY_SETMODE_get,		///<データ取得
  BSWAY_SETMODE_set,		///<セット
  BSWAY_SETMODE_reset,	///<リセット
  BSWAY_SETMODE_inc,		///<1++
  BSWAY_SETMODE_dec,		///<1--
  BSWAY_SETMODE_add,		///<add
  BSWAY_SETMODE_sub,		///<sub
  BSWAY_SETMODE_MAX,
}BSWAY_SETMODE;

//--------------------------------------------------------------
/// スコアデータフラグ アクセス
//--------------------------------------------------------------
typedef enum
{
  BSWAY_SCOREDATA_FLAG_SILVER_GET,
  BSWAY_SCOREDATA_FLAG_GOLD_GET,
  BSWAY_SCOREDATA_FLAG_SILVER_READY,
  BSWAY_SCOREDATA_FLAG_GOLD_READY,
  BSWAY_SCOREDATA_FLAG_WIFI_LOSE_F,
  BSWAY_SCOREDATA_FLAG_WIFI_UPLOAD,
  BSWAY_SCOREDATA_FLAG_WIFI_POKE_DATA,
  BSWAY_SCOREDATA_FLAG_SINGLE_POKE_DATA,
  BSWAY_SCOREDATA_FLAG_SINGLE_RECORD,
  BSWAY_SCOREDATA_FLAG_DOUBLE_RECORD,
  BSWAY_SCOREDATA_FLAG_MULTI_RECORD,
  BSWAY_SCOREDATA_FLAG_CMULTI_RECORD,
  BSWAY_SCOREDATA_FLAG_WIFI_RECORD,
  BSWAY_SCOREDATA_FLAG_COPPER_GET,
  BSWAY_SCOREDATA_FLAG_COPPER_READY,
  BSWAY_SCOREDATA_FLAG_WIFI_MULTI_RECORD,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_SINGLE,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_DOUBLE,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_MULTI,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_COMM_MULTI,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_WIFI,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_WIFI_MULTI,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_SINGLE_S,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_DOUBLE_S,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_MULTI_S,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_COMM_MULTI_S,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_WIFI_S,
  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_WIFI_MULTI_S,
  BSWAY_SCOREDATA_FLAG_SUPPORT_ENCOUNT_END,
  BSWAY_SCOREDATA_FLAG_MAX,
}BSWAY_SCOREDATA_FLAG;

//--------------------------------------------------------------
/// プレイヤーメッセージデータ　アクセスID
//--------------------------------------------------------------
typedef enum
{
 BSWAY_MSG_PLAYER_READY,
 BSWAY_MSG_PLAYER_WIN,
 BSWAY_MSG_PLAYER_LOSE,
 BSWAY_MSG_LEADER,
}BSWAY_PLAYER_MSG_ID;

//--------------------------------------------------------------
/// スコアデータ　ポケモンデータアクセスモード
//--------------------------------------------------------------
typedef enum
{
 BSWAY_SCORE_POKE_SINGLE,
 BSWAY_SCORE_POKE_WIFI,
}BSWAY_SCORE_POKE_DATA;

//--------------------------------------------------------------
/// BSWAY_PLAYMODE
//--------------------------------------------------------------
typedef enum
{
  BSWAY_PLAYMODE_SINGLE, //シングル
  BSWAY_PLAYMODE_DOUBLE, //ダブル
  BSWAY_PLAYMODE_MULTI, //マルチ
  BSWAY_PLAYMODE_COMM_MULTI, //通信マルチ
  BSWAY_PLAYMODE_WIFI, //Wifi
  BSWAY_PLAYMODE_S_SINGLE, //スーパーシングル
  BSWAY_PLAYMODE_S_DOUBLE, //スーパーダブル
  BSWAY_PLAYMODE_S_MULTI, //スーパーマルチ
  BSWAY_PLAYMODE_S_COMM_MULTI, //スーパー通信マルチ
  BSWAY_PLAYMODE_MAX,
}BSWAY_PLAYMODE;

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// バトルサブウェイ プレイデータ構造体
//--------------------------------------------------------------
typedef struct _BSUBWAY_PLAYDATA BSUBWAY_PLAYDATA;

//--------------------------------------------------------------
/// バトルサブウェイ　スコアデータ構造体
//--------------------------------------------------------------
typedef struct _BSUBWAY_SCOREDATA BSUBWAY_SCOREDATA;

//--------------------------------------------------------------
/// バトルサブウェイ プレイヤーメッセージデータ構造体
//--------------------------------------------------------------
typedef struct _BSUBWAY_PLAYER_MSG  BSUBWAY_PLAYER_MSG;

//--------------------------------------------------------------
/// バトルサブウェイ　AIマルチペアの保存が必要なポケモンパラメータ
//  battle/bsubway_battle_data.h
//--------------------------------------------------------------
typedef struct _BSUBWAY_PAREPOKE_PARAM BSUBWAY_PAREPOKE_PARAM;

//--------------------------------------------------------------
/// サブウェイポケモンデータ型
/// battle/b_tower_data.hを必要な箇所以外では
/// includeしないで済むように定義しておく
//--------------------------------------------------------------
typedef struct _BSUBWAY_POKEMON  BSUBWAY_POKEMON;

//--------------------------------------------------------------
///  サブウェイパートナーデータ型
///  　battle/b_tower_data.hを必要な箇所以外では
///   includeしないで済むように定義しておく
//--------------------------------------------------------------
typedef struct _BSUBWAY_PARTNER_DATA  BSUBWAY_PARTNER_DATA;

//--------------------------------------------------------------
///  サブウェイリーダーデータ型
///  　battle/b_tower_data.hを必要な箇所以外では
///   includeしないで済むように定義しておく
//--------------------------------------------------------------
typedef struct _BSUBWAY_LEADER_DATA  BSUBWAY_LEADER_DATA;

//--------------------------------------------------------------
/// バトルサブウェイ WIFIデータ型
//--------------------------------------------------------------
typedef struct _BSUBWAY_WIFI_DATA  BSUBWAY_WIFI_DATA;

//--------------------------------------------------------------
/// バトルサブウェイ WIFIプレイヤーデータ型
//--------------------------------------------------------------
typedef struct _BSUBWAY_WIFI_PLAYER  BSUBWAY_WIFI_PLAYER;



//======================================================================
//  GAMEDATAから取得
//======================================================================
extern BSUBWAY_PLAYDATA * GAMEDATA_GetBSubwayPlayData( GAMEDATA* gamedata );
extern BSUBWAY_SCOREDATA * GAMEDATA_GetBSubwayScoreData( GAMEDATA* gamedata );
extern BSUBWAY_WIFI_DATA * GAMEDATA_GetBSubwayWifiData( GAMEDATA* gamedata );


//======================================================================
//  extern
//======================================================================
//BSUBWAY_PLAYDATA
extern int BSUBWAY_PLAYDATA_GetWorkSize( void );
extern void BSUBWAY_PLAYDATA_Init( BSUBWAY_PLAYDATA *bsw_play );
extern void BSUBWAY_PLAYDATA_SetSaveFlag(
    BSUBWAY_PLAYDATA *bsw_play, BOOL flag );
extern BOOL BSUBWAY_PLAYDATA_GetSaveFlag( BSUBWAY_PLAYDATA *bsw_play );
extern u32 BSUBWAY_PLAYDATA_GetData(
  const BSUBWAY_PLAYDATA *bsw_play, BSWAY_PLAYDATA_ID id, void *buf );
extern void BSUBWAY_PLAYDATA_SetData(
    BSUBWAY_PLAYDATA *bsw_play, BSWAY_PLAYDATA_ID id, const void *buf );
extern void BSUBWAY_PLAYDATA_AddWifiRecord(
    BSUBWAY_PLAYDATA *bsw_play, u8 down,u16 turn,u16 damage );
extern void BSUBWAY_PLAYDATA_ResetRoundNo( BSUBWAY_PLAYDATA *bsw_play );
extern void BSUBWAY_PLAYDATA_IncRoundNo( BSUBWAY_PLAYDATA *bsw_play );
extern void BSUBWAY_PLAYDATA_SetRoundNo( BSUBWAY_PLAYDATA *bsw_play, u8 round );
extern u16 BSUBWAY_PLAYDATA_GetRoundNo( const BSUBWAY_PLAYDATA *bsw_play );

//BSUBWAY_SCOREDATA
extern int BSUBWAY_SCOREDATA_GetWorkSize( void );
extern void BSUBWAY_SCOREDATA_Init( BSUBWAY_SCOREDATA *bsw_score );
extern void BSUBWAY_SCOREDATA_AddBattlePoint(
    BSUBWAY_SCOREDATA *bsw_score, u16 num );
extern void BSUBWAY_SCOREDATA_SubBattlePoint(
    BSUBWAY_SCOREDATA *bsw_score, u16 num );
extern void BSUBWAY_SCOREDATA_SetBattlePoint(
    BSUBWAY_SCOREDATA *bsw_score, u16 num );
extern u16 BSUBWAY_SCOREDATA_GetBattlePoint(
    const BSUBWAY_SCOREDATA *bsw_score );
extern void BSUBWAY_SCOREDATA_IncRenshou(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
extern void BSUBWAY_SCOREDATA_ResetRenshou(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
extern u16 BSUBWAY_SCOREDATA_GetRenshou(
    const BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
extern u16 BSUBWAY_SCOREDATA_GetRenshouMax(
    const BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
extern void BSUBWAY_SCOREDATA_SetRenshouMax(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode, u16 count );
extern void BSUBWAY_SCOREDATA_UpdateRenshouMax(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode, u16 count );
extern void BSUBWAY_SCOREDATA_ResetStageNo( BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
extern void BSUBWAY_SCOREDATA_IncStageNo( BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
extern void BSUBWAY_SCOREDATA_SetStageNo( BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode, u16 stage );
extern u16 BSUBWAY_SCOREDATA_GetStageNo( const BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );

#if 0
extern void BSUBWAY_SCOREDATA_IncStageCount(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
extern void BSUBWAY_SCOREDATA_ResetStageCount(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
extern u16 BSUBWAY_SCOREDATA_GetStageCount(
    const BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
#endif
extern BOOL BSUBWAY_SCOREDATA_SetFlag( BSUBWAY_SCOREDATA *bsw_score,
    BSWAY_SCOREDATA_FLAG id, BSWAY_SETMODE mode );
extern void BSUBWAY_SCOREDATA_IncWifiRank( BSUBWAY_SCOREDATA *bsw_score );
extern void BSUBWAY_SCOREDATA_DecWifiRank( BSUBWAY_SCOREDATA *bsw_score );
extern u8 BSUBWAY_SCOREDATA_GetWifiRank(
    const BSUBWAY_SCOREDATA *bsw_score );
extern u8 BSUBWAY_SCOREDATA_SetWifiLoseCount(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_SETMODE mode );
extern void BSUBWAY_SCOREDATA_SetUsePokeData( BSUBWAY_SCOREDATA *bsw_score, BSWAY_SCORE_POKE_DATA mode, const BSUBWAY_POKEMON* poke_tbl );
extern void BSUBWAY_SCOREDATA_GetUsePokeData( const BSUBWAY_SCOREDATA *bsw_score, BSWAY_SCORE_POKE_DATA mode, BSUBWAY_POKEMON* poke_tbl );
extern void BSUBWAY_SCOREDATA_SetWifiScore( BSUBWAY_SCOREDATA *bsw_score, const BSUBWAY_PLAYDATA *bsw_play );
extern void BSUBWAY_SCOREDATA_ClearWifiScore( BSUBWAY_SCOREDATA *bsw_score );
extern u16 BSUBWAY_SCOREDATA_GetWifiScore( const BSUBWAY_SCOREDATA *bsw_score );
extern u8 BSUBWAY_SCOREDATA_GetWifiNum( const BSUBWAY_SCOREDATA *bsw_score );



//BSUBWAY_WIFIDATA
extern int BSUBWAY_WIFIDATA_GetWorkSize( void );
extern void BSUBWAY_WIFIDATA_Init( BSUBWAY_WIFI_DATA *bsw_wifi );
extern void BSUBWAY_WIFIDATA_SetRoomDataFlag( BSUBWAY_WIFI_DATA *bsw_wifi, u8 rank, u8 room, const RTCDate *day );
extern void BSUBWAY_WIFIDATA_ClearRoomDataFlag( BSUBWAY_WIFI_DATA *bsw_wifi );
extern BOOL BSUBWAY_WIFIDATA_CheckRoomDataFlag( BSUBWAY_WIFI_DATA *bsw_wifi, u8 rank, u8 room, const RTCDate *day );
extern BOOL BSUBWAY_WIFIDATA_CheckPlayerDataEnable( const BSUBWAY_WIFI_DATA *bsw_wifi );
extern BOOL BSUBWAY_WIFIDATA_CheckLeaderDataEnable( const BSUBWAY_WIFI_DATA *bsw_wifi );
extern void BSUBWAY_WIFIDATA_SetPlayerData( BSUBWAY_WIFI_DATA *bsw_wifi, const BSUBWAY_WIFI_PLAYER* dat, u8 rank, u8 room );
extern void BSUBWAY_WIFIDATA_ClearPlayerData( BSUBWAY_WIFI_DATA *bsw_wifi );
extern u8 BSUBWAY_WIFIDATA_GetPlayerRank( const BSUBWAY_WIFI_DATA *bsw_wifi );
extern u8 BSUBWAY_WIFIDATA_GetPlayerRoomNo( const BSUBWAY_WIFI_DATA *bsw_wifi );
extern void BSUBWAY_WIFIDATA_GetBtlPlayerData( const BSUBWAY_WIFI_DATA *bsw_wifi, BSUBWAY_PARTNER_DATA *player, u8 round );

extern void BSUBWAY_WIFIDATA_SetLeaderData( BSUBWAY_WIFI_DATA *bsw_wifi, const BSUBWAY_LEADER_DATA* dat, u8 rank, u8 room );
extern void BSUBWAY_WIFIDATA_ClearLeaderData( BSUBWAY_WIFI_DATA *bsw_wifi );
extern u8 BSUBWAY_WIFIDATA_GetLeaderRank( const BSUBWAY_WIFI_DATA *bsw_wifi );
extern u8 BSUBWAY_WIFIDATA_GetLeaderRoomNo( const BSUBWAY_WIFI_DATA *bsw_wifi );
extern BSUBWAY_LEADER_DATA* BSUBWAY_WIFIDATA_GetLeaderDataAlloc( const BSUBWAY_WIFI_DATA *bsw_wifi, HEAPID heapID );


#define BSUBWAY_SCOREDATA_GetRenshouCount(a,b) \
  BSUBWAY_SCOREDATA_GetRenshou(a,b)
#define BSUBWAY_SCOREDATA_GetMaxRenshouCount(a,b) \
  BSUBWAY_SCOREDATA_GetRenshouMax(a,b)

#endif //__H_BSUBWAY_SAVEDATA_H__
