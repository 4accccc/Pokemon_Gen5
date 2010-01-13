//======================================================================
/**
 * @file  event_mapchange.h
 * @brief マップ遷移関連イベント
 * @date  2008.11.05
 * @author  tamada GAME FREAK inc.
 *
 * @todo  DEBUG_という名前の本番用関数をリネームする
 * @todo  新規ゲームとコンティニューとで同一関数を呼び出しているのを切り分ける
 *
 * @note
 * 基本的にはイベントのみを外部公開して複雑な初期化やフラグ制御は
 * この内部に納めたい。（現在のところ例外はMAPCHG_GameOver）
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
 * @brief ゲーム開始イベント
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_CallGameStart(GAMESYS_WORK * gsys, GAME_INIT_WORK * game_init_work);

//------------------------------------------------------------------
/**
 * @brief デバッグ用：ゲーム終了
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_CallGameEnd( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap);

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_ChangeMapByConnect(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    const CONNECT_DATA * cnct, LOC_EXIT_OFS exit_ofs);

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（座標指定）
 * @param gsys    ゲームシステムへのポインタ
 * @param fieldmap  フィールドシステムへのポインタ
 * @param zone_id   遷移するマップのZONE指定
 * @param pos     遷移するマップでの座標指定
 * @param dir     遷移するマップでの方向指定
 * @return  GMEVENT   生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_ChangeMapPos(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    u16 zone_id, const VecFx32 * pos, u16 dir );

//------------------------------------------------------------------
/**
 * @brief 空を飛ぶマップ遷移イベント生成（座標指定）
 * @param gsys    ゲームシステムへのポインタ
 * @param fieldmap  フィールドシステムへのポインタ
 * @param zone_id   遷移するマップのZONE指定
 * @param pos     遷移するマップでの座標指定
 * @param dir     遷移するマップでの方向指定
 * @return  GMEVENT   生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_ChangeMapSorawotobu(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    u16 zone_id, const VecFx32 * pos, u16 dir );

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（レール座標指定）
 * @param gsys    ゲームシステムへのポインタ
 * @param fieldmap  フィールドシステムへのポインタ
 * @param zone_id   遷移するマップのZONE指定
 * @param rail_loc  遷移するマップでのレール座標指定
 * @param dir     遷移するマップでの方向指定
 * @return  GMEVENT   生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeMapRailLocation(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    u16 zone_id, const RAIL_LOCATION * rail_loc, u16 dir );

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（デフォルト座標）
 * @param gsys    ゲームシステムへのポインタ
 * @param fieldmap  フィールドシステムへのポインタ
 * @param zone_id   遷移するマップのZONE指定
 * @return  GMEVENT   生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeMapDefaultPos(GAMESYS_WORK * gsys,
    FIELDMAP_WORK * fieldmap, u16 zone_id);

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（デフォルト座標, フェード短縮版）
 * @param gsys    ゲームシステムへのポインタ
 * @param fieldmap  フィールドシステムへのポインタ
 * @param zone_id   遷移するマップのZONE指定
 * @return  GMEVENT   生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_QuickChangeMapDefaultPos(GAMESYS_WORK * gsys,
    FIELDMAP_WORK * fieldmap, u16 zone_id);

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（ 流砂 ）
 * @param gsys          ゲームシステムへのポインタ
 * @param fieldmap      フィールドシステムへのポインタ
 * @param disappear_pos 流砂中心点の座標
 * @param zone_id       遷移するマップのZONE指定
 * @param appear        遷移先での座標
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_ChangeMapBySandStream(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, 
    const VecFx32* disappear_pos, u16 zone_id, const VecFx32* appear_pos );

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（ あなぬけのヒモ ）
 * @param gsys ゲームシステムへのポインタ
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT* EVENT_ChangeMapByAnanukenohimo( FIELDMAP_WORK *fieldWork, GAMESYS_WORK * gsys );

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（ あなをほる )
 * @param gsys ゲームシステムへのポインタ
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT* EVENT_ChangeMapByAnawohoru( GAMESYS_WORK * gsys );

//------------------------------------------------------------------
/**
 * @brief マップ遷移イベント生成（ テレポート )
 * @param gsys ゲームシステムへのポインタ
 * @return GMEVENT 生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT* EVENT_ChangeMapByTeleport( GAMESYS_WORK * gsys );

//------------------------------------------------------------------
/**
 * @brief ユニオンへのマップ遷移イベント生成
 * @param  gsys      ゲームシステムへのポインタ
 * @param  fieldmap  フィールドシステムへのポインタ
 * @return GMEVENT   生成したマップ遷移イベント
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_ChangeMapToUnion( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap );

extern GMEVENT * EVENT_ChangeMapFromUnion( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap );

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
extern GMEVENT * DEBUG_EVENT_ChangeToNextMap(GAMESYS_WORK * gsys, FIELDMAP_WORK *fieldmap);

extern GMEVENT * EVENT_ChangeMapPosNoFade(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    u16 zone_id, const VecFx32 * pos, u16 dir );

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void MAPCHANGE_setPlayerVanish(FIELDMAP_WORK * fieldmap, BOOL vanish_flag);

//--------------------------------------------------------------
/**
 * @brief 全滅時のマップ遷移処理（フィールド非生成時）
 */
//--------------------------------------------------------------
extern GMEVENT * EVENT_CallGameOver( GAMESYS_WORK * gsys );


