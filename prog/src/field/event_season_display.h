//////////////////////////////////////////////////////////////////////////
/**
 * @brief  季節表示イベント
 * @file   event_season_display.c
 * @author obata
 * @date   2009.11.25
 */
//////////////////////////////////////////////////////////////////////////
#pragma once


//------------------------------------------------------------------------
/**
 * @brief 四季表示イベントを作成する
 *
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 * @param start    開始季節
 * @param end      最終季節 
 *
 * @return 作成したイベント
 */
//------------------------------------------------------------------------
extern GMEVENT* EVENT_SeasonDisplay( 
    GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, u8 start, u8 end );
