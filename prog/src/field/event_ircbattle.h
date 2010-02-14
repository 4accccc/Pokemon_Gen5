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

#include "net_app/irc_match.h"

#include "demo/shinka_demo.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap.h"
#include "net_app/pokemontrade.h"
#include "net_app/irc_compatible.h"
#include "include/demo/comm_btl_demo.h"

typedef struct _EVENT_IRCBATTLE_WORK EVENT_IRCBATTLE_WORK;


struct _EVENT_IRCBATTLE_WORK{
  GAMESYS_WORK * gsys;
  GAMEDATA* gamedata;
  POKEMONTRADE_PARAM aPokeTr;
  POKEPARTY* pParty;
  POKEPARTY* pNetParty[4];
  SAVE_CONTROL_WORK *ctrl;
  int selectType;
  IRC_COMPATIBLE_PARAM  compatible_param; //赤外線メニューに渡す情報
  IRC_MATCH_WORK irc_match;
  BOOL push;
#if PM_DEBUG
  int debugseq;
#endif
  BATTLE_SETUP_PARAM* para;
  COMM_BTL_DEMO_PARAM demo_prm;
  //COMM_BATTLE_CALL_PROC_PARAM callprm;
};



//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
//extern GMEVENT* EVENT_IrcBattle(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap);
//extern void EVENT_IrcBattle(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * event);
//extern void EVENT_IrcBattle_SetEnd(EVENT_IRCBATTLE_WORK* pWk);



//extern void* IrcBattleBeaconGetFunc(void* pWork);
//extern int IrcBattleBeaconGetSizeFunc(void* pWork);
//extern BOOL IrcBattleBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
extern void EVENT_IrcBattleSetType(EVENT_IRCBATTLE_WORK* pWork, int type);
extern int EVENT_IrcBattleGetType(EVENT_IRCBATTLE_WORK* pWork);
extern GAMESYS_WORK* IrcBattle_GetGAMESYS_WORK(EVENT_IRCBATTLE_WORK* pWork);
extern SAVE_CONTROL_WORK* IrcBattle_GetSAVE_CONTROL_WORK(EVENT_IRCBATTLE_WORK* pWork);

extern GMEVENT* EVENT_IrcBattle(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * prevevent,BOOL bCreate);
