//=============================================================================
/**
 * @file	game_data.c
 * @brief	ゲーム進行データ管理
 * @author	tamada
 * @date	2008.11.04
 *
 * 基本的にゲーム進行データはセーブデータとして存在するが、
 * 通信時の複数プレイヤー情報やフィールドマップ情報などセーブデータに
 * のるとは限らない情報もある。そのようなセーブデータ・非セーブデータへ
 * 並列にアクセスすることを各パートが自分で構成するとプログラム全体の
 * 複雑さが増してしまうので、インターフェイスを作ることによって
 * 簡略化する試み…試みなので途中で方針を変えるかも。
 */
//=============================================================================

#include <gflib.h>

#include "gamesystem/game_data.h"
#include "gamesystem/playerwork.h"

//============================================================================================
//============================================================================================
struct _GAMEDATA{
	PLAYER_WORK playerWork[PLAYER_MAX];
};

//============================================================================================
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
	return gd;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_Delete(GAMEDATA * gamedata)
{
	GFL_HEAP_FreeMemory(gamedata);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
PLAYER_WORK * GAMEDATA_GetPlayerWork(GAMEDATA * gamedata, u32 player_id)
{
	GF_ASSERT(player_id < PLAYER_MAX);
	return &gamedata->playerWork[player_id];
}

PLAYER_WORK * GAMEDATA_GetMyPlayerWork(GAMEDATA * gamedata)
{
	return &gamedata->playerWork[0];
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

