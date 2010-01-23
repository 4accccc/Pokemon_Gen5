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
 * @param parent            親イベント
 * @param gameSystem
 * @param gameData
 * @param fieldmap
 * @param nextLocation      遷移先指定
 * @param exitType          遷移タイプ指定
 * @param seasonDisplayFlag 季節表示を行うかどうか
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
extern GMEVENT* EVENT_EntranceIn( GMEVENT* parent, 
                                  GAMESYS_WORK* gameSystem,
                                  GAMEDATA* gameData, 
                                  FIELDMAP_WORK* fieldmap, 
                                  LOCATION nextLocation, 
                                  EXIT_TYPE exitType,
                                  BOOL seasonDisplayFlag );
