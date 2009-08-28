////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  退場＆登場イベント
 * @file   event_appear.h
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
GMEVENT* EVENT_DISAPPEAR_FallInSand( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap ); 

//------
// 登場
//------
// ワープ
GMEVENT* EVENT_APPEAR_Warp( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );  

// 落下
GMEVENT* EVENT_APPEAR_Fall( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );  
