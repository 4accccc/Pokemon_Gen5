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
#include "field/eventdata_sxy.h"

//======================================================================
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	ゲーム開始イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_SetFirstMapIn(GAMESYS_WORK * gsys, GAME_INIT_WORK * game_init_work);

//------------------------------------------------------------------
/**
 * @brief	デバッグ用：ゲーム終了
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_GameEnd( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap);

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_ChangeMapByConnect(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    const CONNECT_DATA * cnct);

//------------------------------------------------------------------
/**
 * @brief	マップ遷移イベント生成（座標指定）
 * @param	gsys		ゲームシステムへのポインタ
 * @param	fieldmap	フィールドシステムへのポインタ
 * @param	zone_id		遷移するマップのZONE指定
 * @param	pos			遷移するマップでの座標指定
 * @param	dir			遷移するマップでの方向指定
 * @return	GMEVENT		生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeMapPos(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		u16 zone_id, const VecFx32 * pos, u16 dir );

//------------------------------------------------------------------
/**
 * @brief	マップ遷移イベント生成（デフォルト座標）
 * @param	gsys		ゲームシステムへのポインタ
 * @param	fieldmap	フィールドシステムへのポインタ
 * @param	zone_id		遷移するマップのZONE指定
 * @return	GMEVENT		生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeMapDefaultPos(GAMESYS_WORK * gsys,
		FIELDMAP_WORK * fieldmap, u16 zone_id);

//------------------------------------------------------------------
/**
 * @brief	マップ遷移イベント生成（デフォルト座標）
 * @param	gsys		ゲームシステムへのポインタ
 * @param	fieldmap	フィールドシステムへのポインタ
 * @param	zone_id		遷移するマップのZONE指定
 * @return	GMEVENT		生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeMapDefaultPosByWarp(GAMESYS_WORK * gsys,
		FIELDMAP_WORK * fieldmap, u16 zone_id);

//------------------------------------------------------------------
/**
 * @brief	流砂によるマップ遷移イベント生成（座標指定）
 * @param	gsys		ゲームシステムへのポインタ
 * @param	fieldmap	フィールドシステムへのポインタ
 * @param disappear_pos 流砂中心点の座標
 * @param	zone_id		遷移するマップのZONE指定
 * @param appear 遷移先での座標
 * @return	GMEVENT		生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_ChangeMapBySandStream(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, 
    const VecFx32* disappear_pos, u16 zone_id, const VecFx32* appear_pos );

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeToNextMap(GAMESYS_WORK * gsys, FIELDMAP_WORK *fieldmap);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void MAPCHANGE_setPlayerVanish(FIELDMAP_WORK * fieldmap, BOOL vanish_flag);

