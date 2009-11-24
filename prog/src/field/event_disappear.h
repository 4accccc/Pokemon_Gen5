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

// 流砂
GMEVENT* EVENT_DISAPPEAR_FallInSand( GMEVENT* parent, 
                                     GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, 
                                     const VecFx32* stream_pos ); 
// あなぬけのヒモ
GMEVENT* EVENT_DISAPPEAR_Ananukenohimo( GMEVENT* parent, 
                                        GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap ); 
// あなをほる
GMEVENT* EVENT_DISAPPEAR_Anawohoru( GMEVENT* parent, 
                                    GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );
// テレポート
GMEVENT* EVENT_DISAPPEAR_Teleport( GMEVENT* parent, 
                                   GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );
