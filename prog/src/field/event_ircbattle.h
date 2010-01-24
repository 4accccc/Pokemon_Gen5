//============================================================================================
/**
 * @file	event_ircbattle.h
 * @brief	イベント：バトル呼び出し
 * @author	k.ohno
 * @date	2009.01.19
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "demo/shinka_demo.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"
#include "net_app/pokemontrade.h"
#include "net_app/irc_compatible.h"

typedef struct _EVENT_IRCBATTLE_WORK EVENT_IRCBATTLE_WORK;


enum EVENTIRCBTL_IBMODE_ENTRY {
  EVENTIRCBTL_ENTRYMODE_NONE=0,
  EVENTIRCBTL_ENTRYMODE_SINGLE,
  EVENTIRCBTL_ENTRYMODE_DOUBLE,
  EVENTIRCBTL_ENTRYMODE_TRI,
  EVENTIRCBTL_ENTRYMODE_ROTATE,
  EVENTIRCBTL_ENTRYMODE_MULTH,
  EVENTIRCBTL_ENTRYMODE_TRADE,
  EVENTIRCBTL_ENTRYMODE_FRIEND,
	EVENTIRCBTL_ENTRYMODE_COMPATIBLE,
  EVENTIRCBTL_ENTRYMODE_EXIT,
  EVENTIRCBTL_ENTRYMODE_RETRY,
};

struct _EVENT_IRCBATTLE_WORK{
  GAMESYS_WORK * gsys;
  GAMEDATA* gamedata;
  POKEMONTRADE_PARAM aPokeTr;
  POKEPARTY* pParty;
  SAVE_CONTROL_WORK *ctrl;
  int selectType;
  IRC_COMPATIBLE_PARAM  compatible_param; //赤外線メニューに渡す情報
  BOOL push;
#if PM_DEBUG
  int debugseq;
#endif
  BATTLE_SETUP_PARAM* para;
};



//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
//extern GMEVENT* EVENT_IrcBattle(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap);
//extern void EVENT_IrcBattle(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * event);
//extern void EVENT_IrcBattle_SetEnd(EVENT_IRCBATTLE_WORK* pWk);


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
extern void EVENT_IrcBattleSetType(EVENT_IRCBATTLE_WORK* pWork, int type);
extern int EVENT_IrcBattleGetType(EVENT_IRCBATTLE_WORK* pWork);
extern GAMESYS_WORK* IrcBattle_GetGAMESYS_WORK(EVENT_IRCBATTLE_WORK* pWork);
extern SAVE_CONTROL_WORK* IrcBattle_GetSAVE_CONTROL_WORK(EVENT_IRCBATTLE_WORK* pWork);

extern GMEVENT* EVENT_IrcBattle(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * prevevent,BOOL bCreate);
