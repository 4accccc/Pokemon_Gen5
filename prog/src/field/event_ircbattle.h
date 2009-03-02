//============================================================================================
/**
 * @file	event_ircbattle.h
 * @brief	イベント：バトル呼び出し
 * @author	tamada GAMEFREAK inc.
 * @date	2009.01.19
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"

typedef struct _EVENT_IRCBATTLE_WORK EVENT_IRCBATTLE_WORK;

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
//extern GMEVENT* EVENT_IrcBattle(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap);
extern void EVENT_IrcBattle(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap,GMEVENT * event);
extern void EVENT_IrcBattle_SetEnd(EVENT_IRCBATTLE_WORK* pWk);


//--------------------------------------------------------------
/**
 * @brief   ビーコンデータ取得関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------

extern void* IrcBattleBeaconGetFunc(void* pWork);
extern int IrcBattleBeaconGetSizeFunc(void* pWork);
extern BOOL IrcBattleBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
