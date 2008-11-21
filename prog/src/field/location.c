//===========================================================================
/**
 * @file	location.c
 * @brief	マップ位置の初期化など
 * @date	2006.06.02
 * @author	tamada GAME FREAK inc.
 *
 * 2008.11.21	DPプロジェクトから移植
 */
//===========================================================================

//#include "common.h"

#include <gflib.h>

#include "field/zonedata.h"
#include "field/location.h"

//#include "mapdefine.h"		//ZONE_ID_～
//#include "fieldobj_code.h"	//DIR_DOWN

//#include "player.h"
//#include "situation_local.h"

//===========================================================================
//===========================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void LOCATION_DEBUG_SetDefaultPos(LOCATION * loc, u16 zone_id)
{
	loc->zone_id = zone_id;
	loc->door_id = DOOR_ID_JUMP_CODE;
	loc->dir_id = 0;
	ZONEDATA_DEBUG_GetStartPos(zone_id, &loc->pos);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void LOCATION_SetGameStart(LOCATION * loc)
{
	LOCATION_DEBUG_SetDefaultPos(loc, 0);
}

#if 0
//===========================================================================
//===========================================================================
//------------------------------------------------------------------
//	ゲーム開始位置の定義
//------------------------------------------------------------------
enum {
	//GAME_START_ZONE = ZONE_ID_T01R0102,
	GAME_START_ZONE = ZONE_ID_T01R0202,		//06.03.07変更
	GAME_START_X = 4,
	GAME_START_Z = 6,

	DEBUG_START_ZONE = ZONE_ID_C01,
	DEBUG_START_X = 174,
	DEBUG_START_Z = 764,

	GAME_START_EZONE = ZONE_ID_T01,
	GAME_START_EX = 116,
	GAME_START_EZ = 886,
};


#endif
