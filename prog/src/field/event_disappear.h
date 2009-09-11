////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  退場イベント
 * @file   event_disappear.h
 * @author obata
 * @date   2009.08.28
 *
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#pragma once

//------
// 退場
//------
// ワープ
GMEVENT* EVENT_DISAPPEAR_Warp( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap ); 

// 流砂
GMEVENT* EVENT_DISAPPEAR_FallInSand( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, const VecFx32* stream_pos ); 
