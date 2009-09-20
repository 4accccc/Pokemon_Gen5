//==============================================================================
/**
 * @file    event_colosseum_battle.h
 * @brief   コロシアムバトルイベント
 * @author  matsuda
 * @date    2009.07.22(水)
 */
//==============================================================================
#pragma once


//==============================================================================
//  構造体定義
//==============================================================================
///コロシアム戦闘用パラメータ
typedef struct{
  POKEPARTY*      partyPlayer;  ///< プレイヤーのパーティ
  POKEPARTY*      partyPartner; ///< 2vs2時の味方AI（不要ならnull）
  POKEPARTY*      partyEnemy1;  ///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
  POKEPARTY*      partyEnemy2;  ///< 2vs2時の２番目敵AI用（不要ならnull）
}COLOSSEUM_BATTLE_SETUP;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern GMEVENT* EVENT_ColosseumBattle(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, UNION_PLAY_CATEGORY play_category, const COLOSSEUM_BATTLE_SETUP *setup);

