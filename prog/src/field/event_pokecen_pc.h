///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケセンPC関連イベント
 * @file   event_pokecen_pc.c
 * @author obata
 * @date   2009.12.07
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "gamesystem/gamesystem.h"  // for GMEVENT, GAMESYS_WORK
#include "field/fieldmap_proc.h"  // for FIELDMAP_WORK


//-----------------------------------------------------------------------------------------
/**
 * @brief ポケセンのPC起動イベントを作成する
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------
extern GMEVENT* EVENT_PokecenPcOn( GMEVENT* parent, 
                                   GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );

//-----------------------------------------------------------------------------------------
/**
 * @brief ポケセンのPC起動中イベントを作成する
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------
extern GMEVENT* EVENT_PokecenPcRun( GMEVENT* parent, 
                                    GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );

//-----------------------------------------------------------------------------------------
/**
 * @brief ポケセンのPC停止イベントを作成する
 *
 * @param parent   親イベント
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return 作成したイベント
 */
//-----------------------------------------------------------------------------------------
extern GMEVENT* EVENT_PokecenPcOff( GMEVENT* parent, 
                                    GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap );
