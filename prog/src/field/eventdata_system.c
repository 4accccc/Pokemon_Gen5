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

#include "eventdata_local.h"
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

	u16 now_zone_id;
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
extern const CONNECT_DATA SampleConnectData_testpc[];
extern const int SampleConnectDataCount_testpc;
extern const CONNECT_DATA SampleConnectData_testroom[];
extern const int SampleConnectDataCount_testroom;
extern const CONNECT_DATA SampleConnectData_4season[];
extern const int SampleConnectDataCount_4season;
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
	evdata->now_zone_id = zone_id;

	/* テスト的に接続データを設定 */
	switch (zone_id) {
	case ZONE_ID_TESTPC:
		evdata->connect_count = SampleConnectDataCount_testpc;
		evdata->connect_data = SampleConnectData_testpc;
		break;
	case ZONE_ID_TESTROOM:
		evdata->connect_count = SampleConnectDataCount_testroom;
		evdata->connect_data = SampleConnectData_testroom;
		break;
	case ZONE_ID_PLANNERTEST:
		evdata->connect_count = SampleConnectDataCount;
		evdata->connect_data = SampleConnectData;
		break;
	case ZONE_ID_MAPSPRING:
	case ZONE_ID_MAPSUMMER:
	case ZONE_ID_MAPAUTUMN:
	case ZONE_ID_MAPWINTER:
		evdata->connect_count = SampleConnectDataCount_4season;
		evdata->connect_data = SampleConnectData_4season;
		break;

	}
}


//============================================================================================
//
//	出入口接続データ関連
//
//============================================================================================
#define	ZONE_ID_SPECIAL		(0x0fff)
#define	EXIT_ID_SPECIAL		(0x0100)
//------------------------------------------------------------------
//------------------------------------------------------------------
int EVENTDATA_SearchConnectIDByPos(const EVENTDATA_SYSTEM * evdata, const VecFx32 * pos)
{
	int i;
	int x,y,z;
	enum {
		OFS_X = 8,
		OFS_Y = 0,
		OFS_Z = 8,
	};
	const CONNECT_DATA * cnct = evdata->connect_data;
	x = FX_Whole(pos->x);
	y = FX_Whole(pos->y);
	z = FX_Whole(pos->z);
	for (i = 0; i < evdata->connect_count; i++, cnct++ ) {
#if 0
		OS_Printf("CNCT:x,y,z=%d,%d,%d\n",
				FX_Whole(cnct->pos.x),FX_Whole(cnct->pos.y),FX_Whole(cnct->pos.z));
#endif
		if (FX_Whole(cnct->pos.x) + OFS_X != x) continue;
		if (FX_Whole(cnct->pos.y) + OFS_Y != y) continue;
		if (FX_Whole(cnct->pos.z) + OFS_Z != z) continue;
		OS_Printf("CNCT:zone,exit,type=%d,%d,%d\n",cnct->link_zone_id,cnct->link_exit_id,cnct->exit_type);
		return i;
	}
	return EXIT_ID_NONE;
}
#if 0
//------------------------------------------------------------------
//------------------------------------------------------------------
const CONNECT_DATA * EVENTDATA_SearchConnectByPos(const EVENTDATA_SYSTEM * evdata, const VecFx32 * pos)
{
	int idx = EVENTDATA_SearchConnectIDByPos(evdata, pos);
	if (idx == EXIT_ID_NONE) return NULL;
	return evdata->connect_data + idx;
}
#endif
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
BOOL CONNECTDATA_IsSpecialExit(const CONNECT_DATA * connect)
{
	return (connect->link_exit_id == EXIT_ID_SPECIAL);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL EVENTDATA_SetLocationByExitID(const EVENTDATA_SYSTEM * evdata, LOCATION * loc, u16 exit_id)
{
	const CONNECT_DATA * connect = EVENTDATA_GetConnectByID(evdata, exit_id);
	if (connect == NULL) {
		return FALSE;
	}

	//CONNECTDATA_SetLocation(connect, loc);
	LOCATION_Init(loc);
	if (connect->link_exit_id == EXIT_ID_SPECIAL) {
		loc->type = LOCATION_TYPE_SPID;
	} else {
		loc->type = LOCATION_TYPE_EXITID;
	}
	//loc->zone_id = connect->link_zone_id;
	//loc->exit_id = connect->link_exit_id;
	loc->pos = connect->pos;
	loc->dir_id = connect->exit_dir;
	loc->zone_id = evdata->now_zone_id;
	loc->exit_id = exit_id;

	return TRUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const CONNECTDATA_SetNextLocation(const CONNECT_DATA * connect, LOCATION * loc)
{
	LOCATION_SetID(loc, connect->link_zone_id, connect->link_exit_id);
}

//============================================================================================
//
//		サンプルデータ
//		※実際には外部でコンバートされたものをファイルから読み込む
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
const CONNECT_DATA SampleConnectData[] = {
	{
		{FX32_ONE * 160, FX32_ONE * 0, FX32_ONE * 0},
		ZONE_ID_PLANNERTEST,	1,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},
	{
		{FX32_ONE * 48, FX32_ONE * 0, FX32_ONE * 96 },
		ZONE_ID_PLANNERTEST,	0,
		EXIT_DIR_RIGHT, EXIT_TYPE_NONE,
	},
	{
		{FX32_ONE * 704, FX32_ONE * 16, FX32_ONE * 112 },
		ZONE_ID_TESTPC,	1,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},
};
const int SampleConnectDataCount = NELEMS(SampleConnectData);

const CONNECT_DATA SampleConnectData_4season[] = {
	{
		{FX32_ONE * (1432 - 8), FX32_ONE * 0, FX32_ONE * (1288 - 8)},
		ZONE_ID_TESTPC, 1,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},
	{
		{FX32_ONE * (1160 - 8), FX32_ONE * 0, FX32_ONE * (1352 - 8)},
		ZONE_ID_TESTROOM, 0,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},
	{
		{FX32_ONE * (1192 - 8), FX32_ONE * 0, FX32_ONE * (1160 - 8)},
		ZONE_ID_TESTROOM, 0,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},
	{
		{FX32_ONE * (1432 - 8), FX32_ONE * 48, FX32_ONE * (1144 - 8 - 16)},
		ZONE_ID_TESTROOM, 0,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},

};
const int SampleConnectDataCount_4season = NELEMS(SampleConnectData_4season);

const CONNECT_DATA SampleConnectData_testpc[] = {
	{
		{FX32_ONE * 128, FX32_ONE * 0, FX32_ONE * 224 },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		//ZONE_ID_PLANNERTEST,	2,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
	{
		{FX32_ONE * 144, FX32_ONE * 0, FX32_ONE * 224 },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		//ZONE_ID_PLANNERTEST,	2,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
	{
		{FX32_ONE * 160, FX32_ONE * 0, FX32_ONE * 224 },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		//ZONE_ID_PLANNERTEST,	2,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
};
const int SampleConnectDataCount_testpc = NELEMS(SampleConnectData_testpc);

const CONNECT_DATA SampleConnectData_testroom[] = {
	{
		{FX32_ONE * (72 - 8), FX32_ONE * 0, FX32_ONE * (88 - 8) },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
	{
		{FX32_ONE * (88 - 8), FX32_ONE * 0, FX32_ONE * (88 - 8) },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
};
const int SampleConnectDataCount_testroom = NELEMS(SampleConnectData_testroom);

//------------------------------------------------------------------
//------------------------------------------------------------------

