//============================================================================================
/**
 * @file	event_wificlub.c
 * @brief	イベント：WiFiクラブ
 * @author	k.ohno
 * @date	2009.03.31
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"

#include "./event_fieldmap_control.h"
#include "field/event_wificlub.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "battle/battle.h"
#include "poke_tool/monsno_def.h"
#include "system/main.h"			//GFL_HEAPID_APP参照

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
//============================================================================================
//============================================================================================
FS_EXTERN_OVERLAY(wificlub);
FS_EXTERN_OVERLAY(battle);
FS_EXTERN_OVERLAY(ircbattlematch);
extern const GFL_PROC_DATA WifiClubProcData;

//----------------------------------------------------------------
// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];
//----------------------------------------------------------------

#define	HEAPID_CORE GFL_HEAPID_APP

enum _EVENT_IRCBATTLE {
    _FIELD_FADEOUT,
    _CALL_WIFICLUB,
    _WAIT_WIFICLUB,
    _WAIT_NET_END,
    _FIELD_OPEN,
    _FIELD_FADEIN,
    _FIELD_END
};


//============================================================================================
//
//		サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_WiFiClubMain(GMEVENT * event, int *  seq, void * work)
{
	EVENT_WIFICLUB_WORK * dbw = work;
	GAMESYS_WORK * gsys = dbw->gsys;
    
    
	switch (*seq) {
      case _FIELD_FADEOUT:
        dbw->isEndProc = FALSE;
		GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, dbw->fieldmap));
		(*seq)++;
		break;
      case _CALL_WIFICLUB:
        GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(wificlub), &WifiClubProcData, dbw);
		(*seq)++;
		break;
      case _WAIT_WIFICLUB:
        if (!GAMESYSTEM_IsProcExists(gsys)){
            (*seq) ++;
        }
        break;
      case _WAIT_NET_END:
        if(GFL_NET_IsExit()){
            (*seq) ++;
        }
        break;
      case _FIELD_OPEN:
        OS_TPrintf("_FIELD_OPEN\n");
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
		(*seq) ++;
		break;
      case _FIELD_FADEIN:
        OS_TPrintf("_FIELD_FADEIN\n");
		GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, dbw->fieldmap, 0));
		(*seq) ++;
		break;
      case _FIELD_END:
		return GMEVENT_RES_FINISH;
      default:
        GF_ASSERT(0);
        break;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void EVENT_WiFiClub(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap,GMEVENT * event)
{
	GMEVENT * child_event;
	BATTLE_SETUP_PARAM * para;
	EVENT_WIFICLUB_WORK * dbw;

    GMEVENT_Change( event, EVENT_WiFiClubMain, sizeof(EVENT_WIFICLUB_WORK) );
    dbw = GMEVENT_GetEventWork(event);
    dbw->ctrl = SaveControl_GetPointer();
    NET_PRINT("%x\n",(int)dbw->ctrl);
	dbw->gsys = gsys;
	dbw->fieldmap = fieldmap;
	para = &dbw->para;
	{
		para->engine = BTL_ENGINE_ALONE;
		para->rule = BTL_RULE_SINGLE;
		para->competitor = BTL_COMPETITOR_COMM;

        para->commMode = BTL_COMM_DS;
        para->multiMode = 0;

        para->partyPlayer = PokeParty_AllocPartyWork( HEAPID_CORE );	///< プレイヤーのパーティ
        para->partyEnemy1 = NULL;		///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
        para->partyPartner = NULL;	///< 2vs2時の味方AI（不要ならnull）
        para->partyEnemy2 = NULL;		///< 2vs2時の２番目敵AI用（不要ならnull）

		PokeParty_Copy(GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(gsys)), para->partyPlayer);
	}
}


