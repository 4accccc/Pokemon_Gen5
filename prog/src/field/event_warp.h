////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  ワープイベント
 * @file   event_warp.h
 * @author obata
 * @date   2009.08.28
 *
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#pragma once

// ワープイベントを作成する
GMEVENT* EVENT_WarpIn( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );
GMEVENT* EVENT_WarpOut( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );

// 落下イベントを作成する
GMEVENT* EVENT_FallIn( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );
GMEVENT* EVENT_FallOut( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );
