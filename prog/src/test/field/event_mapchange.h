//======================================================================
/**
 * @file	event_mapchange.h
 * @brief
 * @date	2008.11.05
 * @author	tamada GAME FREAK inc.
 */
//======================================================================
#pragma once

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "field/fieldmap.h"

//------------------------------------------------------------------
/**
 * @brief	ゲーム開始イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_SetFirstMapIn(GAMESYS_WORK * gsys, GAME_INIT_WORK * game_init_work);

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_ChangeMap(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap,
		const LOCATION * loc_req);
//------------------------------------------------------------------
/**
 * @brief	マップ遷移イベント生成（出入口指定）
 * @param	gsys		ゲームシステムへのポインタ
 * @param	fieldmap	フィールドシステムへのポインタ
 * @param	zone_id		遷移するマップのZONE指定
 * @param	exit_id		遷移するマップの出入口指定
 * @return	GMEVENT		生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeMap(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap,
		u16 zone_id, s16 exit_id);

//------------------------------------------------------------------
/**
 * @brief	マップ遷移イベント生成（座標指定）
 * @param	gsys		ゲームシステムへのポインタ
 * @param	fieldmap	フィールドシステムへのポインタ
 * @param	zone_id		遷移するマップのZONE指定
 * @param	pos			遷移するマップでの座標指定
 * @return	GMEVENT		生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeMapPos(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap,
		u16 zone_id, const VecFx32 * pos);

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeToNextMap(GAMESYS_WORK * gsys, FIELD_MAIN_WORK *fieldmap);

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern void DEBUG_EVENT_ChangeEventMapChange( GAMESYS_WORK *gsys, GMEVENT *event,FIELD_MAIN_WORK *fieldmap, ZONEID zone_id );

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_FieldSample(GAMESYS_WORK * gsys, FIELD_MAIN_WORK *fieldmap);

