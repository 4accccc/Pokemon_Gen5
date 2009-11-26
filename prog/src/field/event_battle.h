//======================================================================
/**
 * @file	event_battle.h
 * @brief	イベント：バトル呼び出し
 * @author	tamada GAMEFREAK inc.
 * @date	2009.01.19
 */
//======================================================================
#pragma once
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

//======================================================================
//  define
//======================================================================

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef enum {
//芝生
BATTLE_ATTR_ID_LAWN,
//通常地面
BATTLE_ATTR_ID_NORMAL_GROUND,
//地面１
BATTLE_ATTR_ID_GROUND1,
//地面２
BATTLE_ATTR_ID_GROUND2,
//草
BATTLE_ATTR_ID_GRASS,
//水上
BATTLE_ATTR_ID_WATER,
//雪原
BATTLE_ATTR_ID_SNOW,
//砂地
BATTLE_ATTR_ID_SAND,
//浅い湿原
BATTLE_ATTR_ID_MARSH,
//洞窟
BATTLE_ATTR_ID_CAVE,
//水たまり
BATTLE_ATTR_ID_POOL,
//浅瀬
BATTLE_ATTR_ID_SHOAL,
//氷上
BATTLE_ATTR_ID_ICE,
}BATTLE_ATTR_ID;

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern GMEVENT * EVENT_WildPokeBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM* bp, BOOL sub_event_f );

extern GMEVENT * EVENT_TrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, int tr_id0, int tr_id1, u32 flags );

//--------------------------------------------------------------
/**
 * @brief 「敗北処理」とするべきかどうかの判定
 * @param result  バトルシステムが返す戦闘結果
 * @param competitor  対戦相手の種類
 * @return  BOOL  TRUEのとき、敗北処理をするべき
 */
//--------------------------------------------------------------
extern BOOL FIELD_BATTLE_IsLoseResult(BtlResult result, BtlCompetitor competitor);

//--------------------------------------------------------------
/**
 * @brief 野生戦　再戦コードチェック
 * @param result  バトルシステムが返す戦闘結果
 * @return  SCR_WILD_BTL_RES_XXXX
 */
//--------------------------------------------------------------
extern u8 FIELD_BATTLE_GetWildBattleRevengeCode(BtlResult result);

//--------------------------------------------------------------
/**
 * @brief   バトル画面でのアトリビュート指定を返す
 * @param value   マップアトリビュート
 * @retval  BATTLE_ATTR_ID
 */
//--------------------------------------------------------------
extern BATTLE_ATTR_ID FIELD_BATTLE_GetBattleAttrID( MAPATTR_VALUE value );

