////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケセン回復アニメーションイベント
 * @file   event_pc_recovery.c
 * @author obata_toshihiro
 * @date   2009.09.18
 */
//////////////////////////////////////////////////////////////////////////////// 
#include "include/gamesystem/game_event.h"
#include "event_pc_recovery.h"


//==============================================================================
// イベントワーク
//==============================================================================
typedef struct
{
  int temp;
} RECOVERY_WORK;


//==============================================================================
// プロトタイプ宣言
//==============================================================================
GMEVENT_RESULT EVENT_FUNC_PcRecoveryAnime( GMEVENT* event, int* seq, void* work );


//------------------------------------------------------------------------------
/**
 * @brief ポケセン回復アニメーションイベントを作成する
 *
 * @param gsys   ゲームシステム
 * @param parent 親イベント
 *
 * @return 作成した回復アニメイベント
 */
//------------------------------------------------------------------------------
GMEVENT* EVENT_PcRecoveryAnime( GAMESYS_WORK* gsys, GMEVENT* parent )
{
  GMEVENT* event;
  RECOVERY_WORK* work;

  // イベント作成
  event = GMEVENT_Create( gsys, parent, EVENT_FUNC_PcRecoveryAnime, sizeof(RECOVERY_WORK) );

  // イベントワーク初期化
  work = (RECOVERY_WORK*)GMEVENT_GetEventWork( event );

  return event;
}


//==============================================================================
/**
 * @brief イベント制御関数
 *
 * @param event イベント
 * @param seq   シーケンス
 * @param work  イベントワーク
 *
 * @return GMEVENT_RES_CONTINUE or GMEVENT_RES_FINISH
 */
//==============================================================================
GMEVENT_RESULT EVENT_FUNC_PcRecoveryAnime( GMEVENT* event, int* seq, void* work )
{
  return GMEVENT_RES_CONTINUE;
}
