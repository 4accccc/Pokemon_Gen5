//============================================================================================
/**
 * @file	eventdata_system.c
 * @brief	イベント起動用データのロード・保持システム
 * @author	tamada GAME FREAK inc.
 * @date	2008.11.20
 *
 */
//============================================================================================

#include <gflib.h>
#include "gamesystem/gamesystem.h"

#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
	EVDATA_SIZE = 0x800,
	SPSCR_DATA_SIZE = 0x100,
};

//------------------------------------------------------------------
/**
 * @brief	イベント起動データシステムの定義
 */
//------------------------------------------------------------------
struct _EVDATA_SYS {
	HEAPID heapID;

	u16 bg_count;
	//u16 npc_count;
	u16 connect_count;
	u16 pos_count;
	const BG_TALK_DATA * bg_data;
	//const FIELDOBJ * npc_data;
	const CONNECT_DATA * connect_data;
	const POS_EVENT_DATA * pos_data;

	//ENCOUNT_DATA encount_work;
	u32 load_buffer[EVDATA_SIZE / sizeof(u32)];
	u32 spscr_buffer[SPSCR_DATA_SIZE / sizeof(u32)];
};


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
EVENTDATA_SYSTEM * EVENTDATA_SYS_Create(HEAPID heapID)
{
	EVENTDATA_SYSTEM * evdata = GFL_HEAP_AllocClearMemory(heapID, sizeof(EVENTDATA_SYSTEM));
	return evdata;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void EVENTDATA_SYS_Delete(EVENTDATA_SYSTEM * evdata)
{
	GFL_HEAP_FreeMemory(evdata);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void EVENTDATA_SYS_Clear(EVENTDATA_SYSTEM * evdata)
{
	evdata->bg_count = 0;
	//evdata->npc_count = 0;
	evdata->connect_count = 0;
	evdata->pos_count = 0;
	evdata->bg_data = NULL;
	//evdata->npc_data = NULL;
	evdata->connect_data = NULL;
	evdata->pos_data = NULL;
	GFL_STD_MemClear(evdata->load_buffer, EVDATA_SIZE);
	GFL_STD_MemClear(evdata->spscr_buffer, SPSCR_DATA_SIZE);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void EVENTDATA_SYS_Load(EVENTDATA_SYSTEM * evdata, u16 zone_id)
{
	EVENTDATA_SYS_Clear(evdata);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const CONNECT_DATA * EVENTDATA_GetConnectData(const EVENTDATA_SYSTEM * evdata, const VecFx32 * pos)
{
	return NULL;
}

#if 0
BOOL DEBUG_EVENT_GetConnectEvent(GAMESYS_WORK * gsys, const EVENTDATA_SYSTEM * evdata,
		const VecFx32 * player_pos)
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
	const CONNECT_DATA * connect = EVENTDATA_GetConnectData(evdata, player_pos);
	if (connect == NULL) {
		return FALSE;
	}
	return TRUE;
}
#endif

//------------------------------------------------------------------
//------------------------------------------------------------------

//------------------------------------------------------------------
//------------------------------------------------------------------

