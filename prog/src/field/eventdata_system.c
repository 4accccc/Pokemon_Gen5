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

#include "fldmmdl.h"

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
	u16 npc_count;
	u16 connect_count;
	u16 pos_count;
	const BG_TALK_DATA * bg_data;
	const FLDMMDL_HEADER *npc_data;
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
extern const FLDMMDL_HEADER SampleFldMMdlHeader_4season[];
extern const int SampleFldMMdlHeaderCount_4season;

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
	evdata->npc_count = 0;
	evdata->connect_count = 0;
	evdata->pos_count = 0;
	evdata->bg_data = NULL;
	evdata->npc_data = NULL;
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
		evdata->npc_count = SampleFldMMdlHeaderCount_4season;
		evdata->npc_data = SampleFldMMdlHeader_4season;
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
	x = FX_Whole(pos->x) - OFS_X;
	y = FX_Whole(pos->y) - OFS_Y;
	z = FX_Whole(pos->z) - OFS_Z;
	for (i = 0; i < evdata->connect_count; i++, cnct++ ) {
#if 0
		OS_Printf("CNCT:x,y,z=%d,%d,%d\n",
				FX_Whole(cnct->pos.x),FX_Whole(cnct->pos.y),FX_Whole(cnct->pos.z));
#endif
		if (cnct->pos.x != x) continue;
		if (cnct->pos.y != y) continue;
		if (cnct->pos.z != z) continue;
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
	//loc->pos = connect->pos;
	VEC_Set(&loc->pos,
			connect->pos.x * FX32_ONE,
			connect->pos.y * FX32_ONE,
			connect->pos.z * FX32_ONE);
	loc->dir_id = connect->exit_dir;
	loc->zone_id = evdata->now_zone_id;
	loc->exit_id = exit_id;

	return TRUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void CONNECTDATA_SetNextLocation(const CONNECT_DATA * connect, LOCATION * loc)
{
	LOCATION_SetID(loc, connect->link_zone_id, connect->link_exit_id);
}

//============================================================================================
//		動作モデル関連
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	動作モデルヘッダーを取得
 * @param	evdata		イベントデータへのポインタ
 * @retval	FLDMMDL_HEADER*
 */
//------------------------------------------------------------------
const FLDMMDL_HEADER * EVENTDATA_GetNpcData( const EVENTDATA_SYSTEM *evdata )
{
	return( evdata->npc_data );
}

//------------------------------------------------------------------
/**
 * @brief	動作モデル総数を取得
 * @param	evdata		イベントデータへのポインタ
 * @retval	u16
 */
//------------------------------------------------------------------
u16 EVENTDATA_GetNpcCount( const EVENTDATA_SYSTEM *evdata )
{
	return( evdata->npc_count );
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
		{160, 0, 0},
		ZONE_ID_PLANNERTEST,	1,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},
	{
		{48, 0, 96 },
		ZONE_ID_PLANNERTEST,	0,
		EXIT_DIR_RIGHT, EXIT_TYPE_NONE,
	},
	{
		{704, 16, 112 },
		ZONE_ID_TESTPC,	1,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},
};
const int SampleConnectDataCount = NELEMS(SampleConnectData);

const CONNECT_DATA SampleConnectData_4season[] = {
	{
		{(1432 - 8), 0, (1288 - 8)},
		ZONE_ID_TESTPC, 1,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},
	{
		//{(1160 - 8), 0, (1352 - 8)},
		{(1240 - 8), 0, (1336 - 8)},
		ZONE_ID_TESTROOM, 0,
		//EXIT_DIR_DOWN, EXIT_TYPE_NONE,
		EXIT_DIR_RIGHT, EXIT_TYPE_NONE,
	},
	{
		{(1192 - 8), 0, (1160 - 8)},
		ZONE_ID_TESTROOM, 0,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},
	{
		{(1432 - 8), 48, (1144 - 8 - 16)},
		ZONE_ID_TESTROOM, 0,
		EXIT_DIR_DOWN, EXIT_TYPE_NONE,
	},

};
const int SampleConnectDataCount_4season = NELEMS(SampleConnectData_4season);

const CONNECT_DATA SampleConnectData_testpc[] = {
	{
		{128, 0, 224 },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		//ZONE_ID_PLANNERTEST,	2,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
	{
		{144, 0, 224 },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		//ZONE_ID_PLANNERTEST,	2,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
	{
		{160, 0, 224 },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		//ZONE_ID_PLANNERTEST,	2,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
};
const int SampleConnectDataCount_testpc = NELEMS(SampleConnectData_testpc);

const CONNECT_DATA SampleConnectData_testroom[] = {
	{
		{(72 - 8), 0, (88 - 8) },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
	{
		{(88 - 8), 0, (88 - 8) },
		ZONE_ID_PLANNERTEST,	EXIT_ID_SPECIAL,
		EXIT_DIR_UP, EXIT_TYPE_NONE,
	},
};
const int SampleConnectDataCount_testroom = NELEMS(SampleConnectData_testroom);

const FLDMMDL_HEADER SampleFldMMdlHeader_4season[] = {
	{
		0,		///<識別ID
		KABI32,	///<表示するOBJコード
		MV_RND,	///<動作コード
		0,	///<イベントタイプ
		0,	///<イベントフラグ
		0,	///<イベントID
		DIR_DOWN,	///<指定方向
		0,	///<指定パラメタ 0
		0,	///<指定パラメタ 1
		0,	///<指定パラメタ 2
		4,	///<X方向移動制限
		4,	///<Z方向移動制限
		91,	///<グリッドX
		85,	///<グリッドZ
		0,	///<Y値 fx32型
	},
};

const int SampleFldMMdlHeaderCount_4season = NELEMS(SampleFldMMdlHeader_4season);

//------------------------------------------------------------------
//------------------------------------------------------------------

