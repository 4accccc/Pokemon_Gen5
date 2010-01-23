/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file   event_entrance_out.h
 * @breif  出入口からの退出時のイベント作成
 * @author obata
 * @date   2009.08.18
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////
#pragma once


//---------------------------------------------------------------------------------------
/**
 * @brief 出入口退出イベントを作成する
 *
 * @param parent            親イベント
 * @param gameSystem
 * @param gameData
 * @param fieldmap
 * @param location          遷移先指定
 * @param seasonDisplayFlag 季節表示を行うかどうか
 * @param startSeason       最初に表示する季節
 * @param endtSeason        最後に表示する季節
 *
 * @return 作成したイベント
 */
//---------------------------------------------------------------------------------------
GMEVENT* EVENT_EntranceOut( GMEVENT* parent, 
                            GAMESYS_WORK* gameSystem,
                            GAMEDATA* gameData, 
                            FIELDMAP_WORK* fieldmap, 
                            LOCATION location,
                            BOOL seasonDisplayFlag,
                            u8 startSeason,
                            u8 endSeason );
