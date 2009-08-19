/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file   event_entrance_in.h
 * @breif  出入口への進入時のイベント作成
 * @author obata
 * @date   2009.08.18
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////
#pragma once


//---------------------------------------------------------------------------------------
/**
 * @brief 出入口進入イベントを作成する
 *
 * @param parent    親イベント
 * @param gsys      ゲームシステム
 * @param gdata     ゲームデータ
 * @param fieldmap  フィールドマップ
 * @param location  遷移先指定
 * @param exit_type 遷移タイプ指定
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_EntranceIn( GMEVENT* parent, 
                           GAMESYS_WORK* gsys,
                           GAMEDATA* gdata, 
                           FIELD_MAIN_WORK* fieldmap, 
                           LOCATION location, 
                           EXIT_TYPE exit_type );
