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

#include "field/field_const.h"

#include "field/eventdata_type.h"

//#include "mapdefine.h"		//ZONE_ID_～
//#include "fieldobj_code.h"	//DIR_DOWN

//#include "player.h"
//#include "situation_local.h"

#include "arc/fieldmap/zone_id.h"
//===========================================================================
//===========================================================================
enum {
	ZONE_ID_GAMESTART = ZONE_ID_T01R0102,		//とりあえず
};
//--------------------------------------------------------------
//--------------------------------------------------------------
#define DOOR_ID_JUMP_CODE	(-1)


//===========================================================================
//===========================================================================
//--------------------------------------------------------------
/**
 * @brief	LOCATIONの初期化
 */
//--------------------------------------------------------------
void LOCATION_Init(LOCATION * loc)
{
	loc->type = LOCATION_TYPE_INIT;
	loc->zone_id = 0;
	loc->exit_id = 0;
	loc->dir_id = 0;
	loc->pos.x = 0;
	loc->pos.y = 0;
	loc->pos.z = 0;
}

//--------------------------------------------------------------
/**
 * @brief	LOCATIONのセット
 */
//--------------------------------------------------------------
void LOCATION_Set(LOCATION * loc, int zone, s16 door, s16 dir, fx32 x, fx32 y, fx32 z)
{
	loc->type = LOCATION_TYPE_INIT;
	loc->zone_id = zone;
	loc->exit_id = door;
	loc->dir_id = dir;
	loc->pos.x = x;
	loc->pos.y = y;
	loc->pos.z = z;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void LOCATION_SetID(LOCATION * loc, u16 zone_id, u16 exit_id)
{
	LOCATION_Init(loc);
	loc->zone_id = zone_id;
	loc->exit_id = exit_id;
}
//--------------------------------------------------------------
/**
 * @brief	LOCATIONのセット（直接位置指定）
 */
//--------------------------------------------------------------
void LOCATION_SetDirect(LOCATION * loc, int zone, s16 dir, fx32 x, fx32 y, fx32 z)
{
	loc->type = LOCATION_TYPE_DIRECT;
	loc->zone_id = zone;
	loc->exit_id = DOOR_ID_JUMP_CODE;
	loc->dir_id = dir;
	loc->pos.x = x;
	loc->pos.y = y;
	loc->pos.z = z;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void LOCATION_DEBUG_SetDefaultPos(LOCATION * loc, u16 zone_id)
{
	LOCATION_Init(loc);
	loc->type = LOCATION_TYPE_DIRECT;
	loc->zone_id = zone_id;
	loc->exit_id = DOOR_ID_JUMP_CODE;
	loc->dir_id = 0;
	ZONEDATA_DEBUG_GetStartPos(zone_id, &loc->pos);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void LOCATION_SetGameStart(LOCATION * loc)
{
	//LOCATION_DEBUG_SetDefaultPos(loc, ZONE_ID_GAMESTART);
#if 1
  loc->type = LOCATION_TYPE_DIRECT;
  loc->zone_id = ZONE_ID_GAMESTART;
  loc->exit_id = DOOR_ID_JUMP_CODE;
  loc->dir_id = EXIT_DIR_DOWN;
  loc->pos.x = 72 * FX32_ONE;
  loc->pos.y =  0 * FX32_ONE;
  loc->pos.z = 88 * FX32_ONE;
#endif
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
