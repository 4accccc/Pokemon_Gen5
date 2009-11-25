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

extern void BP_SETUP_Wild( BATTLE_SETUP_PARAM* bp, GAMEDATA* gameData, HEAPID heapID, const BtlRule rule,
  const POKEPARTY* partyEnemy, const BtlBgType bgType, const BtlBgAttr bgAttr, const BtlWeather weather );

extern void BTL_SETUP_Single_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  POKEPARTY* partyEnemy, BtlBgType bgType, BtlBgAttr bgAttr, BtlWeather weather, TrainerID trID, HEAPID heapID );

extern void BTL_SETUP_Single_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode );

extern void BTL_SETUP_Double_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  POKEPARTY* partyEnemy, BtlBgType bgType, BtlBgAttr bgAttr, BtlWeather weather, TrainerID trID, HEAPID heapID );

extern void BTL_SETUP_Double_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode );

extern void BTL_SETUP_Multi_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode, u8 commPos );

extern void BTL_SETUP_Triple_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  POKEPARTY* partyEnemy, BtlBgType bgType, BtlBgAttr bgAttr, BtlWeather weather, TrainerID trID, HEAPID heapID );

extern void BTL_SETUP_Triple_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode );

extern void BTL_SETUP_Rotation_Trainer( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  POKEPARTY* partyEnemy, BtlBgType bgType, BtlBgAttr bgAttr, BtlWeather weather, TrainerID trID, HEAPID heapID );

extern void BTL_SETUP_Rotation_Comm( BATTLE_SETUP_PARAM* dst, GAMEDATA* gameData,
  GFL_NETHANDLE* netHandle, BtlCommMode commMode );

