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

//#include "savedata/bsubway_savedata_def.h" //いらんかも

//======================================================================
//  define
//======================================================================
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
  BSWAY_PLAYDATA_ID_rnd_seed,
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
  BSWAY_SCOREDATA_FLAG_WIFI_MULTI_RECORD,		//プラチナ追加
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
  BSWAY_PLAYMODE_RETRY, //リトライ
  BSWAY_PLAYMODE_WIFI_MULTI, //Wifiマルチ
  BSWAY_PLAYMODE_MAX,
}BSWAY_PLAYMODE;

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

//kari pt_save.h
#define	WAZA_TEMOTI_MAX		(4)		///<1体のポケモンがもてる技の最大値

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

//BSUBWAY_SCOREDATA
extern int BSUBWAY_SCOREDATA_GetWorkSize( void );
extern void BSUBWAY_SCOREDATA_Init( BSUBWAY_SCOREDATA *bsw_score );
extern u16 BSUBWAY_SCOREDATA_SetBattlePoint(
    BSUBWAY_SCOREDATA *bsw_score, u16 num, BSWAY_SETMODE mode );
extern u16 BSUBWAY_SCOREDATA_SetStage(
    BSUBWAY_SCOREDATA *bsw_score, u16 id, BSWAY_SETMODE mode );
extern BOOL BSUBWAY_SCOREDATA_SetFlag( BSUBWAY_SCOREDATA *bsw_score,
    BSWAY_SCOREDATA_FLAG id, BSWAY_SETMODE mode );
extern u8 BSUBWAY_SCOREDATA_SetWifiRank(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_SETMODE mode );
extern u16 BSUBWAY_SCOREDATA_CalcRenshou(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_SETMODE mode );
extern u16 BSUBWAY_SCOREDATA_GetRenshouCount( const BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );
extern u16 BSUBWAY_SCOREDATA_GetMaxRenshouCount( const BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode );

//----
#if 0 //wb null
//----
//--------------------------------------------------------------
///    バトルサブウェイ　WIFIポケモンデータ構造体への不完全型ポインタ
//--------------------------------------------------------------
//typedef struct _BSUBWAY_POKE  BSUBWAY_POKE;

//--------------------------------------------------------------
///    バトルサブウェイ　WIFIプレイヤーデータ構造体への不完全型ポインタ
//--------------------------------------------------------------
//typedef struct _BSUBWAY_WIFI_PLAYER  BSUBWAY_WIFI_PLAYER;

//--------------------------------------------------------------
///    バトルサブウェイ　WIFIリーダーデータ構造体への不完全型ポインタ
//--------------------------------------------------------------
//typedef struct _BSUBWAY_WIFI_LEADER  BSUBWAY_WIFI_LEADER;

//--------------------------------------------------------------
///    バトルサブウェイ　WIFIデータ構造体への不完全型ポインタ
//--------------------------------------------------------------
//typedef struct _BSUBWAY_WIFI_DATA  BSUBWAY_WIFI_DATA;


//--------------------------------------------------------------
///  サブウェイパートナーデータ型
///  　battle/b_tower_data.hを必要な箇所以外では
///   includeしないで済むように定義しておく
//--------------------------------------------------------------
//typedef struct _BSUBWAY_PARTNER_DATA  BSUBWAY_PARTNER_DATA;

//--------------------------------------------------------------
///    バトルサブウェイ　WIFIリーダーデータ構造体への不完全型ポインタ
///   battle/b_tower_data.hを必要な箇所以外では
///   includeしないで済むように定義しておく
///  中身は見えませんがポインタ経由で参照できます
//--------------------------------------------------------------
//typedef struct _BSUBWAY_LEADER_DATA  BSUBWAY_LEADER_DATA;

