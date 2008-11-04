//============================================================================================
/**
 * @file	gamemsystem.c
 * @brief	ポケモンゲームシステム（ここが通常ゲームのトップレベル）
 * @author	tamada GAME FREAK Inc.
 * @date	08.10.20
 *
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"		//HEAPIDの参照用

#include "gamesystem/gamesystem.h"
#include "gamesystem/playerwork.h"
#include "game_event.h"	//local

#include "gamesystem/game_init.h"

//============================================================================================
//============================================================================================
enum {
	HEAPID_GAMESYS = HEAPID_TAMADA_DEBUG,

	HEAPSIZE_GAMESYS = 0x2000,
	//x200000
};

//------------------------------------------------------------------
/**
 * @brief	ゲーム制御システム用ワーク保持変数
 *
 * ※たぶんデバッグ用か限定時のグローバル参照用にしか使わないはず
 */
//------------------------------------------------------------------
static GAMESYS_WORK * GameSysWork;

static GAME_INIT_WORK TestGameInitWork;

//============================================================================================
//============================================================================================
extern const GFL_PROC_DATA DebugFieldProcData;

static void GameSystem_Init(GAMESYS_WORK * gsys, HEAPID heapID, GAME_INIT_WORK * init_param);
static BOOL GameSystem_Main(GAMESYS_WORK * gsys);
static void GameSystem_End(GAMESYS_WORK * gsys);
static u32 GAMESYS_WORK_GetSize(void);


//============================================================================================
/**
 * @brief	ゲームシステム用プロセス
 *
 * NitroMain->GFLUser_Main->GFL_PROC_Main()に登録される。
 * 基本的にはゲーム本編開始後はこのプロセスが動作し続けるはず
 */
//============================================================================================
static GFL_PROC_RESULT GameMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT GameMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT GameMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk);

//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT GameMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAMESYS_WORK * gsys;
	u32 work_size = GAMESYS_WORK_GetSize();
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMESYS, HEAPSIZE_GAMESYS );
	gsys = GFL_PROC_AllocWork(proc, work_size, HEAPID_GAMESYS);
	GameSysWork = gsys;
	GameSystem_Init(gsys, HEAPID_GAMESYS, pwk);
#if 1		/* 暫定的にプロセス登録 */
	GameSystem_CallProc(gsys, NO_OVERLAY_ID, &DebugFieldProcData, gsys);
#endif
	return GFL_PROC_RES_FINISH;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT GameMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAMESYS_WORK * gsys = mywk;
	if (GameSystem_Main(gsys)) {
		return GFL_PROC_RES_FINISH;
	} else {
		return GFL_PROC_RES_CONTINUE;
	}
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT GameMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAMESYS_WORK * gsys = mywk;
	GameSystem_End(gsys);
	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap( HEAPID_GAMESYS );
	return GFL_PROC_RES_FINISH;
}
//------------------------------------------------------------------
/**
 * @brief		プロセス関数テーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA GameMainProcData = {
	GameMainProcInit,
	GameMainProcMain,
	GameMainProcEnd,
};


//------------------------------------------------------------------
/**
 * @file	ゲーム開始初期化用ワーク取得
 */
//------------------------------------------------------------------
GAME_INIT_WORK * DEBUG_GetGameInitWork(GAMEINIT_MODE mode, u16 mapid)
{
	TestGameInitWork.mode = mode;
	TestGameInitWork.mapid = mapid;
	return &TestGameInitWork;
}


//============================================================================================
//============================================================================================
enum {
	PLAYER_MAX = 5,
};
//------------------------------------------------------------------
/**
 * @brief	ゲーム制御システム用ワーク定義
 */
//------------------------------------------------------------------
struct _GAMESYS_WORK {
	HEAPID heapID;			///<使用するヒープ指定ID
	void * parent_work;		///<親（呼び出し元）から引き継いだワークへのポインタ

	GFL_PROCSYS * procsys;	///<使用しているPROCシステムへのポインタ
	PLAYER_WORK playerWork[PLAYER_MAX];

	GMEVENT_CONTROL * event;
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static u32 GAMESYS_WORK_GetSize(void)
{
	return sizeof(GAMESYS_WORK);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void GAMESYS_WORK_Init(GAMESYS_WORK * gsys, HEAPID heapID, GAME_INIT_WORK * init_param)
{
	int i;
	gsys->heapID = heapID;
	gsys->parent_work = init_param;
	gsys->procsys = GFL_PROC_LOCAL_boot(gsys->heapID);

	for (i = 0; i < PLAYER_MAX; i++) {
		PLAYERWORK_init(&gsys->playerWork[i]);
	}
	{
		PLAYER_WORK * me = &gsys->playerWork[0];
		PLAYERWORK_setZoneID(me, init_param->mapid);
	}
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void GAMESYS_WORK_Delete(GAMESYS_WORK * gsys)
{
}
//------------------------------------------------------------------
//------------------------------------------------------------------
PLAYER_WORK * GAMESYSTEM_GetMyPlayerWork(GAMESYS_WORK * gsys)
{
	return &gsys->playerWork[0];
}
HEAPID GAMESYSTEM_GetHeapID(GAMESYS_WORK * gsys)
{
	return gsys->heapID;
}
GMEVENT_CONTROL * GAMESYSTEM_GetEvent(GAMESYS_WORK * gsys)
{
	return gsys->event;
}
void GAMESYSTEM_SetEvent(GAMESYS_WORK * gsys, GMEVENT_CONTROL * event)
{
	GF_ASSERT(gsys->event == NULL);
	gsys->event = event;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void GameSystem_Init(GAMESYS_WORK * gsys, HEAPID heapID, GAME_INIT_WORK * init_param)
{
	GAMESYS_WORK_Init(gsys, heapID, init_param);
}

//------------------------------------------------------------------
/**
 * @return BOOL	TRUEのとき、ゲーム終了
 */
//------------------------------------------------------------------
static BOOL GameSystem_Main(GAMESYS_WORK * gsys)
{
	BOOL proc_result;
	//Game Server Proccess
	GAMESYSTEM_EVENT_Main(gsys);
	//	PlayerController/Event
	proc_result = GFL_PROC_LOCAL_Main(gsys->procsys);
	if (proc_result == FALSE && gsys->event == NULL) {
		//プロセスもイベントも存在しないとき、ゲーム終了
		return TRUE;
	} else {
		return FALSE;
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void GameSystem_End(GAMESYS_WORK * gsys)
{
	GFL_PROC_LOCAL_Exit(gsys->procsys);
	GAMESYS_WORK_Delete(gsys);
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void GameSystem_SetNextProc(GAMESYS_WORK * gsys,
		FSOverlayID ov_id, const GFL_PROC_DATA *procdata, void * pwk)
{
	GFL_PROC_LOCAL_SetNextProc(gsys->procsys, ov_id, procdata, pwk);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GameSystem_CallProc(GAMESYS_WORK * gsys,
		FSOverlayID ov_id, const GFL_PROC_DATA *procdata, void * pwk)
{
	GFL_PROC_LOCAL_CallProc(gsys->procsys, ov_id, procdata, pwk);
}


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	PLAYER_WORK初期化
 */
//------------------------------------------------------------------
void PLAYERWORK_init(PLAYER_WORK * player)
{
	player->zoneID = 0;
	player->position.x = 0;
	player->position.y = 0;
	player->position.z = 0;
	player->direction = 0;
	player->mystatus.id = 0;
	player->mystatus.sex = 0;
	player->mystatus.region_code = 0;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYERWORK_setPosition(PLAYER_WORK * player, const VecFx32 * pos)
{
	player->position = *pos;
}

const VecFx32 * PLAYERWORK_getPosition(const PLAYER_WORK * player)
{
	return &player->position;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYERWORK_setZoneID(PLAYER_WORK * player, ZONEID zoneid)
{
	player->zoneID = zoneid;
}

ZONEID PLAYERWORK_getZoneID(const PLAYER_WORK * player)
{
	return player->zoneID;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYERWORK_setDirection(PLAYER_WORK * player, u16 direction)
{
	player->direction = direction;
}

u16 PLAYERWORK_getDirection(const PLAYER_WORK * player)
{
	return player->direction;
}

