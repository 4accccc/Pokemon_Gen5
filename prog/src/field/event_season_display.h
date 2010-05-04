////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  季節表示イベント
 * @file   event_season_display.h
 * @author obata
 * @date   2009.11.25
 */
////////////////////////////////////////////////////////////////////////////////
#pragma once

// コールバック関数の型
typedef void (*SEASON_DISP_CALLBACK_FUNC)(void* callback_param);


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

// コールバック関数ありver. ( フィールド画面がフェードインする直前にコールされる )
extern GMEVENT* EVENT_SeasonDisplay_Callback( 
    GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, u8 start, u8 end,
    SEASON_DISP_CALLBACK_FUNC callback_func, void* callback_param );

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
