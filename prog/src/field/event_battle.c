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

//============================================================================================
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "battle/battle.h"
#include "poke_tool/monsno_def.h"
#include "system/main.h"

#define	HEAPID_CORE GFL_HEAPID_APP

FS_EXTERN_OVERLAY(battle);


extern const GFL_PROC_DATA DebugSogabeMainProcData;
//============================================================================================
//
//		サブイベント
//
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * DEBUG_EVENT_Battle(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap)
{
	GMEVENT * event;
	event = EVENT_FieldSubProc(gsys, fieldmap,NO_OVERLAY_ID, &DebugSogabeMainProcData, NULL);
	return event;

#if 0
	BATTLE_SETUP_PARAM* para = getGenericWork( wk, sizeof(BATTLE_SETUP_PARAM) );
	{
			para->engine = BTL_ENGINE_ALONE;
			para->rule = BTL_RULE_SINGLE;
			para->competitor = BTL_COMPETITOR_WILD;

			para->netHandle = NULL;
			para->commMode = BTL_COMM_NONE;
			para->commPos = 0;
			para->netID = 0;


			para->partyPlayer = PokeParty_AllocPartyWork( HEAPID_CORE );	///< プレイヤーのパーティ
			para->partyEnemy1 = PokeParty_AllocPartyWork( HEAPID_CORE );	///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
			para->partyPartner = NULL;	///< 2vs2時の味方AI（不要ならnull）
			para->partyEnemy2 = NULL;	///< 2vs2時の２番目敵AI用（不要ならnull）

		#ifdef DEBUG_ONLY_FOR_taya
			setup_party( HEAPID_CORE, para->partyPlayer, MONSNO_GYARADOSU, MONSNO_PIKATYUU, MONSNO_RIZAADON, 0 );
			setup_party( HEAPID_CORE, para->partyEnemy1, MONSNO_YADOKINGU, MONSNO_METAGUROSU, MONSNO_SUTAAMII, 0 );
		#else
			setup_party( HEAPID_CORE, para->partyPlayer, MONSNO_AUSU + 2, MONSNO_AUSU + 1, 0 );
			setup_party( HEAPID_CORE, para->partyEnemy1, MONSNO_AUSU + 1, MONSNO_AUSU + 2, 0 );
		#endif

			GFL_PROC_SysCallProc( FS_OVERLAY_ID(battle), &BtlProcData, para );
			(*seq)++;
		}
#endif
}