//--------------------------------------------------------------
///     バトルサブウェイ WIFIライブラリ構造体への不完全ポインタ
///   Wifiライブラリヘッダを外部に非公開にしておくために定義
//--------------------------------------------------------------
#if 0
typedef struct Dpw_Bt_PokemonData  DPW_BT_POKEMON_DATA;
typedef struct Dpw_Bt_Player  DPW_BT_PLAYER;
typedef struct Dpw_Bt_Leader  DPW_BT_LEADER;
typedef struct Dpw_Bt_Room    DPW_BT_ROOM;
#endif

//--------------------------------------------------------------
///   @biref  バトルサブウェイ WIFIルームデータID(rankとroomno)
/// 
///   ランクとルームナンバーを取得したいときようの公開構造体データ型
//--------------------------------------------------------------
#if 0
typedef struct
{
  u8  rank;  //ルームランク(1-10)
  u8  no;    //ルームNo(1-200)
}BTOWER_ROOMID;
#endif


//--------------------------------------------------------------
///     サブウェイ　ポケモンデータサイズ
//--------------------------------------------------------------
extern int BSUBWAY_WifiPoke_GetWorkSize(void);

//--------------------------------------------------------------
///     サブウェイ　Wifiデータサイズ
//--------------------------------------------------------------
extern int  BSUBWAY_WifiData_GetWorkSize(void);
//--------------------------------------------------------------
///   　サブウェイ Wifi/トレーナーロードプレイヤーデータ構造体データサイズ
//--------------------------------------------------------------
extern int BSUBWAY_DpwBtPlayer_GetWorkSize(void);

//--------------------------------------------------------------
///     サブウェイ　プレイデータクリア
//--------------------------------------------------------------
extern void BSUBWAY_PlayData_Clear(BSUBWAY_PLAYWORK* dat);

//--------------------------------------------------------------
///     サブウェイ  スコアデータクリア
//--------------------------------------------------------------
extern void BSUBWAY_ScoreData_Clear(BSUBWAY_SCOREWORK* dat);

//--------------------------------------------------------------
///     サブウェイ　プレイヤーメッセージデータクリア
//--------------------------------------------------------------
extern void BSUBWAY_PlayerMsg_Clear(BSUBWAY_PLAYER_MSG* dat);

//--------------------------------------------------------------
///     サブウェイ　Wifiデータクリア
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_Clear(BSUBWAY_WIFI_DATA* dat);

//======================================================================
//  extern ブロックデータアクセス系
//======================================================================
#if 0 //wb
//--------------------------------------------------------------
///     サブウェイ　プレイデータへのポインタを取得
//--------------------------------------------------------------
extern BSUBWAY_PLAYWORK* SaveData_GetTowerPlayData(SAVEDATA* sv);

//--------------------------------------------------------------
///     サブウェイ　スコアデータへのポインタを取得
//--------------------------------------------------------------
extern BSUBWAY_SCOREWORK* SaveData_GetTowerScoreData(SAVEDATA* sv);

//--------------------------------------------------------------
///     サブウェイ Wifiデータへのポインタを取得  
//--------------------------------------------------------------
extern BSUBWAY_WIFI_DATA*  SaveData_GetTowerWifiData(SAVEDATA* sv);
#endif

//--------------------------------------------------------------
///     プレイデータ　取得
///   @param  dat  BTLTOWRE_PLAYWORK*
///   @param  id  取得するデータID BSUBWAY_PSD_ID型
///   @param  buf  void*:データ取得ポインタ
//--------------------------------------------------------------
extern u32 BSUBWAY_PlayData_Get(
    BSUBWAY_PLAYWORK* dat,BSWAY_PSD_ID id,void* buf);

//--------------------------------------------------------------
///     プレイデータ　セット
///   @param  dat  BTLTOWRE_PLAYWORK*
///   @param  id  取得するデータID BTOWER_PSD_ID型
///   @param  buf  void*:データを格納したバッファへのポインタ
//--------------------------------------------------------------
extern void BSUBWAY_PlayData_Put(
    BSUBWAY_PLAYWORK* dat,BSWAY_PSD_ID id,const void* buf);

//--------------------------------------------------------------
///    プレイデータ　WifiレコードデータAdd  
///   @param  down  倒されたポケモン追加数
///   @param  turn  かかったターン追加数
///   @param  damage  受けたダメージ追加値
//--------------------------------------------------------------
extern void BSUBWAY_PlayData_WifiRecordAdd(
    BSUBWAY_PLAYWORK* dat,u8 down,u16 turn,u16 damage);

//--------------------------------------------------------------
///     プレイデータ　正しくセーブ済みかどうか？
///   @retval  TRUE  正しくセーブされている
///   @retval FALSE  セーブされていない
//--------------------------------------------------------------
extern BOOL BSUBWAY_PlayData_GetSaveFlag(BSUBWAY_PLAYWORK* dat);

//--------------------------------------------------------------
///     プレイデータ　セーブ状態フラグをセット
//--------------------------------------------------------------
extern void BSUBWAY_PlayData_SetSaveFlag(BSUBWAY_PLAYWORK* dat,BOOL flag);

//======================================================================
//  extern スコアデータ取得系
//======================================================================
//--------------------------------------------------------------
///     スコアデータ バトルポイントセット
///   有効コマンド set/get/add/sub
//--------------------------------------------------------------
extern u16 BSUBWAY_ScoreData_SetBattlePoint(
    BSUBWAY_SCOREWORK* dat,u16 num,BSWAY_DATA_SETID mode);

//--------------------------------------------------------------
///     SAVEDATAからバトルポイントをGet
//--------------------------------------------------------------
#if 0 //wb
static inline u16 BattlePoint_Get(SAVEDATA* sv)
{
  return BSUBWAY_ScoreData_SetBattlePoint(
      SaveData_GetTowerScoreData(sv),0,BSWAY_DATA_get);
}
#endif

//--------------------------------------------------------------
///     SAVEDATAからバトルポイントをSet
//--------------------------------------------------------------
#if 0 //wb
static inline u16 BattlePoint_Set(SAVEDATA* sv,u16 num)
{
  return BSUBWAY_ScoreData_SetBattlePoint(
      SaveData_GetTowerScoreData(sv),num,BSWAY_DATA_set);
}
#endif

//--------------------------------------------------------------
///     SAVEDATAからバトルポイントをAdd
//--------------------------------------------------------------
#if 0 //wb
static inline u16  BattlePoint_Add(SAVEDATA* sv,u16 num)
{
  return BSUBWAY_ScoreData_SetBattlePoint(
      SaveData_GetTowerScoreData(sv),num,BSWAY_DATA_add);
}
#endif

//--------------------------------------------------------------
///     SAVEDATAからバトルポイントをSub
//--------------------------------------------------------------
#if 0 //wb
static inline u16  BattlePoint_Sub(SAVEDATA* sv,u16 num)
{
  return BSUBWAY_ScoreData_SetBattlePoint(
      SaveData_GetTowerScoreData(sv),num,BSWAY_DATA_sub);
}
#endif

//--------------------------------------------------------------
///     スコアデータ　Wifi連続敗戦カウント操作
///   有効コマンド get/reset/inc
///   @return  操作後のカウント
//--------------------------------------------------------------
extern u8 BSUBWAY_ScoreData_SetWifiLoseCount(
    BSUBWAY_SCOREWORK* dat,BSWAY_DATA_SETID mode);

//--------------------------------------------------------------
///     スコアデータ　Wifiランク操作
/// 
///   有効コマンド get/reset/inc/dec
//--------------------------------------------------------------
extern u8  BSUBWAY_ScoreData_SetWifiRank(
    BSUBWAY_SCOREWORK* dat,BSWAY_DATA_SETID mode);

//--------------------------------------------------------------
///     使用ポケモンデータ保存
//--------------------------------------------------------------
extern void BSUBWAY_ScoreData_SetUsePokeData( BSUBWAY_SCOREWORK* dat,
    BSWAY_SCORE_POKE_DATA mode,BSUBWAY_POKEMON* poke);

//--------------------------------------------------------------
///     使用ポケモンデータをバッファにコピーして取得(WiFi構造体データ型)
//--------------------------------------------------------------
extern void BSUBWAY_ScoreData_GetUsePokeDataDpw(BSUBWAY_SCOREWORK* dat,
      BSWAY_SCORE_POKE_DATA mode,DPW_BT_POKEMON_DATA* poke);

//--------------------------------------------------------------
/// 使用ポケモンデータをバッファにコピーして取得
//--------------------------------------------------------------
extern void BSUBWAY_ScoreData_GetUsePokeData(BSUBWAY_SCOREWORK* dat,
      BSWAY_SCORE_POKE_DATA mode,BSUBWAY_POKEMON* poke);

//--------------------------------------------------------------
///     スコアデータ　周回数操作
//--------------------------------------------------------------
extern u16 BSUBWAY_ScoreData_SetStage(BSUBWAY_SCOREWORK* dat,u16 id,BSWAY_DATA_SETID mode);

//--------------------------------------------------------------
///     現在のステージ値を直接セットする
//--------------------------------------------------------------
extern u16 BSUBWAY_ScoreData_SetStageValue(BSUBWAY_SCOREWORK* dat,u16 id,u16 value);

#ifdef PM_DEBUG
//--------------------------------------------------------------
///     デバッグ限定　現在のステージ値を直接セットする
//--------------------------------------------------------------
extern u16 BSUBWAY_ScoreData_DebugSetStageValue(BSUBWAY_SCOREWORK* dat,u16 id,u16 value);
#endif  //PM_DEBUG

//--------------------------------------------------------------
///     スコアデータ Wifi成績操作
//--------------------------------------------------------------
extern u16  BSUBWAY_ScoreData_SetWifiScore(BSUBWAY_SCOREWORK* dat,BSUBWAY_PLAYWORK *playdata);

//--------------------------------------------------------------
///     スコアデータ　Wifi成績0クリア
//--------------------------------------------------------------
extern void BSUBWAY_ScoreData_ClearWifiScore(BSUBWAY_SCOREWORK* dat);

//--------------------------------------------------------------
///     スコアデータ　Wifi成績取得
//--------------------------------------------------------------
extern u16  BSUBWAY_ScoreData_GetWifiScore(BSUBWAY_SCOREWORK* dat);

//--------------------------------------------------------------
///     スコアデータから勝ち抜いた数を取得
//--------------------------------------------------------------
extern u8  BSUBWAY_ScoreData_GetWifiWinNum(BSUBWAY_SCOREWORK* dat);

//--------------------------------------------------------------
///     スコアデータ フラグエリアセット
/// 
///   有効コマンド：reset/set/get
//--------------------------------------------------------------
extern BOOL BSUBWAY_ScoreData_SetFlags(BSUBWAY_SCOREWORK* dat,u16 id,BSWAY_DATA_SETID mode);

//--------------------------------------------------------------
///     サブウェイ用日付変化ランダムシード保存
//--------------------------------------------------------------
extern void BSUBWAY_ScoreData_SetDayRndSeed(BSUBWAY_SCOREWORK* dat,u32 rnd_seed);

//--------------------------------------------------------------
///     サブウェイ用日付変化ランダムシード取得
//--------------------------------------------------------------
extern u32 BSUBWAY_ScoreData_GetDayRndSeed(BSUBWAY_SCOREWORK* dat);

//======================================================================
//　extern サブウェイ　プレイヤーメッセージデータアクセス系
//======================================================================
#if 0 //wb
//--------------------------------------------------------------
///     簡易会話データをセット
//--------------------------------------------------------------
extern void BSUBWAY_PlayerMsg_Set(
    SAVEDATA* sv,BSWAY_PLAYER_MSG_ID id,PMS_DATA* src);

//--------------------------------------------------------------
///     簡易会話データを取得
//--------------------------------------------------------------
extern PMS_DATA* BSUBWAY_PlayerMsg_Get(
    SAVEDATA* sv,BSWAY_PLAYER_MSG_ID id);
#endif

