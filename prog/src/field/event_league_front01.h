////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケモンリーグフロント ( 上 ) でのイベント
 * @file   event_league_front01.h
 * @author obata
 * @date   2010.04.09
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"

//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベントを生成する
 * 
 * @param gameSystem
 * @param fieldmap
 *
 * @return リフト降下イベント
 */
//------------------------------------------------------------------------------------------
extern GMEVENT* EVENT_LFRONT01_LiftDown( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap );
