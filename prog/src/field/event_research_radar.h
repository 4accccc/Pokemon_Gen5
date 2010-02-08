///////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー画面呼び出しイベント
 * @file   event_research_radar.c
 * @author obata
 * @date   2010.02.02
 */
///////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"

//-------------------------------------------------------------------------------------
/**
 * @brief 調査レーダー画面呼び出しイベント 生成
 *
 * @param gameSystem
 * @param fieldmap
 */
//-------------------------------------------------------------------------------------
extern GMEVENT* EVENT_ResearchRadar( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap );
