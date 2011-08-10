//=============================================================================================
/**
 * @file  btl_setup.h
 * @brief ポケモンWB バトルパラメータ簡易セットアップ処理
 * @author  taya
 *
 * @date  2009.10.01 作成
 */
//=============================================================================================
#pragma once

#include "battle/battle.h"
#include "gamesystem/game_data.h"
#include "savedata/battle_rec.h"
#include "savedata/regulation.h"

/*
 *  @brief  戦闘パラメータワーク生成
 *  @param  heapID  ワークメモリを確保するheapID
 *
 *  ＊BATTLE_SETUP_PARAM構造体領域をアロケートし、初期化します
 *  ＊デフォルトのパラメータで構造体を初期化します。バトルタイプに応じて、必要な初期化を追加で行ってください
 *  ＊必ず BATTLE_PARAM_Delete()で解放してください
 */
extern BATTLE_SETUP_PARAM* BATTLE_PARAM_Create( HEAPID heapID );

/*
 *  @brief  戦闘パラメータワーク解放
 *  @param  bp  BATTLE_PARAM_Create()で生成されたBATTLE_SETUP_PARAM構造体型ワークへのポインタ
 */
extern void BATTLE_PARAM_Delete( BATTLE_SETUP_PARAM* bp );

/**
 *  @brief  戦闘パラメータワークの内部初期化処理
 *  @param  bp  確保済みのBATTLE_SETUP_PARAM構造体型ワークへのポインタ
 *
 *  ＊デフォルトのパラメータで構造体を初期化します。バトルタイプに応じて、必要な初期化を追加で行ってください
 *  ＊使い終わったら必ずBATTLE_PARAM_Release()関数で解放処理をしてください
 */
extern void BATTLE_PARAM_Init( BATTLE_SETUP_PARAM* bp );

/*
 *  @brief  戦闘パラメータワークの内部アロケーションメモリ解放とクリア
 *  @param  bp  確保済みのBATTLE_SETUP_PARAM構造体型ワークへのポインタ
 */
extern void BATTLE_PARAM_Release( BATTLE_SETUP_PARAM* bp );

/*
 *  @brief  バトルパラム　PokePartyデータセット
 *
 *  渡されたポインタの内容をコピーします
 */
extern void BATTLE_PARAM_SetPokeParty( BATTLE_SETUP_PARAM* bp, const POKEPARTY* party, BTL_CLIENT_ID client );

/*
 *  @brief  バトルパラム　PokePartyポインタ取得
 */
extern POKEPARTY* BATTLE_PARAM_GetPokePartyPointer( BATTLE_SETUP_PARAM* bp, BTL_CLIENT_ID client );

/*
 *  @brief  バトルパラム　バトルステータスフラグセット
 */
extern void BATTLE_PARAM_SetBtlStatusFlag( BATTLE_SETUP_PARAM* bp, BTL_STATUS_FLAG status_f);

/*
 *  @brief  バトルパラム バトルステータスフラグチェック
 */
extern BOOL BATTLE_PARAM_CheckBtlStatusFlag( BATTLE_SETUP_PARAM* bp, BTL_STATUS_FLAG status_f );

/*
 *  @brief  レギュレーションの内容をBATTLE_PARAMへ適応する
 *  @param  bp  確保済みかつ、設定済みのBATTLE_SETUP_PARAM構造体型ワークへのポインタ
 *  @param  reg レギュレーションのポインタ
 *  @param  HEAPID 一時バッファのためのヒープID
 */
extern void BATTLE_PARAM_SetRegulation( BATTLE_SETUP_PARAM* bp, const REGULATION *reg, HEAPID heapID );

/**
 *  @brief  トレーナー肩書の文字列をBATTLE_PARAMにセットする
 *  @param  bp        確保済みのBATTLE_SETUP_PARAM構造体型ワークへのポインタ
 *  @param  clientID  セット位置のバトルクライアントID
 *  @param  strbuf    セットする文字列
 *
 *  セットする文字列はBATTLE_SETUP_PARAMの外で確保・開放すること。
 *  内部ではポインタとして参照するが、一切操作はしない。
 */
static inline void BATTLE_PARAM_SetTrainerTypeName(
    BATTLE_SETUP_PARAM* bp, u8 clientID, const STRBUF * strbuf )
{
  GF_ASSERT( clientID < BTL_CLIENT_NUM );
  GF_ASSERT( bp->tr_type_name[ clientID ] == NULL );
  bp->tr_type_name[ clientID ] = strbuf;
}

/**
 *  @brief  WBT用バトル内メッセージをセットする
 *  @param[out] bp        確保済みのBATTLE_SETUP_PARAM構造体型ワークへのポインタ
 *  @param[in]  win_msg   設定するプレイヤー勝利時メッセージ
 *  @param[in]  lose_msg  設定するプレイヤー敗北時メッセージ
 */
static inline void BATTLE_PARAM_SetWBTMsg(
    BATTLE_SETUP_PARAM* bp, const STRBUF * win_msg, const STRBUF * lose_msg )
{
  bp->wbt_win_msg = win_msg;
  bp->wbt_lose_msg = lose_msg;
}

////////////////////////////////////////////////////////////////////////
//
/*
 *  @brief  戦闘フィールドシチュエーションデータデフォルト初期化
 */
extern void BTL_FIELD_SITUATION_Init( BTL_FIELD_SITUATION* sit );


/*
 *  @brief  セットアップ済みパラメータをバトルサブウェイモード用に切り替え
 */
extern void BTL_SETUP_SetSubwayMode( BATTLE_SETUP_PARAM* dst );


/*
 *  @brief  セットアップ済みパラメータに録画バッファを生成
 */
extern void BTL_SETUP_AllocRecBuffer( BATTLE_SETUP_PARAM* dst, HEAPID heapID );


/*
 *  @brief  セットアップ済みパラメータを録画再生モードに切り替え
 */
extern void BTL_SETUP_SetRecordPlayMode( BATTLE_SETUP_PARAM* dst );

////////////////////////////////////////////////////////////////////////
/*
 * BATTLE_SETUP_PARAM初期化ユーティリティ関数群
 *
 *  BTL_SETUP系関数は、内部でメモリアロケーションを行います
 *  バトルが終了したら、必ずBATTLE_PARAM_Release()orBATTLE_PARAM_Delete()関数で解放を行ってください
 */
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////
//ローカルバトル系
//////////////////////////////////////////////
extern void BTL_SETUP_Wild( BATTLE_SETUP_PARAM* bp, GAMEDATA* gdata,
  const POKEPARTY* partyEnemy, const BTL_FIELD_SITUATION* sit, const BtlRule rule, HEAPID heapID );

extern void BTL_SETUP_WildMulti( BATTLE_SETUP_PARAM* bp, GAMEDATA* gameData,
  const POKEPARTY* partyEnemy, TrainerID trID, const BTL_FIELD_SITUATION* sit, HEAPID heapID );

extern void BTL_SETUP_Single_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  BTL_FIELD_SITUATION* sit, TrainerID trID, HEAPID heapID );

extern void BTL_SETUP_Double_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  BTL_FIELD_SITUATION* sit, TrainerID trID, HEAPID heapID );

extern void BTL_SETUP_Triple_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  BTL_FIELD_SITUATION* sit, TrainerID trID, HEAPID heapID );

extern void BTL_SETUP_Rotation_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  BTL_FIELD_SITUATION* sit, TrainerID trID, HEAPID heapID );

extern void BTL_SETUP_Tag_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  BTL_FIELD_SITUATION* sit, TrainerID tr_id0, TrainerID tr_id1, HEAPID heapID );

extern void BTL_SETUP_AIMulti_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  BTL_FIELD_SITUATION* sit, TrainerID partner, TrainerID tr_id0, TrainerID tr_id1, HEAPID heapID );

extern void BTL_SETUP_CaptureDemo( BATTLE_SETUP_PARAM* bp, GAMEDATA* gameData,
  const POKEPARTY* partyMine, const POKEPARTY* partyEnemy, const BTL_FIELD_SITUATION* sit, HEAPID heapID );

//////////////////////////////////////////////
//通信バトル系
//////////////////////////////////////////////
extern void BTL_SETUP_Single_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode, HEAPID heapID );

extern void BTL_SETUP_Double_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode, HEAPID heapID );

extern void BTL_SETUP_Multi_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode, u8 commPos, HEAPID heapID );

extern void BTL_SETUP_AIMulti_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode, u8 commPos, TrainerID tr_id1, TrainerID tr_id2, HEAPID heapID );

extern void BTL_SETUP_Triple_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode, HEAPID heapID );

extern void BTL_SETUP_Rotation_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode, HEAPID heapID );


//////////////////////////////////////////////
// 録画データ復元
//////////////////////////////////////////////
extern void BTL_SETUP_InitForRecordPlay( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData, HEAPID heapID );
extern void BTL_SETUP_QuitForRecordPlay( BATTLE_SETUP_PARAM* bsp );
extern void BTL_SETUP_SetDifficulty( BATTLE_SETUP_PARAM* bsp, GAMEDATA* gameData, u16 zoneID );

extern BSP_TRAINER_DATA* BSP_TRAINER_DATA_Create( HEAPID heapID );

//-----------------------------------------------------------------------------
/*
 * 以下デバッグ用便利機能を実現するためのセットアップ
 */
//-----------------------------------------------------------------------------
#ifdef PM_DEBUG

extern void BTL_SETUP_SetDebugFlag( BATTLE_SETUP_PARAM* dst, BtlDebugFlag flag );
extern BOOL BTL_SETUP_GetDebugFlag( const BATTLE_SETUP_PARAM* param, BtlDebugFlag flag );


#endif
