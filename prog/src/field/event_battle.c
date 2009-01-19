//============================================================================================
/**
 * @file	event_battle.c
 * @brief	イベント：フィールドマップ制御ツール
 * @author	tamada GAMEFREAK inc.
 * @date	2008.01.19
 */
//============================================================================================


#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"

#include "./event_fieldmap_control.h"
#include "./event_battle.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "battle/battle.h"
#include "poke_tool/monsno_def.h"
#include "system/main.h"			//GFL_HEAPID_APP参照

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
//============================================================================================
//============================================================================================


#define	HEAPID_CORE GFL_HEAPID_APP

FS_EXTERN_OVERLAY(battle);

extern const GFL_PROC_DATA DebugSogabeMainProcData;


typedef struct {
	GAMESYS_WORK * gsys;
	FIELD_MAIN_WORK * fieldmap;
	BATTLE_SETUP_PARAM para;
}DEBUG_BATTLE_WORK;

//============================================================================================
//
//		サブイベント
//
//============================================================================================
static GMEVENT_RESULT DebugBattleEvent(GMEVENT * event, int *  seq, void * work)
{
	DEBUG_BATTLE_WORK * dbw = work;
	GAMESYS_WORK * gsys = dbw->gsys;
#if 0
	switch (*seq) {
	case 0:
		GMEVENT_CallEvent(event,
				EVENT_FieldSubProc(dbw->gsys, dbw->fieldmap,
					FS_OVERLAY_ID(battle), &BtlProcData, &(dbw->para))
				);
		(*seq) ++;
		break;
	case 1:
		return GMEVENT_RES_FINISH;
	}
#endif
	switch (*seq) {
	case 0:
		GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, dbw->fieldmap));
		(*seq)++;
		break;
	case 1:
		GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(battle), &BtlProcData, &dbw->para);
		(*seq)++;
		break;
	case 2:
		if (GAMESYSTEM_IsProcExists(gsys)) break;
		(*seq) ++;
		break;
	case 3:
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
		(*seq) ++;
		break;
	case 4:
		GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, dbw->fieldmap, 0));
		(*seq) ++;
		break;
	case 5:
		return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_Battle(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap)
{
	GMEVENT * event;
	BATTLE_SETUP_PARAM * para;
	DEBUG_BATTLE_WORK * dbw;
	event = GMEVENT_Create(gsys, NULL, DebugBattleEvent, sizeof(BATTLE_SETUP_PARAM));
	dbw = GMEVENT_GetEventWork(event);
	dbw->gsys = gsys;
	dbw->fieldmap = fieldmap;
	para = &dbw->para;
	{
		para->engine = BTL_ENGINE_ALONE;
		para->rule = BTL_RULE_SINGLE;
		para->competitor = BTL_COMPETITOR_WILD;

		para->netHandle = NULL;
		para->commMode = BTL_COMM_NONE;
		para->commPos = 0;
		para->netID = 0;

		para->partyPlayer = PokeParty_AllocPartyWork( HEAPID_CORE );	///< プレイヤーのパーティ
		PokeParty_Copy(GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(gsys)), para->partyPlayer);
		para->partyEnemy1 = PokeParty_AllocPartyWork( HEAPID_CORE );	///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
		PokeParty_Add(para->partyEnemy1, PP_Create( MONSNO_AUSU + 1, 15, 3594, HEAPID_CORE));
		para->partyPartner = NULL;	///< 2vs2時の味方AI（不要ならnull）
		para->partyEnemy2 = NULL;	///< 2vs2時の２番目敵AI用（不要ならnull）

	}

	return event;
}


