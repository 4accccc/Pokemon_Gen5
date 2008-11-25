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

#include "field/location.h"

#include "fieldmap/zone_id.h"
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


extern const CONNECT_DATA SampleConnectData[];
extern const int SampleConnectDataCount;
//============================================================================================
//
//	イベント起動データシステム関連
//
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
	if (zone_id == ZONE_ID_MAPSUMMER || zone_id == ZONE_ID_MAPSPRING) {
		evdata->connect_count = SampleConnectDataCount;
		evdata->connect_data = SampleConnectData;
	}
}


//============================================================================================
//
//	出入口接続データ関連
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
const CONNECT_DATA * EVENTDATA_SearchConnectByPos(const EVENTDATA_SYSTEM * evdata, const VecFx32 * pos)
{
	int i;
	int x,y,z;
	const CONNECT_DATA * cnct = evdata->connect_data;
	x = FX_Whole(pos->x);
	y = FX_Whole(pos->y);
	z = FX_Whole(pos->z);
	for (i = 0; i < evdata->connect_count; i++, cnct++ ) {
		if (FX_Whole(cnct->pos.x) != x) continue;
		if (FX_Whole(cnct->pos.y) != y) continue;
		if (FX_Whole(cnct->pos.z) != z) continue;
		return cnct;
	}
	return NULL;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const CONNECT_DATA * EVENTDATA_GetConnectByID(const EVENTDATA_SYSTEM * evdata, u16 exit_id)
{
	const CONNECT_DATA * cnct = evdata->connect_data;
	if (cnct == NULL) {
		return NULL;
	}
	if (exit_id >= evdata->connect_count) {
		GF_ASSERT(0);
		return NULL;
	}
	return &cnct[exit_id];
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void CONNECTDATA_SetLocation(const CONNECT_DATA * connect, LOCATION * loc)
{
	GF_ASSERT(connect != NULL);
	loc->zone_id = connect->link_zone_id;
	loc->door_id = connect->link_exit_id;
	loc->pos = connect->pos;
	loc->dir_id = 0;
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
const CONNECT_DATA SampleConnectData[] = {
	{
		{FX32_ONE * 160, FX32_ONE * 0, FX32_ONE * 0},
		ZONE_ID_MAPSUMMER,	1,
		EXIT_TYPE_DOWN,
	},
	{
		{FX32_ONE * 48, FX32_ONE * 0, FX32_ONE * 96 },
		ZONE_ID_MAPSUMMER,	0,
		EXIT_TYPE_RIGHT,
	},
};

const int SampleConnectDataCount = NELEMS(SampleConnectData);

//------------------------------------------------------------------
//------------------------------------------------------------------