//======================================================================
//  extern　サブウェイ　Wifiデータアクセス系
//======================================================================
#if 0 //wb
//--------------------------------------------------------------
///     指定したルームデータの取得フラグを立てる
/// 
///   @param  rank  1オリジンなので注意
///   @param  roomno  1オリジンなので注意
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_SetRoomDataFlag(BSUBWAY_WIFI_DATA* dat,
    u8 rank,u8 roomno,RTCDate *day);

//--------------------------------------------------------------
///     ルームデータ取得フラグをクリアする
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_ClearRoomDataFlag(BSUBWAY_WIFI_DATA* dat);

//--------------------------------------------------------------
///     指定したルームのDLフラグが立っているかチェック
///   @param  rank  1オリジンなので注意
///   @param  roomno  1オリジンなので注意
//--------------------------------------------------------------
extern BOOL BSUBWAY_WifiData_CheckRoomDataFlag(BSUBWAY_WIFI_DATA* dat,u8 rank,u8 roomno,RTCDate* day);

//--------------------------------------------------------------
///     プレイヤーデータが存在するかチェック
//--------------------------------------------------------------
extern BOOL BSUBWAY_WifiData_IsPlayerDataEnable(BSUBWAY_WIFI_DATA* dat);

//--------------------------------------------------------------
///     リーダーデータがあるかどうかチェック
//--------------------------------------------------------------
extern BOOL BSUBWAY_WifiData_IsLeaderDataEnable(BSUBWAY_WIFI_DATA* dat);

//--------------------------------------------------------------
///     プレイヤーデータをセーブ
///   
///   @param  rank  1オリジンなので注意
///   @param  roomno  1オリジンなので注意
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_SetPlayerData(BSUBWAY_WIFI_DATA* dat,DPW_BT_PLAYER* src,u8 rank,u8 roomno);

//--------------------------------------------------------------
///     プレイヤーデータをクリア
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_ClearPlayerData(BSUBWAY_WIFI_DATA* dat);

//--------------------------------------------------------------
///     プレイヤーデータのRoomID(ランクとroomNo)を取得
/// 
///   @param  roomid  BTOWER_ROOMID型(b_tower.hで公開)
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_GetPlayerDataRoomID(BSUBWAY_WIFI_DATA* dat,BTOWER_ROOMID *roomid);

//--------------------------------------------------------------
///     プレイヤーデータをサブウェイ戦闘用に解凍
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_GetBtlPlayerData(BSUBWAY_WIFI_DATA* dat,
    BSUBWAY_PARTNER_DATA* player,const u8 round);

//--------------------------------------------------------------
///     リーダーデータをセーブ
///   
///   @param  rank  1オリジンなので注意
///   @param  roomno  1オリジンなので注意
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_SetLeaderData(
    BSUBWAY_WIFI_DATA* dat,DPW_BT_LEADER* src,u8 rank,u8 roomno);

//--------------------------------------------------------------
///     リーダーデータをクリア
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_ClearLeaderData(BSUBWAY_WIFI_DATA* dat);

//--------------------------------------------------------------
///     リーダーデータのRoomID(ランクとroomNo)を取得
/// 
///   @param  roomid  BTOWER_ROOMID型(b_tower.hで公開)
//--------------------------------------------------------------
extern void BSUBWAY_WifiData_GetLeaderDataRoomID(
    BSUBWAY_WIFI_DATA* dat,BTOWER_ROOMID *roomid);

//--------------------------------------------------------------
///     リーダーデータをAllocしたメモリにコピーして取得する
/// 
///   ＊内部でAllocしたメモリ領域を返すので、
///   呼び出し側が明示的に解放すること！
//--------------------------------------------------------------
extern BSUBWAY_LEADER_DATA* BSUBWAY_WifiData_GetLeaderDataAlloc(
    BSUBWAY_WIFI_DATA* dat,int heapID) ;
#endif
//----
#endif //wb null
//----
#endif //__H_BSUBWAY_SAVEDATA_H__
