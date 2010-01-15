////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  登場イベント
 * @file   event_appear.h
 * @author obata
 * @date   2009.08.28
 *
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#pragma once

// 落下
extern GMEVENT* EVENT_APPEAR_Fall( GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );  

// あなぬけのヒモ
extern GMEVENT* EVENT_APPEAR_Ananukenohimo( GMEVENT* parent, 
                                            GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );  
// あなをほる
extern GMEVENT* EVENT_APPEAR_Anawohoru( GMEVENT* parent, 
                                        GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );  
// テレポート
extern GMEVENT* EVENT_APPEAR_Teleport( GMEVENT* parent, 
                                       GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );  
// ワープ
extern GMEVENT* EVENT_APPEAR_Warp( GMEVENT* parent, 
                                   GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );  
