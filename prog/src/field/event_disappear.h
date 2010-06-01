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
extern GMEVENT* EVENT_DISAPPEAR_FallInSand( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, const VecFx32* stream_pos ); 

// あなぬけのヒモ
extern GMEVENT* EVENT_DISAPPEAR_Ananukenohimo( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, BOOL season_change_flag ); 

// あなをほる
extern GMEVENT* EVENT_DISAPPEAR_Anawohoru( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, BOOL season_change_flag ); 

// テレポート
extern GMEVENT* EVENT_DISAPPEAR_Teleport( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, BOOL teleport_f ); 
// ワープ
extern GMEVENT* EVENT_DISAPPEAR_Warp( 
    GMEVENT* parent, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );
