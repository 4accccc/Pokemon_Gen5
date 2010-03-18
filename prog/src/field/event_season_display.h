////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  季節表示イベント
 * @file   event_season_display.h
 * @author obata
 * @date   2009.11.25
 */
////////////////////////////////////////////////////////////////////////////////
#pragma once

//------------------------------------------------------------------------------
/**
 * @brief 四季表示イベントを生成する
 *
 * @param gameSystem
 * @param fieldmap 
 * @param start    開始季節
 * @param end      最終季節 
 *
 * @return マップチェンジとして機能する四季表示イベント
 */
//------------------------------------------------------------------------------
extern GMEVENT* EVENT_SeasonDisplay( 
    GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, u8 start, u8 end );

//------------------------------------------------------------------------------
/**
 * @brief 四季表示イベントを生成する
 *
 * @param gameSystem
 * @param start    開始季節
 * @param end      最終季節 
 *
 * @return 単独動作を想定した四季表示イベント
 */
//------------------------------------------------------------------------------
extern GMEVENT* EVENT_SimpleSeasonDisplay( GAMESYS_WORK* gameSystem, u8 start, u8 end );
