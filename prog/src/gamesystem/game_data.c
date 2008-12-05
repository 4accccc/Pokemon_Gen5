//=============================================================================
/**
 * @file	game_data.c
 * @brief	ゲーム進行データ管理
 * @author	tamada
 * @date	2008.11.04
 *
 * 基本的にゲーム進行データはセーブデータとして存在するが、
 * 通信時の複数プレイヤー情報やフィールドマップ情報などセーブデータに
 * のるとは限らない情報もある。
 * そのようなセーブデータ・非セーブデータへ並列にアクセスするインターフェイスを
 * 各パートごとに構成するとパートごとにアクセス手法が違ったり、また同じ機能の
 * ものが複数存在するなどプログラム全体の複雑さが増してしまう可能性が高い。
 * それを避けるため、共通インターフェイスを作ることによって簡略化する試み
 * …試みなので途中で方針を変えるかも。
 */
//=============================================================================

#include <gflib.h>

#include "gamesystem/game_data.h"
#include "gamesystem/playerwork.h"
#include "field/eventdata_system.h"
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/myitem_savedata.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ゲームデータ内容定義
 */
//------------------------------------------------------------------
struct _GAMEDATA{
	PLAYER_WORK playerWork[PLAYER_MAX];
	EVENTDATA_SYSTEM * evdata;
	LOCATION start_loc;
	LOCATION entrance_loc;
	LOCATION special_loc;
	MYITEM_PTR myitem;			///<手持ちアイテムセーブデータへのポインタ
};

//============================================================================================
//
//
//		GAMEDATA全体
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
GAMEDATA * GAMEDATA_Create(HEAPID heapID)
{
	int i;
	GAMEDATA * gd;
	
	gd = GFL_HEAP_AllocMemory(heapID, sizeof(GAMEDATA));
	GFL_STD_MemClear(gd, sizeof(GAMEDATA));

	for (i = 0; i < PLAYER_MAX; i++) {
		PLAYERWORK_init(&gd->playerWork[i]);
	}
	gd->evdata = EVENTDATA_SYS_Create(heapID);
	EVENTDATA_SYS_Clear(gd->evdata);

	gd->myitem = SaveControl_DataPtrGet(SaveControl_GetPointer(), GMDATA_ID_MYITEM);
	
	return gd;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_Delete(GAMEDATA * gamedata)
{
	EVENTDATA_SYS_Delete(gamedata->evdata);
	GFL_HEAP_FreeMemory(gamedata);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
PLAYER_WORK * GAMEDATA_GetPlayerWork(GAMEDATA * gamedata, u32 player_id)
{
	GF_ASSERT(player_id < PLAYER_MAX);
	return &gamedata->playerWork[player_id];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
PLAYER_WORK * GAMEDATA_GetMyPlayerWork(GAMEDATA * gamedata)
{
	return &gamedata->playerWork[0];
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	イベント起動データシステムへのポインタを取得する
 * @param	gamedata			GAMEDATAへのポインタ
 * @return	EVENTDATA_SYSTEM	イベント起動データシステムへのポインタ
 */
//------------------------------------------------------------------
EVENTDATA_SYSTEM * GAMEDATA_GetEventData(GAMEDATA * gamedata)
{
	return gamedata->evdata;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetStartLocation(const GAMEDATA * gamedata)
{
	return &gamedata->start_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetStartLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
	gamedata->start_loc = *loc;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetEntranceLocation(const GAMEDATA * gamedata)
{
	return &gamedata->entrance_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetEntranceLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
	gamedata->entrance_loc = *loc;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetSpecialLocation(const GAMEDATA * gamedata)
{
	return &gamedata->special_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetSpecialLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
	gamedata->special_loc = *loc;
}

//--------------------------------------------------------------
/**
 * @brief   手持ちアイテムへのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  手持ちアイテムへのポインタ
 */
//--------------------------------------------------------------
MYITEM_PTR GAMEDATA_GetMyItem(const GAMEDATA * gamedata)
{
	return gamedata->myitem;
}


//============================================================================================
//
//
//	PLAYER_WORK用関数
//	※player_work.cを作成して移動させるかも
//
//
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

//------------------------------------------------------------------
//------------------------------------------------------------------
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

//------------------------------------------------------------------
//------------------------------------------------------------------
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

//------------------------------------------------------------------
//------------------------------------------------------------------
u16 PLAYERWORK_getDirection(const PLAYER_WORK * player)
{
	return player->direction;
}

