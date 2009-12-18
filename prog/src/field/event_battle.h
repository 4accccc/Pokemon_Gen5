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
#include "battle/battle_bg_def.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern GMEVENT * EVENT_WildPokeBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM* bp, BOOL sub_event_f );

extern GMEVENT * EVENT_TrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, int partner_id, int tr_id0, int tr_id1, u32 flags );

extern GMEVENT * EVENT_BSubwayTrainerBattle(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldmap, BATTLE_SETUP_PARAM *bp );

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
 * @retval  BtlBgAttr
 */
//--------------------------------------------------------------
extern BtlBgAttr FIELD_BATTLE_GetBattleAttrID( MAPATTR_VALUE value );

